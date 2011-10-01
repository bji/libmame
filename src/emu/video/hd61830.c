/**********************************************************************

    HD61830 LCD Timing Controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "hd61830.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

enum
{
	INSTRUCTION_MODE_CONTROL = 0,
	INSTRUCTION_CHARACTER_PITCH,
	INSTRUCTION_NUMBER_OF_CHARACTERS,
	INSTRUCTION_NUMBER_OF_TIME_DIVISIONS,
	INSTRUCTION_CURSOR_POSITION,
	INSTRUCTION_DISPLAY_START_LOW = 8,
	INSTRUCTION_DISPLAY_START_HIGH,
	INSTRUCTION_CURSOR_ADDRESS_LOW,
	INSTRUCTION_CURSOR_ADDRESS_HIGH,
	INSTRUCTION_DISPLAY_DATA_WRITE,
	INSTRUCTION_DISPLAY_DATA_READ,
	INSTRUCTION_CLEAR_BIT,
	INSTRUCTION_SET_BIT
};

static const int CYCLES[] =
{
	4, 4, 4, 4, 4, -1, -1, -1, 4, 4, 4, 4, 6, 6, 36, 36
};

const int MODE_EXTERNAL_CG		= 0x01;
const int MODE_GRAPHIC			= 0x02;
const int MODE_CURSOR			= 0x04;
const int MODE_BLINK			= 0x08;
const int MODE_MASTER			= 0x10;
const int MODE_DISPLAY_ON		= 0x20;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type HD61830 = &device_creator<hd61830_device>;


// default address map
static ADDRESS_MAP_START( hd61830, AS_0, 8 )
	AM_RANGE(0x0000, 0xffff) AM_RAM
ADDRESS_MAP_END


// internal character generator ROM
ROM_START( hd61830 )
	ROM_REGION( 0x5c0, "hd61830", ROMREGION_LOADBYNAME ) // internal 7360-bit chargen ROM
	ROM_LOAD( "hd61830.bin", 0x000, 0x5c0, BAD_DUMP CRC(06a934da) SHA1(bf3f074db5dc92e6f530cb18d6c013563099a87d) ) // typed in from manual
ROM_END




//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT8 hd61830_device::readbyte(offs_t address)
{
	return space()->read_byte(address);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void hd61830_device::writebyte(offs_t address, UINT8 data)
{
	space()->write_byte(address, data);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hd61830_device - constructor
//-------------------------------------------------

hd61830_device::hd61830_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, HD61830, "Hitachi HD61830", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	  m_bf(false),
	  m_blink(0),
	  m_space_config("videoram", ENDIANNESS_LITTLE, 8, 16, 0, NULL, *ADDRESS_MAP_NAME(hd61830))
{
	m_shortname = "hd61830";
}


//-------------------------------------------------
//  device_rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *hd61830_device::device_rom_region() const
{
	return ROM_NAME(hd61830);
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void hd61830_device::device_config_complete()
{
	// inherit a copy of the static data
	const hd61830_interface *intf = reinterpret_cast<const hd61830_interface *>(static_config());
	if (intf != NULL)
		*static_cast<hd61830_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_rd_cb, 0, sizeof(m_in_rd_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd61830_device::device_start()
{
	// allocate timers
	m_busy_timer = timer_alloc();

	// resolve callbacks
    m_in_rd_func.resolve(m_in_rd_cb, *this);

	m_screen = machine().device<screen_device>(screen_tag);

	// register for state saving
	save_item(NAME(m_bf));
	save_item(NAME(m_ir));
	save_item(NAME(m_mcr));
	save_item(NAME(m_dor));
	save_item(NAME(m_cac));
	save_item(NAME(m_dsa));
	save_item(NAME(m_vp));
	save_item(NAME(m_hp));
	save_item(NAME(m_hn));
	save_item(NAME(m_nx));
	save_item(NAME(m_cp));
	save_item(NAME(m_blink));
	save_item(NAME(m_cursor));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hd61830_device::device_reset()
{
	// display off, slave mode
	m_mcr &= ~(MODE_MASTER | MODE_DISPLAY_ON);

	// default horizontal pitch
	m_hp = 6;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void hd61830_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	// clear busy flag
	m_bf = false;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *hd61830_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}


/*-------------------------------------------------
    set_busy_flag - set busy flag and arm timer
                    to clear it later
-------------------------------------------------*/

void hd61830_device::set_busy_flag()
{
	// set busy flag
	m_bf = true;

	// adjust busy timer
	m_busy_timer->adjust(attotime::from_usec(CYCLES[m_ir]));
}


//-------------------------------------------------
//  status_r - status register read
//-------------------------------------------------

READ8_MEMBER( hd61830_device::status_r )
{
	if (LOG) logerror("HD61830 '%s' Status Read: %s\n", tag(), m_bf ? "busy" : "ready");

	return m_bf ? 0x80 : 0;
}


