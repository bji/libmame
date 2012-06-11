/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/srumbler.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	srumbler_state *state = machine.driver_data<srumbler_state>();
	UINT8 attr = state->m_foregroundram[2*tile_index];
	SET_TILE_INFO(
			0,
			state->m_foregroundram[2*tile_index + 1] + ((attr & 0x03) << 8),
			(attr & 0x3c) >> 2,
			(attr & 0x40) ? TILE_FORCE_LAYER0 : 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	srumbler_state *state = machine.driver_data<srumbler_state>();
	UINT8 attr = state->m_backgroundram[2*tile_index];
	SET_TILE_INFO(
			1,
			state->m_backgroundram[2*tile_index + 1] + ((attr & 0x07) << 8),
			(attr & 0xe0) >> 5,
			((attr & 0x08) ? TILE_FLIPY : 0));
	tileinfo.group = (attr & 0x10) >> 4;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( srumbler )
{
	srumbler_state *state = machine.driver_data<srumbler_state>();
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_cols,8,8,64,32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_cols,    16,16,64,64);

	state->m_fg_tilemap->set_transparent_pen(3);

	state->m_bg_tilemap->set_transmask(0,0xffff,0x0000); /* split type 0 is totally transparent in front half */
	state->m_bg_tilemap->set_transmask(1,0x07ff,0xf800); /* split type 1 has pens 0-10 transparent in front half */
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( srumbler_foreground_w )
{
	srumbler_state *state = space->machine().driver_data<srumbler_state>();
	state->m_foregroundram[offset] = data;
	state->m_fg_tilemap->mark_tile_dirty(offset/2);
}

WRITE8_HANDLER( srumbler_background_w )
{
	srumbler_state *state = space->machine().driver_data<srumbler_state>();
	state->m_backgroundram[offset] = data;
	state->m_bg_tilemap->mark_tile_dirty(offset/2);
}


WRITE8_HANDLER( srumbler_4009_w )
{
	/* bit 0 flips screen */
	flip_screen_set(space->machine(), data & 1);

	/* bits 4-5 used during attract mode, unknown */

	/* bits 6-7 coin counters */
	coin_counter_w(space->machine(), 0,data & 0x40);
	coin_counter_w(space->machine(), 1,data & 0x80);
}


WRITE8_HANDLER( srumbler_scroll_w )
{
	srumbler_state *state = space->machine().driver_data<srumbler_state>();

	state->m_scroll[offset] = data;

	state->m_bg_tilemap->set_scrollx(0,state->m_scroll[0] | (state->m_scroll[1] << 8));
	state->m_bg_tilemap->set_scrolly(0,state->m_scroll[2] | (state->m_scroll[3] << 8));
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *buffered_spriteram = machine.generic.buffered_spriteram.u8;
	int offs;

	/* Draw the sprites. */
	for (offs = machine.generic.spriteram_size-4; offs>=0;offs -= 4)
	{
		/* SPRITES
        =====
        Attribute
        0x80 Code MSB
        0x40 Code MSB
        0x20 Code MSB
        0x10 Colour
        0x08 Colour
        0x04 Colour
        0x02 y Flip
        0x01 X MSB
        */


		int code,colour,sx,sy,flipy;
		int attr = buffered_spriteram[offs+1];
		code = buffered_spriteram[offs];
		code += ( (attr&0xe0) << 3 );
		colour = (attr & 0x1c)>>2;
		sy = buffered_spriteram[offs + 2];
		sx = buffered_spriteram[offs + 3] + 0x100 * ( attr & 0x01);
		flipy = attr & 0x02;

		if (flip_screen_get(machine))
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[2],
				code,
				colour,
				flip_screen_get(machine),flipy,
				sx, sy,15);
	}
}


SCREEN_UPDATE_IND16( srumbler )
{
	srumbler_state *state = screen.machine().driver_data<srumbler_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1,0);
	draw_sprites(screen.machine(), bitmap,cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER0,0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}

SCREEN_VBLANK( srumbler )
{
	// rising edge
	if (vblank_on)
	{
		address_space *space = screen.machine().device("maincpu")->memory().space(AS_PROGRAM);

		buffer_spriteram_w(space,0,0);
	}
}
