#include "emu.h"
#include "kan_pand.h"
#include "includes/galpanic.h"


VIDEO_START( galpanic )
{
	galpanic_state *state = machine->driver_data<galpanic_state>();
	machine->generic.tmpbitmap = machine->primary_screen->alloc_compatible_bitmap();
	state->sprites_bitmap = machine->primary_screen->alloc_compatible_bitmap();
}

PALETTE_INIT( galpanic )
{
	int i;

	/* first 1024 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (i = 0;i < 32768;i++)
		palette_set_color_rgb(machine,i+1024,pal5bit(i >> 5),pal5bit(i >> 10),pal5bit(i >> 0));
}



WRITE16_HANDLER( galpanic_bgvideoram_w )
{
	galpanic_state *state = space->machine->driver_data<galpanic_state>();
	int sx,sy;


	data = COMBINE_DATA(&state->bgvideoram[offset]);

	sy = offset / 256;
	sx = offset % 256;

	*BITMAP_ADDR16(space->machine->generic.tmpbitmap, sy, sx) = 1024 + (data >> 1);
}

WRITE16_HANDLER( galpanic_paletteram_w )
{
	data = COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	/* bit 0 seems to be a transparency flag for the front bitmap */
	palette_set_color_rgb(space->machine,offset,pal5bit(data >> 6),pal5bit(data >> 11),pal5bit(data >> 1));
}


static void comad_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs;
	int sx=0, sy=0;

	for (offs = 0;offs < machine->generic.spriteram_size/2;offs += 4)
	{
		int code,color,flipx,flipy;

		code = spriteram16[offs + 1] & 0x1fff;
		color = (spriteram16[offs] & 0x003c) >> 2;
		flipx = spriteram16[offs] & 0x0002;
		flipy = spriteram16[offs] & 0x0001;

		if((spriteram16[offs] & 0x6000) == 0x6000) /* Link bits */
		{
			sx += spriteram16[offs + 2] >> 6;
			sy += spriteram16[offs + 3] >> 6;
		}
		else
		{
			sx = spriteram16[offs + 2] >> 6;
			sy = spriteram16[offs + 3] >> 6;
		}

		sx = (sx&0x1ff) - (sx&0x200);
		sy = (sy&0x1ff) - (sy&0x200);

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

static void draw_fgbitmap(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	galpanic_state *state = machine->driver_data<galpanic_state>();
	int offs;

	for (offs = 0;offs < state->fgvideoram_size/2;offs++)
	{
		int sx,sy,color;

		sx = offs % 256;
		sy = offs / 256;
		color = state->fgvideoram[offs];
		if (color)
			*BITMAP_ADDR16(bitmap, sy, sx) = color;
	}
}

SCREEN_UPDATE( galpanic )
{
	device_t *pandora = screen->machine->device("pandora");

	/* copy the temporary bitmap to the screen */
	copybitmap(bitmap,screen->machine->generic.tmpbitmap,0,0,0,0,cliprect);

	draw_fgbitmap(screen->machine, bitmap, cliprect);

	pandora_update(pandora, bitmap, cliprect);

	return 0;
}

SCREEN_UPDATE( comad )
{
	galpanic_state *state = screen->machine->driver_data<galpanic_state>();
	/* copy the temporary bitmap to the screen */
	copybitmap(bitmap,screen->machine->generic.tmpbitmap,0,0,0,0,cliprect);

	draw_fgbitmap(screen->machine, bitmap, cliprect);


//  if(galpanic_clear_sprites)
	{
		bitmap_fill(state->sprites_bitmap,cliprect,0);
		comad_draw_sprites(screen->machine,bitmap,cliprect);
	}
//  else
//  {
//      /* keep sprites on the bitmap without clearing them */
//      comad_draw_sprites(screen->machine,state->sprites_bitmap,0);
//      copybitmap_trans(bitmap,state->sprites_bitmap,0,0,0,0,cliprect,0);
//  }
	return 0;
}
