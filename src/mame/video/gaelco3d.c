/*************************************************************************

    Driver for Gaelco 3D games

    driver by Aaron Giles

**************************************************************************/

#include "emu.h"
#include "includes/gaelco3d.h"
#include "cpu/tms32031/tms32031.h"
#include "video/rgbutil.h"
#include "video/poly.h"


#define MAX_POLYGONS		4096
#define MAX_POLYDATA		(MAX_POLYGONS * 21)
#define MAX_VERTICES		32

#define DISPLAY_TEXTURE		0
#define LOG_POLYGONS		0
#define DISPLAY_STATS		0

#define IS_POLYEND(x)		(((x) ^ ((x) >> 1)) & 0x4000)


gaelco3d_renderer::gaelco3d_renderer(gaelco3d_state &state)
	: poly_manager<float, gaelco3d_object_data, 1, 2000>(state.machine()),
	  m_state(state),
	  m_screenbits(state.machine().primary_screen->width(), state.machine().primary_screen->height()),
	  m_zbuffer(state.machine().primary_screen->width(), state.machine().primary_screen->height()),
	  m_polygons(0),
	  m_texture_size(state.machine().region("gfx1")->bytes()),
	  m_texmask_size(state.machine().region("gfx2")->bytes() * 8),
	  m_texture(auto_alloc_array(state.machine(), UINT8, m_texture_size)),
	  m_texmask(auto_alloc_array(state.machine(), UINT8, m_texmask_size))
{
	state_save_register_global_bitmap(state.machine(), &m_screenbits);
	state_save_register_global_bitmap(state.machine(), &m_zbuffer);

	/* first expand the pixel data */
	UINT8 *src = state.machine().region("gfx1")->base();
	UINT8 *dst = m_texture;
	for (int y = 0; y < m_texture_size/4096; y += 2)
		for (int x = 0; x < 4096; x += 2)
		{
			dst[(y + 0) * 4096 + (x + 1)] = src[0*m_texture_size/4 + (y/2) * 2048 + (x/2)];
			dst[(y + 1) * 4096 + (x + 1)] = src[1*m_texture_size/4 + (y/2) * 2048 + (x/2)];
			dst[(y + 0) * 4096 + (x + 0)] = src[2*m_texture_size/4 + (y/2) * 2048 + (x/2)];
			dst[(y + 1) * 4096 + (x + 0)] = src[3*m_texture_size/4 + (y/2) * 2048 + (x/2)];
		}

	/* then expand the mask data */
	src = state.machine().region("gfx2")->base();
	dst = m_texmask;
	for (int y = 0; y < m_texmask_size/4096; y++)
		for (int x = 0; x < 4096; x++)
			dst[y * 4096 + x] = (src[(x / 1024) * (m_texmask_size/8/4) + (y * 1024 + x % 1024) / 8] >> (x % 8)) & 1;
}


/*************************************
 *
 *  Video init
 *
 *************************************/

VIDEO_START( gaelco3d )
{
	gaelco3d_state *state = machine.driver_data<gaelco3d_state>();

	state->m_poly = auto_alloc(machine, gaelco3d_renderer(*state));

	state->m_palette = auto_alloc_array(machine, rgb_t, 32768);
	state->m_polydata_buffer = auto_alloc_array(machine, UINT32, MAX_POLYDATA);

	/* save states */

	state_save_register_global_pointer(machine, state->m_palette, 32768);
	state_save_register_global_pointer(machine, state->m_polydata_buffer, MAX_POLYDATA);
	state_save_register_global(machine, state->m_polydata_count);

	state_save_register_global(machine, state->m_lastscan);
}



/*************************************
 *
 *  Polygon rendering
 *
 *************************************/

