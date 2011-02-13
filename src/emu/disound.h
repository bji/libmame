/***************************************************************************

    disound.h

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

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DISOUND_H__
#define __DISOUND_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int MAX_OUTPUTS		= 4095;			// maximum number of outputs a sound chip can support
const int ALL_OUTPUTS		= MAX_OUTPUTS;	// special value indicating all outputs for the current chip



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SOUND_ADD(_tag, _type, _clock) \
	MCFG_DEVICE_ADD(_tag, _type, _clock) \

#define MCFG_SOUND_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)

#define MCFG_SOUND_CLOCK(_clock) \
	MCFG_DEVICE_CLOCK(_clock)

#define MCFG_SOUND_REPLACE(_tag, _type, _clock) \
	MCFG_DEVICE_REPLACE(_tag, _type, _clock)

#define MCFG_SOUND_CONFIG(_config) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_SOUND_ROUTE_EX(_output, _target, _gain, _input) \
	device_config_sound_interface::static_add_route(device, _output, _target, _gain, _input); \

#define MCFG_SOUND_ROUTE(_output, _target, _gain) \
	MCFG_SOUND_ROUTE_EX(_output, _target, _gain, 0)

#define MCFG_SOUND_ROUTES_RESET() \
	device_config_sound_interface::static_reset_routes(device); \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sound_stream;


// ======================> device_config_sound_interface

// device_config_sound_interface represents configuration information for a sound device
class device_config_sound_interface : public device_config_interface
{
public:
	// construction/destruction
	device_config_sound_interface(const machine_config &mconfig, device_config &devconfig);
	virtual ~device_config_sound_interface();

	class sound_route
	{
	public:
		sound_route(int output, int input, float gain, const char *target);

		sound_route *		m_next;				// pointer to next route
		UINT32				m_output;			// output index, or ALL_OUTPUTS
		UINT32				m_input;			// target input index
		float				m_gain;				// gain
		const char *		m_target;			// target tag
	};

	sound_route *		m_route_list;			// list of sound routes

	// static inline helpers
	static void static_add_route(device_config *device, UINT32 output, const char *target, double gain, UINT32 input = 0);
	static void static_reset_routes(device_config *device);

protected:
	// optional operation overrides
	virtual bool interface_validity_check(const game_driver &driver) const;

	void reset_routes();
};



// ======================> device_sound_interface

class device_sound_interface : public device_interface
{
public:
	// construction/destruction
	device_sound_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_sound_interface();

	// configuration access
	const device_config_sound_interface &sound_config() const { return m_sound_config; }

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

protected:
	// optional operation overrides
	virtual void interface_pre_start();
	virtual void interface_post_start();

	struct sound_output
	{
		sound_stream *	stream;					// associated stream
		int				output;					// output number
	};

	int				m_outputs;					// number of outputs from this instance
	sound_output	m_output[MAX_OUTPUTS];		// array of output information

	const device_config_sound_interface &m_sound_config;
};



#endif	/* __DISOUND_H__ */
