
/***************************************************************************

    v9938 / v9958 emulation

***************************************************************************/

/*
 todo:

 - sprite collision
 - vdp engine -- make run at correct speed
 - vr/hr/fh flags: double-check all of that
 - make vdp engine work in exp. ram
*/

#include "emu.h"
#include "v9938.h"

#define VERBOSE 0
#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#define V9938_MODE_TEXT1	(0)
#define V9938_MODE_MULTI	(1)
#define V9938_MODE_GRAPHIC1	(2)
#define V9938_MODE_GRAPHIC2	(3)
#define V9938_MODE_GRAPHIC3	(4)
#define V9938_MODE_GRAPHIC4	(5)
#define V9938_MODE_GRAPHIC5	(6)
#define V9938_MODE_GRAPHIC6	(7)
#define V9938_MODE_GRAPHIC7	(8)
#define V9938_MODE_TEXT2	(9)
#define V9938_MODE_UNKNOWN	(10)


static const char *const v9938_modes[] = {
	"TEXT 1", "MULTICOLOR", "GRAPHIC 1", "GRAPHIC 2", "GRAPHIC 3",
	"GRAPHIC 4", "GRAPHIC 5", "GRAPHIC 6", "GRAPHIC 7", "TEXT 2",
	"UNKNOWN"
};

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type V9938 = &device_creator<v9938_device>;
const device_type V9958 = &device_creator<v9958_device>;

v99x8_device::v99x8_device(const machine_config &mconfig, device_type type, const char *name, const char *shortname, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, shortname, tag, owner, clock),
	  m_model(0),
	  m_offset_x(0),
	  m_offset_y(0),
	  m_visible_y(0),
	  m_mode(0),
	  m_pal_write_first(0),
	  m_cmd_write_first(0),
	  m_pal_write(0),
	  m_cmd_write(0),
	  m_read_ahead(0),
	  m_address_latch(0),
	  m_vram_exp(NULL),
	  m_vram_size(0),
	  m_int_state(0),
	  m_int_callback_device_name(NULL),
	  m_scanline(0),
	  m_blink(0),
	  m_blink_count(0),
	  m_sprite_limit(0),
	  m_size(0),
	  m_size_old(0),
	  m_size_auto(0),
	  m_size_now(0),
	  m_mx_delta(0),
	  m_my_delta(0),
	  m_button_state(0),
	  m_screen(NULL),
	  m_screen_name(NULL),
	  m_vdp_ops_count(0),
	  m_vdp_engine(NULL)
{
}

v9938_device::v9938_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: v99x8_device(mconfig, V9938, "V9938", "v9938", tag, owner, clock)
{
	m_model = MODEL_V9938;
}

v9958_device::v9958_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: v99x8_device(mconfig, V9938, "V9938", "v9938", tag, owner, clock)
{
	m_model = MODEL_V9958;
}


int v99x8_device::interrupt ()
{
	int scanline, max, pal, scanline_start;

	update_command ();

	pal = m_cont_reg[9] & 2;
	if (pal) scanline_start = 53; else scanline_start = 22;

	// set flags
	if (m_scanline == (m_offset_y + scanline_start) )
	{
		m_stat_reg[2] &= ~0x40;
	}
	else if (m_scanline == (m_offset_y + m_visible_y + scanline_start) )
	{
		m_stat_reg[2] |= 0x40;
		m_stat_reg[0] |= 0x80;
	}

	max = (pal) ? 255 : (m_cont_reg[9] & 0x80) ? 234 : 244;
	scanline = (m_scanline - scanline_start - m_offset_y);
	if ( (scanline >= 0) && (scanline <= max) &&
	   ( ( (scanline + m_cont_reg[23]) & 255) == m_cont_reg[19]) )
	{
		m_stat_reg[1] |= 1;
		LOG(("V9938: scanline interrupt (%d)\n", scanline));
	}
	else
		if ( !(m_cont_reg[0] & 0x10) ) m_stat_reg[1] &= 0xfe;

	check_int ();

	// check for start of vblank
	if ((pal && (m_scanline == 310)) ||
		(!pal && (m_scanline == 259)))
		interrupt_start_vblank ();

	// render the current line
	if ((m_scanline >= scanline_start) && (m_scanline < (212 + 28 + scanline_start)))
	{
		scanline = (m_scanline - scanline_start) & 255;

		refresh_line (scanline);
	}

	max = (m_cont_reg[9] & 2) ? 313 : 262;
	if (++m_scanline == max)
		m_scanline = 0;

	return m_int_state;
}

void v99x8_device::set_resolution (int i)
{
	if (i == RENDER_AUTO)
	{
		m_size_auto = 1;
	}
	else
	{
		m_size = i;
		m_size_auto = 0;
	}
}

/*
    Not really right... won't work with sprites in graphics 7
    and with palette updated mid-screen
*/
int v99x8_device::get_transpen()
{
	if (m_mode == V9938_MODE_GRAPHIC7)
	{
		return m_pal_ind256[0];
	}
	else
	{
		return m_pal_ind16[0];
	}
}

/*
    Driver-specific function: update the vdp mouse state
*/
void v99x8_device::update_mouse_state(int mx_delta, int my_delta, int button_state)
{
	// save button state
	m_button_state = (button_state << 6) & 0xc0;

	if ((m_cont_reg[8] & 0xc0) == 0x80)
	{	// vdp will process mouse deltas only if it is in mouse mode
		m_mx_delta += mx_delta;
		m_my_delta += my_delta;
	}
}



/***************************************************************************

    Palette functions

***************************************************************************/

/*
About the colour burst registers:

The color burst registers will only have effect on the composite video outputfrom
the V9938. but the output is only NTSC (Never The Same Color ,so the
effects are already present) . this system is not used in europe
the european machines use a separate PAL  (Phase Alternating Line) encoder
or no encoder at all , only RGB output.

Erik de Boer.

--
Right now they're not emulated. For completeness sake they should -- with
a dip-switch to turn them off. I really don't know how they work though. :(
*/

/*
 In screen 8, the colors are encoded as:

 7  6  5  4  3  2  1  0
+--+--+--+--+--+--+--+--+
|g2|g1|g0|r2|r1|r0|b2|b1|
+--+--+--+--+--+--+--+--+

b0 is set if b2 and b1 are set (remember, color bus is 3 bits)

*/

PALETTE_INIT( v9938 )
{
	int	i;

	// create the full 512 colour palette
	for (i=0;i<512;i++)
		palette_set_color_rgb(machine, i, pal3bit(i >> 6), pal3bit(i >> 3), pal3bit(i >> 0));
}

/*

The v9958 can display up to 19286 colours. For this we need a larger palette.

The colours are encoded in 17 bits; however there are just 19268 different colours.
Here we calculate the palette and a 2^17 reference table to the palette,
which is: s_pal_indYJK. It's 256K in size, but I can't think of a faster way
to emulate this. Also it keeps the palette a reasonable size. :)

*/

UINT16 *v99x8_device::s_pal_indYJK;

PALETTE_INIT( v9958 )
{
	int r,g,b,y,j,k,i,k0,j0,n;
	UINT8 pal[19268*3];

	// init v9938 512-color palette
	PALETTE_INIT_CALL(v9938);

	if (v99x8_device::s_pal_indYJK == NULL)
		v99x8_device::s_pal_indYJK = global_alloc_array(UINT16, 0x20000);

	// set up YJK table
	LOG(("Building YJK table for V9958 screens, may take a while ... \n"));
	i = 0;
	for (y=0;y<32;y++) for (k=0;k<64;k++) for (j=0;j<64;j++)
	{
		// calculate the color
		if (k >= 32) k0 = (k - 64); else k0 = k;
		if (j >= 32) j0 = (j - 64); else j0 = j;
		r = y + j0;
		b = (y * 5 - 2 * j0 - k0) / 4;
		g = y + k0;
		if (r < 0) r = 0; else if (r > 31) r = 31;
		if (g < 0) g = 0; else if (g > 31) g = 31;
		if (b < 0) b = 0; else if (b > 31) b = 31;

		r = (r << 3) | (r >> 2);
		b = (b << 3) | (b >> 2);
		g = (g << 3) | (g >> 2);
		// have we seen this one before?
		n = 0;
		while (n < i)
		{
			if (pal[n*3+0] == r && pal[n*3+1] == g && pal[n*3+2] == b)
			{
				v99x8_device::s_pal_indYJK[y | j << 5 | k << (5 + 6)] = n + 512;
				break;
			}
			n++;
		}

		if (i == n)
		{
			// so we haven't; add it
			pal[i*3+0] = r;
			pal[i*3+1] = g;
			pal[i*3+2] = b;
			palette_set_color(machine, i+512, MAKE_RGB(r, g, b));
			v99x8_device::s_pal_indYJK[y | j << 5 | k << (5 + 6)] = i + 512;
			i++;
		}
	}

	if (i != 19268)
		LOG( ("Table creation failed - %d colours out of 19286 created\n", i));
}

UINT32 v99x8_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

/*

 so lookups for screen 12 will look like:

 int ind;

 ind = (*data & 7) << 11 | (*(data + 1) & 7) << 14 |
       (*(data + 2) & 7) << 5 | (*(data + 3) & 7) << 8;

 pixel0 = s_pal_indYJK[ind | (*data >> 3) & 31];
 pixel1 = s_pal_indYJK[ind | (*(data + 1) >> 3) & 31];
 pixel2 = s_pal_indYJK[ind | (*(data + 2) >> 3) & 31];
 pixel3 = s_pal_indYJK[ind | (*(data + 3) >> 3) & 31];

and for screen 11:

pixel0 = (*data) & 8 ? pal_ind16[(*data) >> 4] : s_pal_indYJK[ind | (*data >> 3) & 30];
pixel1 = *(data+1) & 8 ? pal_ind16[*(data+1) >> 4] : s_pal_indYJK[ind | *(data+1) >> 3) & 30];
pixel2 = *(data+2) & 8 ? pal_ind16[*(data+2) >> 4] : s_pal_indYJK[ind | *(data+2) >> 3) & 30];
pixel3 = *(data+3) & 8 ? pal_ind16[*(data+3) >> 4] : s_pal_indYJK[ind | *(data+3) >> 3) & 30];

*/

READ8_MEMBER( v99x8_device::read )
{
	switch (offset & 3)
	{
		case 0:	return vram_r();
		case 1:	return status_r();
	}
	return 0xff;
}

WRITE8_MEMBER( v99x8_device::write )
{
	switch (offset & 3)
	{
		case 0:	vram_w(data);		break;
		case 1:	command_w(data);	break;
		case 2:	palette_w(data);	break;
		case 3:	register_w(data);	break;
	}
}

UINT8 v99x8_device::vram_r()
{
	UINT8 ret;
	int address;

	address = ((int)m_cont_reg[14] << 14) | m_address_latch;

	m_cmd_write_first = 0;

	ret = m_read_ahead;

	if (m_cont_reg[45] & 0x40)
	{
		if ( (m_mode == V9938_MODE_GRAPHIC6) || (m_mode == V9938_MODE_GRAPHIC7) )
			address >>= 1;	// correct?
		// correct?
		if (m_vram_exp && address < 0x10000)
			m_read_ahead = m_vram_exp[address];
		else
			m_read_ahead = 0xff;
	}
	else
	{
		m_read_ahead = vram_read (address);
	}

	m_address_latch = (m_address_latch + 1) & 0x3fff;
	if ((!m_address_latch) && (m_cont_reg[0] & 0x0c) ) // correct ???
	{
		m_cont_reg[14] = (m_cont_reg[14] + 1) & 7;
	}

	return ret;
}

UINT8 v99x8_device::status_r()
{
	int reg;
	UINT8 ret;

	m_cmd_write_first = 0;

	reg = m_cont_reg[15] & 0x0f;
	if (reg > 9)
		return 0xff;

	switch (reg)
	{
		case 0:
			ret = m_stat_reg[0];
			m_stat_reg[0] &= 0x1f;
			break;
		case 1:
			ret = m_stat_reg[1];
			m_stat_reg[1] &= 0xfe;
			if ((m_cont_reg[8] & 0xc0) == 0x80)
				// mouse mode: add button state
				ret |= m_button_state & 0xc0;
			break;
		case 2:
			/*update_command ();*/
/*
    WTF is this? Whatever this was intended to do, it is nonsensical.
    Might as well pick a random number....
    This was an attempt to emulate H-Blank flag ;)
            n = cycles_currently_ran ();
            if ( (n < 28) || (n > 199) ) vdp.statReg[2] |= 0x20;
            else vdp.statReg[2] &= ~0x20;
*/
			if (machine().rand() & 1) m_stat_reg[2] |= 0x20;
			else m_stat_reg[2] &= ~0x20;
			ret = m_stat_reg[2];
			break;
		case 3:
			if ((m_cont_reg[8] & 0xc0) == 0x80)
			{	// mouse mode: return x mouse delta
				ret = m_mx_delta;
				m_mx_delta = 0;
			}
			else
				ret = m_stat_reg[3];
			break;
		case 5:
			if ((m_cont_reg[8] & 0xc0) == 0x80)
			{	// mouse mode: return y mouse delta
				ret = m_my_delta;
				m_my_delta = 0;
			}
			else
				ret = m_stat_reg[5];
			break;
		case 7:
			ret = m_stat_reg[7];
			m_stat_reg[7] = m_cont_reg[44] = vdp_to_cpu () ;
			break;
		default:
			ret = m_stat_reg[reg];
			break;
	}

	LOG(("V9938: Read %02x from S#%d\n", ret, reg));
	check_int ();

	return ret;
}

