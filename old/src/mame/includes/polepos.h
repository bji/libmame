/*************************************************************************

    Pole Position hardware

*************************************************************************/

#include "devlegcy.h"
#include "sound/discrete.h"


class polepos_state : public driver_device
{
public:
	polepos_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 steer_last;
	UINT8 steer_delta;
	INT16 steer_accum;
	INT16 last_result;
	INT8 last_signed;
	UINT8 last_unsigned;
	int adc_input;
	int auto_start_mask;
	UINT16 *view16_memory;
	UINT16 *road16_memory;
	UINT16 *alpha16_memory;
	UINT16 *sprite16_memory;
	UINT16 vertical_position_modifier[256];
	UINT16 road16_vscroll;
	tilemap_t *bg_tilemap;
	tilemap_t *tx_tilemap;
	int chacl;
	UINT16 scroll;
};


/*----------- defined in audio/polepos.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(POLEPOS, polepos_sound);

WRITE8_DEVICE_HANDLER( polepos_engine_sound_lsb_w );
WRITE8_DEVICE_HANDLER( polepos_engine_sound_msb_w );

DISCRETE_SOUND_EXTERN( polepos );


/*----------- defined in video/polepos.c -----------*/

VIDEO_START( polepos );
PALETTE_INIT( polepos );
SCREEN_UPDATE( polepos );

WRITE16_HANDLER( polepos_view16_w );
WRITE16_HANDLER( polepos_road16_w );
WRITE16_HANDLER( polepos_alpha16_w );
WRITE16_HANDLER( polepos_sprite16_w );
WRITE8_HANDLER( polepos_view_w );
WRITE8_HANDLER( polepos_road_w );
WRITE8_HANDLER( polepos_alpha_w );
WRITE8_HANDLER( polepos_sprite_w );
WRITE8_HANDLER( polepos_chacl_w );

READ16_HANDLER( polepos_view16_r );
READ16_HANDLER( polepos_road16_r );
READ16_HANDLER( polepos_alpha16_r );
READ16_HANDLER( polepos_sprite16_r );
READ8_HANDLER( polepos_view_r );
READ8_HANDLER( polepos_road_r );
READ8_HANDLER( polepos_alpha_r );
READ8_HANDLER( polepos_sprite_r );
WRITE16_HANDLER( polepos_view16_hscroll_w );
WRITE16_HANDLER( polepos_road16_vscroll_w );
