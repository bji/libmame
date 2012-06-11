/***************************************************************************

Mini Vaders (Space Invaders's mini game)
(c)1990 Taito Corporation

Driver by Takahiro Nogi (nogi@kt.rim.or.jp) 1999/12/19 -

This is a test board sold together with the cabinet (as required by law in
Japan). It has no sound.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


class minivadr_state : public driver_device
{
public:
	minivadr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	size_t   m_videoram_size;
};

/*************************************
 *
 *  Video update
 *
 *************************************/

static SCREEN_UPDATE_RGB32( minivadr )
{
	minivadr_state *state = screen.machine().driver_data<minivadr_state>();
	offs_t offs;

	for (offs = 0; offs < state->m_videoram_size; offs++)
	{
		int i;

		UINT8 x = offs << 3;
		int y = offs >> 5;
		UINT8 data = state->m_videoram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? RGB_WHITE : RGB_BLACK;
			bitmap.pix32(y, x) = pen;

			data = data << 1;
			x = x + 1;
		}
	}

	return 0;
}


static ADDRESS_MAP_START( minivadr_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM AM_BASE_SIZE_MEMBER(minivadr_state, m_videoram, m_videoram_size)
	AM_RANGE(0xe008, 0xe008) AM_READ_PORT("INPUTS") AM_WRITENOP		// W - ???
ADDRESS_MAP_END


static INPUT_PORTS_START( minivadr )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static MACHINE_CONFIG_START( minivadr, minivadr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,24000000 / 6)		 /* 4 MHz ? */
	MCFG_CPU_PROGRAM_MAP(minivadr_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 240-1)
	MCFG_SCREEN_UPDATE_STATIC(minivadr)

	/* the board has no sound hardware */
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( minivadr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d26-01.bin",	0x0000, 0x2000, CRC(a96c823d) SHA1(aa9969ff80e94b0fff0f3530863f6b300510162e) )
ROM_END


GAME( 1990, minivadr, 0, minivadr, minivadr, 0, ROT0, "Taito Corporation", "Mini Vaders", GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )
