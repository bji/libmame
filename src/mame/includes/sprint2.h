/*************************************************************************

    Atari Sprint hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define SPRINT2_SKIDSND1_EN        NODE_01
#define SPRINT2_SKIDSND2_EN        NODE_02
#define SPRINT2_MOTORSND1_DATA     NODE_03
#define SPRINT2_MOTORSND2_DATA     NODE_04
#define SPRINT2_CRASHSND_DATA      NODE_05
#define SPRINT2_ATTRACT_EN         NODE_06
#define SPRINT2_NOISE_RESET        NODE_07

#define DOMINOS_FREQ_DATA          SPRINT2_MOTORSND1_DATA
#define DOMINOS_AMP_DATA           SPRINT2_CRASHSND_DATA
#define DOMINOS_TUMBLE_EN          SPRINT2_SKIDSND1_EN
#define DOMINOS_ATTRACT_EN         SPRINT2_ATTRACT_EN


class sprint2_state : public driver_device
{
public:
	sprint2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_attract;
	int m_steering[2];
	int m_gear[2];
	int m_game;
	UINT8 m_dial[2];
	UINT8* m_video_ram;
	tilemap_t* m_bg_tilemap;
	bitmap_ind16 m_helper;
	int m_collision[2];
};


/*----------- defined in audio/sprint2.c -----------*/

DISCRETE_SOUND_EXTERN( sprint2 );
DISCRETE_SOUND_EXTERN( sprint1 );
DISCRETE_SOUND_EXTERN( dominos );


/*----------- defined in video/sprint2.c -----------*/

READ8_HANDLER( sprint2_collision1_r );
READ8_HANDLER( sprint2_collision2_r );

WRITE8_HANDLER( sprint2_collision_reset1_w );
WRITE8_HANDLER( sprint2_collision_reset2_w );
WRITE8_HANDLER( sprint2_video_ram_w );

PALETTE_INIT( sprint2 );
SCREEN_UPDATE_IND16( sprint2 );
VIDEO_START( sprint2 );
SCREEN_VBLANK( sprint2 );

