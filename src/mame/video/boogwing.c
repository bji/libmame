#include "emu.h"
#include "includes/boogwing.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"
#include "video/decospr.h"

VIDEO_START( boogwing )
{
	machine.device<decospr_device>("spritegen1")->alloc_sprite_bitmap();
	machine.device<decospr_device>("spritegen2")->alloc_sprite_bitmap();
}


/* Mix the 2 sprite planes with the already rendered tilemaps..
 note, if we implement tilemap blending etc. too we'll probably have to mix those in here as well..

 this is just a reimplementation of the old priority system used before conversion but to work with
 the bitmaps.  It could probably be simplified / improved greatly, along with the long-standing bugs
 fixed, with manual mixing you have full control.

 apparently priority is based on a PROM, that should be used if possible.
*/
static void mix_boogwing(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	boogwing_state *state = machine.driver_data<boogwing_state>();
	int y, x;
	const pen_t *paldata = machine.pens;
	bitmap_ind16 *sprite_bitmap1, *sprite_bitmap2;
	bitmap_ind8* priority_bitmap;

	UINT16 priority = decocomn_priority_r(state->m_decocomn, 0, 0xffff);

	sprite_bitmap1 = &machine.device<decospr_device>("spritegen1")->get_sprite_temp_bitmap();
	sprite_bitmap2 = &machine.device<decospr_device>("spritegen2")->get_sprite_temp_bitmap();
	priority_bitmap = &machine.priority_bitmap;

	UINT32* dstline;
	UINT16 *srcline1, *srcline2;
	UINT8 *srcpriline;

	for (y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		srcline1=&sprite_bitmap1->pix16(y,0);
		srcline2=&sprite_bitmap2->pix16(y,0);
		srcpriline=&priority_bitmap->pix8(y,0);

		dstline=&bitmap.pix32(y,0);

		for (x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			UINT16 pix1 = srcline1[x];
			UINT16 pix2 = srcline2[x];

			/* Here we have
             pix1 - raw pixel / colour / priority data from first 1sdt chip
             pix2 - raw pixel / colour / priority data from first 2nd chip
            */

			int pri1, pri2;
			int spri1, spri2, alpha2;
			alpha2 = 0xff;

			// pix1 sprite vs pix2 sprite
			if (pix1 & 0x400)		// todo - check only in pri mode 2??
				spri1 = 8;
			else
				spri1 = 32;

			// pix1 sprite vs playfield
			switch (priority)
			{
				case 0x01:
					{
						if ((pix1 & 0x600))
							pri1 = 16;
						else
							pri1 = 64;
					}
					break;

				default:
					{
						if ((pix1 & 0x600) == 0x600)
							pri1 = 4;
						else if ((pix1 & 0x600) == 0x400)
							pri1 = 16;
						else
							pri1 = 64;
					}
					break;
			}

			// pix2 sprite vs pix1 sprite
			if ((pix2 & 0x600) == 0x600)
				spri2 = 4;
			else if ((pix2 & 0x600))
				spri2 = 16;
			else
				spri2 = 64;

			// Transparency
			if (pix2 & 0x100)
				alpha2 = 0x80;

			// pix2 sprite vs playfield
			switch (priority)
			{
				case 0x02:
					{
						// Additional sprite alpha in this mode
						if (pix2 & 0x400)
							alpha2 = 0x80;

						// Sprite vs playfield
						if ((pix2 & 0x600) == 0x600)
							pri2 = 4;
						else if ((pix2 & 0x600) == 0x400)
							pri2 = 16;
						else
							pri2 = 64;
					}
					break;

				default:
					{
						if ((pix2 & 0x400) == 0x400)
							pri2 = 16;
						else
							pri2 = 64;
					}
					break;
			}

			UINT8 bgpri = srcpriline[x];
			/* once we get here we have

            pri1 - 4/16/64 (sprite chip 1 pixel priority relative to bg)
            pri2 - 4/16/64 (sprite chip 2 pixel priority relative to bg)
            spri1 - 8/32 (priority of sprite chip 1 relative to other sprite chip)
            spri2 - 4/16/64 (priority of sprite chip 2 relative to other sprite chip)
            alpha2 - 0x80/0xff alpha level of sprite chip 2 pixels (0x80 if enabled, 0xff if not)

            bgpri - 0 / 8 / 32 (from drawing tilemaps earlier, to compare above pri1/pri2 priorities against)
            pix1 - same as before (ready to extract just colour data from)
            pix2 - same as before  ^^
            */

			int drawnpixe1 = 0;
			if (pix1 & 0xf)
			{
				if (pri1 > bgpri)
				{
					dstline[x] = paldata[(pix1&0x1ff)+0x500];
					drawnpixe1 = 1;
				}
			}

			if (pix2 & 0xf)
			{
				if (pri2 > bgpri)
				{
					if ((!drawnpixe1) || (spri2 > spri1))
					{
						if (alpha2==0xff)
						{
							dstline[x] = paldata[(pix2&0xff)+0x700];
						}
						else
						{
							UINT32 base = dstline[x];
							dstline[x] = alpha_blend_r32(base, paldata[(pix2&0xff)+0x700], alpha2);
						}
					}
				}
			}
		}
	}
}

SCREEN_UPDATE_RGB32( boogwing )
{
	boogwing_state *state = screen.machine().driver_data<boogwing_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);
	UINT16 priority = decocomn_priority_r(state->m_decocomn, 0, 0xffff);

	/* Draw sprite planes to bitmaps for later mixing */
	screen.machine().device<decospr_device>("spritegen2")->draw_sprites(bitmap, cliprect, state->m_spriteram2->buffer(), 0x400, true);
	screen.machine().device<decospr_device>("spritegen1")->draw_sprites(bitmap, cliprect, state->m_spriteram->buffer(), 0x400, true);

	state->flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	/* Draw playfields */
	bitmap.fill(screen.machine().pens[0x400], cliprect); /* pen not confirmed */
	screen.machine().priority_bitmap.fill(0);

	// bit&0x8 is definitely some kind of palette effect
	// bit&0x4 combines playfields
	if ((priority & 0x7) == 0x5)
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_12_combine_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x1 || (priority & 0x7) == 0x2)
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 8);
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 32);
	}
	else if ((priority & 0x7) == 0x3)
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 8);

		// This mode uses playfield 3 to shadow sprites & playfield 2 (instead of
		// regular alpha-blending, the destination is inverted).  Not yet implemented.
		// deco16ic_tilemap_3_draw(state->m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_ALPHA(0x80), 32);
	}
	else
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 8);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 32);
	}

	mix_boogwing(screen.machine(), bitmap,cliprect);

	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}
