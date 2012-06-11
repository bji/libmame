#pragma once

#ifndef __DSBZ80_H__
#define __DSBZ80_H__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ymz770.h"

#define DSBZ80_TAG "dsbz80"

#define MCFG_DSBZ80_ADD(_tag) \
    MCFG_DEVICE_ADD(_tag, DSBZ80, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dsbz80_device : public device_t
{
public:
    // construction/destruction
    dsbz80_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    // optional information overrides
    virtual machine_config_constructor device_mconfig_additions() const;

    DECLARE_WRITE8_MEMBER(latch_w);

    required_device<cpu_device> m_ourcpu;
    optional_device<ymz770_device> m_ymz770;

    DECLARE_WRITE8_MEMBER(mpeg_trigger_w);
    DECLARE_WRITE8_MEMBER(mpeg_start_w);
    DECLARE_WRITE8_MEMBER(mpeg_end_w);
    DECLARE_WRITE8_MEMBER(mpeg_volume_w);
    DECLARE_WRITE8_MEMBER(mpeg_stereo_w);
    DECLARE_READ8_MEMBER(mpeg_pos_r);
    DECLARE_READ8_MEMBER(latch_r);
    DECLARE_READ8_MEMBER(status_r);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();

private:
    UINT8 m_dsb_latch;
    UINT32 mp_start, mp_end, mp_vol, mp_pan, mp_state, lp_start, lp_end, start, end;
    int status;
};


// device type definition
extern const device_type DSBZ80;

#endif  /* __DSBZ80_H__ */