void v99x8_device::palette_w(UINT8 data)
{
	int indexp;

	if (m_pal_write_first)
	{
		// store in register
		indexp = m_cont_reg[0x10] & 15;
		m_pal_reg[indexp*2] = m_pal_write & 0x77;
		m_pal_reg[indexp*2+1] = data & 0x07;
		// update palette
		m_pal_ind16[indexp] = (((int)m_pal_write << 2) & 0x01c0)  |
							   (((int)data << 3) & 0x0038)  |
								((int)m_pal_write & 0x0007);

		m_cont_reg[0x10] = (m_cont_reg[0x10] + 1) & 15;
		m_pal_write_first = 0;
	}
	else
	{
		m_pal_write = data;
		m_pal_write_first = 1;
	}
}

void v99x8_device::vram_w(UINT8 data)
{
	int address;

	/*update_command ();*/

	m_cmd_write_first = 0;

    address = ((int)m_cont_reg[14] << 14) | m_address_latch;

    if (m_cont_reg[45] & 0x40)
        {
		if ( (m_mode == V9938_MODE_GRAPHIC6) || (m_mode == V9938_MODE_GRAPHIC7) )
			address >>= 1;	// correct?
        if (m_vram_exp && address < 0x10000)
            m_vram_exp[address] = data;
        }
    else
        {
		vram_write (address, data);
        }

	m_address_latch = (m_address_latch + 1) & 0x3fff;
	if ((!m_address_latch) && (m_cont_reg[0] & 0x0c) ) // correct ???
	{
		m_cont_reg[14] = (m_cont_reg[14] + 1) & 7;
	}
}

void v99x8_device::command_w(UINT8 data)
{
	if (m_cmd_write_first)
	{
		if (data & 0x80)
		{
			if (!(data & 0x40))
			register_write (data & 0x3f, m_cmd_write);
		}
		else
		{
			m_address_latch =
				(((UINT16)data << 8) | m_cmd_write) & 0x3fff;
			if ( !(data & 0x40) ) vram_r (); // read ahead!
		}

		m_cmd_write_first = 0;
	}
	else
	{
		m_cmd_write = data;
		m_cmd_write_first = 1;
	}
}

void v99x8_device::register_w(UINT8 data)
{
	int reg;

	reg = m_cont_reg[17] & 0x3f;
	if (reg != 17)
		register_write (reg, data); // true ?

	if (!(m_cont_reg[17] & 0x80))
		m_cont_reg[17] = (m_cont_reg[17] + 1) & 0x3f;
}

void v99x8_device::static_set_screen(device_t &device, const char *screen_name)
{
	downcast<v99x8_device &>(device).m_screen_name = screen_name;
}

void v99x8_device::static_set_vram_size(device_t &device, UINT32 vram_size)
{
	downcast<v99x8_device &>(device).m_vram_size = vram_size;
}

void v99x8_device::static_set_interrupt_callback(device_t &device, v99x8_interrupt_delegate callback, const char *device_name)
{
	v99x8_device &v99x8 = downcast<v99x8_device &>(device);
	v99x8.m_int_callback = callback;
	v99x8.m_int_callback_device_name = device_name;
}


/***************************************************************************

    Init/stop/reset/Interrupt functions

***************************************************************************/

void v99x8_device::device_start()
{
	// find our devices
	m_screen = machine().device<screen_device>(m_screen_name);
	assert(m_screen != NULL);
	if (!m_screen->started())
		throw device_missing_dependencies();

	if (!m_int_callback.isnull())
	{
		device_t *device = (m_int_callback_device_name != NULL) ? machine().device(m_int_callback_device_name) : NULL;
		if (device != NULL)
			m_int_callback.late_bind(*device);
	}

	m_vdp_ops_count = 1;
	m_vdp_engine = NULL;

	m_screen->register_screen_bitmap(m_bitmap);
	m_size_old = -1;

	// allocate VRAM
	assert(m_vram_size > 0);
	memset(m_vram, 0, sizeof(m_vram));
	if (m_vram_size < 0x20000)
	{
		// set unavailable RAM to 0xff
		memset (m_vram + m_vram_size, 0xff, (sizeof(m_vram) - m_vram_size) );
	}

	// do we have expanded memory?
	if (m_vram_size > 0x20000)
		m_vram_exp = auto_alloc_array_clear(machine(), UINT8, 0x10000);
	else
		m_vram_exp = NULL;

	save_item(NAME(m_offset_x));
	save_item(NAME(m_offset_y));
	save_item(NAME(m_visible_y));
	save_item(NAME(m_mode));
	save_item(NAME(m_pal_write_first));
	save_item(NAME(m_cmd_write_first));
	save_item(NAME(m_pal_write));
	save_item(NAME(m_cmd_write));
	save_item(NAME(m_pal_reg));
	save_item(NAME(m_stat_reg));
	save_item(NAME(m_cont_reg));
	save_item(NAME(m_read_ahead));
	save_item(NAME(m_vram));
	if ( m_vram_exp != NULL )
		save_pointer(NAME(m_vram_exp), 0x10000);
	save_item(NAME(m_int_state));
	save_item(NAME(m_scanline));
	save_item(NAME(m_blink));
	save_item(NAME(m_blink_count));
	save_item(NAME(m_size));
	save_item(NAME(m_size_old));
	save_item(NAME(m_size_auto));
	save_item(NAME(m_size_now));
	save_item(NAME(m_mx_delta));
	save_item(NAME(m_my_delta));
	save_item(NAME(m_button_state));
	save_item(NAME(m_pal_ind16));
	save_item(NAME(m_pal_ind256));
	save_item(NAME(m_mmc.SX));
	save_item(NAME(m_mmc.SY));
	save_item(NAME(m_mmc.DX));
	save_item(NAME(m_mmc.DY));
	save_item(NAME(m_mmc.TX));
	save_item(NAME(m_mmc.TY));
	save_item(NAME(m_mmc.NX));
	save_item(NAME(m_mmc.NY));
	save_item(NAME(m_mmc.MX));
	save_item(NAME(m_mmc.ASX));
	save_item(NAME(m_mmc.ADX));
	save_item(NAME(m_mmc.ANX));
	save_item(NAME(m_mmc.CL));
	save_item(NAME(m_mmc.LO));
	save_item(NAME(m_mmc.CM));
	save_item(NAME(m_mmc.MXS));
	save_item(NAME(m_mmc.MXD));
	save_item(NAME(m_vdp_ops_count));
}

void v99x8_device::device_reset()
{
	int i;

	// offset reset
	m_offset_x = 8;
	m_offset_y = 8 + 16;
	m_visible_y = 192;
	// register reset
	reset_palette (); // palette registers
	for (i=0;i<10;i++) m_stat_reg[i] = 0;
	m_stat_reg[2] = 0x0c;
	if (m_model == MODEL_V9958) m_stat_reg[1] |= 4;
	for (i=0;i<48;i++) m_cont_reg[i] = 0;
	m_cmd_write_first = m_pal_write_first = 0;
	m_int_state = 0;
	m_read_ahead = 0; m_address_latch = 0; // ???
	m_scanline = 0;
	// MZ: The status registers 4 and 6 hold the high bits of the sprite
	// collision location. The unused bits are set to 1.
	// SR3: x x x x x x x x
	// SR4: 1 1 1 1 1 1 1 x
	// SR5: y y y y y y y y
	// SR6: 1 1 1 1 1 1 y y
	// Note that status register 4 is used in detection algorithms to tell
	// apart the tms9929 from the v99x8.

	// TODO: SR3-S6 do not yet store the information about the sprite collision
	m_stat_reg[4] = 0xfe;
	m_stat_reg[6] = 0xfc;
}




void v99x8_device::reset_palette ()
	{
	// taken from V9938 Technical Data book, page 148. it's in G-R-B format
	static const UINT8 pal16[16*3] = {
		0, 0, 0, // 0: black/transparent
		0, 0, 0, // 1: black
		6, 1, 1, // 2: medium green
		7, 3, 3, // 3: light green
		1, 1, 7, // 4: dark blue
		3, 2, 7, // 5: light blue
		1, 5, 1, // 6: dark red
		6, 2, 7, // 7: cyan
		1, 7, 1, // 8: medium red
		3, 7, 3, // 9: light red
		6, 6, 1, // 10: dark yellow
		6, 6, 4, // 11: light yellow
		4, 1, 1, // 12: dark green
		2, 6, 5, // 13: magenta
		5, 5, 5, // 14: gray
		7, 7, 7  // 15: white
	};
	int i, red, ind;

	for (i=0;i<16;i++)
	{
		// set the palette registers
		m_pal_reg[i*2+0] = pal16[i*3+1] << 4 | pal16[i*3+2];
		m_pal_reg[i*2+1] = pal16[i*3];
		// set the reference table
		m_pal_ind16[i] = pal16[i*3+1] << 6 | pal16[i*3] << 3 | pal16[i*3+2];
	}

	// set internal palette GRAPHIC 7
	for (i=0;i<256;i++)
	{
		ind = (i << 4) & 0x01c0;
		ind |= (i >> 2) & 0x0038;
		red = (i << 1) & 6; if (red == 6) red++;
		ind |= red;

		m_pal_ind256[i] = ind;
	}
}

/***************************************************************************

    Memory functions

***************************************************************************/

void v99x8_device::vram_write (int offset, int data)
{
	int newoffset;

	if ( (m_mode == V9938_MODE_GRAPHIC6) || (m_mode == V9938_MODE_GRAPHIC7) )
	{
        newoffset = ((offset & 1) << 16) | (offset >> 1);
		if (newoffset < m_vram_size)
        	m_vram[newoffset] = data;
	}
	else
	{
		if (offset < m_vram_size)
			m_vram[offset] = data;
        }
}

int v99x8_device::vram_read (int offset)
{
	if ( (m_mode == V9938_MODE_GRAPHIC6) || (m_mode == V9938_MODE_GRAPHIC7) )
		return m_vram[((offset & 1) << 16) | (offset >> 1)];
	else
		return m_vram[offset];
}

void v99x8_device::check_int ()
{
	UINT8 n;

	n = ( (m_cont_reg[1] & 0x20) && (m_stat_reg[0] & 0x80) /*&& m_vblank_int*/) ||
		( (m_stat_reg[1] & 0x01) && (m_cont_reg[0] & 0x10) );

#if 0
	if(n && m_vblank_int)
	{
		m_vblank_int = 0;
	}
#endif

	if (n != m_int_state)
	{
		m_int_state = n;
		LOG(("V9938: IRQ line %s\n", n ? "up" : "down"));
	}

	/*
    ** Somehow the IRQ request is going down without cpu_irq_line () being
    ** called; because of this Mr. Ghost, Xevious and SD Snatcher don't
    ** run. As a patch it's called every scanline
   */
	m_int_callback (*this, n);
}

/***************************************************************************

    Register functions

***************************************************************************/

void v99x8_device::register_write (int reg, int data)
{
	static UINT8 const reg_mask[] =
	{
		0x7e, 0x7b, 0x7f, 0xff, 0x3f, 0xff, 0x3f, 0xff,
		0xfb, 0xbf, 0x07, 0x03, 0xff, 0xff, 0x07, 0x0f,
		0x0f, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0x00, 0x7f, 0x3f, 0x07
	};

	if (reg <= 27)
	{
		data &= reg_mask[reg];
		if (m_cont_reg[reg] == data)
			return;
	}

	if (reg > 46)
	{
		LOG(("V9938: Attempted to write to non-existant R#%d\n", reg));
		return;
	}

	/*update_command ();*/

	switch (reg) {
	// registers that affect interrupt and display mode
	case 0:
	case 1:
		m_cont_reg[reg] = data;
		set_mode ();
		check_int ();
		LOG(("V9938: mode = %s\n", v9938_modes[m_mode]));
		break;

	case 18:
	case 9:
		m_cont_reg[reg] = data;
		// recalc offset
		m_offset_x = (( (~m_cont_reg[18] - 8) & 0x0f) + 1);
		m_offset_y = ((~(m_cont_reg[18]>>4) - 8) & 0x0f) + 7;
		if (m_cont_reg[9] & 0x80)
		{
			m_visible_y = 212;
		}
		else
		{
			m_visible_y = 192;
			m_offset_y += 10;
		}
		break;

	case 15:
		m_pal_write_first = 0;
		break;

	// color burst registers aren't emulated
	case 20:
	case 21:
	case 22:
		LOG(("V9938: Write %02xh to R#%d; color burst not emulated\n",
			data, reg));
		break;
	case 25:
	case 26:
	case 27:
		if (m_model != MODEL_V9958)
		{
			LOG(("V9938: Attempting to write %02xh to V9958 R#%d\n", data, reg));
			data = 0;
		}
		break;

	case 44:
		cpu_to_vdp (data);
		break;

	case 46:
		command_unit_w (data);
		break;
	}

	if (reg != 15)
		LOG(("V9938: Write %02x to R#%d\n", data, reg));

	m_cont_reg[reg] = data;
}

/***************************************************************************

    Refresh / render function

***************************************************************************/

#define V9938_SECOND_FIELD ( !(((m_cont_reg[9] & 0x04) && !(m_stat_reg[2] & 2)) || m_blink))

/*
 * This file is included for a number of different situations:
 * _Width : can be 512 + 32 or 256 + 16
 * V9938_BPP : can be 8 or 16
 */


template<typename _PixelType, int _Width>
void v99x8_device::default_border(const pen_t *pens, _PixelType *ln)
{
    _PixelType pen;
	int	i;

    pen = pens[m_pal_ind16[(m_cont_reg[7]&0x0f)]];
	i = _Width;
	while (i--) *ln++ = pen;

	if (m_size_now != RENDER_HIGH) m_size_now = RENDER_LOW;
}

template<typename _PixelType, int _Width>
void v99x8_device::graphic7_border(const pen_t *pens, _PixelType *ln)
{
	_PixelType pen;
	int i;

	pen = pens[m_pal_ind256[m_cont_reg[7]]];
	i = _Width;
	while (i--) *ln++ = pen;

	if (m_size_now != RENDER_HIGH) m_size_now = RENDER_LOW;
}

