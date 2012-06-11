/***************************************************************************

    Space Firebird hardware

****************************************************************************/

#include "emu.h"
#include "includes/spacefb.h"
#include "video/resnet.h"


/*************************************
 *
 *  Port setters
 *
 *************************************/

WRITE8_MEMBER(spacefb_state::spacefb_port_0_w)
{
	machine().primary_screen->update_now();
	m_port_0 = data;
}


WRITE8_MEMBER(spacefb_state::spacefb_port_2_w)
{
	machine().primary_screen->update_now();
	m_port_2 = data;
}



/*************************************
 *
 *  Video system start
 *
 *  The sprites use a 32 bytes palette PROM, connected to the RGB output this
 *  way:
 *
 *  bit 7 -- 220 ohm resistor  -- BLUE
 *        -- 470 ohm resistor  -- BLUE
 *        -- 220 ohm resistor  -- GREEN
 *        -- 470 ohm resistor  -- GREEN
 *        -- 1  kohm resistor  -- GREEN
 *        -- 220 ohm resistor  -- RED
 *        -- 470 ohm resistor  -- RED
 *  bit 0 -- 1  kohm resistor  -- RED
 *
 *
 *  The schematics show that the bullet color is connected this way,
 *  but this is impossible
 *
 *           860 ohm resistor  -- RED
 *            68 ohm resistor  -- GREEN
 *
 *
 *  The background color is connected this way:
 *
 *  Ra    -- 220 ohm resistor  -- BLUE
 *  Rb    -- 470 ohm resistor  -- BLUE
 *  Ga    -- 220 ohm resistor  -- GREEN
 *  Gb    -- 470 ohm resistor  -- GREEN
 *  Ba    -- 220 ohm resistor  -- RED
 *  Bb    -- 470 ohm resistor  -- RED
 *
 *************************************/

VIDEO_START( spacefb )
{
	spacefb_state *state = machine.driver_data<spacefb_state>();
	int width, height;

	/* compute the color gun weights */
	static const int resistances_rg[] = { 1000, 470, 220 };
	static const int resistances_b [] = {       470, 220 };

	compute_resistor_weights(0, 0xff, -1.0,
							 3, resistances_rg, state->m_color_weights_rg, 470, 0,
							 2, resistances_b,  state->m_color_weights_b,  470, 0,
							 0, 0, 0, 0, 0);

	width = machine.primary_screen->width();
	height = machine.primary_screen->height();
	state->m_object_present_map = auto_alloc_array(machine, UINT8, width * height);

	/* this start value positions the stars to match the flyer screen shot,
       but most likely, the actual star position is random as the hardware
       uses whatever value is on the shift register on power-up */
	state->m_star_shift_reg = 0x18f89;
}



/*************************************
 *
 *  Star field generator
 *
 *************************************/

#define NUM_STARFIELD_PENS	(0x40)


INLINE void shift_star_generator(spacefb_state *state)
{
	state->m_star_shift_reg = ((state->m_star_shift_reg << 1) | (((~state->m_star_shift_reg >> 16) & 0x01) ^ ((state->m_star_shift_reg >> 4) & 0x01))) & 0x1ffff;
}


static void get_starfield_pens(spacefb_state *state, pen_t *pens)
{
	/* generate the pens based on the various enable bits */
	int i;

	int color_contrast_r   = state->m_port_2 & 0x01;
	int color_contrast_g   = state->m_port_2 & 0x02;
	int color_contrast_b   = state->m_port_2 & 0x04;
	int background_red     = state->m_port_2 & 0x08;
	int background_blue    = state->m_port_2 & 0x10;
	int disable_star_field = state->m_port_2 & 0x80;

	for (i = 0; i < NUM_STARFIELD_PENS; i++)
	{
		UINT8 gb =  ((i >> 0) & 0x01) && color_contrast_g && !disable_star_field;
		UINT8 ga =  ((i >> 1) & 0x01) && !disable_star_field;
		UINT8 bb =  ((i >> 2) & 0x01) && color_contrast_b && !disable_star_field;
		UINT8 ba = (((i >> 3) & 0x01) || background_blue) && !disable_star_field;
		UINT8 ra = (((i >> 4) & 0x01) || background_red) && !disable_star_field;
		UINT8 rb =  ((i >> 5) & 0x01) && color_contrast_r && !disable_star_field;

		UINT8 r = combine_3_weights(state->m_color_weights_rg, 0, rb, ra);
		UINT8 g = combine_3_weights(state->m_color_weights_rg, 0, gb, ga);
		UINT8 b = combine_2_weights(state->m_color_weights_b,     bb, ba);

		pens[i] = MAKE_RGB(r, g, b);
	}
}


