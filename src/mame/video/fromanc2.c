/******************************************************************************

    Video Hardware for Video System Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -

******************************************************************************/

#include "emu.h"
#include "includes/fromanc2.h"

/******************************************************************************

  Callbacks for the TileMap code

******************************************************************************/

INLINE void fromanc2_get_tile_info( running_machine &machine, tile_data *tileinfo, int tile_index, int vram, int layer )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();
	int tile, color;

	tile  = (state->m_videoram[vram][layer][tile_index] & 0x3fff) | (state->m_gfxbank[vram][layer] << 14);
	color = ((state->m_videoram[vram][layer][tile_index] & 0xc000) >> 14) | (0x10 * vram);

	SET_TILE_INFO(layer, tile, color, 0);
}

static TILE_GET_INFO( fromanc2_get_v0_l0_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 0, 0); }
static TILE_GET_INFO( fromanc2_get_v0_l1_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 0, 1); }
static TILE_GET_INFO( fromanc2_get_v0_l2_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 0, 2); }
static TILE_GET_INFO( fromanc2_get_v0_l3_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 0, 3); }
static TILE_GET_INFO( fromanc2_get_v1_l0_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 1, 0); }
static TILE_GET_INFO( fromanc2_get_v1_l1_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 1, 1); }
static TILE_GET_INFO( fromanc2_get_v1_l2_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 1, 2); }
static TILE_GET_INFO( fromanc2_get_v1_l3_tile_info ) { fromanc2_get_tile_info(machine, tileinfo, tile_index, 1, 3); }


INLINE void fromancr_get_tile_info( running_machine &machine, tile_data *tileinfo, int tile_index, int vram, int layer )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();
	int tile, color;

	tile  = state->m_videoram[vram][layer][tile_index] | (state->m_gfxbank[vram][layer] << 16);
	color = vram;

	SET_TILE_INFO(layer, tile, color, 0);
}

static TILE_GET_INFO( fromancr_get_v0_l0_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 0, 0); }
static TILE_GET_INFO( fromancr_get_v0_l1_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 0, 1); }
static TILE_GET_INFO( fromancr_get_v0_l2_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 0, 2); }
static TILE_GET_INFO( fromancr_get_v1_l0_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 1, 0); }
static TILE_GET_INFO( fromancr_get_v1_l1_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 1, 1); }
static TILE_GET_INFO( fromancr_get_v1_l2_tile_info ) { fromancr_get_tile_info(machine, tileinfo, tile_index, 1, 2); }


/******************************************************************************

  Memory handlers

******************************************************************************/

READ16_HANDLER( fromanc2_paletteram_0_r )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	return state->m_paletteram[0][offset];
}

READ16_HANDLER( fromanc2_paletteram_1_r )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	return state->m_paletteram[1][offset];
}

WRITE16_HANDLER( fromanc2_paletteram_0_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	UINT16 color;

	COMBINE_DATA(&state->m_paletteram[0][offset]);

	// GGGG_GRRR_RRBB_BBBx
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(space->machine(), 0x000 + color, pal5bit(data >> 6), pal5bit(data >> 11), pal5bit(data >> 1));
}

WRITE16_HANDLER( fromanc2_paletteram_1_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	UINT16 color;

	COMBINE_DATA(&state->m_paletteram[1][offset]);

	// GGGG_GRRR_RRBB_BBBx
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(space->machine(), 0x100 + color, pal5bit(data >> 6), pal5bit(data >> 11), pal5bit(data >> 1));
}


READ16_HANDLER( fromancr_paletteram_0_r )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	return state->m_paletteram[0][offset];
}

READ16_HANDLER( fromancr_paletteram_1_r )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	return state->m_paletteram[1][offset];
}

WRITE16_HANDLER( fromancr_paletteram_0_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	UINT16 color;

	COMBINE_DATA(&state->m_paletteram[0][offset]);

	// xGGG_GGRR_RRRB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(space->machine(), 0x000 + color, pal5bit(data >> 5), pal5bit(data >> 10), pal5bit(data >> 0));
}

