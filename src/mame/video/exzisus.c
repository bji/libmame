/***************************************************************************

Functions to emulate the video hardware of the machine.

 Video hardware of this hardware is almost similar with "mexico86". So,
 most routines are derived from mexico86 driver.

***************************************************************************/


#include "emu.h"
#include "includes/exzisus.h"


/***************************************************************************
  Memory handlers
***************************************************************************/

READ8_HANDLER ( exzisus_videoram_0_r )
{
	exzisus_state *state = space->machine->driver_data<exzisus_state>();
	return state->videoram0[offset];
}


READ8_HANDLER ( exzisus_videoram_1_r )
{
	exzisus_state *state = space->machine->driver_data<exzisus_state>();
	return state->videoram1[offset];
}


READ8_HANDLER ( exzisus_objectram_0_r )
{
	exzisus_state *state = space->machine->driver_data<exzisus_state>();
	return state->objectram0[offset];
}


READ8_HANDLER ( exzisus_objectram_1_r )
{
	exzisus_state *state = space->machine->driver_data<exzisus_state>();
	return state->objectram1[offset];
}


WRITE8_HANDLER( exzisus_videoram_0_w )
{
	exzisus_state *state = space->machine->driver_data<exzisus_state>();
	state->videoram0[offset] = data;
}


WRITE8_HANDLER( exzisus_videoram_1_w )
{
	exzisus_state *state = space->machine->driver_data<exzisus_state>();
	state->videoram1[offset] = data;
}


WRITE8_HANDLER( exzisus_objectram_0_w )
{
	exzisus_state *state = space->machine->driver_data<exzisus_state>();
	state->objectram0[offset] = data;
}


WRITE8_HANDLER( exzisus_objectram_1_w )
{
	exzisus_state *state = space->machine->driver_data<exzisus_state>();
	state->objectram1[offset] = data;
}


/***************************************************************************
  Screen refresh
***************************************************************************/

VIDEO_UPDATE( exzisus )
{
	exzisus_state *state = screen->machine->driver_data<exzisus_state>();
	int offs;
	int sx, sy, xc, yc;
	int gfx_num, gfx_attr, gfx_offs;

	/* Is this correct ? */
	bitmap_fill(bitmap, cliprect, 1023);

	/* ---------- 1st TC0010VCU ---------- */
	sx = 0;
	for (offs = 0 ; offs < state->objectram_size0 ; offs += 4)
    {
		int height;

		/* Skip empty sprites. */
		if ( !(*(UINT32 *)(&state->objectram0[offs])) )
		{
			continue;
		}

		gfx_num = state->objectram0[offs + 1];
		gfx_attr = state->objectram0[offs + 3];

		if ((gfx_num & 0x80) == 0)	/* 16x16 sprites */
		{
			gfx_offs = ((gfx_num & 0x7f) << 3);
			height = 2;

			sx = state->objectram0[offs + 2];
			sx |= (gfx_attr & 0x40) << 2;
		}
		else	/* tilemaps (each sprite is a 16x256 column) */
		{
			gfx_offs = ((gfx_num & 0x3f) << 7) + 0x0400;
			height = 32;

			if (gfx_num & 0x40)			/* Next column */
			{
				sx += 16;
			}
			else
			{
				sx = state->objectram0[offs + 2];
				sx |= (gfx_attr & 0x40) << 2;
			}
		}

		sy = 256 - (height << 3) - (state->objectram0[offs]);

		for (xc = 0 ; xc < 2 ; xc++)
		{
			int goffs = gfx_offs;
			for (yc = 0 ; yc < height ; yc++)
			{
				int code, color, x, y;

				code  = (state->videoram0[goffs + 1] << 8) | state->videoram0[goffs];
				color = (state->videoram0[goffs + 1] >> 6) | (gfx_attr & 0x0f);
				x = (sx + (xc << 3)) & 0xff;
				y = (sy + (yc << 3)) & 0xff;

				if (flip_screen_get(screen->machine))
				{
					x = 248 - x;
					y = 248 - y;
				}

				drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[0],
						code & 0x3fff,
						color,
						flip_screen_get(screen->machine), flip_screen_get(screen->machine),
						x, y, 15);
				goffs += 2;
			}
			gfx_offs += height << 1;
		}
	}

	/* ---------- 2nd TC0010VCU ---------- */
	sx = 0;
	for (offs = 0 ; offs < state->objectram_size1 ; offs += 4)
    {
		int height;

		/* Skip empty sprites. */
		if ( !(*(UINT32 *)(&state->objectram1[offs])) )
		{
			continue;
		}

		gfx_num = state->objectram1[offs + 1];
		gfx_attr = state->objectram1[offs + 3];

		if ((gfx_num & 0x80) == 0)	/* 16x16 sprites */
		{
			gfx_offs = ((gfx_num & 0x7f) << 3);
			height = 2;

			sx = state->objectram1[offs + 2];
			sx |= (gfx_attr & 0x40) << 2;
		}
		else	/* tilemaps (each sprite is a 16x256 column) */
		{
			gfx_offs = ((gfx_num & 0x3f) << 7) + 0x0400;	///
			height = 32;

			if (gfx_num & 0x40)			/* Next column */
			{
				sx += 16;
			}
			else
			{
				sx = state->objectram1[offs + 2];
				sx |= (gfx_attr & 0x40) << 2;
			}
		}
		sy = 256 - (height << 3) - (state->objectram1[offs]);

		for (xc = 0 ; xc < 2 ; xc++)
		{
			int goffs = gfx_offs;
			for (yc = 0 ; yc < height ; yc++)
			{
				int code, color, x, y;

				code  = (state->videoram1[goffs + 1] << 8) | state->videoram1[goffs];
				color = (state->videoram1[goffs + 1] >> 6) | (gfx_attr & 0x0f);
				x = (sx + (xc << 3)) & 0xff;
				y = (sy + (yc << 3)) & 0xff;

				if (flip_screen_get(screen->machine))
				{
					x = 248 - x;
					y = 248 - y;
				}

				drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[1],
						code & 0x3fff,
						color,
						flip_screen_get(screen->machine), flip_screen_get(screen->machine),
						x, y, 15);
				goffs += 2;
			}
			gfx_offs += height << 1;
		}
	}
	return 0;
}