template<typename _PixelType, int _Width>
void v99x8_device::graphic5_border(const pen_t *pens, _PixelType *ln)
{
	int i;
	_PixelType pen0;
	if (_Width > 512)
	{
		_PixelType pen1;

		pen1 = pens[m_pal_ind16[(m_cont_reg[7]&0x03)]];
		pen0 = pens[m_pal_ind16[((m_cont_reg[7]>>2)&0x03)]];
		i = (_Width) / 2;
		while (i--) { *ln++ = pen0; *ln++ = pen1; }
	}
	else
	{
		pen0 = pens[m_pal_ind16[((m_cont_reg[7]>>2)&0x03)]];
		i = _Width;
		while (i--) *ln++ = pen0;
	}
	m_size_now = RENDER_HIGH;
}

template<typename _PixelType, int _Width>
void v99x8_device::mode_text1(const pen_t *pens, _PixelType *ln, int line)
{
	int pattern, x, xx, name, xxx;
	_PixelType fg, bg, pen;
	UINT8 *nametbl, *patterntbl;

	patterntbl = m_vram + (m_cont_reg[4] << 11);
	nametbl = m_vram + (m_cont_reg[2] << 10);

    fg = pens[m_pal_ind16[m_cont_reg[7] >> 4]];
    bg = pens[m_pal_ind16[m_cont_reg[7] & 15]];

	name = (line/8)*40;

	pen = pens[m_pal_ind16[(m_cont_reg[7]&0x0f)]];

	xxx = m_offset_x + 8;
	if (_Width > 512)
		xxx *= 2;
	while (xxx--) *ln++ = pen;

	for (x=0;x<40;x++)
	{
		pattern = patterntbl[(nametbl[name] * 8) +
			((line + m_cont_reg[23]) & 7)];
		for (xx=0;xx<6;xx++)
		{
			*ln++ = (pattern & 0x80) ? fg : bg;
			if (_Width > 512)
				*ln++ = (pattern & 0x80) ? fg : bg;
			pattern <<= 1;
		}
		/* width height 212, characters start repeating at the bottom */
		name = (name + 1) & 0x3ff;
	}

	xxx = (16 - m_offset_x) + 8;
	if (_Width > 512)
		xxx *= 2;
	while (xxx--) *ln++ = pen;
	if (m_size_now != RENDER_HIGH) m_size_now = RENDER_LOW;
}

template<typename _PixelType, int _Width>
void v99x8_device::mode_text2(const pen_t *pens, _PixelType *ln, int line)
{
	int pattern, x, charcode, name, xxx, patternmask, colourmask;
	_PixelType fg, bg, fg0, bg0, pen;
	UINT8 *nametbl, *patterntbl, *colourtbl;

	patterntbl = m_vram + (m_cont_reg[4] << 11);
	colourtbl = m_vram + ((m_cont_reg[3] & 0xf8) << 6) + (m_cont_reg[10] << 14);
#if 0
	colourmask = ((m_cont_reg[3] & 7) << 5) | 0x1f; /* cause a bug in Forth+ v1.0 on Geneve */
#else
	colourmask = ((m_cont_reg[3] & 7) << 6) | 0x3f; /* verify! */
#endif
	nametbl = m_vram + ((m_cont_reg[2] & 0xfc) << 10);
	patternmask = ((m_cont_reg[2] & 3) << 10) | 0x3ff; /* seems correct */

    fg = pens[m_pal_ind16[m_cont_reg[7] >> 4]];
    bg = pens[m_pal_ind16[m_cont_reg[7] & 15]];
    fg0 = pens[m_pal_ind16[m_cont_reg[12] >> 4]];
    bg0 = pens[m_pal_ind16[m_cont_reg[12] & 15]];

	name = (line/8)*80;

	xxx = m_offset_x + 8;
	pen = pens[m_pal_ind16[(m_cont_reg[7]&0x0f)]];
	if (_Width > 512)
		xxx *= 2;
	while (xxx--) *ln++ = pen;

	for (x=0;x<80;x++)
	{
		charcode = nametbl[name&patternmask];
		if (m_blink)
		{
			pattern = colourtbl[(name/8)&colourmask];
			if (pattern & (0x80 >> (name & 7) ) )
			{
				pattern = patterntbl[(charcode * 8) +
					((line + m_cont_reg[23]) & 7)];

				if (_Width > 512)
				{
					*ln++ = (pattern & 0x80) ? fg0 : bg0;
					*ln++ = (pattern & 0x40) ? fg0 : bg0;
					*ln++ = (pattern & 0x20) ? fg0 : bg0;
					*ln++ = (pattern & 0x10) ? fg0 : bg0;
					*ln++ = (pattern & 0x08) ? fg0 : bg0;
					*ln++ = (pattern & 0x04) ? fg0 : bg0;
				}
				else
				{
					*ln++ = (pattern & 0x80) ? fg0 : bg0;
					*ln++ = (pattern & 0x20) ? fg0 : bg0;
					*ln++ = (pattern & 0x08) ? fg0 : bg0;
				}

				name++;
				continue;
			}
		}

		pattern = patterntbl[(charcode * 8) +
			((line + m_cont_reg[23]) & 7)];

		if (_Width > 512)
		{
			*ln++ = (pattern & 0x80) ? fg : bg;
			*ln++ = (pattern & 0x40) ? fg : bg;
			*ln++ = (pattern & 0x20) ? fg : bg;
			*ln++ = (pattern & 0x10) ? fg : bg;
			*ln++ = (pattern & 0x08) ? fg : bg;
			*ln++ = (pattern & 0x04) ? fg : bg;
		}
		else
		{
			*ln++ = (pattern & 0x80) ? fg : bg;
			*ln++ = (pattern & 0x20) ? fg : bg;
			*ln++ = (pattern & 0x08) ? fg : bg;
		}

		name++;
	}

	xxx = 16  - m_offset_x + 8;
	if (_Width > 512)
		xxx *= 2;
	while (xxx--) *ln++ = pen;
	m_size_now = RENDER_HIGH;
}

template<typename _PixelType, int _Width>
void v99x8_device::mode_multi(const pen_t *pens, _PixelType *ln, int line)
{
	UINT8 *nametbl, *patterntbl, colour;
	int name, line2, x, xx;
	_PixelType pen, pen_bg;

	nametbl = m_vram + (m_cont_reg[2] << 10);
	patterntbl = m_vram + (m_cont_reg[4] << 11);

	line2 = (line - m_cont_reg[23]) & 255;
	name = (line2/8)*32;

	pen_bg = pens[m_pal_ind16[(m_cont_reg[7]&0x0f)]];
	if (_Width < 512)
		xx = m_offset_x;
	else
		xx = m_offset_x * 2;
	while (xx--) *ln++ = pen_bg;

	for (x=0;x<32;x++)
	{
		colour = patterntbl[(nametbl[name] * 8) + ((line2/4)&7)];
		pen = pens[m_pal_ind16[colour>>4]];
		/* eight pixels */
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
		if (_Width > 512)
		{
			*ln++ = pen;
			*ln++ = pen;
			*ln++ = pen;
			*ln++ = pen;
		}
		pen = pens[m_pal_ind16[colour&15]];
		/* eight pixels */
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
		if (_Width > 512)
		{
			*ln++ = pen;
			*ln++ = pen;
			*ln++ = pen;
			*ln++ = pen;
		}
		name++;
	}

	xx = 16 - m_offset_x;
	if (_Width > 512)
		xx *= 2;
	while (xx--) *ln++ = pen_bg;
	if (m_size_now != RENDER_HIGH) m_size_now = RENDER_LOW;
}

template<typename _PixelType, int _Width>
void v99x8_device::mode_graphic1(const pen_t *pens, _PixelType *ln, int line)
{
	_PixelType fg, bg, pen;
	UINT8 *nametbl, *patterntbl, *colourtbl;
	int pattern, x, xx, line2, name, charcode, colour, xxx;

	nametbl = m_vram + (m_cont_reg[2] << 10);
	colourtbl = m_vram + (m_cont_reg[3] << 6) + (m_cont_reg[10] << 14);
	patterntbl = m_vram + (m_cont_reg[4] << 11);

	line2 = (line - m_cont_reg[23]) & 255;

	name = (line2/8)*32;

	pen = pens[m_pal_ind16[(m_cont_reg[7]&0x0f)]];
	if (_Width < 512)
		xxx = m_offset_x;
	else
		xxx = m_offset_x * 2;
	while (xxx--) *ln++ = pen;

	for (x=0;x<32;x++)
	{
		charcode = nametbl[name];
		colour = colourtbl[charcode/8];
		fg = pens[m_pal_ind16[colour>>4]];
		bg = pens[m_pal_ind16[colour&15]];
		pattern = patterntbl[charcode * 8 + (line2 & 7)];

		for (xx=0;xx<8;xx++)
		{
			*ln++ = (pattern & 0x80) ? fg : bg;
			if (_Width > 512)
				*ln++ = (pattern & 0x80) ? fg : bg;
			pattern <<= 1;
		}
		name++;
	}

	xx = 16 - m_offset_x;
	if (_Width > 512)
		xx *= 2;
	while (xx--) *ln++ = pen;
	if (m_size_now != RENDER_HIGH) m_size_now = RENDER_LOW;
}

template<typename _PixelType, int _Width>
void v99x8_device::mode_graphic23(const pen_t *pens, _PixelType *ln, int line)
{
	_PixelType fg, bg, pen;
	UINT8 *nametbl, *patterntbl, *colourtbl;
	int pattern, x, xx, line2, name, charcode,
		colour, colourmask, patternmask, xxx;

	colourmask = (m_cont_reg[3] & 0x7f) * 8 | 7;
	patternmask = (m_cont_reg[4] & 0x03) * 256 | (colourmask & 255);

	nametbl = m_vram + (m_cont_reg[2] << 10);
	colourtbl = m_vram + ((m_cont_reg[3] & 0x80) << 6) + (m_cont_reg[10] << 14);
	patterntbl = m_vram + ((m_cont_reg[4] & 0x3c) << 11);

	line2 = (line + m_cont_reg[23]) & 255;
	name = (line2/8)*32;

	pen = pens[m_pal_ind16[(m_cont_reg[7]&0x0f)]];
	if (_Width < 512)
		xxx = m_offset_x;
	else
		xxx = m_offset_x * 2;
	while (xxx--) *ln++ = pen;

	for (x=0;x<32;x++)
	{
		charcode = nametbl[name] + (line2&0xc0)*4;
		colour = colourtbl[(charcode&colourmask)*8+(line2&7)];
		pattern = patterntbl[(charcode&patternmask)*8+(line2&7)];
        fg = pens[m_pal_ind16[colour>>4]];
        bg = pens[m_pal_ind16[colour&15]];
		for (xx=0;xx<8;xx++)
		{
			*ln++ = (pattern & 0x80) ? fg : bg;
			if (_Width > 512)
				*ln++ = (pattern & 0x80) ? fg : bg;
            pattern <<= 1;
		}
		name++;
	}

	xx = 16 - m_offset_x;
	if (_Width > 512)
		xx *= 2;
	while (xx--) *ln++ = pen;
	if (m_size_now != RENDER_HIGH) m_size_now = RENDER_LOW;
}

template<typename _PixelType, int _Width>
void v99x8_device::mode_graphic4(const pen_t *pens, _PixelType *ln, int line)
{
	UINT8 *nametbl, colour;
	int line2, linemask, x, xx;
	_PixelType pen, pen_bg;

	linemask = ((m_cont_reg[2] & 0x1f) << 3) | 7;

	line2 = ((line + m_cont_reg[23]) & linemask) & 255;

	nametbl = m_vram + ((m_cont_reg[2] & 0x40) << 10) + line2 * 128;
	if ( (m_cont_reg[2] & 0x20) && (V9938_SECOND_FIELD) )
		nametbl += 0x8000;

	pen_bg = pens[m_pal_ind16[(m_cont_reg[7]&0x0f)]];
	if (_Width < 512)
		xx = m_offset_x;
	else
		xx = m_offset_x * 2;
	while (xx--) *ln++ = pen_bg;

	for (x=0;x<128;x++)
	{
		colour = *nametbl++;
        pen = pens[m_pal_ind16[colour>>4]];
		*ln++ = pen;
		if (_Width > 512)
			*ln++ = pen;
        pen = pens[m_pal_ind16[colour&15]];
		*ln++ = pen;
		if (_Width > 512)
			*ln++ = pen;
	}

	xx = 16 - m_offset_x;
	if (_Width > 512)
		xx *= 2;
	while (xx--) *ln++ = pen_bg;
	if (m_size_now != RENDER_HIGH) m_size_now = RENDER_LOW;
}