static void draw_starfield(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	spacefb_state *state = screen.machine().driver_data<spacefb_state>();
	int y;
	pen_t pens[NUM_STARFIELD_PENS];

	get_starfield_pens(state, pens);

	/* the shift register is always shifting -- do the portion in the top VBLANK */
	if (cliprect.min_y == screen.visible_area().min_y)
	{
		int i;

		/* one cycle delay introduced by IC10 D flip-flop at the end of the VBLANK */
		int clock_count = (SPACEFB_HBSTART - SPACEFB_HBEND) * SPACEFB_VBEND - 1;

		for (i = 0; i < clock_count; i++)
			shift_star_generator(state);
	}

	/* visible region of the screen */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int x;

		for (x = SPACEFB_HBEND; x < SPACEFB_HBSTART; x++)
		{
			if (state->m_object_present_map[(y * bitmap.width()) + x] == 0)
			{
				/* draw the star - the 4 possible values come from the effect of the two XOR gates */
				if (((state->m_star_shift_reg & 0x1c0ff) == 0x0c0b7) ||
					((state->m_star_shift_reg & 0x1c0ff) == 0x0c0d7) ||
					((state->m_star_shift_reg & 0x1c0ff) == 0x0c0bb) ||
					((state->m_star_shift_reg & 0x1c0ff) == 0x0c0db))
					bitmap.pix32(y, x) = pens[(state->m_star_shift_reg >> 8) & 0x3f];
				else
					bitmap.pix32(y, x) = pens[0];
			}

			shift_star_generator(state);
		}
	}

	/* do the shifting in the bottom VBLANK */
	if (cliprect.max_y == screen.visible_area().max_y)
	{
		int i;
		int clock_count = (SPACEFB_HBSTART - SPACEFB_HBEND) * (SPACEFB_VTOTAL - SPACEFB_VBSTART);

		for (i = 0; i < clock_count; i++)
			shift_star_generator(state);
	}
}



/*************************************
 *
 *  Sprite/Bullet hardware
 *
 *  Sprites are opaque wrt. each other,
 *  but transparent wrt. to the
 *  star field.
 *
 *************************************/

#define NUM_SPRITE_PENS	(0x40)


static void get_sprite_pens(running_machine &machine, pen_t *pens)
{
	spacefb_state *state = machine.driver_data<spacefb_state>();
	static const double fade_weights[] = { 1.0, 1.5, 2.5, 4.0 };
	const UINT8 *prom = machine.root_device().memregion("proms")->base();
	int i;

	for (i = 0; i < NUM_SPRITE_PENS; i++)
	{
		UINT8 data = prom[((state->m_port_0 & 0x40) >> 2) | (i & 0x0f)];

		UINT8 r0 = (data >> 0) & 0x01;
		UINT8 r1 = (data >> 1) & 0x01;
		UINT8 r2 = (data >> 2) & 0x01;

		UINT8 g0 = (data >> 3) & 0x01;
		UINT8 g1 = (data >> 4) & 0x01;
		UINT8 g2 = (data >> 5) & 0x01;

		UINT8 b1 = (data >> 6) & 0x01;
		UINT8 b2 = (data >> 7) & 0x01;

		UINT8 r = combine_3_weights(state->m_color_weights_rg, r0, r1, r2);
		UINT8 g = combine_3_weights(state->m_color_weights_rg, g0, g1, g2);
		UINT8 b = combine_2_weights(state->m_color_weights_b,      b1, b2);

		if (i >> 4)
		{
			double fade_weight = fade_weights[i >> 4];

			/* faded pens */
			r = (r / fade_weight) + 0.5;
			g = (g / fade_weight) + 0.5;
			b = (b / fade_weight) + 0.5;
		}

		pens[i] = MAKE_RGB(r, g, b);
	}
}


