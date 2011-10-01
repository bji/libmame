/**********************************************************************

    Motorola MCCS1850 Serial Real-Time Clock emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                  Vbat   1 |*    \_/     | 16  Vdd
                  _POR   2 |             | 15  TEST
                  _INT   3 |             | 14  XTAL1
                   SCK   4 |   MCCS1850  | 13  XTAL2
                   SDI   5 |             | 12  _PWRSW
                   SDO   6 |             | 11  NUC
                    CE   7 |             | 10  _PSE
                   Vss   8 |_____________| 9   PSE

**********************************************************************/

#pragma once

#ifndef __MCCS1850__
#define __MCCS1850__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MCCS1850_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, MCCS1850, _clock)	\
	MCFG_DEVICE_CONFIG(_config)


#define MCCS1850_INTERFACE(name) \
	const mccs1850_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mccs1850_interface

struct mccs1850_interface
{
	devcb_write_line		m_out_int_cb;
	devcb_write_line		m_out_pse_cb;
	devcb_write_line		m_out_nuc_cb;
};



// ======================> mccs1850_device

class mccs1850_device :	public device_t,
						public device_rtc_interface,
						public device_nvram_interface,
                        public mccs1850_interface
{
public:
    // construction/destruction
    mccs1850_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( ce_w );
	DECLARE_WRITE_LINE_MEMBER( sck_w );
	DECLARE_READ_LINE_MEMBER( sdo_r );
	DECLARE_WRITE_LINE_MEMBER( sdi_w );
	DECLARE_WRITE_LINE_MEMBER( pwrsw_w );
	DECLARE_WRITE_LINE_MEMBER( por_w );
	DECLARE_WRITE_LINE_MEMBER( test_w );

protected:
    // device-level overrides
	virtual void device_config_complete();
    virtual void device_start();
    virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_rtc_interface overrides
	virtual void rtc_set_time(int year, int month, int day, int day_of_week, int hour, int minute, int second);
	virtual bool rtc_is_year_2000_compliant() { return false; }

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

private:
	inline void check_interrupt();
	inline void set_pse_line(int state);
	inline UINT8 read_register(offs_t offset);
	inline void write_register(offs_t offset, UINT8 data);
	inline void advance_seconds();

	static const device_timer_id TIMER_CLOCK = 0;

	devcb_resolved_write_line	m_out_int_func;
	devcb_resolved_write_line	m_out_pse_func;
	devcb_resolved_write_line	m_out_nuc_func;

	UINT8 m_ram[0x80];			// RAM

	// power supply
	int m_pse;					// power supply enable

	// counter
	UINT32 m_counter;			// seconds counter

	// serial interface
	int m_ce;					// chip enable
	int m_sck;					// serial clock
	int m_sdo;					// serial data out
	int m_sdi;					// serial data in
	int m_state;				// serial interface state
	UINT8 m_address;			// address counter
	int m_bits;					// bit counter
	UINT8 m_shift;				// shift register

	// timers
	emu_timer *m_clock_timer;
};


// device type definition
extern const device_type MCCS1850;



#endif
