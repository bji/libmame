/***************************************************************************

    disound.c

    Device sound interfaces.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"



//**************************************************************************
//  DEVICE CONFIG SOUND INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_sound_interface - constructor
//-------------------------------------------------

device_sound_interface::device_sound_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device),
	  m_outputs(0),
	  m_auto_allocated_inputs(0)
{
}


//-------------------------------------------------
//  ~device_sound_interface - destructor
//-------------------------------------------------

device_sound_interface::~device_sound_interface()
{
}


//-------------------------------------------------
//  static_add_route - configuration helper to add
//  a new route to the device
//-------------------------------------------------

void device_sound_interface::static_add_route(device_t &device, UINT32 output, const char *target, double gain, UINT32 input)
{
	// find our sound interface
	device_sound_interface *sound;
	if (!device.interface(sound))
		throw emu_fatalerror("MCFG_SOUND_ROUTE called on device '%s' with no sound interface", device.tag());

	// append a new route to the list
	astring devtag;
	device.siblingtag(devtag, target);
	sound->m_route_list.append(*global_alloc(sound_route(output, input, gain, devtag.cstr())));
}


//-------------------------------------------------
//  static_reset_routes - configuration helper to
//  reset all existing routes to the device
//-------------------------------------------------

void device_sound_interface::static_reset_routes(device_t &device)
{
	// find our sound interface
	device_sound_interface *sound;
	if (!device.interface(sound))
		throw emu_fatalerror("MCFG_SOUND_ROUTES_RESET called on device '%s' with no sound interface", device.tag());

	// reset the routine list
	sound->m_route_list.reset();
}


//-------------------------------------------------
//  stream_alloc - allocate a stream implicitly
//  associated with this device
//-------------------------------------------------

sound_stream *device_sound_interface::stream_alloc(int inputs, int outputs, int sample_rate)
{
	return device().machine().sound().stream_alloc(*this, inputs, outputs, sample_rate);
}


//-------------------------------------------------
//  inputs - return the total number of inputs
//  for the given device
//-------------------------------------------------

int device_sound_interface::inputs() const
{
	// scan the list counting streams we own and summing their inputs
	int inputs = 0;
	for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != NULL; stream = stream->next())
		if (&stream->device() == &m_device)
			inputs += stream->input_count();
	return inputs;
}


//-------------------------------------------------
//  outputs - return the total number of outputs
//  for the given device
//-------------------------------------------------

int device_sound_interface::outputs() const
{
	// scan the list counting streams we own and summing their outputs
	int outputs = 0;
	for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != NULL; stream = stream->next())
		if (&stream->device() == &m_device)
			outputs += stream->output_count();
	return outputs;
}


//-------------------------------------------------
//  input_to_stream_input - convert a device's
//  input index to a stream and the input index
//  on that stream
//-------------------------------------------------

sound_stream *device_sound_interface::input_to_stream_input(int inputnum, int &stream_inputnum)
{
	assert(inputnum >= 0);

	// scan the list looking for streams owned by this device
	for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != NULL; stream = stream->next())
		if (&stream->device() == &m_device)
		{
			if (inputnum < stream->input_count())
			{
				stream_inputnum = inputnum;
				return stream;
			}
			inputnum -= stream->input_count();
		}

	// not found
	return NULL;
}


//-------------------------------------------------
//  output_to_stream_output - convert a device's
//  output index to a stream and the output index
//  on that stream
//-------------------------------------------------

sound_stream *device_sound_interface::output_to_stream_output(int outputnum, int &stream_outputnum)
{
	assert(outputnum >= 0);

	// scan the list looking for streams owned by this device
	for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != NULL; stream = stream->next())
		if (&stream->device() == &device())
		{
			if (outputnum < stream->output_count())
			{
				stream_outputnum = outputnum;
				return stream;
			}
			outputnum -= stream->output_count();
		}

	// not found
	return NULL;
}


//-------------------------------------------------
//  set_output_gain - set the gain on the given
//  output index of the device
//-------------------------------------------------

void device_sound_interface::set_output_gain(int outputnum, float gain)
{
	// handle ALL_OUTPUTS as a special case
	if (outputnum == ALL_OUTPUTS)
	{
		for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != NULL; stream = stream->next())
			if (&stream->device() == &device())
				for (int outputnum = 0; outputnum < stream->output_count(); outputnum++)
					stream->set_output_gain(outputnum, gain);
	}

	// look up the stream and stream output index
	else
	{
		int stream_outputnum;
		sound_stream *stream = output_to_stream_output(outputnum, stream_outputnum);
		if (stream != NULL)
			stream->set_output_gain(stream_outputnum, gain);
	}
}


//-------------------------------------------------
//  interface_validity_check - validation for a
//  device after the configuration has been
//  constructed
//-------------------------------------------------

void device_sound_interface::interface_validity_check(validity_checker &valid) const
{
	// loop over all the routes
	for (const sound_route *route = first_route(); route != NULL; route = route->next())
	{
		// find a device with the requested tag
		const device_t *target = device().siblingdevice(route->m_target.cstr());
		if (target == NULL)
			mame_printf_error("Attempting to route sound to non-existant device '%s'\n", route->m_target.cstr());

		// if it's not a speaker or a sound device, error
		const device_sound_interface *sound;
		if (target != NULL && target->type() != SPEAKER && !target->interface(sound))
			mame_printf_error("Attempting to route sound to a non-sound device '%s' (%s)\n", route->m_target.cstr(), target->name());
	}
}


//-------------------------------------------------
//  interface_pre_start - make sure all our input
//  devices are started
//-------------------------------------------------

void device_sound_interface::interface_pre_start()
{
	// scan all the sound devices
	sound_interface_iterator iter(m_device.machine().root_device());
	for (device_sound_interface *sound = iter.first(); sound != NULL; sound = iter.next())
	{
		// scan each route on the device
		for (const sound_route *route = sound->first_route(); route != NULL; route = route->next())
		{
			// see if we are the target of this route; if we are, make sure the source device is started
			device_t *target_device = m_device.machine().device(route->m_target);
			if (target_device == &m_device && !sound->device().started())
				throw device_missing_dependencies();
		}
	}

	// now iterate through devices again and assign any auto-allocated inputs
	m_auto_allocated_inputs = 0;
	for (device_sound_interface *sound = iter.first(); sound != NULL; sound = iter.next())
	{
		// scan each route on the device
		for (const sound_route *route = sound->first_route(); route != NULL; route = route->next())
		{
			// see if we are the target of this route
			device_t *target_device = m_device.machine().device(route->m_target);
			if (target_device == &m_device && route->m_input == AUTO_ALLOC_INPUT)
			{
				const_cast<sound_route *>(route)->m_input = m_auto_allocated_inputs;
				m_auto_allocated_inputs += (route->m_output == ALL_OUTPUTS) ? sound->outputs() : 1;
			}
		}
	}
}


//-------------------------------------------------
//  interface_post_start - verify that state was
//  properly set up
//-------------------------------------------------

void device_sound_interface::interface_post_start()
{
	// iterate over all the sound devices
	sound_interface_iterator iter(m_device.machine().root_device());
	for (device_sound_interface *sound = iter.first(); sound != NULL; sound = iter.next())
	{
		// scan each route on the device
		for (const sound_route *route = sound->first_route(); route != NULL; route = route->next())
		{
			// if we are the target of this route, hook it up
			device_t *target_device = m_device.machine().device(route->m_target);
			if (target_device == &m_device)
			{
				// iterate over all outputs, matching any that apply
				int inputnum = route->m_input;
				int numoutputs = sound->outputs();
				for (int outputnum = 0; outputnum < numoutputs; outputnum++)
					if (route->m_output == outputnum || route->m_output == ALL_OUTPUTS)
					{
						// find the output stream to connect from
						int streamoutputnum;
						sound_stream *outputstream = sound->output_to_stream_output(outputnum, streamoutputnum);
						if (outputstream == NULL)
							fatalerror("Sound device '%s' specifies route for non-existant output #%d", route->m_target.cstr(), outputnum);

						// find the input stream to connect to
						int streaminputnum;
						sound_stream *inputstream = input_to_stream_input(inputnum++, streaminputnum);
						if (inputstream == NULL)
							fatalerror("Sound device '%s' targeted output #%d to non-existant device '%s' input %d", route->m_target.cstr(), outputnum, m_device.tag(), inputnum - 1);

						// set the input
						inputstream->set_input(streaminputnum, outputstream, streamoutputnum, route->m_gain);
					}
			}
		}
	}
}


//-------------------------------------------------
//  interface_pre_reset - called prior to
//  resetting the device
//-------------------------------------------------

void device_sound_interface::interface_pre_reset()
{
	// update all streams on this device prior to reset
	for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != NULL; stream = stream->next())
		if (&stream->device() == &device())
			stream->update();
}



//**************************************************************************
//  SOUND ROUTE
//**************************************************************************

//-------------------------------------------------
//  sound_route - constructor
//-------------------------------------------------

device_sound_interface::sound_route::sound_route(int output, int input, float gain, const char *target)
	: m_next(NULL),
	  m_output(output),
	  m_input(input),
	  m_gain(gain),
	  m_target(target)
{
}
