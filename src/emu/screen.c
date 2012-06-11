/***************************************************************************

    screen.c

    Core MAME screen device.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "png.h"
#include "rendutil.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE						(0)
#define LOG_PARTIAL_UPDATES(x)		do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SCREEN = &device_creator<screen_device>;

const attotime screen_device::DEFAULT_FRAME_PERIOD(attotime::from_hz(DEFAULT_FRAME_RATE));



//**************************************************************************
//  SCREEN DEVICE
//**************************************************************************

//-------------------------------------------------
//  screen_device - constructor
//-------------------------------------------------

screen_device::screen_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SCREEN, "Video Screen", tag, owner, clock),
	  m_type(SCREEN_TYPE_RASTER),
	  m_oldstyle_vblank_supplied(false),
	  m_refresh(0),
	  m_vblank(0),
	  m_xoffset(0.0f),
	  m_yoffset(0.0f),
	  m_xscale(1.0f),
	  m_yscale(1.0f),
	  m_container(NULL),
	  m_width(100),
	  m_height(100),
	  m_visarea(0, 99, 0, 99),
	  m_curbitmap(0),
	  m_curtexture(0),
	  m_changed(true),
	  m_last_partial_scan(0),
	  m_frame_period(DEFAULT_FRAME_PERIOD.as_attoseconds()),
	  m_scantime(1),
	  m_pixeltime(1),
	  m_vblank_period(0),
	  m_vblank_start_time(attotime::zero),
	  m_vblank_end_time(attotime::zero),
	  m_vblank_begin_timer(NULL),
	  m_vblank_end_timer(NULL),
	  m_scanline0_timer(NULL),
	  m_scanline_timer(NULL),
	  m_frame_number(0),
	  m_partial_updates_this_frame(0)
{
	memset(m_texture, 0, sizeof(m_texture));
}


//-------------------------------------------------
//  ~screen_device - destructor
//-------------------------------------------------

screen_device::~screen_device()
{
}


//-------------------------------------------------
//  static_set_type - configuration helper
//  to set the screen type
//-------------------------------------------------

void screen_device::static_set_type(device_t &device, screen_type_enum type)
{
	downcast<screen_device &>(device).m_type = type;
}


//-------------------------------------------------
//  static_set_raw - configuration helper
//  to set the raw screen parameters
//-------------------------------------------------

void screen_device::static_set_raw(device_t &device, UINT32 pixclock, UINT16 htotal, UINT16 hbend, UINT16 hbstart, UINT16 vtotal, UINT16 vbend, UINT16 vbstart)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_refresh = HZ_TO_ATTOSECONDS(pixclock) * htotal * vtotal;
	screen.m_vblank = screen.m_refresh / vtotal * (vtotal - (vbstart - vbend));
	screen.m_width = htotal;
	screen.m_height = vtotal;
	screen.m_visarea.set(hbend, hbstart - 1, vbend, vbstart - 1);
}


//-------------------------------------------------
//  static_set_refresh - configuration helper
//  to set the refresh rate
//-------------------------------------------------

void screen_device::static_set_refresh(device_t &device, attoseconds_t rate)
{
	downcast<screen_device &>(device).m_refresh = rate;
}


//-------------------------------------------------
//  static_set_vblank_time - configuration helper
//  to set the VBLANK duration
//-------------------------------------------------

void screen_device::static_set_vblank_time(device_t &device, attoseconds_t time)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_vblank = time;
	screen.m_oldstyle_vblank_supplied = true;
}


//-------------------------------------------------
//  static_set_size - configuration helper to set
//  the width/height of the screen
//-------------------------------------------------

void screen_device::static_set_size(device_t &device, UINT16 width, UINT16 height)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_width = width;
	screen.m_height = height;
}


//-------------------------------------------------
//  static_set_visarea - configuration helper to
//  set the visible area of the screen
//-------------------------------------------------

void screen_device::static_set_visarea(device_t &device, INT16 minx, INT16 maxx, INT16 miny, INT16 maxy)
{
	downcast<screen_device &>(device).m_visarea.set(minx, maxx, miny, maxy);
}


//-------------------------------------------------
//  static_set_default_position - configuration
//  helper to set the default position and scale
//  factors for the screen
//-------------------------------------------------

void screen_device::static_set_default_position(device_t &device, double xscale, double xoffs, double yscale, double yoffs)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_xscale = xscale;
	screen.m_xoffset = xoffs;
	screen.m_yscale = yscale;
	screen.m_yoffset = yoffs;
}


//-------------------------------------------------
//  static_set_screen_update - set the legacy
//  screen update callback in the device
//  configuration
//-------------------------------------------------

void screen_device::static_set_screen_update(device_t &device, screen_update_ind16_delegate callback)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_screen_update_ind16 = callback;
	screen.m_screen_update_rgb32 = screen_update_rgb32_delegate();
}

void screen_device::static_set_screen_update(device_t &device, screen_update_rgb32_delegate callback)
{
	screen_device &screen = downcast<screen_device &>(device);
	screen.m_screen_update_ind16 = screen_update_ind16_delegate();
	screen.m_screen_update_rgb32 = callback;
}


//-------------------------------------------------
//  static_set_screen_vblank - set the screen
//  VBLANK callback in the device configuration
//-------------------------------------------------

void screen_device::static_set_screen_vblank(device_t &device, screen_vblank_delegate callback)
{
	downcast<screen_device &>(device).m_screen_vblank = callback;
}


//-------------------------------------------------
//  device_validity_check - verify device
//  configuration
//-------------------------------------------------

void screen_device::device_validity_check(validity_checker &valid) const
{
	// sanity check dimensions
	if (m_width <= 0 || m_height <= 0)
		mame_printf_error("Invalid display dimensions\n");

	// sanity check display area
	if (m_type != SCREEN_TYPE_VECTOR)
	{
		if (m_visarea.empty() || m_visarea.max_x >= m_width || m_visarea.max_y >= m_height)
			mame_printf_error("Invalid display area\n");

		// sanity check screen formats
		if (m_screen_update_ind16.isnull() && m_screen_update_rgb32.isnull())
			mame_printf_error("Missing SCREEN_UPDATE function\n");
	}

	// check for zero frame rate
	if (m_refresh == 0)
		mame_printf_error("Invalid (zero) refresh rate\n");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void screen_device::device_start()
{
	// bind our handlers
	m_screen_update_ind16.bind_relative_to(*owner());
	m_screen_update_rgb32.bind_relative_to(*owner());
	m_screen_vblank.bind_relative_to(*owner());

	// configure bitmap formats and allocate screen bitmaps
	texture_format texformat = !m_screen_update_ind16.isnull() ? TEXFORMAT_PALETTE16 : TEXFORMAT_RGB32;
	for (int index = 0; index < ARRAY_LENGTH(m_bitmap); index++)
	{
		m_bitmap[index].set_format(format(), texformat);
		register_screen_bitmap(m_bitmap[index]);
	}

	// allocate raw textures
	m_texture[0] = machine().render().texture_alloc();
	m_texture[1] = machine().render().texture_alloc();

	// configure the default cliparea
	render_container::user_settings settings;
	m_container->get_user_settings(settings);
	settings.m_xoffset = m_xoffset;
	settings.m_yoffset = m_yoffset;
	settings.m_xscale = m_xscale;
	settings.m_yscale = m_yscale;
	m_container->set_user_settings(settings);

	// allocate the VBLANK timers
	m_vblank_begin_timer = timer_alloc(TID_VBLANK_START);
	m_vblank_end_timer = timer_alloc(TID_VBLANK_END);

	// allocate a timer to reset partial updates
	m_scanline0_timer = timer_alloc(TID_SCANLINE0);

	// allocate a timer to generate per-scanline updates
	if ((machine().config().m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0)
		m_scanline_timer = timer_alloc(TID_SCANLINE);

	// configure the screen with the default parameters
	configure(m_width, m_height, m_visarea, m_refresh);

	// reset VBLANK timing
	m_vblank_start_time = attotime::zero;
	m_vblank_end_time = attotime(0, m_vblank_period);

	// start the timer to generate per-scanline updates
	if ((machine().config().m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0)
		m_scanline_timer->adjust(time_until_pos(0));

	// create burn-in bitmap
	if (machine().options().burnin())
	{
		int width, height;
		if (sscanf(machine().options().snap_size(), "%dx%d", &width, &height) != 2 || width == 0 || height == 0)
			width = height = 300;
		m_burnin.allocate(width, height);
		m_burnin.fill(0);
	}

	// load the effect overlay
	const char *overname = machine().options().effect();
	if (overname != NULL && strcmp(overname, "none") != 0)
		load_effect_overlay(overname);

	// register items for saving
	save_item(NAME(m_width));
	save_item(NAME(m_height));
	save_item(NAME(m_visarea.min_x));
	save_item(NAME(m_visarea.min_y));
	save_item(NAME(m_visarea.max_x));
	save_item(NAME(m_visarea.max_y));
	save_item(NAME(m_last_partial_scan));
	save_item(NAME(m_frame_period));
	save_item(NAME(m_scantime));
	save_item(NAME(m_pixeltime));
	save_item(NAME(m_vblank_period));
	save_item(NAME(m_vblank_start_time));
	save_item(NAME(m_vblank_end_time));
	save_item(NAME(m_frame_number));
}


//-------------------------------------------------
//  device_stop - clean up before the machine goes
//  away
//-------------------------------------------------

void screen_device::device_stop()
{
	machine().render().texture_free(m_texture[0]);
	machine().render().texture_free(m_texture[1]);
	if (m_burnin.valid())
		finalize_burnin();
}


//-------------------------------------------------
//  device_post_load - device-specific update
//  after a save state is loaded
//-------------------------------------------------

void screen_device::device_post_load()
{
	realloc_screen_bitmaps();
}


//-------------------------------------------------
//  device_timer - called whenever a device timer
//  fires
//-------------------------------------------------

void screen_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		// signal VBLANK start
		case TID_VBLANK_START:
			vblank_begin();
			break;

		// signal VBLANK end
		case TID_VBLANK_END:
			vblank_end();
			break;

		// first visible scanline
		case TID_SCANLINE0:
			reset_partial_updates();
			break;

		// subsequent scanlines when scanline updates are enabled
		case TID_SCANLINE:

			// force a partial update to the current scanline
			update_partial(param);

			// compute the next visible scanline
			param++;
			if (param > m_visarea.max_y)
				param = m_visarea.min_y;
			m_scanline_timer->adjust(time_until_pos(param), param);
			break;
	}
}


//-------------------------------------------------
//  configure - configure screen parameters
//-------------------------------------------------

void screen_device::configure(int width, int height, const rectangle &visarea, attoseconds_t frame_period)
{
	// validate arguments
	assert(width > 0);
	assert(height > 0);
	assert(visarea.min_x >= 0);
	assert(visarea.min_y >= 0);
//  assert(visarea.max_x < width);
//  assert(visarea.max_y < height);
	assert(m_type == SCREEN_TYPE_VECTOR || visarea.min_x < width);
	assert(m_type == SCREEN_TYPE_VECTOR || visarea.min_y < height);
	assert(frame_period > 0);

	// fill in the new parameters
	m_width = width;
	m_height = height;
	m_visarea = visarea;

	// reallocate bitmap if necessary
	realloc_screen_bitmaps();

	// compute timing parameters
	m_frame_period = frame_period;
	m_scantime = frame_period / height;
	m_pixeltime = frame_period / (height * width);

	// if there has been no VBLANK time specified in the MACHINE_DRIVER, compute it now
    // from the visible area, otherwise just used the supplied value
	if (m_vblank == 0 && !m_oldstyle_vblank_supplied)
		m_vblank_period = m_scantime * (height - visarea.height());
	else
		m_vblank_period = m_vblank;

	// if we are on scanline 0 already, reset the update timer immediately
	// otherwise, defer until the next scanline 0
	if (vpos() == 0)
		m_scanline0_timer->adjust(attotime::zero);
	else
		m_scanline0_timer->adjust(time_until_pos(0));

	// start the VBLANK timer
	m_vblank_begin_timer->adjust(time_until_vblank_start());

	// adjust speed if necessary
	machine().video().update_refresh_speed();
}


//-------------------------------------------------
//  reset_origin - reset the timing such that the
//  given (x,y) occurs at the current time
//-------------------------------------------------

void screen_device::reset_origin(int beamy, int beamx)
{
	// compute the effective VBLANK start/end times
	attotime curtime = machine().time();
	m_vblank_end_time = curtime - attotime(0, beamy * m_scantime + beamx * m_pixeltime);
	m_vblank_start_time = m_vblank_end_time - attotime(0, m_vblank_period);

	// if we are resetting relative to (0,0) == VBLANK end, call the
	// scanline 0 timer by hand now; otherwise, adjust it for the future
	if (beamy == 0 && beamx == 0)
		reset_partial_updates();
	else
		m_scanline0_timer->adjust(time_until_pos(0));

	// if we are resetting relative to (visarea.max_y + 1, 0) == VBLANK start,
	// call the VBLANK start timer now; otherwise, adjust it for the future
	if (beamy == m_visarea.max_y + 1 && beamx == 0)
		vblank_begin();
	else
		m_vblank_begin_timer->adjust(time_until_vblank_start());
}


//-------------------------------------------------
//  realloc_screen_bitmaps - reallocate bitmaps
//  and textures as necessary
//-------------------------------------------------

void screen_device::realloc_screen_bitmaps()
{
	// doesn't apply for vector games
	if (m_type == SCREEN_TYPE_VECTOR)
		return;

	// determine effective size to allocate
	INT32 effwidth = MAX(m_width, m_visarea.max_x + 1);
	INT32 effheight = MAX(m_height, m_visarea.max_y + 1);

	// reize all registered screen bitmaps
	for (auto_bitmap_item *item = m_auto_bitmap_list.first(); item != NULL; item = item->next())
		item->m_bitmap.resize(effwidth, effheight);

	// re-set up textures
	m_texture[0]->set_bitmap(m_bitmap[0], m_visarea, m_bitmap[0].texformat());
	m_texture[1]->set_bitmap(m_bitmap[1], m_visarea, m_bitmap[1].texformat());
}


//-------------------------------------------------
//  set_visible_area - just set the visible area
//-------------------------------------------------

void screen_device::set_visible_area(int min_x, int max_x, int min_y, int max_y)
{
	rectangle visarea(min_x, max_x, min_y, max_y);
	assert(!visarea.empty());
	configure(m_width, m_height, visarea, m_frame_period);
}


//-------------------------------------------------
//  update_partial - perform a partial update from
//  the last scanline up to and including the
//  specified scanline
//-----------------------------------------------*/

