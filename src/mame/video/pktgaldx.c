#include "emu.h"
#include "includes/deco16ic.h"

UINT16* pcktgaldb_fgram;
UINT16* pcktgaldb_sprites;


static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs;
	int flipscreen=!flip_screen_get(machine);

	for (offs = 0;offs < 0x400;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		sprite = spriteram16[offs+1];
		if (!sprite) continue;

		y = spriteram16[offs];
		flash=y&0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1)) continue;

		x = spriteram16[offs+2];
		colour = (x >>9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
        x = 304 - x;

		if (x>320) continue;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flipscreen)
		{
			y=240-y;
			x=304-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);

			multi--;
		}
	}
}

static int pktgaldx_bank_callback(const int bank)
{
	return ((bank>>4) & 0x7) * 0x1000;
}

/* Video on the orginal */
VIDEO_START(pktgaldx)
{
	deco16_1_video_init(machine);

	deco16_set_tilemap_bank_callback(1,pktgaldx_bank_callback);
}

VIDEO_UPDATE(pktgaldx)
{
	flip_screen_set(screen->machine,  deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);

	bitmap_fill(bitmap,cliprect,0); /* not Confirmed */
	bitmap_fill(screen->machine->priority_bitmap,NULL,0);

	deco16_tilemap_2_draw(screen,bitmap,cliprect,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	deco16_tilemap_1_draw(screen,bitmap,cliprect,0,0);
	return 0;
}

/* Video for the bootleg */
VIDEO_START(pktgaldb)
{
}

VIDEO_UPDATE(pktgaldb)
{
	int x,y;
	int offset=0;
	int tileno;
	int colour;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	/* the bootleg seems to treat the tilemaps as sprites */
	for (offset = 0;offset<0x1600/2;offset+=8)
	{
		tileno =  pcktgaldb_sprites[offset+3] | (pcktgaldb_sprites[offset+2]<<16);
		colour =  pcktgaldb_sprites[offset+1]>>1;
		x = pcktgaldb_sprites[offset+0];
		y = pcktgaldb_sprites[offset+4];

		x-=0xc2;
		y&=0x1ff;
		y-=8;

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],tileno^0x1000,colour,0,0,x,y,0);
	}

	for (offset = 0x1600/2;offset<0x2000/2;offset+=8)
	{
		tileno =  pcktgaldb_sprites[offset+3] | (pcktgaldb_sprites[offset+2]<<16);
		colour =  pcktgaldb_sprites[offset+1]>>1;
		x = pcktgaldb_sprites[offset+0]&0x1ff;
		y = pcktgaldb_sprites[offset+4]&0x0ff;

		x-=0xc2;
		y&=0x1ff;
		y-=8;

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],tileno^0x4000,colour,0,0,x,y,0);
	}

	for (offset = 0x2000/2;offset<0x4000/2;offset+=8)
	{
		tileno =  pcktgaldb_sprites[offset+3] | (pcktgaldb_sprites[offset+2]<<16);
		colour =  pcktgaldb_sprites[offset+1]>>1;
		x = pcktgaldb_sprites[offset+0]&0x1ff;
		y = pcktgaldb_sprites[offset+4]&0x0ff;

		x-=0xc2;
		y&=0x1ff;
		y-=8;

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],tileno^0x3000,colour,0,0,x,y,0);
	}

	return 0;
}
