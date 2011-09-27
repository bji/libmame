#include "emu.h"
#include "includes/pushman.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( background_scan_rows )
{
	/* logical (col,row) -> memory offset */
	return ((col & 0x7)) + ((7 - (row & 0x7)) << 3) + ((col & 0x78) << 3) + ((0x38 - (row & 0x38)) << 7);
}

static TILE_GET_INFO( get_back_tile_info )
{
	UINT8 *bg_map = machine.region("gfx4")->base();
	int tile;

	tile = bg_map[tile_index << 1] + (bg_map[(tile_index << 1) + 1] << 8);
	SET_TILE_INFO(
			2,
			(tile & 0xff) | ((tile & 0x4000) >> 6),
			(tile >> 8) & 0xf,
			(tile & 0x2000) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	pushman_state *state = machine.driver_data<pushman_state>();

	int tile = state->m_videoram[tile_index];
	SET_TILE_INFO(
			0,
			(tile & 0xff) | ((tile & 0xc000) >> 6) | ((tile & 0x2000) >> 3),
			(tile >> 8) & 0xf,
			(tile & 0x1000) ? TILE_FLIPY : 0);	/* not used? from Tiger Road */
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( pushman )
{
	pushman_state *state = machine.driver_data<pushman_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_back_tile_info, background_scan_rows, 32, 32, 128, 64);
	state->m_tx_tilemap = tilemap_create(machine, get_text_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->m_tx_tilemap, 3);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( pushman_scroll_w )
{
	pushman_state *state = space->machine().driver_data<pushman_state>();
	COMBINE_DATA(&state->m_control[offset]);
}

WRITE16_HANDLER( pushman_videoram_w )
{
	pushman_state *state = space->machine().driver_data<pushman_state>();
	COMBINE_DATA(&state->m_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_tx_tilemap, offset);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	pushman_state *state = machine.driver_data<pushman_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs, x, y, color, sprite, flipx, flipy;

	for (offs = 0x0800 - 4; offs >=0; offs -= 4)
	{
		/* Don't draw empty sprite table entries */
		x = spriteram[offs + 3] & 0x1ff;
		if (x == 0x180)
			continue;
		if (x > 0xff)
			x = 0 - (0x200 - x);

		y = 240 - spriteram[offs + 2];
		color = ((spriteram[offs + 1] >> 2) & 0xf);
		sprite = spriteram[offs] & 0x7ff;
		/* ElSemi - Sprite flip info */
		flipx = spriteram[offs + 1] & 2;
		flipy = spriteram[offs + 1] & 1;	/* flip y untested */

		if (flip_screen_get(machine))
		{
			x = 240 - x;
			y = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1], sprite,
                color, flipx, flipy, x, y, 15);
	}
}

SCREEN_UPDATE( pushman )
{
	pushman_state *state = screen->machine().driver_data<pushman_state>();

	/* Setup the tilemaps */
	tilemap_set_scrollx(state->m_bg_tilemap, 0, state->m_control[0]);
	tilemap_set_scrolly(state->m_bg_tilemap, 0, 0xf00 - state->m_control[1]);

	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	draw_sprites(screen->machine(), bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->m_tx_tilemap, 0, 0);
	return 0;
}
