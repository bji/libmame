/*********************************************************************

    6850acia.c

    6850 ACIA code

*********************************************************************/

#include "emu.h"
#include "6850acia.h"
#include "devhelpr.h"


/***************************************************************************
    MACROS
***************************************************************************/

#define LOG 0

#define CR1_0	0x03
#define CR4_2	0x1C
#define CR6_5	0x60
#define CR7		0x80

#define TXD(_data) \
	devcb_call_write_line(&m_out_tx_func, _data)

#define RTS(_data) \
	devcb_call_write_line(&m_out_rts_func, _data)

/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

const int acia6850_device::ACIA6850_DIVIDE[3] = { 1, 16, 64 };

const int acia6850_device::ACIA6850_WORD[8][3] =
{
	{ 7, EVEN, 2 },
	{ 7, ODD,  2 },
	{ 7, EVEN, 1 },
	{ 7, ODD,  1 },
	{ 8, NONE, 2 },
	{ 8, NONE, 1 },
	{ 8, EVEN, 1 },
	{ 8, ODD,  1 }
};



/***************************************************************************
    LIVE DEVICE
***************************************************************************/

const device_type ACIA6850 = acia6850_device_config::static_alloc_device_config;

GENERIC_DEVICE_CONFIG_SETUP(acia6850, "6850 ACIA")

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void acia6850_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const acia6850_interface *intf = reinterpret_cast<const acia6850_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<acia6850_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		m_tx_clock = 0;
		m_rx_clock = 0;
    	memset(&m_in_rx_func, 0, sizeof(m_in_rx_func));
    	memset(&m_out_tx_func, 0, sizeof(m_out_tx_func));
    	memset(&m_in_cts_func, 0, sizeof(m_in_cts_func));
    	memset(&m_out_rts_func, 0, sizeof(m_out_rts_func));
    	memset(&m_in_dcd_func, 0, sizeof(m_in_dcd_func));
    	memset(&m_out_irq_func, 0, sizeof(m_out_irq_func));
	}
}


//-------------------------------------------------
//  acia6850_device - constructor
//-------------------------------------------------

acia6850_device::acia6850_device(running_machine &_machine, const acia6850_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acia6850_device::device_start()
{
	/* resolve callbacks */
	devcb_resolve_read_line(&m_in_rx_func, &m_config.m_in_rx_func, this);
	devcb_resolve_write_line(&m_out_tx_func, &m_config.m_out_tx_func, this);
	devcb_resolve_read_line(&m_in_cts_func, &m_config.m_in_cts_func, this);
	devcb_resolve_write_line(&m_out_rts_func, &m_config.m_out_rts_func, this);
	devcb_resolve_read_line(&m_in_dcd_func, &m_config.m_in_dcd_func, this);
	devcb_resolve_write_line(&m_out_irq_func, &m_config.m_out_irq_func, this);

	m_rx_clock = m_config.m_rx_clock;
	m_tx_clock = m_config.m_tx_clock;
	m_tx_counter = 0;
	m_rx_counter = 0;
	m_rx_timer = m_machine.scheduler().timer_alloc(FUNC(receive_event_callback), (void *)this);
	m_tx_timer = m_machine.scheduler().timer_alloc(FUNC(transmit_event_callback), (void *)this);
	m_first_reset = 1;
	m_status_read = 0;
	m_brk = 0;

	m_rx_timer->reset();
	m_tx_timer->reset();

	save_item(NAME(m_ctrl));
	save_item(NAME(m_status));
	save_item(NAME(m_rx_clock));
	save_item(NAME(m_tx_clock));
	save_item(NAME(m_rx_counter));
	save_item(NAME(m_tx_counter));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_rdr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_tx_int));

	save_item(NAME(m_divide));
	save_item(NAME(m_overrun));
	save_item(NAME(m_reset));
	save_item(NAME(m_first_reset));
	save_item(NAME(m_rts));
	save_item(NAME(m_brk));
	save_item(NAME(m_status_read));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acia6850_device::device_reset()
{
	int cts = devcb_call_read_line(&m_in_cts_func);
	int dcd = devcb_call_read_line(&m_in_dcd_func);

	m_status = (cts << 3) | (dcd << 2) | ACIA6850_STATUS_TDRE;
	m_tdr = 0;
	m_rdr = 0;
	m_tx_shift = 0;
	m_rx_shift = 0;
	m_tx_counter = 0;
	m_rx_counter = 0;

	TXD(1);
	m_overrun = 0;
	m_status_read = 0;
	m_brk = 0;

	m_rx_state = START;
	m_tx_state = START;
	m_irq = 0;

	devcb_call_write_line(&m_out_irq_func, 1);

	if (m_first_reset)
	{
		m_first_reset = 0;

		RTS(1);
	}
	else
	{
		RTS(m_rts);
	}
}



