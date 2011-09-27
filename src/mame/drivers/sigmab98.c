/*************************************************************************************************************

                                     -= Sigma B-98 Hardware / Sammy Medal Games =-

                                                 driver by Luca Elia

CPU     :   TAXAN KY-80 (Yamaha)
Custom  :   TAXAN KY-3211
Sound   :   YMZ280B
NVRAM   :   93C46, Battery

Graphics are made of sprites only.
Each sprite is composed of X x Y tiles and can be zoomed / shrunk.
Tiles can be 16x16x4 or 16x16x8.

----------------------------------------------------------------

Sammy Kids Medal Series

CPU     :   KL5C80A120FP (Z80 Compatible High Speed Microcontroller)
Custom  :   TAXAN KY-3211 ?
Sound   :   OKI M9810B
NVRAM   :   93C46, Battery

Cartridge based system. Carts contain just some 16Mb flash eeproms.

Info from Tatsuya Fujita:

According to some news articles for the AOU show 2002 and 2003 the correct system name
is "Treasure Fall" (despite the cart label is "Treasure Hall").

Dumped games:

Animal Catch
Itazura Monkey
Taihou de Doboon
Pye-nage Taikai
Hae Hae Ka Ka Ka

Games with the same cabinet which might be on the same hardware:

1999 Shatekids
1999 Otakara Locomo
1999 Dokidoki Kingyosukui
2000 Taihou de Doboon
2000 Pye-nage Taikai
2000 Animal Catch
2000 Itazura Monkey
2000 Otoshicha Ottotto
2000 Go Go Cowboy
2001 Mushitori Meijin
2001 Morino Dodgeball Senshuken
2001 Waiwai Wanage
2001 Hae Hae Ka Ka Ka
2001 Zarigani Tsuri
2001 Kotekitai Slot
2002 Shateki Yokochou
2002 Ipponzuri Slot
2002 Karateman
2002 One-touchable
2003 Gun Kids
2003 Kurukuru Train
2003 Zakuzaku Kaizokudan
2004 Animal Punch
2004 Dotabata Zaurus
2004 Ninchuu Densetsu

from:
http://www.tsc-acnet.com/index.php?sort=8&action=cataloglist&s=1&mode=3&genre_id=40&freeword=%25A5%25B5%25A5%25DF%25A1%25BC

To Do:

- KL5C80 emulation is needed to consolidate the sammymdl games in one memory map and to run the BIOS
- Remove ROM patches from gegege
- gegege checks the EEPROM output after reset, and wants a timed 0->1 transition or locks up while
  saving setting in service mode. Using a reset_delay of 7 works, unless when "play style" is set
  to "coin" (it probably changes the number of reads from port $C0).
  I guess the reset_delay mechanism should be implemented with a timer in eeprom.c.
- animalc needs a green backgound during part of the intro (floating animals in and out of the screen).
  This can be achieved using either pen 0 or ff as background color, but messes up the other games.
- pyenaget intro: when the theater scrolls out to the left, the train should scroll in from the right,
  with no visible gaps. It currently leaves the screen empty instead, for several seconds.

Notes:

- "BACKUP RAM NG" error: in test mode, choose "SET MODE" -> "RAM CLEAR" and keep the button pressed for long.

*************************************************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "sound/okim9810.h"
#include "sound/ymz280b.h"
#include "machine/eeprom.h"
#include "machine/nvram.h"
#include "machine/ticket.h"


class sigmab98_state : public driver_device
{
public:
	sigmab98_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/***************************************************************************

    Video

***************************************************************************/

/***************************************************************************

    Sprites

    Offset:     Bits:         Value:

    0           7654 ----
                ---- 3210     Color
    1           7--- ----
                -6-- ----     256 Color Sprite
                --5- ----
                ---4 ----     Flip X
                ---- 3---     Flip Y
                ---- -2--     Draw Sprite
                ---- --10     Priority (0 = Front .. 3 = Back)
    2                         Tile Code (High)
    3                         Tile Code (Low)
    4           7654 3---     Number of X Tiles - 1
                ---- -210     X (High)
    5                         X (Low)
    6           7654 3---     Number of Y Tiles - 1
                ---- -210     Y (High)
    7                         Y (Low)
    8                         Shrink Factor (<< 8, High)
    9                         Shrink Factor (<< 8, Low)
    a
    b
    c           7654 3---
                ---- -210     Delta X (High)
    d                         Delta X (Low)
    e           7654 3---
                ---- -210     Delta Y (High)
    f                         Delta Y (Low)

***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int pri_mask)
{
	sigmab98_state *state = machine.driver_data<sigmab98_state>();
	UINT8 *end		=	state->m_spriteram - 0x10;
	UINT8 *s		=	end + state->m_spriteram_size;

	for ( ; s != end; s -= 0x10 )
	{
		int gfx, code, color, zoom, dim, scale;
		int sx, nx, x, x0, x1, dx, flipx;
		int sy, ny, y, y0, y1, dy, flipy;

		if ( (s[ 0x01 ] & 0x04) == 0)
			continue;

		if ( ((1 << (s[ 0x01 ] & 0x03)) & pri_mask) == 0 )
			continue;

		color	=	s[ 0x00 ] & 0xf;

		gfx		=	(s[ 0x01 ] & 0x40 ) ? 1 : 0;

		code	=	s[ 0x02 ] * 256 + s[ 0x03 ];

		nx		=	((s[ 0x04 ] & 0xf8) >> 3) + 1;

		sx		=	(s[ 0x04 ] & 0x03) * 256 + s[ 0x05 ];

		ny		=	((s[ 0x06 ] & 0xf8) >> 3) + 1;

		sy		=	(s[ 0x06 ] & 0x03) * 256 + s[ 0x07 ];

		zoom	=	(s[ 0x08 ] & 0xff) * 256 + s[ 0x09 ];

		dx		=	(s[ 0x0c ] & 0x03) * 256 + s[ 0x0d ];
		dy		=	(s[ 0x0e ] & 0x03) * 256 + s[ 0x0f ];

		// Sign extend the position
		sx		=	(sx & 0x1ff) - (sx & 0x200);
		sy		=	(sy & 0x1ff) - (sy & 0x200);
		dx		=	(dx & 0x1ff) - (dx & 0x200);
		dy		=	(dy & 0x1ff) - (dy & 0x200);

		// Add shift (negated, as it seems more correct in haekaka)
		sx		-=	dx;
		sy		-=	dy;

		// Use fixed point values (16.16), for accuracy
		sx		<<=	16;
		sy		<<=	16;

		zoom	=	(1 << 16) / (zoom ? zoom : 1);
		dim		=	(0x10 << 8) * zoom;
		scale	=	dim / 0x10;

		// Let's approximate to the nearest greater integer value
        // to avoid holes in between tiles
		if (scale & 0xffff)	scale += (1<<16) / 0x10;

		flipx	=	s[ 0x01 ] & 0x10;
		flipy	=	s[ 0x01 ] & 0x08;

		if ( flipx )	{	x0 = nx - 1;	x1 = -1;	dx = -1;	}
		else			{	x0 = 0;			x1 = nx;	dx = +1;	}

		if ( flipy )	{	y0 = ny - 1;	y1 = -1;	dy = -1;	}
		else			{	y0 = 0;			y1 = ny;	dy = +1;	}

		for (y = y0; y != y1; y += dy)
		{
			for (x = x0; x != x1; x += dx)
			{
				drawgfxzoom_transpen(	bitmap,	cliprect, machine.gfx[gfx],
										code++, color,
										flipx, flipy,
										(sx + x * dim) / 0x10000, (sy + y * dim) / 0x10000,
										scale, scale, 0	);
			}
		}
	}
}

static SCREEN_UPDATE(sigmab98)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (input_code_pressed(screen->machine(), KEYCODE_Z))
	{
		int msk = 0;
		if (input_code_pressed(screen->machine(), KEYCODE_Q))	msk |= 1;
		if (input_code_pressed(screen->machine(), KEYCODE_W))	msk |= 2;
		if (input_code_pressed(screen->machine(), KEYCODE_E))	msk |= 4;
		if (input_code_pressed(screen->machine(), KEYCODE_R))	msk |= 8;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));

	// Draw from priority 3 (bottom, converted to a bitmask) to priority 0 (top)
	draw_sprites(screen->machine(), bitmap, cliprect, layers_ctrl & 8);
	draw_sprites(screen->machine(), bitmap, cliprect, layers_ctrl & 4);
	draw_sprites(screen->machine(), bitmap, cliprect, layers_ctrl & 2);
	draw_sprites(screen->machine(), bitmap, cliprect, layers_ctrl & 1);

	return 0;
}


/***************************************************************************

    Memory Maps

***************************************************************************/