WRITE16_HANDLER( fromancr_paletteram_1_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	UINT16 color;

	COMBINE_DATA(&state->m_paletteram[1][offset]);

	// xGGG_GGRR_RRRB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(space->machine(), 0x100 + color, pal5bit(data >> 5), pal5bit(data >> 10), pal5bit(data >> 0));
}


READ16_HANDLER( fromanc4_paletteram_0_r )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	return state->m_paletteram[0][offset];
}

READ16_HANDLER( fromanc4_paletteram_1_r )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	return state->m_paletteram[1][offset];
}

WRITE16_HANDLER( fromanc4_paletteram_0_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	UINT16 color;

	COMBINE_DATA(&state->m_paletteram[0][offset]);

	// xRRR_RRGG_GGGB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(space->machine(), 0x000 + color, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}

WRITE16_HANDLER( fromanc4_paletteram_1_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	UINT16 color;

	COMBINE_DATA(&state->m_paletteram[1][offset]);

	// xRRR_RRGG_GGGB_BBBB
	color = ((offset & 0x0700) << 1) + (offset & 0x00ff);
	palette_set_color_rgb(space->machine(), 0x100 + color, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
}


INLINE void fromanc2_dispvram_w( running_machine &machine, offs_t offset, UINT16 data, UINT16 mem_mask, int vram, int layer )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();
	layer += (offset < 0x1000) ? 0 : 1;

	COMBINE_DATA(&state->m_videoram[vram][layer][offset & 0x0fff]);
	tilemap_mark_tile_dirty(state->m_tilemap[vram][layer], offset & 0x0fff);
}

WRITE16_HANDLER( fromanc2_videoram_0_w ) { fromanc2_dispvram_w(space->machine(), offset, data, mem_mask, 0, 0); }
WRITE16_HANDLER( fromanc2_videoram_1_w ) { fromanc2_dispvram_w(space->machine(), offset, data, mem_mask, 0, 2); }
WRITE16_HANDLER( fromanc2_videoram_2_w ) { fromanc2_dispvram_w(space->machine(), offset, data, mem_mask, 1, 0); }
WRITE16_HANDLER( fromanc2_videoram_3_w ) { fromanc2_dispvram_w(space->machine(), offset, data, mem_mask, 1, 2); }

WRITE16_HANDLER( fromanc2_gfxreg_0_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	switch (offset)
	{
		case 0x00:	state->m_scrollx[0][0] = -(data - 0x000); break;
		case 0x01:	state->m_scrolly[0][0] = -(data - 0x000); break;
		case 0x02:	state->m_scrollx[0][1] = -(data - 0x004); break;
		case 0x03:	state->m_scrolly[0][1] = -(data - 0x000); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc2_gfxreg_1_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	switch (offset)
	{
		case 0x00:	state->m_scrollx[1][0] = -(data - 0x1be); break;
		case 0x01:	state->m_scrolly[1][0] = -(data - 0x1ef); break;
		case 0x02:	state->m_scrollx[1][1] = -(data - 0x1c2); break;
		case 0x03:	state->m_scrolly[1][1] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc2_gfxreg_2_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	switch (offset)
	{
		case 0x00:	state->m_scrollx[0][2] = -(data - 0x1c0); break;
		case 0x01:	state->m_scrolly[0][2] = -(data - 0x1ef); break;
		case 0x02:	state->m_scrollx[0][3] = -(data - 0x1c3); break;
		case 0x03:	state->m_scrolly[0][3] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc2_gfxreg_3_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	switch (offset)
	{
		case 0x00:	state->m_scrollx[1][2] = -(data - 0x1bf); break;
		case 0x01:	state->m_scrolly[1][2] = -(data - 0x1ef); break;
		case 0x02:	state->m_scrollx[1][3] = -(data - 0x1c3); break;
		case 0x03:	state->m_scrolly[1][3] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc2_gfxbank_0_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	state->m_gfxbank[0][0] = (data & 0x000f) >>  0;
	state->m_gfxbank[0][1] = (data & 0x00f0) >>  4;
	state->m_gfxbank[0][2] = (data & 0x0f00) >>  8;
	state->m_gfxbank[0][3] = (data & 0xf000) >> 12;
	tilemap_mark_all_tiles_dirty(state->m_tilemap[0][0]);
	tilemap_mark_all_tiles_dirty(state->m_tilemap[0][1]);
	tilemap_mark_all_tiles_dirty(state->m_tilemap[0][2]);
	tilemap_mark_all_tiles_dirty(state->m_tilemap[0][3]);
}

WRITE16_HANDLER( fromanc2_gfxbank_1_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	state->m_gfxbank[1][0] = (data & 0x000f) >>  0;
	state->m_gfxbank[1][1] = (data & 0x00f0) >>  4;
	state->m_gfxbank[1][2] = (data & 0x0f00) >>  8;
	state->m_gfxbank[1][3] = (data & 0xf000) >> 12;
	tilemap_mark_all_tiles_dirty(state->m_tilemap[1][0]);
	tilemap_mark_all_tiles_dirty(state->m_tilemap[1][1]);
	tilemap_mark_all_tiles_dirty(state->m_tilemap[1][2]);
	tilemap_mark_all_tiles_dirty(state->m_tilemap[1][3]);
}


INLINE void fromancr_vram_w(running_machine &machine, offs_t offset, UINT16 data, UINT16 mem_mask, int layer )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();
	int vram = (offset < 0x1000) ? 0 : 1;

	COMBINE_DATA(&state->m_videoram[vram][layer][offset & 0x0fff]);
	tilemap_mark_tile_dirty(state->m_tilemap[vram][layer], offset & 0x0fff);
}

WRITE16_HANDLER( fromancr_videoram_0_w ) { fromancr_vram_w(space->machine(), offset, data, mem_mask, 1); }
WRITE16_HANDLER( fromancr_videoram_1_w ) { fromancr_vram_w(space->machine(), offset, data, mem_mask, 0); }
WRITE16_HANDLER( fromancr_videoram_2_w ) { fromancr_vram_w(space->machine(), offset, data, mem_mask, 2); }

WRITE16_HANDLER( fromancr_gfxreg_0_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	switch (offset)
	{
		case 0x00:	state->m_scrollx[0][0] = -(data - 0x1bf); break;
		case 0x01:	state->m_scrolly[0][0] = -(data - 0x1ef); break;
		case 0x02:	state->m_scrollx[1][0] = -(data - 0x1c3); break;
		case 0x03:	state->m_scrolly[1][0] = -(data - 0x1ef); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromancr_gfxreg_1_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	switch (offset)
	{
		case 0x00:	state->m_scrollx[0][1] = -(data - 0x000); break;
		case 0x01:	state->m_scrolly[0][1] = -(data - 0x000); break;
		case 0x02:	state->m_scrollx[1][1] = -(data - 0x004); break;
		case 0x03:	state->m_scrolly[1][1] = -(data - 0x000); break;
		// offset 0x04 - 0x11 unknown
		default:	break;
	}
}

void fromancr_gfxbank_w( running_machine &machine, int data )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();

	state->m_gfxbank[0][0] = (data & 0x0010) >>  4;	// BG (1P)
	state->m_gfxbank[0][1] = (data & 0xf000) >> 12;	// FG (1P)
	state->m_gfxbank[1][0] = (data & 0x0008) >>  3;	// BG (2P)
	state->m_gfxbank[1][1] = (data & 0x0f00) >>  8;	// FG (2P)
	tilemap_mark_all_tiles_dirty(state->m_tilemap[0][0]);
	tilemap_mark_all_tiles_dirty(state->m_tilemap[0][1]);
	tilemap_mark_all_tiles_dirty(state->m_tilemap[1][0]);
	tilemap_mark_all_tiles_dirty(state->m_tilemap[1][1]);
}


INLINE void fromanc4_vram_w( running_machine &machine, offs_t offset, UINT16 data, UINT16 mem_mask, int layer )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();
	int vram = (offset < 0x4000) ? 0 : 1;

	COMBINE_DATA(&state->m_videoram[vram][layer][offset & 0x3fff]);
	tilemap_mark_tile_dirty(state->m_tilemap[vram][layer], offset & 0x3fff);
}

WRITE16_HANDLER( fromanc4_videoram_0_w ) { fromanc4_vram_w(space->machine(), offset, data, mem_mask, 2); }
WRITE16_HANDLER( fromanc4_videoram_1_w ) { fromanc4_vram_w(space->machine(), offset, data, mem_mask, 1); }
WRITE16_HANDLER( fromanc4_videoram_2_w ) { fromanc4_vram_w(space->machine(), offset, data, mem_mask, 0); }

WRITE16_HANDLER( fromanc4_gfxreg_0_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	switch (offset)
	{
		case 0x00:	state->m_scrollx[0][2] = -(data - 0xfbb); break;
		case 0x01:	state->m_scrolly[0][2] = -(data - 0x1e4); break;
		case 0x02:	state->m_scrollx[1][2] = -(data - 0xfbb); break;
		case 0x03:	state->m_scrolly[1][2] = -(data - 0x1e4); break;
		case 0x05:	state->m_gfxbank[0][2] = (data & 0x000f) >> 0;
				state->m_gfxbank[1][2] = (data & 0x0f00) >> 8;
				tilemap_mark_all_tiles_dirty(state->m_tilemap[0][2]);
				tilemap_mark_all_tiles_dirty(state->m_tilemap[1][2]);
				break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc4_gfxreg_1_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	switch (offset)
	{
		case 0x00:	state->m_scrollx[0][1] = -(data - 0xfba); break;
		case 0x01:	state->m_scrolly[0][1] = -(data - 0x1e4); break;
		case 0x02:	state->m_scrollx[1][1] = -(data - 0xfba); break;
		case 0x03:	state->m_scrolly[1][1] = -(data - 0x1e4); break;
		case 0x05:	state->m_gfxbank[0][1] = (data & 0x000f) >> 0;
				state->m_gfxbank[1][1] = (data & 0x0f00) >> 8;
				tilemap_mark_all_tiles_dirty(state->m_tilemap[0][1]);
				tilemap_mark_all_tiles_dirty(state->m_tilemap[1][1]);
				break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:	break;
	}
}

WRITE16_HANDLER( fromanc4_gfxreg_2_w )
{
	fromanc2_state *state = space->machine().driver_data<fromanc2_state>();
	switch (offset)
	{
		case 0x00:	state->m_scrollx[0][0] = -(data - 0xfbb); break;
		case 0x01:	state->m_scrolly[0][0] = -(data - 0x1e4); break;
		case 0x02:	state->m_scrollx[1][0] = -(data - 0xfbb); break;
		case 0x03:	state->m_scrolly[1][0] = -(data - 0x1e4); break;
		case 0x05:	state->m_gfxbank[0][0] = (data & 0x000f) >> 0;
				state->m_gfxbank[1][0] = (data & 0x0f00) >> 8;
				tilemap_mark_all_tiles_dirty(state->m_tilemap[0][0]);
				tilemap_mark_all_tiles_dirty(state->m_tilemap[1][0]);
				break;
		// offset 0x04, 0x06 - 0x11 unknown
		default:	break;
	}
}


/******************************************************************************

  Start the video hardware emulation.

******************************************************************************/

VIDEO_START( fromanc2 )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();

	state->m_tilemap[0][0] = tilemap_create(machine, fromanc2_get_v0_l0_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[0][1] = tilemap_create(machine, fromanc2_get_v0_l1_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[0][2] = tilemap_create(machine, fromanc2_get_v0_l2_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[0][3] = tilemap_create(machine, fromanc2_get_v0_l3_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[1][0] = tilemap_create(machine, fromanc2_get_v1_l0_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[1][1] = tilemap_create(machine, fromanc2_get_v1_l1_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[1][2] = tilemap_create(machine, fromanc2_get_v1_l2_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[1][3] = tilemap_create(machine, fromanc2_get_v1_l3_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	tilemap_set_transparent_pen(state->m_tilemap[0][1], 0x000);
	tilemap_set_transparent_pen(state->m_tilemap[0][2], 0x000);
	tilemap_set_transparent_pen(state->m_tilemap[0][3], 0x000);
	tilemap_set_transparent_pen(state->m_tilemap[1][1], 0x000);
	tilemap_set_transparent_pen(state->m_tilemap[1][2], 0x000);
	tilemap_set_transparent_pen(state->m_tilemap[1][3], 0x000);

	state->m_videoram[0][0] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[0][1] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[0][2] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[0][3] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[1][0] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[1][1] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[1][2] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[1][3] = auto_alloc_array(machine, UINT16, (64 * 64));

	state->m_paletteram[0] = auto_alloc_array(machine, UINT16, 0x800);
	state->m_paletteram[1] = auto_alloc_array(machine, UINT16, 0x800);

	state->save_pointer(NAME(state->m_videoram[0][0]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[0][1]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[0][2]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[0][3]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[1][0]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[1][1]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[1][2]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[1][3]), (64 * 64));
	state->save_pointer(NAME(state->m_paletteram[0]), 0x800);
	state->save_pointer(NAME(state->m_paletteram[1]), 0x800);
	state->save_item(NAME(state->m_scrollx[0]));
	state->save_item(NAME(state->m_scrollx[1]));
	state->save_item(NAME(state->m_scrolly[0]));
	state->save_item(NAME(state->m_scrolly[1]));
	state->save_item(NAME(state->m_gfxbank[0]));
	state->save_item(NAME(state->m_gfxbank[1]));
}

VIDEO_START( fromancr )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();

	state->m_tilemap[0][0] = tilemap_create(machine, fromancr_get_v0_l0_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[0][1] = tilemap_create(machine, fromancr_get_v0_l1_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[0][2] = tilemap_create(machine, fromancr_get_v0_l2_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[0][3] = 0;
	state->m_tilemap[1][0] = tilemap_create(machine, fromancr_get_v1_l0_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[1][1] = tilemap_create(machine, fromancr_get_v1_l1_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[1][2] = tilemap_create(machine, fromancr_get_v1_l2_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_tilemap[1][3] = 0;

	tilemap_set_transparent_pen(state->m_tilemap[0][1], 0x0ff);
	tilemap_set_transparent_pen(state->m_tilemap[0][2], 0x0ff);
	tilemap_set_transparent_pen(state->m_tilemap[1][1], 0x0ff);
	tilemap_set_transparent_pen(state->m_tilemap[1][2], 0x0ff);

	state->m_videoram[0][0] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[0][1] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[0][2] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[1][0] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[1][1] = auto_alloc_array(machine, UINT16, (64 * 64));
	state->m_videoram[1][2] = auto_alloc_array(machine, UINT16, (64 * 64));

	state->m_paletteram[0] = auto_alloc_array(machine, UINT16, 0x800);
	state->m_paletteram[1] = auto_alloc_array(machine, UINT16, 0x800);

	state->save_pointer(NAME(state->m_videoram[0][0]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[0][1]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[0][2]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[1][0]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[1][1]), (64 * 64));
	state->save_pointer(NAME(state->m_videoram[1][2]), (64 * 64));
	state->save_pointer(NAME(state->m_paletteram[0]), 0x800);
	state->save_pointer(NAME(state->m_paletteram[1]), 0x800);
	state->save_item(NAME(state->m_scrollx[0]));
	state->save_item(NAME(state->m_scrollx[1]));
	state->save_item(NAME(state->m_scrolly[0]));
	state->save_item(NAME(state->m_scrolly[1]));
	state->save_item(NAME(state->m_gfxbank[0]));
	state->save_item(NAME(state->m_gfxbank[1]));
}


VIDEO_START( fromanc4 )
{
	fromanc2_state *state = machine.driver_data<fromanc2_state>();

	state->m_tilemap[0][0] = tilemap_create(machine, fromancr_get_v0_l0_tile_info, tilemap_scan_rows, 8, 8, 256, 64);
	state->m_tilemap[0][1] = tilemap_create(machine, fromancr_get_v0_l1_tile_info, tilemap_scan_rows, 8, 8, 256, 64);
	state->m_tilemap[0][2] = tilemap_create(machine, fromancr_get_v0_l2_tile_info, tilemap_scan_rows, 8, 8, 256, 64);
	state->m_tilemap[0][3] = 0;
	state->m_tilemap[1][0] = tilemap_create(machine, fromancr_get_v1_l0_tile_info, tilemap_scan_rows, 8, 8, 256, 64);
	state->m_tilemap[1][1] = tilemap_create(machine, fromancr_get_v1_l1_tile_info, tilemap_scan_rows, 8, 8, 256, 64);
	state->m_tilemap[1][2] = tilemap_create(machine, fromancr_get_v1_l2_tile_info, tilemap_scan_rows, 8, 8, 256, 64);
	state->m_tilemap[1][3] = 0;

	tilemap_set_transparent_pen(state->m_tilemap[0][1], 0x000);
	tilemap_set_transparent_pen(state->m_tilemap[0][2], 0x000);
	tilemap_set_transparent_pen(state->m_tilemap[1][1], 0x000);
	tilemap_set_transparent_pen(state->m_tilemap[1][2], 0x000);

	state->m_videoram[0][0] = auto_alloc_array(machine, UINT16, (256 * 64));
	state->m_videoram[0][1] = auto_alloc_array(machine, UINT16, (256 * 64));
	state->m_videoram[0][2] = auto_alloc_array(machine, UINT16, (256 * 64));
	state->m_videoram[1][0] = auto_alloc_array(machine, UINT16, (256 * 64));
	state->m_videoram[1][1] = auto_alloc_array(machine, UINT16, (256 * 64));
	state->m_videoram[1][2] = auto_alloc_array(machine, UINT16, (256 * 64));

	state->m_paletteram[0] = auto_alloc_array(machine, UINT16, 0x800);
	state->m_paletteram[1] = auto_alloc_array(machine, UINT16, 0x800);

	state->save_pointer(NAME(state->m_videoram[0][0]), (256 * 64));
	state->save_pointer(NAME(state->m_videoram[0][1]), (256 * 64));
	state->save_pointer(NAME(state->m_videoram[0][2]), (256 * 64));
	state->save_pointer(NAME(state->m_videoram[1][0]), (256 * 64));
	state->save_pointer(NAME(state->m_videoram[1][1]), (256 * 64));
	state->save_pointer(NAME(state->m_videoram[1][2]), (256 * 64));
	state->save_pointer(NAME(state->m_paletteram[0]), 0x800);
	state->save_pointer(NAME(state->m_paletteram[1]), 0x800);
	state->save_item(NAME(state->m_scrollx[0]));
	state->save_item(NAME(state->m_scrollx[1]));
	state->save_item(NAME(state->m_scrolly[0]));
	state->save_item(NAME(state->m_scrolly[1]));
	state->save_item(NAME(state->m_gfxbank[0]));
	state->save_item(NAME(state->m_gfxbank[1]));
}

/******************************************************************************

  Display refresh

******************************************************************************/

SCREEN_UPDATE( fromanc2 )
{
	fromanc2_state *state = screen->machine().driver_data<fromanc2_state>();
	int i;
	int *scrollx = NULL;
	int *scrolly = NULL;
	tilemap_t **tilemaps = NULL;

	if (screen == state->m_left_screen)
	{
		tilemaps = state->m_tilemap[0];
		scrollx = state->m_scrollx[0];
		scrolly = state->m_scrolly[0];
	}
	else if (screen == state->m_right_screen)
	{
		tilemaps = state->m_tilemap[1];
		scrollx = state->m_scrollx[1];
		scrolly = state->m_scrolly[1];
	}

	for (i = 0; i < 4; i++)
	{
		if (tilemaps[i])
		{
			tilemap_set_scrollx(tilemaps[i], 0, -scrollx[i]);
			tilemap_set_scrolly(tilemaps[i], 0, -scrolly[i]);
			tilemap_draw(bitmap, cliprect, tilemaps[i], 0, 0);
		}
	}

	return 0;
}