template<typename _PixelType, int _Width>
void v99x8_device::mode_graphic5(const pen_t *pens, _PixelType *ln, int line)
{
	UINT8 *nametbl, colour;
	int line2, linemask, x, xx;
	_PixelType pen_bg0[4];
	_PixelType pen_bg1[4];

	linemask = ((m_cont_reg[2] & 0x1f) << 3) | 7;

	line2 = ((line + m_cont_reg[23]) & linemask) & 255;

	nametbl = m_vram + ((m_cont_reg[2] & 0x40) << 10) + line2 * 128;
	if ( (m_cont_reg[2] & 0x20) && (V9938_SECOND_FIELD) )
		nametbl += 0x8000;

	if (_Width > 512)
	{
		pen_bg1[0] = pens[m_pal_ind16[(m_cont_reg[7]&0x03)]];
		pen_bg0[0] = pens[m_pal_ind16[((m_cont_reg[7]>>2)&0x03)]];

		xx = m_offset_x;
		while (xx--) { *ln++ = pen_bg0[0]; *ln++ = pen_bg1[0]; }

		x = (m_cont_reg[8] & 0x20) ? 0 : 1;

	    for (;x<4;x++)
		{
			pen_bg0[x] = pens[m_pal_ind16[x]];
			pen_bg1[x] = pens[m_pal_ind16[x]];
		}

		for (x=0;x<128;x++)
		{
			colour = *nametbl++;

	        *ln++ = pen_bg0[colour>>6];
			*ln++ = pen_bg1[(colour>>4)&3];
	        *ln++ = pen_bg0[(colour>>2)&3];
	    	*ln++ = pen_bg1[(colour&3)];
		}

		pen_bg1[0] = pens[m_pal_ind16[(m_cont_reg[7]&0x03)]];
		pen_bg0[0] = pens[m_pal_ind16[((m_cont_reg[7]>>2)&0x03)]];
		xx = 16 - m_offset_x;
		while (xx--) { *ln++ = pen_bg0[0]; *ln++ = pen_bg1[0]; }
	}
	else
	{
		pen_bg0[0] = pens[m_pal_ind16[((m_cont_reg[7]>>2)&0x03)]];

		x = (m_cont_reg[8] & 0x20) ? 0 : 1;

	    for (;x<4;x++)
			pen_bg0[x] = pens[m_pal_ind16[x]];

		xx = m_offset_x;
		while (xx--) *ln++ = pen_bg0[0];

		for (x=0;x<128;x++)
		{
			colour = *nametbl++;
	        *ln++ = pen_bg0[colour>>6];
	        *ln++ = pen_bg0[(colour>>2)&3];
		}

		pen_bg0[0] = pens[m_pal_ind16[((m_cont_reg[7]>>2)&0x03)]];
		xx = 16 - m_offset_x;
		while (xx--) *ln++ = pen_bg0[0];
	}
	m_size_now = RENDER_HIGH;
}

template<typename _PixelType, int _Width>
void v99x8_device::mode_graphic6(const pen_t *pens, _PixelType *ln, int line)
{
    UINT8 colour;
    int line2, linemask, x, xx, nametbl;
    _PixelType pen_bg, fg0;
	_PixelType fg1;

    linemask = ((m_cont_reg[2] & 0x1f) << 3) | 7;

	line2 = ((line + m_cont_reg[23]) & linemask) & 255;

    nametbl = line2 << 8 ;
	if ( (m_cont_reg[2] & 0x20) && (V9938_SECOND_FIELD) )
        nametbl += 0x10000;

	pen_bg = pens[m_pal_ind16[(m_cont_reg[7]&0x0f)]];
	if (_Width < 512)
		xx = m_offset_x;
	else
		xx = m_offset_x * 2;
	while (xx--) *ln++ = pen_bg;

	if (m_cont_reg[2] & 0x40)
	{
		for (x=0;x<32;x++)
		{
			nametbl++;
			colour = m_vram[((nametbl&1) << 16) | (nametbl>>1)];
        	fg0 = pens[m_pal_ind16[colour>>4]];
			if (_Width < 512)
			{
				*ln++ = fg0; *ln++ = fg0;
				*ln++ = fg0; *ln++ = fg0;
				*ln++ = fg0; *ln++ = fg0;
				*ln++ = fg0; *ln++ = fg0;
			}
			else
			{
	        	fg1 = pens[m_pal_ind16[colour&15]];
				*ln++ = fg0; *ln++ = fg1; *ln++ = fg0; *ln++ = fg1;
				*ln++ = fg0; *ln++ = fg1; *ln++ = fg0; *ln++ = fg1;
				*ln++ = fg0; *ln++ = fg1; *ln++ = fg0; *ln++ = fg1;
				*ln++ = fg0; *ln++ = fg1; *ln++ = fg0; *ln++ = fg1;
			}
			nametbl += 7;
		}
	}
	else
	{
		for (x=0;x<256;x++)
		{
			colour = m_vram[((nametbl&1) << 16) | (nametbl>>1)];
        	*ln++ = pens[m_pal_ind16[colour>>4]];
			if (_Width > 512)
	        	*ln++ = pens[m_pal_ind16[colour&15]];
			nametbl++;
        }
	}

	xx = 16 - m_offset_x;
	if (_Width > 512)
		xx *= 2;
	while (xx--) *ln++ = pen_bg;
	m_size_now = RENDER_HIGH;
}

template<typename _PixelType, int _Width>
void v99x8_device::mode_graphic7(const pen_t *pens, _PixelType *ln, int line)
{
    UINT8 colour;
    int line2, linemask, x, xx, nametbl;
    _PixelType pen, pen_bg;

	linemask = ((m_cont_reg[2] & 0x1f) << 3) | 7;

	line2 = ((line + m_cont_reg[23]) & linemask) & 255;

	nametbl = line2 << 8;
	if ( (m_cont_reg[2] & 0x20) && (V9938_SECOND_FIELD) )
		nametbl += 0x10000;

	pen_bg = pens[m_pal_ind256[m_cont_reg[7]]];
	if (_Width < 512)
		xx = m_offset_x;
	else
		xx = m_offset_x * 2;
	while (xx--) *ln++ = pen_bg;

	if (m_cont_reg[2] & 0x40)
	{
		for (x=0;x<32;x++)
		{
			nametbl++;
			colour = m_vram[((nametbl&1) << 16) | (nametbl>>1)];
			pen = pens[m_pal_ind256[colour]];
			*ln++ = pen; *ln++ = pen;
			*ln++ = pen; *ln++ = pen;
			*ln++ = pen; *ln++ = pen;
			*ln++ = pen; *ln++ = pen;
			if (_Width > 512)
			{
				*ln++ = pen; *ln++ = pen;
				*ln++ = pen; *ln++ = pen;
				*ln++ = pen; *ln++ = pen;
				*ln++ = pen; *ln++ = pen;
			}
			nametbl++;
		}
	}
	else
	{
		for (x=0;x<256;x++)
        {
			colour = m_vram[((nametbl&1) << 16) | (nametbl>>1)];
			pen = pens[m_pal_ind256[colour]];
			*ln++ = pen;
			if (_Width > 512)
				*ln++ = pen;
			nametbl++;
    	}
	}

	xx = 16 - m_offset_x;
	if (_Width > 512)
		xx *= 2;
	while (xx--) *ln++ = pen_bg;
	if (m_size_now != RENDER_HIGH) m_size_now = RENDER_LOW;
}

template<typename _PixelType, int _Width>
void v99x8_device::mode_unknown(const pen_t *pens, _PixelType *ln, int line)
{
	_PixelType fg, bg;
	int x;

    fg = pens[m_pal_ind16[m_cont_reg[7] >> 4]];
    bg = pens[m_pal_ind16[m_cont_reg[7] & 15]];

	if (_Width < 512)
	{
		x = m_offset_x;
		while (x--) *ln++ = bg;

		x = 256;
		while (x--) *ln++ = fg;

		x = 16 - m_offset_x;
		while (x--) *ln++ = bg;
	}
	else
	{
		x = m_offset_x * 2;
		while (x--) *ln++ = bg;

		x = 512;
		while (x--) *ln++ = fg;

		x = (16 - m_offset_x) * 2;
		while (x--) *ln++ = bg;
	}
	if (m_size_now != RENDER_HIGH) m_size_now = RENDER_LOW;
}

template<typename _PixelType, int _Width>
void v99x8_device::default_draw_sprite(const pen_t *pens, _PixelType *ln, UINT8 *col)
{
	int i;
	if (_Width > 512)
		ln += m_offset_x * 2;
	else
		ln += m_offset_x;

	for (i=0;i<256;i++)
	{
		if (col[i] & 0x80)
		{
			*ln++ = pens[m_pal_ind16[col[i]&0x0f]];
			if (_Width > 512)
				*ln++ = pens[m_pal_ind16[col[i]&0x0f]];
		}
		else
		{
			if (_Width > 512)
				ln += 2;
			else
				ln++;
		}
	}
}
template<typename _PixelType, int _Width>
void v99x8_device::graphic5_draw_sprite(const pen_t *pens, _PixelType *ln, UINT8 *col)
{
	int i;
	if (_Width > 512)
		ln += m_offset_x * 2;
	else
		ln += m_offset_x;

	for (i=0;i<256;i++)
	{
		if (col[i] & 0x80)
		{
			*ln++ = pens[m_pal_ind16[(col[i]>>2)&0x03]];
			if (_Width > 512)
				*ln++ = pens[m_pal_ind16[col[i]&0x03]];
		}
		else
		{
			if (_Width > 512)
				ln += 2;
			else
				ln++;
		}
	}
}


template<typename _PixelType, int _Width>
void v99x8_device::graphic7_draw_sprite(const pen_t *pens, _PixelType *ln, UINT8 *col)
{
	static const UINT16 g7_ind16[16] = {
		0, 2, 192, 194, 48, 50, 240, 242,
		482, 7, 448, 455, 56, 63, 504, 511  };
	int i;

	if (_Width > 512)
		ln += m_offset_x * 2;
	else
		ln += m_offset_x;

	for (i=0;i<256;i++)
	{
		if (col[i] & 0x80)
		{
			*ln++ = pens[g7_ind16[col[i]&0x0f]];
			if (_Width > 512)
				*ln++ = pens[g7_ind16[col[i]&0x0f]];
		}
		else
		{
			if (_Width > 512)
				ln += 2;
			else
				ln++;
		}
	}
}


void v99x8_device::sprite_mode1 (int line, UINT8 *col)
{
	UINT8	*attrtbl, *patterntbl, *patternptr;
	int x, y, p, height, c, p2, i, n, pattern;

	memset (col, 0, 256);

	// are sprites disabled?
	if (m_cont_reg[8] & 0x02) return;

	attrtbl = m_vram + (m_cont_reg[5] << 7) + (m_cont_reg[11] << 15);
	patterntbl = m_vram + (m_cont_reg[6] << 11);

	// 16x16 or 8x8 sprites
	height = (m_cont_reg[1] & 2) ? 16 : 8;
	// magnified sprites (zoomed)
	if (m_cont_reg[1] & 1) height *= 2;

	p2 = p = 0;
	while (1)
	{
		y = attrtbl[0];
		if (y == 208) break;
		y = (y - m_cont_reg[23]) & 255;
		if (y > 208)
			y = -(~y&255);
		else
			y++;

		// if sprite in range, has to be drawn
		if ( (line >= y) && (line  < (y + height) ) )
		{
			if (p2 == 4)
			{
				// max maximum sprites per line!
				if ( !(m_stat_reg[0] & 0x40) )
					m_stat_reg[0] = (m_stat_reg[0] & 0xa0) | 0x40 | p;

				if (m_sprite_limit) break;
			}
			// get x
			x = attrtbl[1];
			if (attrtbl[3] & 0x80) x -= 32;

			// get pattern
			pattern = attrtbl[2];
			if (m_cont_reg[1] & 2)
				pattern &= 0xfc;
			n = line - y;
			patternptr = patterntbl + pattern * 8 +
				((m_cont_reg[1] & 1) ? n/2  : n);
			pattern = patternptr[0] << 8 | patternptr[16];

			// get colour
			c = attrtbl[3] & 0x0f;

			// draw left part
			n = 0;
			while (1)
			{
				if (n == 0) pattern = patternptr[0];
				else if ( (n == 1) && (m_cont_reg[1] & 2) ) pattern = patternptr[16];
				else break;

				n++;

				for (i=0;i<8;i++)
				{
					if (pattern & 0x80)
					{
						if ( (x >= 0) && (x < 256) )
						{
							if (col[x] & 0x40)
							{
								// we have a collision!
								if (p2 < 4)
									m_stat_reg[0] |= 0x20;
							}
							if ( !(col[x] & 0x80) )
							{
								if (c || (m_cont_reg[8] & 0x20) )
									col[x] |= 0xc0 | c;
								else
									col[x] |= 0x40;
							}

							// if zoomed, draw another pixel
							if (m_cont_reg[1] & 1)
							{
								if (col[x+1] & 0x40)
    	                        {
    		                    	// we have a collision!
									if (p2 < 4)
										m_stat_reg[0] |= 0x20;
                            	}
                        		if ( !(col[x+1] & 0x80) )
	                            {
		                        	if (c || (m_cont_reg[8] & 0x20) )
										col[x+1] |= 0xc0 | c;
									else
										col[x+1] |= 0x80;
                            	}
							}
						}
					}
					if (m_cont_reg[1] & 1) x += 2; else x++;
					pattern <<= 1;
				}
			}

			p2++;
		}

		if (p >= 31) break;
		p++;
		attrtbl += 4;
	}

	if ( !(m_stat_reg[0] & 0x40) )
		m_stat_reg[0] = (m_stat_reg[0] & 0xa0) | p;
}

