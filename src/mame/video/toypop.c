/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/toypop.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  toypop has three 256x4 palette PROM and two 256x8 color lookup table PROMs
  (one for characters, one for sprites).

***************************************************************************/

PALETTE_INIT( toypop )
{
	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 256);

	for (int i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		// red component
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// green component
		bit0 = (color_prom[i+0x100] >> 0) & 0x01;
		bit1 = (color_prom[i+0x100] >> 1) & 0x01;
		bit2 = (color_prom[i+0x100] >> 2) & 0x01;
		bit3 = (color_prom[i+0x100] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// blue component
		bit0 = (color_prom[i+0x200] >> 0) & 0x01;
		bit1 = (color_prom[i+0x200] >> 1) & 0x01;
		bit2 = (color_prom[i+0x200] >> 2) & 0x01;
		bit3 = (color_prom[i+0x200] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r,g,b));
	}

	for (int i = 0;i < 256;i++)
	{
		UINT8 entry;

		// characters
		colortable_entry_set_value(machine.colortable, i + 0*256, (color_prom[i + 0x300] & 0x0f) | 0x70);
		colortable_entry_set_value(machine.colortable, i + 1*256, (color_prom[i + 0x300] & 0x0f) | 0xf0);
		// sprites
		entry = color_prom[i + 0x500];
		colortable_entry_set_value(machine.colortable, i + 2*256, entry);
	}
	for (int i = 0;i < 16;i++)
	{
		// background
		colortable_entry_set_value(machine.colortable, i + 3*256 + 0*16, 0x60 + i);
		colortable_entry_set_value(machine.colortable, i + 3*256 + 1*16, 0xe0 + i);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
static TILEMAP_MAPPER( tilemap_scan )
{
	int offs;

	row += 2;
	col -= 2;
	if (col & 0x20)
		offs = row + ((col & 0x1f) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

static TILE_GET_INFO( get_tile_info )
{
	toypop_state *state = machine.driver_data<toypop_state>();
	UINT8 attr = state->m_videoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			state->m_videoram[tile_index],
			(attr & 0x3f) + 0x40 * state->m_palettebank,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( toypop )
{
	toypop_state *state = machine.driver_data<toypop_state>();
	state->m_bg_tilemap = tilemap_create(machine,get_tile_info,tilemap_scan,8,8,36,28);

	tilemap_set_transparent_pen(state->m_bg_tilemap, 0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( toypop_videoram_w )
{
	toypop_state *state = space->machine().driver_data<toypop_state>();
	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( toypop_palettebank_w )
{
	toypop_state *state = space->machine().driver_data<toypop_state>();
	if (state->m_palettebank != (offset & 1))
	{
		state->m_palettebank = offset & 1;
		tilemap_mark_all_tiles_dirty_all(space->machine());
	}
}

WRITE16_HANDLER( toypop_flipscreen_w )
{
	toypop_state *state = space->machine().driver_data<toypop_state>();
	state->m_bitmapflip = offset & 1;
}

READ16_HANDLER( toypop_merged_background_r )
{
	toypop_state *state = space->machine().driver_data<toypop_state>();
	int data1, data2;

	// 0x0a0b0c0d is read as 0xabcd
	data1 = state->m_bg_image[2*offset];
	data2 = state->m_bg_image[2*offset + 1];
	return ((data1 & 0xf00) << 4) | ((data1 & 0xf) << 8) | ((data2 & 0xf00) >> 4) | (data2 & 0xf);
}

WRITE16_HANDLER( toypop_merged_background_w )
{
	toypop_state *state = space->machine().driver_data<toypop_state>();

	// 0xabcd is written as 0x0a0b0c0d in the background image
	if (ACCESSING_BITS_8_15)
		state->m_bg_image[2*offset] = ((data & 0xf00) >> 8) | ((data & 0xf000) >> 4);

	if (ACCESSING_BITS_0_7)
		state->m_bg_image[2*offset+1] = (data & 0xf) | ((data & 0xf0) << 4);
}

static void draw_background(running_machine &machine, bitmap_t *bitmap)
{
	toypop_state *state = machine.driver_data<toypop_state>();
	pen_t pen_base = 0x300 + 0x10*state->m_palettebank;

	// copy the background image from RAM (0x190200-0x19FDFF) to bitmap
	if (state->m_bitmapflip)
	{
		int offs = 0xFDFE/2;
		for (int y = 0; y < 224; y++)
		{
			UINT16 *scanline = BITMAP_ADDR16(bitmap, y, 0);
			for (int x = 0; x < 288; x+=2)
			{
				UINT16 data = state->m_bg_image[offs];
				scanline[x]   = pen_base | (data & 0x0f);
				scanline[x+1] = pen_base | (data >> 8);
				offs--;
			}
		}
	}
	else
	{
		int offs = 0x200/2;
		for (int y = 0; y < 224; y++)
		{
			UINT16 *scanline = BITMAP_ADDR16(bitmap, y, 0);
			for (int x = 0; x < 288; x+=2)
			{
				UINT16 data = state->m_bg_image[offs];
				scanline[x]   = pen_base | (data >> 8);
				scanline[x+1] = pen_base | (data & 0x0f);
				offs++;
			}
		}
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/


void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 *spriteram_base)
{
	UINT8 *spriteram = spriteram_base + 0x780;
	UINT8 *spriteram_2 = spriteram + 0x800;
	UINT8 *spriteram_3 = spriteram_2 + 0x800;
	enum { xoffs = -31, yoffs = -8 };

	for (int offs = 0;offs < 0x80;offs += 2)
	{
		/* is it on? */
		if ((spriteram_3[offs+1] & 2) == 0)
		{
			static const UINT8 gfx_offs[2][2] =
			{
				{ 0, 1 },
				{ 2, 3 }
			};
			int sprite = spriteram[offs];
			int color = spriteram[offs+1];
			int sx = spriteram_2[offs+1] + 0x100 * (spriteram_3[offs+1] & 1) - 40 + xoffs;
			int sy = 256 - spriteram_2[offs] + yoffs + 1;	// sprites are buffered and delayed by one scanline
			int flipx = (spriteram_3[offs] & 0x01);
			int flipy = (spriteram_3[offs] & 0x02) >> 1;
			int sizex = (spriteram_3[offs] & 0x04) >> 2;
			int sizey = (spriteram_3[offs] & 0x08) >> 3;

			sprite &= ~sizex;
			sprite &= ~(sizey << 1);

			sy -= 16 * sizey;
			sy = (sy & 0xff) - 32;	// fix wraparound

			if (flip_screen_get(machine))
			{
				flipx ^= 1;
				flipy ^= 1;
				sy += 40;
			}

			for (int y = 0;y <= sizey;y++)
			{
				for (int x = 0;x <= sizex;x++)
				{
					drawgfx_transmask(bitmap,cliprect,machine.gfx[1],
						sprite + gfx_offs[y ^ (sizey & flipy)][x ^ (sizex & flipx)],
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,
						colortable_get_transpen_mask(machine.colortable, machine.gfx[1], color, 0xff));
				}
			}
		}
	}
}


SCREEN_UPDATE( toypop )
{
	toypop_state *state = screen->machine().driver_data<toypop_state>();
	draw_background(screen->machine(), bitmap);
	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,0);
	draw_sprites(screen->machine(), bitmap, cliprect, state->m_spriteram);
	return 0;
}
