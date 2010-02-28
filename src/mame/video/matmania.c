/***************************************************************************

    video.c

    Functions to emulate the video hardware of the machine.

    There are only a few differences between the video hardware of Mysterious
    Stones and Mat Mania. The tile bank select bit is different and the sprite
    selection seems to be different as well. Additionally, the palette is stored
    differently. I'm also not sure that the 2nd tile page is really used in
    Mysterious Stones.

***************************************************************************/

#include "emu.h"
#include "includes/matmania.h"



UINT8 *matmania_videoram,*matmania_colorram;
size_t matmania_videoram_size;
UINT8 *matmania_videoram2,*matmania_colorram2;
size_t matmania_videoram2_size;
UINT8 *matmania_videoram3,*matmania_colorram3;
size_t matmania_videoram3_size;
UINT8 *matmania_scroll;
static bitmap_t *tmpbitmap;
static bitmap_t *tmpbitmap2;

UINT8 *matmania_pageselect;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mat Mania is unusual in that it has both PROMs and RAM to control the
  palette. PROMs are used for characters and background tiles, RAM for
  sprites.
  I don't know for sure how the PROMs are connected to the RGB output,
  but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/
PALETTE_INIT( matmania )
{
	int i;

	for (i = 0;i < 64;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[64] >> 0) & 0x01;
		bit1 = (color_prom[64] >> 1) & 0x01;
		bit2 = (color_prom[64] >> 2) & 0x01;
		bit3 = (color_prom[64] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}



WRITE8_HANDLER( matmania_paletteram_w )
{
	int bit0,bit1,bit2,bit3,val;
	int r,g,b;
	int offs2;


	space->machine->generic.paletteram.u8[offset] = data;
	offs2 = offset & 0x0f;

	val = space->machine->generic.paletteram.u8[offs2];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = space->machine->generic.paletteram.u8[offs2 | 0x10];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = space->machine->generic.paletteram.u8[offs2 | 0x20];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	palette_set_color(space->machine,offs2 + 64,MAKE_RGB(r,g,b));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
VIDEO_START( matmania )
{
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);
	bitmap_format format = video_screen_get_format(machine->primary_screen);

	/* Mat Mania has a virtual screen twice as large as the visible screen */
	tmpbitmap  = auto_bitmap_alloc(machine, width, 2*height, format);
	tmpbitmap2 = auto_bitmap_alloc(machine, width, 2*height, format);
}



VIDEO_UPDATE( matmania )
{
	UINT8 *spriteram = screen->machine->generic.spriteram.u8;
	int offs;


	/* Update the tiles in the left tile ram bank */
	for (offs = matmania_videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy;


		sx = 15 - offs / 32;
		sy = offs % 32;

		drawgfx_opaque(tmpbitmap,0,screen->machine->gfx[1],
				matmania_videoram[offs] + ((matmania_colorram[offs] & 0x08) << 5),
				(matmania_colorram[offs] & 0x30) >> 4,
				0,sy >= 16,	/* flip horizontally tiles on the right half of the bitmap */
				16*sx,16*sy);
	}

	/* Update the tiles in the right tile ram bank */
	for (offs = matmania_videoram3_size - 1;offs >= 0;offs--)
	{
			int sx,sy;


		sx = 15 - offs / 32;
		sy = offs % 32;

		drawgfx_opaque(tmpbitmap2,0,screen->machine->gfx[1],
				matmania_videoram3[offs] + ((matmania_colorram3[offs] & 0x08) << 5),
				(matmania_colorram3[offs] & 0x30) >> 4,
				0,sy >= 16,	/* flip horizontally tiles on the right half of the bitmap */
				16*sx,16*sy);
	}


	/* copy the temporary bitmap to the screen */
	{
		int scrolly;


		scrolly = -*matmania_scroll;
		if (matmania_pageselect[0]&0x01) // maniach sets 0x20 sometimes, which must have a different meaning
			copyscrollbitmap(bitmap,tmpbitmap2,0,0,1,&scrolly,cliprect);
		else
			copyscrollbitmap(bitmap,tmpbitmap,0,0,1,&scrolly,cliprect);
	}


	/* Draw the sprites */
	for (offs = 0;offs < screen->machine->generic.spriteram_size;offs += 4)
	{
		if (spriteram[offs] & 0x01)
		{
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[2],
					spriteram[offs+1] + ((spriteram[offs] & 0xf0) << 4),
					(spriteram[offs] & 0x08) >> 3,
					spriteram[offs] & 0x04,spriteram[offs] & 0x02,
					239 - spriteram[offs+3],(240 - spriteram[offs+2]) & 0xff,0);
		}
	}


	/* draw the frontmost playfield. They are characters, but draw them as sprites */
	for (offs = matmania_videoram2_size - 1;offs >= 0;offs--)
	{
		int sx,sy;


		sx = 31 - offs / 32;
		sy = offs % 32;

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],
				matmania_videoram2[offs] + 256 * (matmania_colorram2[offs] & 0x07),
				(matmania_colorram2[offs] & 0x30) >> 4,
				0,0,
				8*sx,8*sy,0);
	}
	return 0;
}

