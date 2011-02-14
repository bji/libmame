/***************************************************************************

    Atari Runaway video emulation

****************************************************************************/

#include "emu.h"
#include "includes/runaway.h"


WRITE8_HANDLER( runaway_paletteram_w )
{
	int R =
		0x21 * ((~data >> 2) & 1) +
		0x47 * ((~data >> 3) & 1) +
		0x97 * ((~data >> 4) & 1);

	int G =
		0x21 * ((~data >> 5) & 1) +
		0x47 * ((~data >> 6) & 1) +
		0x97 * ((~data >> 7) & 1);

	int B =
		0x21 * 0 +
		0x47 * ((~data >> 0) & 1) +
		0x97 * ((~data >> 1) & 1);

	palette_set_color(space->machine, offset, MAKE_RGB(R, G, B));
}



WRITE8_HANDLER( runaway_video_ram_w )
{
	runaway_state *state = space->machine->driver_data<runaway_state>();
	state->video_ram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}



WRITE8_HANDLER( runaway_tile_bank_w )
{
	runaway_state *state = space->machine->driver_data<runaway_state>();
	if ((data & 1) != state->tile_bank)
	{
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	state->tile_bank = data & 1;
}


static TILE_GET_INFO( runaway_get_tile_info )
{
	runaway_state *state = machine->driver_data<runaway_state>();
	UINT8 code = state->video_ram[tile_index];

	SET_TILE_INFO(0, ((code & 0x3f) << 1) | ((code & 0x40) >> 6) | (state->tile_bank << 7), 0, (code & 0x80) ? TILE_FLIPY : 0);
}


static TILE_GET_INFO( qwak_get_tile_info )
{
	runaway_state *state = machine->driver_data<runaway_state>();
	UINT8 code = state->video_ram[tile_index];

	SET_TILE_INFO(0, ((code & 0x7f) << 1) | ((code & 0x80) >> 7), 0, 0);
}



VIDEO_START( runaway )
{
	runaway_state *state = machine->driver_data<runaway_state>();
	state->bg_tilemap = tilemap_create(machine, runaway_get_tile_info, tilemap_scan_rows,  8, 8, 32, 30);

	state->save_item(NAME(state->tile_bank));
}


VIDEO_START( qwak )
{
	runaway_state *state = machine->driver_data<runaway_state>();
	state->bg_tilemap = tilemap_create(machine, qwak_get_tile_info, tilemap_scan_rows,  8, 8, 32, 30);

	state->save_item(NAME(state->tile_bank));
}



VIDEO_UPDATE( runaway )
{
	runaway_state *state = screen->machine->driver_data<runaway_state>();
	int i;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	for (i = 0; i < 16; i++)
	{
		unsigned code = state->sprite_ram[i] & 0x3f;

		int x = state->sprite_ram[i + 0x20];
		int y = state->sprite_ram[i + 0x10];

		int flipx = state->sprite_ram[i] & 0x40;
		int flipy = state->sprite_ram[i] & 0x80;

		code |= (state->sprite_ram[i + 0x30] << 2) & 0x1c0;

		drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[1],
			code,
			0,
			flipx, flipy,
			x, 240 - y, 0);

		drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[1],
			code,
			0,
			flipx, flipy,
			x - 256, 240 - y, 0);
	}
	return 0;
}


VIDEO_UPDATE( qwak )
{
	runaway_state *state = screen->machine->driver_data<runaway_state>();
	int i;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	for (i = 0; i < 16; i++)
	{
		unsigned code = state->sprite_ram[i] & 0x7f;

		int x = state->sprite_ram[i + 0x20];
		int y = state->sprite_ram[i + 0x10];

		int flipx = 0;
		int flipy = state->sprite_ram[i] & 0x80;

		code |= (state->sprite_ram[i + 0x30] << 2) & 0x1c0;

		drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[1],
			code,
			0,
			flipx, flipy,
			x, 240 - y, 0);

		drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[1],
			code,
			0,
			flipx, flipy,
			x - 256, 240 - y, 0);
	}
	return 0;
}
