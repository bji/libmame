/***************************************************************************

    Atari Sprint 2 video emulation

***************************************************************************/

#include "emu.h"
#include "includes/sprint2.h"


PALETTE_INIT( sprint2 )
{
	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 4);

	colortable_palette_set_color(machine.colortable, 0, MAKE_RGB(0x00, 0x00, 0x00));
	colortable_palette_set_color(machine.colortable, 1, MAKE_RGB(0x5b, 0x5b, 0x5b));
	colortable_palette_set_color(machine.colortable, 2, MAKE_RGB(0xa4, 0xa4, 0xa4));
	colortable_palette_set_color(machine.colortable, 3, MAKE_RGB(0xff, 0xff, 0xff));

	colortable_entry_set_value(machine.colortable, 0x0, 1);	/* black playfield */
	colortable_entry_set_value(machine.colortable, 0x1, 0);
	colortable_entry_set_value(machine.colortable, 0x2, 1);	/* white playfield */
	colortable_entry_set_value(machine.colortable, 0x3, 3);

	colortable_entry_set_value(machine.colortable, 0x4, 1);	/* car #1 */
	colortable_entry_set_value(machine.colortable, 0x5, 3);
	colortable_entry_set_value(machine.colortable, 0x6, 1);	/* car #2 */
	colortable_entry_set_value(machine.colortable, 0x7, 0);
	colortable_entry_set_value(machine.colortable, 0x8, 1);	/* car #3 */
	colortable_entry_set_value(machine.colortable, 0x9, 2);
	colortable_entry_set_value(machine.colortable, 0xa, 1);	/* car #4 */
	colortable_entry_set_value(machine.colortable, 0xb, 2);
}


static TILE_GET_INFO( get_tile_info )
{
	sprint2_state *state = machine.driver_data<sprint2_state>();
	UINT8 code = state->m_video_ram[tile_index];

	SET_TILE_INFO(0, code & 0x3f, code >> 7, 0);
}


VIDEO_START( sprint2 )
{
	sprint2_state *state = machine.driver_data<sprint2_state>();
	state->m_helper = machine.primary_screen->alloc_compatible_bitmap();

	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 16, 8, 32, 32);
}


READ8_HANDLER( sprint2_collision1_r )
{
	sprint2_state *state = space->machine().driver_data<sprint2_state>();
	return state->m_collision[0];
}
READ8_HANDLER( sprint2_collision2_r )
{
	sprint2_state *state = space->machine().driver_data<sprint2_state>();
	return state->m_collision[1];
}


WRITE8_HANDLER( sprint2_collision_reset1_w )
{
	sprint2_state *state = space->machine().driver_data<sprint2_state>();
	state->m_collision[0] = 0;
}
WRITE8_HANDLER( sprint2_collision_reset2_w )
{
	sprint2_state *state = space->machine().driver_data<sprint2_state>();
	state->m_collision[1] = 0;
}


WRITE8_HANDLER( sprint2_video_ram_w )
{
	sprint2_state *state = space->machine().driver_data<sprint2_state>();
	state->m_video_ram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}


static UINT8 collision_check(sprint2_state *state, colortable_t *colortable, rectangle* rect)
{
	UINT8 data = 0;

	int x;
	int y;

	for (y = rect->min_y; y <= rect->max_y; y++)
		for (x = rect->min_x; x <= rect->max_x; x++)
		{
			UINT16 a = colortable_entry_get_value(colortable, *BITMAP_ADDR16(state->m_helper, y, x));

			if (a == 0)
				data |= 0x40;

			if (a == 3)
				data |= 0x80;
		}

	return data;
}


INLINE int get_sprite_code(UINT8 *video_ram, int n)
{
	return video_ram[0x398 + 2 * n + 1] >> 3;
}
INLINE int get_sprite_x(UINT8 *video_ram, int n)
{
	return 2 * (248 - video_ram[0x390 + 1 * n]);
}
INLINE int get_sprite_y(UINT8 *video_ram, int n)
{
	return 1 * (248 - video_ram[0x398 + 2 * n]);
}


SCREEN_UPDATE( sprint2 )
{
	sprint2_state *state = screen->machine().driver_data<sprint2_state>();
	UINT8 *video_ram = state->m_video_ram;
	int i;

	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);

	/* draw the sprites */

	for (i = 0; i < 4; i++)
	{
		drawgfx_transpen(bitmap, cliprect, screen->machine().gfx[1],
			get_sprite_code(video_ram, i),
			i,
			0, 0,
			get_sprite_x(video_ram, i),
			get_sprite_y(video_ram, i), 0);
	}
	return 0;
}


SCREEN_EOF( sprint2 )
{
	sprint2_state *state = machine.driver_data<sprint2_state>();
	UINT8 *video_ram = state->m_video_ram;
	int i;
	int j;
	const rectangle &visarea = machine.primary_screen->visible_area();

	/*
     * Collisions are detected for both player cars:
     *
     * D7 => state->m_collision w/ white playfield
     * D6 => state->m_collision w/ black playfield or another car
     *
     */

	for (i = 0; i < 2; i++)
	{
		rectangle rect;

		rect.min_x = get_sprite_x(video_ram, i);
		rect.min_y = get_sprite_y(video_ram, i);
		rect.max_x = get_sprite_x(video_ram, i) + machine.gfx[1]->width - 1;
		rect.max_y = get_sprite_y(video_ram, i) + machine.gfx[1]->height - 1;

		if (rect.min_x < visarea.min_x)
			rect.min_x = visarea.min_x;
		if (rect.min_y < visarea.min_y)
			rect.min_y = visarea.min_y;
		if (rect.max_x > visarea.max_x)
			rect.max_x = visarea.max_x;
		if (rect.max_y > visarea.max_y)
			rect.max_y = visarea.max_y;

		/* check for sprite-tilemap collisions */

		tilemap_draw(state->m_helper, &rect, state->m_bg_tilemap, 0, 0);

		drawgfx_transpen(state->m_helper, &rect, machine.gfx[1],
			get_sprite_code(video_ram, i),
			0,
			0, 0,
			get_sprite_x(video_ram, i),
			get_sprite_y(video_ram, i), 1);

		state->m_collision[i] |= collision_check(state, machine.colortable, &rect);

		/* check for sprite-sprite collisions */

		for (j = 0; j < 4; j++)
			if (j != i)
			{
				drawgfx_transpen(state->m_helper, &rect, machine.gfx[1],
					get_sprite_code(video_ram, j),
					1,
					0, 0,
					get_sprite_x(video_ram, j),
					get_sprite_y(video_ram, j), 0);
			}

		drawgfx_transpen(state->m_helper, &rect, machine.gfx[1],
			get_sprite_code(video_ram, i),
			0,
			0, 0,
			get_sprite_x(video_ram, i),
			get_sprite_y(video_ram, i), 1);

		state->m_collision[i] |= collision_check(state, machine.colortable, &rect);
	}
}
