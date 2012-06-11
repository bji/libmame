/***************************************************************************

    Fujistu MB3773

    Power Supply Monitor with Watch Dog Timer (i.e. Reset IC)

***************************************************************************/

#pragma once

#ifndef __MB3773_H__
#define __MB3773_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MB3773_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MB3773, 0)


// ======================> mb3773_device

class mb3773_device :
	public device_t
{
public:
	// construction/destruction
	mb3773_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock );

	// I/O operations
	void set_ck( int state );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();

	// internal helpers
	static TIMER_CALLBACK( watchdog_timeout );
	void reset_timer();

	// internal state
	emu_timer *m_watchdog_timer;
	int m_ck;
};


// device type definition
extern const device_type MB3773;


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_DEVICE_HANDLER( mb3773_set_ck );

#endif
