/*************************************************************************

    Atari Video Pinball video emulation

*************************************************************************/

#include "emu.h"
#include "includes/videopin.h"





static TILEMAP_MAPPER( get_memory_offset )
{
	return num_rows * ((col + 16) % 48) + row;
}


static TILE_GET_INFO( get_tile_info )
{
	videopin_state *state = machine->driver_data<videopin_state>();
	UINT8 code = state->video_ram[tile_index];

	SET_TILE_INFO(0, code, 0, (code & 0x40) ? TILE_FLIPY : 0);
}


VIDEO_START( videopin )
{
	videopin_state *state = machine->driver_data<videopin_state>();
	state->bg_tilemap = tilemap_create(machine, get_tile_info, get_memory_offset,  8, 8, 48, 32);
}


VIDEO_UPDATE( videopin )
{
	videopin_state *state = screen->machine->driver_data<videopin_state>();
	int col;
	int row;

	tilemap_set_scrollx(state->bg_tilemap, 0, -8);   /* account for delayed loading of shift reg C6 */

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	for (row = 0; row < 32; row++)
	{
		for (col = 0; col < 48; col++)
		{
			UINT32 offset = get_memory_offset(col, row, 48, 32);

			if (state->video_ram[offset] & 0x80)   /* ball bit found */
			{
				rectangle rect;

				int x = 8 * col;
				int y = 8 * row;

				int i;
				int j;

				x += 4;   /* account for delayed loading of flip-flop C4 */

				rect.min_x = x;
				rect.min_y = y;
				rect.max_x = x + 15;
				rect.max_y = y + 15;

				if (rect.min_x < cliprect->min_x)
					rect.min_x = cliprect->min_x;
				if (rect.min_y < cliprect->min_y)
					rect.min_y = cliprect->min_y;
				if (rect.max_x > cliprect->max_x)
					rect.max_x = cliprect->max_x;
				if (rect.max_y > cliprect->max_y)
					rect.max_y = cliprect->max_y;

				x -= state->ball_x;
				y -= state->ball_y;

				/* ball placement is still 0.5 pixels off but don't tell anyone */

				for (i = 0; i < 2; i++)
				{
					for (j = 0; j < 2; j++)
					{
						drawgfx_transpen(bitmap, &rect, screen->machine->gfx[1],
							0, 0,
							0, 0,
							x + 16 * i,
							y + 16 * j, 0);
					}
				}

				return 0;   /* keep things simple and ignore the rest */
			}
		}
	}
	return 0;
}


WRITE8_HANDLER( videopin_ball_w )
{
	videopin_state *state = space->machine->driver_data<videopin_state>();
	state->ball_x = data & 15;
	state->ball_y = data >> 4;
}


WRITE8_HANDLER( videopin_video_ram_w )
{
	videopin_state *state = space->machine->driver_data<videopin_state>();
	state->video_ram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}