static UINT8 reg, rombank;
static UINT8 reg2, rambank;

/***************************************************************************
                        GeGeGe no Kitarou Youkai Slot
***************************************************************************/

// rombank
static WRITE8_HANDLER( regs_w )
{
	if (offset == 0)
	{
		reg = data;
		return;
	}

	switch ( reg )
	{
		case 0x1f:
			rombank = data;
			if (data >= 0x18)
				logerror("%s: unknown rom bank = %02x\n", space->machine().describe_context(), data);
			else
				memory_set_bank(space->machine(), "rombank", data);
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", space->machine().describe_context(), reg, data);
	}
}
static READ8_HANDLER( regs_r )
{
	if (offset == 0)
		return reg;

	switch ( reg )
	{
		case 0x1f:
			return rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", space->machine().describe_context(), reg);
			return 0x00;
	}
}

// rambank
static WRITE8_HANDLER( regs2_w )
{
	if (offset == 0)
	{
		reg2 = data;
		return;
	}

	switch ( reg2 )
	{
		case 0xb5:
			rambank = data;
			switch (data)
			{
				case 0x32:
					memory_set_bank(space->machine(), "rambank", 0);
					break;
				case 0x36:
					memory_set_bank(space->machine(), "rambank", 1);
					break;
				default:
					logerror("%s: unknown ram bank = %02x\n", space->machine().describe_context(), data);
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", space->machine().describe_context(), reg2, data);
	}
}
static READ8_HANDLER( regs2_r )
{
	if (offset == 0)
		return reg2;

	switch ( reg2 )
	{
		case 0xb5:
			return rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", space->machine().describe_context(), reg2);
			return 0x00;
	}
}


// Outputs

static UINT8 c0,c4,c6,c8;
static void show_outputs()
{
#ifdef MAME_DEBUG
//  popmessage("0: %02X  4: %02X  6: %02X  8: %02X",c0,c4,c6,c8);
#endif
}

// Port c0
static WRITE8_DEVICE_HANDLER( eeprom_w )
{
	// latch the bit
	eeprom_write_bit(device, data & 0x40);

	// reset line asserted: reset.
//  if ((c0 ^ data) & 0x20)
		eeprom_set_cs_line(device, (data & 0x20) ? CLEAR_LINE : ASSERT_LINE);

	// clock line asserted: write latch or select next bit to read
	eeprom_set_clock_line(device, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	c0 = data;
	show_outputs();
}

// Port c4
// 10 led?
static WRITE8_HANDLER( c4_w )
{
	set_led_status(space->machine(), 0, (data & 0x10));

	c4 = data;
	show_outputs();
}

// Port c6
// 03 lockout (active low, 02 is cleared when reaching 99 credits)
// 04 pulsed on coin in
// 08 always blinks
// 10 led?
// 20 blinks after coin up
static WRITE8_HANDLER( c6_w )
{
	coin_lockout_w(space->machine(), 0, (~data) & 0x02);

	coin_counter_w(space->machine(), 0,   data  & 0x04);

	set_led_status(space->machine(), 1,   data  & 0x08);
	set_led_status(space->machine(), 2,   data  & 0x10);
	set_led_status(space->machine(), 3,   data  & 0x20);	//

	c6 = data;
	show_outputs();
}

// Port c8
// 01 hopper enable?
// 02 hopper motor on (active low)?
static WRITE8_HANDLER( c8_w )
{
	ticket_dispenser_w(space->machine().device("hopper"), 0, (!(data & 0x02) && (data & 0x01)) ? 0x00 : 0x80);

	c8 = data;
	show_outputs();
}

static ADDRESS_MAP_START( gegege_mem_map, AS_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x7fff ) AM_ROM
	AM_RANGE( 0x8000, 0x9fff ) AM_ROMBANK("rombank")

	AM_RANGE( 0xa000, 0xafff ) AM_RAM AM_BASE_SIZE_MEMBER(sigmab98_state, m_spriteram, m_spriteram_size)

	AM_RANGE( 0xc000, 0xc1ff ) AM_RAM_WRITE(paletteram_xRRRRRGGGGGBBBBB_be_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE( 0xc800, 0xc87f ) AM_RAM

//  AM_RANGE( 0xd001, 0xd021 ) AM_RAM
	AM_RANGE( 0xd800, 0xdfff ) AM_RAMBANK("rambank")

	AM_RANGE( 0xe000, 0xefff ) AM_RAM AM_SHARE("nvram")	// battery

	AM_RANGE( 0xf000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( gegege_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)

	AM_RANGE( 0x00, 0x01 ) AM_DEVWRITE( "ymz", ymz280b_w )

	AM_RANGE( 0xa0, 0xa1 ) AM_READWRITE( regs_r,  regs_w )
//  AM_RANGE( 0xa2, 0xa3 )
	AM_RANGE( 0xa4, 0xa5 ) AM_READWRITE( regs2_r, regs2_w )

	AM_RANGE( 0xc0, 0xc0 ) AM_READ_PORT( "EEPROM" )
	AM_RANGE( 0xc0, 0xc0 ) AM_DEVWRITE("eeprom", eeprom_w)

	AM_RANGE( 0xc2, 0xc2 ) AM_READ_PORT( "IN1" )

	AM_RANGE( 0xc4, 0xc4 ) AM_READ_PORT( "IN2" )
	AM_RANGE( 0xc4, 0xc4 ) AM_WRITE( c4_w )

	AM_RANGE( 0xc6, 0xc6 ) AM_WRITE( c6_w )

	AM_RANGE( 0xc8, 0xc8 ) AM_WRITE( c8_w )

	AM_RANGE( 0xe5, 0xe5 ) AM_READNOP	// during irq
ADDRESS_MAP_END


/***************************************************************************
                                 Animal Catch
***************************************************************************/

// rombank
static WRITE8_HANDLER( animalc_rombank_w )
{
	if (offset == 0)
	{
		reg = data;
		return;
	}

	UINT8 *rom = space->machine().region("maincpu")->base();
	switch ( reg )
	{
		case 0x0f:
			rombank = data;
			switch (data)
			{
				case 0x10:	memory_set_bankptr(space->machine(), "rombank", rom + 0x400 + 0x4000);	break;
				case 0x14:	memory_set_bankptr(space->machine(), "rombank", rom + 0x400 + 0x8000);	break;
				case 0x18:	memory_set_bankptr(space->machine(), "rombank", rom + 0x400 + 0xc000);	break;
				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", space->machine().describe_context(), data, reg);
			}
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", space->machine().describe_context(), reg, data);
	}
}
static READ8_HANDLER( animalc_rombank_r )
{
	if (offset == 0)
		return reg;

	switch ( reg )
	{
		case 0x0f:
			return rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", space->machine().describe_context(), reg);
			return 0x00;
	}
}

// rambank
static WRITE8_HANDLER( animalc_rambank_w )
{
	if (offset == 0)
	{
		reg2 = data;
		return;
	}

	int bank = 0;
	switch ( reg2 )
	{
		case 0x1f:
			rambank = data;
			switch (data)
			{
				case 0x58:	bank = 0;	break;
				case 0x62:	bank = 1;	break;
				case 0x63:	bank = 2;	break;
				case 0x64:	bank = 3;	break;
				case 0x65:	bank = 4;	break;
				default:
					logerror("%s: unknown ram bank = %02x, reg2 = %02x\n", space->machine().describe_context(), data, reg2);
					return;
			}
			memory_set_bank(space->machine(), "rambank", bank);
			if ( (bank == 1) || (bank == 2) || (bank == 3) )
				memory_set_bank(space->machine(), "sprbank", bank-1);
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", space->machine().describe_context(), reg2, data);
	}
}
static READ8_HANDLER( animalc_rambank_r )
{
	if (offset == 0)
		return reg2;

	switch ( reg2 )
	{
		case 0x1f:
			return rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", space->machine().describe_context(), reg2);
			return 0x00;
	}
}


static READ8_DEVICE_HANDLER( sammymdl_eeprom_r )
{
	return eeprom_read_bit(device) ? 0x80 : 0;
}

static WRITE8_DEVICE_HANDLER( sammymdl_eeprom_w )
{
	// latch the bit
	eeprom_write_bit(device, data & 0x40);

	// reset line asserted: reset.
	eeprom_set_cs_line(device, (data & 0x20) ? CLEAR_LINE : ASSERT_LINE);

	// clock line asserted: write latch or select next bit to read
	eeprom_set_clock_line(device, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	if (data & 0x8f)
		logerror("%s: unknown eeeprom bits written %02x\n", device->machine().describe_context(), data);
}

static READ8_HANDLER( unk_34_r )
{
	// mask 0x01?
	return 0x01;
}

static UINT8 vblank;

static READ8_HANDLER( vblank_r )
{
	// mask 0x04 must be set before writing sprite list
	// mask 0x10 must be set or irq/00 hangs?
	return	vblank | 0x14;
}

static WRITE8_HANDLER( vblank_w )
{
	vblank = (vblank & ~0x03) | (data & 0x03);
}

static SCREEN_EOF( sammymdl )
{
	vblank &= ~0x01;
}

static UINT8 out[3];
static void show_3_outputs()
{
#ifdef MAME_DEBUG
//  popmessage("COIN: %02X  LED: %02X  HOP: %02X", out[0], out[1], out[2]);
#endif
}
// Port 31
static WRITE8_HANDLER( sammymdl_coin_w )
{
	coin_counter_w(space->machine(), 0,   data  & 0x01 );	// coin1 in
	coin_counter_w(space->machine(), 1,   data  & 0x02 );	// coin2 in
	coin_counter_w(space->machine(), 2,   data  & 0x04 );	// medal in

//  coin_lockout_w(space->machine(), 1, (~data) & 0x08 ); // coin2 lockout?
//  coin_lockout_w(space->machine(), 0, (~data) & 0x10 ); // coin1 lockout
//  coin_lockout_w(space->machine(), 2, (~data) & 0x20 ); // medal lockout?

	out[0] = data;
	show_3_outputs();
}

// Port 32
static WRITE8_HANDLER( sammymdl_leds_w )
{
	set_led_status(space->machine(), 0,	data & 0x01);	// button

	out[1] = data;
	show_3_outputs();
}

// Port b0
// 02 hopper enable?
// 01 hopper motor on (active low)?
static WRITE8_HANDLER( sammymdl_hopper_w )
{
	ticket_dispenser_w(space->machine().device("hopper"), 0, (!(data & 0x01) && (data & 0x02)) ? 0x00 : 0x80);

	out[2] = data;
	show_3_outputs();
}

static READ8_HANDLER( sammymdl_coin_hopper_r )
{
	UINT8 ret = input_port_read(space->machine(), "COIN");

//  if ( !ticket_dispenser_r(space->machine().device("hopper"), 0) )
//      ret &= ~0x01;

	return ret;
}

static UINT8 *nvram;
static ADDRESS_MAP_START( animalc_map, AS_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x3fff ) AM_ROM
	AM_RANGE( 0x4000, 0x7fff ) AM_ROMBANK( "rombank" )
	AM_RANGE( 0x8000, 0x8fff ) AM_RAMBANK( "rambank" ) AM_SHARE( "nvram" ) AM_BASE( &nvram )

	AM_RANGE( 0x9000, 0x9fff ) AM_RAM
	AM_RANGE( 0xa000, 0xafff ) AM_RAM
	AM_RANGE( 0xb000, 0xbfff ) AM_RAMBANK("sprbank")

	AM_RANGE( 0xd000, 0xd1ff ) AM_RAM_WRITE( paletteram_xRRRRRGGGGGBBBBB_be_w ) AM_BASE_GENERIC( paletteram )
	AM_RANGE( 0xd800, 0xd87f ) AM_RAM	// table?

	AM_RANGE( 0xe011, 0xe011 ) AM_WRITENOP	// IRQ Enable? Screen disable?
	AM_RANGE( 0xe013, 0xe013 ) AM_READWRITE( vblank_r, vblank_w )	// IRQ Ack?

	AM_RANGE( 0xfe00, 0xffff ) AM_RAM	// High speed internal RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( animalc_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x02, 0x03 ) AM_READWRITE( animalc_rombank_r, animalc_rombank_w )
	AM_RANGE( 0x04, 0x05 ) AM_READWRITE( animalc_rambank_r, animalc_rambank_w )

	AM_RANGE( 0x2c, 0x2c ) AM_DEVREADWRITE( "eeprom", sammymdl_eeprom_r, sammymdl_eeprom_w )
	AM_RANGE( 0x2e, 0x2e ) AM_READ( sammymdl_coin_hopper_r )
	AM_RANGE( 0x30, 0x30 ) AM_READ_PORT( "BUTTON" )
	AM_RANGE( 0x31, 0x31 ) AM_WRITE( sammymdl_coin_w )
	AM_RANGE( 0x32, 0x32 ) AM_WRITE( sammymdl_leds_w )
	AM_RANGE( 0x34, 0x34 ) AM_READ( unk_34_r )
	AM_RANGE( 0x90, 0x90 ) AM_DEVWRITE_MODERN("oki", okim9810_device, write )
	AM_RANGE( 0x91, 0x91 ) AM_DEVWRITE_MODERN("oki", okim9810_device, write_TMP_register )
	AM_RANGE( 0x92, 0x92 ) AM_DEVREAD_MODERN("oki", okim9810_device, read )
	AM_RANGE( 0xb0, 0xb0 ) AM_WRITE( sammymdl_hopper_w )
	AM_RANGE( 0xc0, 0xc0 ) AM_WRITE( watchdog_reset_w )	// 1
ADDRESS_MAP_END

/***************************************************************************
                             Hae Hae Ka Ka Ka
***************************************************************************/

// rombank
static WRITE8_HANDLER( haekaka_rombank_w )
{
	if (offset == 0)
	{
		reg = data;
		return;
	}

	switch ( reg )
	{
		case 0x2b:
			rombank = data;
			switch (data)
			{
				case 0x10:	// ROM
				case 0x11:
				case 0x12:
				case 0x13:
				case 0x14:
				case 0x15:
				case 0x16:
				case 0x17:
				case 0x18:
				case 0x19:
				case 0x1a:
				case 0x1b:
				case 0x1c:
				case 0x1d:
				case 0x1e:
				case 0x1f:

				case 0x65:	// SPRITERAM
				case 0x67:	// PALETTE RAM + TABLE + REGS
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", space->machine().describe_context(), data, reg);
			}
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", space->machine().describe_context(), reg, data);
	}
}
static READ8_HANDLER( haekaka_rombank_r )
{
	if (offset == 0)
		return reg;

	switch ( reg )
	{
		case 0x2b:
			return rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", space->machine().describe_context(), reg);
			return 0x00;
	}
}

// rambank
static WRITE8_HANDLER( haekaka_rambank_w )
{
	if (offset == 0)
	{
		reg2 = data;
		return;
	}

	switch ( reg2 )
	{
		case 0x33:
			rambank = data;
			switch (data)
			{
				case 0x53:
					break;

				default:
					logerror("%s: unknown ram bank = %02x, reg2 = %02x\n", space->machine().describe_context(), data, reg2);
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", space->machine().describe_context(), reg2, data);
	}
}
static READ8_HANDLER( haekaka_rambank_r )
{
	if (offset == 0)
		return reg2;

	switch ( reg2 )
	{
		case 0x33:
			return rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", space->machine().describe_context(), reg2);
			return 0x00;
	}
}

static READ8_HANDLER( haekaka_vblank_r )
{
	return space->machine().primary_screen->vblank() ? 0 : 0x1c;
}

static READ8_HANDLER( haekaka_b000_r )
{
	sigmab98_state *state = space->machine().driver_data<sigmab98_state>();
	switch (rombank)
	{
		case 0x10:	// ROM
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			return space->machine().region("maincpu")->base()[offset + 0xb400 + 0x1000 * (rombank-0x10)];

		case 0x65:	// SPRITERAM
			if (offset < 0x1000)
				return state->m_spriteram[offset];

		case 0x67:	// PALETTERAM + TABLE? + REGS
			if (offset < 0x200)
				return space->machine().generic.paletteram.u8[offset];
			else if (offset == (0xc013-0xb000))
				return haekaka_vblank_r(space, offset);
			break;
	}

	logerror("%s: unknown read from %02x with rombank = %02x\n", space->machine().describe_context(), offset+0xb000, rombank);
	return 0x00;
}

static WRITE8_HANDLER( haekaka_b000_w )
{
	sigmab98_state *state = space->machine().driver_data<sigmab98_state>();
	switch (rombank)
	{
		case 0x65:	// SPRITERAM
			if (offset < 0x1000)
			{
				state->m_spriteram[offset] = data;
				return;
			}
			break;

		case 0x67:	// PALETTERAM + TABLE? + REGS
			if (offset < 0x200)
			{
				paletteram_xRRRRRGGGGGBBBBB_be_w(space, offset, data);
//              space->machine().generic.paletteram.u8[offset] = data;
				return;
			}
			else if ((offset >= 0x800) && (offset < 0x880))
			{
				// table?
				return;
			}
			break;
	}

	logerror("%s: unknown write to %02x = %02x with rombank = %02x\n", space->machine().describe_context(), offset+0xb000, data, rombank);
}

static WRITE8_HANDLER( haekaka_leds_w )
{
	// All used
	set_led_status(space->machine(), 0,	data & 0x01);
	set_led_status(space->machine(), 1,	data & 0x02);
	set_led_status(space->machine(), 2,	data & 0x04);
	set_led_status(space->machine(), 3,	data & 0x08);
	set_led_status(space->machine(), 4,	data & 0x10);
	set_led_status(space->machine(), 5,	data & 0x20);
	set_led_status(space->machine(), 6,	data & 0x40);
	set_led_status(space->machine(), 7,	data & 0x80);

	out[1] = data;
	show_3_outputs();
}

static WRITE8_HANDLER( haekaka_coin_w )
{
	coin_counter_w(space->machine(), 0,   data & 0x01 );	// medal out
//                                      data & 0x02 ?
//                                      data & 0x04 ?
//                                      data & 0x10 ?

	out[0] = data;
	show_3_outputs();
}

static ADDRESS_MAP_START( haekaka_map, AS_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x7fff ) AM_ROM
	AM_RANGE( 0xb000, 0xcfff ) AM_READWRITE( haekaka_b000_r, haekaka_b000_w )
	AM_RANGE( 0xd000, 0xefff ) AM_RAM AM_SHARE( "nvram" ) AM_BASE( &nvram )
	AM_RANGE( 0xfe00, 0xffff ) AM_RAM	// High speed internal RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( haekaka_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x02, 0x03 ) AM_READWRITE( haekaka_rombank_r, haekaka_rombank_w )
	AM_RANGE( 0x04, 0x05 ) AM_READWRITE( haekaka_rambank_r, haekaka_rambank_w )

	AM_RANGE( 0x2c, 0x2c ) AM_DEVREADWRITE( "eeprom", sammymdl_eeprom_r, sammymdl_eeprom_w )
	AM_RANGE( 0x2e, 0x2e ) AM_READ( sammymdl_coin_hopper_r )
	AM_RANGE( 0x30, 0x30 ) AM_READ_PORT( "BUTTON" )
	AM_RANGE( 0x31, 0x31 ) AM_WRITE( haekaka_coin_w )
	AM_RANGE( 0x32, 0x32 ) AM_WRITE( haekaka_leds_w )
	AM_RANGE( 0x90, 0x90 ) AM_DEVWRITE_MODERN("oki", okim9810_device, write )
	AM_RANGE( 0x91, 0x91 ) AM_DEVWRITE_MODERN("oki", okim9810_device, write_TMP_register )
	AM_RANGE( 0x92, 0x92 ) AM_DEVREAD_MODERN("oki", okim9810_device, read )
	AM_RANGE( 0xb0, 0xb0 ) AM_WRITE( sammymdl_hopper_w )
	AM_RANGE( 0xc0, 0xc0 ) AM_WRITE( watchdog_reset_w )	// 1
ADDRESS_MAP_END

/***************************************************************************
                              Itazura Monkey
***************************************************************************/

// rombank
static WRITE8_HANDLER( itazuram_rombank_w )
{
	sigmab98_state *state = space->machine().driver_data<sigmab98_state>();
	if (offset == 0)
	{
		reg = data;
		return;
	}

	UINT8 *rom = space->machine().region("maincpu")->base();
	switch ( reg )
	{
		case 0x0d:
			rombank = data;
			switch (data)
			{
				case 0x11:	// 3800 IS ROM
					memory_set_bankptr(space->machine(), "rombank0", rom + 0x4c00);
					memory_set_bankptr(space->machine(), "rombank1", rom + 0x5c00);
					memory_set_bankptr(space->machine(), "sprbank0", state->m_spriteram + 0x1000*4);	// scratch
					memory_set_bankptr(space->machine(), "sprbank1", state->m_spriteram + 0x1000*4);	// scratch
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", space->machine().describe_context(), data, reg);
			}
			break;

		case 0x4d:
			rombank = data;
			switch (data)
			{
				case 0x14:	// 3800 IS ROM
					memory_set_bankptr(space->machine(), "rombank0", rom + 0x8000);
					memory_set_bankptr(space->machine(), "rombank1", rom + 0x9000);
					memory_set_bankptr(space->machine(), "sprbank0", state->m_spriteram + 0x1000*4);	// scratch
					memory_set_bankptr(space->machine(), "sprbank1", state->m_spriteram + 0x1000*4);	// scratch
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", space->machine().describe_context(), data, reg);
			}
			break;

		case 0x8d:
			rombank = data;
			switch (data)
			{
				case 0x0f:	// 3800 IS ROM
					memory_set_bankptr(space->machine(), "rombank0", rom + 0x3400);
					memory_set_bankptr(space->machine(), "rombank1", rom + 0x4400);
					memory_set_bankptr(space->machine(), "sprbank0", state->m_spriteram + 0x1000*4);	// scratch
					memory_set_bankptr(space->machine(), "sprbank1", state->m_spriteram + 0x1000*4);	// scratch
					break;

				case 0x12:	// 3800 IS ROM
					memory_set_bankptr(space->machine(), "rombank0", rom + 0x6400);
					memory_set_bankptr(space->machine(), "rombank1", rom + 0x7400);
					memory_set_bankptr(space->machine(), "sprbank0", state->m_spriteram + 0x1000*4);	// scratch
					memory_set_bankptr(space->machine(), "sprbank1", state->m_spriteram + 0x1000*4);	// scratch
					break;

				// used in test mode:
//              case 0x5c:  memory_set_bankptr(space->machine(), "rombank", rom + 0x400 + 0x0000);    break;  // 3800 IS RAM! (8000 bytes)

				case 0x5e:	// 3800 IS RAM! (1404 bytes)
					memory_set_bankptr(space->machine(), "rombank0", state->m_spriteram + 0x1000*1);
					memory_set_bankptr(space->machine(), "sprbank0", state->m_spriteram + 0x1000*1);
					memory_set_bankptr(space->machine(), "rombank1", state->m_spriteram + 0x1000*2);
					memory_set_bankptr(space->machine(), "sprbank1", state->m_spriteram + 0x1000*2);
					break;

				case 0x6c:	// 3800 IS RAM! (1000 bytes) - SPRITERAM
					memory_set_bankptr(space->machine(), "rombank0", state->m_spriteram);
					memory_set_bankptr(space->machine(), "sprbank0", state->m_spriteram);
//                  memory_set_bankptr(space->machine(), "sprbank1", state->m_spriteram + 0x1000*4);    // scratch
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", space->machine().describe_context(), data, reg);
			}
			break;

		case 0xcd:
			rombank = data;
			switch (data)
			{
				case 0x14:	// 3800 IS ROM
					memory_set_bankptr(space->machine(), "rombank0", rom + 0x8800);
					memory_set_bankptr(space->machine(), "rombank1", rom + 0x9800);
					memory_set_bankptr(space->machine(), "sprbank0", state->m_spriteram + 0x1000*4);	// scratch
					memory_set_bankptr(space->machine(), "sprbank1", state->m_spriteram + 0x1000*4);	// scratch
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", space->machine().describe_context(), data, reg);
			}
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", space->machine().describe_context(), reg, data);
	}
}
static READ8_HANDLER( itazuram_rombank_r )
{
	if (offset == 0)
		return reg;

	switch ( reg )
	{
		// FIXME different registers
		case 0x0d:
		case 0x4d:
		case 0x8d:
		case 0xcd:
			return rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", space->machine().describe_context(), reg);
			return 0x00;
	}
}

// rambank
static WRITE8_HANDLER( itazuram_rambank_w )
{
	if (offset == 0)
	{
		reg2 = data;
		return;
	}

	switch ( reg2 )
	{
		case 0x76:
			rambank = data;
			switch (data)
			{
				case 0x52:	memory_set_bankptr(space->machine(), "palbank", nvram);									break;
				case 0x64:	memory_set_bankptr(space->machine(), "palbank", space->machine().generic.paletteram.u8);	break;
				default:
					logerror("%s: unknown ram bank = %02x, reg2 = %02x\n", space->machine().describe_context(), data, reg2);
					return;
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", space->machine().describe_context(), reg2, data);
	}
}

static READ8_HANDLER( itazuram_rambank_r )
{
	if (offset == 0)
		return reg2;

	switch ( reg2 )
	{
		case 0x76:
			return rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", space->machine().describe_context(), reg2);
			return 0x00;
	}
}

static WRITE8_HANDLER( itazuram_nvram_palette_w )
{
	if (rambank == 0x64)
	{
		paletteram_xRRRRRGGGGGBBBBB_be_w(space, offset, data);
//      space->machine().generic.paletteram.u8[offset] = data;
	}
	else if (rambank == 0x52)
	{
		nvram[offset] = data;
	}
	else
	{
		logerror("%s: itazuram_nvram_palette_w offset = %03x with unknown bank = %02x\n", space->machine().describe_context(), offset, rambank);
	}
}

static WRITE8_HANDLER( itazuram_palette_w )
{
	if (rombank == 0x6c)
	{
		if (offset < 0x200)
			paletteram_xRRRRRGGGGGBBBBB_be_w(space, offset, data);
//          space->machine().generic.paletteram.u8[offset] = data;
	}
	else
	{
		logerror("%s: itazuram_palette_w offset = %03x with unknown bank = %02x\n", space->machine().describe_context(), offset, rombank);
	}
}
static READ8_HANDLER( itazuram_palette_r )
{
	return space->machine().generic.paletteram.u8[offset];
}

static ADDRESS_MAP_START( itazuram_map, AS_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x37ff ) AM_ROM
	AM_RANGE( 0x3800, 0x47ff ) AM_READ_BANK( "rombank0" ) AM_WRITE_BANK( "sprbank0" )
	AM_RANGE( 0x4800, 0x57ff ) AM_READ_BANK( "rombank1" ) AM_WRITE_BANK( "sprbank1" )

	AM_RANGE( 0x5800, 0x59ff ) AM_READWRITE( itazuram_palette_r, itazuram_palette_w )
	AM_RANGE( 0x6000, 0x607f ) AM_RAM	// table?

	AM_RANGE( 0x6811, 0x6811 ) AM_WRITENOP	// IRQ Enable? Screen disable?
	AM_RANGE( 0x6813, 0x6813 ) AM_WRITENOP	// IRQ Ack?
	AM_RANGE( 0xdc00, 0xfdff ) AM_READ_BANK( "palbank" ) AM_WRITE( itazuram_nvram_palette_w ) AM_SHARE( "nvram" ) AM_BASE( &nvram )	// nvram | paletteram

	AM_RANGE( 0xfe00, 0xffff ) AM_RAM	// High speed internal RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( itazuram_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x02, 0x03 ) AM_READWRITE( itazuram_rombank_r, itazuram_rombank_w )
	AM_RANGE( 0x04, 0x05 ) AM_READWRITE( itazuram_rambank_r, itazuram_rambank_w )

	AM_RANGE( 0x2c, 0x2c ) AM_DEVREADWRITE( "eeprom", sammymdl_eeprom_r, sammymdl_eeprom_w )
	AM_RANGE( 0x2e, 0x2e ) AM_READ( sammymdl_coin_hopper_r )
	AM_RANGE( 0x30, 0x30 ) AM_READ_PORT( "BUTTON" )
	AM_RANGE( 0x31, 0x31 ) AM_WRITE( sammymdl_coin_w )
	AM_RANGE( 0x32, 0x32 ) AM_WRITE( sammymdl_leds_w )
	AM_RANGE( 0x90, 0x90 ) AM_DEVWRITE_MODERN("oki", okim9810_device, write )
	AM_RANGE( 0x91, 0x91 ) AM_DEVWRITE_MODERN("oki", okim9810_device, write_TMP_register )
	AM_RANGE( 0x92, 0x92 ) AM_DEVREAD_MODERN("oki", okim9810_device, read )
	AM_RANGE( 0xb0, 0xb0 ) AM_WRITE( sammymdl_hopper_w )
	AM_RANGE( 0xc0, 0xc0 ) AM_WRITE( watchdog_reset_w )	// 1
ADDRESS_MAP_END

/***************************************************************************
                             Pye-nage Taikai
***************************************************************************/

static ADDRESS_MAP_START( pyenaget_io, AS_IO, 8 )
	AM_RANGE( 0x31, 0x31 ) AM_WRITE( sammymdl_coin_w )
	AM_IMPORT_FROM( haekaka_io )
ADDRESS_MAP_END

/***************************************************************************
                             Taihou de Doboon
***************************************************************************/

// rombank
static WRITE8_HANDLER( tdoboon_rombank_w )
{
	if (offset == 0)
	{
		reg = data;
		return;
	}

	switch ( reg )
	{
		case 0x2f:
			rombank = data;
			switch (data)
			{
				case 0x10:	// ROM
				case 0x11:
				case 0x12:
				case 0x13:
				case 0x14:
				case 0x15:
				case 0x16:
				case 0x17:
				case 0x18:
				case 0x19:
				case 0x1a:
				case 0x1b:
				case 0x1c:
				case 0x1d:
				case 0x1e:
				case 0x1f:

				case 0x64:	// SPRITERAM
				case 0x66:	// PALETTE RAM + TABLE
				case 0x67:	// REGS
					break;

				default:
					logerror("%s: unknown rom bank = %02x, reg = %02x\n", space->machine().describe_context(), data, reg);
			}
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", space->machine().describe_context(), reg, data);
	}
}
static READ8_HANDLER( tdoboon_rombank_r )
{
	if (offset == 0)
		return reg;

	switch ( reg )
	{
		case 0x2f:
			return rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", space->machine().describe_context(), reg);
			return 0x00;
	}
}

// rambank
static WRITE8_HANDLER( tdoboon_rambank_w )
{
	if (offset == 0)
	{
		reg2 = data;
		return;
	}

	switch ( reg2 )
	{
		case 0x33:
			rambank = data;
			switch (data)
			{
				case 0x53:
					break;

				default:
					logerror("%s: unknown ram bank = %02x, reg2 = %02x\n", space->machine().describe_context(), data, reg2);
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", space->machine().describe_context(), reg2, data);
	}
}
static READ8_HANDLER( tdoboon_rambank_r )
{
	if (offset == 0)
		return reg2;

	switch ( reg2 )
	{
		case 0x33:
			return rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", space->machine().describe_context(), reg2);
			return 0x00;
	}
}

static READ8_HANDLER( tdoboon_c000_r )
{
	sigmab98_state *state = space->machine().driver_data<sigmab98_state>();
	switch (rombank)
	{
		case 0x10:	// ROM
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			return space->machine().region("maincpu")->base()[offset + 0xc400 + 0x1000 * (rombank-0x10)];

		case 0x64:	// SPRITERAM
			if (offset < 0x1000)
				return state->m_spriteram[offset];
			break;

		case 0x66:	// PALETTERAM + TABLE?
			if (offset < 0x200)
				return space->machine().generic.paletteram.u8[offset];
			break;

		case 0x67:	// REGS
			if (offset == (0xc013-0xc000))
				return haekaka_vblank_r(space, offset);
			break;
	}

	logerror("%s: unknown read from %02x with rombank = %02x\n", space->machine().describe_context(), offset+0xc000, rombank);
	return 0x00;
}

static WRITE8_HANDLER( tdoboon_c000_w )
{
	sigmab98_state *state = space->machine().driver_data<sigmab98_state>();
	switch (rombank)
	{
		case 0x64:	// SPRITERAM
			if (offset < 0x1000)
			{
				state->m_spriteram[offset] = data;
				return;
			}
			break;

		case 0x66:	// PALETTERAM + TABLE?
			if (offset < 0x200)
			{
				paletteram_xRRRRRGGGGGBBBBB_be_w(space, offset, data);
//              space->machine().generic.paletteram.u8[offset] = data;
				return;
			}
			else if ((offset >= 0x800) && (offset < 0x880))
			{
				// table?
				return;
			}
			break;
	}

	logerror("%s: unknown write to %02x = %02x with rombank = %02x\n", space->machine().describe_context(), offset+0xc000, data, rombank);
}

static ADDRESS_MAP_START( tdoboon_map, AS_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0xbfff ) AM_ROM
	AM_RANGE( 0xc000, 0xcfff ) AM_READWRITE( tdoboon_c000_r, tdoboon_c000_w )
	AM_RANGE( 0xd000, 0xefff ) AM_RAM AM_SHARE( "nvram" ) AM_BASE( &nvram )
	AM_RANGE( 0xfe00, 0xffff ) AM_RAM	// High speed internal RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tdoboon_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x02, 0x03 ) AM_READWRITE( tdoboon_rombank_r, tdoboon_rombank_w )
	AM_RANGE( 0x04, 0x05 ) AM_READWRITE( tdoboon_rambank_r, tdoboon_rambank_w )

	AM_RANGE( 0x2c, 0x2c ) AM_DEVREADWRITE( "eeprom", sammymdl_eeprom_r, sammymdl_eeprom_w )
	AM_RANGE( 0x2e, 0x2e ) AM_READ( sammymdl_coin_hopper_r )
	AM_RANGE( 0x30, 0x30 ) AM_READ_PORT( "BUTTON" )
	AM_RANGE( 0x31, 0x31 ) AM_WRITE( sammymdl_coin_w )
	AM_RANGE( 0x32, 0x32 ) AM_WRITE( sammymdl_leds_w )
	AM_RANGE( 0x90, 0x90 ) AM_DEVWRITE_MODERN("oki", okim9810_device, write )
	AM_RANGE( 0x91, 0x91 ) AM_DEVWRITE_MODERN("oki", okim9810_device, write_TMP_register )
	AM_RANGE( 0x92, 0x92 ) AM_DEVREAD_MODERN("oki", okim9810_device, read )
	AM_RANGE( 0xb0, 0xb0 ) AM_WRITE( sammymdl_hopper_w )
	AM_RANGE( 0xc0, 0xc0 ) AM_WRITE( watchdog_reset_w )	// 1
ADDRESS_MAP_END


/***************************************************************************

    Graphics Layout

***************************************************************************/

static const gfx_layout sigmab98_16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 4*1,4*0, 4*3,4*2, 4*5,4*4, 4*7,4*6, 4*9,4*8, 4*11,4*10, 4*13,4*12, 4*15,4*14 },
	{ STEP16(0,16*4) },
	16*16*4
};

static const gfx_layout sigmab98_16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16*16*8
};

static GFXDECODE_START( sigmab98 )
	GFXDECODE_ENTRY( "sprites", 0, sigmab98_16x16x4_layout, 0, 0x100/16  )
	GFXDECODE_ENTRY( "sprites", 0, sigmab98_16x16x8_layout, 0, 0x100/256 )
GFXDECODE_END


/***************************************************************************

    Input Ports

***************************************************************************/

/***************************************************************************
                        GeGeGe no Kitarou Youkai Slot
***************************************************************************/

static INPUT_PORTS_START( gegege )

	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL )	// protection? checks. Must be 0
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL )	// protection? checks. Must be 0
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN2   ) PORT_IMPULSE(5)	// ? (coin error, pulses mask 4 of port c6)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN1   ) PORT_IMPULSE(5) PORT_NAME("Medal")	// coin/medal in (coin error)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("hopper", ticket_dispenser_line_r)
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_NAME("Bet")	// bet / select in test menu
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("Play")	// play game / select in test menu
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_NAME("Pay Out")	// pay out / change option in test menu
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

INPUT_PORTS_END

/***************************************************************************
                             Sammy Medal Games
***************************************************************************/

static INPUT_PORTS_START( sammymdl )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shot
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // freeze (itazuram)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1   ) PORT_IMPULSE(5)	// coin1 in
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2   ) PORT_IMPULSE(5)	// coin2 in
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN3   ) PORT_IMPULSE(5) PORT_NAME("Medal")	// medal in
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE )	// test sw
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("hopper", ticket_dispenser_line_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( haekaka )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(5) PORT_NAME( "Medal" )	// medal in ("chacker")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE  )	// test sw
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1  )	// button
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL  ) PORT_READ_LINE_DEVICE("hopper", ticket_dispenser_line_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SERVICE1 )	// service coin / set in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/