bool screen_device::update_partial(int scanline)
{
	// validate arguments
	assert(scanline >= 0);

	LOG_PARTIAL_UPDATES(("Partial: update_partial(%s, %d): ", tag(), scanline));

	// these two checks only apply if we're allowed to skip frames
	if (!(machine().config().m_video_attributes & VIDEO_ALWAYS_UPDATE))
	{
		// if skipping this frame, bail
		if (machine().video().skip_this_frame())
		{
			LOG_PARTIAL_UPDATES(("skipped due to frameskipping\n"));
			return FALSE;
		}

		// skip if this screen is not visible anywhere
		if (!machine().render().is_live(*this))
		{
			LOG_PARTIAL_UPDATES(("skipped because screen not live\n"));
			return FALSE;
		}
	}

	// skip if less than the lowest so far
	if (scanline < m_last_partial_scan)
	{
		LOG_PARTIAL_UPDATES(("skipped because less than previous\n"));
		return FALSE;
	}

	// set the start/end scanlines
	rectangle clip = m_visarea;
	if (m_last_partial_scan > clip.min_y)
		clip.min_y = m_last_partial_scan;
	if (scanline < clip.max_y)
		clip.max_y = scanline;

	// render if necessary
	bool result = false;
	if (clip.min_y <= clip.max_y)
	{
		UINT32 flags = UPDATE_HAS_NOT_CHANGED;

		g_profiler.start(PROFILER_VIDEO);
		LOG_PARTIAL_UPDATES(("updating %d-%d\n", clip.min_y, clip.max_y));

		flags = 0;
		screen_bitmap &curbitmap = m_bitmap[m_curbitmap];
		switch (curbitmap.format())
		{
			default:
			case BITMAP_FORMAT_IND16:	flags = m_screen_update_ind16(*this, curbitmap.as_ind16(), clip);	break;
			case BITMAP_FORMAT_RGB32:	flags = m_screen_update_rgb32(*this, curbitmap.as_rgb32(), clip);	break;
		}

		m_partial_updates_this_frame++;
		g_profiler.stop();

		// if we modified the bitmap, we have to commit
		m_changed |= ~flags & UPDATE_HAS_NOT_CHANGED;
		result = true;
	}

	// remember where we left off
	m_last_partial_scan = scanline + 1;
	return result;
}