//-------------------------------------------------
//  control_w - instruction register write
//-------------------------------------------------

WRITE8_MEMBER( hd61830_device::control_w )
{
	m_ir = data;
}


//-------------------------------------------------
//  data_r - data register read
//-------------------------------------------------

READ8_MEMBER( hd61830_device::data_r )
{
	UINT8 data = m_dor;

	if (LOG) logerror("HD61830 '%s' Display Data Read %02x\n", tag(), m_dor);

	m_dor = readbyte(m_cac);

	m_cac++;

	return data;
}


//-------------------------------------------------
//  data_w - data register write
//-------------------------------------------------

WRITE8_MEMBER( hd61830_device::data_w )
{
	if (m_bf)
	{
		logerror("HD61830 '%s' Ignoring data write %02x due to business\n", tag(), data);
		return;
	}

	switch (m_ir)
	{
	case INSTRUCTION_MODE_CONTROL:
		m_mcr = data;

		if (LOG)
		{
			logerror("HD61830 '%s' %s CG\n", tag(), (data & MODE_EXTERNAL_CG) ? "External" : "Internal");
			logerror("HD61830 '%s' %s Display Mode\n", tag(), (data & MODE_GRAPHIC) ? "Graphic" : "Character");
			logerror("HD61830 '%s' %s Mode\n", tag(), (data & MODE_MASTER) ? "Master" : "Slave");
			logerror("HD61830 '%s' Cursor %s\n", tag(), (data & MODE_CURSOR) ? "On" : "Off");
			logerror("HD61830 '%s' Blink %s\n", tag(), (data & MODE_BLINK) ? "On" : "Off");
			logerror("HD61830 '%s' Display %s\n", tag(), (data & MODE_DISPLAY_ON) ? "On" : "Off");
		}
		break;

	case INSTRUCTION_CHARACTER_PITCH:
		m_hp = (data & 0x07) + 1;
		m_vp = (data >> 4) + 1;

		if (LOG) logerror("HD61830 '%s' Horizontal Character Pitch: %u\n", tag(), m_hp);
		if (LOG) logerror("HD61830 '%s' Vertical Character Pitch: %u\n", tag(), m_vp);
		break;

	case INSTRUCTION_NUMBER_OF_CHARACTERS:
		m_hn = (data & 0x7f) + 1;

		if (LOG) logerror("HD61830 '%s' Number of Characters: %u\n", tag(), m_hn);
		break;

	case INSTRUCTION_NUMBER_OF_TIME_DIVISIONS:
		m_nx = (data & 0x7f) + 1;

		if (LOG) logerror("HD61830 '%s' Number of Time Divisions: %u\n", tag(), m_nx);
		break;

	case INSTRUCTION_CURSOR_POSITION:
		m_cp = (data & 0x7f) + 1;

		if (LOG) logerror("HD61830 '%s' Cursor Position: %u\n", tag(), m_cp);
		break;

	case INSTRUCTION_DISPLAY_START_LOW:
		m_dsa = (m_dsa & 0xff00) | data;

		if (LOG) logerror("HD61830 '%s' Display Start Address Low %04x\n", tag(), m_dsa);
		break;

	case INSTRUCTION_DISPLAY_START_HIGH:
		m_dsa = (data << 8) | (m_dsa & 0xff);

		if (LOG) logerror("HD61830 '%s' Display Start Address High %04x\n", tag(), m_dsa);
		break;

	case INSTRUCTION_CURSOR_ADDRESS_LOW:
		if (BIT(m_cac, 7) && !BIT(data, 7))
		{
			m_cac = (((m_cac >> 8) + 1) << 8) | data;
		}
		else
		{
			m_cac = (m_cac & 0xff00) | data;
		}

		if (LOG) logerror("HD61830 '%s' Cursor Address Low %02x: %04x\n", tag(), data, m_cac);
		break;

	case INSTRUCTION_CURSOR_ADDRESS_HIGH:
		m_cac = (data << 8) | (m_cac & 0xff);

		if (LOG) logerror("HD61830 '%s' Cursor Address High %02x: %04x\n", tag(), data, m_cac);
		break;

	case INSTRUCTION_DISPLAY_DATA_WRITE:
		writebyte(m_cac, data);

		if (LOG) logerror("HD61830 '%s' Display Data Write %02x -> %04x row %u col %u\n", tag(), data, m_cac, m_cac / 40, m_cac % 40);

		m_cac++;
		break;

	case INSTRUCTION_CLEAR_BIT:
		{
		int bit = data & 0x07;
		UINT8 md = readbyte(m_cac);

		md &= ~(1 << bit);

		if (LOG) logerror("HD61830 '%s' Clear Bit %u at %04x\n", tag(), bit + 1, m_cac);

		writebyte(m_cac, md);

		m_cac++;
		}
		break;

	case INSTRUCTION_SET_BIT:
		{
		int bit = data & 0x07;
		UINT8 md = readbyte(m_cac);

		md |= 1 << bit;

		if (LOG) logerror("HD61830 '%s' Set Bit %u at %04x\n", tag(), bit + 1, m_cac);

		writebyte(m_cac, md);

		m_cac++;
		}
		break;

	default:
		logerror("HD61830 '%s' Illegal Instruction %02x!\n", tag(), m_ir);
		return;
	}

	// burn cycles
	set_busy_flag();
}


