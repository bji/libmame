/******************************************************************************


    CD-i Mono-I CDIC MCU simulation
    -------------------

    MESS implementation by Harmony


*******************************************************************************

STATUS:

- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:

- Decapping and proper emulation.

*******************************************************************************/

#pragma once

#ifndef __CDICDIC_H__
#define __CDICDIC_H__

#include "emu.h"
#include "cdrom.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDICDIC_ADD(_tag) \
    MCFG_DEVICE_ADD(_tag, MACHINE_CDICDIC, 0) \

#define MCFG_CDICDIC_REPLACE(_tag) \
    MCFG_DEVICE_REPLACE(_tag, MACHINE_CDICDIC, 0) \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdicdic_device_config

class cdicdic_device_config :  public device_config
{
    friend class cdicdic_device;

    // construction/destruction
    cdicdic_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

    // inline configuration indexes go here (none yet)

protected:
    // device_config overrides (none yet)

    // internal state goes here (none yet)
};



// ======================> cdicdic_device

class cdicdic_device : public device_t
{
    friend class cdicdic_device_config;

    // construction/destruction
    cdicdic_device(running_machine &_machine, const cdicdic_device_config &config);

public:
    // non-static internal members
    void sample_trigger();
    void process_delayed_command();
    void ram_write(const UINT32 offset, const UINT16 data, const UINT16 mem_mask);
    UINT16 ram_read(const UINT32 offset, const UINT16 mem_mask);
    void register_write(const UINT32 offset, const UINT16 data, const UINT16 mem_mask);
    UINT16 register_read(const UINT32 offset, const UINT16 mem_mask);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

    // internal callbacks
    static TIMER_CALLBACK( audio_sample_trigger );
    static TIMER_CALLBACK( trigger_readback_int );

    // internal state
    const cdicdic_device_config &m_config;

private:
    UINT16 m_command;           // CDIC Command Register            (0x303c00)
    UINT32 m_time;              // CDIC Time Register               (0x303c02)
    UINT16 m_file;              // CDIC File Register               (0x303c06)
    UINT32 m_channel;           // CDIC Channel Register            (0x303c08)
    UINT16 m_audio_channel;     // CDIC Audio Channel Register      (0x303c0c)

    UINT16 m_audio_buffer;      // CDIC Audio Buffer Register       (0x303ff4)
    UINT16 m_x_buffer;          // CDIC X-Buffer Register           (0x303ff6)
    UINT16 m_dma_control;       // CDIC DMA Control Register        (0x303ff8)
    UINT16 m_z_buffer;          // CDIC Z-Buffer Register           (0x303ffa)
    UINT16 m_interrupt_vector;  // CDIC Interrupt Vector Register   (0x303ffc)
    UINT16 m_data_buffer;       // CDIC Data Buffer Register        (0x303ffe)

    emu_timer *m_interrupt_timer;
    cdrom_file *m_cd;

    emu_timer *m_audio_sample_timer;
    INT32 m_audio_sample_freq;
    INT32 m_audio_sample_size;

    UINT16 m_decode_addr;
    UINT8 m_decode_delay;
    attotime m_decode_period;

    int m_xa_last[4];
    UINT16 *m_ram;

    void register_globals();
    void init();

    // static internal members
    static void decode_xa_mono(INT32 *cdic_xa_last, const UINT8 *xa, INT16 *dp);
    static void decode_xa_mono8(INT32 *cdic_xa_last, const UINT8 *xa, INT16 *dp);
    static void decode_xa_stereo(INT32 *cdic_xa_last, const UINT8 *xa, INT16 *dp);
    static void decode_xa_stereo8(INT32 *cdic_xa_last, const UINT8 *xa, INT16 *dp);

    static const INT32 s_cdic_adpcm_filter_coef[5][2];

    // non-static internal members
    UINT32 increment_cdda_frame_bcd(UINT32 bcd);
    UINT32 increment_cdda_sector_bcd(UINT32 bcd);
    void decode_audio_sector(const UINT8 *xa, INT32 triggered);
};


// device type definition
extern const device_type MACHINE_CDICDIC;



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ16_DEVICE_HANDLER( cdic_r );
WRITE16_DEVICE_HANDLER( cdic_w );
READ16_DEVICE_HANDLER( cdic_ram_r );
WRITE16_DEVICE_HANDLER( cdic_ram_w );


#endif // __CDICDIC_H__
