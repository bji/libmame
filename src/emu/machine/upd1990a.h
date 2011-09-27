/**********************************************************************

    NEC uPD1990AC Serial I/O Calendar & Clock emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                    C2   1 |*    \_/     | 14  Vdd
                    C1   2 |             | 13  XTAL
                    C0   3 |             | 12  _XTAL
                   STB   4 |  uPD1990AC  | 11  OUT ENBL
                    CS   5 |             | 10  TP
               DATA IN   6 |             | 9   DATA OUT
                   GND   7 |_____________| 8   CLK

**********************************************************************/

#pragma once

#ifndef __UPD1990A__
#define __UPD1990A__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_UPD1990A_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD((_tag), UPD1990A, _clock)	\
	MCFG_DEVICE_CONFIG(_config)

#define UPD1990A_INTERFACE(name) \
	const upd1990a_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd1990a_interface

struct upd1990a_interface
{
	devcb_write_line		m_out_data_func;
	devcb_write_line		m_out_tp_func;
};



// ======================> upd1990a_device_config

class upd1990a_device_config :   public device_config,
                                public upd1990a_interface
{
    friend class upd1990a_device;

    // construction/destruction
    upd1990a_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
	// device_config overrides
	virtual void device_config_complete();
};



// ======================> upd1990a_device

class upd1990a_device :	public device_t
{
    friend class upd1990a_device_config;

    // construction/destruction
    upd1990a_device(running_machine &_machine, const upd1990a_device_config &_config);

public:
	DECLARE_WRITE_LINE_MEMBER( oe_w );
	DECLARE_WRITE_LINE_MEMBER( cs_w );
	DECLARE_WRITE_LINE_MEMBER( stb_w );
	DECLARE_WRITE_LINE_MEMBER( clk_w );
	DECLARE_WRITE_LINE_MEMBER( c0_w );
	DECLARE_WRITE_LINE_MEMBER( c1_w );
	DECLARE_WRITE_LINE_MEMBER( c2_w );
	DECLARE_WRITE_LINE_MEMBER( data_in_w );
	DECLARE_READ_LINE_MEMBER( data_out_r );
	DECLARE_READ_LINE_MEMBER( tp_r );

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	inline UINT8 convert_to_bcd(int val);
	inline int bcd_to_integer(UINT8 val);
	inline void advance_seconds();

	static const device_timer_id TIMER_CLOCK = 0;
	static const device_timer_id TIMER_TP = 1;
	static const device_timer_id TIMER_DATA_OUT = 2;

	devcb_resolved_write_line	m_out_data_func;
	devcb_resolved_write_line	m_out_tp_func;

	UINT8 m_time_counter[5];	// time counter
	UINT8 m_shift_reg[5];		// shift register

	int m_oe;					// output enable
	int m_cs;					// chip select
	int m_stb;					// strobe
	int m_data_in;				// data in
	int m_data_out;				// data out
	int m_c;					// command
	int m_clk;					// shift clock
	int m_tp;					// time pulse

	// timers
	emu_timer *m_timer_clock;
	emu_timer *m_timer_tp;
	emu_timer *m_timer_data_out;

	const upd1990a_device_config &m_config;
};


// device type definition
extern const device_type UPD1990A;



#endif