//-------------------------------------------------
//  draw_scanline - draw one graphics scanline
//-------------------------------------------------

void hd61830_device::draw_scanline(bitmap_t *bitmap, const rectangle *cliprect, int y, UINT16 ra)
{
	for (int sx = 0; sx < m_hn; sx++)
	{
		UINT8 data = readbyte(ra++);

		for (int x = 0; x < m_hp; x++)
		{
			*BITMAP_ADDR16(bitmap, y, (sx * m_hp) + x) = BIT(data, x);
		}
	}
}


//-------------------------------------------------
//  update_graphics - draw graphics mode screen
//-------------------------------------------------

void hd61830_device::update_graphics(bitmap_t *bitmap, const rectangle *cliprect)
{
	for (int y = 0; y < m_nx; y++)
	{
		UINT16 rac1 = m_dsa + (y * m_hn);
		UINT16 rac2 = rac1 + (m_nx * m_hn);

		/* draw upper half scanline */
		draw_scanline(bitmap, cliprect, y, rac1);

		/* draw lower half scanline */
		draw_scanline(bitmap, cliprect, y + m_nx, rac2);
	}
}


//-------------------------------------------------
//  draw_char - draw a char
//-------------------------------------------------

void hd61830_device::draw_char(bitmap_t *bitmap, const rectangle *cliprect, UINT16 ma, int x, int y, UINT8 md)
{
	for (int cl = 0; cl < m_vp; cl++)
	{
		for (int cr = 0; cr < m_hp; cr++)
		{
			int sy = y * m_vp + cl;
			int sx = x * m_hp + cr;
			UINT8 data = 0;

			if (m_mcr & MODE_EXTERNAL_CG)
			{
				data = m_in_rd_func((cl << 12) | md);
			}
			else
			{
				UINT16 addr = 0;

				if (md >= 0x20 && md < 0x80 && cl < 7)
				{
					// 5x7 characters 0x20..0x7f
					addr = (md - 0x20) * 7 + cl;
				}
				else if (md >= 0xa0 && md < 0xe0 && cl < 7)
				{
					// 5x7 characters 0xa0..0xdf
					addr = 96*7 + (md - 0xa0) * 7 + cl;
				}
				else if (md >= 0xe0 && cl < 11)
				{
					// 5x11 characters 0xe0..0xff
					addr = 160*7 + (md - 0xe0) * 11 + cl;
				}

				data = subregion("hd61830")->u8(addr);
			}

			int cursor = m_mcr & MODE_CURSOR;
			int blink = m_mcr & MODE_BLINK;

			// cursor off
			int pixel = BIT(data, cr);

			if (blink && (ma == m_cac))
			{
				// cursor off, character blink
				if (!cursor)
					pixel = m_cursor ? pixel : 0;

				// cursor blink
				if (cursor && (cl == m_cp))
					pixel = m_cursor ? 1 : 0;
			}
			else
			{
				// cursor on
				if (cursor && (cl == m_cp))
					pixel = m_cursor ? 1 : 0;
			}

			if (sy < m_screen->height() && sx < m_screen->width())
				*BITMAP_ADDR16(bitmap, sy, sx) = pixel;
		}
	}
}


//-------------------------------------------------
//  update_text - draw text mode screen
//-------------------------------------------------

void hd61830_device::update_text(bitmap_t *bitmap, const rectangle *cliprect)
{
	for (int y = 0; y < (m_nx / m_vp); y++)
	{
		for (int x = 0; x < m_hn; x++)
		{
			UINT16 ma = y * m_hn + x;
			UINT8 md = readbyte(ma);

			draw_char(bitmap, cliprect, ma, x, y, md);
		}
	}
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

void hd61830_device::update_screen(bitmap_t *bitmap, const rectangle *cliprect)
{
	if (m_mcr & MODE_DISPLAY_ON)
	{
		if (m_mcr & MODE_GRAPHIC)
		{
			update_graphics(bitmap, cliprect);
		}
		else
		{
			update_text(bitmap, cliprect);
		}
	}
	else
	{
		bitmap_fill(bitmap, cliprect, 0);
	}

	m_blink++;

	if (m_blink == 0x20)
	{
		m_blink = 0;
		m_cursor = !m_cursor;
	}
}
