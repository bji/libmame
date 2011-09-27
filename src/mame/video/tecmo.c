/***************************************************************************

  video hardware for Tecmo games

***************************************************************************/

#include "emu.h"
#include "includes/tecmo.h"


/*
   video_type is used to distinguish Rygar, Silkworm and Gemini Wing.
   This is needed because there is a difference in the tile and sprite indexing.
*/



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	tecmo_state *state = machine.driver_data<tecmo_state>();
	UINT8 attr = state->m_bgvideoram[tile_index+0x200];
	SET_TILE_INFO(
			3,
			state->m_bgvideoram[tile_index] + ((attr & 0x07) << 8),
			attr >> 4,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	tecmo_state *state = machine.driver_data<tecmo_state>();
	UINT8 attr = state->m_fgvideoram[tile_index+0x200];
	SET_TILE_INFO(
			2,
			state->m_fgvideoram[tile_index] + ((attr & 0x07) << 8),
			attr >> 4,
			0);
}

static TILE_GET_INFO( gemini_get_bg_tile_info )
{
	tecmo_state *state = machine.driver_data<tecmo_state>();
	UINT8 attr = state->m_bgvideoram[tile_index+0x200];
	SET_TILE_INFO(
			3,
			state->m_bgvideoram[tile_index] + ((attr & 0x70) << 4),
			attr & 0x0f,
			0);
}

static TILE_GET_INFO( gemini_get_fg_tile_info )
{
	tecmo_state *state = machine.driver_data<tecmo_state>();
	UINT8 attr = state->m_fgvideoram[tile_index+0x200];
	SET_TILE_INFO(
			2,
			state->m_fgvideoram[tile_index] + ((attr & 0x70) << 4),
			attr & 0x0f,
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	tecmo_state *state = machine.driver_data<tecmo_state>();
	UINT8 attr = state->m_txvideoram[tile_index+0x400];
	SET_TILE_INFO(
			0,
			state->m_txvideoram[tile_index] + ((attr & 0x03) << 8),
			attr >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( tecmo )
{
	tecmo_state *state = machine.driver_data<tecmo_state>();
	if (state->m_video_type == 2)	/* gemini */
	{
		state->m_bg_tilemap = tilemap_create(machine, gemini_get_bg_tile_info,tilemap_scan_rows,16,16,32,16);
		state->m_fg_tilemap = tilemap_create(machine, gemini_get_fg_tile_info,tilemap_scan_rows,16,16,32,16);
	}
	else	/* rygar, silkworm */
	{
		state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,16,16,32,16);
		state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,16,16,32,16);
	}
	state->m_tx_tilemap = tilemap_create(machine, get_tx_tile_info,tilemap_scan_rows, 8, 8,32,32);

	tilemap_set_transparent_pen(state->m_bg_tilemap,0);
	tilemap_set_transparent_pen(state->m_fg_tilemap,0);
	tilemap_set_transparent_pen(state->m_tx_tilemap,0);

	tilemap_set_scrolldx(state->m_bg_tilemap,-48,256+48);
	tilemap_set_scrolldx(state->m_fg_tilemap,-48,256+48);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( tecmo_txvideoram_w )
{
	tecmo_state *state = space->machine().driver_data<tecmo_state>();
	state->m_txvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_tx_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( tecmo_fgvideoram_w )
{
	tecmo_state *state = space->machine().driver_data<tecmo_state>();
	state->m_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap,offset & 0x1ff);
}

WRITE8_HANDLER( tecmo_bgvideoram_w )
{
	tecmo_state *state = space->machine().driver_data<tecmo_state>();
	state->m_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset & 0x1ff);
}

WRITE8_HANDLER( tecmo_fgscroll_w )
{
	tecmo_state *state = space->machine().driver_data<tecmo_state>();

	state->m_fgscroll[offset] = data;

	tilemap_set_scrollx(state->m_fg_tilemap, 0, state->m_fgscroll[0] + 256 * state->m_fgscroll[1]);
	tilemap_set_scrolly(state->m_fg_tilemap, 0, state->m_fgscroll[2]);
}

WRITE8_HANDLER( tecmo_bgscroll_w )
{
	tecmo_state *state = space->machine().driver_data<tecmo_state>();

	state->m_bgscroll[offset] = data;

	tilemap_set_scrollx(state->m_bg_tilemap, 0, state->m_bgscroll[0] + 256 * state->m_bgscroll[1]);
	tilemap_set_scrolly(state->m_bg_tilemap, 0, state->m_bgscroll[2]);
}

WRITE8_HANDLER( tecmo_flipscreen_w )
{
	flip_screen_set(space->machine(), data & 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	tecmo_state *state = machine.driver_data<tecmo_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;
	static const UINT8 layout[8][8] =
	{
		{0,1,4,5,16,17,20,21},
		{2,3,6,7,18,19,22,23},
		{8,9,12,13,24,25,28,29},
		{10,11,14,15,26,27,30,31},
		{32,33,36,37,48,49,52,53},
		{34,35,38,39,50,51,54,55},
		{40,41,44,45,56,57,60,61},
		{42,43,46,47,58,59,62,63}
	};

	for (offs = state->m_spriteram_size-8;offs >= 0;offs -= 8)
	{
		int flags = spriteram[offs+3];
		int priority = flags>>6;
		int bank = spriteram[offs+0];
		if (bank & 4)
		{ /* visible */
			int which = spriteram[offs+1];
			int code,xpos,ypos,flipx,flipy,priority_mask,x,y;
			int size = spriteram[offs + 2] & 3;

			if (state->m_video_type != 0)	/* gemini, silkworm */
			  code = which + ((bank & 0xf8) << 5);
			else						/* rygar */
			  code = which + ((bank & 0xf0) << 4);

			code &= ~((1 << (size*2)) - 1);
			size = 1 << size;

			xpos = spriteram[offs + 5] - ((flags & 0x10) << 4);
			ypos = spriteram[offs + 4] - ((flags & 0x20) << 3);
			flipx = bank & 1;
			flipy = bank & 2;

			if (flip_screen_get(machine))
			{
				xpos = 256 - (8 * size) - xpos;
				ypos = 256 - (8 * size) - ypos;
				flipx = !flipx;
				flipy = !flipy;
			}

			/* bg: 1; fg:2; text: 4 */
			switch (priority)
			{
				default:
				case 0x0: priority_mask = 0; break;
				case 0x1: priority_mask = 0xf0; break; /* obscured by text layer */
				case 0x2: priority_mask = 0xf0|0xcc; break;	/* obscured by foreground */
				case 0x3: priority_mask = 0xf0|0xcc|0xaa; break; /* obscured by bg and fg */
			}

			for (y = 0;y < size;y++)
			{
				for (x = 0;x < size;x++)
				{
					int sx = xpos + 8*(flipx?(size-1-x):x);
					int sy = ypos + 8*(flipy?(size-1-y):y);
					pdrawgfx_transpen(bitmap,cliprect,machine.gfx[1],
							code + layout[y][x],
							flags & 0xf,
							flipx,flipy,
							sx,sy,
							machine.priority_bitmap,
							priority_mask,0);
				}
			}
		}
	}
}


SCREEN_UPDATE( tecmo )
{
	tecmo_state *state = screen->machine().driver_data<tecmo_state>();
	bitmap_fill(screen->machine().priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,0x100);
	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,0,1);
	tilemap_draw(bitmap,cliprect,state->m_fg_tilemap,0,2);
	tilemap_draw(bitmap,cliprect,state->m_tx_tilemap,0,4);

	draw_sprites(screen->machine(), bitmap,cliprect);
	return 0;
}