/*-------------------------------------------------
    acia6850_stat_r - Read Status Register
-------------------------------------------------*/

READ8_DEVICE_HANDLER_TRAMPOLINE(acia6850, acia6850_stat_r)
{
	UINT8 status;

	m_status_read = 1;
	status = m_status;

	if (status & ACIA6850_STATUS_CTS)
	{
		status &= ~ACIA6850_STATUS_TDRE;
	}

	return status;
}


/*-------------------------------------------------
    acia6850_ctrl_w - Write Control Register
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER_TRAMPOLINE(acia6850, acia6850_ctrl_w )
{
	if (LOG) logerror("MC6850 '%s' Control: %02x\n", tag(), data);

	// Counter Divide Select Bits

	int divide = data & CR1_0;

	if (divide == 3)
	{
		m_reset = 1;
		device_reset();
	}
	else
	{
		m_reset = 0;
		m_divide = ACIA6850_DIVIDE[divide];
	}

	// Word Select Bits

	int wordsel = (data & CR4_2) >> 2;

	m_bits = ACIA6850_WORD[wordsel][0];
	m_parity = (parity_type)ACIA6850_WORD[wordsel][1];
	m_stopbits = ACIA6850_WORD[wordsel][2];

	// Transmitter Control Bits

	switch ((data & CR6_5) >> 5)
	{
	case 0:
		m_rts = 0;
		RTS(m_rts);

		m_tx_int = 0;
		m_brk = 0;
		break;

	case 1:
		m_rts = 0;
		RTS(m_rts);

		m_tx_int = 1;
		m_brk = 0;
		break;

	case 2:
		m_rts = 1;
		RTS(m_rts);

		m_tx_int = 0;
		m_brk = 0;
		break;

	case 3:
		m_rts = 0;
		RTS(m_rts);

		m_tx_int = 0;
		m_brk = 1;
		break;
	}

	check_interrupts();

	// After writing the word type, set the rx/tx clocks (provided the divide values have changed)

	if ((m_ctrl ^ data) & CR1_0)
	{
		if (!m_reset)
		{
			if (m_rx_clock)
			{
				attotime rx_period = attotime::from_hz(m_rx_clock) *  m_divide;
				m_rx_timer->adjust(rx_period, 0, rx_period);
			}

			if (m_tx_clock)
			{
				attotime tx_period = attotime::from_hz(m_tx_clock) * m_divide;
				m_tx_timer->adjust(tx_period, 0, tx_period);
			}
		}
	}
	m_ctrl = data;
}


/*-------------------------------------------------
    check_interrupts
-------------------------------------------------*/

void acia6850_device::check_interrupts()
{
	int irq = (m_tx_int && (m_status & ACIA6850_STATUS_TDRE) && (~m_status & ACIA6850_STATUS_CTS)) ||
		((m_ctrl & 0x80) && ((m_status & (ACIA6850_STATUS_RDRF|ACIA6850_STATUS_DCD)) || m_overrun));

	if (irq != m_irq)
	{
		m_irq = irq;

		if (irq)
		{
			m_status |= ACIA6850_STATUS_IRQ;
			devcb_call_write_line(&m_out_irq_func, 0);
		}
		else
		{
			m_status &= ~ACIA6850_STATUS_IRQ;
			devcb_call_write_line(&m_out_irq_func, 1);
		}
	}
}