//-------------------------------------------------
//  update_now - perform an update from the last
//  beam position up to the current beam position
//-------------------------------------------------

void screen_device::update_now()
{
	int current_vpos = vpos();
	int current_hpos = hpos();

	// since we can currently update only at the scanline
    // level, we are trying to do the right thing by
    // updating including the current scanline, only if the
    // beam is past the halfway point horizontally.
    // If the beam is in the first half of the scanline,
    // we only update up to the previous scanline.
    // This minimizes the number of pixels that might be drawn
    // incorrectly until we support a pixel level granularity
	if (current_hpos < (m_width / 2) && current_vpos > 0)
		current_vpos = current_vpos - 1;

	update_partial(current_vpos);
}


//-------------------------------------------------
//  reset_partial_updates - reset the partial
//  updating state
//-------------------------------------------------

void screen_device::reset_partial_updates()
{
	m_last_partial_scan = 0;
	m_partial_updates_this_frame = 0;
	m_scanline0_timer->adjust(time_until_pos(0));
}


//-------------------------------------------------
//  vpos - returns the current vertical position
//  of the beam
//-------------------------------------------------

int screen_device::vpos() const
{
	attoseconds_t delta = (machine().time() - m_vblank_start_time).as_attoseconds();
	int vpos;

	// round to the nearest pixel
	delta += m_pixeltime / 2;

	// compute the v position relative to the start of VBLANK
	vpos = delta / m_scantime;

	// adjust for the fact that VBLANK starts at the bottom of the visible area
	return (m_visarea.max_y + 1 + vpos) % m_height;
}