/***************************************************************************
                        GeGeGe no Kitarou Youkai Slot
***************************************************************************/

const eeprom_interface eeprom_intf =
{
	6,				// address bits 6
	16,				// data bits    16
	"*110",			// read         1 10 aaaaaa
	"*101",			// write        1 01 aaaaaa dddddddddddddddd
	"*111",			// erase        1 11 aaaaaa
	"*10000xxxx",	// lock         1 00 00xxxx
	"*10011xxxx",	// unlock       1 00 11xxxx
	0,				// enable_multi_read
	7				// reset_delay (otherwise gegege will hang when saving settings)
};

static INTERRUPT_GEN( gegege_vblank_interrupt )
{
	device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x5a);
}

static MACHINE_CONFIG_START( gegege, sigmab98_state )
	MCFG_CPU_ADD("maincpu", Z80, 10000000)	// !! TAXAN KY-80, clock @X1? !!
	MCFG_CPU_PROGRAM_MAP(gegege_mem_map)
	MCFG_CPU_IO_MAP(gegege_io_map)
	MCFG_CPU_VBLANK_INT("screen", gegege_vblank_interrupt)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_EEPROM_ADD("eeprom", eeprom_intf)

	MCFG_TICKET_DISPENSER_ADD("hopper", 200, TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)					// ?
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)	// game reads vblank state
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(0x200, 0x200)
	MCFG_SCREEN_VISIBLE_AREA(0,0x140-1, 0,0xf0-1)
	MCFG_SCREEN_UPDATE(sigmab98)

	MCFG_GFXDECODE(sigmab98)
	MCFG_PALETTE_LENGTH(0x100)

	// sound hardware
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymz", YMZ280B, 16934400)	// clock @X2?
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/***************************************************************************
                             Sammy Medal Games
***************************************************************************/

