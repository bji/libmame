/***************************************************************************

    Atari GX2 hardware

*****************************************************************************

    MO data has 12 bits total: MVID0-11
    MVID9-11 form the priority
    MVID0-9 form the color bits

    PF data has 13 bits total: PF.VID0-12
    PF.VID10-12 form the priority
    PF.VID0-9 form the color bits

    Upper bits come from the low 5 bits of the HSCROLL value in alpha RAM
    Playfield bank comes from low 2 bits of the VSCROLL value in alpha RAM
    For GX2, there are 4 bits of bank1

****************************************************************************/


#include "emu.h"
#include "video/atarirle.h"
#include "includes/atarigx2.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	atarigx2_state *state = machine.driver_data<atarigx2_state>();
	UINT16 data = state->m_alpha32[tile_index / 2] >> (16 * (~tile_index & 1));
	int code = data & 0xfff;
	int color = (data >> 12) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	atarigx2_state *state = machine.driver_data<atarigx2_state>();
	UINT16 data = state->m_playfield32[tile_index / 2] >> (16 * (~tile_index & 1));
	int code = (state->m_playfield_tile_bank << 12) | (data & 0xfff);
	int color = (state->m_playfield_base >> 5) + ((state->m_playfield_color_bank << 3) & 0x18) + ((data >> 12) & 7);
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
	tileinfo.category = (state->m_playfield_color_bank >> 2) & 7;
}


static TILEMAP_MAPPER( atarigx2_playfield_scan )
{
	int bank = 1 - (col / (num_cols / 2));
	return bank * (num_rows * num_cols / 2) + row * (num_cols / 2) + (col % (num_cols / 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atarigx2 )
{
	atarigx2_state *state = machine.driver_data<atarigx2_state>();

	/* blend the playfields and free the temporary one */
	atarigen_blend_gfx(machine, 0, 2, 0x0f, 0x30);

	/* initialize the playfield */
	state->m_playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, atarigx2_playfield_scan,  8,8, 128,64);

	/* initialize the motion objects */
	state->m_rle = machine.device("rle");

	/* initialize the alphanumerics */
	state->m_alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,32);
	state->m_alpha_tilemap->set_transparent_pen(0);

	/* save states */
	state->save_item(NAME(state->m_current_control));
	state->save_item(NAME(state->m_playfield_tile_bank));
	state->save_item(NAME(state->m_playfield_color_bank));
	state->save_item(NAME(state->m_playfield_xscroll));
	state->save_item(NAME(state->m_playfield_yscroll));
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

WRITE16_HANDLER( atarigx2_mo_control_w )
{
	atarigx2_state *state = space->machine().driver_data<atarigx2_state>();

	logerror("MOCONT = %d (scan = %d)\n", data, space->machine().primary_screen->vpos());

	/* set the control value */
	COMBINE_DATA(&state->m_current_control);
}


void atarigx2_scanline_update(screen_device &screen, int scanline)
{
	atarigx2_state *state = screen.machine().driver_data<atarigx2_state>();
	UINT32 *base = &state->m_alpha32[(scanline / 8) * 32 + 24];
	int i;

	if (scanline == 0) logerror("-------\n");

	/* keep in range */
	if (base >= &state->m_alpha32[0x400])
		return;

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT32 word = *base++;

		if (word & 0x80000000)
		{
			int newscroll = (word >> 21) & 0x3ff;
			int newbank = (word >> 16) & 0x1f;
			if (newscroll != state->m_playfield_xscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				state->m_playfield_tilemap->set_scrollx(0, newscroll);
				state->m_playfield_xscroll = newscroll;
			}
			if (newbank != state->m_playfield_color_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				state->m_playfield_tilemap->mark_all_dirty();
				state->m_playfield_color_bank = newbank;
			}
		}

		if (word & 0x00008000)
		{
			int newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int newbank = word & 15;
			if (newscroll != state->m_playfield_yscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				state->m_playfield_tilemap->set_scrolly(0, newscroll);
				state->m_playfield_yscroll = newscroll;
			}
			if (newbank != state->m_playfield_tile_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				state->m_playfield_tilemap->mark_all_dirty();
				state->m_playfield_tile_bank = newbank;
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

SCREEN_UPDATE_IND16( atarigx2 )
{
	atarigx2_state *state = screen.machine().driver_data<atarigx2_state>();
	bitmap_ind8 &priority_bitmap = screen.machine().priority_bitmap;

	/* draw the playfield */
	priority_bitmap.fill(0, cliprect);
	state->m_playfield_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_playfield_tilemap->draw(bitmap, cliprect, 1, 1);
	state->m_playfield_tilemap->draw(bitmap, cliprect, 2, 2);
	state->m_playfield_tilemap->draw(bitmap, cliprect, 3, 3);
	state->m_playfield_tilemap->draw(bitmap, cliprect, 4, 4);
	state->m_playfield_tilemap->draw(bitmap, cliprect, 5, 5);
	state->m_playfield_tilemap->draw(bitmap, cliprect, 6, 6);
	state->m_playfield_tilemap->draw(bitmap, cliprect, 7, 7);

	/* copy the motion objects on top */
	{
		bitmap_ind16 *mo_bitmap = atarirle_get_vram(state->m_rle, 0);
		int left	= cliprect.min_x;
		int top		= cliprect.min_y;
		int right	= cliprect.max_x + 1;
		int bottom	= cliprect.max_y + 1;
		int x, y;

		/* now blend with the playfield */
		for (y = top; y < bottom; y++)
		{
			UINT16 *pf = &bitmap.pix16(y);
			UINT16 *mo = &mo_bitmap->pix16(y);
			UINT8 *pri = &priority_bitmap.pix8(y);
			for (x = left; x < right; x++)
				if (mo[x] && (mo[x] >> ATARIRLE_PRIORITY_SHIFT) >= pri[x])
					pf[x] = mo[x] & ATARIRLE_DATA_MASK;
		}
	}

	/* add the alpha on top */
	state->m_alpha_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_VBLANK( atarigx2 )
{
	// rising edge
	if (vblank_on)
	{
		atarigx2_state *state = screen.machine().driver_data<atarigx2_state>();

		atarirle_eof(state->m_rle);
	}
}