/*
    Polygon data stream format:

    data[0]  (float) = scale factor to map Z to fixed Z buffer value
    data[1]  (float) = dvoz/dy (change in v/z per Y)
    data[2]  (float) = dvoz/dx (change in v/z per X)
    data[3]  (float) = dooz/dy (change in 1/z per Y)
    data[4]  (float) = dooz/dx (change in 1/z per X)
    data[5]  (float) = duoz/dy (change in u/z per Y)
    data[6]  (float) = duoz/dx (change in u/z per X)
    data[7]  (float) = voz origin (value of v/z at coordinate (0,0))
    data[8]  (float) = ooz origin (value of 1/z at coordinate (0,0))
    data[9]  (float) = uoz origin (value of u/z at coordinate (0,0))
    data[10] (int)   = palette base (bits 14-8)
    data[11] (int)   = texture address
    data[12] (int)   = start point (bits 15-8 = X coordinate, bits 12-0 = Y coordinate)
    data[13] (int)   = next point (bits 15-8 = X coordinate, bits 12-0 = Y coordinate)
    data[14] (int)   = 16.16 dx/dy from previous point to this point
    (repeat these two for each additional point in the fan)
*/

void gaelco3d_renderer::render_poly(screen_device &screen, UINT32 *polydata)
{
	float midx = screen.width() / 2;
	float midy = screen.height() / 2;
	float z0 = tms3203x_device::fp_to_float(polydata[0]);
	float voz_dy = tms3203x_device::fp_to_float(polydata[1]) * 256.0f;
	float voz_dx = tms3203x_device::fp_to_float(polydata[2]) * 256.0f;
	float ooz_dy = tms3203x_device::fp_to_float(polydata[3]);
	float ooz_dx = tms3203x_device::fp_to_float(polydata[4]);
	float uoz_dy = tms3203x_device::fp_to_float(polydata[5]) * 256.0f;
	float uoz_dx = tms3203x_device::fp_to_float(polydata[6]) * 256.0f;
	float voz_base = tms3203x_device::fp_to_float(polydata[7]) * 256.0f - midx * voz_dx - midy * voz_dy;
	float ooz_base = tms3203x_device::fp_to_float(polydata[8]) - midx * ooz_dx - midy * ooz_dy;
	float uoz_base = tms3203x_device::fp_to_float(polydata[9]) * 256.0f - midx * uoz_dx - midy * uoz_dy;
	gaelco3d_object_data &object = object_data_alloc();
	int color = (polydata[10] & 0x7f) << 8;
	vertex_t vert[MAX_VERTICES];
	UINT32 data;
	int vertnum;

	if (LOG_POLYGONS)
	{
		int t;
		logerror("poly: %12.2f %12.2f %12.2f %12.2f %12.2f %12.2f %12.2f %12.2f %12.2f %12.2f %08X %08X (%4d,%4d) %08X",
				(double)tms3203x_device::fp_to_float(polydata[0]),
				(double)tms3203x_device::fp_to_float(polydata[1]),
				(double)tms3203x_device::fp_to_float(polydata[2]),
				(double)tms3203x_device::fp_to_float(polydata[3]),
				(double)tms3203x_device::fp_to_float(polydata[4]),
				(double)tms3203x_device::fp_to_float(polydata[5]),
				(double)tms3203x_device::fp_to_float(polydata[6]),
				(double)tms3203x_device::fp_to_float(polydata[7]),
				(double)tms3203x_device::fp_to_float(polydata[8]),
				(double)tms3203x_device::fp_to_float(polydata[9]),
				polydata[10],
				polydata[11],
				(INT16)(polydata[12] >> 16), (INT16)(polydata[12] << 2) >> 2, polydata[12]);

		logerror(" (%4d,%4d) %08X %08X", (INT16)(polydata[13] >> 16), (INT16)(polydata[13] << 2) >> 2, polydata[13], polydata[14]);
		for (t = 15; !IS_POLYEND(polydata[t - 2]); t += 2)
			logerror(" (%4d,%4d) %08X %08X", (INT16)(polydata[t] >> 16), (INT16)(polydata[t] << 2) >> 2, polydata[t], polydata[t+1]);
		logerror("\n");
	}

	/* fill in object data */
	object.tex = polydata[11];
	object.color = color;
	object.ooz_dx = ooz_dx;
	object.ooz_dy = ooz_dy;
	object.ooz_base = ooz_base;
	object.uoz_dx = uoz_dx;
	object.uoz_dy = uoz_dy;
	object.uoz_base = uoz_base;
	object.voz_dx = voz_dx;
	object.voz_dy = voz_dy;
	object.voz_base = voz_base;
	object.z0 = z0;

	/* extract vertices */
	data = 0;
	for (vertnum = 0; vertnum < ARRAY_LENGTH(vert) && !IS_POLYEND(data); vertnum++)
	{
		/* extract vertex data */
		data = polydata[13 + vertnum * 2];
		vert[vertnum].x = midx + (float)((INT32)data >> 16) + 0.5f;
		vert[vertnum].y = midy + (float)((INT32)(data << 18) >> 18) + 0.5f;
	}

	/* if we have a valid number of verts, render them */
	if (vertnum >= 3)
	{
		const rectangle &visarea = screen.visible_area();

		/* special case: no Z buffering and no perspective correction */
		if (color != 0x7f00 && z0 < 0 && ooz_dx == 0 && ooz_dy == 0)
			render_triangle_fan(visarea, render_delegate(FUNC(gaelco3d_renderer::render_noz_noperspective), this), 0, vertnum, &vert[0]);

		/* general case: non-alpha blended */
		else if (color != 0x7f00)
			render_triangle_fan(visarea, render_delegate(FUNC(gaelco3d_renderer::render_normal), this), 0, vertnum, &vert[0]);

		/* color 0x7f seems to be hard-coded as a 50% alpha blend */
		else
			render_triangle_fan(visarea, render_delegate(FUNC(gaelco3d_renderer::render_alphablend), this), 0, vertnum, &vert[0]);

		m_polygons += vertnum - 2;
	}
}



