/***************************************************************************

    device.c

    Device interface functions.

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
#include "debug/debugcpu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define TEMP_STRING_POOL_ENTRIES		16
#define MAX_STRING_LENGTH				256



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

static char temp_string_pool[TEMP_STRING_POOL_ENTRIES][MAX_STRING_LENGTH];
static int temp_string_pool_index;



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  get_temp_string_buffer - return a pointer to
//  a temporary string buffer
//-------------------------------------------------

char *get_temp_string_buffer(void)
{
	char *string = &temp_string_pool[temp_string_pool_index++ % TEMP_STRING_POOL_ENTRIES][0];
	string[0] = 0;
	return string;
}


resource_pool &machine_get_pool(running_machine &machine)
{
	// temporary to get around include dependencies, until CPUs
	// get a proper device class
	return machine.respool();
}



//**************************************************************************
//  DEVICE LIST MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_list - device list constructor
//-------------------------------------------------

device_list::device_list(resource_pool &pool)
	: tagged_list<device_t>(pool)
{
}


//-------------------------------------------------
//  set_machine_all - once the machine is created,
//  tell every device about it
//-------------------------------------------------

void device_list::set_machine_all(running_machine &machine)
{
	// add exit and reset callbacks
	m_machine = &machine;

	// iterate over devices and set their machines as well
	for (device_t *device = first(); device != NULL; device = device->next())
		device->set_machine(machine);
}


//-------------------------------------------------
//  start_all - start all the devices in the
//  list
//-------------------------------------------------

void device_list::start_all()
{
	// add exit and reset callbacks
	machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(device_list::reset_all), this));
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(device_list::exit), this));

	// add pre-save and post-load callbacks
	machine().save().register_presave(save_prepost_delegate(FUNC(device_list::presave_all), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(device_list::postload_all), this));

	// start_new_devices does all the necessary work
	start_new_devices();
}


//-------------------------------------------------
//  start_new_devices - start any unstarted devices
//-------------------------------------------------

void device_list::start_new_devices()
{
	assert(m_machine != NULL);

	// iterate through the devices
	device_t *nextdevice;
	for (device_t *device = first(); device != NULL; device = nextdevice)
	{
		// see if this device is what we want
		nextdevice = device->next();
		if (!device->started())
		{
			// attempt to start the device, catching any expected exceptions
			try
			{
				// if the device doesn't have a machine yet, set it first
				if (device->m_machine == NULL)
					device->set_machine(machine());

				// now start the device
				mame_printf_verbose("Starting %s '%s'\n", device->name(), device->tag());
				device->start();
			}

			// handle missing dependencies by moving the device to the end
			catch (device_missing_dependencies &)
			{
				// if we're the end, fail
				mame_printf_verbose("  (missing dependencies; rescheduling)\n");
				if (nextdevice == NULL)
					throw emu_fatalerror("Circular dependency in device startup; unable to start %s '%s'\n", device->name(), device->tag());
				detach(*device);
				append(device->tag(), *device);
			}
		}
	}
}


//-------------------------------------------------
//  reset_all - reset all devices in the list
//-------------------------------------------------

void device_list::reset_all()
{
	// iterate over devices and reset them
	for (device_t *device = first(); device != NULL; device = device->next())
		device->reset();
}


//-------------------------------------------------
//  stop_all - stop all the devices in the
//  list
//-------------------------------------------------

void device_list::stop_all()
{
	// iterate over devices and stop them
	for (device_t *device = first(); device != NULL; device = device->next())
		device->stop();

	// leave with no machine
	m_machine = NULL;
}


//-------------------------------------------------
//  first - return the first device of the given
//  type
//-------------------------------------------------

device_t *device_list::first(device_type type) const
{
	device_t *cur;
	for (cur = super::first(); cur != NULL && cur->type() != type; cur = cur->next()) ;
	return cur;
}


//-------------------------------------------------
//  count - count the number of devices of the
//  given type
//-------------------------------------------------

int device_list::count(device_type type) const
{
	int num = 0;
	for (const device_t *curdev = first(type); curdev != NULL; curdev = curdev->typenext()) num++;
	return num;
}


//-------------------------------------------------
//  indexof - return the index of the given device
//  among its kind
//-------------------------------------------------

int device_list::indexof(device_type type, device_t &object) const
{
	int num = 0;
	for (device_t *cur = first(type); cur != NULL; cur = cur->typenext(), num++)
		if (cur == &object) return num;
	return -1;
}


//-------------------------------------------------
//  indexof - return the index of the given device
//  among its kind
//-------------------------------------------------

int device_list::indexof(device_type type, const char *tag) const
{
	device_t *object = find(tag);
	return (object != NULL && object->type() == type) ? indexof(type, *object) : -1;
}


//-------------------------------------------------
//  find - find a device by type + index
//-------------------------------------------------

device_t *device_list::find(device_type type, int index) const
{
	for (device_t *cur = first(type); cur != NULL; cur = cur->typenext())
		if (index-- == 0) return cur;
	return NULL;
}


//-------------------------------------------------
//  static_exit - tear down all the devices
//-------------------------------------------------

void device_list::exit()
{
	// first let the debugger save comments
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_comment_save(machine());

	// stop all the devices before we go away
	stop_all();

	// then nuke the devices
	reset();
}


//-------------------------------------------------
//  presave_all - tell all the devices we are
//  about to save
//-------------------------------------------------

void device_list::presave_all()
{
	for (device_t *device = first(); device != NULL; device = device->next())
		device->pre_save();
}


//-------------------------------------------------
//  postload_all - tell all the devices we just
//  completed a load
//-------------------------------------------------

void device_list::postload_all()
{
	for (device_t *device = first(); device != NULL; device = device->next())
		device->post_load();
}



//**************************************************************************
//  LIVE DEVICE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_t - constructor for a new
//  running device; initial state is derived
//  from the provided config
//-------------------------------------------------

device_t::device_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: m_debug(NULL),
	  m_execute(NULL),
	  m_memory(NULL),
	  m_state(NULL),
	  m_next(NULL),
	  m_owner(owner),
	  m_interface_list(NULL),
	  m_type(type),
	  m_configured_clock(clock),
	  m_machine_config(mconfig),
	  m_static_config(NULL),
	  m_input_defaults(NULL),
	  m_name(name),
	  m_started(false),
	  m_clock(clock),
	  m_region(NULL),
	  m_unscaled_clock(clock),
	  m_clock_scale(1.0),
	  m_attoseconds_per_clock((clock == 0) ? 0 : HZ_TO_ATTOSECONDS(clock)),
	  m_auto_finder_list(NULL),
	  m_machine(NULL),
	  m_save(NULL),
	  m_tag(tag),
	  m_config_complete(false)
{
	static_set_clock(*this, clock);
}


device_t::device_t(const machine_config &mconfig, device_type type, const char *name, const char *shortname, const char *tag, device_t *owner, UINT32 clock)
	: m_debug(NULL),
	  m_execute(NULL),
	  m_memory(NULL),
	  m_state(NULL),
	  m_next(NULL),
	  m_owner(owner),
	  m_interface_list(NULL),
	  m_type(type),
	  m_configured_clock(clock),
	  m_machine_config(mconfig),
	  m_static_config(NULL),
	  m_input_defaults(NULL),
	  m_name(name),
	  m_shortname(shortname),
	  m_searchpath(shortname),
	  m_started(false),
	  m_clock(clock),
	  m_region(NULL),
	  m_unscaled_clock(clock),
	  m_clock_scale(1.0),
	  m_attoseconds_per_clock((clock == 0) ? 0 : HZ_TO_ATTOSECONDS(clock)),
	  m_auto_finder_list(NULL),
	  m_machine(NULL),
	  m_save(NULL),
	  m_tag(tag),
	  m_config_complete(false)
{
	static_set_clock(*this, clock);
}


//-------------------------------------------------
//  ~device_t - destructor for a device_t
//-------------------------------------------------

device_t::~device_t()
{
}


//-------------------------------------------------
//  subregion - return a pointer to the region
//  info for a given region
//-------------------------------------------------

const memory_region *device_t::subregion(const char *_tag) const
{
	// safety first
	if (this == NULL)
		return NULL;

	// build a fully-qualified name
	astring tempstring;
	return machine().region(subtag(tempstring, _tag));
}


//-------------------------------------------------
//  subdevice - return a pointer to the given
//  device that is owned by us
//-------------------------------------------------

device_t *device_t::subdevice(const char *_tag) const
{
	// safety first
	if (this == NULL)
		return NULL;

	// build a fully-qualified name
	astring tempstring;
	return mconfig().devicelist().find((const char *)subtag(tempstring, _tag));
}


//-------------------------------------------------
//  siblingdevice - return a pointer to the given
//  device that is owned by our same owner
//-------------------------------------------------

device_t *device_t::siblingdevice(const char *_tag) const
{
	// safety first
	if (this == NULL)
		return NULL;

	// build a fully-qualified name
	astring tempstring;
	return mconfig().devicelist().find((const char *)siblingtag(tempstring, _tag));
}


//-------------------------------------------------
//  static_set_clock - set/change the clock on
//  a device
//-------------------------------------------------

void device_t::static_set_clock(device_t &device, UINT32 clock)
{
	// derive the clock from our owner if requested
	if ((clock & 0xff000000) == 0xff000000)
	{
		assert(device.m_owner != NULL);
		clock = device.m_owner->m_configured_clock * ((clock >> 12) & 0xfff) / ((clock >> 0) & 0xfff);
	}

	device.m_clock = device.m_unscaled_clock = device.m_configured_clock = clock;
	device.m_attoseconds_per_clock = (clock == 0) ? 0 : HZ_TO_ATTOSECONDS(clock);
}


//-------------------------------------------------
//  config_complete - called when the
//  configuration of a device is complete
//-------------------------------------------------

void device_t::config_complete()
{
	// first notify the interfaces
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_config_complete();

	// then notify the device itself
	device_config_complete();

	// then mark ourselves complete
	m_config_complete = true;
}


//-------------------------------------------------
//  validity_check - validate a device after the
//  configuration has been constructed
//-------------------------------------------------

bool device_t::validity_check(emu_options &options, const game_driver &driver) const
{
	bool error = false;

	// validate via the interfaces
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		if (intf->interface_validity_check(options, driver))
			error = true;

	// let the device itself validate
	if (device_validity_check(options, driver))
		error = true;

	return error;
}


//-------------------------------------------------
//  reset - reset a device
//-------------------------------------------------

void device_t::reset()
{
	// let the interfaces do their pre-work
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_pre_reset();

	// reset the device
	device_reset();

	// let the interfaces do their post-work
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_post_reset();
}


//-------------------------------------------------
//  set_unscaled_clock - sets the given device's
//  unscaled clock
//-------------------------------------------------

void device_t::set_unscaled_clock(UINT32 clock)
{
	m_unscaled_clock = clock;
	m_clock = m_unscaled_clock * m_clock_scale;
	m_attoseconds_per_clock = HZ_TO_ATTOSECONDS(m_clock);
	notify_clock_changed();
}


//-------------------------------------------------
//  set_clock_scale - sets a scale factor for the
//  device's clock
//-------------------------------------------------

void device_t::set_clock_scale(double clockscale)
{
	m_clock_scale = clockscale;
	m_clock = m_unscaled_clock * m_clock_scale;
	m_attoseconds_per_clock = HZ_TO_ATTOSECONDS(m_clock);
	notify_clock_changed();
}


//-------------------------------------------------
//  clocks_to_attotime - converts a number of
//  clock ticks to an attotime
//-------------------------------------------------

attotime device_t::clocks_to_attotime(UINT64 numclocks) const
{
	if (numclocks < m_clock)
		return attotime(0, numclocks * m_attoseconds_per_clock);
	else
	{
		UINT32 remainder;
		UINT32 quotient = divu_64x32_rem(numclocks, m_clock, &remainder);
		return attotime(quotient, (UINT64)remainder * (UINT64)m_attoseconds_per_clock);
	}
}


//-------------------------------------------------
//  attotime_to_clocks - converts a duration as
//  attotime to CPU clock ticks
//-------------------------------------------------

UINT64 device_t::attotime_to_clocks(attotime duration) const
{
	return mulu_32x32(duration.seconds, m_clock) + (UINT64)duration.attoseconds / (UINT64)m_attoseconds_per_clock;
}


//-------------------------------------------------
//  timer_alloc - allocate a timer for our device
//  callback
//-------------------------------------------------

emu_timer *device_t::timer_alloc(device_timer_id id, void *ptr)
{
	return machine().scheduler().timer_alloc(*this, id, ptr);
}


//-------------------------------------------------
//  timer_set - set a temporary timer that will
//  call our device callback
//-------------------------------------------------

void device_t::timer_set(attotime duration, device_timer_id id, int param, void *ptr)
{
	machine().scheduler().timer_set(duration, *this, id, param, ptr);
}


//-------------------------------------------------
//  set_machine - notify that the machine now
//  exists
//-------------------------------------------------

void device_t::set_machine(running_machine &machine)
{
	m_machine = &machine;
	m_save = &machine.save();
}


//-------------------------------------------------
//  start - start a device
//-------------------------------------------------

void device_t::start()
{
	// populate the machine and the region field
	m_region = machine().region(tag());

	// find all the registered devices
	for (auto_finder_base *autodev = m_auto_finder_list; autodev != NULL; autodev = autodev->m_next)
		autodev->findit(*this);

	// let the interfaces do their pre-work
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_pre_start();

	// remember the number of state registrations
	int state_registrations = machine().save().registration_count();

	// start the device
	device_start();

	// complain if nothing was registered by the device
	state_registrations = machine().save().registration_count() - state_registrations;
	device_execute_interface *exec;
	device_sound_interface *sound;
	if (state_registrations == 0 && (interface(exec) || interface(sound)))
	{
		logerror("Device '%s' did not register any state to save!\n", tag());
		if ((machine().system().flags & GAME_SUPPORTS_SAVE) != 0)
			fatalerror("Device '%s' did not register any state to save!", tag());
	}

	// let the interfaces do their post-work
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_post_start();

	// force an update of the clock
	notify_clock_changed();

	// if we're debugging, create a device_debug object
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		m_debug = auto_alloc(machine(), device_debug(*this));
		debug_setup();
	}

	// register our save states
	save_item(NAME(m_clock));
	save_item(NAME(m_unscaled_clock));
	save_item(NAME(m_clock_scale));

	// we're now officially started
	m_started = true;
}


//-------------------------------------------------
//  stop - stop a device
//-------------------------------------------------

void device_t::stop()
{
	// let the interfaces do their pre-work
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_pre_stop();

	// stop the device
	device_stop();

	// let the interfaces do their post-work
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_post_stop();

	// free any debugging info
	auto_free(machine(), m_debug);

	// we're now officially stopped, and the machine is off-limits
	m_started = false;
	m_machine = NULL;
}


//-------------------------------------------------
//  debug_setup - set up for debugging
//-------------------------------------------------

void device_t::debug_setup()
{
	// notify the interface
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_debug_setup();

	// notify the device
	device_debug_setup();
}


//-------------------------------------------------
//  pre_save - tell the device and its interfaces
//  that we are about to save
//-------------------------------------------------

void device_t::pre_save()
{
	// notify the interface
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_pre_save();

	// notify the device
	device_pre_save();
}


//-------------------------------------------------
//  post_load - tell the device and its interfaces
//  that we just completed a load
//-------------------------------------------------

void device_t::post_load()
{
	// notify the interface
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_post_load();

	// notify the device
	device_post_load();
}


//-------------------------------------------------
//  notify_clock_changed - notify all interfaces
//  that the clock has changed
//-------------------------------------------------

void device_t::notify_clock_changed()
{
	// first notify interfaces
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_clock_changed();

	// then notify the device
	device_clock_changed();
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void device_t::device_config_complete()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_validity_check - validate a device after
//  the configuration has been constructed
//-------------------------------------------------

bool device_t::device_validity_check(emu_options &options, const game_driver &driver) const
{
	// indicate no error by default
	return false;
}


//-------------------------------------------------
//  rom_region - return a pointer to the implicit
//  rom region description for this device
//-------------------------------------------------

const rom_entry *device_t::device_rom_region() const
{
	// none by default
	return NULL;
}


//-------------------------------------------------
//  machine_config - return a pointer to a machine
//  config constructor describing sub-devices for
//  this device
//-------------------------------------------------

machine_config_constructor device_t::device_mconfig_additions() const
{
	// none by default
	return NULL;
}



//-------------------------------------------------
//  input_ports - return a pointer to the implicit
//  input ports description for this device
//-------------------------------------------------

ioport_constructor device_t::device_input_ports() const
{
	// none by default
	return NULL;
}


//-------------------------------------------------
//  device_reset - actually handle resetting of
//  a device; designed to be overriden by the
//  actual device implementation
//-------------------------------------------------

void device_t::device_reset()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_stop - clean up anything that needs to
//  happen before the running_machine goes away
//-------------------------------------------------

void device_t::device_stop()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_pre_save - called prior to saving the
//  state, so that registered variables can be
//  properly normalized
//-------------------------------------------------

void device_t::device_pre_save()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_post_load - called after the loading a
//  saved state, so that registered variables can
//  be expaneded as necessary
//-------------------------------------------------

void device_t::device_post_load()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_clock_changed - called when the
//  device clock is altered in any way; designed
//  to be overridden by the actual device
//  implementation
//-------------------------------------------------

void device_t::device_clock_changed()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_debug_setup - called when the debugger
//  is active to allow for device-specific setup
//-------------------------------------------------

void device_t::device_debug_setup()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_timer - called whenever a device timer
//  fires
//-------------------------------------------------

void device_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	// do nothing by default
}


//-------------------------------------------------
//  register_auto_finder - add a new item to the
//  list of stuff to find after we go live
//-------------------------------------------------

void device_t::register_auto_finder(auto_finder_base &autodev)
{
	// add to this list
	autodev.m_next = m_auto_finder_list;
	m_auto_finder_list = &autodev;
}


//-------------------------------------------------
//  auto_finder_base - constructor
//-------------------------------------------------

device_t::auto_finder_base::auto_finder_base(device_t &base, const char *tag)
	: m_next(NULL),
	  m_tag(tag)
{
	// register ourselves with our device class
	base.register_auto_finder(*this);
}


//-------------------------------------------------
//  ~auto_finder_base - destructor
//-------------------------------------------------

device_t::auto_finder_base::~auto_finder_base()
{
}


//-------------------------------------------------
//  find_device - find a device; done here instead
//  of inline in the template due to include
//  dependency ordering
//-------------------------------------------------

device_t *device_t::auto_finder_base::find_device(device_t &base, const char *tag)
{
	return base.subdevice(tag);
}


//-------------------------------------------------
//  find_shared_ptr - find a shared pointer
//-------------------------------------------------

void *device_t::auto_finder_base::find_shared_ptr(device_t &base, const char *tag)
{
	return memory_get_shared(base.machine(), tag);
}


//-------------------------------------------------
//  find_shared_size - find a shared pointer size
//-------------------------------------------------

size_t device_t::auto_finder_base::find_shared_size(device_t &base, const char *tag)
{
	size_t result = 0;
	memory_get_shared(base.machine(), tag, result);
	return result;
}



//**************************************************************************
//  LIVE DEVICE INTERFACES
//**************************************************************************

//-------------------------------------------------
//  device_interface - constructor
//-------------------------------------------------

device_interface::device_interface(device_t &device)
	: m_interface_next(NULL),
	  m_device(device)
{
	device_interface **tailptr;
	for (tailptr = &device.m_interface_list; *tailptr != NULL; tailptr = &(*tailptr)->m_interface_next) ;
	*tailptr = this;
}


//-------------------------------------------------
//  ~device_interface - destructor
//-------------------------------------------------

device_interface::~device_interface()
{
}


//-------------------------------------------------
//  interface_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void device_interface::interface_config_complete()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_validity_check - default validation
//  for a device after the configuration has been
//  constructed
//-------------------------------------------------

bool device_interface::interface_validity_check(emu_options &options, const game_driver &driver) const
{
	return false;
}


//-------------------------------------------------
//  interface_pre_start - called before the
//  device's own start function
//-------------------------------------------------

void device_interface::interface_pre_start()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_post_start - called after the
//  device's own start function
//-------------------------------------------------

void device_interface::interface_post_start()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_pre_reset - called before the
//  device's own reset function
//-------------------------------------------------

void device_interface::interface_pre_reset()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_post_reset - called after the
//  device's own reset function
//-------------------------------------------------

void device_interface::interface_post_reset()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_pre_stop - called before the
//  device's own stop function
//-------------------------------------------------

void device_interface::interface_pre_stop()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_post_stop - called after the
//  device's own stop function
//-------------------------------------------------

void device_interface::interface_post_stop()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_pre_save - called prior to saving the
//  state, so that registered variables can be
//  properly normalized
//-------------------------------------------------

void device_interface::interface_pre_save()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_post_load - called after the loading a
//  saved state, so that registered variables can
//  be expaneded as necessary
//-------------------------------------------------

void device_interface::interface_post_load()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_clock_changed - called when the
//  device clock is altered in any way; designed
//  to be overridden by the actual device
//  implementation
//-------------------------------------------------

void device_interface::interface_clock_changed()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_debug_setup - called to allow
//  interfaces to set up any debugging for this
//  device
//-------------------------------------------------

void device_interface::interface_debug_setup()
{
	// do nothing by default
}
