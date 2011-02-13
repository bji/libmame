/*************************************************************************

    Atari Crystal Castles hardware

*************************************************************************/

#include "cpu/m6502/m6502.h"
#include "machine/x2212.h"

class ccastles_state : public driver_device
{
public:
	ccastles_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  maincpu(*this, "maincpu"),
		  nvram_4b(*this, "nvram_4b"),
		  nvram_4a(*this, "nvram_4a") { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  spriteram;

	/* video-related */
	const UINT8 *syncprom;
	const UINT8 *wpprom;
	const UINT8 *priprom;
	bitmap_t *spritebitmap;
	double rweights[3], gweights[3], bweights[3];
	UINT8 video_control[8];
	UINT8 bitmode_addr[2];
	UINT8 hscroll;
	UINT8 vscroll;

	/* misc */
	int      vblank_start;
	int      vblank_end;
	emu_timer *irq_timer;
	UINT8    irq_state;
	UINT8    nvram_store[2];

	/* devices */
	required_device<m6502_device> maincpu;
	required_device<x2212_device> nvram_4b;
	required_device<x2212_device> nvram_4a;
};


/*----------- defined in video/ccastles.c -----------*/


VIDEO_START( ccastles );
VIDEO_UPDATE( ccastles );

WRITE8_HANDLER( ccastles_hscroll_w );
WRITE8_HANDLER( ccastles_vscroll_w );
WRITE8_HANDLER( ccastles_video_control_w );

WRITE8_HANDLER( ccastles_paletteram_w );
WRITE8_HANDLER( ccastles_videoram_w );

READ8_HANDLER( ccastles_bitmode_r );
WRITE8_HANDLER( ccastles_bitmode_w );
WRITE8_HANDLER( ccastles_bitmode_addr_w );