void gaelco3d_renderer::render_noz_noperspective(INT32 scanline, const extent_t &extent, const gaelco3d_object_data &object, int threadid)
{
	float zbase = recip_approx(object.ooz_base);
	float uoz_step = object.uoz_dx * zbase;
	float voz_step = object.voz_dx * zbase;
	int zbufval = (int)(-object.z0 * zbase);
	offs_t endmask = m_texture_size - 1;
	const rgb_t *palsource = m_state.m_palette + object.color;
	UINT32 tex = object.tex;
	UINT16 *dest = &m_screenbits.pix16(scanline);
	UINT16 *zbuf = &m_zbuffer.pix16(scanline);
	int startx = extent.startx;
	float uoz = (object.uoz_base + scanline * object.uoz_dy + startx * object.uoz_dx) * zbase;
	float voz = (object.voz_base + scanline * object.voz_dy + startx * object.voz_dx) * zbase;
	int x;

	for (x = startx; x < extent.stopx; x++)
	{
		int u = (int)uoz;
		int v = (int)voz;
		int pixeloffs = (tex + (v >> 8) * 4096 + (u >> 8)) & endmask;
		if (pixeloffs >= m_texmask_size || !m_texmask[pixeloffs])
		{
			rgb_t rgb00 = palsource[m_texture[pixeloffs]];
			rgb_t rgb01 = palsource[m_texture[(pixeloffs + 1) & endmask]];
			rgb_t rgb10 = palsource[m_texture[(pixeloffs + 4096) & endmask]];
			rgb_t rgb11 = palsource[m_texture[(pixeloffs + 4097) & endmask]];
			rgb_t filtered = rgb_bilinear_filter(rgb00, rgb01, rgb10, rgb11, u, v);
			dest[x] = (filtered & 0x1f) | ((filtered & 0x1ff800) >> 6);
			zbuf[x] = zbufval;
		}

		/* advance texture params to the next pixel */
		uoz += uoz_step;
		voz += voz_step;
	}
}


