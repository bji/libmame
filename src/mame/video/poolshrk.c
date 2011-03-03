/***************************************************************************

Atari Poolshark video emulation

***************************************************************************/

#include "emu.h"
#include "includes/poolshrk.h"




static TILE_GET_INFO( get_tile_info )
{
	poolshrk_state *state = machine->driver_data<poolshrk_state>();
	SET_TILE_INFO(1, state->playfield_ram[tile_index] & 0x3f, 0, 0);
}


VIDEO_START( poolshrk )
{
	poolshrk_state *state = machine->driver_data<poolshrk_state>();
	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->bg_tilemap, 0);
}


SCREEN_UPDATE( poolshrk )
{
	poolshrk_state *state = screen->machine->driver_data<poolshrk_state>();
	int i;

	tilemap_mark_all_tiles_dirty(state->bg_tilemap);

	bitmap_fill(bitmap, cliprect, 0);

	/* draw sprites */

	for (i = 0; i < 16; i++)
	{
		int hpos = state->hpos_ram[i];
		int vpos = state->vpos_ram[i];

		drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[0], i, (i == 0) ? 0 : 1, 0, 0,
			248 - hpos, vpos - 15, 0);
	}

	/* draw playfield */

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	return 0;
}
