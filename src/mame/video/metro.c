/***************************************************************************

                              -= Metro Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

                Q       Shows Layer 0
                W       Shows Layer 1
                E       Shows Layer 2
                A       Shows Sprites

        Keys can be used together!


                            [ 3 Scrolling Layers ]

        There is memory for a huge layer, but the actual tilemap
        is a smaller window (of fixed size) carved from anywhere
        inside that layer.

        Tile Size:                  8 x 8 x 4
        (later games can switch to  8 x 8 x 8, 16 x 16 x 4/8 at run time)

        Big Layer Size:         2048 x 2048 (8x8 tiles) or 4096 x 4096 (16x16 tiles)

        Tilemap Window Size:    512 x 256 (8x8 tiles) or 1024 x 512 (16x16 tiles)

        The tile codes in memory do not map directly to tiles. They
        are indexes into a table (with 0x200 entries) that defines
        a virtual set of tiles for the 3 layers. Each entry in that
        table adds 16 tiles to the set of available tiles, and decides
        their color code.

        Tile code with their msbit set are different as they mean:
        draw a tile filled with a single color (0-1ff)


                            [ 512 Zooming Sprites ]

        The sprites are NOT tile based: the "tile" size can vary from
        8 to 64 (independently for width and height) with an 8 pixel
        granularity. The "tile" address is a multiple of 8x8 pixels.

        Each sprite can be shrinked to ~1/4 or enlarged to ~32x following
        an exponential curve of sizes (with one zoom value for both width
        and height)


***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"

/* Variables that driver has access to: */

UINT16 *metro_videoregs;
UINT16 *metro_screenctrl;
UINT16 *metro_scroll;
UINT16 *metro_tiletable;
size_t metro_tiletable_size;
UINT16 *metro_vram_0,*metro_vram_1,*metro_vram_2;
UINT16 *metro_window;

static int support_8bpp,support_16x16;
static int has_zoom;


UINT16 *metro_K053936_ram;
static tilemap_t *metro_K053936_tilemap;

static UINT16 *metro_tiletable_old;
static UINT8 *dirtyindex;


static TILE_GET_INFO( metro_K053936_get_tile_info )
{
	int code = metro_K053936_ram[tile_index];

	SET_TILE_INFO(
			2,
			code & 0x7fff,
			0x1e,
			0);
}

static TILE_GET_INFO( metro_K053936_gstrik2_get_tile_info )
{
	int code = metro_K053936_ram[tile_index];

	SET_TILE_INFO(
			2,
			(code & 0x7fff)>>2,
			0x1e,
			0);
}

WRITE16_HANDLER( metro_K053936_w )
{
	COMBINE_DATA(&metro_K053936_ram[offset]);
	tilemap_mark_tile_dirty(metro_K053936_tilemap,offset);
}

static TILEMAP_MAPPER( tilemap_scan_gstrik2 )
{
	/* logical (col,row) -> memory offset */
	int val;

	val = (row&0x3f)*(256*2) + (col*2);

	if (row&0x40) val+=1;
	if (row&0x80) val+=256;

	return val;
}


/***************************************************************************


                            Palette GGGGGRRRRRBBBBBx


***************************************************************************/

#ifdef UNUSED_FUNCTION
WRITE16_HANDLER( metro_paletteram_w )
{
	data = COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	palette_set_color_rgb(space->machine,offset,pal5bit(data >> 6),pal5bit(data >> 11),pal5bit(data >> 1));
}
#endif


/***************************************************************************


                        Tilemaps: Tiles Set & Window

    Each entry in the Tiles Set RAM uses 2 words to specify a starting
    tile code and a color code. This adds 16 consecutive tiles with
    that color code to the set of available tiles.

        Offset:     Bits:                   Value:

        0.w         fedc ---- ---- ----
                    ---- ba98 7654 ----     Color Code
                    ---- ---- ---- 3210     Code High Bits

        2.w                                 Code Low Bits


***************************************************************************/


/***************************************************************************


                            Tilemaps: Rendering


***************************************************************************/

static tilemap_t *bg_tilemap[3];
static tilemap_t *tilemap_16x16[3];
static UINT8 *empty_tiles;

/* A 2048 x 2048 virtual tilemap */

#define BIG_NX		(0x100)
#define BIG_NY		(0x100)

/* A smaller 512 x 256 window defines the actual tilemap */

#define WIN_NX		(0x40)
#define WIN_NY		(0x20)
//#define WIN_NX        (0x40+1)
//#define WIN_NY        (0x20+1)


