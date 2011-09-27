/***************************************************************************

    Blue Print

***************************************************************************/

#include "emu.h"
#include "includes/blueprnt.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Blue Print doesn't have color PROMs. For sprites, the ROM data is directly
  converted into colors; for characters, it is converted through the color
  code (bits 0-2 = RBG for 01 pixels, bits 3-5 = RBG for 10 pixels, 00 pixels
  always black, 11 pixels use the OR of bits 0-2 and 3-5. Bit 6 is intensity
  control)

***************************************************************************/

PALETTE_INIT( blueprnt )
{
	int i;

	for (i = 0; i < machine.total_colors(); i++)
	{
		UINT8 pen;
		int r, g, b;

		if (i < 0x200)
			/* characters */
			pen = ((i & 0x100) >> 5) |
				  ((i & 0x002) ? ((i & 0x0e0) >> 5) : 0) |
				  ((i & 0x001) ? ((i & 0x01c) >> 2) : 0);
		else
			/* sprites */
			pen = i - 0x200;

		r = ((pen >> 0) & 1) * ((pen & 0x08) ? 0xbf : 0xff);
		g = ((pen >> 2) & 1) * ((pen & 0x08) ? 0xbf : 0xff);
		b = ((pen >> 1) & 1) * ((pen & 0x08) ? 0xbf : 0xff);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

WRITE8_HANDLER( blueprnt_videoram_w )
{
	blueprnt_state *state = space->machine().driver_data<blueprnt_state>();

	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

WRITE8_HANDLER( blueprnt_colorram_w )
{
	blueprnt_state *state = space->machine().driver_data<blueprnt_state>();

	state->m_colorram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

WRITE8_HANDLER( blueprnt_flipscreen_w )
{
	blueprnt_state *state = space->machine().driver_data<blueprnt_state>();

	flip_screen_set(space->machine(), ~data & 0x02);

	if (state->m_gfx_bank != ((data & 0x04) >> 2))
	{
		state->m_gfx_bank = ((data & 0x04) >> 2);
		tilemap_mark_all_tiles_dirty_all(space->machine());
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	blueprnt_state *state = machine.driver_data<blueprnt_state>();
	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] + 256 * state->m_gfx_bank;
	int color = attr & 0x7f;

	tileinfo->category = (attr & 0x80) ? 1 : 0;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( blueprnt )
{
	blueprnt_state *state = machine.driver_data<blueprnt_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols_flip_x, 8, 8, 32, 32);
	tilemap_set_transparent_pen(state->m_bg_tilemap, 0);
	tilemap_set_scroll_cols(state->m_bg_tilemap, 32);

	state->save_item(NAME(state->m_gfx_bank));
}

static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	blueprnt_state *state = machine.driver_data<blueprnt_state>();
	int offs;

	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		int code = state->m_spriteram[offs + 1];
		int sx = state->m_spriteram[offs + 3];
		int sy = 240 - state->m_spriteram[offs];
		int flipx = state->m_spriteram[offs + 2] & 0x40;
		int flipy = state->m_spriteram[offs + 2 - 4] & 0x80;	// -4? Awkward, isn't it?

		if (flip_screen_get(machine))
		{
			sx = 248 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		// sprites are slightly misplaced, regardless of the screen flip
		drawgfx_transpen(bitmap, cliprect, machine.gfx[1], code, 0, flipx, flipy, 2 + sx, sy - 1, 0);
	}
}

SCREEN_UPDATE( blueprnt )
{
	blueprnt_state *state = screen->machine().driver_data<blueprnt_state>();
	int i;

	if (flip_screen_get(screen->machine()))
		for (i = 0; i < 32; i++)
			tilemap_set_scrolly(state->m_bg_tilemap, i, state->m_scrollram[32 - i]);
	else
		for (i = 0; i < 32; i++)
			tilemap_set_scrolly(state->m_bg_tilemap, i, state->m_scrollram[30 - i]);

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	draw_sprites(screen->machine(), bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 1, 0);
	return 0;
}
