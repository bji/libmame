/***************************************************************************
  video/mole.c
  Functions to emulate the video hardware of Mole Attack!.
  Mole Attack's Video hardware is essentially two banks of 512 characters.
  The program uses a single byte to indicate which character goes in each location,
  and uses a control location (0x8400) to select the character sets
***************************************************************************/

#include "emu.h"

static int tile_bank;
static UINT16 *tileram;
static tilemap_t *bg_tilemap;

PALETTE_INIT( mole )
{
	int i;

	for (i = 0; i < 8; i++) {
		palette_set_color_rgb(machine, i, pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1));
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT16 code = tileram[tile_index];
	SET_TILE_INFO((code & 0x200) ? 1 : 0, code & 0x1ff, 0, 0);
}

VIDEO_START( mole )
{
	tileram = auto_alloc_array(machine, UINT16, 0x400);

	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 40, 25);
}

WRITE8_HANDLER( mole_videoram_w )
{
	tileram[offset] = data | (tile_bank << 8);
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( mole_tilebank_w )
{
	tile_bank = data;
	tilemap_mark_all_tiles_dirty(bg_tilemap);
}

WRITE8_HANDLER( mole_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x01);
}

VIDEO_UPDATE( mole )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}