/* 8x8x4 tiles only */
INLINE void get_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,int layer,UINT16 *vram)
{
	UINT16 code;
	int      table_index;
	UINT32   tile;

	/* The actual tile index depends on the window */
	tile_index	=	((tile_index / WIN_NX + metro_window[layer * 2 + 0] / 8) % BIG_NY) * BIG_NX +
					((tile_index % WIN_NX + metro_window[layer * 2 + 1] / 8) % BIG_NX);

	/* Fetch the code */
	code			=	vram[ tile_index ];

	/* Use it as an index into the tiles set table */
	table_index		=	( (code & 0x1ff0) >> 4 ) * 2;
	tile			=	(metro_tiletable[table_index + 0] << 16 ) +
						 metro_tiletable[table_index + 1];

	if (code & 0x8000) /* Special: draw a tile of a single color (i.e. not from the gfx ROMs) */
	{
		int _code = code & 0x000f;
		tileinfo->pen_data = empty_tiles + _code*16*16;
		tileinfo->palette_base = ((code & 0x0ff0)) + 0x1000;
		tileinfo->flags = 0;
		tileinfo->group = 0;
	}
	else
	{
		tileinfo->group = 0;
		SET_TILE_INFO(
				0,
				(tile & 0xfffff) + (code & 0xf),
				(((tile & 0x0ff00000) >> 20)) + 0x100,
				TILE_FLIPXY((code & 0x6000) >> 13));
	}
}


/* 8x8x4 or 8x8x8 tiles. It's the tile's color that decides: if its low 4
   bits are high ($f,$1f,$2f etc) the tile is 8bpp, otherwise it's 4bpp */
INLINE void get_tile_info_8bit(running_machine *machine,tile_data *tileinfo,int tile_index,int layer,UINT16 *vram)
{
	UINT16 code;
	int      table_index;
	UINT32   tile;

	/* The actual tile index depends on the window */
	tile_index	=	((tile_index / WIN_NX + metro_window[layer * 2 + 0] / 8) % BIG_NY) * BIG_NX +
					((tile_index % WIN_NX + metro_window[layer * 2 + 1] / 8) % BIG_NX);

	/* Fetch the code */
	code			=	vram[ tile_index ];

	/* Use it as an index into the tiles set table */
	table_index		=	( (code & 0x1ff0) >> 4 ) * 2;
	tile			=	(metro_tiletable[table_index + 0] << 16 ) +
						 metro_tiletable[table_index + 1];

	if (code & 0x8000) /* Special: draw a tile of a single color (i.e. not from the gfx ROMs) */
	{
		int _code = code & 0x000f;
		tileinfo->pen_data = empty_tiles + _code*16*16;
		tileinfo->palette_base = ((code & 0x0ff0)) + 0x1000;
		tileinfo->flags = 0;
		tileinfo->group = 0;
	}
	else if ((tile & 0x00f00000)==0x00f00000)	/* draw tile as 8bpp */
	{
		tileinfo->group = 1;
		SET_TILE_INFO(
				1,
				(tile & 0xfffff) + 2*(code & 0xf),
				((tile & 0x0f000000) >> 24) + 0x10,
				TILE_FLIPXY((code & 0x6000) >> 13));
	}
	else
	{
		tileinfo->group = 0;
		SET_TILE_INFO(
				0,
				(tile & 0xfffff) + (code & 0xf),
				(((tile & 0x0ff00000) >> 20)) + 0x100,
				TILE_FLIPXY((code & 0x6000) >> 13));
	}
}

/* 16x16x4 or 16x16x8 tiles. It's the tile's color that decides: if its low 4
   bits are high ($f,$1f,$2f etc) the tile is 8bpp, otherwise it's 4bpp */
