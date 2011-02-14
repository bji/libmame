/***************************************************************************

    Video Hardware description for Taito Gladiator

***************************************************************************/

#include "emu.h"
#include "includes/gladiatr.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( bg_get_tile_info )
{
	gladiatr_state *state = machine->driver_data<gladiatr_state>();
	UINT8 attr = state->colorram[tile_index];

	SET_TILE_INFO(
			1,
			state->videoram[tile_index] + ((attr & 0x07) << 8) + (state->bg_tile_bank << 11),
			(attr >> 3) ^ 0x1f,
			0);
}

static TILE_GET_INFO( fg_get_tile_info )
{
	gladiatr_state *state = machine->driver_data<gladiatr_state>();
	SET_TILE_INFO(
			0,
			state->textram[tile_index] + (state->fg_tile_bank << 8),
			0,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ppking )
{
	gladiatr_state *state = machine->driver_data<gladiatr_state>();
	state->bg_tilemap = tilemap_create(machine, bg_get_tile_info,tilemap_scan_rows,8,8,32,64);
	state->fg_tilemap = tilemap_create(machine, fg_get_tile_info,tilemap_scan_rows,8,8,32,64);

	tilemap_set_transparent_pen(state->fg_tilemap,0);

	tilemap_set_scroll_cols(state->bg_tilemap, 0x10);

	state->sprite_bank = 1;
}

VIDEO_START( gladiatr )
{
	gladiatr_state *state = machine->driver_data<gladiatr_state>();
	state->bg_tilemap = tilemap_create(machine, bg_get_tile_info,tilemap_scan_rows,8,8,64,32);
	state->fg_tilemap = tilemap_create(machine, fg_get_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_transparent_pen(state->fg_tilemap,0);

	tilemap_set_scrolldx(state->bg_tilemap, -0x30, 0x12f);
	tilemap_set_scrolldx(state->fg_tilemap, -0x30, 0x12f);

	state->sprite_bank = 2;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( gladiatr_videoram_w )
{
	gladiatr_state *state = space->machine->driver_data<gladiatr_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap,offset);
}

WRITE8_HANDLER( gladiatr_colorram_w )
{
	gladiatr_state *state = space->machine->driver_data<gladiatr_state>();
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap,offset);
}

WRITE8_HANDLER( gladiatr_textram_w )
{
	gladiatr_state *state = space->machine->driver_data<gladiatr_state>();
	state->textram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap,offset);
}

WRITE8_HANDLER( gladiatr_paletteram_w )
{
	int r,g,b;

	space->machine->generic.paletteram.u8[offset] = data;
	offset &= 0x3ff;

	r = (space->machine->generic.paletteram.u8[offset] >> 0) & 0x0f;
	g = (space->machine->generic.paletteram.u8[offset] >> 4) & 0x0f;
	b = (space->machine->generic.paletteram.u8[offset + 0x400] >> 0) & 0x0f;

	r = (r << 1) + ((space->machine->generic.paletteram.u8[offset + 0x400] >> 4) & 0x01);
	g = (g << 1) + ((space->machine->generic.paletteram.u8[offset + 0x400] >> 5) & 0x01);
	b = (b << 1) + ((space->machine->generic.paletteram.u8[offset + 0x400] >> 6) & 0x01);

	palette_set_color_rgb(space->machine,offset,pal5bit(r),pal5bit(g),pal5bit(b));
}


WRITE8_HANDLER( gladiatr_spritebuffer_w )
{
	gladiatr_state *state = space->machine->driver_data<gladiatr_state>();
	state->sprite_buffer = data & 1;
}

WRITE8_HANDLER( gladiatr_spritebank_w )
{
	gladiatr_state *state = space->machine->driver_data<gladiatr_state>();
	state->sprite_bank = (data & 1) ? 4 : 2;
}


WRITE8_HANDLER( ppking_video_registers_w )
{
	gladiatr_state *state = space->machine->driver_data<gladiatr_state>();
	switch (offset & 0x300)
	{
		case 0x000:
			tilemap_set_scrolly(state->bg_tilemap, offset & 0x0f, 0x100-data);
			break;
		case 0x200:
			if (data & 0x80)
				state->fg_scrolly = data + 0x100;
			else
				state->fg_scrolly = data;
			break;
		case 0x300:
			if (state->fg_tile_bank != (data & 0x03))
			{
				state->fg_tile_bank = data & 0x03;
				tilemap_mark_all_tiles_dirty(state->fg_tilemap);
			}
			state->video_attributes = data;
			break;
	}

//popmessage("%02x %02x",state->fg_scrolly, state->video_attributes);
}