static const eeprom_interface eeprom_interface_93C46_8bit =
{
	7,				// address bits 6
	8,				// data bits    8
	"*110",			// read         1 10 aaaaaa
	"*101",			// write        1 01 aaaaaa dddddddd
	"*111",			// erase        1 11 aaaaaa
	"*10000xxxx",	// lock         1 00 00xxxx
	"*10011xxxx",	// unlock       1 00 11xxxx
	0,				// enable_multi_read
	1				// reset_delay (needed by animalc)
//  "*10001xxxx"    // write all    1 00 01xxxx dddddddd
//  "*10010xxxx"    // erase all    1 00 10xxxx
};

static MACHINE_RESET( sammymdl )
{
	cpu_set_reg(machine.device("maincpu"), Z80_PC, 0x400);	// code starts at 400 ??? (000 = cart header)
}

static MACHINE_CONFIG_START( sammymdl, sigmab98_state )
	MCFG_CPU_ADD("maincpu", Z80, XTAL_20MHz / 2)	// !! KL5C80A120FP @ 10MHz? (actually 4 times faster than Z80) !!
	MCFG_CPU_PROGRAM_MAP( animalc_map )
	MCFG_CPU_IO_MAP( animalc_io )

	MCFG_MACHINE_RESET( sammymdl )

	MCFG_NVRAM_ADD_0FILL("nvram")	// battery
	MCFG_EEPROM_ADD("eeprom", eeprom_interface_93C46_8bit)

	MCFG_TICKET_DISPENSER_ADD("hopper", 200, TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW )

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(0x140, 0x100)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x140-1, 0, 0xf0-1)
	MCFG_SCREEN_UPDATE(sigmab98)
	MCFG_SCREEN_EOF(sammymdl)

	MCFG_GFXDECODE(sigmab98)
	MCFG_PALETTE_LENGTH(0x100)

	// sound hardware
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM9810_ADD("oki", XTAL_4_096MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)
MACHINE_CONFIG_END