INLINE void get_tile_info_16x16_8bit(running_machine *machine,tile_data *tileinfo,int tile_index,int layer,UINT16 *vram)
{
	UINT16 code;
	int      table_index;
	UINT32   tile;

	/* The actual tile index depends on the window */
	tile_index	=	((tile_index / WIN_NX + metro_window[layer * 2 + 0] / 8) % BIG_NY) * BIG_NX +
					((tile_index % WIN_NX + metro_window[layer * 2 + 1] / 8) % BIG_NX);

	/* Fetch the code */
	code			=	vram[ tile_index ];

	/* Use it as an index into the tiles set table */
	table_index		=	( (code & 0x1ff0) >> 4 ) * 2;
	tile			=	(metro_tiletable[table_index + 0] << 16 ) +
						 metro_tiletable[table_index + 1];

	if (code & 0x8000) /* Special: draw a tile of a single color (i.e. not from the gfx ROMs) */
	{
		int _code = code & 0x000f;
		tileinfo->pen_data = empty_tiles + _code*16*16;
		tileinfo->palette_base = ((code & 0x0ff0)) + 0x1000;
		tileinfo->flags = 0;
		tileinfo->group = 0;
	}
	else if ((tile & 0x00f00000)==0x00f00000)	/* draw tile as 8bpp */
	{
		tileinfo->group = 1;
		SET_TILE_INFO(
				3,
				(tile & 0xfffff) + 8*(code & 0xf),
				((tile & 0x0f000000) >> 24) + 0x10,
				TILE_FLIPXY((code & 0x6000) >> 13));
	}
	else
	{
		tileinfo->group = 0;
		SET_TILE_INFO(
				2,
				(tile & 0xfffff) + 4*(code & 0xf),
				(((tile & 0x0ff00000) >> 20)) + 0x100,
				TILE_FLIPXY((code & 0x6000) >> 13));

	}
}


INLINE void metro_vram_w(offs_t offset,UINT16 data,UINT16 mem_mask,int layer,UINT16 *vram)
{
	COMBINE_DATA(&vram[offset]);
	{
		/* Account for the window */
		int col		=	(offset % BIG_NX) - ((metro_window[layer * 2 + 1] / 8) % BIG_NX);
		int row		=	(offset / BIG_NX) - ((metro_window[layer * 2 + 0] / 8) % BIG_NY);
		if (col < -(BIG_NX-WIN_NX))	col += (BIG_NX-WIN_NX) + WIN_NX;
		if (row < -(BIG_NY-WIN_NY))	row += (BIG_NY-WIN_NY) + WIN_NY;
		if	( (col >= 0) && (col < WIN_NX) &&
			  (row >= 0) && (row < WIN_NY) )
		{
			tilemap_mark_tile_dirty(bg_tilemap[layer], row * WIN_NX + col );
			if (tilemap_16x16[layer])
				tilemap_mark_tile_dirty(tilemap_16x16[layer], row * WIN_NX + col );
		}
	}
}



static TILE_GET_INFO( get_tile_info_0 ) { get_tile_info(machine,tileinfo,tile_index,0,metro_vram_0); }
static TILE_GET_INFO( get_tile_info_1 ) { get_tile_info(machine,tileinfo,tile_index,1,metro_vram_1); }
static TILE_GET_INFO( get_tile_info_2 ) { get_tile_info(machine,tileinfo,tile_index,2,metro_vram_2); }

static TILE_GET_INFO( get_tile_info_0_8bit ) { get_tile_info_8bit(machine,tileinfo,tile_index,0,metro_vram_0); }
static TILE_GET_INFO( get_tile_info_1_8bit ) { get_tile_info_8bit(machine,tileinfo,tile_index,1,metro_vram_1); }
static TILE_GET_INFO( get_tile_info_2_8bit ) { get_tile_info_8bit(machine,tileinfo,tile_index,2,metro_vram_2); }

static TILE_GET_INFO( get_tile_info_0_16x16_8bit ) { get_tile_info_16x16_8bit(machine,tileinfo,tile_index,0,metro_vram_0); }
static TILE_GET_INFO( get_tile_info_1_16x16_8bit ) { get_tile_info_16x16_8bit(machine,tileinfo,tile_index,1,metro_vram_1); }
static TILE_GET_INFO( get_tile_info_2_16x16_8bit ) { get_tile_info_16x16_8bit(machine,tileinfo,tile_index,2,metro_vram_2); }

WRITE16_HANDLER( metro_vram_0_w ) { metro_vram_w(offset,data,mem_mask,0,metro_vram_0); }
WRITE16_HANDLER( metro_vram_1_w ) { metro_vram_w(offset,data,mem_mask,1,metro_vram_1); }
WRITE16_HANDLER( metro_vram_2_w ) { metro_vram_w(offset,data,mem_mask,2,metro_vram_2); }



/* Dirty the relevant tilemap when its window changes */
WRITE16_HANDLER( metro_window_w )
{
	UINT16 olddata = metro_window[offset];
	UINT16 newdata = COMBINE_DATA( &metro_window[offset] );
	if ( newdata != olddata )
	{
		offset /= 2;
		tilemap_mark_all_tiles_dirty(bg_tilemap[offset]);
		if (tilemap_16x16[offset]) tilemap_mark_all_tiles_dirty(tilemap_16x16[offset]);
	}
}



/***************************************************************************


                            Video Init Routines


***************************************************************************/

