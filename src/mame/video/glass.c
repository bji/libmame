/***************************************************************************

  Glass Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "includes/glass.h"

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | xxxxxxxx xxxxxxxx | code
      1  | -------- ---xxxxx | color (uses colors 0x200-0x3ff)
      1  | -------- --x----- | not used?
      1  | -------- -x------ | flip x
      1  | -------- x------- | flip y
      1  | xxxxxxxx -------- | not used
*/

static TILE_GET_INFO( get_tile_info_glass_screen0 )
{
	glass_state *state = machine.driver_data<glass_state>();
	int data = state->m_videoram[tile_index << 1];
	int data2 = state->m_videoram[(tile_index << 1) + 1];
	int code = ((data & 0x03) << 14) | ((data & 0x0fffc) >> 2);

	SET_TILE_INFO(0, code, 0x20 + (data2 & 0x1f), TILE_FLIPYX((data2 & 0xc0) >> 6));
}


static TILE_GET_INFO( get_tile_info_glass_screen1 )
{
	glass_state *state = machine.driver_data<glass_state>();
	int data = state->m_videoram[(0x1000 / 2) + (tile_index << 1)];
	int data2 = state->m_videoram[(0x1000 / 2) + (tile_index << 1) + 1];
	int code = ((data & 0x03) << 14) | ((data & 0x0fffc) >> 2);

	SET_TILE_INFO(0, code, 0x20 + (data2 & 0x1f), TILE_FLIPYX((data2 & 0xc0) >> 6));
}

/***************************************************************************

    Blitter

***************************************************************************/

/*
    The blitter is accessed writing 5 consecutive bits. The stream is: P0 P1 B2 B1 B0

    if P0 is set, the hardware selects the first half of ROM H9 (girls)
    if P1 is set, the hardware selects the second half of ROM H9 (boys)

    B2B1B0 selects the picture (there are 8 pictures in each half of the ROM)
*/

WRITE16_MEMBER(glass_state::glass_blitter_w)
{
	m_blitter_serial_buffer[m_current_bit] = data & 0x01;
	m_current_bit++;

	if (m_current_bit == 5)
	{
		m_current_command = (m_blitter_serial_buffer[0] << 4) |
							(m_blitter_serial_buffer[1] << 3) |
							(m_blitter_serial_buffer[2] << 2) |
							(m_blitter_serial_buffer[3] << 1) |
							(m_blitter_serial_buffer[4] << 0);
		m_current_bit = 0;

		/* fill the screen bitmap with the current picture */
		{
			int i, j;
			UINT8 *gfx = (UINT8 *)machine().root_device().memregion("gfx3")->base();

			gfx = gfx + (m_current_command & 0x07) * 0x10000 + (m_current_command & 0x08) * 0x10000 + 0x140;

			if ((m_current_command & 0x18) != 0)
			{
				for (j = 0; j < 200; j++)
				{
					for (i = 0; i < 320; i++)
					{
						int color = *gfx;
						gfx++;
						m_screen_bitmap->pix16(j, i) = color & 0xff;
					}
				}
			}
			else
				m_screen_bitmap->fill(0);
		}
	}
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

WRITE16_MEMBER(glass_state::glass_vram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_pant[offset >> 11]->mark_tile_dirty(((offset << 1) & 0x0fff) >> 2);
}


/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

VIDEO_START( glass )
{
	glass_state *state = machine.driver_data<glass_state>();
	state->m_pant[0] = tilemap_create(machine, get_tile_info_glass_screen0, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_pant[1] = tilemap_create(machine, get_tile_info_glass_screen1, tilemap_scan_rows, 16, 16, 32, 32);
	state->m_screen_bitmap = auto_bitmap_ind16_alloc (machine, 320, 200);

	state->save_item(NAME(*state->m_screen_bitmap));

	state->m_pant[0]->set_transparent_pen(0);
	state->m_pant[1]->set_transparent_pen(0);
}


/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | --xxxxxx -------- | not used?
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used?
      2  | -------x xxxxxxxx | x position
      2  | ---xxxx- -------- | sprite color (uses colors 0x100-0x1ff)
      2  | xx------ -------- | not used?
      3  | xxxxxxxx xxxxxxxx | sprite code
*/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	glass_state *state = machine.driver_data<glass_state>();
	int i;
	const gfx_element *gfx = machine.gfx[0];

	for (i = 3; i < (0x1000 - 6) / 2; i += 4)
	{
		int sx = state->m_spriteram[i + 2] & 0x01ff;
		int sy = (240 - (state->m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = state->m_spriteram[i + 3];
		int color = (state->m_spriteram[i + 2] & 0x1e00) >> 9;
		int attr = (state->m_spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;

		number = ((number & 0x03) << 14) | ((number & 0x0fffc) >> 2);

		drawgfx_transpen(bitmap,cliprect,gfx,number,
				0x10 + (color & 0x0f),xflip,yflip,
				sx-0x0f,sy,0);
	}
}

/***************************************************************************

    Display Refresh

****************************************************************************/

SCREEN_UPDATE_IND16( glass )
{
	glass_state *state = screen.machine().driver_data<glass_state>();
	/* set scroll registers */
	state->m_pant[0]->set_scrolly(0, state->m_vregs[0]);
	state->m_pant[0]->set_scrollx(0, state->m_vregs[1] + 0x04);
	state->m_pant[1]->set_scrolly(0, state->m_vregs[2]);
	state->m_pant[1]->set_scrollx(0, state->m_vregs[3]);

	/* draw layers + sprites */
	bitmap.fill(get_black_pen(screen.machine()), cliprect);
	copybitmap(bitmap, *state->m_screen_bitmap, 0, 0, 0x18, 0x24, cliprect);
	state->m_pant[1]->draw(bitmap, cliprect, 0, 0);
	state->m_pant[0]->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
