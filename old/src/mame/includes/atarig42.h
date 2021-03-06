/*************************************************************************

    Atari G42 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarig42_state : public atarigen_state
{
public:
	atarig42_state(running_machine &machine, const driver_device_config_base &config)
		: atarigen_state(machine, config) { }

	UINT16			playfield_base;

	UINT16			current_control;
	UINT8			playfield_tile_bank;
	UINT8			playfield_color_bank;
	UINT16			playfield_xscroll;
	UINT16			playfield_yscroll;

	UINT8			analog_data;
	UINT16 *		mo_command;

	int 			sloop_bank;
	int 			sloop_next_bank;
	int 			sloop_offset;
	int 			sloop_state;
	UINT16 *		sloop_base;

	device_t *		rle;
	UINT32			last_accesses[8];
};


/*----------- defined in video/atarig42.c -----------*/

VIDEO_START( atarig42 );
SCREEN_EOF( atarig42 );
SCREEN_UPDATE( atarig42 );

WRITE16_HANDLER( atarig42_mo_control_w );

void atarig42_scanline_update(screen_device &screen, int scanline);