/***************************************************************************
                                 Animal Catch
***************************************************************************/

static INTERRUPT_GEN( animalc )
{
	switch (cpu_getiloops(device))
	{
		case 0:
			device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x00);	// increment counter
			break;

		case 1:
			device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x1c);	// read hopper state
			break;

		case 2:
			device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x1e);	// drive hopper motor
			break;
	}
}

static MACHINE_CONFIG_DERIVED( animalc, sammymdl )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP( animalc_map )
	MCFG_CPU_IO_MAP( animalc_io )
	MCFG_CPU_VBLANK_INT_HACK(animalc, 3) // IM 2 needs a vector on the data bus
MACHINE_CONFIG_END

/***************************************************************************
                             Hae Hae Ka Ka Ka
***************************************************************************/

static INTERRUPT_GEN( haekaka )
{
	switch (cpu_getiloops(device))
	{
		case 0:
			device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x04);
			break;

		case 1:
			device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x1a);
			break;

		case 2:
			device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x1c);
			break;
	}
}

static MACHINE_CONFIG_DERIVED( haekaka, sammymdl )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP( haekaka_map )
	MCFG_CPU_IO_MAP( haekaka_io )
	MCFG_CPU_VBLANK_INT_HACK(haekaka, 3) // IM 2 needs a vector on the data bus
