/*************************************************************************

    Atari System 1 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarisy1_state : public atarigen_state
{
public:
	atarisy1_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config),
		  joystick_timer(*this, "joystick_timer"),
		  yscroll_reset_timer(*this, "yreset_timer"),
		  scanline_timer(*this, "scan_timer"),
		  int3off_timer(*this, "int3off_timer") { }

	UINT16 *		bankselect;

	UINT8			joystick_type;
	UINT8			trackball_type;

	required_device<timer_device> joystick_timer;
	UINT8			joystick_int;
	UINT8			joystick_int_enable;
	UINT8			joystick_value;

	/* playfield parameters */
	UINT16			playfield_lookup[256];
	UINT8			playfield_tile_bank;
	UINT16			playfield_priority_pens;
	required_device<timer_device> yscroll_reset_timer;

	/* INT3 tracking */
	int 			next_timer_scanline;
	required_device<timer_device> scanline_timer;
	required_device<timer_device> int3off_timer;

	/* graphics bank tracking */
	UINT8			bank_gfx[3][8];
	UINT8			bank_color_shift[MAX_GFX_ELEMENTS];

	UINT8			cur[2][2];
};


/*----------- defined in video/atarisy1.c -----------*/

TIMER_DEVICE_CALLBACK( atarisy1_int3_callback );
TIMER_DEVICE_CALLBACK( atarisy1_int3off_callback );
TIMER_DEVICE_CALLBACK( atarisy1_reset_yscroll_callback );

READ16_HANDLER( atarisy1_int3state_r );

WRITE16_HANDLER( atarisy1_spriteram_w );
WRITE16_HANDLER( atarisy1_bankselect_w );
WRITE16_HANDLER( atarisy1_xscroll_w );
WRITE16_HANDLER( atarisy1_yscroll_w );
WRITE16_HANDLER( atarisy1_priority_w );

VIDEO_START( atarisy1 );
SCREEN_UPDATE( atarisy1 );