/*-------------------------------------------------
    acia6850_data_w - Write transmit register
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER_TRAMPOLINE(acia6850, acia6850_data_w)
{
	if (LOG) logerror("MC6850 '%s' Data: %02x\n", tag(), data);

	if (!m_reset)
	{
		m_tdr = data;
		m_status &= ~ACIA6850_STATUS_TDRE;
		check_interrupts();
	}
	else
	{
		logerror("%s:ACIA %p: Data write while in reset!\n", m_machine.describe_context(), this);
	}
}


/*-------------------------------------------------
    acia6850_data_r - Read character
-------------------------------------------------*/

READ8_DEVICE_HANDLER_TRAMPOLINE(acia6850, acia6850_data_r)
{
	m_status &= ~(ACIA6850_STATUS_RDRF | ACIA6850_STATUS_IRQ | ACIA6850_STATUS_PE);

	if (m_status_read)
	{
		int dcd = devcb_call_read_line(&m_in_dcd_func);

		m_status_read = 0;
		m_status &= ~(ACIA6850_STATUS_OVRN | ACIA6850_STATUS_DCD);

		if (dcd)
		{
			m_status |= ACIA6850_STATUS_DCD;
		}
	}

	if (m_overrun == 1)
	{
		m_status |= ACIA6850_STATUS_OVRN;
		m_overrun = 0;
	}

	check_interrupts();

	return m_rdr;
}


/*-------------------------------------------------
    tx_tick - Transmit a bit
-------------------------------------------------*/

void acia6850_device::tx_tick()
{
	switch (m_tx_state)
	{
		case START:
		{
			if (m_brk)
			{
				// transmit break

				TXD(0);
			}
			else
			{
				int _cts = devcb_call_read_line(&m_in_cts_func);

				if (_cts)
				{
					m_status |= ACIA6850_STATUS_CTS;
				}
				else
				{
					m_status &= ~ACIA6850_STATUS_CTS;
				}

				check_interrupts();

				if (m_status & ACIA6850_STATUS_TDRE)
				{
					// transmitter idle
					TXD(1);
				}
				else
				{
					// transmit character

					if (LOG) logerror("MC6850 '%s': TX DATA %x\n", tag(), m_tdr);
					if (LOG) logerror("MC6850 '%s': TX START BIT\n", tag());

					TXD(0);

					m_tx_bits = m_bits;
					m_tx_shift = m_tdr;
					m_tx_parity = 0;
					m_tx_state = DATA;
				}
			}
			break;
		}
		case DATA:
		{
			int val = m_tx_shift & 1;
			if (LOG) logerror("MC6850 '%s': TX DATA BIT %x\n", tag(), val);

			TXD(val);
			m_tx_parity ^= val;
			m_tx_shift >>= 1;

			if (--(m_tx_bits) == 0)
			{
				m_tx_state = (m_parity == NONE) ? STOP : PARITY;
			}

			break;
		}
		case PARITY:
		{
			int parity = 0;

			if (m_parity == EVEN)
			{
				parity = (m_tx_parity & 1) ? 1 : 0;
			}
			else
			{
				parity = (m_tx_parity & 1) ? 0 : 1;
			}

			TXD(parity);

			if (LOG) logerror("MC6850 '%s': TX PARITY BIT %x\n", tag(), parity);
			m_tx_state = STOP;
			break;
		}
		case STOP:
		{
			if (LOG) logerror("MC6850 '%s': TX STOP BIT\n", tag());
			TXD(1);

			if (m_stopbits == 1)
			{
				m_tx_state = START;
				m_status |= ACIA6850_STATUS_TDRE;
			}
			else
			{
				m_tx_state = STOP2;
			}
			break;
		}
		case STOP2:
		{
			if (LOG) logerror("MC6850 '%s': TX STOP BIT\n", tag());
			TXD(1);
			m_tx_state = START;
			m_status |= ACIA6850_STATUS_TDRE;
			break;
		}
	}
}


