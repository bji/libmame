/***************************************************************************

    Atari Arcade Classics hardware (prototypes)

    Note: this video hardware has some similarities to Shuuz & company
    The sprite offset registers are stored to 3EFF80

****************************************************************************/


#include "emu.h"
#include "video/atarimo.h"
#include "includes/arcadecl.h"


static void arcadecl_bitmap_render(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect);

/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( arcadecl )
{
	static const atarimo_desc modesc =
	{
		0,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		0,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x100,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0x00ff,0,0,0 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0,0x7fff,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0,0x000f,0 }},	/* mask for the color */
		{{ 0,0,0xff80,0 }},	/* mask for the X position */
		{{ 0,0,0,0xff80 }},	/* mask for the Y position */
		{{ 0,0,0,0x0070 }},	/* mask for the width, in tiles*/
		{{ 0,0,0,0x0007 }},	/* mask for the height, in tiles */
		{{ 0,0x8000,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0 }},			/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0,					/* callback routine for special entries */
	};
	arcadecl_state *state = machine.driver_data<arcadecl_state>();

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);

	/* set the intial scroll offset */
	atarimo_set_xscroll(0, -12);
	atarimo_set_yscroll(0, 0x110);
	state->m_has_mo = (machine.gfx[0]->total_elements > 10);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

SCREEN_UPDATE_IND16( arcadecl )
{
	arcadecl_state *state = screen.machine().driver_data<arcadecl_state>();

	/* draw the playfield */
	arcadecl_bitmap_render(screen.machine(), bitmap, cliprect);

	/* draw and merge the MO */
	if (state->m_has_mo)
	{
		atarimo_rect_list rectlist;
		bitmap_ind16 *mobitmap;
		int x, y, r;

		mobitmap = atarimo_render(0, cliprect, &rectlist);
		for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
			for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
			{
				UINT16 *mo = &mobitmap->pix16(y);
				UINT16 *pf = &bitmap.pix16(y);
				for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
					if (mo[x])
					{
						/* not yet verified
                        */
						pf[x] = mo[x];

						/* erase behind ourselves */
						mo[x] = 0;
					}
			}
	}
	return 0;
}



/*************************************
 *
 *  Bitmap rendering
 *
 *************************************/

static void arcadecl_bitmap_render(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	arcadecl_state *state = machine.driver_data<arcadecl_state>();
	int x, y;

	/* update any dirty scanlines */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const UINT16 *src = &state->m_bitmap[256 * y];
		UINT16 *dst = &bitmap.pix16(y);

		/* regenerate the line */
		for (x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
		{
			int bits = src[(x - 8) / 2];
			dst[x + 0] = bits >> 8;
			dst[x + 1] = bits & 0xff;
		}
	}
}
