/**********************************************************************

    Intel 8155 - 2048-Bit Static MOS RAM with I/O Ports and Timer emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - strobed mode

*/

#include "emu.h"
#include "i8155.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1

enum
{
	REGISTER_COMMAND = 0,
	REGISTER_STATUS = 0,
	REGISTER_PORT_A,
	REGISTER_PORT_B,
	REGISTER_PORT_C,
	REGISTER_TIMER_LOW,
	REGISTER_TIMER_HIGH
};

enum
{
	PORT_A = 0,
	PORT_B,
	PORT_C,
	PORT_COUNT
};

enum
{
	PORT_MODE_INPUT = 0,
	PORT_MODE_OUTPUT,
	PORT_MODE_STROBED_PORT_A,	// not supported
	PORT_MODE_STROBED			// not supported
};

enum
{
	MEMORY = 0,
	IO
};

#define COMMAND_PA					0x01
#define COMMAND_PB					0x02
#define COMMAND_PC_MASK				0x0c
#define COMMAND_PC_ALT_1			0x00
#define COMMAND_PC_ALT_2			0x0c
#define COMMAND_PC_ALT_3			0x04	// not supported
#define COMMAND_PC_ALT_4			0x08	// not supported
#define COMMAND_IEA					0x10	// not supported
#define COMMAND_IEB					0x20	// not supported
#define COMMAND_TM_MASK				0xc0
#define COMMAND_TM_NOP				0x00
#define COMMAND_TM_STOP				0x40
#define COMMAND_TM_STOP_AFTER_TC	0x80
#define COMMAND_TM_START			0xc0

#define STATUS_INTR_A				0x01	// not supported
#define STATUS_A_BF					0x02	// not supported
#define STATUS_INTE_A				0x04	// not supported
#define STATUS_INTR_B				0x08	// not supported
#define STATUS_B_BF					0x10	// not supported
#define STATUS_INTE_B				0x20	// not supported
#define STATUS_TIMER				0x40

#define TIMER_MODE_MASK				0xc0
#define TIMER_MODE_LOW				0x00
#define TIMER_MODE_SQUARE_WAVE		0x40
#define TIMER_MODE_SINGLE_PULSE		0x80
#define TIMER_MODE_AUTOMATIC_RELOAD	0xc0



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type I8155 = i8155_device_config::static_alloc_device_config;


// default address map
static ADDRESS_MAP_START( i8155, AS_0, 8 )
	AM_RANGE(0x00, 0xff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  i8155_device_config - constructor
//-------------------------------------------------

i8155_device_config::i8155_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "Intel 8155", tag, owner, clock),
	  device_config_memory_interface(mconfig, *this),
	  m_space_config("ram", ENDIANNESS_LITTLE, 8, 8, 0, NULL, *ADDRESS_MAP_NAME(i8155))
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *i8155_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(i8155_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *i8155_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(machine, i8155_device(machine, *this));
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *i8155_device_config::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8155_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const i8155_interface *intf = reinterpret_cast<const i8155_interface *>(static_config());
	if (intf != NULL)
		*static_cast<i8155_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&in_pa_func, 0, sizeof(in_pa_func));
		memset(&out_pa_func, 0, sizeof(out_pa_func));
		memset(&in_pb_func, 0, sizeof(in_pb_func));
		memset(&out_pb_func, 0, sizeof(out_pb_func));
		memset(&in_pc_func, 0, sizeof(in_pc_func));
		memset(&out_pc_func, 0, sizeof(out_pc_func));
		memset(&out_to_func, 0, sizeof(out_to_func));
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline UINT8 i8155_device::get_timer_mode()
{
	return (m_count_length >> 8) & TIMER_MODE_MASK;
}

inline void i8155_device::timer_output()
{
	devcb_call_write_line(&m_out_to_func, m_to);

	if (LOG) logerror("8155 '%s' Timer Output: %u\n", tag(), m_to);
}

inline void i8155_device::pulse_timer_output()
{
	m_to = 0; timer_output();
	m_to = 1; timer_output();
}

inline int i8155_device::get_port_mode(int port)
{
	int mode = -1;

	switch (port)
	{
	case PORT_A:
		mode = (m_command & COMMAND_PA) ? PORT_MODE_OUTPUT : PORT_MODE_INPUT;
		break;

	case PORT_B:
		mode = (m_command & COMMAND_PB) ? PORT_MODE_OUTPUT : PORT_MODE_INPUT;
		break;

	case PORT_C:
		switch (m_command & COMMAND_PC_MASK)
		{
		case COMMAND_PC_ALT_1: mode = PORT_MODE_INPUT;			break;
		case COMMAND_PC_ALT_2: mode = PORT_MODE_OUTPUT;			break;
		case COMMAND_PC_ALT_3: mode = PORT_MODE_STROBED_PORT_A; break;
		case COMMAND_PC_ALT_4: mode = PORT_MODE_STROBED;		break;
		}
		break;
	}

	return mode;
}

