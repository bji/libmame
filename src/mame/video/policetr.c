/***************************************************************************

    P&P Marketing Police Trainer hardware

***************************************************************************/

#include "emu.h"
#include "cpu/mips/r3000.h"
#include "includes/policetr.h"


/* constants */
#define SRCBITMAP_WIDTH		4096

#define DSTBITMAP_WIDTH		512
#define DSTBITMAP_HEIGHT	256


/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( policetr )
{
	policetr_state *state = machine.driver_data<policetr_state>();
	/* the source bitmap is in ROM */
	state->m_srcbitmap = machine.region("gfx1")->base();

	/* compute the height */
	state->m_srcbitmap_height_mask = (machine.region("gfx1")->bytes() / SRCBITMAP_WIDTH) - 1;

	/* the destination bitmap is not directly accessible to the CPU */
	state->m_dstbitmap = auto_alloc_array(machine, UINT8, DSTBITMAP_WIDTH * DSTBITMAP_HEIGHT);
}



/*************************************
 *
 *  Display list processor
 *
 *************************************/

static void render_display_list(running_machine &machine, offs_t offset)
{
	policetr_state *state = machine.driver_data<policetr_state>();
	/* mask against the R3000 address space */
	offset &= 0x1fffffff;

	/* loop over all items */
	while (offset != 0x1fffffff)
	{
		UINT32 *entry = &state->m_rambase[offset / 4];
		UINT32 srcx = entry[0] & 0xfffffff;
		UINT32 srcy = entry[1] & ((state->m_srcbitmap_height_mask << 16) | 0xffff);
		UINT32 srcxstep = entry[2];
		UINT32 srcystep = entry[3];
		int dstw = (entry[4] & 0x1ff) + 1;
		int dsth = ((entry[4] >> 12) & 0x1ff) + 1;
		int dstx = entry[5] & 0x1ff;
		int dsty = (entry[5] >> 12) & 0x1ff;
		UINT8 mask = ~entry[6] >> 16;
		UINT8 color = (entry[6] >> 24) & ~mask;
		UINT32 curx, cury;
		int x, y;

		if (dstx > state->m_render_clip.max_x)
		{
			dstw -= (512 - dstx);
			dstx = 0;
		}
		/* apply X clipping */
		if (dstx < state->m_render_clip.min_x)
		{
			srcx += srcxstep * (state->m_render_clip.min_x - dstx);
			dstw -= state->m_render_clip.min_x - dstx;
			dstx = state->m_render_clip.min_x;
		}
		if (dstx + dstw > state->m_render_clip.max_x)
			dstw = state->m_render_clip.max_x - dstx + 1;

		/* apply Y clipping */
		if (dsty < state->m_render_clip.min_y)
		{
			srcy += srcystep * (state->m_render_clip.min_y - dsty);
			dsth -= state->m_render_clip.min_y - dsty;
			dsty = state->m_render_clip.min_y;
		}
		if (dsty + dsth > state->m_render_clip.max_y)
			dsth = state->m_render_clip.max_y - dsty + 1;

		/* special case for fills */
		if (srcxstep == 0 && srcystep == 0)
		{
			/* prefetch the pixel */
			UINT8 pixel = state->m_srcbitmap[((srcy >> 16) * state->m_srcbitmap_height_mask) * SRCBITMAP_WIDTH + (srcx >> 16) % SRCBITMAP_WIDTH];
			pixel = color | (pixel & mask);

			/* loop over rows and columns */
			if (dstw > 0)
				for (y = 0; y < dsth; y++)
				{
					UINT8 *dst = &state->m_dstbitmap[(dsty + y) * DSTBITMAP_WIDTH + dstx];
					memset(dst, pixel, dstw);
				}
		}

		/* otherwise, standard render */
		else
		{
			/* loop over rows */
			for (y = 0, cury = srcy; y < dsth; y++, cury += srcystep)
			{
				UINT8 *src = &state->m_srcbitmap[((cury >> 16) & state->m_srcbitmap_height_mask) * SRCBITMAP_WIDTH];
				UINT8 *dst = &state->m_dstbitmap[(dsty + y) * DSTBITMAP_WIDTH + dstx];

				/* loop over columns */
				for (x = 0, curx = srcx; x < dstw; x++, curx += srcxstep)
				{
					UINT8 pixel = src[(curx >> 16) % SRCBITMAP_WIDTH];
					if (pixel)
						dst[x] = color | (pixel & mask);
				}
			}
		}

		/* advance to the next link */
		offset = entry[7] & 0x1fffffff;
	}
}