/*
 Sprites are not tile based, so we decode their graphics at runtime.

 We can't do it at startup because drawgfx requires the tiles to be
 pre-rotated to support vertical games, and that, in turn, requires
 the tile's sizes to be known at startup - which we don't!
*/

static int metro_sprite_xoffs, metro_sprite_yoffs;


static void alloc_empty_tiles(running_machine *machine)
{
	int code,i;

	empty_tiles = auto_alloc_array(machine, UINT8, 16*16*16);

	for (code = 0;code < 0x10;code++)
		for (i = 0;i < 16*16;i++)
			empty_tiles[16*16*code + i] = code;
}

VIDEO_START( metro_14100 )
{
	support_8bpp = 0;
	support_16x16 = 0;
	has_zoom = 0;

	alloc_empty_tiles(machine);
	metro_tiletable_old = auto_alloc_array(machine, UINT16, metro_tiletable_size/2);
	dirtyindex = auto_alloc_array(machine, UINT8, metro_tiletable_size/4);

	bg_tilemap[0] = tilemap_create(machine, get_tile_info_0,tilemap_scan_rows,8,8,WIN_NX,WIN_NY);
	bg_tilemap[1] = tilemap_create(machine, get_tile_info_1,tilemap_scan_rows,8,8,WIN_NX,WIN_NY);
	bg_tilemap[2] = tilemap_create(machine, get_tile_info_2,tilemap_scan_rows,8,8,WIN_NX,WIN_NY);

	tilemap_16x16[0] = NULL;
	tilemap_16x16[1] = NULL;
	tilemap_16x16[2] = NULL;

	tilemap_map_pen_to_layer(bg_tilemap[0], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(bg_tilemap[0], 1, 255, TILEMAP_PIXEL_TRANSPARENT);

	tilemap_map_pen_to_layer(bg_tilemap[1], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(bg_tilemap[1], 1, 255, TILEMAP_PIXEL_TRANSPARENT);

	tilemap_map_pen_to_layer(bg_tilemap[2], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(bg_tilemap[2], 1, 255, TILEMAP_PIXEL_TRANSPARENT);

}

VIDEO_START( metro_14220 )
{
	support_8bpp = 1;
	support_16x16 = 0;
	has_zoom = 0;

	alloc_empty_tiles(machine);
	metro_tiletable_old = auto_alloc_array(machine, UINT16, metro_tiletable_size/2);
	dirtyindex = auto_alloc_array(machine, UINT8, metro_tiletable_size/4);

	bg_tilemap[0] = tilemap_create(machine, get_tile_info_0_8bit,tilemap_scan_rows,8,8,WIN_NX,WIN_NY);
	bg_tilemap[1] = tilemap_create(machine, get_tile_info_1_8bit,tilemap_scan_rows,8,8,WIN_NX,WIN_NY);
	bg_tilemap[2] = tilemap_create(machine, get_tile_info_2_8bit,tilemap_scan_rows,8,8,WIN_NX,WIN_NY);

	tilemap_16x16[0] = NULL;
	tilemap_16x16[1] = NULL;
	tilemap_16x16[2] = NULL;

	tilemap_map_pen_to_layer(bg_tilemap[0], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(bg_tilemap[0], 1, 255, TILEMAP_PIXEL_TRANSPARENT);

	tilemap_map_pen_to_layer(bg_tilemap[1], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(bg_tilemap[1], 1, 255, TILEMAP_PIXEL_TRANSPARENT);

	tilemap_map_pen_to_layer(bg_tilemap[2], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(bg_tilemap[2], 1, 255, TILEMAP_PIXEL_TRANSPARENT);

	tilemap_set_scrolldx(bg_tilemap[0], -2, 2);
	tilemap_set_scrolldx(bg_tilemap[1], -2, 2);
	tilemap_set_scrolldx(bg_tilemap[2], -2, 2);
}

VIDEO_START( metro_14300 )
{
	support_8bpp = 1;
	support_16x16 = 1;
	has_zoom = 0;

	alloc_empty_tiles(machine);
	metro_tiletable_old = auto_alloc_array(machine, UINT16, metro_tiletable_size/2);
	dirtyindex = auto_alloc_array(machine, UINT8, metro_tiletable_size/4);

	bg_tilemap[0] = tilemap_create(machine, get_tile_info_0_8bit,tilemap_scan_rows,8,8,WIN_NX,WIN_NY);
	bg_tilemap[1] = tilemap_create(machine, get_tile_info_1_8bit,tilemap_scan_rows,8,8,WIN_NX,WIN_NY);
	bg_tilemap[2] = tilemap_create(machine, get_tile_info_2_8bit,tilemap_scan_rows,8,8,WIN_NX,WIN_NY);

	tilemap_16x16[0] = tilemap_create(machine, get_tile_info_0_16x16_8bit,tilemap_scan_rows,16,16,WIN_NX,WIN_NY);
	tilemap_16x16[1] = tilemap_create(machine, get_tile_info_1_16x16_8bit,tilemap_scan_rows,16,16,WIN_NX,WIN_NY);
	tilemap_16x16[2] = tilemap_create(machine, get_tile_info_2_16x16_8bit,tilemap_scan_rows,16,16,WIN_NX,WIN_NY);

	tilemap_map_pen_to_layer(bg_tilemap[0], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(bg_tilemap[0], 1, 255, TILEMAP_PIXEL_TRANSPARENT);

	tilemap_map_pen_to_layer(bg_tilemap[1], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(bg_tilemap[1], 1, 255, TILEMAP_PIXEL_TRANSPARENT);

	tilemap_map_pen_to_layer(bg_tilemap[2], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(bg_tilemap[2], 1, 255, TILEMAP_PIXEL_TRANSPARENT);


	tilemap_map_pen_to_layer(tilemap_16x16[0], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(tilemap_16x16[0], 1, 255, TILEMAP_PIXEL_TRANSPARENT);

	tilemap_map_pen_to_layer(tilemap_16x16[1], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(tilemap_16x16[1], 1, 255, TILEMAP_PIXEL_TRANSPARENT);

	tilemap_map_pen_to_layer(tilemap_16x16[2], 0, 15,  TILEMAP_PIXEL_TRANSPARENT);
	tilemap_map_pen_to_layer(tilemap_16x16[2], 1, 255, TILEMAP_PIXEL_TRANSPARENT);

}

VIDEO_START( blzntrnd )
{
	VIDEO_START_CALL(metro_14220);

	has_zoom = 1;

	metro_K053936_tilemap = tilemap_create(machine, metro_K053936_get_tile_info, tilemap_scan_rows, 8,8, 256, 512 );

	tilemap_set_scrolldx(bg_tilemap[0], 8, -8);
	tilemap_set_scrolldx(bg_tilemap[1], 8, -8);
	tilemap_set_scrolldx(bg_tilemap[2], 8, -8);
}

VIDEO_START( gstrik2 )
{
	VIDEO_START_CALL(metro_14220);

	has_zoom = 1;

	metro_K053936_tilemap = tilemap_create(machine, metro_K053936_gstrik2_get_tile_info, tilemap_scan_gstrik2, 16,16, 128, 256 );

	tilemap_set_scrolldx(bg_tilemap[0], 8, -8);
	tilemap_set_scrolldx(bg_tilemap[1], 0, 0);
	tilemap_set_scrolldx(bg_tilemap[2], 8, -8);
}

/***************************************************************************

                                Video Registers


        Offset:     Bits:                   Value:

        0.w                                 Number Of Sprites To Draw
        2.w         f--- ---- ---- ----     Disable Sprites Layer Priority
                    -edc ---- ---- ----
                    ---- ba-- ---- ----     Sprites Masked Layer
                    ---- --98 ---- ----     Sprites Priority
                    ---- ---- 765- ----
                    ---- ---- ---4 3210     Sprites Masked Number
        4.w                                 Sprites Y Offset
        6.w                                 Sprites X Offset
        8.w                                 Sprites Color Codes Start

        -

        10.w        fedc ba98 76-- ----
                    ---- ---- --54 ----     Layer 2 Priority (3 backmost, 0 frontmost)
                    ---- ---- ---- 32--     Layer 1 Priority
                    ---- ---- ---- --10     Layer 0 Priority

        12.w                                Backround Color


***************************************************************************/



/***************************************************************************


                                Sprites Drawing


        Offset:     Bits:                   Value:

        0.w         fedc b--- ---- ----     Priority (0 = Max)
                    ---- -a98 7654 3210     X

        2.w         fedc ba-- ---- ----     Zoom (Both X & Y)
                    ---- --98 7654 3210     Y

        4.w         f--- ---- ---- ----     Flip X
                    -e-- ---- ---- ----     Flip Y
                    --dc b--- ---- ----     Size X *
                    ---- -a98 ---- ----     Size Y *
                    ---- ---- 7654 ----     Color
                    ---- ---- ---- 3210     Code High Bits **

        6.w                                 Code Low Bits  **

*  8 pixel increments
** 8x8 pixel increments

***************************************************************************/

/* Draw sprites */

void metro_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *base_gfx	=	memory_region(machine, "gfx1");
	UINT8 *gfx_max	=	base_gfx + memory_region_length(machine, "gfx1");

	int max_x = video_screen_get_width(machine->primary_screen);
	int max_y = video_screen_get_height(machine->primary_screen);

	int max_sprites			=	machine->generic.spriteram_size / 8;
	int sprites				=	metro_videoregs[0x00/2] % max_sprites;

	int color_start			=	((metro_videoregs[0x08/2] & 0xf) << 4 ) + 0x100;

	int i, j, pri;
	static const int primask[4] = { 0x0000, 0xff00, 0xff00|0xf0f0, 0xff00|0xf0f0|0xcccc };

	UINT16 *src;
	int inc;

	if (sprites == 0)
		return;

	for (i=0; i<0x20; i++)
	{
		gfx_element gfx;

		if (!(metro_videoregs[0x02/2] & 0x8000))
		{
			src = machine->generic.spriteram.u16 + (sprites - 1) * (8/2);
			inc = -(8/2);
		} else {
			src = machine->generic.spriteram.u16;
			inc = (8/2);
		}

		for (j=0; j<sprites; j++)
		{
			int x,y, attr,code,color,flipx,flipy, zoom, curr_pri,width,height;
			UINT8 *gfxdata;

			/* Exponential zoom table extracted from daitoride */
			static const int zoomtable[0x40] =
			{	0xAAC,0x800,0x668,0x554,0x494,0x400,0x390,0x334,
				0x2E8,0x2AC,0x278,0x248,0x224,0x200,0x1E0,0x1C8,
				0x1B0,0x198,0x188,0x174,0x164,0x154,0x148,0x13C,
				0x130,0x124,0x11C,0x110,0x108,0x100,0x0F8,0x0F0,
				0x0EC,0x0E4,0x0DC,0x0D8,0x0D4,0x0CC,0x0C8,0x0C4,
				0x0C0,0x0BC,0x0B8,0x0B4,0x0B0,0x0AC,0x0A8,0x0A4,
				0x0A0,0x09C,0x098,0x094,0x090,0x08C,0x088,0x080,
				0x078,0x070,0x068,0x060,0x058,0x050,0x048,0x040	};

			x					=	src[ 0 ];
			curr_pri			=	(x & 0xf800) >> 11;

			if ((curr_pri == 0x1f) || (curr_pri != i))
			{
				src += inc;
				continue;
			}

			pri = (metro_videoregs[0x02/2] & 0x0300) >> 8;

			if (!(metro_videoregs[0x02/2] & 0x8000))
			{
				if (curr_pri > (metro_videoregs[0x02/2] & 0x1f))
					pri = (metro_videoregs[0x02/2] & 0x0c00) >> 10;
			}

			y					=	src[ 1 ];
			attr				=	src[ 2 ];
			code				=	src[ 3 ];

			flipx				=	attr & 0x8000;
			flipy				=	attr & 0x4000;
			color				=   (attr & 0xf0) >> 4;

			zoom				=	zoomtable[(y & 0xfc00) >> 10] << (16-8);

			x					=	(x & 0x07ff) - metro_sprite_xoffs;
			y					=	(y & 0x03ff) - metro_sprite_yoffs;

			width				= (( (attr >> 11) & 0x7 ) + 1 ) * 8;
			height				= (( (attr >>  8) & 0x7 ) + 1 ) * 8;

			gfxdata		=	base_gfx + (8*8*4/8) * (((attr & 0x000f) << 16) + code);

			if (flip_screen_get(machine))
			{
				flipx = !flipx;		x = max_x - x - width;
				flipy = !flipy;		y = max_y - y - height;
			}

			if (support_8bpp && color == 0xf)	/* 8bpp */
			{
				/* Bounds checking */
				if ( (gfxdata + width * height - 1) >= gfx_max )
					continue;

				gfx_element_build_temporary(&gfx, machine, gfxdata, width, height, width, 0, 256, 0);

				pdrawgfxzoom_transpen(	bitmap,cliprect, &gfx,
								0,
								color_start >> 4,
								flipx, flipy,
								x, y,
								zoom, zoom,
								machine->priority_bitmap,primask[pri], 255);
			}
			else
			{
				/* Bounds checking */
				if ( (gfxdata + width/2 * height - 1) >= gfx_max )
					continue;

				gfx_element_build_temporary(&gfx, machine, gfxdata, width, height, width/2, 0, 16, GFX_ELEMENT_PACKED);

				pdrawgfxzoom_transpen(	bitmap,cliprect, &gfx,
								0,
								color + color_start,
								flipx, flipy,
								x, y,
								zoom, zoom,
								machine->priority_bitmap,primask[pri], 15);
			}
#if 0
{	/* Display priority + zoom on each sprite */
	char buf[80];
	sprintf(buf, "%02X %02X",((src[ 0 ] & 0xf800) >> 11)^0x1f,((src[ 1 ] & 0xfc00) >> 10) );
	ui_draw_text(buf, x, y);
}
#endif
			src += inc;
		}
	}
}



/***************************************************************************


                                Screen Drawing


***************************************************************************/

static void draw_tilemap(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, tilemap_t *tmap, UINT32 flags, UINT32 priority,
						 int sx, int sy, int wx, int wy)	// scroll & window values
{
#if 1
		tilemap_set_scrollx(tmap, 0, sx - wx + (wx & 7));
		tilemap_set_scrolly(tmap, 0, sy - wy + (wy & 7));
		tilemap_draw(bitmap,cliprect,tmap, flags, priority);
#else
	int x,y,i;
	const rectangle *visarea = video_screen_get_visible_area(machine->primary_screen);

	/* sub tile placement */
//  sx      =   sx - (wx & ~7) + (wx & 7);
	sx		=	sx - wx;
	sx		=	( (sx & 0x7fff) - (sx & 0x8000) ) % ((WIN_NX-1)*8);

//  sy      =   sy - (wy & ~7) + (wy & 7);
	sy		=	sy - wy;
	sy		=	( (sy & 0x7fff) - (sy & 0x8000) ) % ((WIN_NY-1)*8);

	/* splitting point */
	x		=	(WIN_NX-1)*8 - sx;

	y		=	(WIN_NY-1)*8 - sy;

	for ( i = 0; i < 4 ; i++ )
	{
		rectangle clip;

		tilemap_set_scrollx(tmap, 0, sx + ((i & 1) ? -x : 0));
		tilemap_set_scrolly(tmap, 0, sy + ((i & 2) ? -y : 0));

		clip.min_x	=	x - ((i & 1) ? 0 : (WIN_NX-1)*8);
		clip.min_y	=	y - ((i & 2) ? 0 : (WIN_NY-1)*8);

		clip.max_x	=	clip.min_x + (WIN_NX-1)*8 - 1;
		clip.max_y	=	clip.min_y + (WIN_NY-1)*8 - 1;

		if (clip.min_x > visarea->max_x)	continue;
		if (clip.min_y > visarea->max_y)	continue;

		if (clip.max_x < visarea->min_x)	continue;
		if (clip.max_y < visarea->min_y)	continue;

		if (clip.min_x < visarea->min_x)	clip.min_x = visarea->min_x;
		if (clip.max_x > visarea->max_x)	clip.max_x = visarea->max_x;

		if (clip.min_y < visarea->min_y)	clip.min_y = visarea->min_y;
		if (clip.max_y > visarea->max_y)	clip.max_y = visarea->max_y;

		/* The clip region's width must be a multiple of 8!
           This fact renderes the function useless, as far as
           we are concerned! */
		tilemap_set_clip(tmap, &clip);
		tilemap_draw(bitmap,cliprect,tmap, flags, priority);
	}
#endif
}


/* Draw all the layers that match the given priority */
static void draw_layers(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pri, int layers_ctrl)
{
	UINT16 layers_pri = metro_videoregs[0x10/2];
	int layer;

	/* Draw all the layers with priority == pri */
	for (layer = 2; layer >= 0; layer--)	// tilemap[2] below?
	{
		if ( pri == ((layers_pri >> (layer*2)) & 3) )
		{
			/* Scroll and Window values */
			UINT16 sy = metro_scroll[layer * 2 + 0];	UINT16 sx = metro_scroll[layer * 2 + 1];
			UINT16 wy = metro_window[layer * 2 + 0];	UINT16 wx = metro_window[layer * 2 + 1];

			if (layers_ctrl & (1<<layer))	// for debug
			{
				/* Only *one* of tilemap_16x16 & tilemap is enabled at any given time! */
				draw_tilemap(machine, bitmap,cliprect,bg_tilemap[layer], 0, 1<<(3-pri), sx, sy, wx, wy);
				if (tilemap_16x16[layer]) draw_tilemap(machine, bitmap,cliprect,tilemap_16x16[layer], 0, 1<<(3-pri), sx, sy, wx, wy);
			}
		}
	}
}



/* Dirty tilemaps when the tiles set changes */
static void dirty_tiles(int layer,UINT16 *vram)
{
	int col,row;

	for (row = 0;row < WIN_NY;row++)
	{
		for (col = 0;col < WIN_NX;col++)
		{
			int offset = (col + metro_window[layer * 2 + 1] / 8) % BIG_NX +
						((row + metro_window[layer * 2 + 0] / 8) % BIG_NY) * BIG_NX;
			UINT16 code = vram[offset];

			if (!(code & 0x8000) && dirtyindex[(code & 0x1ff0) >> 4])
			{
				tilemap_mark_tile_dirty(bg_tilemap[layer], row * WIN_NX + col );
				if (tilemap_16x16[layer])
					tilemap_mark_tile_dirty(tilemap_16x16[layer], row * WIN_NX + col );
			}
		}
	}
}


VIDEO_UPDATE( metro )
{
	running_device *k053936 = devtag_get_device(screen->machine, "k053936");
	int i,pri,layers_ctrl = -1;
	UINT16 screenctrl = *metro_screenctrl;

	{
		int dirty = 0;

		memset(dirtyindex,0,metro_tiletable_size/4);
		for (i = 0;i < metro_tiletable_size/4;i++)
		{
			UINT32 tile_new = (metro_tiletable[2*i + 0] << 16 ) + metro_tiletable[2*i + 1];
			UINT32 tile_old = (metro_tiletable_old[2*i + 0] << 16 ) + metro_tiletable_old[2*i + 1];

			if ((tile_new ^ tile_old) & 0x0fffffff)
			{
				dirtyindex[i] = 1;
				dirty = 1;
			}
		}
		memcpy(metro_tiletable_old,metro_tiletable,metro_tiletable_size);

		if (dirty)
		{
			dirty_tiles(0,metro_vram_0);
			dirty_tiles(1,metro_vram_1);
			dirty_tiles(2,metro_vram_2);
		}
	}

	metro_sprite_xoffs	=	metro_videoregs[0x06/2] - video_screen_get_width(screen)  / 2;
	metro_sprite_yoffs	=	metro_videoregs[0x04/2] - video_screen_get_height(screen) / 2;

	/* The background color is selected by a register */
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,((metro_videoregs[0x12/2] & 0x0fff)) + 0x1000);

	/*  Screen Control Register:

        f--- ---- ---- ----     ?
        -edc b--- ---- ----
        ---- -a98 ---- ----     ? Leds
        ---- ---- 7--- ----     16x16 Tiles (Layer 2)
        ---- ---- -6-- ----     16x16 Tiles (Layer 1)
        ---- ---- --5- ----     16x16 Tiles (Layer 0)
        ---- ---- ---4 32--
        ---- ---- ---- --1-     ? Blank Screen
        ---- ---- ---- ---0     Flip  Screen    */
	if (screenctrl & 2)	return 0;
	flip_screen_set(screen->machine, screenctrl & 1);

	/* If the game supports 16x16 tiles, make sure that the
       16x16 and 8x8 tilemaps of a given layer are not simultaneously
       enabled! */
	if (support_16x16)
	{
		int layer;

		for (layer = 0;layer < 3;layer++)
		{
			int big = screenctrl & (0x0020 << layer);

			tilemap_set_enable(bg_tilemap[layer],!big);
			tilemap_set_enable(tilemap_16x16[layer],big);
		}
	}


#ifdef MAME_DEBUG
if (input_code_pressed(screen->machine, KEYCODE_Z))
{
	int msk = 0;
	if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= 2;
	if (input_code_pressed(screen->machine, KEYCODE_E))	msk |= 4;
	if (input_code_pressed(screen->machine, KEYCODE_A))	msk |= 8;
	if (msk != 0)
	{
		bitmap_fill(bitmap,cliprect,0);
		layers_ctrl &= msk;
	}

	popmessage("l %x-%x-%x r %04x %04x %04x",
				(metro_videoregs[0x10/2]&0x30)>>4,(metro_videoregs[0x10/2]&0xc)>>2,metro_videoregs[0x10/2]&3,
				metro_videoregs[0x02/2], metro_videoregs[0x12/2],
				*metro_screenctrl);
}
#endif

	if (has_zoom)
		k053936_zoom_draw(k053936, bitmap, cliprect, metro_K053936_tilemap, 0, 0, 1);

	for (pri = 3; pri >= 0; pri--)
		draw_layers(screen->machine, bitmap,cliprect,pri,layers_ctrl);

	if (layers_ctrl & 0x08)
		metro_draw_sprites(screen->machine, bitmap,cliprect);
	return 0;
}
