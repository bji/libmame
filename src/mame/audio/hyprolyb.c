#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/msm5205.h"
#include "audio/hyprolyb.h"

typedef struct _hyprolyb_adpcm_state hyprolyb_adpcm_state;
struct _hyprolyb_adpcm_state
{
	device_t *m_msm;
	address_space *m_space;
	UINT8    m_adpcm_ready;	// only bootlegs
	UINT8    m_adpcm_busy;
	UINT8    m_vck_ready;
};

INLINE hyprolyb_adpcm_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == HYPROLYB_ADPCM);

	return (hyprolyb_adpcm_state *)downcast<legacy_device_base *>(device)->token();
}

static DEVICE_START( hyprolyb_adpcm )
{
	hyprolyb_adpcm_state *state = get_safe_token(device);

	state->m_space = device->machine().device("audiocpu")->memory().space(AS_PROGRAM);
	state->m_msm = device->machine().device("msm");
	device->save_item(NAME(state->m_adpcm_ready));	// only bootlegs
	device->save_item(NAME(state->m_adpcm_busy));
	device->save_item(NAME(state->m_vck_ready));
}


static DEVICE_RESET( hyprolyb_adpcm )
{
	hyprolyb_adpcm_state *state = get_safe_token(device);

	state->m_adpcm_ready = 0;
	state->m_adpcm_busy = 0;
	state->m_vck_ready = 0;
}

WRITE8_DEVICE_HANDLER( hyprolyb_adpcm_w )
{
	hyprolyb_adpcm_state *state = get_safe_token(device);

	soundlatch2_w(state->m_space, offset, data);
	state->m_adpcm_ready = 0x80;
}

READ8_DEVICE_HANDLER( hyprolyb_adpcm_busy_r )
{
	hyprolyb_adpcm_state *state = get_safe_token(device);

	return state->m_adpcm_busy ? 0x10 : 0x00;
}

static WRITE8_DEVICE_HANDLER( hyprolyb_msm_data_w )
{
	hyprolyb_adpcm_state *state = get_safe_token(device);

	msm5205_data_w(state->m_msm, data);
	state->m_adpcm_busy = ~data & 0x80;
}

static READ8_DEVICE_HANDLER( hyprolyb_msm_vck_r )
{
	hyprolyb_adpcm_state *state = get_safe_token(device);

	UINT8 old = state->m_vck_ready;
	state->m_vck_ready = 0x00;
	return old;
}

static READ8_DEVICE_HANDLER( hyprolyb_adpcm_ready_r )
{
	hyprolyb_adpcm_state *state = get_safe_token(device);

	return state->m_adpcm_ready;
}

static READ8_DEVICE_HANDLER( hyprolyb_adpcm_data_r )
{
	hyprolyb_adpcm_state *state = get_safe_token(device);

	state->m_adpcm_ready = 0x00;
	return soundlatch2_r(state->m_space, offset);
}

static ADDRESS_MAP_START( hyprolyb_adpcm_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x1000, 0x1000) AM_DEVREAD("hyprolyb_adpcm", hyprolyb_adpcm_data_r)
	AM_RANGE(0x1001, 0x1001) AM_DEVREAD("hyprolyb_adpcm", hyprolyb_adpcm_ready_r)
	AM_RANGE(0x1002, 0x1002) AM_DEVWRITE("hyprolyb_adpcm", hyprolyb_msm_data_w)
	AM_RANGE(0x1003, 0x1003) AM_DEVREAD("hyprolyb_adpcm", hyprolyb_msm_vck_r)
		// on init:
		//    $1003 = $00
		//    $1002 = $FF
		//    $1003 = $34
		//    $1001 = $36
		//    $1002 = $80
		// loops while ($1003) & 0x80 == 0
		// 1002 = ADPCM data written (low 4 bits)
		//
		// $1003 & $80 (in) = 5205 DRQ
		// $1002 & $0f (out) = 5205 data
		// $1001 & $80 (in) = sound latch request
		// $1000 (in) = sound latch data
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static void adpcm_vck_callback( device_t *device )
{
	device_t *adpcm = device->machine().device("hyprolyb_adpcm");
	hyprolyb_adpcm_state *state = get_safe_token(adpcm);

	state->m_vck_ready = 0x80;
}

static const msm5205_interface hyprolyb_msm5205_config =
{
	adpcm_vck_callback,	/* VCK function */
	MSM5205_S96_4B		/* 4 kHz */
};

MACHINE_CONFIG_FRAGMENT( hyprolyb_adpcm )
	MCFG_CPU_ADD("adpcm", M6802, XTAL_14_31818MHz/8)	/* unknown clock */
	MCFG_CPU_PROGRAM_MAP(hyprolyb_adpcm_map)

	MCFG_SOUND_ADD("hyprolyb_adpcm", HYPROLYB_ADPCM, 0)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_SOUND_CONFIG(hyprolyb_msm5205_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/*****************************************************************************
    DEVICE DEFINITION
*****************************************************************************/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##hyprolyb_adpcm##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME				"Hyper Olympics Audio"
#define DEVTEMPLATE_FAMILY				"Hyper Olympics Audio IC"
#include "devtempl.h"

DEFINE_LEGACY_SOUND_DEVICE(HYPROLYB_ADPCM, hyprolyb_adpcm);