void gaelco3d_renderer::render_normal(INT32 scanline, const extent_t &extent, const gaelco3d_object_data &object, int threadid)
{
	float ooz_dx = object.ooz_dx;
	float uoz_dx = object.uoz_dx;
	float voz_dx = object.voz_dx;
	offs_t endmask = m_texture_size - 1;
	const rgb_t *palsource = m_state.m_palette + object.color;
	UINT32 tex = object.tex;
	float z0 = object.z0;
	UINT16 *dest = &m_screenbits.pix16(scanline);
	UINT16 *zbuf = &m_zbuffer.pix16(scanline);
	int startx = extent.startx;
	float ooz = object.ooz_base + scanline * object.ooz_dy + startx * ooz_dx;
	float uoz = object.uoz_base + scanline * object.uoz_dy + startx * uoz_dx;
	float voz = object.voz_base + scanline * object.voz_dy + startx * voz_dx;
	int x;

	for (x = startx; x < extent.stopx; x++)
	{
		if (ooz > 0)
		{
			/* compute Z and check the Z buffer value first */
			float z = recip_approx(ooz);
			int zbufval = (int)(z0 * z);
			if (zbufval < zbuf[x])
			{
				int u = (int)(uoz * z);
				int v = (int)(voz * z);
				int pixeloffs = (tex + (v >> 8) * 4096 + (u >> 8)) & endmask;
				if (pixeloffs >= m_texmask_size || !m_texmask[pixeloffs])
				{
					rgb_t rgb00 = palsource[m_texture[pixeloffs]];
					rgb_t rgb01 = palsource[m_texture[(pixeloffs + 1) & endmask]];
					rgb_t rgb10 = palsource[m_texture[(pixeloffs + 4096) & endmask]];
					rgb_t rgb11 = palsource[m_texture[(pixeloffs + 4097) & endmask]];
					rgb_t filtered = rgb_bilinear_filter(rgb00, rgb01, rgb10, rgb11, u, v);
					dest[x] = (filtered & 0x1f) | ((filtered & 0x1ff800) >> 6);
					zbuf[x] = (zbufval < 0) ? -zbufval : zbufval;
				}
			}
		}

		/* advance texture params to the next pixel */
		ooz += ooz_dx;
		uoz += uoz_dx;
		voz += voz_dx;
	}
}


void gaelco3d_renderer::render_alphablend(INT32 scanline, const extent_t &extent, const gaelco3d_object_data &object, int threadid)
{
	float ooz_dx = object.ooz_dx;
	float uoz_dx = object.uoz_dx;
	float voz_dx = object.voz_dx;
	offs_t endmask = m_texture_size - 1;
	const rgb_t *palsource = m_state.m_palette + object.color;
	UINT32 tex = object.tex;
	float z0 = object.z0;
	UINT16 *dest = &m_screenbits.pix16(scanline);
	UINT16 *zbuf = &m_zbuffer.pix16(scanline);
	int startx = extent.startx;
	float ooz = object.ooz_base + object.ooz_dy * scanline + startx * ooz_dx;
	float uoz = object.uoz_base + object.uoz_dy * scanline + startx * uoz_dx;
	float voz = object.voz_base + object.voz_dy * scanline + startx * voz_dx;
	int x;

	for (x = startx; x < extent.stopx; x++)
	{
		if (ooz > 0)
		{
			/* compute Z and check the Z buffer value first */
			float z = recip_approx(ooz);
			int zbufval = (int)(z0 * z);
			if (zbufval < zbuf[x])
			{
				int u = (int)(uoz * z);
				int v = (int)(voz * z);
				int pixeloffs = (tex + (v >> 8) * 4096 + (u >> 8)) & endmask;
				if (pixeloffs >= m_texmask_size || !m_texmask[pixeloffs])
				{
					rgb_t rgb00 = palsource[m_texture[pixeloffs]];
					rgb_t rgb01 = palsource[m_texture[(pixeloffs + 1) & endmask]];
					rgb_t rgb10 = palsource[m_texture[(pixeloffs + 4096) & endmask]];
					rgb_t rgb11 = palsource[m_texture[(pixeloffs + 4097) & endmask]];
					rgb_t filtered = rgb_bilinear_filter(rgb00, rgb01, rgb10, rgb11, u, v) >> 1;
					dest[x] = ((filtered & 0x0f) | ((filtered & 0x0f7800) >> 6)) + ((dest[x] >> 1) & 0x3def);
					zbuf[x] = (zbufval < 0) ? -zbufval : zbufval;
				}
			}
		}

		/* advance texture params to the next pixel */
		ooz += ooz_dx;
		uoz += uoz_dx;
		voz += voz_dx;
	}
}


/*************************************
 *
 *  Scene rendering
 *
 *************************************/

void gaelco3d_render(screen_device &screen)
{
	gaelco3d_state *state = screen.machine().driver_data<gaelco3d_state>();
	/* wait for any queued stuff to complete */
	state->m_poly->wait("Time to render");

#if DISPLAY_STATS
{
	int scan = screen.vpos();
	popmessage("Polys = %4d  Timeleft = %3d", state->m_poly->polygons(), (state->m_lastscan < scan) ? (scan - state->m_lastscan) : (scan + (state->m_lastscan - screen.visible_area().max_y)));
}
#endif

	state->m_polydata_count = 0;
	state->m_lastscan = -1;
}