inline UINT8 i8155_device::read_port(int port)
{
	UINT8 data = 0;

	switch (port)
	{
	case PORT_A:
	case PORT_B:
		switch (get_port_mode(port))
		{
		case PORT_MODE_INPUT:
			data = devcb_call_read8(&m_in_port_func[port], 0);
			break;

		case PORT_MODE_OUTPUT:
			data = m_output[port];
			break;
		}
		break;

	case PORT_C:
		switch (get_port_mode(PORT_C))
		{
		case PORT_MODE_INPUT:
			data = devcb_call_read8(&m_in_port_func[port], 0) & 0x3f;
			break;

		case PORT_MODE_OUTPUT:
			data = m_output[port] & 0x3f;
			break;

		default:
			logerror("8155 '%s' Unsupported Port C mode!\n", tag());
		}
		break;
	}

	return data;
}

inline void i8155_device::write_port(int port, UINT8 data)
{
	switch (get_port_mode(port))
	{
	case PORT_MODE_OUTPUT:
		m_output[port] = data;
		devcb_call_write8(&m_out_port_func[port], 0, m_output[port]);
		break;
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8155_device - constructor
//-------------------------------------------------

i8155_device::i8155_device(running_machine &_machine, const i8155_device_config &config)
    : device_t(_machine, config),
	  device_memory_interface(_machine, config, *this),
      m_config(config)
{

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8155_device::device_start()
{
	// resolve callbacks
	devcb_resolve_read8(&m_in_port_func[0], &m_config.in_pa_func, this);
	devcb_resolve_read8(&m_in_port_func[1], &m_config.in_pb_func, this);
	devcb_resolve_read8(&m_in_port_func[2], &m_config.in_pc_func, this);
	devcb_resolve_write8(&m_out_port_func[0], &m_config.out_pa_func, this);
	devcb_resolve_write8(&m_out_port_func[1], &m_config.out_pb_func, this);
	devcb_resolve_write8(&m_out_port_func[2], &m_config.out_pc_func, this);
	devcb_resolve_write_line(&m_out_to_func, &m_config.out_to_func, this);

	// allocate timers
	m_timer = timer_alloc();

	// register for state saving
	save_item(NAME(m_io_m));
	save_item(NAME(m_ad));
	save_item(NAME(m_command));
	save_item(NAME(m_status));
	save_item(NAME(m_output));
	save_item(NAME(m_count_length));
	save_item(NAME(m_counter));
	save_item(NAME(m_to));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8155_device::device_reset()
{
	// clear output registers
	m_output[PORT_A] = 0;
	m_output[PORT_B] = 0;
	m_output[PORT_C] = 0;

	// set ports to input mode
	register_w(REGISTER_COMMAND, m_command & ~(COMMAND_PA | COMMAND_PB | COMMAND_PC_MASK));

	// clear timer flag
	m_status &= ~STATUS_TIMER;

	// stop counting
	m_timer->enable(0);

	// clear timer output
	m_to = 1;
	timer_output();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void i8155_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	// count down
	m_counter--;

	if (get_timer_mode() == TIMER_MODE_LOW)
	{
		// pulse on every count
		pulse_timer_output();
	}

	if (m_counter == 0)
	{
		if (LOG) logerror("8155 '%s' Timer Count Reached\n", tag());

		switch (m_command & COMMAND_TM_MASK)
		{
		case COMMAND_TM_STOP_AFTER_TC:
			// stop timer
			m_timer->enable(0);

			if (LOG) logerror("8155 '%s' Timer Stopped\n", tag());
			break;
		}

		switch (get_timer_mode())
		{
		case TIMER_MODE_SQUARE_WAVE:
			// toggle timer output
			m_to = !m_to;
			timer_output();
			break;

		case TIMER_MODE_SINGLE_PULSE:
			// single pulse upon TC being reached
			pulse_timer_output();

			// clear timer mode setting
			m_command &= ~COMMAND_TM_MASK;
			break;

		case TIMER_MODE_AUTOMATIC_RELOAD:
			// automatic reload, i.e. single pulse every time TC is reached
			pulse_timer_output();
			break;
		}

		// set timer flag
		m_status |= STATUS_TIMER;

		// reload timer counter
		m_counter = m_count_length & 0x3fff;
	}
}


//-------------------------------------------------
//  io_r - register read
//-------------------------------------------------

READ8_MEMBER( i8155_device::io_r )
{
	UINT8 data = 0;

	switch (offset & 0x03)
	{
	case REGISTER_STATUS:
		data = m_status;

		// clear timer flag
		m_status &= ~STATUS_TIMER;
		break;

	case REGISTER_PORT_A:
		data = read_port(PORT_A);
		break;

	case REGISTER_PORT_B:
		data = read_port(PORT_B);
		break;

	case REGISTER_PORT_C:
		data = read_port(PORT_C);
		break;
	}

	return data;
}


//-------------------------------------------------
//  register_w - register write
//-------------------------------------------------

void i8155_device::register_w(int offset, UINT8 data)
{
	switch (offset & 0x07)
	{
	case REGISTER_COMMAND:
		m_command = data;

		if (LOG) logerror("8155 '%s' Port A Mode: %s\n", tag(), (data & COMMAND_PA) ? "output" : "input");
		if (LOG) logerror("8155 '%s' Port B Mode: %s\n", tag(), (data & COMMAND_PB) ? "output" : "input");

		if (LOG) logerror("8155 '%s' Port A Interrupt: %s\n", tag(), (data & COMMAND_IEA) ? "enabled" : "disabled");
		if (LOG) logerror("8155 '%s' Port B Interrupt: %s\n", tag(), (data & COMMAND_IEB) ? "enabled" : "disabled");

		switch (data & COMMAND_PC_MASK)
		{
		case COMMAND_PC_ALT_1:
			if (LOG) logerror("8155 '%s' Port C Mode: Alt 1\n", tag());
			break;

		case COMMAND_PC_ALT_2:
			if (LOG) logerror("8155 '%s' Port C Mode: Alt 2\n", tag());
			break;

		case COMMAND_PC_ALT_3:
			if (LOG) logerror("8155 '%s' Port C Mode: Alt 3\n", tag());
			break;

		case COMMAND_PC_ALT_4:
			if (LOG) logerror("8155 '%s' Port C Mode: Alt 4\n", tag());
			break;
		}

		switch (data & COMMAND_TM_MASK)
		{
		case COMMAND_TM_NOP:
			// do not affect counter operation
			break;

		case COMMAND_TM_STOP:
			// NOP if timer has not started, stop counting if the timer is running
			if (LOG) logerror("8155 '%s' Timer Command: Stop\n", tag());
			m_timer->enable(0);
			break;

		case COMMAND_TM_STOP_AFTER_TC:
			// stop immediately after present TC is reached (NOP if timer has not started)
			if (LOG) logerror("8155 '%s' Timer Command: Stop after TC\n", tag());
			break;

		case COMMAND_TM_START:
			if (LOG) logerror("8155 '%s' Timer Command: Start\n", tag());

			if (m_timer->enabled())
			{
				// if timer is running, start the new mode and CNT length immediately after present TC is reached
			}
			else
			{
				// load mode and CNT length and start immediately after loading (if timer is not running)
				m_counter = m_count_length;
				m_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));
			}
			break;
		}
		break;

	case REGISTER_PORT_A:
		write_port(PORT_A, data);
		break;

	case REGISTER_PORT_B:
		write_port(PORT_B, data);
		break;

	case REGISTER_PORT_C:
		write_port(PORT_C, data);
		break;

	case REGISTER_TIMER_LOW:
		m_count_length = (m_count_length & 0xff00) | data;
		if (LOG) logerror("8155 '%s' Count Length Low: %04x\n", tag(), m_count_length);
		break;

	case REGISTER_TIMER_HIGH:
		m_count_length = (data << 8) | (m_count_length & 0xff);
		if (LOG) logerror("8155 '%s' Count Length High: %04x\n", tag(), m_count_length);

		switch (data & TIMER_MODE_MASK)
		{
		case TIMER_MODE_LOW:
			// puts out LOW during second half of count
			if (LOG) logerror("8155 '%s' Timer Mode: LOW\n", tag());
			break;

		case TIMER_MODE_SQUARE_WAVE:
			// square wave, i.e. the period of the square wave equals the count length programmed with automatic reload at terminal count
			if (LOG) logerror("8155 '%s' Timer Mode: Square wave\n", tag());
			break;

		case TIMER_MODE_SINGLE_PULSE:
			// single pulse upon TC being reached
			if (LOG) logerror("8155 '%s' Timer Mode: Single pulse\n", tag());
			break;

		case TIMER_MODE_AUTOMATIC_RELOAD:
			// automatic reload, i.e. single pulse every time TC is reached
			if (LOG) logerror("8155 '%s' Timer Mode: Automatic reload\n", tag());
			break;
		}
		break;
	}
}

//-------------------------------------------------
//  io_w - register write
//-------------------------------------------------

WRITE8_MEMBER( i8155_device::io_w )
{
	register_w(offset, data);
}


//-------------------------------------------------
//  memory_r - internal RAM read
//-------------------------------------------------

READ8_MEMBER( i8155_device::memory_r )
{
	return this->space()->read_byte(offset);
}


//-------------------------------------------------
//  memory_w - internal RAM write
//-------------------------------------------------

WRITE8_MEMBER( i8155_device::memory_w )
{
	this->space()->write_byte(offset, data);
}


//-------------------------------------------------
//  ale_w - address latch write
//-------------------------------------------------

WRITE8_MEMBER( i8155_device::ale_w )
{
	// I/O / memory select
	m_io_m = BIT(offset, 0);

	// address
	m_ad = data;
}


//-------------------------------------------------
//  read - memory or I/O read
//-------------------------------------------------

READ8_MEMBER( i8155_device::read )
{
	UINT8 data = 0;

	switch (m_io_m)
	{
	case MEMORY:
		data = memory_r(space, m_ad);
		break;

	case IO:
		data = io_r(space, m_ad);
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - memory or I/O write
//-------------------------------------------------

WRITE8_MEMBER( i8155_device::write )
{
	switch (m_io_m)
	{
	case MEMORY:
		memory_w(space, m_ad, data);
		break;

	case IO:
		io_w(space, m_ad, data);
		break;
	}
}