void v99x8_device::sprite_mode2 (int line, UINT8 *col)
{
	int attrtbl, patterntbl, patternptr, colourtbl;
	int x, i, y, p, height, c, p2, n, pattern, colourmask, first_cc_seen;

	memset (col, 0, 256);

	// are sprites disabled?
	if (m_cont_reg[8] & 0x02) return;

	attrtbl = ( (m_cont_reg[5] & 0xfc) << 7) + (m_cont_reg[11] << 15);
	colourtbl =  ( (m_cont_reg[5] & 0xf8) << 7) + (m_cont_reg[11] << 15);
	patterntbl = (m_cont_reg[6] << 11);
	colourmask = ( (m_cont_reg[5] & 3) << 3) | 0x7; // check this!

	// 16x16 or 8x8 sprites
	height = (m_cont_reg[1] & 2) ? 16 : 8;
	// magnified sprites (zoomed)
	if (m_cont_reg[1] & 1) height *= 2;

	p2 = p = first_cc_seen = 0;
	while (1)
	{
		y = vram_read (attrtbl);
		if (y == 216) break;
		y = (y - m_cont_reg[23]) & 255;
		if (y > 216)
			y = -(~y&255);
		else
			y++;

		// if sprite in range, has to be drawn
		if ( (line >= y) && (line  < (y + height) ) )
		{
			if (p2 == 8)
			{
				// max maximum sprites per line!
				if ( !(m_stat_reg[0] & 0x40) )
					m_stat_reg[0] = (m_stat_reg[0] & 0xa0) | 0x40 | p;

				if (m_sprite_limit) break;
			}

			n = line - y; if (m_cont_reg[1] & 1) n /= 2;
			// get colour
			c = vram_read (colourtbl + (((p&colourmask)*16) + n));

			// don't draw all sprite with CC set before any sprites
            // with CC = 0 are seen on this line
			if (c & 0x40)
			{
				if (!first_cc_seen)
					goto skip_first_cc_set;
			}
			else
				first_cc_seen = 1;

			// get pattern
			pattern = vram_read (attrtbl + 2);
			if (m_cont_reg[1] & 2)
				pattern &= 0xfc;
			patternptr = patterntbl + pattern * 8 + n;
			pattern = (vram_read (patternptr) << 8) |
				vram_read (patternptr + 16);

			// get x
			x = vram_read (attrtbl + 1);
			if (c & 0x80) x -= 32;

			n = (m_cont_reg[1] & 2) ? 16 : 8;
			while (n--)
			{
				for (i=0;i<=(m_cont_reg[1] & 1);i++)
				{
					if ( (x >= 0) && (x < 256) )
					{
						if ( (pattern & 0x8000) && !(col[x] & 0x10) )
						{
							if ( (c & 15) || (m_cont_reg[8] & 0x20) )
							{
								if ( !(c & 0x40) )
								{
									if (col[x] & 0x20) col[x] |= 0x10;
									else
										col[x] |= 0x20 | (c & 15);
								}
								else
									col[x] |= c & 15;

								col[x] |= 0x80;
							}
						}
						else
						{
							if ( !(c & 0x40) && (col[x] & 0x20) )
								col[x] |= 0x10;
						}

						if ( !(c & 0x60) && (pattern & 0x8000) )
						{
							if (col[x] & 0x40)
							{
								// sprite collision!
								if (p2 < 8)
									m_stat_reg[0] |= 0x20;
							}
							else
								col[x] |= 0x40;
						}

						x++;
					}
				}

				pattern <<= 1;
			}

skip_first_cc_set:
			p2++;
		}

		if (p >= 31) break;
		p++;
		attrtbl += 4;
	}

	if ( !(m_stat_reg[0] & 0x40) )
		m_stat_reg[0] = (m_stat_reg[0] & 0xa0) | p;
}

#define SHORT_WIDTH (256 + 16)
#define LONG_WIDTH (512 + 32)

const v99x8_device::v99x8_mode v99x8_device::s_modes[] = {
	{ 0x02,
		&v99x8_device::mode_text1<UINT16, LONG_WIDTH>,
		&v99x8_device::mode_text1<UINT16, SHORT_WIDTH>,
		&v99x8_device::default_border<UINT16, LONG_WIDTH>,
		&v99x8_device::default_border<UINT16, SHORT_WIDTH>,
		NULL,
		NULL,
		NULL },
	{ 0x01,
		&v99x8_device::mode_multi<UINT16, LONG_WIDTH>,
		&v99x8_device::mode_multi<UINT16, SHORT_WIDTH>,
		&v99x8_device::default_border<UINT16, LONG_WIDTH>,
		&v99x8_device::default_border<UINT16, SHORT_WIDTH>,
		&v99x8_device::sprite_mode1,
		&v99x8_device::default_draw_sprite<UINT16, LONG_WIDTH>,
		&v99x8_device::default_draw_sprite<UINT16, SHORT_WIDTH> },
	{ 0x00,
		&v99x8_device::mode_graphic1<UINT16, LONG_WIDTH>,
		&v99x8_device::mode_graphic1<UINT16, SHORT_WIDTH>,
		&v99x8_device::default_border<UINT16, LONG_WIDTH>,
		&v99x8_device::default_border<UINT16, SHORT_WIDTH>,
		&v99x8_device::sprite_mode1,
		&v99x8_device::default_draw_sprite<UINT16, LONG_WIDTH>,
		&v99x8_device::default_draw_sprite<UINT16, SHORT_WIDTH> },
	{ 0x04,
		&v99x8_device::mode_graphic23<UINT16, LONG_WIDTH>,
		&v99x8_device::mode_graphic23<UINT16, SHORT_WIDTH>,
		&v99x8_device::default_border<UINT16, LONG_WIDTH>,
		&v99x8_device::default_border<UINT16, SHORT_WIDTH>,
		&v99x8_device::sprite_mode1,
		&v99x8_device::default_draw_sprite<UINT16, LONG_WIDTH>,
		&v99x8_device::default_draw_sprite<UINT16, SHORT_WIDTH> },
	{ 0x08,
		&v99x8_device::mode_graphic23<UINT16, LONG_WIDTH>,
		&v99x8_device::mode_graphic23<UINT16, SHORT_WIDTH>,
		&v99x8_device::default_border<UINT16, LONG_WIDTH>,
		&v99x8_device::default_border<UINT16, SHORT_WIDTH>,
		&v99x8_device::sprite_mode2,
		&v99x8_device::default_draw_sprite<UINT16, LONG_WIDTH>,
		&v99x8_device::default_draw_sprite<UINT16, SHORT_WIDTH> },
	{ 0x0c,
		&v99x8_device::mode_graphic4<UINT16, LONG_WIDTH>,
		&v99x8_device::mode_graphic4<UINT16, SHORT_WIDTH>,
		&v99x8_device::default_border<UINT16, LONG_WIDTH>,
		&v99x8_device::default_border<UINT16, SHORT_WIDTH>,
		&v99x8_device::sprite_mode2,
		&v99x8_device::default_draw_sprite<UINT16, LONG_WIDTH>,
		&v99x8_device::default_draw_sprite<UINT16, SHORT_WIDTH> },
	{ 0x10,
		&v99x8_device::mode_graphic5<UINT16, LONG_WIDTH>,
		&v99x8_device::mode_graphic5<UINT16, SHORT_WIDTH>,
		&v99x8_device::graphic5_border<UINT16, LONG_WIDTH>,
		&v99x8_device::graphic5_border<UINT16, SHORT_WIDTH>,
		&v99x8_device::sprite_mode2,
		&v99x8_device::graphic5_draw_sprite<UINT16, LONG_WIDTH>,
		&v99x8_device::graphic5_draw_sprite<UINT16, SHORT_WIDTH> },
	{ 0x14,
		&v99x8_device::mode_graphic6<UINT16, LONG_WIDTH>,
		&v99x8_device::mode_graphic6<UINT16, SHORT_WIDTH>,
		&v99x8_device::default_border<UINT16, LONG_WIDTH>,
		&v99x8_device::default_border<UINT16, SHORT_WIDTH>,
		&v99x8_device::sprite_mode2,
		&v99x8_device::default_draw_sprite<UINT16, LONG_WIDTH>,
		&v99x8_device::default_draw_sprite<UINT16, SHORT_WIDTH> },
	{ 0x1c,
		&v99x8_device::mode_graphic7<UINT16, LONG_WIDTH>,
		&v99x8_device::mode_graphic7<UINT16, SHORT_WIDTH>,
		&v99x8_device::graphic7_border<UINT16, LONG_WIDTH>,
		&v99x8_device::graphic7_border<UINT16, SHORT_WIDTH>,
		&v99x8_device::sprite_mode2,
		&v99x8_device::graphic7_draw_sprite<UINT16, LONG_WIDTH>,
		&v99x8_device::graphic7_draw_sprite<UINT16, SHORT_WIDTH> },
	{ 0x0a,
		&v99x8_device::mode_text2<UINT16, LONG_WIDTH>,
		&v99x8_device::mode_text2<UINT16, SHORT_WIDTH>,
		&v99x8_device::default_border<UINT16, LONG_WIDTH>,
		&v99x8_device::default_border<UINT16, SHORT_WIDTH>,
		NULL,
		NULL,
		NULL },
	{ 0xff,
		&v99x8_device::mode_unknown<UINT16, LONG_WIDTH>,
		&v99x8_device::mode_unknown<UINT16, SHORT_WIDTH>,
		&v99x8_device::default_border<UINT16, LONG_WIDTH>,
		&v99x8_device::default_border<UINT16, SHORT_WIDTH>,
		NULL,
		NULL,
		NULL },
};

void v99x8_device::set_mode ()
{
	int n,i;

	n = (((m_cont_reg[0] & 0x0e) << 1) | ((m_cont_reg[1] & 0x18) >> 3));
	for (i=0;;i++)
	{
		if ( (s_modes[i].m == n) || (s_modes[i].m == 0xff) ) break;
	}
	m_mode = i;
}

void v99x8_device::refresh_16 (int line)
{
	const pen_t *pens = machine().pens;
	int i, double_lines;
	UINT8 col[256];
	UINT16 *ln, *ln2 = NULL;

	double_lines = 0;

	if (m_size == RENDER_HIGH)
	{
		if (m_cont_reg[9] & 0x08)
		{
			m_size_now = RENDER_HIGH;
			ln = &m_bitmap.pix16(line*2+((m_stat_reg[2]>>1)&1));
		}
		else
		{
			ln = &m_bitmap.pix16(line*2);
			ln2 = &m_bitmap.pix16(line*2+1);
			double_lines = 1;
		}
	}
	else
		ln = &m_bitmap.pix16(line);

	if ( !(m_cont_reg[1] & 0x40) || (m_stat_reg[2] & 0x40) )
	{
		if (m_size == RENDER_HIGH)
			(this->*s_modes[m_mode].border_16) (pens, ln);
		else
			(this->*s_modes[m_mode].border_16s) (pens, ln);
	}
	else
	{
		i = (line - m_offset_y) & 255;
		if (m_size == RENDER_HIGH)
		{
			(this->*s_modes[m_mode].visible_16) (pens, ln, i);
			if (s_modes[m_mode].sprites)
			{
				(this->*s_modes[m_mode].sprites) (i, col);
				(this->*s_modes[m_mode].draw_sprite_16) (pens, ln, col);
			}
		}
		else
		{
			(this->*s_modes[m_mode].visible_16s) (pens, ln, i);
			if (s_modes[m_mode].sprites)
			{
				(this->*s_modes[m_mode].sprites) (i, col);
				(this->*s_modes[m_mode].draw_sprite_16s) (pens, ln, col);
			}
		}
	}

	if (double_lines)
		memcpy (ln2, ln, (512 + 32) * 2);
}

void v99x8_device::refresh_line (int line)
{
	int ind16, ind256;

	ind16 = m_pal_ind16[0];
	ind256 = m_pal_ind256[0];

	if ( !(m_cont_reg[8] & 0x20) && (m_mode != V9938_MODE_GRAPHIC5) )
	{
		m_pal_ind16[0] = m_pal_ind16[(m_cont_reg[7] & 0x0f)];
		m_pal_ind256[0] = m_pal_ind256[m_cont_reg[7]];
	}

	refresh_16 (line);

	if ( !(m_cont_reg[8] & 0x20) && (m_mode != V9938_MODE_GRAPHIC5) )
	{
		m_pal_ind16[0] = ind16;
		m_pal_ind256[0] = ind256;
	}
}

/*

From: awulms@inter.nl.net (Alex Wulms)
*** About the HR/VR topic: this is how it works according to me:

*** HR:
HR is very straightforward:
-HR=1 during 'display time'
-HR=0 during 'horizontal border, horizontal retrace'
I have put 'display time' and 'horizontal border, horizontal retrace' between
quotes because HR does not only flip between 0 and 1 during the display of
the 192/212 display lines, but also during the vertical border and during the
vertical retrace.

*** VR:
VR is a little bit tricky
-VR always gets set to 0 when the VDP starts with display line 0
-VR gets set to 1 when the VDP reaches display line (192 if LN=0) or (212 if
LN=1)
-The VDP displays contents of VRAM as long as VR=0

As a consequence of this behaviour, it is possible to program the famous
overscan trick, where VRAM contents is shown in the borders:
Generate an interrupt at line 230 (or so) and on this interrupt: set LN=1
Generate an interrupt at line 200 (or so) and on this interrupt: set LN=0
Repeat the above two steps

*** The top/bottom border contents during overscan:
On screen 0:
1) The VDP keeps increasing the name table address pointer during bottom
border, vertical retrace and top border
2) The VDP resets the name table address pointer when the first display line
is reached

On the other screens:
1) The VDP keeps increasing the name table address pointer during the bottom
border
2) The VDP resets the name table address pointer such that the top border
contents connects up with the first display line. E.g., when the top border
is 26 lines high, the VDP will take:
'logical'      vram line
TOPB000  256-26
...
TOPB025  256-01
DISPL000 000
...
DISPL211 211
BOTB000  212
...
BOTB024  236



*** About the horizontal interrupt

All relevant definitions on a row:
-FH: Bit 0 of status register 1
-IE1: Bit 4 of mode register 0
-IL: Line number in mode register 19
-DL: The line that the VDP is going to display (corrected for vertical scroll)
-IRQ: Interrupt request line of VDP to Z80

At the *start* of every new line (display, bottom border, part of vertical
display), the VDP does:
-FH = (FH && IE1) || (IL==DL)

After reading of status register 1 by the CPU, the VDP does:
-FH = 0

Furthermore, the following is true all the time:
-IRQ = FH && IE1

The resulting behaviour:
When IE1=0:
-FH will be set as soon as display of line IL starts
-FH will be reset as soon as status register 1 is read
-FH will be reset as soon as the next display line is reached

When IE=1:
-FH and IRQ will be set as soon as display line IL is reached
-FH and IRQ will be reset as soon as status register 1 is read

Another subtile result:
If, while FH and IRQ are set, IE1 gets reset, the next happens:
-IRQ is reset immediately (since IRQ is always FH && IE1)
-FH will be reset as soon as display of the next line starts (unless the next
line is line IL)


*** About the vertical interrupt:
Another relevant definition:
-FV: Bit 7 of status register 0
-IE0: Bit 5 of mode register 1

I only know for sure the behaviour when IE0=1:
-FV and IRQ will be set as soon as VR changes from 0 to 1
-FV and IRQ will be reset as soon as status register 0 is read

A consequence is that NO vertical interrupts will be generated during the
overscan trick, described in the VR section above.

I do not know the behaviour of FV when IE0=0. That is the part that I still
have to test.
*/