MACHINE_CONFIG_END

/***************************************************************************
                              Itazura Monkey
***************************************************************************/

static INTERRUPT_GEN( itazuram )
{
	switch (cpu_getiloops(device))
	{
		case 0:
			device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x00);	// sprites
			break;

		case 1:
			device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x02);	// copy palette
			break;

		case 2:
			device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x16);	// hopper, i/o
			break;
	}
}

static MACHINE_CONFIG_DERIVED( itazuram, sammymdl )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP( itazuram_map )
	MCFG_CPU_IO_MAP( itazuram_io )
	MCFG_CPU_VBLANK_INT_HACK(itazuram, 3) // IM 2 needs a vector on the data bus
MACHINE_CONFIG_END

/***************************************************************************
                             Pye-nage Taikai
***************************************************************************/

static MACHINE_CONFIG_DERIVED( pyenaget, sammymdl )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP( haekaka_map )
	MCFG_CPU_IO_MAP( pyenaget_io )
	MCFG_CPU_VBLANK_INT_HACK(haekaka, 3) // IM 2 needs a vector on the data bus
MACHINE_CONFIG_END

/***************************************************************************
                             Taihou de Doboon
***************************************************************************/

static MACHINE_CONFIG_DERIVED( tdoboon, sammymdl )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP( tdoboon_map )
	MCFG_CPU_IO_MAP( tdoboon_io )
	MCFG_CPU_VBLANK_INT_HACK(haekaka, 3) // IM 2 needs a vector on the data bus

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0,0x140-1, 0+4,0xf0+4-1)
MACHINE_CONFIG_END