/*************************************
 *
 *  Video controller writes
 *
 *************************************/

WRITE32_HANDLER( policetr_video_w )
{
	policetr_state *state = space->machine().driver_data<policetr_state>();
	/* we assume 4-byte accesses */
	if (mem_mask)
		logerror("%08X: policetr_video_w access with mask %08X\n", cpu_get_previouspc(&space->device()), mem_mask);

	/* 4 offsets */
	switch (offset)
	{
		/* offset 0 specifies the start address of a display list */
		case 0:
			render_display_list(space->machine(), data);
			break;

		/* offset 1 specifies a latch value in the upper 8 bits */
		case 1:
			state->m_video_latch = data >> 24;
			break;

		/* offset 2 has various meanings based on the latch */
		case 2:
		{
			switch (state->m_video_latch)
			{
				/* latch 0x04 specifies the source X offset for a source bitmap pixel read */
				case 0x04:
					state->m_src_xoffs = data >> 16;
					break;

				/* latch 0x14 specifies the source Y offset for a source bitmap pixel read */
				case 0x14:
					state->m_src_yoffs = data >> 16;
					break;

				/* latch 0x20 specifies the top/left corners of the render cliprect */
				case 0x20:
					state->m_render_clip.min_y = (data >> 12) & 0xfff;
					state->m_render_clip.min_x = data & 0xfff;
					break;

				/* latch 0x30 specifies the bottom/right corners of the render cliprect */
				case 0x30:
					state->m_render_clip.max_y = (data >> 12) & 0xfff;
					state->m_render_clip.max_x = data & 0xfff;
					break;

				/* latch 0x50 allows a direct write to the destination bitmap */
				case 0x50:
					if (ACCESSING_BITS_24_31 && state->m_dst_xoffs < DSTBITMAP_WIDTH && state->m_dst_yoffs < DSTBITMAP_HEIGHT)
						state->m_dstbitmap[state->m_dst_yoffs * DSTBITMAP_WIDTH + state->m_dst_xoffs] = data >> 24;
					break;

				/* log anything else */
				default:
					logerror("%08X: policetr_video_w(2) = %08X & %08X with latch %02X\n", cpu_get_previouspc(&space->device()), data, mem_mask, state->m_video_latch);
					break;
			}
			break;
		}

		/* offset 3 has various meanings based on the latch */
		case 3:
		{
			switch (state->m_video_latch)
			{
				/* latch 0x00 is unknown; 0, 1, and 2 get written into the upper 12 bits before rendering */
				case 0x00:
					if (data != (0 << 20) && data != (1 << 20) && data != (2 << 20))
						logerror("%08X: policetr_video_w(3) = %08X & %08X with latch %02X\n", cpu_get_previouspc(&space->device()), data, mem_mask, state->m_video_latch);
					break;

				/* latch 0x10 specifies destination bitmap X and Y offsets */
				case 0x10:
					state->m_dst_yoffs = (data >> 12) & 0xfff;
					state->m_dst_xoffs = data & 0xfff;
					break;

				/* latch 0x20 is unknown; either 0xef or 0x100 is written every IRQ4 */
				case 0x20:
					if (data != (0x100 << 12) && data != (0xef << 12))
						logerror("%08X: policetr_video_w(3) = %08X & %08X with latch %02X\n", cpu_get_previouspc(&space->device()), data, mem_mask, state->m_video_latch);
					break;

				/* latch 0x40 is unknown; a 0 is written every IRQ4 */
				case 0x40:
					if (data != 0)
						logerror("%08X: policetr_video_w(3) = %08X & %08X with latch %02X\n", cpu_get_previouspc(&space->device()), data, mem_mask, state->m_video_latch);
					break;

				/* latch 0x50 clears IRQ4 */
				case 0x50:
					cputag_set_input_line(space->machine(), "maincpu", R3000_IRQ4, CLEAR_LINE);
					break;

				/* latch 0x60 clears IRQ5 */
				case 0x60:
					cputag_set_input_line(space->machine(), "maincpu", R3000_IRQ5, CLEAR_LINE);
					break;

				/* log anything else */
				default:
					logerror("%08X: policetr_video_w(3) = %08X & %08X with latch %02X\n", cpu_get_previouspc(&space->device()), data, mem_mask, state->m_video_latch);
					break;
			}
			break;
		}
	}
}