void v99x8_device::interrupt_start_vblank ()
{
#if 0
	if (machine.input().code_pressed (KEYCODE_D) )
	{
		FILE *fp;
		int i;

		fp = fopen ("vram.dmp", "wb");
		if (fp)
		{
			fwrite (m_vram, 0x10000, 1, fp);
			fclose (fp);
			popmessage("saved");
		}

		for (i=0;i<24;i++) mame_printf_debug ("R#%d = %02x\n", i, m_cont_reg[i]);
	}
#endif

	// at every frame, vdp switches fields
	m_stat_reg[2] = (m_stat_reg[2] & 0xfd) | (~m_stat_reg[2] & 2);

	// color blinking
	if (!(m_cont_reg[13] & 0xf0))
		m_blink = 0;
	else if (!(m_cont_reg[13] & 0x0f))
		m_blink = 1;
	else
	{
		// both on and off counter are non-zero: timed blinking
		if (m_blink_count)
			m_blink_count--;
		if (!m_blink_count)
		{
			m_blink = !m_blink;
			if (m_blink)
				m_blink_count = (m_cont_reg[13] >> 4) * 10;
			else
				m_blink_count = (m_cont_reg[13] & 0x0f) * 10;
		}
	}

	// check screen rendering size
	if (m_size_auto && (m_size_now >= 0) && (m_size != m_size_now) )
		m_size = m_size_now;

	if (m_size != m_size_old)
	{
		if (m_size == RENDER_HIGH)
			m_screen->set_visible_area (0, 512 + 32 - 1, 0, 424 + 56 - 1);
		else
			m_screen->set_visible_area (0, 256 + 16 - 1, 0, 212 + 28 - 1);

		m_size_old = m_size;
	}

	m_size_now = -1;
}

/***************************************************************************

    Command unit

***************************************************************************/

#define VDP m_cont_reg
#define VDPStatus m_stat_reg
#define VRAM m_vram
#define VRAM_EXP m_vram_exp
#define ScrMode m_mode

/*************************************************************/
/** Completely rewritten by Alex Wulms:                     **/
/**  - VDP Command execution 'in parallel' with CPU         **/
/**  - Corrected behaviour of VDP commands                  **/
/**  - Made it easier to implement correct S7/8 mapping     **/
/**    by concentrating VRAM access in one single place     **/
/**  - Made use of the 'in parallel' VDP command exec       **/
/**    and correct timing. You must call the function       **/
/**    LoopVDP() from LoopZ80 in MSX.c. You must call it    **/
/**    exactly 256 times per screen refresh.                **/
/** Started on       : 11-11-1999                           **/
/** Beta release 1 on:  9-12-1999                           **/
/** Beta release 2 on: 20-01-2000                           **/
/**  - Corrected behaviour of VRM <-> Z80 transfer          **/
/**  - Improved performance of the code                     **/
/** Public release 1.0: 20-04-2000                          **/
/*************************************************************/

#define VDP_VRMP5(MX, X, Y) ((!MX) ? (VRAM + ((Y&1023)<<7) + ((X&255)>>1)) : (VRAM_EXP + ((Y&511)<<7) + ((X&255)>>1)))
#define VDP_VRMP6(MX, X, Y) ((!MX) ? (VRAM + ((Y&1023)<<7) + ((X&511)>>2)) : (VRAM_EXP + ((Y&511)<<7) + ((X&511)>>2)))
//#define VDP_VRMP7(MX, X, Y) ((!MX) ? (VRAM + ((Y&511)<<8) + ((X&511)>>1)) : (VRAM_EXP + ((Y&255)<<8) + ((X&511)>>1)))
#define VDP_VRMP7(MX, X, Y) ((!MX) ? (VRAM + ((X&2)<<15) + ((Y&511)<<7) + ((X&511)>>2)) : (VRAM_EXP + ((Y&511)<<7) + ((X&511)>>2))/*(VRAM_EXP + ((Y&255)<<8) + ((X&511)>>1))*/)
//#define VDP_VRMP8(MX, X, Y) ((!MX) ? (VRAM + ((Y&511)<<8) + (X&255)) : (VRAM_EXP + ((Y&255)<<8) + (X&255)))
#define VDP_VRMP8(MX, X, Y) ((!MX) ? (VRAM + ((X&1)<<16) + ((Y&511)<<7) + ((X>>1)&127)) : (VRAM_EXP + ((Y&511)<<7) + ((X>>1)&127))/*(VRAM_EXP + ((Y&255)<<8) + (X&255))*/)

#define VDP_VRMP(M, MX, X, Y) VDPVRMP(M, MX, X, Y)
#define VDP_POINT(M, MX, X, Y) VDPpoint(M, MX, X, Y)
#define VDP_PSET(M, MX, X, Y, C, O) VDPpset(M, MX, X, Y, C, O)

#define CM_ABRT  0x0
#define CM_POINT 0x4
#define CM_PSET  0x5
#define CM_SRCH  0x6
#define CM_LINE  0x7
#define CM_LMMV  0x8
#define CM_LMMM  0x9
#define CM_LMCM  0xA
#define CM_LMMC  0xB
#define CM_HMMV  0xC
#define CM_HMMM  0xD
#define CM_YMMM  0xE
#define CM_HMMC  0xF

/*************************************************************/
// Many VDP commands are executed in some kind of loop but
// essentially, there are only a few basic loop structures
// that are re-used. We define the loop structures that are
// re-used here so that they have to be entered only once
/*************************************************************/
#define pre_loop \
    while ((cnt-=delta) > 0) {

#define post_loop \
    }

// Loop over DX, DY
#define post__x_y(MX) \
    if (!--ANX || ((ADX+=TX)&MX)) { \
      if (!(--NY&1023) || (DY+=TY)==-1) \
        break; \
      else { \
        ADX=DX; \
        ANX=NX; \
      } \
    } \
  post_loop

// Loop over DX, SY, DY
#define post__xyy(MX) \
    if ((ADX+=TX)&MX) { \
      if (!(--NY&1023) || (SY+=TY)==-1 || (DY+=TY)==-1) \
        break; \
      else \
        ADX=DX; \
    } \
  post_loop

// Loop over SX, DX, SY, DY
#define post_xxyy(MX) \
    if (!--ANX || ((ASX+=TX)&MX) || ((ADX+=TX)&MX)) { \
      if (!(--NY&1023) || (SY+=TY)==-1 || (DY+=TY)==-1) \
        break; \
      else { \
        ASX=SX; \
        ADX=DX; \
        ANX=NX; \
      } \
    } \
  post_loop

/*************************************************************/
/** Variables visible only in this module                   **/
/*************************************************************/
static const UINT8 Mask[4] = { 0x0F,0x03,0x0F,0xFF };
static const int  PPB[4]  = { 2,4,2,1 };
static const int  PPL[4]  = { 256,512,512,256 };

//  SprOn SprOn SprOf SprOf
//  ScrOf ScrOn ScrOf ScrOn
static const int srch_timing[8]={
  818, 1025,  818,  830, // ntsc
  696,  854,  696,  684  // pal
};
static const int line_timing[8]={
  1063, 1259, 1063, 1161,
  904,  1026, 904,  953
};
static const int hmmv_timing[8]={
  439,  549,  439,  531,
  366,  439,  366,  427
};
static const int lmmv_timing[8]={
  873,  1135, 873, 1056,
  732,  909,  732,  854
};
static const int ymmm_timing[8]={
  586,  952,  586,  610,
  488,  720,  488,  500
};
static const int hmmm_timing[8]={
  818,  1111, 818,  854,
  684,  879,  684,  708
};
static const int lmmm_timing[8]={
  1160, 1599, 1160, 1172,
  964,  1257, 964,  977
};

/** VDPVRMP() **********************************************/
/** Calculate addr of a pixel in vram                       **/
/*************************************************************/
inline UINT8 *v99x8_device::VDPVRMP(UINT8 M,int MX,int X,int Y)
{
  switch(M)
  {
    case 0: return VDP_VRMP5(MX,X,Y);
    case 1: return VDP_VRMP6(MX,X,Y);
    case 2: return VDP_VRMP7(MX,X,Y);
    case 3: return VDP_VRMP8(MX,X,Y);
  }

  return(VRAM);
}

/** VDPpoint5() ***********************************************/
/** Get a pixel on screen 5                                 **/
/*************************************************************/
inline UINT8 v99x8_device::VDPpoint5(int MXS, int SX, int SY)
{
  return (*VDP_VRMP5(MXS, SX, SY) >>
          (((~SX)&1)<<2)
         )&15;
}

/** VDPpoint6() ***********************************************/
/** Get a pixel on screen 6                                 **/
/*************************************************************/
inline UINT8 v99x8_device::VDPpoint6(int MXS, int SX, int SY)
{
  return (*VDP_VRMP6(MXS, SX, SY) >>
          (((~SX)&3)<<1)
         )&3;
}

/** VDPpoint7() ***********************************************/
/** Get a pixel on screen 7                                 **/
/*************************************************************/
inline UINT8 v99x8_device::VDPpoint7(int MXS, int SX, int SY)
{
  return (*VDP_VRMP7(MXS, SX, SY) >>
          (((~SX)&1)<<2)
         )&15;
}

/** VDPpoint8() ***********************************************/
/** Get a pixel on screen 8                                 **/
/*************************************************************/
inline UINT8 v99x8_device::VDPpoint8(int MXS, int SX, int SY)
{
  return *VDP_VRMP8(MXS, SX, SY);
}

/** VDPpoint() ************************************************/
/** Get a pixel on a screen                                 **/
/*************************************************************/
inline UINT8 v99x8_device::VDPpoint(UINT8 SM, int MXS, int SX, int SY)
{
  switch(SM)
  {
    case 0: return VDPpoint5(MXS,SX,SY);
    case 1: return VDPpoint6(MXS,SX,SY);
    case 2: return VDPpoint7(MXS,SX,SY);
    case 3: return VDPpoint8(MXS,SX,SY);
  }

  return(0);
}

/** VDPpsetlowlevel() ****************************************/
/** Low level function to set a pixel on a screen           **/
/** Make it inline to make it fast                          **/
/*************************************************************/
inline void v99x8_device::VDPpsetlowlevel(UINT8 *P, UINT8 CL, UINT8 M, UINT8 OP)
{
  switch (OP)
  {
    case 0: *P = (*P & M) | CL; break;
    case 1: *P = *P & (CL | M); break;
    case 2: *P |= CL; break;
    case 3: *P ^= CL; break;
    case 4: *P = (*P & M) | ~(CL | M); break;
    case 8: if (CL) *P = (*P & M) | CL; break;
    case 9: if (CL) *P = *P & (CL | M); break;
    case 10: if (CL) *P |= CL; break;
    case 11:  if (CL) *P ^= CL; break;
    case 12:  if (CL) *P = (*P & M) | ~(CL|M); break;
  }
}

/** VDPpset5() ***********************************************/
/** Set a pixel on screen 5                                 **/
/*************************************************************/
inline void v99x8_device::VDPpset5(int MXD, int DX, int DY, UINT8 CL, UINT8 OP)
{
  UINT8 SH = ((~DX)&1)<<2;

  VDPpsetlowlevel(VDP_VRMP5(MXD, DX, DY),
                  CL << SH, ~(15<<SH), OP);
}

/** VDPpset6() ***********************************************/
/** Set a pixel on screen 6                                 **/
/*************************************************************/
inline void v99x8_device::VDPpset6(int MXD, int DX, int DY, UINT8 CL, UINT8 OP)
{
  UINT8 SH = ((~DX)&3)<<1;

  VDPpsetlowlevel(VDP_VRMP6(MXD, DX, DY),
                  CL << SH, ~(3<<SH), OP);
}

/** VDPpset7() ***********************************************/
/** Set a pixel on screen 7                                 **/
/*************************************************************/
inline void v99x8_device::VDPpset7(int MXD, int DX, int DY, UINT8 CL, UINT8 OP)
{
  UINT8 SH = ((~DX)&1)<<2;

  VDPpsetlowlevel(VDP_VRMP7(MXD, DX, DY),
                  CL << SH, ~(15<<SH), OP);
}

/** VDPpset8() ***********************************************/
/** Set a pixel on screen 8                                 **/
/*************************************************************/
inline void v99x8_device::VDPpset8(int MXD, int DX, int DY, UINT8 CL, UINT8 OP)
{
  VDPpsetlowlevel(VDP_VRMP8(MXD, DX, DY),
                  CL, 0, OP);
}

/** VDPpset() ************************************************/
/** Set a pixel on a screen                                 **/
/*************************************************************/
inline void v99x8_device::VDPpset(UINT8 SM, int MXD, int DX, int DY, UINT8 CL, UINT8 OP)
{
  switch (SM) {
    case 0: VDPpset5(MXD, DX, DY, CL, OP); break;
    case 1: VDPpset6(MXD, DX, DY, CL, OP); break;
    case 2: VDPpset7(MXD, DX, DY, CL, OP); break;
    case 3: VDPpset8(MXD, DX, DY, CL, OP); break;
  }
}

