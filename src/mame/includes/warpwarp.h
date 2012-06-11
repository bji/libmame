#include "devlegcy.h"

class warpwarp_state : public driver_device
{
public:
	warpwarp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_geebee_videoram;
	UINT8 *m_videoram;
	int m_geebee_bgw;
	int m_ball_on;
	int m_ball_h;
	int m_ball_v;
	int m_ball_pen;
	int m_ball_sizex;
	int m_ball_sizey;
	int m_handle_joystick;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/warpwarp.c -----------*/

PALETTE_INIT( geebee );
PALETTE_INIT( navarone );
PALETTE_INIT( warpwarp );
VIDEO_START( geebee );
VIDEO_START( navarone );
VIDEO_START( warpwarp );
SCREEN_UPDATE_IND16( geebee );
WRITE8_HANDLER( warpwarp_videoram_w );
WRITE8_HANDLER( geebee_videoram_w );


/*----------- defined in audio/geebee.c -----------*/

WRITE8_DEVICE_HANDLER( geebee_sound_w );

DECLARE_LEGACY_SOUND_DEVICE(GEEBEE, geebee_sound);


/*----------- defined in audio/warpwarp.c -----------*/

WRITE8_DEVICE_HANDLER( warpwarp_sound_w );
WRITE8_DEVICE_HANDLER( warpwarp_music1_w );
WRITE8_DEVICE_HANDLER( warpwarp_music2_w );

DECLARE_LEGACY_SOUND_DEVICE(WARPWARP, warpwarp_sound);