/*************************************
 *
 *  Video controller reads
 *
 *************************************/

READ32_HANDLER( policetr_video_r )
{
	policetr_state *state = space->machine().driver_data<policetr_state>();
	int inputval;
	int width = space->machine().primary_screen->width();
	int height = space->machine().primary_screen->height();

	/* the value read is based on the latch */
	switch (state->m_video_latch)
	{
		/* latch 0x00 is player 1's gun X coordinate */
		case 0x00:
			inputval = ((input_port_read(space->machine(), "GUNX1") & 0xff) * width) >> 8;
			inputval += 0x50;
			return (inputval << 20) | 0x20000000;

		/* latch 0x01 is player 1's gun Y coordinate */
		case 0x01:
			inputval = ((input_port_read(space->machine(), "GUNY1") & 0xff) * height) >> 8;
			inputval += 0x17;
			return (inputval << 20);

		/* latch 0x02 is player 2's gun X coordinate */
		case 0x02:
			inputval = ((input_port_read(space->machine(), "GUNX2") & 0xff) * width) >> 8;
			inputval += 0x50;
			return (inputval << 20) | 0x20000000;

		/* latch 0x03 is player 2's gun Y coordinate */
		case 0x03:
			inputval = ((input_port_read(space->machine(), "GUNY2") & 0xff) * height) >> 8;
			inputval += 0x17;
			return (inputval << 20);

		/* latch 0x04 is the pixel value in the ROM at the specified address */
		case 0x04:
			return state->m_srcbitmap[(state->m_src_yoffs & state->m_srcbitmap_height_mask) * SRCBITMAP_WIDTH + state->m_src_xoffs % SRCBITMAP_WIDTH] << 24;

		/* latch 0x50 is read at IRQ 4; the top 2 bits are checked. If they're not 0,
            they skip the rest of the interrupt processing */
		case 0x50:
			return 0;
	}

	/* log anything else */
	logerror("%08X: policetr_video_r with latch %02X\n", cpu_get_previouspc(&space->device()), state->m_video_latch);
	return 0;
}




/*************************************
 *
 *  Palette access
 *
 *************************************/

WRITE32_HANDLER( policetr_palette_offset_w )
{
	policetr_state *state = space->machine().driver_data<policetr_state>();
	if (ACCESSING_BITS_16_23)
	{
		state->m_palette_offset = (data >> 16) & 0xff;
		state->m_palette_index = 0;
	}
}


WRITE32_HANDLER( policetr_palette_data_w )
{
	policetr_state *state = space->machine().driver_data<policetr_state>();
	if (ACCESSING_BITS_16_23)
	{
		state->m_palette_data[state->m_palette_index] = (data >> 16) & 0xff;
		if (++state->m_palette_index == 3)
		{
			palette_set_color(space->machine(), state->m_palette_offset, MAKE_RGB(state->m_palette_data[0], state->m_palette_data[1], state->m_palette_data[2]));
			state->m_palette_index = 0;
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

SCREEN_UPDATE_IND16( policetr )
{
	policetr_state *state = screen.machine().driver_data<policetr_state>();
	int width = cliprect.width();
	int y;

	/* render all the scanlines from the dstbitmap to MAME's bitmap */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		draw_scanline8(bitmap, cliprect.min_x, y, width, &state->m_dstbitmap[DSTBITMAP_WIDTH * y + cliprect.min_x], NULL);

	return 0;
}
