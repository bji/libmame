/******************************************************************************

Kikiippatsu Mayumi-chan (c) 1988 Victory L.L.C.

Video hardware
    driver by Uki

******************************************************************************/

#include "emu.h"

UINT8 *mayumi_videoram;
static tilemap_t *mayumi_tilemap;

static TILE_GET_INFO( get_tile_info )
{
	int code = mayumi_videoram[tile_index] + (mayumi_videoram[tile_index+0x800] & 0x1f)*0x100 ;
	int col = (mayumi_videoram[tile_index+0x1000] >> 3) & 0x1f;

	SET_TILE_INFO(0, code, col, 0);
}

VIDEO_START( mayumi )
{
	mayumi_videoram = auto_alloc_array(machine, UINT8, 0x1800);

	mayumi_tilemap = tilemap_create( machine, get_tile_info,tilemap_scan_rows,8,8,64,32 );
}

WRITE8_HANDLER( mayumi_videoram_w )
{
	mayumi_videoram[offset] = data;
	tilemap_mark_tile_dirty(mayumi_tilemap, offset & 0x7ff );
}

VIDEO_UPDATE( mayumi )
{
	tilemap_draw(bitmap, cliprect, mayumi_tilemap, 0, 0);
	return 0;
}

