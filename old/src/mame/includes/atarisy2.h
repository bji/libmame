/*************************************************************************

    Atari System 2 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarisy2_state : public atarigen_state
{
public:
	atarisy2_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT16 *		slapstic_base;

	UINT8			interrupt_enable;
	UINT16 *		bankselect;

	INT8			pedal_count;

	UINT8			has_tms5220;

	UINT8			which_adc;

	UINT8			p2portwr_state;
	UINT8			p2portrd_state;

	UINT16 *		rombank1;
	UINT16 *		rombank2;

	UINT8			sound_reset_state;

	emu_timer *		yscroll_reset_timer;
	UINT32			playfield_tile_bank[2];
	UINT32			videobank;

	// 720 fake joystick
	double			joy_last_angle;
	int				joy_rotations;

	// 720 fake spinner
	UINT32			spin_last_rotate_count;
	INT32			spin_pos;					/* track fake position of spinner */
	UINT32			spin_center_count;

	UINT16			vram[0x8000/2];
};


/*----------- defined in video/atarisy2.c -----------*/

READ16_HANDLER( atarisy2_slapstic_r );
READ16_HANDLER( atarisy2_videoram_r );

WRITE16_HANDLER( atarisy2_slapstic_w );
WRITE16_HANDLER( atarisy2_yscroll_w );
WRITE16_HANDLER( atarisy2_xscroll_w );
WRITE16_HANDLER( atarisy2_videoram_w );
WRITE16_HANDLER( atarisy2_paletteram_w );

VIDEO_START( atarisy2 );
SCREEN_UPDATE( atarisy2 );