/***************************************************************************

    ROMs Loading

***************************************************************************/

/***************************************************************************

  GeGeGe no Kitarou Youkai Slot

  (C) 1997 Banpresto, Sigma

  PCB:

    (c) 1997 Sigma B-98-1 MAIN PCB
    970703 (Sticker)

  CPU:

    TAXAN KY-80 YAMAHA 9650 AZGC (@IC1)
    XTAL ?? (@X1)

  Video:

    TAXAN KY-3211 9722 AZGC (@IC11)
    XTAL 27.000 MHz (@XOSC1)
    M548262-60 (@IC24) - 262144-Word x 8-Bit Multiport DRAM

  Sound:

    YAMAHA YMZ280B-F (@IC14)
    XTAL ?? (@X2)
    Trimmer

  Other:

    93C46AN EEPROM (@IC5)
    MAX232CPE (@IC6)
    Battery (@BAT)

***************************************************************************/

ROM_START( gegege )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9804-1.ic7", 0x00000, 0x20000, CRC(f8b4f855) SHA1(598bd9f91123e9ab539ce3f33779bff2d072e731) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "b9804-2.ic12", 0x00000, 0x80000, CRC(4211079d) SHA1(d601c623fb909f1346fd02b8fb37b67956e2cd4e) )
	ROM_LOAD( "b9804-3.ic13", 0x80000, 0x80000, CRC(54aeb2aa) SHA1(ccf939111f6288a889846d51bab47ff4e992c542) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b9804-5.ic16", 0x00000, 0x80000, CRC(ddd7984c) SHA1(3558c495776671ffd3cd5c665b87827b3959b360) )
ROM_END

static DRIVER_INIT( gegege )
{
	UINT8 *rom = machine.region("maincpu")->base();

	// Protection?
	rom[0xbd3] = 0x18;
	rom[0xbd4] = 0x14;

	rom[0xbef] = 0x18;
	rom[0xbf0] = 0x14;

	rom[0xdec] = 0x00;
	rom[0xded] = 0x00;

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;

	// ROM banks
	memory_configure_bank(machine, "rombank", 0, 0x18, rom + 0x8000, 0x1000);
	memory_set_bank(machine, "rombank", 0);

	// RAM banks
	UINT8 *bankedram = auto_alloc_array(machine, UINT8, 0x800 * 2);

	memory_configure_bank(machine, "rambank", 0, 2, bankedram, 0x800);
	memory_set_bank(machine, "rambank", 0);
}


/***************************************************************************

  Sammy Medal Games

  PCB:

    Sammy AM3AHF-01 SC MAIN PCB VER2 (Etched)
    MAIN PCB VER2 VM12-6001-0 (Sticker)

  CPU:

    KAWASAKI KL5C80A120FP (@U1) - Z80 Compatible High Speed Microcontroller
    XTAL 20 MHz  (@X1)
    MX29F040TC-12 VM1211L01 (@U2) - 4M-bit [512kx8] CMOS Equal Sector Flash Memory
    BSI BS62LV256SC-70      (@U4) - Very Low Power/Voltage CMOS SRAM 32K X 8 bit

  Video:

    TAXAN KY-3211 ? (@U17)
    M548262-60 (@U18) - 262144-Word x 8-Bit Multiport DRAM
    XTAL 27 MHz (@X3)

  Sound:

    OKI M9810B (@U11)
    XTAL 4.09 MHz (@X2)
    Trimmer (@VR1)
    Toshiba TA7252AP (@U16) - 5.9W Audio Power Amplifier

  Other:

    Xilinx XC9536 VM1212F01 (@U5) - In-System Programmable CPLD
    MX29F0??C (@U3) - Empty 32 Pin ROM Socket
    M93C46MN6T (@U11?) - Serial EEPROM
    Cell Battery (@BAT)
    25 Pin Edge Connector
    56 Pin Cartridge Connector
    6 Pin Connector

***************************************************************************/

#define SAMMYMDL_BIOS																								\
	ROM_REGION( 0x80000, "mainbios", 0 )																			\
	ROM_SYSTEM_BIOS( 0, "v5", "IPL Ver. 5.0" )																		\
	ROM_LOAD( "vm1211l01.u2", 0x000000, 0x080000, CRC(c3c74dc5) SHA1(07352e6dba7514214e778ba39e1ca773e4698858) )

ROM_START( sammymdl )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASEFF )

	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x200000, "sprites", ROMREGION_ERASEFF )
