#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/spdodgeb.h"


PALETTE_INIT( spdodgeb )
{
	const UINT8 *color_prom = machine.root_device().memregion("proms")->base();
	int i;


	for (i = 0;i < machine.total_colors();i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[machine.total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[machine.total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[machine.total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[machine.total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( background_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	spdodgeb_state *state = machine.driver_data<spdodgeb_state>();
	UINT8 code = state->m_videoram[tile_index];
	UINT8 attr = state->m_videoram[tile_index + 0x800];
	SET_TILE_INFO(
			0,
			code + ((attr & 0x1f) << 8),
			((attr & 0xe0) >> 5) + 8 * state->m_tile_palbank,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( spdodgeb )
{
	spdodgeb_state *state = machine.driver_data<spdodgeb_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,background_scan,8,8,64,32);
}


/***************************************************************************

  Memory handlers

***************************************************************************/


TIMER_DEVICE_CALLBACK( spdodgeb_interrupt )
{
	spdodgeb_state *state = timer.machine().driver_data<spdodgeb_state>();
	int scanline = param;

	if (scanline == 256)
	{
		device_set_input_line(state->m_maincpu, INPUT_LINE_NMI, PULSE_LINE);
		timer.machine().primary_screen->update_partial(256);
	}
	else if ((scanline % 8) == 0)
	{
		device_set_input_line(state->m_maincpu, M6502_IRQ_LINE, HOLD_LINE);
		timer.machine().primary_screen->update_partial(scanline+16); /* TODO: pretty off ... */
	}
}

WRITE8_MEMBER(spdodgeb_state::spdodgeb_scrollx_lo_w)
{
	m_lastscroll = (m_lastscroll & 0x100) | data;
}

WRITE8_MEMBER(spdodgeb_state::spdodgeb_ctrl_w)
{
	UINT8 *rom = memregion("maincpu")->base();

	/* bit 0 = flip screen */
	flip_screen_set(data & 0x01);

	/* bit 1 = ROM bank switch */
	membank("bank1")->set_base(rom + 0x10000 + 0x4000 * ((~data & 0x02) >> 1));

	/* bit 2 = scroll high bit */
	m_lastscroll = (m_lastscroll & 0x0ff) | ((data & 0x04) << 6);

	/* bit 3 = to mcu?? */

	/* bits 4-7 = palette bank select */
	if (m_tile_palbank != ((data & 0x30) >> 4))
	{
		m_tile_palbank = ((data & 0x30) >> 4);
		m_bg_tilemap->mark_all_dirty();
	}
	m_sprite_palbank = (data & 0xc0) >> 6;
}

WRITE8_MEMBER(spdodgeb_state::spdodgeb_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

#define DRAW_SPRITE( order, sx, sy ) drawgfx_transpen( bitmap, \
					cliprect,gfx, \
					(which+order),color+ 8 * state->m_sprite_palbank,flipx,flipy,sx,sy,0);

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	spdodgeb_state *state = machine.driver_data<spdodgeb_state>();
	UINT8 *spriteram = state->m_spriteram;
	const gfx_element *gfx = machine.gfx[1];
	UINT8 *src;
	int i;

	src = spriteram;

/*  240-SY   Z|F|CLR|WCH WHICH    SX
    xxxxxxxx x|x|xxx|xxx xxxxxxxx xxxxxxxx
*/
	for (i = 0;i < state->m_spriteram.bytes();i += 4)
	{
		int attr = src[i+1];
		int which = src[i+2]+((attr & 0x07)<<8);
		int sx = src[i+3];
		int sy = 240 - src[i];
		int size = (attr & 0x80) >> 7;
		int color = (attr & 0x38) >> 3;
		int flipx = ~attr & 0x40;
		int flipy = 0;
		int dy = -16;
		int cy;

		if (state->flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
			dy = -dy;
		}

		if (sx < -8) sx += 256; else if (sx > 248) sx -= 256;

		switch (size)
		{
			case 0: /* normal */
			if (sy < -8) sy += 256; else if (sy > 248) sy -= 256;
			DRAW_SPRITE(0,sx,sy);
			break;

			case 1: /* double y */
			if (state->flip_screen()) { if (sy > 240) sy -= 256; } else { if (sy < 0) sy += 256; }
			cy = sy + dy;
			which &= ~1;
			DRAW_SPRITE(0,sx,cy);
			DRAW_SPRITE(1,sx,sy);
			break;
		}
	}
}

#undef DRAW_SPRITE


SCREEN_UPDATE_IND16( spdodgeb )
{
	spdodgeb_state *state = screen.machine().driver_data<spdodgeb_state>();
	state->m_bg_tilemap->set_scrollx(0,state->m_lastscroll+5);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap,cliprect);
	return 0;
}
