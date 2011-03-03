/***************************************************************************

Atari Sprint 4 video emulation

***************************************************************************/

#include "emu.h"
#include "audio/sprint4.h"
#include "includes/sprint4.h"


PALETTE_INIT( sprint4 )
{
	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 6);

	colortable_palette_set_color(machine->colortable, 0, MAKE_RGB(0x00, 0x00, 0x00)); /* black  */
	colortable_palette_set_color(machine->colortable, 1, MAKE_RGB(0xfc, 0xdf, 0x80)); /* peach  */
	colortable_palette_set_color(machine->colortable, 2, MAKE_RGB(0xf0, 0x00, 0xf0)); /* violet */
	colortable_palette_set_color(machine->colortable, 3, MAKE_RGB(0x00, 0xf0, 0x0f)); /* green  */
	colortable_palette_set_color(machine->colortable, 4, MAKE_RGB(0x30, 0x4f, 0xff)); /* blue   */
	colortable_palette_set_color(machine->colortable, 5, MAKE_RGB(0xff, 0xff, 0xff)); /* white  */

	colortable_entry_set_value(machine->colortable, 0, 0);
	colortable_entry_set_value(machine->colortable, 2, 0);
	colortable_entry_set_value(machine->colortable, 4, 0);
	colortable_entry_set_value(machine->colortable, 6, 0);
	colortable_entry_set_value(machine->colortable, 8, 0);

	colortable_entry_set_value(machine->colortable, 1, 1);
	colortable_entry_set_value(machine->colortable, 3, 2);
	colortable_entry_set_value(machine->colortable, 5, 3);
	colortable_entry_set_value(machine->colortable, 7, 4);
	colortable_entry_set_value(machine->colortable, 9, 5);
}


static TILE_GET_INFO( sprint4_tile_info )
{
	sprint4_state *state = machine->driver_data<sprint4_state>();
	UINT8 *videoram = state->videoram;
	UINT8 code = videoram[tile_index];

	if ((code & 0x30) == 0x30)
		SET_TILE_INFO(0, code & ~0x40, (code >> 6) ^ 3, 0);
	else
		SET_TILE_INFO(0, code, 4, 0);
}


VIDEO_START( sprint4 )
{
	sprint4_state *state = machine->driver_data<sprint4_state>();
	state->helper = machine->primary_screen->alloc_compatible_bitmap();

	state->playfield = tilemap_create(machine, sprint4_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}


SCREEN_UPDATE( sprint4 )
{
	sprint4_state *state = screen->machine->driver_data<sprint4_state>();
	UINT8 *videoram = state->videoram;
	int i;

	tilemap_draw(bitmap, cliprect, state->playfield, 0, 0);

	for (i = 0; i < 4; i++)
	{
		int bank = 0;

		UINT8 horz = videoram[0x390 + 2 * i + 0];
		UINT8 attr = videoram[0x390 + 2 * i + 1];
		UINT8 vert = videoram[0x398 + 2 * i + 0];
		UINT8 code = videoram[0x398 + 2 * i + 1];

		if (i & 1)
			bank = 32;

		drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[1],
			(code >> 3) | bank,
			(attr & 0x80) ? 4 : i,
			0, 0,
			horz - 15,
			vert - 15, 0);
	}
	return 0;
}


SCREEN_EOF( sprint4 )
{
	sprint4_state *state = machine->driver_data<sprint4_state>();
	UINT8 *videoram = state->videoram;
	int i;

	/* check for sprite-playfield collisions */

	device_t *discrete = machine->device("discrete");

	for (i = 0; i < 4; i++)
	{
		rectangle rect;

		int x;
		int y;

		int bank = 0;

		UINT8 horz = videoram[0x390 + 2 * i + 0];
		UINT8 vert = videoram[0x398 + 2 * i + 0];
		UINT8 code = videoram[0x398 + 2 * i + 1];

		rect.min_x = horz - 15;
		rect.min_y = vert - 15;
		rect.max_x = horz - 15 + machine->gfx[1]->width - 1;
		rect.max_y = vert - 15 + machine->gfx[1]->height - 1;

		sect_rect(&rect, &machine->primary_screen->visible_area());

		tilemap_draw(state->helper, &rect, state->playfield, 0, 0);

		if (i & 1)
			bank = 32;

		drawgfx_transpen(state->helper, &rect, machine->gfx[1],
			(code >> 3) | bank,
			4,
			0, 0,
			horz - 15,
			vert - 15, 1);

		for (y = rect.min_y; y <= rect.max_y; y++)
			for (x = rect.min_x; x <= rect.max_x; x++)
				if (colortable_entry_get_value(machine->colortable, *BITMAP_ADDR16(state->helper, y, x)) != 0)
					state->collision[i] = 1;
	}

	/* update sound status */

	discrete_sound_w(discrete, SPRINT4_MOTOR_DATA_1, videoram[0x391] & 15);
	discrete_sound_w(discrete, SPRINT4_MOTOR_DATA_2, videoram[0x393] & 15);
	discrete_sound_w(discrete, SPRINT4_MOTOR_DATA_3, videoram[0x395] & 15);
	discrete_sound_w(discrete, SPRINT4_MOTOR_DATA_4, videoram[0x397] & 15);
}


WRITE8_HANDLER( sprint4_video_ram_w )
{
	sprint4_state *state = space->machine->driver_data<sprint4_state>();
	UINT8 *videoram = state->videoram;
	videoram[offset] = data;
	tilemap_mark_tile_dirty(state->playfield, offset);
}
