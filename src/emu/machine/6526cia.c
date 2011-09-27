/**********************************************************************

    MOS 6526/8520 CIA interface and emulation

    This function emulates all the functionality MOS6526 or
    MOS8520 complex interface adapters.

**********************************************************************/

#include "emu.h"
#include "6526cia.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* CIA registers */
#define CIA_PRA			0
#define CIA_PRB			1
#define CIA_DDRA		2
#define CIA_DDRB		3
#define CIA_TALO		4
#define CIA_TAHI		5
#define CIA_TBLO		6
#define CIA_TBHI		7
#define CIA_TOD0		8		/* 6526: 1/10 seconds   8520: bits  0- 7 */
#define CIA_TOD1		9		/* 6526: seconds        8520: bits  8-15 */
#define CIA_TOD2		10		/* 6526: minutes        8520: bits 16-23 */
#define CIA_TOD3		11		/* 6526: hours          8520: N/A */
#define CIA_SDR			12
#define CIA_ICR			13
#define CIA_CRA			14
#define CIA_CRB			15

#define CIA_CRA_START	0x01
#define	CIA_CRA_PBON	0x02
#define	CIA_CRA_OUTMODE	0x04
#define	CIA_CRA_RUNMODE	0x08
#define	CIA_CRA_LOAD	0x10
#define	CIA_CRA_INMODE	0x20
#define	CIA_CRA_SPMODE	0x40
#define	CIA_CRA_TODIN	0x80

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  mos6526_device_config - constructor
//-------------------------------------------------

