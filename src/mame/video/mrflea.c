/******************************************************************

Mr. F. Lea
(C) 1983 PACIFIC NOVELTY MFG. INC.

******************************************************************/

#include "emu.h"

static int mrflea_gfx_bank;

WRITE8_HANDLER( mrflea_gfx_bank_w ){
	mrflea_gfx_bank = data;
	if( data & ~0x14 ){
		logerror( "unknown gfx bank: 0x%02x\n", data );
	}
}

WRITE8_HANDLER( mrflea_videoram_w ){
	int bank = offset/0x400;
	offset &= 0x3ff;
	space->machine->generic.videoram.u8[offset] = data;
	space->machine->generic.videoram.u8[offset+0x400] = bank;
	/*  the address range that tile data is written to sets one bit of
    **  the bank select.  The remaining bits are from a video register.
    */
}

WRITE8_HANDLER( mrflea_spriteram_w ){
	if( offset&2 ){ /* tile_number */
		space->machine->generic.spriteram.u8[offset|1] = offset&1;
		offset &= ~1;
	}
	space->machine->generic.spriteram.u8[offset] = data;
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	const gfx_element *gfx = machine->gfx[0];
	const UINT8 *source = machine->generic.spriteram.u8;
	const UINT8 *finish = source+0x100;
	rectangle clip = *video_screen_get_visible_area(machine->primary_screen);
	clip.max_x -= 24;
	clip.min_x += 16;
	while( source<finish ){
		int xpos = source[1]-3;
		int ypos = source[0]-16+3;
		int tile_number = source[2]+source[3]*0x100;

		drawgfx_transpen( bitmap, &clip,gfx,
			tile_number,
			0, /* color */
			0,0, /* no flip */
			xpos,ypos,0 );
		drawgfx_transpen( bitmap, &clip,gfx,
			tile_number,
			0, /* color */
			0,0, /* no flip */
			xpos,256+ypos,0 );
		source+=4;
	}
}

static void draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	const UINT8 *source = machine->generic.videoram.u8;
	const gfx_element *gfx = machine->gfx[1];
	int sx,sy;
	int base = 0;
	if( mrflea_gfx_bank&0x04 ) base |= 0x400;
	if( mrflea_gfx_bank&0x10 ) base |= 0x200;
	for( sy=0; sy<256; sy+=8 ){
		for( sx=0; sx<256; sx+=8 ){
			int tile_number = base+source[0]+source[0x400]*0x100;
			source++;
			drawgfx_opaque( bitmap, cliprect,
				gfx,
				tile_number,
				0, /* color */
				0,0, /* no flip */
				sx,sy );
		}
	}
}

VIDEO_START( mrflea ){
}

VIDEO_UPDATE( mrflea )
{
	draw_background(screen->machine, bitmap, cliprect);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
