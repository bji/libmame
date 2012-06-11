/******************************************************************************

    Video Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -

******************************************************************************/

#include "emu.h"
#include "includes/nb1413m3.h"
#include "includes/nbmj8891.h"


static void nbmj8891_vramflip(running_machine &machine, int vram);
static void nbmj8891_gfxdraw(running_machine &machine);


/******************************************************************************


******************************************************************************/
READ8_MEMBER(nbmj8891_state::nbmj8891_palette_type1_r)
{
	return m_palette[offset];
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_palette_type1_w)
{
	int r, g, b;

	m_palette[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((m_palette[offset + 0] & 0x0f) >> 0);
	g = ((m_palette[offset + 1] & 0xf0) >> 4);
	b = ((m_palette[offset + 1] & 0x0f) >> 0);

	palette_set_color_rgb(machine(), (offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}

READ8_MEMBER(nbmj8891_state::nbmj8891_palette_type2_r)
{
	return m_palette[offset];
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_palette_type2_w)
{
	int r, g, b;

	m_palette[offset] = data;

	if (!(offset & 0x100)) return;

	offset &= 0x0ff;

	r = ((m_palette[offset + 0x000] & 0x0f) >> 0);
	g = ((m_palette[offset + 0x000] & 0xf0) >> 4);
	b = ((m_palette[offset + 0x100] & 0x0f) >> 0);

	palette_set_color_rgb(machine(), (offset & 0x0ff), pal4bit(r), pal4bit(g), pal4bit(b));
}

READ8_MEMBER(nbmj8891_state::nbmj8891_palette_type3_r)
{
	return m_palette[offset];
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_palette_type3_w)
{
	int r, g, b;

	m_palette[offset] = data;

	if (!(offset & 1)) return;

	offset &= 0x1fe;

	r = ((m_palette[offset + 1] & 0x0f) >> 0);
	g = ((m_palette[offset + 0] & 0xf0) >> 4);
	b = ((m_palette[offset + 0] & 0x0f) >> 0);

	palette_set_color_rgb(machine(), (offset >> 1), pal4bit(r), pal4bit(g), pal4bit(b));
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_clutsel_w)
{
	m_clutsel = data;
}

READ8_MEMBER(nbmj8891_state::nbmj8891_clut_r)
{
	return m_clut[offset];
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_clut_w)
{
	m_clut[((m_clutsel & 0x7f) * 0x10) + (offset & 0x0f)] = data;
}

/******************************************************************************


******************************************************************************/
WRITE8_MEMBER(nbmj8891_state::nbmj8891_blitter_w)
{
	switch (offset)
	{
		case 0x00:	m_blitter_src_addr = (m_blitter_src_addr & 0xff00) | data; break;
		case 0x01:	m_blitter_src_addr = (m_blitter_src_addr & 0x00ff) | (data << 8); break;
		case 0x02:	m_blitter_destx = data; break;
		case 0x03:	m_blitter_desty = data; break;
		case 0x04:	m_blitter_sizex = data; break;
		case 0x05:	m_blitter_sizey = data;
					/* writing here also starts the blit */
					nbmj8891_gfxdraw(machine());
					break;
		case 0x06:	m_blitter_direction_x = (data & 0x01) ? 1 : 0;
					m_blitter_direction_y = (data & 0x02) ? 1 : 0;
					m_flipscreen = (data & 0x04) ? 1 : 0;
					m_dispflag = (data & 0x08) ? 0 : 1;
					if (m_gfxdraw_mode) nbmj8891_vramflip(machine(), 1);
					nbmj8891_vramflip(machine(), 0);
					break;
		case 0x07:	break;
	}
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_taiwanmb_blitter_w)
{
	switch (offset)
	{
		case 0:	m_blitter_src_addr = (m_blitter_src_addr & 0xff00) | data; break;
		case 1:	m_blitter_src_addr = (m_blitter_src_addr & 0x00ff) | (data << 8); break;
		case 2:	m_blitter_destx = data; break;
		case 3:	m_blitter_desty = data; break;
		case 4:	m_blitter_sizex = (data - 1) & 0xff; break;
		case 5:	m_blitter_sizey = (data - 1) & 0xff; break;
	}
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_taiwanmb_gfxdraw_w)
{
//  nbmj8891_gfxdraw(machine());
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_taiwanmb_gfxflag_w)
{
	m_flipscreen = (data & 0x04) ? 1 : 0;

	nbmj8891_vramflip(machine(), 0);
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_taiwanmb_mcu_w)
{

	m_param_old[m_param_cnt & 0x0f] = data;

	if (data == 0x00)
	{
		m_blitter_direction_x = 0;
		m_blitter_direction_y = 0;
		m_blitter_destx = 0;
		m_blitter_desty = 0;
		m_blitter_sizex = 0;
		m_blitter_sizey = 0;
		m_dispflag = 0;
	}

/*
    if (data == 0x02)
    {
        if (m_param_old[(m_param_cnt - 1) & 0x0f] == 0x18)
        {
            m_dispflag = 1;
        }
        else if (m_param_old[(m_param_cnt - 1) & 0x0f] == 0x1a)
        {
            m_dispflag = 0;
        }
    }
*/

	if (data == 0x04)
	{
		// CLUT Transfer?
	}

	if (data == 0x12)
	{
		if (m_param_old[(m_param_cnt - 1) & 0x0f] == 0x08)
		{
			m_blitter_direction_x = 1;
			m_blitter_direction_y = 0;
			m_blitter_destx += m_blitter_sizex + 1;
			m_blitter_desty += 0;
			m_blitter_sizex ^= 0xff;
			m_blitter_sizey ^= 0x00;
		}
		else if (m_param_old[(m_param_cnt - 1) & 0x0f] == 0x0a)
		{
			m_blitter_direction_x = 0;
			m_blitter_direction_y = 1;
			m_blitter_destx += 0;
			m_blitter_desty += m_blitter_sizey + 1;
			m_blitter_sizex ^= 0x00;
			m_blitter_sizey ^= 0xff;
		}
		else if (m_param_old[(m_param_cnt - 1) & 0x0f] == 0x0c)
		{
			m_blitter_direction_x = 1;
			m_blitter_direction_y = 1;
			m_blitter_destx += m_blitter_sizex + 1;
			m_blitter_desty += m_blitter_sizey + 1;
			m_blitter_sizex ^= 0xff;
			m_blitter_sizey ^= 0xff;
		}
		else if (m_param_old[(m_param_cnt - 1) & 0x0f] == 0x0e)
		{
			m_blitter_direction_x = 0;
			m_blitter_direction_y = 0;
			m_blitter_destx += 0;
			m_blitter_desty += 0;
			m_blitter_sizex ^= 0x00;
			m_blitter_sizey ^= 0x00;
		}

		nbmj8891_gfxdraw(machine());
	}

//  m_blitter_direction_x = 0;                // for debug
//  m_blitter_direction_y = 0;                // for debug
	m_dispflag = 1;					// for debug

	m_param_cnt++;
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_scrolly_w)
{
	m_scrolly = data;
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_vramsel_w)
{
	/* protection - not sure about this */
	nb1413m3_sndromrgntag = (data & 0x20) ? "protection" : "voice";

	m_vram = data;
}

WRITE8_MEMBER(nbmj8891_state::nbmj8891_romsel_w)
{
	int gfxlen = memregion("gfx1")->bytes();
	m_gfxrom = (data & 0x0f);

	if ((0x20000 * m_gfxrom) > (gfxlen - 1))
	{
#ifdef MAME_DEBUG
		popmessage("GFXROM BANK OVER!!");
#endif
		m_gfxrom &= (gfxlen / 0x20000 - 1);
	}
}

/******************************************************************************


******************************************************************************/
void nbmj8891_vramflip(running_machine &machine, int vram)
{
	nbmj8891_state *state = machine.driver_data<nbmj8891_state>();
	int x, y;
	UINT8 color1, color2;
	UINT8 *vidram;

	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	if (state->m_flipscreen == state->m_flipscreen_old) return;

	vidram = vram ? state->m_videoram1 : state->m_videoram0;

	for (y = 0; y < (height / 2); y++)
	{
		for (x = 0; x < width; x++)
		{
			color1 = vidram[(y * width) + x];
			color2 = vidram[((y ^ 0xff) * width) + (x ^ 0x1ff)];
			vidram[(y * width) + x] = color2;
			vidram[((y ^ 0xff) * width) + (x ^ 0x1ff)] = color1;
		}
	}

	state->m_flipscreen_old = state->m_flipscreen;
	state->m_screen_refresh = 1;
}


static void update_pixel0(running_machine &machine, int x, int y)
{
	nbmj8891_state *state = machine.driver_data<nbmj8891_state>();
	UINT8 color = state->m_videoram0[(y * machine.primary_screen->width()) + x];
	state->m_tmpbitmap0.pix16(y, x) = color;
}

static void update_pixel1(running_machine &machine, int x, int y)
{
	nbmj8891_state *state = machine.driver_data<nbmj8891_state>();
	UINT8 color = state->m_videoram1[(y * machine.primary_screen->width()) + x];
	state->m_tmpbitmap1.pix16(y, x) = (color == 0x7f) ? 0xff : color;
}

static TIMER_CALLBACK( blitter_timer_callback )
{
	nb1413m3_busyflag = 1;
}

static void nbmj8891_gfxdraw(running_machine &machine)
{
	nbmj8891_state *state = machine.driver_data<nbmj8891_state>();
	UINT8 *GFX = state->memregion("gfx1")->base();
	int width = machine.primary_screen->width();

	int x, y;
	int dx1, dx2, dy1, dy2;
	int startx, starty;
	int sizex, sizey;
	int skipx, skipy;
	int ctrx, ctry;
	UINT8 color, color1, color2;
	int gfxaddr, gfxlen;

	nb1413m3_busyctr = 0;

	startx = state->m_blitter_destx + state->m_blitter_sizex;
	starty = state->m_blitter_desty + state->m_blitter_sizey;

	if (state->m_blitter_direction_x)
	{
		sizex = state->m_blitter_sizex ^ 0xff;
		skipx = 1;
	}
	else
	{
		sizex = state->m_blitter_sizex;
		skipx = -1;
	}

	if (state->m_blitter_direction_y)
	{
		sizey = state->m_blitter_sizey ^ 0xff;
		skipy = 1;
	}
	else
	{
		sizey = state->m_blitter_sizey;
		skipy = -1;
	}

	gfxlen = machine.root_device().memregion("gfx1")->bytes();
	gfxaddr = (state->m_gfxrom << 17) + (state->m_blitter_src_addr << 1);

	for (y = starty, ctry = sizey; ctry >= 0; y += skipy, ctry--)
	{
		for (x = startx, ctrx = sizex; ctrx >= 0; x += skipx, ctrx--)
		{
			if ((gfxaddr > (gfxlen - 1)))
			{
#ifdef MAME_DEBUG
				popmessage("GFXROM ADDRESS OVER!!");
#endif
				gfxaddr &= (gfxlen - 1);
			}

			color = GFX[gfxaddr++];

			dx1 = (2 * x + 0) & 0x1ff;
			dx2 = (2 * x + 1) & 0x1ff;

			if (state->m_gfxdraw_mode)
			{
				// 2 layer type
				dy1 = y & 0xff;
				dy2 = (y + state->m_scrolly) & 0xff;
			}
			else
			{
				// 1 layer type
				dy1 = (y + state->m_scrolly) & 0xff;
				dy2 = 0;
			}

			if (!state->m_flipscreen)
			{
				dx1 ^= 0x1ff;
				dx2 ^= 0x1ff;
				dy1 ^= 0xff;
				dy2 ^= 0xff;
			}

			if (state->m_blitter_direction_x)
			{
				// flip
				color1 = (color & 0x0f) >> 0;
				color2 = (color & 0xf0) >> 4;
			}
			else
			{
				// normal
				color1 = (color & 0xf0) >> 4;
				color2 = (color & 0x0f) >> 0;
			}

			color1 = state->m_clut[((state->m_clutsel & 0x7f) << 4) + color1];
			color2 = state->m_clut[((state->m_clutsel & 0x7f) << 4) + color2];

			if ((!state->m_gfxdraw_mode) || (state->m_vram & 0x01))
			{
				// layer 1
				if (color1 != 0xff)
				{
					state->m_videoram0[(dy1 * width) + dx1] = color1;
					update_pixel0(machine, dx1, dy1);
				}
				if (color2 != 0xff)
				{
					state->m_videoram0[(dy1 * width) + dx2] = color2;
					update_pixel0(machine, dx2, dy1);
				}
			}
			if (state->m_gfxdraw_mode && (state->m_vram & 0x02))
			{
				// layer 2
				if (state->m_vram & 0x08)
				{
					// transparent enable
					if (color1 != 0xff)
					{
						state->m_videoram1[(dy2 * width) + dx1] = color1;
						update_pixel1(machine, dx1, dy2);
					}
					if (color2 != 0xff)
					{
						state->m_videoram1[(dy2 * width) + dx2] = color2;
						update_pixel1(machine, dx2, dy2);
					}
				}
				else
				{
					// transparent disable
					state->m_videoram1[(dy2 * width) + dx1] = color1;
					update_pixel1(machine, dx1, dy2);
					state->m_videoram1[(dy2 * width) + dx2] = color2;
					update_pixel1(machine, dx2, dy2);
				}
			}

			nb1413m3_busyctr++;
		}
	}

	nb1413m3_busyflag = 0;
	machine.scheduler().timer_set(attotime::from_hz(400000) * nb1413m3_busyctr, FUNC(blitter_timer_callback));
}

/******************************************************************************


******************************************************************************/
VIDEO_START( nbmj8891_1layer )
{
	nbmj8891_state *state = machine.driver_data<nbmj8891_state>();
	UINT8 *CLUT = state->memregion("protection")->base();
	int i;
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmap0);
	state->m_videoram0 = auto_alloc_array(machine, UINT8, width * height);
	state->m_palette = auto_alloc_array(machine, UINT8, 0x200);
	state->m_clut = auto_alloc_array(machine, UINT8, 0x800);
	memset(state->m_videoram0, 0xff, (width * height * sizeof(char)));
	state->m_gfxdraw_mode = 0;

	if (nb1413m3_type == NB1413M3_TAIWANMB)
		for (i = 0; i < 0x0800; i++) state->m_clut[i] = CLUT[i];
}

VIDEO_START( nbmj8891_2layer )
{
	nbmj8891_state *state = machine.driver_data<nbmj8891_state>();
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();

	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmap0);
	machine.primary_screen->register_screen_bitmap(state->m_tmpbitmap1);
	state->m_videoram0 = auto_alloc_array(machine, UINT8, width * height);
	state->m_videoram1 = auto_alloc_array(machine, UINT8, width * height);
	state->m_palette = auto_alloc_array(machine, UINT8, 0x200);
	state->m_clut = auto_alloc_array(machine, UINT8, 0x800);
	memset(state->m_videoram0, 0xff, (width * height * sizeof(UINT8)));
	memset(state->m_videoram1, 0xff, (width * height * sizeof(UINT8)));
	state->m_gfxdraw_mode = 1;
}

/******************************************************************************


******************************************************************************/
SCREEN_UPDATE_IND16( nbmj8891 )
{
	nbmj8891_state *state = screen.machine().driver_data<nbmj8891_state>();
	int x, y;

	if (state->m_screen_refresh)
	{
		int width = screen.width();
		int height = screen.height();

		state->m_screen_refresh = 0;
		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
				update_pixel0(screen.machine(), x, y);

		if (state->m_gfxdraw_mode)
			for (y = 0; y < height; y++)
				for (x = 0; x < width; x++)
					update_pixel1(screen.machine(), x, y);
	}

	if (state->m_dispflag)
	{
		int scrolly;
		if (!state->m_flipscreen) scrolly =   state->m_scrolly;
		else                      scrolly = (-state->m_scrolly) & 0xff;

		if (state->m_gfxdraw_mode)
		{
			copyscrollbitmap      (bitmap, state->m_tmpbitmap0, 0, 0, 0, 0, cliprect);
			copyscrollbitmap_trans(bitmap, state->m_tmpbitmap1, 0, 0, 1, &scrolly, cliprect, 0xff);
		}
		else
			copyscrollbitmap(bitmap, state->m_tmpbitmap0, 0, 0, 1, &scrolly, cliprect);
	}
	else
		bitmap.fill(0xff);

	return 0;
}