WRITE8_HANDLER( gladiatr_video_registers_w )
{
	gladiatr_state *state = space->machine->driver_data<gladiatr_state>();
	switch (offset)
	{
		case 0x000:
			state->fg_scrolly = data;
			break;
		case 0x080:
			if (state->fg_tile_bank != (data & 0x03))
			{
				state->fg_tile_bank = data & 0x03;
				tilemap_mark_all_tiles_dirty(state->fg_tilemap);
			}
			if (state->bg_tile_bank != ((data & 0x10) >> 4))
			{
				state->bg_tile_bank = (data & 0x10) >> 4;
				tilemap_mark_all_tiles_dirty(state->bg_tilemap);
			}
			state->video_attributes = data;
			break;
		case 0x100:
			state->fg_scrollx = data;
			break;
		case 0x200:
			state->bg_scrolly = data;
			break;
		case 0x300:
			state->bg_scrollx = data;
			break;
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	gladiatr_state *state = machine->driver_data<gladiatr_state>();
	int offs;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		static const int tile_offset[2][2] =
		{
			{0x0,0x1},
			{0x2,0x3},
		};
		UINT8 *src = &machine->generic.spriteram.u8[offs + (state->sprite_buffer << 7)];
		int attributes = src[0x800];
		int size = (attributes & 0x10) >> 4;
		int bank = (attributes & 0x01) + ((attributes & 0x02) ? state->sprite_bank : 0);
		int tile_number = (src[0]+256*bank);
		int sx = src[0x400+1] + 256*(src[0x801]&1) - 0x38;
		int sy = 240 - src[0x400] - (size ? 16 : 0);
		int xflip = attributes & 0x04;
		int yflip = attributes & 0x08;
		int color = src[1] & 0x1f;
		int x,y;

		if (flip_screen_get(machine))
		{
			xflip = !xflip;
			yflip = !yflip;
		}

		for (y = 0; y <= size; y++)
		{
			for (x = 0; x <= size; x++)
			{
				int ex = xflip ? (size - x) : x;
				int ey = yflip ? (size - y) : y;

				int t = tile_offset[ey][ex] + tile_number;

				drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
						t,
						color,
						xflip, yflip,
						sx+x*16, sy+y*16,0);
			}
		}
	}
}



VIDEO_UPDATE( ppking )
{
	gladiatr_state *state = screen->machine->driver_data<gladiatr_state>();
	tilemap_draw(bitmap,cliprect,state->bg_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,cliprect);

	/* the fg layer just selects the upper palette bank on underlying pixels */
	{
		bitmap_t *flagsbitmap;
		int sx = cliprect->min_x;
		int sy = cliprect->min_y;

		tilemap_get_pixmap( state->fg_tilemap );
		flagsbitmap = tilemap_get_flagsmap( state->fg_tilemap );

		while( sy <= cliprect->max_y )
		{
			int x = sx;
			int y = (sy + state->fg_scrolly) & 0x1ff;

			UINT16 *dest = BITMAP_ADDR16(bitmap, sy, sx);
			while( x <= cliprect->max_x )
			{
				if( *BITMAP_ADDR8(flagsbitmap, y, x)&TILEMAP_PIXEL_LAYER0 )
				{
					*dest += 512;
				}
				x++;
				dest++;
			} /* next x */
			sy++;
		} /* next y */
	}
	return 0;
}

VIDEO_UPDATE( gladiatr )
{
	gladiatr_state *state = screen->machine->driver_data<gladiatr_state>();
	if (state->video_attributes & 0x20)
	{
		int scroll;

		scroll = state->bg_scrollx + ((state->video_attributes & 0x04) << 6);
		tilemap_set_scrollx(state->bg_tilemap, 0, scroll ^ (flip_screen_get(screen->machine) ? 0x0f : 0));
		scroll = state->fg_scrollx + ((state->video_attributes & 0x08) << 5);
		tilemap_set_scrollx(state->fg_tilemap, 0, scroll ^ (flip_screen_get(screen->machine) ? 0x0f : 0));

		// always 0 anyway
		tilemap_set_scrolly(state->bg_tilemap, 0, state->bg_scrolly);
		tilemap_set_scrolly(state->fg_tilemap, 0, state->fg_scrolly);

		tilemap_draw(bitmap,cliprect,state->bg_tilemap,0,0);
		draw_sprites(screen->machine, bitmap,cliprect);
		tilemap_draw(bitmap,cliprect,state->fg_tilemap,0,0);
	}
	else
		bitmap_fill( bitmap, cliprect , get_black_pen(screen->machine));
	return 0;
}
