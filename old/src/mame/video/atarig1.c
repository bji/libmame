/***************************************************************************

    Atari G1 hardware

****************************************************************************/

#include "emu.h"
#include "video/atarirle.h"
#include "includes/atarig1.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	atarig1_state *state = machine->driver_data<atarig1_state>();
	UINT16 data = state->alpha[tile_index];
	int code = data & 0xfff;
	int color = (data >> 12) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	atarig1_state *state = machine->driver_data<atarig1_state>();
	UINT16 data = state->playfield[tile_index];
	int code = (state->playfield_tile_bank << 12) | (data & 0xfff);
	int color = (data >> 12) & 7;
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atarig1 )
{
	atarig1_state *state = machine->driver_data<atarig1_state>();

	/* blend the playfields and free the temporary one */
	atarigen_blend_gfx(machine, 0, 2, 0x0f, 0x10);

	/* initialize the playfield */
	state->playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, tilemap_scan_rows,  8,8, 64,64);

	/* initialize the motion objects */
	state->rle = machine->device("rle");

	/* initialize the alphanumerics */
	state->alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,32);
	tilemap_set_transparent_pen(state->alpha_tilemap, 0);

	/* reset statics */
	state->pfscroll_xoffset = state->is_pitfight ? 2 : 0;

	/* state saving */
	state->save_item(NAME(state->current_control));
	state->save_item(NAME(state->playfield_tile_bank));
	state->save_item(NAME(state->playfield_xscroll));
	state->save_item(NAME(state->playfield_yscroll));
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

WRITE16_HANDLER( atarig1_mo_control_w )
{
	atarig1_state *state = space->machine->driver_data<atarig1_state>();

	logerror("MOCONT = %d (scan = %d)\n", data, space->machine->primary_screen->vpos());

	/* set the control value */
	COMBINE_DATA(&state->current_control);
}


void atarig1_scanline_update(screen_device &screen, int scanline)
{
	atarig1_state *state = screen.machine->driver_data<atarig1_state>();
	UINT16 *base = &state->alpha[(scanline / 8) * 64 + 48];
	int i;

	//if (scanline == 0) logerror("-------\n");

	/* keep in range */
	if (base >= &state->alpha[0x800])
		return;
	screen.update_partial(MAX(scanline - 1, 0));

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT16 word;

		/* first word controls horizontal scroll */
		word = *base++;
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) + state->pfscroll_xoffset) & 0x1ff;
			if (newscroll != state->playfield_xscroll)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
				tilemap_set_scrollx(state->playfield_tilemap, 0, newscroll);
				state->playfield_xscroll = newscroll;
			}
		}

		/* second word controls vertical scroll and tile bank */
		word = *base++;
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int newbank = word & 7;
			if (newscroll != state->playfield_yscroll)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
				tilemap_set_scrolly(state->playfield_tilemap, 0, newscroll);
				state->playfield_yscroll = newscroll;
			}
			if (newbank != state->playfield_tile_bank)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
				tilemap_mark_all_tiles_dirty(state->playfield_tilemap);
				state->playfield_tile_bank = newbank;
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

SCREEN_UPDATE( atarig1 )
{
	atarig1_state *state = screen->machine->driver_data<atarig1_state>();

	/* draw the playfield */
	tilemap_draw(bitmap, cliprect, state->playfield_tilemap, 0, 0);

	/* copy the motion objects on top */
	copybitmap_trans(bitmap, atarirle_get_vram(state->rle, 0), 0, 0, 0, 0, cliprect, 0);

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, state->alpha_tilemap, 0, 0);
	return 0;
}

SCREEN_EOF( atarig1 )
{
	atarig1_state *state = machine->driver_data<atarig1_state>();

	atarirle_eof(state->rle);
}