mos6526_device_config::mos6526_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
    : device_config(mconfig, static_alloc_device_config, "MOS6526", tag, owner, clock)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *mos6526_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
    return global_alloc(mos6526_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *mos6526_device_config::alloc_device(running_machine &machine) const
{
    return auto_alloc(machine, mos6526_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mos6526_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const mos6526_interface *intf = reinterpret_cast<const mos6526_interface *>(static_config());
	if (intf != NULL)
		*static_cast<mos6526_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_tod_clock = 0;
    	memset(&m_out_irq_func, 0, sizeof(m_out_irq_func));
    	memset(&m_out_pc_func, 0, sizeof(m_out_pc_func));
    	memset(&m_out_cnt_func, 0, sizeof(m_out_cnt_func));
    	memset(&m_out_sp_func, 0, sizeof(m_out_sp_func));
    	memset(&m_in_pa_func, 0, sizeof(m_in_pa_func));
    	memset(&m_out_pa_func, 0, sizeof(m_out_pa_func));
    	memset(&m_in_pb_func, 0, sizeof(m_in_pb_func));
    	memset(&m_out_pb_func, 0, sizeof(m_out_pb_func));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

const device_type MOS6526R1 = mos6526_device_config::static_alloc_device_config;
const device_type MOS6526R2 = mos6526_device_config::static_alloc_device_config;
const device_type MOS8520 = mos6526_device_config::static_alloc_device_config;

//-------------------------------------------------
//  mos6526_device - constructor
//-------------------------------------------------

mos6526_device::mos6526_device(running_machine &_machine, const mos6526_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6526_device::device_reset()
{
	/* clear things out */
	m_port[0].m_latch = 0x00;
	m_port[0].m_in = 0x00;
	m_port[0].m_out = 0x00;
	m_port[0].m_mask_value = 0xff;
	m_port[1].m_latch = 0x00;
	m_port[1].m_in = 0x00;
	m_port[1].m_out = 0x00;
	m_port[1].m_mask_value = 0xff;
	m_tod = 0;
	m_tod_latch = 0;
	m_alarm = 0;
	m_icr = 0x00;
	m_ics = 0x00;
	m_irq = 0;
	m_shift = 0;
	m_loaded = 0;
	m_cnt = 1;
	m_sp = 0;

	/* initialize data direction registers */
	m_port[0].m_ddr = !strcmp(tag(), "cia_0") ? 0x03 : 0xff;
	m_port[1].m_ddr = !strcmp(tag(), "cia_0") ? 0x00 : 0xff;

	/* TOD running by default */
	m_tod_running = TRUE;

	/* initialize timers */
	for(int t = 0; t < 2; t++)
	{
		cia_timer *timer = &m_timer[t];
		timer->m_cia = this;
		timer->m_clock = clock();
		timer->m_latch = 0xffff;
		timer->m_count = 0x0000;
		timer->m_mode = 0x00;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6526_device::device_start()
{
	/* clear out CIA structure, and copy the interface */
	devcb_resolve_write_line(&m_out_irq_func, &m_config.m_out_irq_func, this);
	devcb_resolve_write_line(&m_out_pc_func, &m_config.m_out_pc_func, this);
	devcb_resolve_write_line(&m_out_cnt_func, &m_config.m_out_cnt_func, this);
	devcb_resolve_write_line(&m_out_sp_func, &m_config.m_out_sp_func, this);
	m_flag = 1;

	/* setup ports */
	devcb_resolve_read8(&m_port[0].m_read, &m_config.m_in_pa_func, this);
	devcb_resolve_write8(&m_port[0].m_write, &m_config.m_out_pa_func, this);
	devcb_resolve_read8(&m_port[1].m_read, &m_config.m_in_pb_func, this);
	devcb_resolve_write8(&m_port[1].m_write, &m_config.m_out_pb_func, this);

	for (int p = 0; p < (sizeof(m_port) / sizeof(m_port[0])); p++)
	{
		m_port[p].m_mask_value = 0xff;
	}

	/* setup timers */
	for (int t = 0; t < (sizeof(m_timer) / sizeof(m_timer[0])); t++)
	{
		cia_timer *timer = &m_timer[t];
		timer->m_timer = m_machine.scheduler().timer_alloc(FUNC(timer_proc), (void*)this);
		timer->m_cia = this;
		timer->m_irq = 0x01 << t;
	}

	/* setup TOD timer, if appropriate */
	if (m_config.m_tod_clock != 0)
	{
		m_machine.scheduler().timer_pulse(attotime::from_hz(m_config.m_tod_clock), FUNC(clock_tod_callback), 0, (void *)this);
	}

	/* state save support */
	save_item(NAME(m_port[0].m_ddr));
	save_item(NAME(m_port[0].m_latch));
	save_item(NAME(m_port[0].m_in));
	save_item(NAME(m_port[0].m_out));
	save_item(NAME(m_port[0].m_mask_value));
	save_item(NAME(m_port[1].m_ddr));
	save_item(NAME(m_port[1].m_latch));
	save_item(NAME(m_port[1].m_in));
	save_item(NAME(m_port[1].m_out));
	save_item(NAME(m_port[1].m_mask_value));
	save_item(NAME(m_timer[0].m_latch));
	save_item(NAME(m_timer[0].m_count));
	save_item(NAME(m_timer[0].m_mode));
	save_item(NAME(m_timer[0].m_irq));
	save_item(NAME(m_timer[1].m_latch));
	save_item(NAME(m_timer[1].m_count));
	save_item(NAME(m_timer[1].m_mode));
	save_item(NAME(m_timer[1].m_irq));
	save_item(NAME(m_tod));
	save_item(NAME(m_tod_latch));
	save_item(NAME(m_tod_latched));
	save_item(NAME(m_tod_running));
	save_item(NAME(m_alarm));
	save_item(NAME(m_icr));
	save_item(NAME(m_ics));
	save_item(NAME(m_irq));
	save_item(NAME(m_flag));
	save_item(NAME(m_loaded));
	save_item(NAME(m_sdr));
	save_item(NAME(m_sp));
	save_item(NAME(m_cnt));
	save_item(NAME(m_shift));
	save_item(NAME(m_serial));
}


/*-------------------------------------------------
    set_port_mask_value
-------------------------------------------------*/

void mos6526_device::set_port_mask_value(int port, int data)
{
	m_port[port].m_mask_value = data;
}

/*-------------------------------------------------
    update_pc - pulse /pc output
-------------------------------------------------*/

void mos6526_device::update_pc()
{
	/* this should really be one cycle long */
	devcb_call_write_line(&m_out_pc_func, 0);
	devcb_call_write_line(&m_out_pc_func, 1);
}

/*-------------------------------------------------
    update_interrupts
-------------------------------------------------*/

void mos6526_device::update_interrupts()
{
	UINT8 new_irq;

	/* always update the high bit of ICS */
	if (m_ics & 0x7f)
	{
		m_ics |= 0x80;
	}
	else
	{
		m_ics &= ~0x80;
	}

	/* based on what is enabled, set/clear the IRQ via the custom chip */
	new_irq = (m_ics & m_icr) ? 1 : 0;
	if (m_irq != new_irq)
	{
		m_irq = new_irq;
		devcb_call_write_line(&m_out_irq_func, m_irq);
	}
}


/*-------------------------------------------------
    timer_bump
-------------------------------------------------*/

void mos6526_device::timer_bump(int timer)
{
	m_timer[timer].update(timer, -1);

	if (m_timer[timer].m_count == 0x00)
	{
		timer_underflow(timer);
	}
	else
	{
		m_timer[timer].update(timer, m_timer[timer].m_count - 1);
	}
}

/*-------------------------------------------------
    cia_timer_underflow
-------------------------------------------------*/

void mos6526_device::timer_underflow(int timer)
{
	assert((timer == 0) || (timer == 1));

	/* set the status and update interrupts */
	m_ics |= m_timer[timer].m_irq;
	update_interrupts();

	/* if one-shot mode, turn it off */
	if (m_timer[timer].m_mode & 0x08)
	{
		m_timer[timer].m_mode &= 0xfe;
	}

	/* reload the timer */
	m_timer[timer].update(timer, m_timer[timer].m_latch);

	/* timer A has some interesting properties */
	if (timer == 0)
	{
		/* such as cascading to timer B */
		if ((m_timer[1].m_mode & 0x41) == 0x41)
		{
			if (m_cnt || !(m_timer[1].m_mode & 0x20))
			{
				timer_bump(1);
			}
		}

		/* also the serial line */
		if ((m_timer[timer].m_irq == 0x01) && (m_timer[timer].m_mode & CIA_CRA_SPMODE))
		{
			if (m_loaded || m_shift)
			{
				/* falling edge */
				if (m_cnt)
				{
					if (m_shift == 0)
					{
						/* load shift register */
						m_loaded = 0;
						m_serial = m_sdr;
					}

					/* transmit MSB */
					m_sp = BIT(m_serial, 7);
					devcb_call_write_line(&m_out_sp_func, m_sp);

					/* toggle CNT */
					m_cnt = !m_cnt;
					devcb_call_write_line(&m_out_cnt_func, m_cnt);

					/* shift data */
					m_serial <<= 1;
					m_shift++;

					if (m_shift == 8)
					{
						/* signal interrupt */
						m_ics |= 0x08;
						update_interrupts();
					}
				}
				else
				{
					/* toggle CNT */
					m_cnt = !m_cnt;
					devcb_call_write_line(&m_out_cnt_func, m_cnt);

					if (m_shift == 8)
					{
						m_shift = 0;
					}
				}
			}
		}
	}
}

/*-------------------------------------------------
    TIMER_CALLBACK( cia_timer_proc )
-------------------------------------------------*/

TIMER_CALLBACK( mos6526_device::timer_proc )
{
    mos6526_device *cia = reinterpret_cast<mos6526_device *>(ptr);

	cia->timer_underflow(param);
}

/*-------------------------------------------------
    bcd_increment
-------------------------------------------------*/

static UINT8 bcd_increment(UINT8 value)
{
	value++;
	if ((value & 0x0f) >= 0x0a)
		value += 0x10 - 0x0a;
	return value;
}

/*-------------------------------------------------
    cia6526_increment
-------------------------------------------------*/

void mos6526_device::increment()
{
	/* break down TOD value into components */
	UINT8 subsecond	= (UINT8) (m_tod >>  0);
	UINT8 second	= (UINT8) (m_tod >>  8);
	UINT8 minute	= (UINT8) (m_tod >> 16);
	UINT8 hour		= (UINT8) (m_tod >> 24);

	subsecond = bcd_increment(subsecond);
	if (subsecond >= 0x10)
	{
		subsecond = 0x00;
		second = bcd_increment(second);
		if (second >= ((m_timer[0].m_mode & 0x80) ? 0x50 : 0x60))
		{
			second = 0x00;
			minute = bcd_increment(minute);
			if (minute >= 0x60)
			{
				minute = 0x00;
				if (hour == 0x91)
					hour = 0x00;
				else if (hour == 0x89)
					hour = 0x90;
				else if (hour == 0x11)
					hour = 0x80;
				else if (hour == 0x09)
					hour = 0x10;
				else
					hour++;
			}
		}
	}

	/* update the TOD with new value */
	m_tod = (((UINT32) subsecond)	<<  0)
		  | (((UINT32) second)		<<  8)
		  | (((UINT32) minute)		<< 16)
		  | (((UINT32) hour)		<< 24);
}

/*-------------------------------------------------
    cia_clock_tod - Update TOD on CIA A
-------------------------------------------------*/

void mos6526_device::clock_tod()
{
	if (m_tod_running)
	{
		if ((type() == MOS6526R1) || (type() == MOS6526R2))
		{
			/* The 6526 split the value into hours, minutes, seconds and
             * subseconds */
			increment();
		}
		else if (type() == MOS8520)
		{
			/* the 8520 has a straight 24-bit counter */
			m_tod++;
			m_tod &= 0xffffff;
		}

		if (m_tod == m_alarm)
		{
			m_ics |= 0x04;
			update_interrupts();
		}
	}
}


/*-------------------------------------------------
    clock_tod_callback
-------------------------------------------------*/

TIMER_CALLBACK( mos6526_device::clock_tod_callback )
{
    mos6526_device *cia = reinterpret_cast<mos6526_device *>(ptr);
	cia->clock_tod();
}


/*-------------------------------------------------
    cnt_w
-------------------------------------------------*/

void mos6526_device::cnt_w(UINT8 state)
{
	/* is this a rising edge? */
	if (!m_cnt && state)
	{
		if (m_timer[0].m_mode & CIA_CRA_START)
		{
			/* does timer #0 bump on CNT? */
			if (m_timer[0].m_mode & CIA_CRA_INMODE)
			{
				timer_bump(0);
			}
		}

		/* if the serial port is set to input, the CNT will shift the port */
		if (!(m_timer[0].m_mode & CIA_CRA_SPMODE))
		{
			m_serial <<= 1;
			m_shift++;

			if (m_sp)
			{
				m_serial |= 0x01;
			}

			if (m_shift == 8)
			{
				m_sdr = m_serial;
				m_serial = 0;
				m_shift = 0;
				m_ics |= 0x08;
				update_interrupts();
			}
		}

		/* does timer #1 bump on CNT? */
		if ((m_timer[1].m_mode & 0x61) == 0x21)
		{
			timer_bump(1);
		}
	}

	m_cnt = state;
}

void mos6526_device::flag_w(UINT8 state)
{
	/* falling edge */
	if (m_flag && !state)
	{
		m_ics |= 0x10;
		update_interrupts();
	}

	m_flag = state;
}

/*-------------------------------------------------
    reg_r
-------------------------------------------------*/

UINT8 mos6526_device::reg_r(UINT8 offset)
{
	cia_timer *timer;
	cia_port *port;
	UINT8 data = 0x00;

	offset &= 0x0F;

	switch(offset)
	{
		/* port A/B data */
		case CIA_PRA:
		case CIA_PRB:
			port = &m_port[offset & 1];
			data = devcb_call_read8(&port->m_read, 0);
			data = ((data & ~port->m_ddr) | (port->m_latch & port->m_ddr)) & port->m_mask_value;

			port->m_in = data;

			if (offset == CIA_PRB)
			{
				/* timer #0 can change PB6 */
				if (m_timer[0].m_mode & 0x02)
				{
					m_timer[0].update(0, -1);
					if (m_timer[0].m_count != 0)
					{
						data |= 0x40;
					}
					else
					{
						data &= ~0x40;
					}
				}

				/* timer #1 can change PB7 */
				if (m_timer[1].m_mode & 0x02)
				{
					m_timer[1].update(1, -1);
					if (m_timer[1].m_count != 0)
					{
						data |= 0x80;
					}
					else
					{
						data &= ~0x80;
					}
				}

				/* pulse /PC following the read */
				update_pc();
			}
			break;

		/* port A/B direction */
		case CIA_DDRA:
		case CIA_DDRB:
			port = &m_port[offset & 1];
			data = port->m_ddr;
			break;

		/* timer A/B low byte */
		case CIA_TALO:
		case CIA_TBLO:
			timer = &m_timer[(offset >> 1) & 1];
			data = timer->get_count() >> 0;
			break;

		/* timer A/B high byte */
		case CIA_TAHI:
		case CIA_TBHI:
			timer = &m_timer[(offset >> 1) & 1];
			data = timer->get_count() >> 8;
			break;

		/* TOD counter */
		case CIA_TOD0:
		case CIA_TOD1:
		case CIA_TOD2:
		case CIA_TOD3:
			if (type() == MOS8520)
			{
				if (offset == CIA_TOD2)
				{
					m_tod_latch = m_tod;
					m_tod_latched = TRUE;
				}
			}
			else
			{
				if (offset == CIA_TOD3)
				{
					m_tod_latch = m_tod;
					m_tod_latched = TRUE;
				}
			}
			if (offset == CIA_TOD0)
			{
				m_tod_latched = FALSE;
			}

			if (m_tod_latched)
			{
				data = m_tod_latch >> ((offset - CIA_TOD0) * 8);
			}
			else
			{
				data = m_tod >> ((offset - CIA_TOD0) * 8);
			}
			break;

		/* serial data ready */
		case CIA_SDR:
			data = m_sdr;
			break;

		/* interrupt status/clear */
		case CIA_ICR:
			data = m_ics;
			m_ics = 0; /* clear on read */
			update_interrupts();
			break;

		/* timer A/B mode */
		case CIA_CRA:
		case CIA_CRB:
			timer = &m_timer[offset & 1];
			data = timer->m_mode;
			break;
	}

	return data;
}

/*-------------------------------------------------
    reg_w
-------------------------------------------------*/

void mos6526_device::reg_w(UINT8 offset, UINT8 data)
{
	cia_timer *timer;
	cia_port *port;
	int shift;

	offset &= 0x0F;

	switch(offset)
	{
		/* port A/B data */
		case CIA_PRA:
		case CIA_PRB:
			port = &m_port[offset & 1];
			port->m_latch = data;
			port->m_out = (data & port->m_ddr) | (port->m_in & ~port->m_ddr);
			devcb_call_write8(&port->m_write, 0, port->m_out);

			/* pulse /PC following the write */
			if (offset == CIA_PRB)
			{
				update_pc();
			}

			break;

		/* port A/B direction */
		case CIA_DDRA:
		case CIA_DDRB:
			port = &m_port[offset & 1];
			port->m_ddr = data;
			break;

		/* timer A/B latch low */
		case CIA_TALO:
		case CIA_TBLO:
			timer = &m_timer[(offset >> 1) & 1];
			timer->m_latch = (timer->m_latch & 0xff00) | (data << 0);
			break;

		/* timer A latch high */
		case CIA_TAHI:
		case CIA_TBHI:
			timer = &m_timer[(offset >> 1) & 1];
			timer->m_latch = (timer->m_latch & 0x00ff) | (data << 8);

			/* if the timer is one-shot, then force a start on it */
			if (timer->m_mode & 0x08)
			{
				timer->m_mode |= 1;
				timer->update((offset >> 1) & 1, timer->m_latch);
			}
			else
			{
				/* if the timer is off, update the count */
				if (!(timer->m_mode & 0x01))
				{
					timer->update((offset >> 1) & 1, timer->m_latch);
				}
			}
			break;

		/* time of day latches */
		case CIA_TOD0:
		case CIA_TOD1:
		case CIA_TOD2:
		case CIA_TOD3:
			shift = 8 * ((offset - CIA_TOD0));

			/* alarm setting mode? */
			if (m_timer[1].m_mode & 0x80)
			{
				m_alarm = (m_alarm & ~(0xff << shift)) | (data << shift);
			}
			/* counter setting mode */
			else
			{
				m_tod = (m_tod & ~(0xff << shift)) | (data << shift);
			}

			if (type() == MOS8520)
			{
				if (offset == CIA_TOD2)
				{
					m_tod_running = FALSE;
				}
			}
			else
			{
				if (offset == CIA_TOD3)
				{
					m_tod_running = FALSE;
				}
			}
			if (offset == CIA_TOD0)
			{
				m_tod_running = TRUE;
			}
			break;

		/* serial data ready */
		case CIA_SDR:
			m_sdr = data;
			if (m_timer[0].m_mode & 0x40)
			{
				m_loaded = 1;
			}
			break;

		/* interrupt control register */
		case CIA_ICR:
			if (data & 0x80)
			{
				m_icr |= data & 0x7f;
			}
			else
			{
				m_icr &= ~(data & 0x7f);
			}
			update_interrupts();
			break;

		/* timer A/B modes */
		case CIA_CRA:
		case CIA_CRB:
			timer = &m_timer[offset & 1];
			timer->m_mode = data & 0xef;

			/* force load? */
			if (data & 0x10)
			{
				timer->update(offset & 1, timer->m_latch);
			}
			else
			{
				timer->update(offset & 1, -1);
			}
			break;
	}
}

/*-------------------------------------------------
    is_timer_active
-------------------------------------------------*/

static int is_timer_active(emu_timer *timer)
{
	attotime t = timer->expire();
	return (t != attotime::never);
}

/*-------------------------------------------------
    update - updates the count and emu_timer for
    a given CIA timer
-------------------------------------------------*/

void mos6526_device::cia_timer::update(int which, INT32 new_count)
{
	/* sanity check arguments */
	assert((new_count >= -1) && (new_count <= 0xffff));

	/* update the timer count, if necessary */
	if ((new_count == -1) && is_timer_active(m_timer))
	{
		UINT16 current_count = (m_timer->elapsed() * m_clock).as_double();
		m_count = m_count - MIN(m_count, current_count);
	}

	/* set the timer if we are instructed to */
	if (new_count != -1)
	{
		m_count = new_count;
	}

	/* now update the MAME timer */
	if ((m_mode & 0x01) && ((m_mode & (which ? 0x60 : 0x20)) == 0x00))
	{
		/* timer is on and is connected to clock */
		attotime period = attotime::from_hz(m_clock) * (m_count ? m_count : 0x10000);
		m_timer->adjust(period, which);
	}
	else
	{
		/* timer is off or not connected to clock */
		m_timer->adjust(attotime::never, which);
	}
}

/*-------------------------------------------------
    get_count - get the count for a given CIA
    timer
-------------------------------------------------*/

UINT16 mos6526_device::cia_timer::get_count()
{
	UINT16 count;

	if (is_timer_active(m_timer))
	{
		UINT16 current_count = (m_timer->elapsed() * m_clock).as_double();
		count = m_count - MIN(m_count, current_count);
	}
	else
	{
		count = m_count;
	}

	return count;
}

/***************************************************************************
    TRAMPOLINES
***************************************************************************/

void cia_set_port_mask_value(device_t *device, int port, int data) { downcast<mos6526_device *>(device)->set_port_mask_value(port, data); }

READ8_DEVICE_HANDLER( mos6526_r ) { return downcast<mos6526_device *>(device)->reg_r(offset); }
WRITE8_DEVICE_HANDLER( mos6526_w ) { downcast<mos6526_device *>(device)->reg_w(offset, data); }

READ8_DEVICE_HANDLER( mos6526_pa_r ) { return downcast<mos6526_device *>(device)->pa_r(offset); }
READ8_DEVICE_HANDLER( mos6526_pb_r ) { return downcast<mos6526_device *>(device)->pb_r(offset); }

READ_LINE_DEVICE_HANDLER( mos6526_irq_r ) { return downcast<mos6526_device *>(device)->irq_r(); }

WRITE_LINE_DEVICE_HANDLER( mos6526_tod_w ) { downcast<mos6526_device *>(device)->tod_w(state); }

READ_LINE_DEVICE_HANDLER( mos6526_cnt_r ) { return downcast<mos6526_device *>(device)->cnt_r(); }
WRITE_LINE_DEVICE_HANDLER( mos6526_cnt_w ) { downcast<mos6526_device *>(device)->cnt_w(state); }

READ_LINE_DEVICE_HANDLER( mos6526_sp_r ) { return downcast<mos6526_device *>(device)->sp_r(); }
WRITE_LINE_DEVICE_HANDLER( mos6526_sp_w ) { downcast<mos6526_device *>(device)->sp_w(state); }

WRITE_LINE_DEVICE_HANDLER( mos6526_flag_w ) { downcast<mos6526_device *>(device)->flag_w(state); }
