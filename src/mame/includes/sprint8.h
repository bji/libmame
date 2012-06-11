#include "sound/discrete.h"

class sprint8_state : public driver_device
{
public:
	sprint8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_video_ram(*this, "video_ram"),
		m_pos_h_ram(*this, "pos_h_ram"),
		m_pos_v_ram(*this, "pos_v_ram"),
		m_pos_d_ram(*this, "pos_d_ram"),
		m_team(*this, "team"){ }

	int m_steer_dir[8];
	int m_steer_flag[8];
	int m_collision_reset;
	int m_collision_index;
	UINT8 m_dial[8];
	required_shared_ptr<UINT8> m_video_ram;
	required_shared_ptr<UINT8> m_pos_h_ram;
	required_shared_ptr<UINT8> m_pos_v_ram;
	required_shared_ptr<UINT8> m_pos_d_ram;
	required_shared_ptr<UINT8> m_team;
	tilemap_t* m_tilemap1;
	tilemap_t* m_tilemap2;
	bitmap_ind16 m_helper1;
	bitmap_ind16 m_helper2;
	DECLARE_READ8_MEMBER(sprint8_collision_r);
	DECLARE_READ8_MEMBER(sprint8_input_r);
	DECLARE_WRITE8_MEMBER(sprint8_lockout_w);
	DECLARE_WRITE8_MEMBER(sprint8_int_reset_w);
	DECLARE_WRITE8_MEMBER(sprint8_video_ram_w);
};


/*----------- defined in drivers/sprint8.c -----------*/

void sprint8_set_collision(running_machine &machine, int n);


/*----------- defined in video/sprint8.c -----------*/

PALETTE_INIT( sprint8 );
SCREEN_VBLANK( sprint8 );
VIDEO_START( sprint8 );
SCREEN_UPDATE_IND16( sprint8 );



/*----------- defined in audio/sprint8.c -----------*/

DISCRETE_SOUND_EXTERN( sprint8 );

WRITE8_DEVICE_HANDLER( sprint8_crash_w );
WRITE8_DEVICE_HANDLER( sprint8_screech_w );
WRITE8_DEVICE_HANDLER( sprint8_attract_w );
WRITE8_DEVICE_HANDLER( sprint8_motor_w );
