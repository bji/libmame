/***************************************************************************

    i8243.h

    Intel 8243 Port Expander

    Copyright Aaron Giles

***************************************************************************/

#pragma once

#ifndef __I8243_H__
#define __I8243_H__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_I8243_ADD(_tag, _read, _write) \
	MCFG_DEVICE_ADD(_tag, I8243, 0) \
	MCFG_I8243_READHANDLER(_read) \
	MCFG_I8243_WRITEHANDLER(_write) \

#define MCFG_I8243_READHANDLER(_read) \
	i8243_device_config::static_set_read_handler(device, _read); \

#define MCFG_I8243_WRITEHANDLER(_write) \
	i8243_device_config::static_set_write_handler(device, _write); \

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> i8243_device_config

class i8243_device_config : public device_config
{
    friend class i8243_device;

    // construction/destruction
    i8243_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

	// inline configuration helpers
	static void static_set_read_handler(device_config *device, read8_device_func callback);
	static void static_set_write_handler(device_config *device, write8_device_func callback);

protected:
	devcb_read8		m_readhandler;
	devcb_write8	m_writehandler;
};


// ======================> i8243_device

class i8243_device :  public device_t
{
    friend class i8243_device_config;

    // construction/destruction
    i8243_device(running_machine &_machine, const i8243_device_config &_config);

public:

	UINT8 i8243_p2_r(UINT32 offset);
	void i8243_p2_w(UINT32 offset, UINT8 data);

	void i8243_prog_w(UINT32 offset, UINT8 data);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

private:

	UINT8		m_p[4];				/* 4 ports' worth of data */
	UINT8		m_p2out;			/* port 2 bits that will be returned */
	UINT8		m_p2;				/* most recent port 2 value */
	UINT8		m_opcode;			/* latched opcode */
	UINT8		m_prog;				/* previous PROG state */

	devcb_resolved_read8	m_readhandler;
	devcb_resolved_write8	m_writehandler;

    const i8243_device_config &m_config;
};


// device type definition
extern const device_type I8243;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER( i8243_p2_r );
WRITE8_DEVICE_HANDLER( i8243_p2_w );

WRITE8_DEVICE_HANDLER( i8243_prog_w );


#endif  /* __I8243_H__ */