static void draw_bullet(running_machine &machine, offs_t offs, pen_t pen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flip)
{
	spacefb_state *state = machine.driver_data<spacefb_state>();
	UINT8 sy;

	UINT8 *gfx = state->memregion("gfx2")->base();

	UINT8 code = state->m_videoram[offs + 0x0200] & 0x3f;
	UINT8 y = ~state->m_videoram[offs + 0x0100] - 2;

	for (sy = 0; sy < 4; sy++)
	{
		UINT8 sx, dy;

		UINT8 data = gfx[(code << 2) | sy];
		UINT8 x = state->m_videoram[offs + 0x0000];

		if (flip)
			dy = ~y;
		else
			dy = y;

		if ((dy > cliprect.min_y) && (dy < cliprect.max_y))
		{
			for (sx = 0; sx < 4; sx++)
			{
				if (data & 0x01)
				{
					UINT16 dx;

					if (flip)
						dx = (255 - x) * 2;
					else
						dx = x * 2;

					bitmap.pix32(dy, dx + 0) = pen;
					bitmap.pix32(dy, dx + 1) = pen;

					state->m_object_present_map[(dy * bitmap.width()) + dx + 0] = 1;
					state->m_object_present_map[(dy * bitmap.width()) + dx + 1] = 1;
				}

				x = x + 1;
				data = data >> 1;
			}
		}

		y = y + 1;
	}
}


static void draw_sprite(running_machine &machine, offs_t offs, pen_t *pens, bitmap_rgb32 &bitmap, const rectangle &cliprect, int flip)
{
	spacefb_state *state = machine.driver_data<spacefb_state>();
	UINT8 sy;

	UINT8 *gfx = state->memregion("gfx1")->base();

	UINT8 code = ~state->m_videoram[offs + 0x0200];
	UINT8 color_base = (~state->m_videoram[offs + 0x0300] & 0x0f) << 2;
	UINT8 y = ~state->m_videoram[offs + 0x0100] - 2;

	for (sy = 0; sy < 8; sy++)
	{
		UINT8 sx, dy;

		UINT8 data1 = gfx[0x0000 | (code << 3) | (sy ^ 0x07)];
		UINT8 data2 = gfx[0x0800 | (code << 3) | (sy ^ 0x07)];

		UINT8 x = state->m_videoram[offs + 0x0000] - 3;

		if (flip)
			dy = ~y;
		else
			dy = y;

		if ((dy > cliprect.min_y) && (dy < cliprect.max_y))
		{
			for (sx = 0; sx < 8; sx++)
			{
				UINT16 dx;
				UINT8 data;
				pen_t pen;

				if (flip)
					dx = (255 - x) * 2;
				else
					dx = x * 2;

				data = ((data1 << 1) & 0x02) | (data2 & 0x01);
				pen = pens[color_base | data];

				bitmap.pix32(dy, dx + 0) = pen;
				bitmap.pix32(dy, dx + 1) = pen;

				state->m_object_present_map[(dy * bitmap.width()) + dx + 0] = (data != 0);
				state->m_object_present_map[(dy * bitmap.width()) + dx + 1] = (data != 0);

				x = x + 1;
				data1 = data1 >> 1;
				data2 = data2 >> 1;
			}
		}

		y = y + 1;
	}
}


static void draw_objects(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	spacefb_state *state = machine.driver_data<spacefb_state>();
	pen_t sprite_pens[NUM_SPRITE_PENS];

	offs_t offs = (state->m_port_0 & 0x20) ? 0x80 : 0x00;
	int flip = state->m_port_0 & 0x01;

	/* since the way the schematics show the bullet color
       connected is impossible, just use pure red for now */
	pen_t bullet_pen = MAKE_RGB(0xff, 0x00, 0x00);

	get_sprite_pens(machine, sprite_pens);

	memset(state->m_object_present_map, 0, bitmap.width() * bitmap.height());

	while (1)
	{
		if (state->m_videoram[offs + 0x0300] & 0x20)
			draw_bullet(machine, offs, bullet_pen, bitmap, cliprect, flip);
		else if (state->m_videoram[offs + 0x0300] & 0x40)
			draw_sprite(machine, offs, sprite_pens, bitmap, cliprect, flip);

		/* next object */
		offs = offs + 1;

		/* end of bank? */
		if ((offs & 0x7f) == 0)  break;
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

SCREEN_UPDATE_RGB32( spacefb )
{
	draw_objects(screen.machine(), bitmap, cliprect);
	draw_starfield(screen, bitmap, cliprect);

	return 0;
}
