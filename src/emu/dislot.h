#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DISLOT_H__
#define __DISLOT_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
// ======================> device_slot_interface

struct slot_interface
{
	const char *	name;
	const device_type & devtype;
};

#define MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp) \
	device_slot_interface::static_set_slot_info(*device, SLOT_INTERFACE_NAME(_slot_intf), _def_slot, DEVICE_INPUT_DEFAULTS_NAME(_def_inp));

#define SLOT_INTERFACE_NAME(name)	slot_interface_##name

#define SLOT_INTERFACE_START(name)								\
	const slot_interface slot_interface_##name[] =				\
	{															\

#define SLOT_INTERFACE(tag,device) \
	{ tag, device }, \

#define SLOT_INTERFACE_END \
		{ NULL, NULL }							\
	};

class device_slot_card_interface;

class device_slot_interface : public device_interface
{
public:
	// construction/destruction
	device_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_slot_interface();

	static void static_set_slot_info(device_t &device, const slot_interface *slots_info, const char *default_card,const input_device_default *default_input);
	const slot_interface* get_slot_interfaces() const { return m_slot_interfaces; };
	virtual const char * get_default_card(const device_list &devlist, emu_options &options) const { return m_default_card; };
	const input_device_default *input_ports_defaults() const { return m_input_defaults; }
	device_t* get_card_device();
protected:
	const char *m_default_card;
	const input_device_default *m_input_defaults;
	const slot_interface *m_slot_interfaces;
};

// ======================> device_slot_card_interface

class device_slot_card_interface : public device_interface
{
public:
	// construction/destruction
	device_slot_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_slot_card_interface();
};

#endif  /* __DISLOT_H__ */