//-------------------------------------------------
//  hpos - returns the current horizontal position
//  of the beam
//-------------------------------------------------

int screen_device::hpos() const
{
	attoseconds_t delta = (machine().time() - m_vblank_start_time).as_attoseconds();

	// round to the nearest pixel
	delta += m_pixeltime / 2;

	// compute the v position relative to the start of VBLANK
	int vpos = delta / m_scantime;

	// subtract that from the total time
	delta -= vpos * m_scantime;

	// return the pixel offset from the start of this scanline
	return delta / m_pixeltime;
}


//-------------------------------------------------
//  time_until_pos - returns the amount of time
//  remaining until the beam is at the given
//  hpos,vpos
//-------------------------------------------------

attotime screen_device::time_until_pos(int vpos, int hpos) const
{
	// validate arguments
	assert(vpos >= 0);
	assert(hpos >= 0);

	// since we measure time relative to VBLANK, compute the scanline offset from VBLANK
	vpos += m_height - (m_visarea.max_y + 1);
	vpos %= m_height;

	// compute the delta for the given X,Y position
	attoseconds_t targetdelta = (attoseconds_t)vpos * m_scantime + (attoseconds_t)hpos * m_pixeltime;

	// if we're past that time (within 1/2 of a pixel), head to the next frame
	attoseconds_t curdelta = (machine().time() - m_vblank_start_time).as_attoseconds();
	if (targetdelta <= curdelta + m_pixeltime / 2)
		targetdelta += m_frame_period;
	while (targetdelta <= curdelta)
		targetdelta += m_frame_period;

	// return the difference
	return attotime(0, targetdelta - curdelta);
}


