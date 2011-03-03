/*************************************************************************

    Atari G1 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarig1_state : public atarigen_state
{
public:
	atarig1_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT8			is_pitfight;

	UINT8			which_input;
	UINT16 *		mo_command;

	UINT16 *		bslapstic_base;
	void *			bslapstic_bank0;
	UINT8			bslapstic_bank;
	UINT8			bslapstic_primed;

	int 			pfscroll_xoffset;
	UINT16			current_control;
	UINT8			playfield_tile_bank;
	UINT16			playfield_xscroll;
	UINT16			playfield_yscroll;

	device_t *		rle;
};


/*----------- defined in video/atarig1.c -----------*/

WRITE16_HANDLER( atarig1_mo_control_w );

VIDEO_START( atarig1 );
SCREEN_EOF( atarig1 );
SCREEN_UPDATE( atarig1 );

void atarig1_scanline_update(screen_device &screen, int scanline);
