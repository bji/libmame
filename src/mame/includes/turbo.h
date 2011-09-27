/*************************************************************************

    Sega Z80-3D system

*************************************************************************/

#include "sound/discrete.h"

/* sprites are scaled in the analog domain; to give a better */
/* rendition of this, we scale in the X direction by this factor */
#define TURBO_X_SCALE		2


struct i8279_state
{
	UINT8		command;
	UINT8		mode;
	UINT8		prescale;
	UINT8		inhibit;
	UINT8		clear;
	UINT8		ram[16];
};


class turbo_state : public driver_device
{
public:
	turbo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *		m_videoram;
	UINT8 *		m_spriteram;
	UINT8 *		m_sprite_position;
	UINT8 *		m_buckrog_bitmap_ram;

	/* machine states */
	i8279_state	m_i8279;

	/* sound state */
	UINT8		m_turbo_osel;
	UINT8		m_turbo_bsel;
	UINT8		m_sound_state[3];

	/* video state */
	tilemap_t *	m_fg_tilemap;

	/* Turbo-specific states */
	UINT8		m_turbo_opa;
	UINT8		m_turbo_opb;
	UINT8		m_turbo_opc;
	UINT8		m_turbo_ipa;
	UINT8		m_turbo_ipb;
	UINT8		m_turbo_ipc;
	UINT8		m_turbo_fbpla;
	UINT8		m_turbo_fbcol;
	UINT8		m_turbo_speed;
	UINT8		m_turbo_collision;
	UINT8		m_turbo_last_analog;
	UINT8		m_turbo_accel;

	/* Subroc-specific states */
	UINT8		m_subroc3d_col;
	UINT8		m_subroc3d_ply;
	UINT8		m_subroc3d_flip;
	UINT8		m_subroc3d_mdis;
	UINT8		m_subroc3d_mdir;
	UINT8		m_subroc3d_tdis;
	UINT8		m_subroc3d_tdir;
	UINT8		m_subroc3d_fdis;
	UINT8		m_subroc3d_fdir;
	UINT8		m_subroc3d_hdis;
	UINT8		m_subroc3d_hdir;

	/* Buck Rogers-specific states */
	UINT8		m_buckrog_fchg;
	UINT8		m_buckrog_mov;
	UINT8		m_buckrog_obch;
	UINT8		m_buckrog_command;
	UINT8		m_buckrog_myship;
	int m_last_sound_a;
};


/*----------- defined in audio/turbo.c -----------*/

MACHINE_CONFIG_EXTERN( turbo_samples );
MACHINE_CONFIG_EXTERN( subroc3d_samples );
MACHINE_CONFIG_EXTERN( buckrog_samples );

WRITE8_DEVICE_HANDLER( turbo_sound_a_w );
WRITE8_DEVICE_HANDLER( turbo_sound_b_w );
WRITE8_DEVICE_HANDLER( turbo_sound_c_w );

WRITE8_DEVICE_HANDLER( subroc3d_sound_a_w );
WRITE8_DEVICE_HANDLER( subroc3d_sound_b_w );
WRITE8_DEVICE_HANDLER( subroc3d_sound_c_w );

WRITE8_DEVICE_HANDLER( buckrog_sound_a_w );
WRITE8_DEVICE_HANDLER( buckrog_sound_b_w );


/*----------- defined in video/turbo.c -----------*/

PALETTE_INIT( turbo );
VIDEO_START( turbo );
SCREEN_UPDATE( turbo );

PALETTE_INIT( subroc3d );
SCREEN_UPDATE( subroc3d );

PALETTE_INIT( buckrog );
VIDEO_START( buckrog );
SCREEN_UPDATE( buckrog );

WRITE8_HANDLER( turbo_videoram_w );
WRITE8_HANDLER( buckrog_bitmap_w );