//-------------------------------------------------
//  time_until_vblank_end - returns the amount of
//  time remaining until the end of the current
//  VBLANK (if in progress) or the end of the next
//  VBLANK
//-------------------------------------------------

attotime screen_device::time_until_vblank_end() const
{
	// if we are in the VBLANK region, compute the time until the end of the current VBLANK period
	attotime target_time = m_vblank_end_time;
	if (!vblank())
		target_time += attotime(0, m_frame_period);
	return target_time - machine().time();
}


//-------------------------------------------------
//  register_vblank_callback - registers a VBLANK
//  callback
//-------------------------------------------------

void screen_device::register_vblank_callback(vblank_state_delegate vblank_callback)
{
	// validate arguments
	assert(!vblank_callback.isnull());

	// check if we already have this callback registered
	callback_item *item;
	for (item = m_callback_list.first(); item != NULL; item = item->next())
		if (item->m_callback == vblank_callback)
			break;

	// if not found, register
	if (item == NULL)
		m_callback_list.append(*global_alloc(callback_item(vblank_callback)));
}


//-------------------------------------------------
//  register_screen_bitmap - registers a bitmap
//  that should track the screen size
//-------------------------------------------------

void screen_device::register_screen_bitmap(bitmap_t &bitmap)
{
	// append to the list
	m_auto_bitmap_list.append(*global_alloc(auto_bitmap_item(bitmap)));

	// if allocating now, just do it
	bitmap.allocate(width(), height());
	bitmap.set_palette(machine().palette);
}