VIDEO_UPDATE( maniach )
{
	UINT8 *spriteram = screen->machine->generic.spriteram.u8;
	int offs;


	/* Update the tiles in the left tile ram bank */
	for (offs = matmania_videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy;


		sx = 15 - offs / 32;
		sy = offs % 32;

		drawgfx_opaque(tmpbitmap,0,screen->machine->gfx[1],
				matmania_videoram[offs] + ((matmania_colorram[offs] & 0x03) << 8),
				(matmania_colorram[offs] & 0x30) >> 4,
				0,sy >= 16,	/* flip horizontally tiles on the right half of the bitmap */
				16*sx,16*sy);
	}

	/* Update the tiles in the right tile ram bank */
	for (offs = matmania_videoram3_size - 1;offs >= 0;offs--)
	{
		int sx,sy;


		sx = 15 - offs / 32;
		sy = offs % 32;

		drawgfx_opaque(tmpbitmap2,0,screen->machine->gfx[1],
				matmania_videoram3[offs] + ((matmania_colorram3[offs] & 0x03) << 8),
				(matmania_colorram3[offs] & 0x30) >> 4,
				0,sy >= 16,	/* flip horizontally tiles on the right half of the bitmap */
				16*sx,16*sy);
	}


	/* copy the temporary bitmap to the screen */
	{
		int scrolly;


		scrolly = -*matmania_scroll;

		if (matmania_pageselect[0]&0x01) // this sets 0x20 sometimes, which must have a different meaning
			copyscrollbitmap(bitmap,tmpbitmap2,0,0,1,&scrolly,cliprect);
		else
			copyscrollbitmap(bitmap,tmpbitmap,0,0,1,&scrolly,cliprect);
	}


	/* Draw the sprites */
	for (offs = 0;offs < screen->machine->generic.spriteram_size;offs += 4)
	{
		if (spriteram[offs] & 0x01)
		{
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[2],
					spriteram[offs+1] + ((spriteram[offs] & 0xf0) << 4),
					(spriteram[offs] & 0x08) >> 3,
					spriteram[offs] & 0x04,spriteram[offs] & 0x02,
					239 - spriteram[offs+3],(240 - spriteram[offs+2]) & 0xff,0);
		}
	}


	/* draw the frontmost playfield. They are characters, but draw them as sprites */
	for (offs = matmania_videoram2_size - 1;offs >= 0;offs--)
	{
		int sx,sy;


		sx = 31 - offs / 32;
		sy = offs % 32;

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],
				matmania_videoram2[offs] + 256 * (matmania_colorram2[offs] & 0x07),
				(matmania_colorram2[offs] & 0x30) >> 4,
				0,0,
				8*sx,8*sy,0);
	}
	return 0;
}
