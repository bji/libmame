/**********************************************************************

    MOS 6526/8520 Complex Interface Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 40  CNT
                   PA0   2 |             | 39  SP
                   PA1   3 |             | 38  RS0
                   PA2   4 |             | 37  RS1
                   PA3   5 |             | 36  RS2
                   PA4   6 |             | 35  RS3
                   PA5   7 |             | 34  _RES
                   PA6   8 |             | 33  DB0
                   PA7   9 |             | 32  DB1
                   PB0  10 |   MOS6526   | 31  DB2
                   PB1  11 |   MOS8520   | 30  DB3
                   PB2  12 |             | 29  DB4
                   PB3  13 |             | 28  DB5
                   PB4  14 |             | 27  DB6
                   PB5  15 |             | 26  DB7
                   PB6  16 |             | 25  phi2
                   PB7  17 |             | 24  _FLAG
                   _PC  18 |             | 23  _CS
                   TOD  19 |             | 22  R/W
                   Vcc  20 |_____________| 21  _IRQ

**********************************************************************/

#pragma once

#ifndef __6526CIA_H__
#define __6526CIA_H__

#include "emu.h"



	//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_MOS6526R1_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, MOS6526R1, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_MOS6526R2_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, MOS6526R2, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_MOS8520_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, MOS8520, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define MOS6526_INTERFACE(name) \
	const mos6526_interface (name)=

#define MOS8520_INTERFACE(name) \
	const mos6526_interface (name)=

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> mos6526_interface

struct mos6526_interface
{
	int m_tod_clock;

	devcb_write_line	m_out_irq_func;
	devcb_write_line	m_out_pc_func;
	devcb_write_line	m_out_cnt_func;
	devcb_write_line	m_out_sp_func;

	devcb_read8			m_in_pa_func;
	devcb_write8		m_out_pa_func;

	devcb_read8			m_in_pb_func;
	devcb_write8		m_out_pb_func;
};



// ======================> mos6526_device_config

class mos6526_device_config :   public device_config,
                                public mos6526_interface
{
    friend class mos6526_device;

    // construction/destruction
    mos6526_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
    // device_config overrides
    virtual void device_config_complete();
};



// ======================> mos6526_device

class mos6526_device :  public device_t
{
    friend class mos6526_device_config;
    friend class dart_channel;

    // construction/destruction
    mos6526_device(running_machine &_machine, const mos6526_device_config &_config);

public:
	UINT8 reg_r(UINT8 offset);
	void reg_w(UINT8 offset, UINT8 data);

	/* port access */
	UINT8 pa_r(UINT8 offset) { return (m_port[0].m_latch | ~m_port[0].m_ddr); }
	UINT8 pb_r(UINT8 offset) { return (m_port[1].m_latch | ~m_port[1].m_ddr); }

	/* interrupt request */
	UINT8 irq_r() { return m_irq; }

	/* time of day clock */
	void tod_w(UINT8 state) { if(state) clock_tod(); }

	/* serial counter */
	UINT8 cnt_r() { return m_cnt; }
	void cnt_w(UINT8 state);

	/* serial port */
	UINT8 sp_r() { return m_sp; }
	void sp_w(UINT8 state) { m_sp = state; }

	/* flag */
	void flag_w(UINT8 state);

	/* port mask */
	void set_port_mask_value(int port, int data);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

	static TIMER_CALLBACK( timer_proc );
	static TIMER_CALLBACK( clock_tod_callback );

private:

	void update_pc();
	void update_interrupts();
	void timer_bump(int timer);
	void timer_underflow(int timer);
	void increment();
	void clock_tod();

	struct cia_timer
	{
	public:
		cia_timer() { }

		UINT16 get_count();
		void update(int which, INT32 new_count);

		UINT32			m_clock;
		UINT16			m_latch;
		UINT16			m_count;
		UINT8			m_mode;
		UINT8			m_irq;
		emu_timer*		m_timer;
		mos6526_device*	m_cia;
	};

	struct cia_port
	{
		UINT8		m_ddr;
		UINT8		m_latch;
		UINT8		m_in;
		UINT8		m_out;
		devcb_resolved_read8	m_read;
		devcb_resolved_write8	m_write;
		UINT8		m_mask_value; /* in READ operation the value can be forced by a extern electric circuit */
	};

	devcb_resolved_write_line m_out_irq_func;
	devcb_resolved_write_line m_out_pc_func;
	devcb_resolved_write_line m_out_cnt_func;
	devcb_resolved_write_line m_out_sp_func;

	cia_port		m_port[2];
	cia_timer		m_timer[2];

	/* Time Of the Day clock (TOD) */
	UINT32			m_tod;
	UINT32			m_tod_latch;
	UINT8			m_tod_latched;
	UINT8			m_tod_running;
	UINT32			m_alarm;

	/* Interrupts */
	UINT8			m_icr;
	UINT8			m_ics;
	UINT8			m_irq;
	int				m_flag;

	/* Serial */
	UINT8			m_loaded;
	UINT8			m_sdr;
	UINT8			m_sp;
	UINT8			m_cnt;
	UINT8			m_shift;
	UINT8			m_serial;

    const mos6526_device_config &m_config;
};


// device type definition
extern const device_type MOS6526R1;
extern const device_type MOS6526R2;
extern const device_type MOS8520;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* register access */
READ8_DEVICE_HANDLER( mos6526_r );
WRITE8_DEVICE_HANDLER( mos6526_w );

/* port access */
READ8_DEVICE_HANDLER( mos6526_pa_r );
READ8_DEVICE_HANDLER( mos6526_pb_r );

/* interrupt request */
READ_LINE_DEVICE_HANDLER( mos6526_irq_r );

/* time of day clock */
WRITE_LINE_DEVICE_HANDLER( mos6526_tod_w );

/* serial counter */
READ_LINE_DEVICE_HANDLER( mos6526_cnt_r );
WRITE_LINE_DEVICE_HANDLER( mos6526_cnt_w );

/* serial port */
READ_LINE_DEVICE_HANDLER( mos6526_sp_r );
WRITE_LINE_DEVICE_HANDLER( mos6526_sp_w );

/* flag */
WRITE_LINE_DEVICE_HANDLER( mos6526_flag_w );

/* port mask */
void cia_set_port_mask_value(running_device *device, int port, int data);

#endif /* __6526CIA_H__ */