ROM_END

/***************************************************************************

  Animal Catch ( VX2002L02 ANIMALCAT 200011211536 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  1 x MX29F1610ATC-12 (@U021)
  1 x MX29F1610TC-12  (@U016)

***************************************************************************/

ROM_START( animalc )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", 0 )
	ROM_LOAD( "vx2302l01.u021", 0x00000, 0x200000, CRC(84cf123b) SHA1(d8b425c93ff1a560e3f92c70d7eb93a05c3581af) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "vx2301l01.u016", 0x00000, 0x200000, CRC(4ae14ff9) SHA1(1273d15ea642452fecacff572655cd3ab47a5884) )	// 1xxxxxxxxxxxxxxxxxxxx = 0x00
ROM_END

static DRIVER_INIT( animalc )
{
	sigmab98_state *state = machine.driver_data<sigmab98_state>();
	// RAM banks
	UINT8 *bankedram = auto_alloc_array(machine, UINT8, 0x1000 * 5);
	memory_configure_bank(machine, "rambank", 0, 1, nvram,     0x1000);
	memory_configure_bank(machine, "rambank", 1, 4, bankedram, 0x1000);
	memory_set_bank(machine, "rambank", 0);

	state->m_spriteram = auto_alloc_array(machine, UINT8, 0x1000 * 5);
	memset(state->m_spriteram, 0, 0x1000 * 5);
	state->m_spriteram_size = 0x1000;
	memory_configure_bank(machine, "sprbank", 0, 5, state->m_spriteram, 0x1000);
	memory_set_bank(machine, "sprbank", 0);
}

/***************************************************************************

  Itazura Monkey ( VX1902L02 ITZRMONKY 200011211639 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

***************************************************************************/

ROM_START( itazuram )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", 0 )
	ROM_LOAD( "vx2002l01.u021", 0x00000, 0x200000, CRC(ddbdd2f3) SHA1(91f67a938929be0261442e066e3d2c03b5e9f06a) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "vx2001l01.u016", 0x00000, 0x200000, CRC(9ee95222) SHA1(7154d43ef312a48a882207ca37e1c61e8b215a9b) )
ROM_END

static DRIVER_INIT( itazuram )
{
	sigmab98_state *state = machine.driver_data<sigmab98_state>();
	// ROM banks
	UINT8 *rom = machine.region("maincpu")->base();
	memory_set_bankptr(machine, "rombank0", rom + 0x3400);
	memory_set_bankptr(machine, "rombank1", rom + 0x4400);
	rombank = 0x0f;

	// RAM banks
	machine.generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x3000);
	memset(machine.generic.paletteram.u8, 0, 0x3000);
	memory_set_bankptr(machine, "palbank", machine.generic.paletteram.u8);
	rambank = 0x64;

	state->m_spriteram = auto_alloc_array(machine, UINT8, 0x1000 * 5);
	memset(state->m_spriteram, 0, 0x1000 * 5);
	state->m_spriteram_size = 0x1000;
	memory_set_bankptr(machine, "sprbank0",  state->m_spriteram + 0x1000*4);	// scratch
	memory_set_bankptr(machine, "sprbank1",  state->m_spriteram + 0x1000*4);	// scratch
}

/***************************************************************************

  Taihou de Doboon ( EM4210L01 PUSHERDBN 200203151028 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

  The flash roms found in the cart are labeled:

  vx1801l01.u016
  vx1802l01.u021

  which correspond to "Pye-nage Taikai". But they were reflashed with the
  software noted in the sticker on the back of the cart (rom names reflect that):

  System: Treasure Hall
  Soft: Taihou de Doboon
  2003.02.14
  Char Rev: EM4209L01
  Pro  Rev: EM4210L01

***************************************************************************/

ROM_START( tdoboon )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", 0 )
	ROM_LOAD( "em4210l01.u021.bin", 0x00000, 0x200000, CRC(3523e314) SHA1(d07c5d17d3f285be4cde810547f427e84f98968f) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "em4209l01.u016.bin", 0x00000, 0x200000, CRC(aca220fa) SHA1(7db441add16af554700e597fd9926b6ccd19d628) )	// 1xxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

/***************************************************************************

  Pye-nage Taikai ( VX1802L01 PAINAGETK 200011021216 SAMMY CORP. AM. )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

***************************************************************************/

ROM_START( pyenaget )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", 0 )
	ROM_LOAD( "vx1802l01.u021", 0x00000, 0x200000, CRC(7a22a657) SHA1(2a98085862fd958209253c5401e41eae4f7c06ea) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "vx1801l01.u016", 0x00000, 0x200000, CRC(c4607403) SHA1(f4f4699442afccc5ed4354447f91b1bee36ae3e5) )
ROM_END

/***************************************************************************

  Hae Hae Ka Ka Ka ( EM4208L01 PUSHERHAEHAE 200203151032 SAMMY CORP. AM )

  Sammy AM3ADT-01 Memory Card Type 3:
  2 x MX29F1610ATC-12

  The flash roms found in the cart are labeled:

  vx1801l01.u016
  vx1802l01.u021

  which correspond to "Pye-nage Taikai". But they were reflashed with the
  software noted in the sticker on the back of the cart (rom names reflect that):

  System: Treasure Hall
  Soft: Hae Hae Ka Ka Ka
  2003.02.14
  Char Rev: EM4207L01
  Pro  Rev: EM4208L01

***************************************************************************/

ROM_START( haekaka )
	SAMMYMDL_BIOS

	ROM_REGION( 0x1000000, "oki", 0 )
	ROM_LOAD( "em4208l01.u021.bin", 0x00000, 0x200000, CRC(d23bb748) SHA1(38d5b6c4b2cd470b3a68574aeca3f9fa9032245e) )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_COPY( "oki", 0x1c0000, 0x00000, 0x40000 )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "em4207l01.u016.bin", 0x00000, 0x200000, CRC(3876961c) SHA1(3d842c1f63ea5aa7e799967928b86c5fabb4e65e) )
ROM_END

static DRIVER_INIT( haekaka )
{
	sigmab98_state *state = machine.driver_data<sigmab98_state>();
	// RAM banks
	machine.generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x200);
	memset(machine.generic.paletteram.u8, 0, 0x200);

	state->m_spriteram = auto_alloc_array(machine, UINT8, 0x1000);
	memset(state->m_spriteram, 0, 0x1000);
	state->m_spriteram_size = 0x1000;

	rombank = 0x65;
	rambank = 0x53;
}


/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 1997, gegege,   0,        gegege,   gegege,   gegege,   ROT0, "Banpresto / Sigma", "GeGeGe no Kitarou Youkai Slot", 0 )
// Sammy Medal Games:
GAME( 2000, sammymdl, 0,        sammymdl, sammymdl, 0,        ROT0, "Sammy",             "Sammy Medal Game System Bios",  GAME_IS_BIOS_ROOT )
GAME( 2000, animalc,  sammymdl, animalc,  sammymdl, animalc,  ROT0, "Sammy",             "Animal Catch",                  0 )
GAME( 2000, itazuram, sammymdl, itazuram, sammymdl, itazuram, ROT0, "Sammy",             "Itazura Monkey",                0 )
GAME( 2000, pyenaget, sammymdl, pyenaget, sammymdl, haekaka,  ROT0, "Sammy",             "Pye-nage Taikai",               0 )
GAME( 2000, tdoboon,  sammymdl, tdoboon,  haekaka,  haekaka,  ROT0, "Sammy",             "Taihou de Doboon",              0 )
GAME( 2001, haekaka,  sammymdl, haekaka,  haekaka,  haekaka,  ROT0, "Sammy",             "Hae Hae Ka Ka Ka",              0 )