//-------------------------------------------------
//  vblank_port_read - custom port handler to
//  return a VBLANK state
//-------------------------------------------------

int screen_device::vblank_port_read()
{
	return vblank() ? -1 : 0;
}


//-------------------------------------------------
//  vblank_begin - call any external callbacks to
//  signal the VBLANK period has begun
//-------------------------------------------------

void screen_device::vblank_begin()
{
	// reset the starting VBLANK time
	m_vblank_start_time = machine().time();
	m_vblank_end_time = m_vblank_start_time + attotime(0, m_vblank_period);

	// if this is the primary screen and we need to update now
	if (this == machine().primary_screen && !(machine().config().m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		machine().video().frame_update();

	// call the screen specific callbacks
	for (callback_item *item = m_callback_list.first(); item != NULL; item = item->next())
		item->m_callback(*this, true);
	if (!m_screen_vblank.isnull())
		m_screen_vblank(*this, true);

	// reset the VBLANK start timer for the next frame
	m_vblank_begin_timer->adjust(time_until_vblank_start());

	// if no VBLANK period, call the VBLANK end callback immedietely, otherwise reset the timer
	if (m_vblank_period == 0)
		vblank_end();
	else
		m_vblank_end_timer->adjust(time_until_vblank_end());
}


//-------------------------------------------------
//  vblank_end - call any external callbacks to
//  signal the VBLANK period has ended
//-------------------------------------------------

void screen_device::vblank_end()
{
	// call the screen specific callbacks
	for (callback_item *item = m_callback_list.first(); item != NULL; item = item->next())
		item->m_callback(*this, false);
	if (!m_screen_vblank.isnull())
		m_screen_vblank(*this, false);

	// if this is the primary screen and we need to update now
	if (this == machine().primary_screen && (machine().config().m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		machine().video().frame_update();

	// increment the frame number counter
	m_frame_number++;
}


//-------------------------------------------------
//  update_quads - set up the quads for this
//  screen
//-------------------------------------------------

bool screen_device::update_quads()
{
	// only update if live
	if (machine().render().is_live(*this))
	{
		// only update if empty and not a vector game; otherwise assume the driver did it directly
		if (m_type != SCREEN_TYPE_VECTOR && (machine().config().m_video_attributes & VIDEO_SELF_RENDER) == 0)
		{
			// if we're not skipping the frame and if the screen actually changed, then update the texture
			if (!machine().video().skip_this_frame() && m_changed)
			{
				m_texture[m_curbitmap]->set_bitmap(m_bitmap[m_curbitmap], m_visarea, m_bitmap[m_curbitmap].texformat());
				m_curtexture = m_curbitmap;
				m_curbitmap = 1 - m_curbitmap;
			}

			// create an empty container with a single quad
			m_container->empty();
			m_container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(0xff,0xff,0xff,0xff), m_texture[m_curtexture], PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));
		}
	}

	// reset the screen changed flags
	bool result = m_changed;
	m_changed = false;
	return result;
}


//-------------------------------------------------
//  update_burnin - update the burnin bitmap
//-------------------------------------------------

void screen_device::update_burnin()
{
#undef rand
	if (!m_burnin.valid())
		return;

	screen_bitmap &curbitmap = m_bitmap[m_curtexture];
	if (!curbitmap.valid())
		return;

	int srcwidth = curbitmap.width();
	int srcheight = curbitmap.height();
	int dstwidth = m_burnin.width();
	int dstheight = m_burnin.height();
	int xstep = (srcwidth << 16) / dstwidth;
	int ystep = (srcheight << 16) / dstheight;
	int xstart = ((UINT32)rand() % 32767) * xstep / 32767;
	int ystart = ((UINT32)rand() % 32767) * ystep / 32767;
	int srcx, srcy;
	int x, y;

	switch (curbitmap.format())
	{
		default:
		case BITMAP_FORMAT_IND16:
		{
			// iterate over rows in the destination
			bitmap_ind16 &srcbitmap = curbitmap.as_ind16();
			for (y = 0, srcy = ystart; y < dstheight; y++, srcy += ystep)
			{
				UINT64 *dst = &m_burnin.pix64(y);
				const UINT16 *src = &srcbitmap.pix16(srcy >> 16);
				const rgb_t *palette = palette_entry_list_adjusted(machine().palette);
				for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
				{
					rgb_t pixel = palette[src[srcx >> 16]];
					dst[x] += RGB_GREEN(pixel) + RGB_RED(pixel) + RGB_BLUE(pixel);
				}
			}
			break;
		}

		case BITMAP_FORMAT_RGB32:
		{
			// iterate over rows in the destination
			bitmap_rgb32 &srcbitmap = curbitmap.as_rgb32();
			for (y = 0, srcy = ystart; y < dstheight; y++, srcy += ystep)
			{
				UINT64 *dst = &m_burnin.pix64(y);
				const UINT32 *src = &srcbitmap.pix32(srcy >> 16);
				for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
				{
					rgb_t pixel = src[srcx >> 16];
					dst[x] += RGB_GREEN(pixel) + RGB_RED(pixel) + RGB_BLUE(pixel);
				}
			}
			break;
		}
	}
}


//-------------------------------------------------
//  finalize_burnin - finalize the burnin bitmap
//-------------------------------------------------

void screen_device::finalize_burnin()
{
	if (!m_burnin.valid())
		return;

	// compute the scaled visible region
	rectangle scaledvis;
	scaledvis.min_x = m_visarea.min_x * m_burnin.width() / m_width;
	scaledvis.max_x = m_visarea.max_x * m_burnin.width() / m_width;
	scaledvis.min_y = m_visarea.min_y * m_burnin.height() / m_height;
	scaledvis.max_y = m_visarea.max_y * m_burnin.height() / m_height;

	// wrap a bitmap around the memregion we care about
	bitmap_argb32 finalmap(scaledvis.width(), scaledvis.height());
	int srcwidth = m_burnin.width();
	int srcheight = m_burnin.height();
	int dstwidth = finalmap.width();
	int dstheight = finalmap.height();
	int xstep = (srcwidth << 16) / dstwidth;
	int ystep = (srcheight << 16) / dstheight;

	// find the maximum value
	UINT64 minval = ~(UINT64)0;
	UINT64 maxval = 0;
	for (int y = 0; y < srcheight; y++)
	{
		UINT64 *src = &m_burnin.pix64(y);
		for (int x = 0; x < srcwidth; x++)
		{
			minval = MIN(minval, src[x]);
			maxval = MAX(maxval, src[x]);
		}
	}

	if (minval == maxval)
		return;

	// now normalize and convert to RGB
	for (int y = 0, srcy = 0; y < dstheight; y++, srcy += ystep)
	{
		UINT64 *src = &m_burnin.pix64(srcy >> 16);
		UINT32 *dst = &finalmap.pix32(y);
		for (int x = 0, srcx = 0; x < dstwidth; x++, srcx += xstep)
		{
			int brightness = (UINT64)(maxval - src[srcx >> 16]) * 255 / (maxval - minval);
			dst[x] = MAKE_ARGB(0xff, brightness, brightness, brightness);
		}
	}

	// write the final PNG

	// compute the name and create the file
	emu_file file(machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	file_error filerr = file.open(machine().basename(), PATH_SEPARATOR "burnin-", tag(), ".png") ;
	if (filerr == FILERR_NONE)
	{
		png_info pnginfo = { 0 };
//      png_error pngerr;
		char text[256];

		// add two text entries describing the image
		sprintf(text,"%s %s", emulator_info::get_appname(), build_version);
		png_add_text(&pnginfo, "Software", text);
		sprintf(text, "%s %s", machine().system().manufacturer, machine().system().description);
		png_add_text(&pnginfo, "System", text);

		// now do the actual work
		png_write_bitmap(file, &pnginfo, finalmap, 0, NULL);

		// free any data allocated
		png_free(&pnginfo);
	}
}


//-------------------------------------------------
//  finalize_burnin - finalize the burnin bitmap
//-------------------------------------------------

void screen_device::load_effect_overlay(const char *filename)
{
	// ensure that there is a .png extension
	astring fullname(filename);
	int extension = fullname.rchr(0, '.');
	if (extension != -1)
		fullname.del(extension, -1);
	fullname.cat(".png");

	// load the file
	emu_file file(machine().options().art_path(), OPEN_FLAG_READ);
	render_load_png(m_screen_overlay_bitmap, file, NULL, fullname);
	if (m_screen_overlay_bitmap.valid())
		m_container->set_overlay(&m_screen_overlay_bitmap);
	else
		mame_printf_warning("Unable to load effect PNG file '%s'\n", fullname.cstr());
}