/*-------------------------------------------------
    transmit_event
-------------------------------------------------*/

TIMER_CALLBACK( acia6850_device::transmit_event_callback ) { reinterpret_cast<acia6850_device *>(ptr)->transmit_event(); }

void acia6850_device::transmit_event()
{
	tx_tick();
	m_tx_counter = 0;
}


/*-------------------------------------------------
    tx_clock_in - As above, but using the tx pin
-------------------------------------------------*/

void acia6850_tx_clock_in(device_t *device) { downcast<acia6850_device*>(device)->tx_clock_in(); }

void acia6850_device::tx_clock_in()
{
	int _cts = devcb_call_read_line(&m_in_cts_func);

	if (_cts)
	{
		m_status |= ACIA6850_STATUS_CTS;
	}
	else
	{
		m_status &= ~ACIA6850_STATUS_CTS;
	}

	m_tx_counter ++;

	if ( m_tx_counter > m_divide - 1)
	{
		tx_tick();
		m_tx_counter = 0;
	}

}


/*-------------------------------------------------
    rx_tick - Receive a bit
-------------------------------------------------*/

void acia6850_device::rx_tick()
{
	int dcd = devcb_call_read_line(&m_in_dcd_func);

	if (dcd)
	{
		m_status |= ACIA6850_STATUS_DCD;
		check_interrupts();
	}
	else if ((m_status & (ACIA6850_STATUS_DCD | ACIA6850_STATUS_IRQ)) == ACIA6850_STATUS_DCD)
	{
		m_status &= ~ACIA6850_STATUS_DCD;
	}

	if (m_status & ACIA6850_STATUS_DCD)
	{
		m_rx_state = START;
	}
	else
	{
		int rxd = devcb_call_read_line(&m_in_rx_func);

		switch (m_rx_state)
		{
			case START:
			{
				if (rxd == 0)
				{
					if (LOG) logerror("MC6850 '%s': RX START BIT\n", tag());
					m_rx_shift = 0;
					m_rx_parity = 0;
					m_rx_bits = m_bits;
					m_rx_state = DATA;
				}
				break;
			}
			case DATA:
			{
				if (LOG) logerror("MC6850 '%s': RX DATA BIT %x\n", tag(), rxd);
				m_rx_shift |= rxd ? 0x80 : 0;
				m_rx_parity ^= rxd;

				if (--m_rx_bits == 0)
				{
					if (m_status & ACIA6850_STATUS_RDRF)
					{
						m_overrun = 1;
						check_interrupts();
					}

					m_rx_state = m_parity == NONE ? STOP : PARITY;
				}
				else
				{
					m_rx_shift >>= 1;
				}
				break;
			}
			case PARITY:
			{
				if (LOG) logerror("MC6850 '%s': RX PARITY BIT %x\n", tag(), rxd);
				m_rx_parity ^= rxd;

				if (m_parity == EVEN)
				{
					if (m_rx_parity)
					{
						m_status |= ACIA6850_STATUS_PE;
					}
				}
				else
				{
					if (!m_rx_parity)
					{
						m_status |= ACIA6850_STATUS_PE;
					}
				}

				m_rx_state = STOP;
				break;
			}
			case STOP:
			{
				if (rxd == 1)
				{
					if (LOG) logerror("MC6850 '%s': RX STOP BIT\n", tag());
					if (m_stopbits == 1)
					{
						m_status &= ~ACIA6850_STATUS_FE;

						if (!(m_status & ACIA6850_STATUS_RDRF))
						{
							if (LOG) logerror("MC6850 '%s': RX DATA %x\n", tag(), m_rx_shift);
							m_rdr = m_rx_shift;
							m_status |= ACIA6850_STATUS_RDRF;
							check_interrupts();
						}

						m_rx_state = START;
					}
					else
					{
						m_rx_state = STOP2;
					}
				}
				else
				{
					m_status |= ACIA6850_STATUS_FE;
					m_rx_state = START;
				}
				break;
			}
			case STOP2:
			{
				if (rxd == 1)
				{
					if (LOG) logerror("MC6850 '%s': RX STOP BIT\n", tag());
					m_status &= ~ACIA6850_STATUS_FE;

					if (!(m_status & ACIA6850_STATUS_RDRF))
					{
						if (LOG) logerror("MC6850 '%s': RX DATA %x\n", tag(), m_rx_shift);
						m_rdr = m_rx_shift;
						m_status |= ACIA6850_STATUS_RDRF;
						check_interrupts();
					}

					m_rx_state = START;
				}
				else
				{
					m_status |= ACIA6850_STATUS_FE;
					m_rx_state = START;
				}
				break;
			}
		}
	}
}