/** GetVdpTimingValue() **************************************/
/** Get timing value for a certain VDP command              **/
/*************************************************************/
int v99x8_device::GetVdpTimingValue(const int *timing_values)
{
  return(timing_values[((VDP[1]>>6)&1)|(VDP[8]&2)|((VDP[9]<<1)&4)]);
}

/** SrchEgine()** ********************************************/
/** Search a dot                                            **/
/*************************************************************/
void v99x8_device::SrchEngine()
{
  int SX=m_mmc.SX;
  int SY=m_mmc.SY;
  int TX=m_mmc.TX;
  int ANX=m_mmc.ANX;
  UINT8 CL=m_mmc.CL;
  int MXD = m_mmc.MXD;
  int cnt;
  int delta;

  delta = GetVdpTimingValue(srch_timing);
  cnt = m_vdp_ops_count;

#define pre_srch \
    pre_loop \
      if ((
#define post_srch(MX) \
           ==CL) ^ANX) { \
      VDPStatus[2]|=0x10; /* Border detected */\
      break; \
    } \
    if ((SX+=TX) & MX) { \
      VDPStatus[2]&=0xEF; /* Border not detected */\
      break; \
    } \
  post_loop

  switch (ScrMode) {
    default:
    case 5: pre_srch VDPpoint5(MXD, SX, SY) post_srch(256)
            break;
    case 6: pre_srch VDPpoint6(MXD, SX, SY) post_srch(512)
            break;
    case 7: pre_srch VDPpoint7(MXD, SX, SY) post_srch(512)
            break;
    case 8: pre_srch VDPpoint8(MXD, SX, SY) post_srch(256)
            break;
  }

  if ((m_vdp_ops_count=cnt)>0) {
    // Command execution done
    VDPStatus[2]&=0xFE;
    m_vdp_engine=0;
    // Update SX in VDP registers
    VDPStatus[8]=SX&0xFF;
    VDPStatus[9]=(SX>>8)|0xFE;
  }
  else {
    m_mmc.SX=SX;
  }
}

/** LineEgine()** ********************************************/
/** Draw a line                                             **/
/*************************************************************/
void v99x8_device::LineEngine()
{
  int DX=m_mmc.DX;
  int DY=m_mmc.DY;
  int TX=m_mmc.TX;
  int TY=m_mmc.TY;
  int NX=m_mmc.NX;
  int NY=m_mmc.NY;
  int ASX=m_mmc.ASX;
  int ADX=m_mmc.ADX;
  UINT8 CL=m_mmc.CL;
  UINT8 LO=m_mmc.LO;
  int MXD = m_mmc.MXD;
  int cnt;
  int delta;

  delta = GetVdpTimingValue(line_timing);
  cnt = m_vdp_ops_count;

#define post_linexmaj(MX) \
      DX+=TX; \
      if ((ASX-=NY)<0) { \
        ASX+=NX; \
        DY+=TY; \
      } \
      ASX&=1023; /* Mask to 10 bits range */\
      if (ADX++==NX || (DX&MX)) \
        break; \
    post_loop

#define post_lineymaj(MX) \
      DY+=TY; \
      if ((ASX-=NY)<0) { \
        ASX+=NX; \
        DX+=TX; \
      } \
      ASX&=1023; /* Mask to 10 bits range */\
      if (ADX++==NX || (DX&MX)) \
        break; \
    post_loop

  if ((VDP[45]&0x01)==0)
    // X-Axis is major direction
    switch (ScrMode) {
      default:
      case 5: pre_loop VDPpset5(MXD, DX, DY, CL, LO); post_linexmaj(256)
              break;
      case 6: pre_loop VDPpset6(MXD, DX, DY, CL, LO); post_linexmaj(512)
              break;
      case 7: pre_loop VDPpset7(MXD, DX, DY, CL, LO); post_linexmaj(512)
              break;
      case 8: pre_loop VDPpset8(MXD, DX, DY, CL, LO); post_linexmaj(256)
              break;
    }
  else
    // Y-Axis is major direction
    switch (ScrMode) {
      default:
      case 5: pre_loop VDPpset5(MXD, DX, DY, CL, LO); post_lineymaj(256)
              break;
      case 6: pre_loop VDPpset6(MXD, DX, DY, CL, LO); post_lineymaj(512)
              break;
      case 7: pre_loop VDPpset7(MXD, DX, DY, CL, LO); post_lineymaj(512)
              break;
      case 8: pre_loop VDPpset8(MXD, DX, DY, CL, LO); post_lineymaj(256)
              break;
    }

  if ((m_vdp_ops_count=cnt)>0) {
    // Command execution done
    VDPStatus[2]&=0xFE;
    m_vdp_engine=0;
    VDP[38]=DY & 0xFF;
    VDP[39]=(DY>>8) & 0x03;
  }
  else {
    m_mmc.DX=DX;
    m_mmc.DY=DY;
    m_mmc.ASX=ASX;
    m_mmc.ADX=ADX;
  }
}

/** LmmvEngine() *********************************************/
/** VDP -> Vram                                             **/
/*************************************************************/
void v99x8_device::LmmvEngine()
{
  int DX=m_mmc.DX;
  int DY=m_mmc.DY;
  int TX=m_mmc.TX;
  int TY=m_mmc.TY;
  int NX=m_mmc.NX;
  int NY=m_mmc.NY;
  int ADX=m_mmc.ADX;
  int ANX=m_mmc.ANX;
  UINT8 CL=m_mmc.CL;
  UINT8 LO=m_mmc.LO;
  int MXD = m_mmc.MXD;
  int cnt;
  int delta;

  delta = GetVdpTimingValue(lmmv_timing);
  cnt = m_vdp_ops_count;

  switch (ScrMode) {
    default:
    case 5: pre_loop VDPpset5(MXD, ADX, DY, CL, LO); post__x_y(256)
            break;
    case 6: pre_loop VDPpset6(MXD, ADX, DY, CL, LO); post__x_y(512)
            break;
    case 7: pre_loop VDPpset7(MXD, ADX, DY, CL, LO); post__x_y(512)
            break;
    case 8: pre_loop VDPpset8(MXD, ADX, DY, CL, LO); post__x_y(256)
            break;
  }

  if ((m_vdp_ops_count=cnt)>0) {
    // Command execution done
    VDPStatus[2]&=0xFE;
    m_vdp_engine=0;
    if (!NY)
      DY+=TY;
    VDP[38]=DY & 0xFF;
    VDP[39]=(DY>>8) & 0x03;
    VDP[42]=NY & 0xFF;
    VDP[43]=(NY>>8) & 0x03;
  }
  else {
    m_mmc.DY=DY;
    m_mmc.NY=NY;
    m_mmc.ANX=ANX;
    m_mmc.ADX=ADX;
  }
}

/** LmmmEngine() *********************************************/
/** Vram -> Vram                                            **/
/*************************************************************/
void v99x8_device::LmmmEngine()
{
  int SX=m_mmc.SX;
  int SY=m_mmc.SY;
  int DX=m_mmc.DX;
  int DY=m_mmc.DY;
  int TX=m_mmc.TX;
  int TY=m_mmc.TY;
  int NX=m_mmc.NX;
  int NY=m_mmc.NY;
  int ASX=m_mmc.ASX;
  int ADX=m_mmc.ADX;
  int ANX=m_mmc.ANX;
  UINT8 LO=m_mmc.LO;
  int MXS = m_mmc.MXS;
  int MXD = m_mmc.MXD;
  int cnt;
  int delta;

  delta = GetVdpTimingValue(lmmm_timing);
  cnt = m_vdp_ops_count;

  switch (ScrMode) {
    default:
    case 5: pre_loop VDPpset5(MXD, ADX, DY, VDPpoint5(MXS, ASX, SY), LO); post_xxyy(256)
            break;
    case 6: pre_loop VDPpset6(MXD, ADX, DY, VDPpoint6(MXS, ASX, SY), LO); post_xxyy(512)
            break;
    case 7: pre_loop VDPpset7(MXD, ADX, DY, VDPpoint7(MXS, ASX, SY), LO); post_xxyy(512)
            break;
    case 8: pre_loop VDPpset8(MXD, ADX, DY, VDPpoint8(MXS, ASX, SY), LO); post_xxyy(256)
            break;
  }

  if ((m_vdp_ops_count=cnt)>0) {
    // Command execution done
    VDPStatus[2]&=0xFE;
    m_vdp_engine=0;
    if (!NY) {
      SY+=TY;
      DY+=TY;
    }
    else
      if (SY==-1)
        DY+=TY;
    VDP[42]=NY & 0xFF;
    VDP[43]=(NY>>8) & 0x03;
    VDP[34]=SY & 0xFF;
    VDP[35]=(SY>>8) & 0x03;
    VDP[38]=DY & 0xFF;
    VDP[39]=(DY>>8) & 0x03;
  }
  else {
    m_mmc.SY=SY;
    m_mmc.DY=DY;
    m_mmc.NY=NY;
    m_mmc.ANX=ANX;
    m_mmc.ASX=ASX;
    m_mmc.ADX=ADX;
  }
}

/** LmcmEngine() *********************************************/
/** Vram -> CPU                                             **/
/*************************************************************/
void v99x8_device::LmcmEngine()
{
  if ((VDPStatus[2]&0x80)!=0x80) {

    VDPStatus[7]=VDP[44]=VDP_POINT(((ScrMode >= 5) && (ScrMode <= 8)) ? (ScrMode-5) : 0, m_mmc.MXS, m_mmc.ASX, m_mmc.SY);
    m_vdp_ops_count-=GetVdpTimingValue(lmmv_timing);
    VDPStatus[2]|=0x80;

    if (!--m_mmc.ANX || ((m_mmc.ASX+=m_mmc.TX)&m_mmc.MX)) {
      if (!(--m_mmc.NY & 1023) || (m_mmc.SY+=m_mmc.TY)==-1) {
        VDPStatus[2]&=0xFE;
        m_vdp_engine=0;
        if (!m_mmc.NY)
          m_mmc.DY+=m_mmc.TY;
        VDP[42]=m_mmc.NY & 0xFF;
        VDP[43]=(m_mmc.NY>>8) & 0x03;
        VDP[34]=m_mmc.SY & 0xFF;
        VDP[35]=(m_mmc.SY>>8) & 0x03;
      }
      else {
        m_mmc.ASX=m_mmc.SX;
        m_mmc.ANX=m_mmc.NX;
      }
    }
  }
}

/** LmmcEngine() *********************************************/
/** CPU -> Vram                                             **/
/*************************************************************/
void v99x8_device::LmmcEngine()
{
  if ((VDPStatus[2]&0x80)!=0x80) {
    UINT8 SM=((ScrMode >= 5) && (ScrMode <= 8)) ? (ScrMode-5) : 0;

    VDPStatus[7]=VDP[44]&=Mask[SM];
    VDP_PSET(SM, m_mmc.MXD, m_mmc.ADX, m_mmc.DY, VDP[44], m_mmc.LO);
    m_vdp_ops_count-=GetVdpTimingValue(lmmv_timing);
    VDPStatus[2]|=0x80;

    if (!--m_mmc.ANX || ((m_mmc.ADX+=m_mmc.TX)&m_mmc.MX)) {
      if (!(--m_mmc.NY&1023) || (m_mmc.DY+=m_mmc.TY)==-1) {
        VDPStatus[2]&=0xFE;
        m_vdp_engine=0;
        if (!m_mmc.NY)
          m_mmc.DY+=m_mmc.TY;
        VDP[42]=m_mmc.NY & 0xFF;
        VDP[43]=(m_mmc.NY>>8) & 0x03;
        VDP[38]=m_mmc.DY & 0xFF;
        VDP[39]=(m_mmc.DY>>8) & 0x03;
      }
      else {
        m_mmc.ADX=m_mmc.DX;
        m_mmc.ANX=m_mmc.NX;
      }
    }
  }
}

/** HmmvEngine() *********************************************/
/** VDP --> Vram                                            **/
/*************************************************************/
void v99x8_device::HmmvEngine()
{
  int DX=m_mmc.DX;
  int DY=m_mmc.DY;
  int TX=m_mmc.TX;
  int TY=m_mmc.TY;
  int NX=m_mmc.NX;
  int NY=m_mmc.NY;
  int ADX=m_mmc.ADX;
  int ANX=m_mmc.ANX;
  UINT8 CL=m_mmc.CL;
  int MXD = m_mmc.MXD;
  int cnt;
  int delta;

  delta = GetVdpTimingValue(hmmv_timing);
  cnt = m_vdp_ops_count;

  switch (ScrMode) {
    default:
    case 5: pre_loop *VDP_VRMP5(MXD, ADX, DY) = CL; post__x_y(256)
            break;
    case 6: pre_loop *VDP_VRMP6(MXD, ADX, DY) = CL; post__x_y(512)
            break;
    case 7: pre_loop *VDP_VRMP7(MXD, ADX, DY) = CL; post__x_y(512)
            break;
    case 8: pre_loop *VDP_VRMP8(MXD, ADX, DY) = CL; post__x_y(256)
            break;
  }

  if ((m_vdp_ops_count=cnt)>0) {
    // Command execution done
    VDPStatus[2]&=0xFE;
    m_vdp_engine=0;
    if (!NY)
      DY+=TY;
    VDP[42]=NY & 0xFF;
    VDP[43]=(NY>>8) & 0x03;
    VDP[38]=DY & 0xFF;
    VDP[39]=(DY>>8) & 0x03;
  }
  else {
    m_mmc.DY=DY;
    m_mmc.NY=NY;
    m_mmc.ANX=ANX;
    m_mmc.ADX=ADX;
  }
}

