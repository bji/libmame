/**********************************************************************

    RCA CDP1869/1870/1876 Video Interface System (VIS) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - white noise
    - scanline based update
    - CMSEL output

*/

#include "emu.h"
#include "cdp1869.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define CDP1869_WEIGHT_RED		30 // % of max luminance
#define CDP1869_WEIGHT_GREEN	59
#define CDP1869_WEIGHT_BLUE		11

#define CDP1869_COLUMNS_HALF	20
#define CDP1869_COLUMNS_FULL	40
#define CDP1869_ROWS_HALF		12
#define CDP1869_ROWS_FULL_PAL	25
#define CDP1869_ROWS_FULL_NTSC	24

enum
{
	CDB0 = 0,
	CDB1,
	CDB2,
	CDB3,
	CDB4,
	CDB5,
	CCB0,
	CCB1
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type CDP1869 = cdp1869_device_config::static_alloc_device_config;


// default address map
static ADDRESS_MAP_START( cdp1869, 0, 8 )
	AM_RANGE(0x000, 0x7ff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  cdp1869_device_config - constructor
//-------------------------------------------------

cdp1869_device_config::cdp1869_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "RCA CDP1869", tag, owner, clock),
	  device_config_sound_interface(mconfig, *this),
	  device_config_memory_interface(mconfig, *this),
	  m_space_config("pageram", ENDIANNESS_LITTLE, 8, 11, 0, NULL, *ADDRESS_MAP_NAME(cdp1869))
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *cdp1869_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(cdp1869_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *cdp1869_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, cdp1869_device(machine, *this));
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *cdp1869_device_config::memory_space_config(int spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cdp1869_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const cdp1869_interface *intf = reinterpret_cast<const cdp1869_interface *>(static_config());
	if (intf != NULL)
		*static_cast<cdp1869_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&in_pal_ntsc_func, 0, sizeof(in_pal_ntsc_func));
		memset(&out_prd_func, 0, sizeof(out_prd_func));
		in_pcb_func = NULL;
		in_char_ram_func = NULL;
		out_char_ram_func = NULL;
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  is_ntsc - is device in NTSC mode
//-------------------------------------------------

inline bool cdp1869_device::is_ntsc()
{
	return devcb_call_read_line(&m_in_pal_ntsc_func) ? false : true;
}


//-------------------------------------------------
//  read_page_ram_byte - read a page RAM byte at
//  the given address
//-------------------------------------------------

inline UINT8 cdp1869_device::read_page_ram_byte(offs_t pma)
{
	return space()->read_byte(pma);
}


//-------------------------------------------------
//  write_page_ram_byte - write a page RAM byte at
//  the given address
//-------------------------------------------------

inline void cdp1869_device::write_page_ram_byte(offs_t pma, UINT8 data)
{
	space()->write_byte(pma, data);
}


//-------------------------------------------------
//  read_char_ram_byte - read a char RAM byte at
//  the given address
//-------------------------------------------------

inline UINT8 cdp1869_device::read_char_ram_byte(offs_t pma, offs_t cma, UINT8 pmd)
{
	UINT8 data = 0;

	if (m_in_char_ram_func != NULL)
	{
		data = m_in_char_ram_func(this, pma, cma, pmd);
	}

	return data;
}


//-------------------------------------------------
//  write_char_ram_byte - write a char RAM byte at
//  the given address
//-------------------------------------------------

inline void cdp1869_device::write_char_ram_byte(offs_t pma, offs_t cma, UINT8 pmd, UINT8 data)
{
	if (m_out_char_ram_func != NULL)
	{
		m_out_char_ram_func(this, pma, cma, pmd, data);
	}
}


//-------------------------------------------------
//  read_pcb - read page control bit
//-------------------------------------------------

inline int cdp1869_device::read_pcb(offs_t pma, offs_t cma, UINT8 pmd)
{
	int pcb = 0;

	if (m_in_pcb_func != NULL)
	{
		pcb = m_in_pcb_func(this, pma, cma, pmd);
	}

	return pcb;
}


//-------------------------------------------------
//  update_prd_changed_timer -
//-------------------------------------------------

inline void cdp1869_device::update_prd_changed_timer()
{
	int start = CDP1869_SCANLINE_PREDISPLAY_START_PAL;
	int end = CDP1869_SCANLINE_PREDISPLAY_END_PAL;
	int next_state;
	int scanline = m_screen->vpos();
	int next_scanline;

	if (is_ntsc())
	{
		start = CDP1869_SCANLINE_PREDISPLAY_START_NTSC;
		end = CDP1869_SCANLINE_PREDISPLAY_END_NTSC;
	}

	if (scanline < start)
	{
		next_scanline = start;
		next_state = ASSERT_LINE;
	}
	else if (scanline < end)
	{
		next_scanline = end;
		next_state = CLEAR_LINE;
	}
	else
	{
		next_scanline = start;
		next_state = ASSERT_LINE;
	}

	if (m_dispoff)
	{
		next_state = CLEAR_LINE;
	}

	attotime duration = m_screen->time_until_pos(next_scanline);
	timer_adjust_oneshot(m_prd_timer, duration, next_state);
}


//-------------------------------------------------
//  get_rgb - get RGB value
//-------------------------------------------------

inline rgb_t cdp1869_device::get_rgb(int i, int c, int l)
{
	int luma = 0, r, g, b;

	luma += (l & 4) ? CDP1869_WEIGHT_RED : 0;
	luma += (l & 1) ? CDP1869_WEIGHT_GREEN : 0;
	luma += (l & 2) ? CDP1869_WEIGHT_BLUE : 0;

	luma = (luma * 0xff) / 100;

	r = (c & 4) ? luma : 0;
	g = (c & 1) ? luma : 0;
	b = (c & 2) ? luma : 0;

	return MAKE_RGB(r, g, b);
}


//-------------------------------------------------
//  get_lines - get number of character lines
//-------------------------------------------------

inline int cdp1869_device::get_lines()
{
	if (m_line16 && !m_dblpage)
	{
		return 16;
	}
	else if (!m_line9)
	{
		return 9;
	}
	else
	{
		return 8;
	}
}


//-------------------------------------------------
//  get_pmemsize - get page memory size
//-------------------------------------------------

inline UINT16 cdp1869_device::get_pmemsize(int cols, int rows)
{
	int pmemsize = cols * rows;

	if (m_dblpage) pmemsize *= 2;
	if (m_line16) pmemsize *= 2;

	return pmemsize;
}


//-------------------------------------------------
//  get_pma - get page memory address
//-------------------------------------------------

inline UINT16 cdp1869_device::get_pma()
{
	if (m_dblpage)
	{
		return m_pma;
	}
	else
	{
		return m_pma & 0x3ff;
	}
}


//-------------------------------------------------
//  get_pen - get pen for color bits
//-------------------------------------------------

inline int cdp1869_device::get_pen(int ccb0, int ccb1, int pcb)
{
	int r = 0, g = 0, b = 0;

	switch (m_col)
	{
	case 0:
		r = ccb0;
		b = ccb1;
		g = pcb;
		break;

	case 1:
		r = ccb0;
		b = pcb;
		g = ccb1;
		break;

	case 2:
	case 3:
		r = pcb;
		b = ccb0;
		g = ccb1;
		break;
	}

	int color = (r << 2) + (b << 1) + g;

	if (m_cfc)
	{
		return color + ((m_bkg + 1) * 8);
	}
	else
	{
		return color;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1869_device - constructor
//-------------------------------------------------

cdp1869_device::cdp1869_device(running_machine &_machine, const cdp1869_device_config &config)
    : device_t(_machine, config),
	  device_sound_interface(_machine, config, *this),
	  device_memory_interface(_machine, config, *this),
	  m_stream(NULL),
      m_config(config)
{

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1869_device::device_start()
{
	// get the screen device
	m_screen = m_machine.device<screen_device>(m_config.screen_tag);
	assert(m_screen != NULL);

	// resolve callbacks
	devcb_resolve_read_line(&m_in_pal_ntsc_func, &m_config.in_pal_ntsc_func, this);
	devcb_resolve_write_line(&m_out_prd_func, &m_config.out_prd_func, this);
	m_in_pcb_func = m_config.in_pcb_func;
	m_in_char_ram_func = m_config.in_char_ram_func;
	m_out_char_ram_func = m_config.out_char_ram_func;

	// allocate timers
	m_prd_timer = device_timer_alloc(*this);
	update_prd_changed_timer();

	// initialize palette
	initialize_palette();

	// create sound stream
	m_stream = stream_create(this, 0, 1, m_machine.sample_rate, this, static_stream_generate);

	// register for state saving
	state_save_register_device_item(this, 0, m_prd);
	state_save_register_device_item(this, 0, m_dispoff);
	state_save_register_device_item(this, 0, m_fresvert);
	state_save_register_device_item(this, 0, m_freshorz);
	state_save_register_device_item(this, 0, m_cmem);
	state_save_register_device_item(this, 0, m_dblpage);
	state_save_register_device_item(this, 0, m_line16);
	state_save_register_device_item(this, 0, m_line9);
	state_save_register_device_item(this, 0, m_cfc);
	state_save_register_device_item(this, 0, m_col);
	state_save_register_device_item(this, 0, m_bkg);
	state_save_register_device_item(this, 0, m_pma);
	state_save_register_device_item(this, 0, m_hma);
	state_save_register_device_item(this, 0, m_signal);
	state_save_register_device_item(this, 0, m_incr);
	state_save_register_device_item(this, 0, m_toneoff);
	state_save_register_device_item(this, 0, m_wnoff);
	state_save_register_device_item(this, 0, m_tonediv);
	state_save_register_device_item(this, 0, m_tonefreq);
	state_save_register_device_item(this, 0, m_toneamp);
	state_save_register_device_item(this, 0, m_wnfreq);
	state_save_register_device_item(this, 0, m_wnamp);
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void cdp1869_device::device_post_load()
{
	update_prd_changed_timer();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void cdp1869_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	devcb_call_write_line(&m_out_prd_func, param);
	m_prd = param;

	update_prd_changed_timer();
}


//-------------------------------------------------
//  initialize_palette - initialize palette
//-------------------------------------------------

void cdp1869_device::initialize_palette()
{
	// color-on-color display (CFC=0)
	int i;

	for (i = 0; i < 8; i++)
	{
		palette_set_color(machine, i, get_rgb(i, i, 15));
	}

	// tone-on-tone display (CFC=1)
	for (int c = 0; c < 8; c++)
	{
		for (int l = 0; l < 8; l++)
		{
			palette_set_color(machine, i, get_rgb(i, c, l));
			i++;
		}
	}
}


//-------------------------------------------------
//  stream_generate - handle update requests for
//  our sound stream
//-------------------------------------------------

STREAM_UPDATE( cdp1869_device::static_stream_generate )
{
	reinterpret_cast<cdp1869_device *>(param)->stream_generate(inputs, outputs, samples);
}

void cdp1869_device::stream_generate(stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// reset the output stream
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	INT16 signal = m_signal;
	stream_sample_t *buffer = outputs[0];

	if (!m_toneoff && m_toneamp)
	{
		double frequency = (clock() / 2) / (512 >> m_tonefreq) / (m_tonediv + 1);
//      double amplitude = m_toneamp * ((0.78*5) / 15);

		int rate = m_machine.sample_rate / 2;

		/* get progress through wave */
		int incr = m_incr;

		if (signal < 0)
		{
			signal = -(m_toneamp * (0x07fff / 15));
		}
		else
		{
			signal = m_toneamp * (0x07fff / 15);
		}

		while( samples-- > 0 )
		{
			*buffer++ = signal;
			incr -= frequency;
			while( incr < 0 )
			{
				incr += rate;
				signal = -signal;
			}
		}

		/* store progress through wave */
		m_incr = incr;
		m_signal = signal;
	}
/*
    if (!m_wnoff)
    {
        double amplitude = m_wnamp * ((0.78*5) / 15);

        for (int wndiv = 0; wndiv < 128; wndiv++)
        {
            double frequency = (clock() / 2) / (4096 >> m_wnfreq) / (wndiv + 1):

            sum_square_wave(buffer, frequency, amplitude);
        }
    }
*/

}


//-------------------------------------------------
//  draw_line - draw character line
//-------------------------------------------------

void cdp1869_device::draw_line(bitmap_t *bitmap, const rectangle *rect, int x, int y, UINT8 data, int color)
{
	int i;

	data <<= 2;

	for (i = 0; i < CDP1869_CHAR_WIDTH; i++)
	{
		if (data & 0x80)
		{
			*BITMAP_ADDR16(bitmap, y, x) = color;

			if (!m_fresvert)
			{
				*BITMAP_ADDR16(bitmap, y + 1, x) = color;
			}

			if (!m_freshorz)
			{
				*BITMAP_ADDR16(bitmap, y, x + 1) = color;

				if (!m_fresvert)
				{
					*BITMAP_ADDR16(bitmap, y + 1, x + 1) = color;
				}
			}
		}

		if (!m_freshorz)
		{
			x++;
		}

		x++;

		data <<= 1;
	}
}


//-------------------------------------------------
//  draw_char - draw character
//-------------------------------------------------

void cdp1869_device::draw_char(bitmap_t *bitmap, const rectangle *rect, int x, int y, UINT16 pma)
{
	UINT8 pmd = read_page_ram_byte(pma);

	for (UINT8 cma = 0; cma < get_lines(); cma++)
	{
		UINT8 data = read_char_ram_byte(pma, cma, pmd);

		int ccb0 = BIT(data, CCB0);
		int ccb1 = BIT(data, CCB1);
		int pcb = read_pcb(pma, cma, pmd);

		int color = get_pen(ccb0, ccb1, pcb);

		draw_line(bitmap, rect, rect->min_x + x, rect->min_y + y, data, color);

		y++;

		if (!m_fresvert)
		{
			y++;
		}
	}
}


//-------------------------------------------------
//  out3_w - register 3 write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::out3_w )
{
	/*
      bit   description

        0   bkg green
        1   bkg blue
        2   bkg red
        3   cfc
        4   disp off
        5   colb0
        6   colb1
        7   fres horz
    */

	m_bkg = data & 0x07;
	m_cfc = BIT(data, 3);
	m_dispoff = BIT(data, 4);
	m_col = (data & 0x60) >> 5;
	m_freshorz = BIT(data, 7);
}


//-------------------------------------------------
//  out4_w - register 4 write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::out4_w )
{
	/*
      bit   description

        0   tone amp 2^0
        1   tone amp 2^1
        2   tone amp 2^2
        3   tone amp 2^3
        4   tone freq sel0
        5   tone freq sel1
        6   tone freq sel2
        7   tone off
        8   tone / 2^0
        9   tone / 2^1
       10   tone / 2^2
       11   tone / 2^3
       12   tone / 2^4
       13   tone / 2^5
       14   tone / 2^6
       15   always 0
    */

	m_toneamp = offset & 0x0f;
	m_tonefreq = (offset & 0x70) >> 4;
	m_toneoff = BIT(offset, 7);
	m_tonediv = (offset & 0x7f00) >> 8;

	stream_update(m_stream);
}


//-------------------------------------------------
//  out5_w - register 5 write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::out5_w )
{
	/*
      bit   description

        0   cmem access mode
        1   x
        2   x
        3   9-line
        4   x
        5   16 line hi-res
        6   double page
        7   fres vert
        8   wn amp 2^0
        9   wn amp 2^1
       10   wn amp 2^2
       11   wn amp 2^3
       12   wn freq sel0
       13   wn freq sel1
       14   wn freq sel2
       15   wn off
    */

	m_cmem = BIT(offset, 0);
	m_line9 = BIT(offset, 3);
	m_line16 = BIT(offset, 5);
	m_dblpage = BIT(offset, 6);
	m_fresvert = BIT(offset, 7);
	m_wnamp = (offset & 0x0f00) >> 8;
	m_wnfreq = (offset & 0x7000) >> 12;
	m_wnoff = BIT(offset, 15);

	stream_update(m_stream);

	if (m_cmem)
	{
		m_pma = offset;
	}
	else
	{
		m_pma = 0;
	}
}


//-------------------------------------------------
//  out6_w - register 6 write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::out6_w )
{
	/*
      bit   description

        0   pma0 reg
        1   pma1 reg
        2   pma2 reg
        3   pma3 reg
        4   pma4 reg
        5   pma5 reg
        6   pma6 reg
        7   pma7 reg
        8   pma8 reg
        9   pma9 reg
       10   pma10 reg
       11   x
       12   x
       13   x
       14   x
       15   x
    */

	m_pma = offset & 0x7ff;
}


//-------------------------------------------------
//  out7_w - register 7 write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::out7_w )
{
	/*
      bit   description

        0   x
        1   x
        2   hma2 reg
        3   hma3 reg
        4   hma4 reg
        5   hma5 reg
        6   hma6 reg
        7   hma7 reg
        8   hma8 reg
        9   hma9 reg
       10   hma10 reg
       11   x
       12   x
       13   x
       14   x
       15   x
    */

	m_hma = offset & 0x7fc;
}


//-------------------------------------------------
//  char_ram_r - character RAM read
//-------------------------------------------------

READ8_MEMBER( cdp1869_device::char_ram_r )
{
	UINT8 cma = offset & 0x0f;
	UINT16 pma;

	if (m_cmem)
	{
		pma = get_pma();
	}
	else
	{
		pma = offset;
	}

	if (m_dblpage)
	{
		cma &= 0x07;
	}

	UINT8 pmd = read_page_ram_byte(pma);

	return read_char_ram_byte(pma, cma, pmd);
}


//-------------------------------------------------
//  char_ram_w - character RAM write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::char_ram_w )
{
	UINT8 cma = offset & 0x0f;
	UINT16 pma;

	if (m_cmem)
	{
		pma = get_pma();
	}
	else
	{
		pma = offset;
	}

	if (m_dblpage)
	{
		cma &= 0x07;
	}

	UINT8 pmd = read_page_ram_byte(pma);

	write_char_ram_byte(pma, cma, pmd, data);
}


//-------------------------------------------------
//  page_ram_r - page RAM read
//-------------------------------------------------

READ8_MEMBER( cdp1869_device::page_ram_r )
{
	UINT16 pma;

	if (m_cmem)
	{
		pma = get_pma();
	}
	else
	{
		pma = offset;
	}

	return read_page_ram_byte(pma);
}


//-------------------------------------------------
//  page_ram_w - page RAM write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::page_ram_w )
{
	UINT16 pma;

	if (m_cmem)
	{
		pma = get_pma();
	}
	else
	{
		pma = offset;
	}

	write_page_ram_byte(pma, data);
}


//-------------------------------------------------
//  page_ram_w - predisplay
//-------------------------------------------------

READ_LINE_MEMBER( cdp1869_device::predisplay_r )
{
	return m_prd;
}


//-------------------------------------------------
//  pal_ntsc_r - PAL/NTSC
//-------------------------------------------------

READ_LINE_MEMBER( cdp1869_device::pal_ntsc_r )
{
	return devcb_call_read_line(&m_in_pal_ntsc_func);
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

void cdp1869_device::update_screen(bitmap_t *bitmap, const rectangle *cliprect)
{
	rectangle screen_rect, outer;

	if (is_ntsc())
	{
		outer.min_x = CDP1869_HBLANK_END;
		outer.max_x = CDP1869_HBLANK_START - 1;
		outer.min_y = CDP1869_SCANLINE_VBLANK_END_NTSC;
		outer.max_y = CDP1869_SCANLINE_VBLANK_START_NTSC - 1;
		screen_rect.min_x = CDP1869_SCREEN_START_NTSC;
		screen_rect.max_x = CDP1869_SCREEN_END - 1;
		screen_rect.min_y = CDP1869_SCANLINE_DISPLAY_START_NTSC;
		screen_rect.max_y = CDP1869_SCANLINE_DISPLAY_END_NTSC - 1;
	}
	else
	{
		outer.min_x = CDP1869_HBLANK_END;
		outer.max_x = CDP1869_HBLANK_START - 1;
		outer.min_y = CDP1869_SCANLINE_VBLANK_END_PAL;
		outer.max_y = CDP1869_SCANLINE_VBLANK_START_PAL - 1;
		screen_rect.min_x = CDP1869_SCREEN_START_PAL;
		screen_rect.max_x = CDP1869_SCREEN_END - 1;
		screen_rect.min_y = CDP1869_SCANLINE_DISPLAY_START_PAL;
		screen_rect.max_y = CDP1869_SCANLINE_DISPLAY_END_PAL - 1;
	}

	sect_rect(&outer, cliprect);
	bitmap_fill(bitmap, &outer, m_bkg);

	if (!m_dispoff)
	{
		int width = CDP1869_CHAR_WIDTH;
		int height = get_lines();

		if (!m_freshorz)
		{
			width *= 2;
		}

		if (!m_fresvert)
		{
			height *= 2;
		}

		int cols = m_freshorz ? CDP1869_COLUMNS_FULL : CDP1869_COLUMNS_HALF;
		int rows = (screen_rect.max_y - screen_rect.min_y + 1) / height;

		UINT16 pmemsize = get_pmemsize(cols, rows);
		UINT16 addr = m_hma;

		for (int sy = 0; sy < rows; sy++)
		{
			for (int sx = 0; sx < cols; sx++)
			{
				int x = sx * width;
				int y = sy * height;

				draw_char(bitmap, &screen_rect, x, y, addr);

				addr++;

				if (addr == pmemsize) addr = 0;
			}
		}
	}
}