/*-------------------------------------------------
    TIMER_CALLBACK( receive_event_callback ) -
    Called on receive timer event
-------------------------------------------------*/

TIMER_CALLBACK( acia6850_device::receive_event_callback ) { reinterpret_cast<acia6850_device *>(ptr)->receive_event(); }

void acia6850_device::receive_event()
{
	rx_tick();
	m_rx_counter = 0;
}


/*-------------------------------------------------
    rx_clock_in - As above, but using the rx pin
-------------------------------------------------*/

void acia6850_rx_clock_in(device_t *device) { downcast<acia6850_device*>(device)->rx_clock_in(); }

void acia6850_device::rx_clock_in()
{
	int dcd = devcb_call_read_line(&m_in_dcd_func);

	if (dcd)
	{
		m_status |= ACIA6850_STATUS_DCD;
		check_interrupts();
	}
	else if ((m_status & (ACIA6850_STATUS_DCD|ACIA6850_STATUS_IRQ)) == ACIA6850_STATUS_DCD)
	{
		m_status &= ~ACIA6850_STATUS_DCD;
	}

	m_rx_counter ++;

	if ( m_rx_counter > m_divide - 1)
	{
		rx_tick();
		m_rx_counter = 0;
	}
}


/*-------------------------------------------------
    set_rx_clock - set receiver clock
-------------------------------------------------*/

void acia6850_device::set_rx_clock(int clock)
{
	m_rx_clock = clock;

	if (m_rx_clock)
	{
		attotime rx_period = attotime::from_hz(m_rx_clock) * m_divide;
		m_rx_timer->adjust(rx_period, 0, rx_period);
	}
}


/*-------------------------------------------------
    acia6850_set_rx_clock - Set clock frequencies
    dynamically
-------------------------------------------------*/

void acia6850_set_rx_clock(device_t *device, int clock)
{
	downcast<acia6850_device*>(device)->set_rx_clock(clock);
}


/*-------------------------------------------------
    set_tx_clock - set receiver clock
-------------------------------------------------*/

void acia6850_device::set_tx_clock(int clock)
{
	m_tx_clock = clock;

	if (m_tx_clock)
	{
		attotime tx_period = attotime::from_hz(m_tx_clock) * m_divide;
		m_tx_timer->adjust(tx_period, 0, tx_period);
	}
}


/*-------------------------------------------------
    acia6850_set_tx_clock - Set clock frequencies
    dynamically
-------------------------------------------------*/

void acia6850_set_tx_clock(device_t *device, int clock)
{
	downcast<acia6850_device*>(device)->set_tx_clock(clock);
}


/*-------------------------------------------------
    receive_data - receive data byte
-------------------------------------------------*/

void acia6850_device::receive_data(UINT8 data)
{
	m_rdr = data;
	m_status |= ACIA6850_STATUS_RDRF;
	check_interrupts();
}


/*-------------------------------------------------
    acia6850_receive_data - Receive data byte
-------------------------------------------------*/

void acia6850_receive_data(device_t *device, UINT8 data)
{
	downcast<acia6850_device*>(device)->receive_data(data);
}
