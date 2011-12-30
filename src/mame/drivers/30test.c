/***************************************************************************

    30 Test (Remake) (c) 1997 Namco

    driver by Angelo Salese

    TODO:
    - clickable artwork;
    - portd meaning is a mystery
    - inputs are annoying to map;
    - EEPROM

============================================================================

cheats:
- [0xb0-0xb3] timer

lamps:
?OK???!! = really OK! (91+)
???????? = pretty good (80+)
???~??? = not bad (70+) (0x84)
??? = normal (55+) (0x88)
????? = pretty bad (40+) (0x90)
???~ = worst (39 or less) (0xa0)
??????? = game over (0xe0)


============================================================================

30-TEST (Remake)
NAMCO 1997
GAME CODE M125

MC68HC11K1
M6295
X1 1.056MHz
OSC1 16.000MHz


cabinet photo
http://blogs.yahoo.co.jp/nadegatayosoyuki/59285865.html

***************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/okim6295.h"
#include "30test.lh"

#define MAIN_CLOCK XTAL_16MHz

class namco_30test_state : public driver_device
{
public:
	namco_30test_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_mux_data;
	UINT8 m_oki_bank;
};


static VIDEO_START( 30test )
{

}

static SCREEN_UPDATE( 30test )
{
	return 0;
}

static READ8_HANDLER(hc11_mux_r)
{
	namco_30test_state *state = space->machine().driver_data<namco_30test_state>();

	return state->m_mux_data;
}

static WRITE8_HANDLER(hc11_mux_w)
{
	namco_30test_state *state = space->machine().driver_data<namco_30test_state>();

	state->m_mux_data = data;
}

static READ8_HANDLER(namco_30test_mux_r)
{
	namco_30test_state *state = space->machine().driver_data<namco_30test_state>();
	UINT8 res = 0xff;

	switch(state->m_mux_data)
	{
		case 0x01: res = input_port_read(space->machine(), "IN0"); break;
		case 0x02: res = input_port_read(space->machine(), "IN1"); break;
		case 0x04: res = input_port_read(space->machine(), "IN2"); break;
		case 0x08: res = input_port_read(space->machine(), "IN3"); break;
	}

	return res;
}

static const UINT8 led_map[16] =
	{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x77,0x7c,0x39,0x5e,0x79,0x00 };

static WRITE8_HANDLER( namco_30test_led_w )
{
	output_set_digit_value(0 + offset * 2, led_map[(data & 0xf0) >> 4]);
	output_set_digit_value(1 + offset * 2, led_map[(data & 0x0f) >> 0]);
}

static WRITE8_HANDLER( namco_30test_led_rank_w )
{
	output_set_digit_value(64 + offset * 2, led_map[(data & 0xf0) >> 4]);
	output_set_digit_value(65 + offset * 2, led_map[(data & 0x0f) >> 0]);
}

static WRITE8_HANDLER( namco_30test_lamps_w )
{
	if(!(data & 0x80)) // guess: lamps mask
		data = 0;

	output_set_lamp_value(0, (data & 0x01) >> 0); // really OK! lamp
	output_set_lamp_value(1, (data & 0x02) >> 1); // pretty good lamp
	output_set_lamp_value(2, (data & 0x04) >> 2); // not bad lamp
	output_set_lamp_value(3, (data & 0x08) >> 3); // normal lamp
	output_set_lamp_value(4, (data & 0x10) >> 4); // pretty bad lamp
	output_set_lamp_value(5, (data & 0x20) >> 5); // worst lamp
	output_set_lamp_value(6, (data & 0x40) >> 6); // game over lamp
}

static READ8_DEVICE_HANDLER( hc11_okibank_r )
{
	namco_30test_state *state = device->machine().driver_data<namco_30test_state>();
	return state->m_oki_bank;
}

static WRITE8_DEVICE_HANDLER( hc11_okibank_w )
{
	namco_30test_state *state = device->machine().driver_data<namco_30test_state>();
	okim6295_device *oki = downcast<okim6295_device *>(device);

	state->m_oki_bank = data;
	oki->set_bank_base((data & 1) ? 0x40000 : 0);
}


static ADDRESS_MAP_START( namco_30test_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x003f) AM_RAM // internal I/O
	AM_RANGE(0x007c, 0x007c) AM_READWRITE(hc11_mux_r,hc11_mux_w)
	AM_RANGE(0x007e, 0x007e) AM_DEVREADWRITE("oki",hc11_okibank_r,hc11_okibank_w)
	AM_RANGE(0x0040, 0x007f) AM_RAM // more internal I/O, HC11 change pending
	AM_RANGE(0x0080, 0x037f) AM_RAM // internal RAM
	AM_RANGE(0x0d80, 0x0dbf) AM_RAM	// EEPROM read-back data goes there
	AM_RANGE(0x2000, 0x2000) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	/* 0x401e-0x401f: time */
	AM_RANGE(0x4000, 0x401f) AM_WRITE(namco_30test_led_w) // 7-seg leds
	/* 0x6000: 1st place 7-seg led */
	/* 0x6001: 2nd place 7-seg led */
	/* 0x6002: 3rd place 7-seg led */
	/* 0x6003: current / last play score */
	/* 0x6004: lamps */
	AM_RANGE(0x6000, 0x6003) AM_WRITE(namco_30test_led_rank_w)
	AM_RANGE(0x6004, 0x6004) AM_WRITE(namco_30test_lamps_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( namco_30test_io, AS_IO, 8 )
	AM_RANGE(MC68HC11_IO_PORTA,MC68HC11_IO_PORTA) AM_READ(namco_30test_mux_r)
//  AM_RANGE(MC68HC11_IO_PORTD,MC68HC11_IO_PORTD) AM_RAM
	AM_RANGE(MC68HC11_IO_PORTE,MC68HC11_IO_PORTE) AM_READ_PORT("SYSTEM")
ADDRESS_MAP_END


static INPUT_PORTS_START( 30test )
	/* we use num pad to map system inputs */
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-1") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-1") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-1") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-1") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("6-1") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-2") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-2") PORT_CODE(KEYCODE_W)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-2") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-2") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-2") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("6-2") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-3") PORT_CODE(KEYCODE_F)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-3") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("6-3") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-4") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-4") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-4") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("6-4") PORT_CODE(KEYCODE_N)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-5") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-5") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-5") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4-5") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5-5") PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("6-5") PORT_CODE(KEYCODE_RCONTROL)
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CODE(KEYCODE_6_PAD)
	PORT_DIPNAME( 0x08, 0x08, "SYSTEM" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static MACHINE_START( 30test )
{
	namco_30test_state *state = machine.driver_data<namco_30test_state>();

	state->save_item(NAME(state->m_mux_data));
	state->save_item(NAME(state->m_oki_bank));
}

static MACHINE_RESET( 30test )
{

}

static const hc11_config namco_30test_config =
{
	0,	   //has extended internal I/O
	768,   //internal RAM size
	0x00   //registers are at 0-0x100
};


static MACHINE_CONFIG_START( 30test, namco_30test_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", MC68HC11,MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(namco_30test_map)
	MCFG_CPU_IO_MAP(namco_30test_io)
	MCFG_CPU_CONFIG(namco_30test_config)

	MCFG_MACHINE_START(30test)
	MCFG_MACHINE_RESET(30test)

	/* video hardware */
	/* TODO: NOT raster! */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE(30test)

//  MCFG_PALETTE_INIT(30test)
	MCFG_PALETTE_LENGTH(2)

	MCFG_VIDEO_START(30test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( 30test )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt1-mpr0b.8p",   0x0000, 0x10000, CRC(455043d5) SHA1(46b15324d193ee621beabce92c0dc493b608b8dd) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "tt1-voi0.7p",   0x0000, 0x80000, CRC(b4fc5921) SHA1(92a88d5adb50dae48715847f12e88a35e37ef78c) )
ROM_END

GAMEL( 1997, 30test,  0,   30test,  30test,  0, ROT0, "Namco", "30 Test (Remake)", GAME_SUPPORTS_SAVE, layout_30test )
