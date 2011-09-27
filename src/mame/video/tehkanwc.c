/***************************************************************************

Tehkan World Cup - (c) Tehkan 1985


Ernesto Corvi
ernesto@imagina.com

Roberto Juan Fresca
robbiex@rocketmail.com

***************************************************************************/

#include "emu.h"
#include "includes/tehkanwc.h"


WRITE8_HANDLER( tehkanwc_videoram_w )
{
	tehkanwc_state *state = space->machine().driver_data<tehkanwc_state>();
	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset);
}

WRITE8_HANDLER( tehkanwc_colorram_w )
{
	tehkanwc_state *state = space->machine().driver_data<tehkanwc_state>();
	state->m_colorram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset);
}

WRITE8_HANDLER( tehkanwc_videoram2_w )
{
	tehkanwc_state *state = space->machine().driver_data<tehkanwc_state>();
	state->m_videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset / 2);
}

WRITE8_HANDLER( tehkanwc_scroll_x_w )
{
	tehkanwc_state *state = space->machine().driver_data<tehkanwc_state>();
	state->m_scroll_x[offset] = data;
}

WRITE8_HANDLER( tehkanwc_scroll_y_w )
{
	tehkanwc_state *state = space->machine().driver_data<tehkanwc_state>();
	tilemap_set_scrolly(state->m_bg_tilemap, 0, data);
}

WRITE8_HANDLER( tehkanwc_flipscreen_x_w )
{
	flip_screen_x_set(space->machine(), data & 0x40);
}

WRITE8_HANDLER( tehkanwc_flipscreen_y_w )
{
	flip_screen_y_set(space->machine(), data & 0x40);
}

WRITE8_HANDLER( gridiron_led0_w )
{
	tehkanwc_state *state = space->machine().driver_data<tehkanwc_state>();
	state->m_led0 = data;
}
WRITE8_HANDLER( gridiron_led1_w )
{
	tehkanwc_state *state = space->machine().driver_data<tehkanwc_state>();
	state->m_led1 = data;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	tehkanwc_state *state = machine.driver_data<tehkanwc_state>();
	int offs = tile_index * 2;
	int attr = state->m_videoram2[offs + 1];
	int code = state->m_videoram2[offs] + ((attr & 0x30) << 4);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(2, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	tehkanwc_state *state = machine.driver_data<tehkanwc_state>();
	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] + ((attr & 0x10) << 4);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	tileinfo->category = (attr & 0x20) ? 0 : 1;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( tehkanwc )
{
	tehkanwc_state *state = machine.driver_data<tehkanwc_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 16, 8, 32, 32);

	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->m_fg_tilemap, 0);
}

/*
   Gridiron Fight has a LED display on the control panel, to let each player
   choose the formation without letting the other know.

    ---0---
   |       |
   5       1
   |       |
    ---6---
   |       |
   4       2
   |       |
    ---3---

   bit 7 = enable (0 = display off)
 */

static void gridiron_draw_led(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 led,int player)
{
	if (led&0x80)
		output_set_digit_value(player, led&0x7f);
		else
		output_set_digit_value(player, 0x00);
}

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	tehkanwc_state *state = machine.driver_data<tehkanwc_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0;offs < state->m_spriteram_size;offs += 4)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs] + ((attr & 0x08) << 5);
		int color = attr & 0x07;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = spriteram[offs + 2] + ((attr & 0x20) << 3) - 128;
		int sy = spriteram[offs + 3];

		if (flip_screen_x_get(machine))
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (flip_screen_y_get(machine))
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
			code, color, flipx, flipy, sx, sy, 0);
	}
}

SCREEN_UPDATE( tehkanwc )
{
	tehkanwc_state *state = screen->machine().driver_data<tehkanwc_state>();
	tilemap_set_scrollx(state->m_bg_tilemap, 0, state->m_scroll_x[0] + 256 * state->m_scroll_x[1]);
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);
	draw_sprites(screen->machine(), bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 1, 0);
	gridiron_draw_led(screen->machine(), bitmap, cliprect, state->m_led0, 0);
	gridiron_draw_led(screen->machine(), bitmap, cliprect, state->m_led1, 1);
	return 0;
}