/** HmmmEngine() *********************************************/
/** Vram -> Vram                                            **/
/*************************************************************/
void v99x8_device::HmmmEngine()
{
  int SX=m_mmc.SX;
  int SY=m_mmc.SY;
  int DX=m_mmc.DX;
  int DY=m_mmc.DY;
  int TX=m_mmc.TX;
  int TY=m_mmc.TY;
  int NX=m_mmc.NX;
  int NY=m_mmc.NY;
  int ASX=m_mmc.ASX;
  int ADX=m_mmc.ADX;
  int ANX=m_mmc.ANX;
  int MXS = m_mmc.MXS;
  int MXD = m_mmc.MXD;
  int cnt;
  int delta;

  delta = GetVdpTimingValue(hmmm_timing);
  cnt = m_vdp_ops_count;

  switch (ScrMode) {
    default:
    case 5: pre_loop *VDP_VRMP5(MXD, ADX, DY) = *VDP_VRMP5(MXS, ASX, SY); post_xxyy(256)
            break;
    case 6: pre_loop *VDP_VRMP6(MXD, ADX, DY) = *VDP_VRMP6(MXS, ASX, SY); post_xxyy(512)
            break;
    case 7: pre_loop *VDP_VRMP7(MXD, ADX, DY) = *VDP_VRMP7(MXS, ASX, SY); post_xxyy(512)
            break;
    case 8: pre_loop *VDP_VRMP8(MXD, ADX, DY) = *VDP_VRMP8(MXS, ASX, SY); post_xxyy(256)
            break;
  }

  if ((m_vdp_ops_count=cnt)>0) {
    // Command execution done
    VDPStatus[2]&=0xFE;
    m_vdp_engine=0;
    if (!NY) {
      SY+=TY;
      DY+=TY;
    }
    else
      if (SY==-1)
        DY+=TY;
    VDP[42]=NY & 0xFF;
    VDP[43]=(NY>>8) & 0x03;
    VDP[34]=SY & 0xFF;
    VDP[35]=(SY>>8) & 0x03;
    VDP[38]=DY & 0xFF;
    VDP[39]=(DY>>8) & 0x03;
  }
  else {
    m_mmc.SY=SY;
    m_mmc.DY=DY;
    m_mmc.NY=NY;
    m_mmc.ANX=ANX;
    m_mmc.ASX=ASX;
    m_mmc.ADX=ADX;
  }
}

/** YmmmEngine() *********************************************/
/** Vram -> Vram                                            **/
/*************************************************************/
void v99x8_device::YmmmEngine()
{
  int SY=m_mmc.SY;
  int DX=m_mmc.DX;
  int DY=m_mmc.DY;
  int TX=m_mmc.TX;
  int TY=m_mmc.TY;
  int NY=m_mmc.NY;
  int ADX=m_mmc.ADX;
  int MXD = m_mmc.MXD;
  int cnt;
  int delta;

  delta = GetVdpTimingValue(ymmm_timing);
  cnt = m_vdp_ops_count;

  switch (ScrMode) {
    default:
    case 5: pre_loop *VDP_VRMP5(MXD, ADX, DY) = *VDP_VRMP5(MXD, ADX, SY); post__xyy(256)
            break;
    case 6: pre_loop *VDP_VRMP6(MXD, ADX, DY) = *VDP_VRMP6(MXD, ADX, SY); post__xyy(512)
            break;
    case 7: pre_loop *VDP_VRMP7(MXD, ADX, DY) = *VDP_VRMP7(MXD, ADX, SY); post__xyy(512)
            break;
    case 8: pre_loop *VDP_VRMP8(MXD, ADX, DY) = *VDP_VRMP8(MXD, ADX, SY); post__xyy(256)
            break;
  }

  if ((m_vdp_ops_count=cnt)>0) {
    // Command execution done
    VDPStatus[2]&=0xFE;
    m_vdp_engine=0;
    if (!NY) {
      SY+=TY;
      DY+=TY;
    }
    else
      if (SY==-1)
        DY+=TY;
    VDP[42]=NY & 0xFF;
    VDP[43]=(NY>>8) & 0x03;
    VDP[34]=SY & 0xFF;
    VDP[35]=(SY>>8) & 0x03;
    VDP[38]=DY & 0xFF;
    VDP[39]=(DY>>8) & 0x03;
  }
  else {
    m_mmc.SY=SY;
    m_mmc.DY=DY;
    m_mmc.NY=NY;
    m_mmc.ADX=ADX;
  }
}

/** HmmcEngine() *********************************************/
/** CPU -> Vram                                             **/
/*************************************************************/
void v99x8_device::HmmcEngine()
{
  if ((VDPStatus[2]&0x80)!=0x80) {

    *VDP_VRMP(((ScrMode >= 5) && (ScrMode <= 8)) ? (ScrMode-5) : 0, m_mmc.MXD, m_mmc.ADX, m_mmc.DY)=VDP[44];
    m_vdp_ops_count-=GetVdpTimingValue(hmmv_timing);
    VDPStatus[2]|=0x80;

    if (!--m_mmc.ANX || ((m_mmc.ADX+=m_mmc.TX)&m_mmc.MX)) {
      if (!(--m_mmc.NY&1023) || (m_mmc.DY+=m_mmc.TY)==-1) {
        VDPStatus[2]&=0xFE;
        m_vdp_engine=0;
        if (!m_mmc.NY)
          m_mmc.DY+=m_mmc.TY;
        VDP[42]=m_mmc.NY & 0xFF;
        VDP[43]=(m_mmc.NY>>8) & 0x03;
        VDP[38]=m_mmc.DY & 0xFF;
        VDP[39]=(m_mmc.DY>>8) & 0x03;
      }
      else {
        m_mmc.ADX=m_mmc.DX;
        m_mmc.ANX=m_mmc.NX;
      }
    }
  }
}

/** VDPWrite() ***********************************************/
/** Use this function to transfer pixel(s) from CPU to m_ **/
/*************************************************************/
void v99x8_device::cpu_to_vdp (UINT8 V)
{
  VDPStatus[2]&=0x7F;
  VDPStatus[7]=VDP[44]=V;
  if(m_vdp_engine&&(m_vdp_ops_count>0)) (this->*m_vdp_engine)();
}

/** VDPRead() ************************************************/
/** Use this function to transfer pixel(s) from VDP to CPU. **/
/*************************************************************/
UINT8 v99x8_device::vdp_to_cpu ()
{
  VDPStatus[2]&=0x7F;
  if(m_vdp_engine&&(m_vdp_ops_count>0)) (this->*m_vdp_engine)();
  return(VDP[44]);
}

/** ReportVdpCommand() ***************************************/
/** Report VDP Command to be executed                       **/
/*************************************************************/
void v99x8_device::ReportVdpCommand(UINT8 Op)
{
	static const char *const Ops[16] =
	{
		"SET ","AND ","OR  ","XOR ","NOT ","NOP ","NOP ","NOP ",
		"TSET","TAND","TOR ","TXOR","TNOT","NOP ","NOP ","NOP "
	};
	static const char *const Commands[16] =
	{
		" ABRT"," ????"," ????"," ????","POINT"," PSET"," SRCH"," LINE",
		" LMMV"," LMMM"," LMCM"," LMMC"," HMMV"," HMMM"," YMMM"," HMMC"
	};

	UINT8 CL, CM, LO;
	int SX,SY, DX,DY, NX,NY;

	// Fetch arguments
	CL = VDP[44];
	SX = (VDP[32]+((int)VDP[33]<<8)) & 511;
	SY = (VDP[34]+((int)VDP[35]<<8)) & 1023;
	DX = (VDP[36]+((int)VDP[37]<<8)) & 511;
	DY = (VDP[38]+((int)VDP[39]<<8)) & 1023;
	NX = (VDP[40]+((int)VDP[41]<<8)) & 1023;
	NY = (VDP[42]+((int)VDP[43]<<8)) & 1023;
	CM = Op>>4;
	LO = Op&0x0F;

	LOG(("V9938: Opcode %02Xh %s-%s (%d,%d)->(%d,%d),%d [%d,%d]%s\n",
			Op, Commands[CM], Ops[LO],
			SX,SY, DX,DY, CL, VDP[45]&0x04? -NX:NX,
			VDP[45]&0x08? -NY:NY,
			VDP[45]&0x70? " on ExtVRAM":""
		));
}

/** VDPDraw() ************************************************/
/** Perform a given V9938 operation Op.                     **/
/*************************************************************/
UINT8 v99x8_device::command_unit_w (UINT8 Op)
{
  int SM;

  // V9938 ops only work in SCREENs 5-8
  if (ScrMode<5)
    return(0);

  SM = ScrMode-5;         // Screen mode index 0..3

  m_mmc.CM = Op>>4;
  if ((m_mmc.CM & 0x0C) != 0x0C && m_mmc.CM != 0)
    // Dot operation: use only relevant bits of color
    VDPStatus[7]=(VDP[44]&=Mask[SM]);

//  if(Verbose&0x02)
    ReportVdpCommand(Op);

  switch(Op>>4) {
    case CM_ABRT:
      VDPStatus[2]&=0xFE;
      m_vdp_engine=0;
      return 1;
    case CM_POINT:
      VDPStatus[2]&=0xFE;
      m_vdp_engine=0;
      VDPStatus[7]=VDP[44]=
                   VDP_POINT(SM, (VDP[45] & 0x10) != 0,
                                 VDP[32]+((int)VDP[33]<<8),
                                 VDP[34]+((int)VDP[35]<<8));
      return 1;
    case CM_PSET:
      VDPStatus[2]&=0xFE;
      m_vdp_engine=0;
      VDP_PSET(SM, (VDP[45] & 0x20) != 0,
               VDP[36]+((int)VDP[37]<<8),
               VDP[38]+((int)VDP[39]<<8),
               VDP[44],
               Op&0x0F);
      return 1;
    case CM_SRCH:
      m_vdp_engine=&v99x8_device::SrchEngine;
      break;
    case CM_LINE:
      m_vdp_engine=&v99x8_device::LineEngine;
      break;
    case CM_LMMV:
      m_vdp_engine=&v99x8_device::LmmvEngine;
      break;
    case CM_LMMM:
      m_vdp_engine=&v99x8_device::LmmmEngine;
      break;
    case CM_LMCM:
      m_vdp_engine=&v99x8_device::LmcmEngine;
      break;
    case CM_LMMC:
      m_vdp_engine=&v99x8_device::LmmcEngine;
      break;
    case CM_HMMV:
      m_vdp_engine=&v99x8_device::HmmvEngine;
      break;
    case CM_HMMM:
      m_vdp_engine=&v99x8_device::HmmmEngine;
      break;
    case CM_YMMM:
      m_vdp_engine=&v99x8_device::YmmmEngine;
      break;
    case CM_HMMC:
      m_vdp_engine=&v99x8_device::HmmcEngine;
      break;
    default:
      LOG(("V9938: Unrecognized opcode %02Xh\n",Op));
        return(0);
  }

  // Fetch unconditional arguments
  m_mmc.SX = (VDP[32]+((int)VDP[33]<<8)) & 511;
  m_mmc.SY = (VDP[34]+((int)VDP[35]<<8)) & 1023;
  m_mmc.DX = (VDP[36]+((int)VDP[37]<<8)) & 511;
  m_mmc.DY = (VDP[38]+((int)VDP[39]<<8)) & 1023;
  m_mmc.NY = (VDP[42]+((int)VDP[43]<<8)) & 1023;
  m_mmc.TY = VDP[45]&0x08? -1:1;
  m_mmc.MX = PPL[SM];
  m_mmc.CL = VDP[44];
  m_mmc.LO = Op&0x0F;
  m_mmc.MXS = (VDP[45] & 0x10) != 0;
  m_mmc.MXD = (VDP[45] & 0x20) != 0;

  // Argument depends on UINT8 or dot operation
  if ((m_mmc.CM & 0x0C) == 0x0C) {
    m_mmc.TX = VDP[45]&0x04? -PPB[SM]:PPB[SM];
    m_mmc.NX = ((VDP[40]+((int)VDP[41]<<8)) & 1023)/PPB[SM];
  }
  else {
    m_mmc.TX = VDP[45]&0x04? -1:1;
    m_mmc.NX = (VDP[40]+((int)VDP[41]<<8)) & 1023;
  }

  // X loop variables are treated specially for LINE command
  if (m_mmc.CM == CM_LINE) {
    m_mmc.ASX=((m_mmc.NX-1)>>1);
    m_mmc.ADX=0;
  }
  else {
    m_mmc.ASX = m_mmc.SX;
    m_mmc.ADX = m_mmc.DX;
  }

  // NX loop variable is treated specially for SRCH command
  if (m_mmc.CM == CM_SRCH)
    m_mmc.ANX=(VDP[45]&0x02)!=0; // Do we look for "==" or "!="?
  else
    m_mmc.ANX = m_mmc.NX;

  // Command execution started
  VDPStatus[2]|=0x01;

  // Start execution if we still have time slices
  if(m_vdp_engine&&(m_vdp_ops_count>0)) (this->*m_vdp_engine)();

  // Operation successfull initiated
  return(1);
}

/** LoopVDP() ************************************************/
/** Run X steps of active VDP command                       **/
/*************************************************************/
void v99x8_device::update_command ()
{
  if(m_vdp_ops_count<=0)
  {
    m_vdp_ops_count+=13662;
    if(m_vdp_engine&&(m_vdp_ops_count>0)) (this->*m_vdp_engine)();
  }
  else
  {
    m_vdp_ops_count=13662;
    if(m_vdp_engine) (this->*m_vdp_engine)();
  }
}

