/******************************************************************************


    CD-i Mono-I SLAVE MCU simulation
    -------------------

    MESS implementation by Harmony


*******************************************************************************

STATUS:

- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:

- Decapping and proper emulation.

*******************************************************************************/

#pragma once

#ifndef __CDISLAVE_H__
#define __CDISLAVE_H__

#include "emu.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDISLAVE_ADD(_tag) \
    MCFG_DEVICE_ADD(_tag, MACHINE_CDISLAVE, 0) \

#define MCFG_CDISLAVE_REPLACE(_tag) \
    MCFG_DEVICE_REPLACE(_tag, MACHINE_CDISLAVE, 0) \

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdislave_device

class cdislave_device : public device_t
{
public:
    // construction/destruction
    cdislave_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    // external callbacks
    static INPUT_CHANGED( mouse_update );

    UINT8* get_lcd_state() { return m_lcd_state; }
    void readback_trigger();
    void register_write(const UINT32 offset, const UINT16 data, const UINT16 mem_mask);
    UINT16 register_read(const UINT32 offset, const UINT16 mem_mask);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

    // internal callbacks
    static TIMER_CALLBACK( trigger_readback_int );

private:
    // internal state
    class channel_state
    {
    public:
        channel_state() { }

        UINT8 m_out_buf[4];
        UINT8 m_out_index;
        UINT8 m_out_count;
        UINT8 m_out_cmd;
    };

    channel_state m_channel[4];
    emu_timer *m_interrupt_timer;

    UINT8 m_in_buf[17];
    UINT8 m_in_index;
    UINT8 m_in_count;

    UINT8 m_polling_active;

    UINT8 m_xbus_interrupt_enable;

    UINT8 m_lcd_state[16];

    UINT16 m_real_mouse_x;
    UINT16 m_real_mouse_y;

    UINT16 m_fake_mouse_x;
    UINT16 m_fake_mouse_y;

    void register_globals();
    void init();

    // static internal members

    // non-static internal members
    void prepare_readback(attotime delay, UINT8 channel, UINT8 count, UINT8 data0, UINT8 data1, UINT8 data2, UINT8 data3, UINT8 cmd);
    void perform_mouse_update();
    void set_mouse_position();
};


// device type definition
extern const device_type MACHINE_CDISLAVE;



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ16_DEVICE_HANDLER( slave_r );
WRITE16_DEVICE_HANDLER( slave_w );


#endif // __CDISLAVE_H__
