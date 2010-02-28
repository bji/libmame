/***************************************************************************

Atari Orbit video emulation

***************************************************************************/

#include "emu.h"
#include "includes/orbit.h"

UINT8* orbit_playfield_ram;
UINT8* orbit_sprite_ram;

static tilemap_t* bg_tilemap;

static int orbit_flip_screen;


WRITE8_HANDLER( orbit_playfield_w )
{
	orbit_playfield_ram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


static TILE_GET_INFO( get_tile_info )
{
	UINT8 code = orbit_playfield_ram[tile_index];

	int flags = 0;

	if (code & 0x40)
		flags |= TILE_FLIPX;
	if (orbit_flip_screen)
		flags |= TILE_FLIPY;

	SET_TILE_INFO(3, code & 0x3f, 0, flags);
}


VIDEO_START( orbit )
{
	bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 16, 16, 32, 30);
}


static void draw_sprites(running_machine *machine, bitmap_t* bitmap, const rectangle* cliprect)
{
	const UINT8* p = orbit_sprite_ram;

	int i;

	for (i = 0; i < 16; i++)
	{
		int code = *p++;
		int vpos = *p++;
		int hpos = *p++;
		int flag = *p++;

		int layout =
			((flag & 0xc0) == 0x80) ? 1 :
			((flag & 0xc0) == 0xc0) ? 2 : 0;

		int flip_x = code & 0x40;
		int flip_y = code & 0x80;

		int zoom_x = 0x10000;
		int zoom_y = 0x10000;

		code &= 0x3f;

		if (flag & 1)
			code |= 0x40;
		if (flag & 2)
			zoom_x *= 2;

		vpos = 240 - vpos;

		hpos <<= 1;
		vpos <<= 1;

		drawgfxzoom_transpen(bitmap, cliprect, machine->gfx[layout], code, 0, flip_x, flip_y,
			hpos, vpos, zoom_x, zoom_y, 0);
	}
}


VIDEO_UPDATE( orbit )
{
	orbit_flip_screen = input_port_read(screen->machine, "DSW2") & 8;

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
