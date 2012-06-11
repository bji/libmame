#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "includes/beezer.h"


TIMER_DEVICE_CALLBACK( beezer_interrupt )
{
	int scanline = param;
//  beezer_state *state = timer.machine().driver_data<beezer_state>();
	via6522_device *via_0 = timer.machine().device<via6522_device>("via6522_0");

	via_0->write_ca2((scanline & 0x20) ? 1 : 0);
	#if 0
	if (scanline == 240) // actually unused by the game! (points to a tight loop)
		device_set_input_line(state->m_maincpu, M6809_FIRQ_LINE, ASSERT_LINE);
	else
		device_set_input_line(state->m_maincpu, M6809_FIRQ_LINE, CLEAR_LINE);
	#endif
}

SCREEN_UPDATE_IND16( beezer )
{
	beezer_state *state = screen.machine().driver_data<beezer_state>();
	UINT8 *videoram = state->m_videoram;
	int x,y;

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		for (x = cliprect.min_x; x <= cliprect.max_x; x+=2)
		{
			bitmap.pix16(y, x+1) = videoram[0x80*x+y] & 0x0f;
			bitmap.pix16(y, x+0) = videoram[0x80*x+y] >> 4;
		}

	return 0;
}

WRITE8_HANDLER( beezer_map_w )
{
	/*
      bit 7 -- 330  ohm resistor  -- BLUE
            -- 560  ohm resistor  -- BLUE
            -- 330  ohm resistor  -- GREEN
            -- 560  ohm resistor  -- GREEN
            -- 1.2 kohm resistor  -- GREEN
            -- 330  ohm resistor  -- RED
            -- 560  ohm resistor  -- RED
      bit 0 -- 1.2 kohm resistor  -- RED
    */

	int r, g, b, bit0, bit1, bit2;

	/* red component */
	bit0 = (data >> 0) & 0x01;
	bit1 = (data >> 1) & 0x01;
	bit2 = (data >> 2) & 0x01;
	r = 0x26 * bit0 + 0x50 * bit1 + 0x89 * bit2;
	/* green component */
	bit0 = (data >> 3) & 0x01;
	bit1 = (data >> 4) & 0x01;
	bit2 = (data >> 5) & 0x01;
	g = 0x26 * bit0 + 0x50 * bit1 + 0x89 * bit2;
	/* blue component */
	bit0 = (data >> 6) & 0x01;
	bit1 = (data >> 7) & 0x01;
	b = 0x5f * bit0 + 0xa0 * bit1;

	palette_set_color(space->machine(), offset, MAKE_RGB(r, g, b));
}

READ8_HANDLER( beezer_line_r )
{
	return space->machine().primary_screen->vpos();
//  Note: was (state->m_scanline & 0xfe) << 1; with scanline % 128
}

