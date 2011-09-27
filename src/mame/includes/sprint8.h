#include "sound/discrete.h"

class sprint8_state : public driver_device
{
public:
	sprint8_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_steer_dir[8];
	int m_steer_flag[8];
	int m_collision_reset;
	int m_collision_index;
	UINT8 m_dial[8];
	UINT8* m_video_ram;
	UINT8* m_pos_h_ram;
	UINT8* m_pos_v_ram;
	UINT8* m_pos_d_ram;
	UINT8* m_team;
	tilemap_t* m_tilemap1;
	tilemap_t* m_tilemap2;
	bitmap_t* m_helper1;
	bitmap_t* m_helper2;
};


/*----------- defined in drivers/sprint8.c -----------*/

void sprint8_set_collision(running_machine &machine, int n);


/*----------- defined in video/sprint8.c -----------*/

PALETTE_INIT( sprint8 );
SCREEN_EOF( sprint8 );
VIDEO_START( sprint8 );
SCREEN_UPDATE( sprint8 );

WRITE8_HANDLER( sprint8_video_ram_w );


/*----------- defined in audio/sprint8.c -----------*/

DISCRETE_SOUND_EXTERN( sprint8 );

WRITE8_DEVICE_HANDLER( sprint8_crash_w );
WRITE8_DEVICE_HANDLER( sprint8_screech_w );
WRITE8_DEVICE_HANDLER( sprint8_attract_w );
WRITE8_DEVICE_HANDLER( sprint8_motor_w );