/*************************************
 *
 *  Renderer access
 *
 *************************************/

WRITE32_HANDLER( gaelco3d_render_w )
{
	gaelco3d_state *state = space->machine().driver_data<gaelco3d_state>();
	/* append the data to our buffer */
	state->m_polydata_buffer[state->m_polydata_count++] = data;
	if (state->m_polydata_count >= MAX_POLYDATA)
		fatalerror("Out of polygon buffer space!");

	/* if we've accumulated a completed poly set of data, queue it */
	if (!space->machine().video().skip_this_frame())
	{
		if (state->m_polydata_count >= 18 && (state->m_polydata_count % 2) == 1 && IS_POLYEND(state->m_polydata_buffer[state->m_polydata_count - 2]))
		{
			state->m_poly->render_poly(*space->machine().primary_screen, &state->m_polydata_buffer[0]);
			state->m_polydata_count = 0;
		}
		state->m_video_changed = TRUE;
	}

#if DISPLAY_STATS
	state->m_lastscan = space->machine().primary_screen->vpos();
#endif
}



/*************************************
 *
 *  Palette access
 *
 *************************************/

WRITE16_HANDLER( gaelco3d_paletteram_w )
{
	gaelco3d_state *state = space->machine().driver_data<gaelco3d_state>();
	state->m_poly->wait("Palette change");
	COMBINE_DATA(&space->machine().generic.paletteram.u16[offset]);
	state->m_palette[offset] = ((space->machine().generic.paletteram.u16[offset] & 0x7fe0) << 6) | (space->machine().generic.paletteram.u16[offset] & 0x1f);
}


WRITE32_HANDLER( gaelco3d_paletteram_020_w )
{
	gaelco3d_state *state = space->machine().driver_data<gaelco3d_state>();
	state->m_poly->wait("Palette change");
	COMBINE_DATA(&space->machine().generic.paletteram.u32[offset]);
	state->m_palette[offset*2+0] = ((space->machine().generic.paletteram.u32[offset] & 0x7fe00000) >> 10) | ((space->machine().generic.paletteram.u32[offset] & 0x1f0000) >> 16);
	state->m_palette[offset*2+1] = ((space->machine().generic.paletteram.u32[offset] & 0x7fe0) << 6) | (space->machine().generic.paletteram.u32[offset] & 0x1f);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

SCREEN_UPDATE_IND16( gaelco3d )
{
	gaelco3d_state *state = screen.machine().driver_data<gaelco3d_state>();
	int ret;

/*
    if (DISPLAY_TEXTURE && (screen.machine().input().code_pressed(KEYCODE_Z) || screen.machine().input().code_pressed(KEYCODE_X)))
    {
        static int xv = 0, yv = 0x1000;
        UINT8 *base = state->m_texture;
        int length = state->m_texture_size;

        if (screen.machine().input().code_pressed(KEYCODE_X))
        {
            base = state->m_texmask;
            length = state->m_texmask_size;
        }

        if (screen.machine().input().code_pressed(KEYCODE_LEFT) && xv >= 4)
            xv -= 4;
        if (screen.machine().input().code_pressed(KEYCODE_RIGHT) && xv < 4096 - 4)
            xv += 4;

        if (screen.machine().input().code_pressed(KEYCODE_UP) && yv >= 4)
            yv -= 4;
        if (screen.machine().input().code_pressed(KEYCODE_DOWN) && yv < 0x40000)
            yv += 4;

        for (y = cliprect.min_y; y <= cliprect.max_y; y++)
        {
            UINT16 *dest = &bitmap.pix16(y);
            for (x = cliprect.min_x; x <= cliprect.max_x; x++)
            {
                int offs = (yv + y - cliprect.min_y) * 4096 + xv + x - cliprect.min_x;
                if (offs < length)
                    dest[x] = base[offs];
                else
                    dest[x] = 0;
            }
        }
        popmessage("(%04X,%04X)", xv, yv);
    }
    else*/
	{
		if (state->m_video_changed)
			copybitmap(bitmap, state->m_poly->screenbits(), 0,1, 0,0, cliprect);
		ret = state->m_video_changed;
		state->m_video_changed = FALSE;
	}

	logerror("---update---\n");
	return (!ret);
}
