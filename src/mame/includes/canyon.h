/*************************************************************************

    Atari Canyon Bomber hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define CANYON_MOTOR1_DATA		NODE_01
#define CANYON_MOTOR2_DATA		NODE_02
#define CANYON_EXPLODE_DATA		NODE_03
#define CANYON_WHISTLE1_EN		NODE_04
#define CANYON_WHISTLE2_EN		NODE_05
#define CANYON_ATTRACT1_EN		NODE_06
#define CANYON_ATTRACT2_EN		NODE_07



class canyon_state : public driver_device
{
public:
	canyon_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  m_videoram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
};


/*----------- defined in audio/canyon.c -----------*/

WRITE8_DEVICE_HANDLER( canyon_motor_w );
WRITE8_DEVICE_HANDLER( canyon_explode_w );
WRITE8_DEVICE_HANDLER( canyon_attract_w );
WRITE8_DEVICE_HANDLER( canyon_whistle_w );

DISCRETE_SOUND_EXTERN( canyon );


/*----------- defined in video/canyon.c -----------*/

VIDEO_START( canyon );
SCREEN_UPDATE( canyon );

WRITE8_HANDLER( canyon_videoram_w );
