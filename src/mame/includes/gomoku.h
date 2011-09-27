#include "devlegcy.h"

class gomoku_state : public driver_device
{
public:
	gomoku_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_bgram;
	int m_flipscreen;
	int m_bg_dispsw;
	tilemap_t *m_fg_tilemap;
	bitmap_t *m_bg_bitmap;
};


/*----------- defined in audio/gomoku.c -----------*/

WRITE8_DEVICE_HANDLER( gomoku_sound1_w );
WRITE8_DEVICE_HANDLER( gomoku_sound2_w );

DECLARE_LEGACY_SOUND_DEVICE(GOMOKU, gomoku_sound);


/*----------- defined in video/gomoku.c -----------*/

PALETTE_INIT( gomoku );
VIDEO_START( gomoku );
SCREEN_UPDATE( gomoku );

WRITE8_HANDLER( gomoku_videoram_w );
WRITE8_HANDLER( gomoku_colorram_w );
WRITE8_HANDLER( gomoku_bgram_w );
WRITE8_HANDLER( gomoku_flipscreen_w );
WRITE8_HANDLER( gomoku_bg_dispsw_w );
