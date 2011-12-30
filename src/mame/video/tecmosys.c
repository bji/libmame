/***************************************************************************

 tecmosys video driver

***************************************************************************/

#include "emu.h"
#include "includes/tecmosys.h"


static TILE_GET_INFO( get_bg0tile_info )
{
	tecmosys_state *state = machine.driver_data<tecmosys_state>();

	SET_TILE_INFO(
			1,
			state->m_bg0tilemap_ram[2*tile_index+1],
			(state->m_bg0tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((state->m_bg0tilemap_ram[2*tile_index]&0xc0)>>6));
}

static TILE_GET_INFO( get_bg1tile_info )
{
	tecmosys_state *state = machine.driver_data<tecmosys_state>();

	SET_TILE_INFO(
			2,
			state->m_bg1tilemap_ram[2*tile_index+1],
			(state->m_bg1tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((state->m_bg1tilemap_ram[2*tile_index]&0xc0)>>6));
}

static TILE_GET_INFO( get_bg2tile_info )
{
	tecmosys_state *state = machine.driver_data<tecmosys_state>();

	SET_TILE_INFO(
			3,
			state->m_bg2tilemap_ram[2*tile_index+1],
			(state->m_bg2tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((state->m_bg2tilemap_ram[2*tile_index]&0xc0)>>6));
}

static TILE_GET_INFO( get_fg_tile_info )
{
	tecmosys_state *state = machine.driver_data<tecmosys_state>();

	SET_TILE_INFO(
			0,
			state->m_fgtilemap_ram[2*tile_index+1],
			(state->m_fgtilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((state->m_fgtilemap_ram[2*tile_index]&0xc0)>>6));
}


WRITE16_HANDLER( bg0_tilemap_w )
{
	tecmosys_state *state = space->machine().driver_data<tecmosys_state>();
	COMBINE_DATA(&state->m_bg0tilemap_ram[offset]);
	tilemap_mark_tile_dirty(state->m_bg0tilemap,offset/2);
}

WRITE16_HANDLER( bg1_tilemap_w )
{
	tecmosys_state *state = space->machine().driver_data<tecmosys_state>();
	COMBINE_DATA(&state->m_bg1tilemap_ram[offset]);
	tilemap_mark_tile_dirty(state->m_bg1tilemap,offset/2);
}

WRITE16_HANDLER( bg2_tilemap_w )
{
	tecmosys_state *state = space->machine().driver_data<tecmosys_state>();
	COMBINE_DATA(&state->m_bg2tilemap_ram[offset]);
	tilemap_mark_tile_dirty(state->m_bg2tilemap,offset/2);
}

WRITE16_HANDLER( fg_tilemap_w )
{
	tecmosys_state *state = space->machine().driver_data<tecmosys_state>();
	COMBINE_DATA(&state->m_fgtilemap_ram[offset]);
	tilemap_mark_tile_dirty(state->m_txt_tilemap,offset/2);
}


INLINE void set_color_555(running_machine &machine, pen_t color, int rshift, int gshift, int bshift, UINT16 data)
{
	palette_set_color_rgb(machine, color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}

WRITE16_HANDLER( tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w )
{
	tecmosys_state *state = space->machine().driver_data<tecmosys_state>();
	COMBINE_DATA(&state->m_tilemap_paletteram16[offset]);
	set_color_555(space->machine(), offset+0x4000, 5, 10, 0, state->m_tilemap_paletteram16[offset]);
}

WRITE16_HANDLER( bg0_tilemap_lineram_w )
{
	tecmosys_state *state = space->machine().driver_data<tecmosys_state>();
	COMBINE_DATA(&state->m_bg0tilemap_lineram[offset]);
	if (data!=0x0000) popmessage("non 0 write to bg0 lineram %04x %04x",offset,data);
}

WRITE16_HANDLER( bg1_tilemap_lineram_w )
{
	tecmosys_state *state = space->machine().driver_data<tecmosys_state>();
	COMBINE_DATA(&state->m_bg1tilemap_lineram[offset]);
	if (data!=0x0000) popmessage("non 0 write to bg1 lineram %04x %04x",offset,data);
}

WRITE16_HANDLER( bg2_tilemap_lineram_w )
{
	tecmosys_state *state = space->machine().driver_data<tecmosys_state>();
	COMBINE_DATA(&state->m_bg2tilemap_lineram[offset]);
	if (data!=0x0000) popmessage("non 0 write to bg2 lineram %04x %04x",offset,data);
}



static void tecmosys_render_sprites_to_bitmap(running_machine &machine, bitmap_t *bitmap, UINT16 extrax, UINT16 extray )
{
	tecmosys_state *state = machine.driver_data<tecmosys_state>();
	UINT8 *gfxsrc    = machine.region       ( "gfx1" )->base();
	int i;

	/* render sprites (with priority information) to temp bitmap */
	bitmap_fill(state->m_sprite_bitmap, NULL, 0x0000);
	/* there are multiple spritelists in here, to allow for buffering */
	for (i=(state->m_spritelist*0x4000)/2;i<((state->m_spritelist+1)*0x4000)/2;i+=8)
	{
		int xcnt,ycnt;
		int drawx, drawy;
		UINT16* dstptr;

		int x, y;
		int address;
		int xsize = 16;
		int ysize = 16;
		int colour;
		int flipx, flipy;
		int priority;
		int zoomx, zoomy;

		x = state->m_spriteram[i+0]+386;
		y = (state->m_spriteram[i+1]+1);

		x-= extrax;
		y-= extray;

		y&=0x1ff;
		x&=0x3ff;

		if (x&0x200) x-=0x400;
		if (y&0x100) y-=0x200;

		address =  state->m_spriteram[i+5]| ((state->m_spriteram[i+4]&0x000f)<<16);

		address<<=8;

		flipx = (state->m_spriteram[i+4]&0x0040)>>6;
		flipy = (state->m_spriteram[i+4]&0x0080)>>7; // used by some move effects in tkdensho

		zoomx = (state->m_spriteram[i+2] & 0x0fff)>>0; // zoom?
		zoomy = (state->m_spriteram[i+3] & 0x0fff)>>0; // zoom?

		if ((!zoomx) || (!zoomy)) continue;

		ysize =  ((state->m_spriteram[i+6] & 0x00ff))*16;
		xsize =  (((state->m_spriteram[i+6] & 0xff00)>>8))*16;

		colour =  ((state->m_spriteram[i+4] & 0x3f00))>>8;

		priority = ((state->m_spriteram[i+4] & 0x0030))>>4;

		if (state->m_spriteram[i+4] & 0x8000) continue;

		for (ycnt = 0; ycnt < ysize; ycnt++)
		{
			int actualycnt = (ycnt * zoomy) >> 8;
			int actualysize = (ysize * zoomy) >> 8;

			if (flipy) drawy = y + (actualysize-1) - actualycnt;
			else drawy = y + actualycnt;

			for (xcnt = 0; xcnt < xsize; xcnt++)
			{
				int actualxcnt = (xcnt * zoomx) >> 8;
				int actualxsize = (xsize *zoomx) >> 8;

				if (flipx) drawx = x + (actualxsize-1) - actualxcnt;
				else drawx = x + actualxcnt;

				if ((drawx>=0 && drawx<320) && (drawy>=0 && drawy<240))
				{
					UINT8 data;

					dstptr = BITMAP_ADDR16(state->m_sprite_bitmap, drawy, drawx);


					data =  (gfxsrc[address]);


					if(data) dstptr[0] = (data + (colour*0x100)) | (priority << 14);
				}

				address++;
			}
		}
	}
}

static void tecmosys_tilemap_copy_to_compose(tecmosys_state *state, UINT16 pri)
{
	int y,x;
	UINT16 *srcptr;
	UINT16 *dstptr;
	for (y=0;y<240;y++)
	{
		srcptr = BITMAP_ADDR16(state->m_tmp_tilemap_renderbitmap, y, 0);
		dstptr = BITMAP_ADDR16(state->m_tmp_tilemap_composebitmap, y, 0);
		for (x=0;x<320;x++)
		{
			if ((srcptr[x]&0xf)!=0x0)
			    dstptr[x] =  (srcptr[x]&0x7ff) | pri;
		}
	}
}

static void tecmosys_do_final_mix(running_machine &machine, bitmap_t* bitmap)
{
	tecmosys_state *state = machine.driver_data<tecmosys_state>();
	const pen_t *paldata = machine.pens;
	int y,x;
	UINT16 *srcptr;
	UINT16 *srcptr2;
	UINT32 *dstptr;

	for (y=0;y<240;y++)
	{
		srcptr = BITMAP_ADDR16(state->m_tmp_tilemap_composebitmap, y, 0);
		srcptr2 = BITMAP_ADDR16(state->m_sprite_bitmap, y, 0);

		dstptr = BITMAP_ADDR32(bitmap, y, 0);
		for (x=0;x<320;x++)
		{
			UINT16 pri, pri2;
			UINT16 penvalue;
			UINT16 penvalue2;
			UINT32 colour;
			UINT32 colour2;

			pri = srcptr[x] & 0xc000;
			pri2 = srcptr2[x] & 0xc000;

			penvalue = state->m_tilemap_paletteram16[srcptr[x]&0x7ff];
			colour =   paldata[(srcptr[x]&0x7ff) | 0x4000];

			if (srcptr2[x]&0x3fff)
			{
				penvalue2 = machine.generic.paletteram.u16[srcptr2[x]&0x3fff];
				colour2 = paldata[srcptr2[x]&0x3fff];
			}
			else
			{
				penvalue2 = state->m_tilemap_paletteram16[srcptr[x]&0x7ff];
				colour2 =   paldata[(srcptr[x]&0x7ff) | 0x4000];
			}

			if ((penvalue & 0x8000) && (penvalue2 & 0x8000)) // blend
			{
				int r,g,b;
				int r2,g2,b2;
				b = (colour & 0x000000ff) >> 0;
				g = (colour & 0x0000ff00) >> 8;
				r = (colour & 0x00ff0000) >> 16;

				b2 = (colour2 & 0x000000ff) >> 0;
				g2 = (colour2 & 0x0000ff00) >> 8;
				r2 = (colour2 & 0x00ff0000) >> 16;

				r = (r + r2) >> 1;
				g = (g + g2) >> 1;
				b = (b + b2) >> 1;

				dstptr[x] = b | (g<<8) | (r<<16);
			}
			else if (pri2 >= pri)
			{
				dstptr[x] = colour2;
			}
			else
			{
				dstptr[x] = colour;
			}
		}
	}
}


SCREEN_UPDATE(tecmosys)
{
	tecmosys_state *state = screen->machine().driver_data<tecmosys_state>();

	bitmap_fill(bitmap,cliprect,screen->machine().pens[0x4000]);


	tilemap_set_scrolly( state->m_bg0tilemap, 0, state->m_c80000regs[1]+16);
	tilemap_set_scrollx( state->m_bg0tilemap, 0, state->m_c80000regs[0]+104);

	tilemap_set_scrolly( state->m_bg1tilemap, 0, state->m_a80000regs[1]+17);
	tilemap_set_scrollx( state->m_bg1tilemap, 0, state->m_a80000regs[0]+106);

	tilemap_set_scrolly( state->m_bg2tilemap, 0, state->m_b00000regs[1]+17);
	tilemap_set_scrollx( state->m_bg2tilemap, 0, state->m_b00000regs[0]+106);

	bitmap_fill(state->m_tmp_tilemap_composebitmap,cliprect,0);

	bitmap_fill(state->m_tmp_tilemap_renderbitmap,cliprect,0);
	tilemap_draw(state->m_tmp_tilemap_renderbitmap,cliprect,state->m_bg0tilemap,0,0);
	tecmosys_tilemap_copy_to_compose(state, 0x0000);

	bitmap_fill(state->m_tmp_tilemap_renderbitmap,cliprect,0);
	tilemap_draw(state->m_tmp_tilemap_renderbitmap,cliprect,state->m_bg1tilemap,0,0);
	tecmosys_tilemap_copy_to_compose(state, 0x4000);

	bitmap_fill(state->m_tmp_tilemap_renderbitmap,cliprect,0);
	tilemap_draw(state->m_tmp_tilemap_renderbitmap,cliprect,state->m_bg2tilemap,0,0);
	tecmosys_tilemap_copy_to_compose(state, 0x8000);

	bitmap_fill(state->m_tmp_tilemap_renderbitmap,cliprect,0);
	tilemap_draw(state->m_tmp_tilemap_renderbitmap,cliprect,state->m_txt_tilemap,0,0);
	tecmosys_tilemap_copy_to_compose(state, 0xc000);


	tecmosys_do_final_mix(screen->machine(), bitmap);

	// prepare sprites for NEXT frame - causes 1 frame palette errors, but prevents sprite lag in tkdensho, which is correct?
	tecmosys_render_sprites_to_bitmap(screen->machine(), bitmap, state->m_880000regs[0x0], state->m_880000regs[0x1]);

	return 0;
}

VIDEO_START(tecmosys)
{
	tecmosys_state *state = machine.driver_data<tecmosys_state>();
	state->m_sprite_bitmap = auto_bitmap_alloc(machine,320,240,BITMAP_FORMAT_INDEXED16);
	bitmap_fill(state->m_sprite_bitmap, NULL, 0x4000);

	state->m_tmp_tilemap_composebitmap = auto_bitmap_alloc(machine,320,240,BITMAP_FORMAT_INDEXED16);
	state->m_tmp_tilemap_renderbitmap = auto_bitmap_alloc(machine,320,240,BITMAP_FORMAT_INDEXED16);

	bitmap_fill(state->m_tmp_tilemap_composebitmap, NULL, 0x0000);
	bitmap_fill(state->m_tmp_tilemap_renderbitmap, NULL, 0x0000);


	state->m_txt_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,32*2,32*2);
	tilemap_set_transparent_pen(state->m_txt_tilemap,0);

	state->m_bg0tilemap = tilemap_create(machine, get_bg0tile_info,tilemap_scan_rows,16,16,32,32);
	tilemap_set_transparent_pen(state->m_bg0tilemap,0);

	state->m_bg1tilemap = tilemap_create(machine, get_bg1tile_info,tilemap_scan_rows,16,16,32,32);
	tilemap_set_transparent_pen(state->m_bg1tilemap,0);

	state->m_bg2tilemap = tilemap_create(machine, get_bg2tile_info,tilemap_scan_rows,16,16,32,32);
	tilemap_set_transparent_pen(state->m_bg2tilemap,0);
}
