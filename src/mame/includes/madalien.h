/***************************************************************************

    Mad Alien (c) 1980 Data East Corporation

    Original driver by Norbert Kehrer (February 2004)

***************************************************************************/

#include "sound/discrete.h"


#define MADALIEN_MAIN_CLOCK		XTAL_10_595MHz


class madalien_state : public driver_device
{
public:
	madalien_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_shift_hi;
	UINT8 *m_shift_lo;
	UINT8 *m_videoram;
	UINT8 *m_charram;
	UINT8 *m_video_flags;
	UINT8 *m_video_control;
	UINT8 *m_scroll;
	UINT8 *m_edge1_pos;
	UINT8 *m_edge2_pos;
	UINT8 *m_headlight_pos;
	tilemap_t *m_tilemap_fg;
	tilemap_t *m_tilemap_edge1[4];
	tilemap_t *m_tilemap_edge2[4];
	bitmap_t *m_headlight_bitmap;
};


/*----------- defined in video/madalien.c -----------*/

MACHINE_CONFIG_EXTERN( madalien_video );

WRITE8_HANDLER( madalien_videoram_w );
WRITE8_HANDLER( madalien_charram_w );


/*----------- defined in audio/madalien.c -----------*/

DISCRETE_SOUND_EXTERN( madalien );

/* Discrete Sound Input Nodes */
#define MADALIEN_8910_PORTA			NODE_01
#define MADALIEN_8910_PORTB			NODE_02
