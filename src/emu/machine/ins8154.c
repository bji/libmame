/***************************************************************************

    National Semiconductor INS8154

    N-Channel 128-by-8 Bit RAM Input/Output (RAM I/O)


    TODO: Strobed modes

***************************************************************************/

#include "emu.h"
#include "ins8154.h"
#include "devhelpr.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define VERBOSE 1

/* Mode Definition Register */
enum
{
	MDR_BASIC                 = 0x00,
	MDR_STROBED_INPUT         = 0x20,
	MDR_STROBED_OUTPUT        = 0x60,
	MDR_STROBED_OUTPUT_3STATE = 0xe0
};


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(ins8154, "INS8154")

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ins8154_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const ins8154_interface *intf = reinterpret_cast<const ins8154_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<ins8154_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_in_a_func, 0, sizeof(m_in_a_func));
    	memset(&m_in_b_func, 0, sizeof(m_in_b_func));
    	memset(&m_out_a_func, 0, sizeof(m_out_a_func));
    	memset(&m_out_b_func, 0, sizeof(m_out_b_func));
    	memset(&m_out_irq_func, 0, sizeof(m_out_irq_func));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

const device_type INS8154 = ins8154_device_config::static_alloc_device_config;

//-------------------------------------------------
//  ins8154_device - constructor
//-------------------------------------------------

ins8154_device::ins8154_device(running_machine &_machine, const ins8154_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{

}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ins8154_device::device_start()
{
	/* resolve callbacks */
	devcb_resolve_read8(&m_in_a_func, &m_config.m_in_a_func, this);
	devcb_resolve_write8(&m_out_a_func, &m_config.m_out_a_func, this);
	devcb_resolve_read8(&m_in_b_func, &m_config.m_in_b_func, this);
	devcb_resolve_write8(&m_out_b_func, &m_config.m_out_b_func, this);
	devcb_resolve_write_line(&m_out_irq_func, &m_config.m_out_irq_func, this);

	/* register for state saving */
	state_save_register_device_item(this, 0, m_in_a);
	state_save_register_device_item(this, 0, m_in_b);
	state_save_register_device_item(this, 0, m_out_a);
	state_save_register_device_item(this, 0, m_out_b);
	state_save_register_device_item(this, 0, m_mdr);
	state_save_register_device_item(this, 0, m_odra);
	state_save_register_device_item(this, 0, m_odrb);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ins8154_device::device_reset()
{
	m_in_a = 0;
	m_in_b = 0;
	m_out_a = 0;
	m_out_b = 0;
	m_mdr = 0;
	m_odra = 0;
	m_odrb = 0;
}


READ8_DEVICE_HANDLER_TRAMPOLINE(ins8154, ins8154_r)
{
	UINT8 val = 0xff;

	if (offset > 0x24)
	{
		if (VERBOSE)
		{
			logerror("%s: INS8154 '%s' Read from invalid offset %02x!\n", cpuexec_describe_context(&m_machine), tag(), offset);
		}
		return 0xff;
	}

	switch (offset)
	{
	case 0x20:
		if(m_in_a_func.read != NULL)
		{
			val = devcb_call_read8(&m_in_a_func, 0);
		}
		m_in_a = val;
		break;

	case 0x21:
		if(m_in_b_func.read != NULL)
		{
			val = devcb_call_read8(&m_in_b_func, 0);
		}
		m_in_b = val;
		break;

	default:
		if (offset < 0x08)
		{
			if(m_in_a_func.read != NULL)
			{
				val = (devcb_call_read8(&m_in_a_func, 0) << (8 - offset)) & 0x80;
			}
			m_in_a = val;
		}
		else
		{
			if(m_in_b_func.read != NULL)
			{
				val = (devcb_call_read8(&m_in_b_func, 0) << (8 - (offset >> 4))) & 0x80;
			}
			m_in_b = val;
		}
		break;
	}

	return val;
}

WRITE8_DEVICE_HANDLER_TRAMPOLINE(ins8154, ins8154_porta_w)
{
	m_out_a = data;

	/* Test if any pins are set as outputs */
	if (m_odra)
	{
		devcb_call_write8(&m_out_a_func, 0, (data & m_odra) | (m_odra ^ 0xff));
	}
}

WRITE8_DEVICE_HANDLER_TRAMPOLINE(ins8154, ins8154_portb_w)
{
	m_out_b = data;

	/* Test if any pins are set as outputs */
	if (m_odrb)
	{
		devcb_call_write8(&m_out_b_func, 0, (data & m_odrb) | (m_odrb ^ 0xff));
	}
}

WRITE8_DEVICE_HANDLER_TRAMPOLINE(ins8154, ins8154_w)
{
	if (offset > 0x24)
	{
		if (VERBOSE)
		{
			logerror("%s: INS8154 '%s' Write %02x to invalid offset %02x!\n", cpuexec_describe_context(&m_machine), tag(), data, offset);
		}
		return;
	}

	switch (offset)
	{
	case 0x20:
		ins8154_porta_w(0, data);
		break;

	case 0x21:
		ins8154_portb_w(0, data);
		break;

	case 0x22:
		if (VERBOSE)
		{
			logerror("%s: INS8154 '%s' ODRA set to %02x\n", cpuexec_describe_context(&m_machine), tag(), data);
		}

		m_odra = data;
		break;

	case 0x23:
		if (VERBOSE)
		{
			logerror("%s: INS8154 '%s' ODRB set to %02x\n", cpuexec_describe_context(&m_machine), tag(), data);
		}

		m_odrb = data;
		break;

	case 0x24:
		if (VERBOSE)
		{
			logerror("%s: INS8154 '%s' MDR set to %02x\n", cpuexec_describe_context(&m_machine), tag(), data);
		}

		m_mdr = data;
		break;

	default:
		if (offset & 0x10)
		{
			/* Set bit */
			if (offset < 0x08)
			{
				ins8154_porta_w(0, m_out_a |= offset & 0x07);
			}
			else
			{
				ins8154_portb_w(0, m_out_b |= (offset >> 4) & 0x07);
			}
		}
		else
		{
			/* Clear bit */
			if (offset < 0x08)
			{
				ins8154_porta_w(0, m_out_a & ~(offset & 0x07));
			}
			else
			{
				ins8154_portb_w(0, m_out_b & ~((offset >> 4) & 0x07));
			}
		}

		break;
	}
}
