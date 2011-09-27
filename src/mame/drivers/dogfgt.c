/***************************************************************************

Acrobatic Dog-Fight / Batten O'hara no Sucha-Raka Kuuchuu Sen
(c) 1984 Technos Japan

driver by Nicola Salmoria

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "deprecat.h"
#include "includes/dogfgt.h"
#include "sound/ay8910.h"


static READ8_HANDLER( sharedram_r )
{
	dogfgt_state *state = space->machine().driver_data<dogfgt_state>();
	return state->m_sharedram[offset];
}

static WRITE8_HANDLER( sharedram_w )
{
	dogfgt_state *state = space->machine().driver_data<dogfgt_state>();
	state->m_sharedram[offset] = data;
}


static WRITE8_HANDLER( subirqtrigger_w )
{
	dogfgt_state *state = space->machine().driver_data<dogfgt_state>();
	/* bit 0 used but unknown */
	if (data & 0x04)
		device_set_input_line(state->m_subcpu, 0, ASSERT_LINE);
}

static WRITE8_HANDLER( sub_irqack_w )
{
	dogfgt_state *state = space->machine().driver_data<dogfgt_state>();
	device_set_input_line(state->m_subcpu, 0, CLEAR_LINE);
}

static WRITE8_HANDLER( dogfgt_soundlatch_w )
{
	dogfgt_state *state = space->machine().driver_data<dogfgt_state>();
	state->m_soundlatch = data;
}

static WRITE8_HANDLER( dogfgt_soundcontrol_w )
{
	dogfgt_state *state = space->machine().driver_data<dogfgt_state>();

	/* bit 5 goes to 8910 #0 BDIR pin  */
	if ((state->m_last_snd_ctrl & 0x20) == 0x20 && (data & 0x20) == 0x00)
		ay8910_data_address_w(space->machine().device("ay1"), state->m_last_snd_ctrl >> 4, state->m_soundlatch);

	/* bit 7 goes to 8910 #1 BDIR pin  */
	if ((state->m_last_snd_ctrl & 0x80) == 0x80 && (data & 0x80) == 0x00)
		ay8910_data_address_w(space->machine().device("ay2"), state->m_last_snd_ctrl >> 6, state->m_soundlatch);

	state->m_last_snd_ctrl = data;
}



static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_READWRITE(sharedram_r, sharedram_w) AM_BASE_MEMBER(dogfgt_state, m_sharedram)
	AM_RANGE(0x0f80, 0x0fdf) AM_WRITEONLY AM_BASE_SIZE_MEMBER(dogfgt_state, m_spriteram, m_spriteram_size)
	AM_RANGE(0x1000, 0x17ff) AM_WRITE(dogfgt_bgvideoram_w) AM_BASE_MEMBER(dogfgt_state, m_bgvideoram)
	AM_RANGE(0x1800, 0x1800) AM_READ_PORT("P1")
	AM_RANGE(0x1800, 0x1800) AM_WRITE(dogfgt_1800_w)	/* text color, flip screen & coin counters */
	AM_RANGE(0x1810, 0x1810) AM_READ_PORT("P2")
	AM_RANGE(0x1810, 0x1810) AM_WRITE(subirqtrigger_w)
	AM_RANGE(0x1820, 0x1820) AM_READ_PORT("DSW1")
	AM_RANGE(0x1820, 0x1823) AM_WRITE(dogfgt_scroll_w)
	AM_RANGE(0x1824, 0x1824) AM_WRITE(dogfgt_plane_select_w)
	AM_RANGE(0x1830, 0x1830) AM_READ_PORT("DSW2")
	AM_RANGE(0x1830, 0x1830) AM_WRITE(dogfgt_soundlatch_w)
	AM_RANGE(0x1840, 0x1840) AM_WRITE(dogfgt_soundcontrol_w)
	AM_RANGE(0x1870, 0x187f) AM_WRITE(paletteram_BBGGGRRR_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(dogfgt_bitmapram_r, dogfgt_bitmapram_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x2000, 0x27ff) AM_READWRITE(sharedram_r, sharedram_w)
	AM_RANGE(0x4000, 0x4000) AM_WRITE(sub_irqack_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



static INPUT_PORTS_START( dogfgt )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) /* Manual states dips 5 & 6 are "Unused" */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, "Upright 1 Player" )
	PORT_DIPSETTING(    0x80, "Upright 2 Players" )
//  PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )     // "Cocktail 1 Player" - IMPOSSIBLE !
	PORT_DIPSETTING(    0xc0, DEF_STR( Cocktail ) )		// "Cocktail 2 Players"


/*  Manual shows:

    Dip #7  TV-Screen
        OFF Table type
        ON  Up-right type use
    Dip #8  Control Panel
        OFF Table type use
        ON  Up-right use

There is a side note for these two: "Change both together"
*/

	PORT_START("DSW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

/*  Manual shows:

    Dip #8  TV-Screen
        OFF Normal
        ON  Invert
*/

INPUT_PORTS_END



static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0,
			7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*16
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
			16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*16
};

static GFXDECODE_START( dogfgt )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   16, 4 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,  0, 2 )
GFXDECODE_END



static MACHINE_START( dogfgt )
{
	dogfgt_state *state = machine.driver_data<dogfgt_state>();

	state->m_subcpu = machine.device("sub");

	state->save_item(NAME(state->m_bm_plane));
	state->save_item(NAME(state->m_lastflip));
	state->save_item(NAME(state->m_pixcolor));
	state->save_item(NAME(state->m_lastpixcolor));
	state->save_item(NAME(state->m_soundlatch));
	state->save_item(NAME(state->m_last_snd_ctrl));

	state->save_item(NAME(state->m_scroll));
}

static MACHINE_RESET( dogfgt )
{
	dogfgt_state *state = machine.driver_data<dogfgt_state>();
	int i;

	state->m_bm_plane = 0;
	state->m_lastflip = 0;
	state->m_pixcolor = 0;
	state->m_lastpixcolor = 0;
	state->m_soundlatch = 0;
	state->m_last_snd_ctrl = 0;

	for (i = 0; i < 3; i++)
		state->m_scroll[i] = 0;
}


static MACHINE_CONFIG_START( dogfgt, dogfgt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 1500000)	/* 1.5 MHz ???? */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_HACK(irq0_line_hold,16)	/* ? controls music tempo */

	MCFG_CPU_ADD("sub", M6502, 1500000)	/* 1.5 MHz ???? */
	MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_START(dogfgt)
	MCFG_MACHINE_RESET(dogfgt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE(dogfgt)

	MCFG_GFXDECODE(dogfgt)
	MCFG_PALETTE_LENGTH(16+64)

	MCFG_PALETTE_INIT(dogfgt)
	MCFG_VIDEO_START(dogfgt)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("ay2", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END



ROM_START( dogfgt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bx00.52",     0x8000, 0x2000, CRC(e602a21c) SHA1(12c659608d04ffc35ea9c1c0e1e82a8aab9f24bb) )
	ROM_LOAD( "bx01.37",     0xa000, 0x2000, CRC(4921c4fb) SHA1(995b9ac123110c5c6d34d90f706ed72afdeaa231) )
	ROM_LOAD( "bx02-5.36",   0xc000, 0x2000, CRC(d11b50c3) SHA1(99dbbc85e8ff66eadc48a9f65f800676b10e35e4) )
	ROM_LOAD( "bx03-5.22",   0xe000, 0x2000, CRC(0e4813fb) SHA1(afcbd17029bc3c2de83c15cc941fe8f2ad062a5d) )

	ROM_REGION( 0x10000, "sub", 0 )	/* 64k for audio code */
	ROM_LOAD( "bx04.117",    0x8000, 0x2000, CRC(f8945f9d) SHA1(a0782a5007dc5efc302c4fd61827e1b68475e7ab) )
	ROM_LOAD( "bx05.118",    0xa000, 0x2000, CRC(3ade57ad) SHA1(cc0a35257c00c463614a6718a24cc6dee75c2e5d) )
	ROM_LOAD( "bx06.119",    0xc000, 0x2000, CRC(4a3b34cf) SHA1(f2e0bf9923a288b8137840f46fd90a23010f8018) )
	ROM_LOAD( "bx07.120",    0xe000, 0x2000, CRC(ae21f907) SHA1(6374619f930a1ea8a222d95435158a0847450aac) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "bx17.56",     0x0000, 0x2000, CRC(fd3245d7) SHA1(4cbe887e0988382a38b7376c41ec1406fa66d18d) )
	ROM_LOAD( "bx18.57",     0x2000, 0x2000, CRC(03a5ef06) SHA1(44931222c722dec91516577d732478d01734efb3) )
	ROM_LOAD( "bx19.58",     0x4000, 0x2000, CRC(f62a16f4) SHA1(e7f2891aba1cf708d765229b76e36ee9c91596ea) )

	ROM_REGION( 0x12000, "gfx2", 0 )
	ROM_LOAD( "bx08.128",    0x00000, 0x2000, CRC(8bf41b27) SHA1(346da090ba216c182530df40bd8d0af96c7a705b) )
	ROM_LOAD( "bx09.127",    0x02000, 0x2000, CRC(c3ea6509) SHA1(4ebe36f1cc59f44808d2975fe3d30ee089535bbc) )
	ROM_LOAD( "bx10.126",    0x04000, 0x2000, CRC(474a1c64) SHA1(14b2967d23903175e8e0b9340fcd11f7ce9d15dd) )
	ROM_LOAD( "bx11.125",    0x06000, 0x2000, CRC(ba67e382) SHA1(7f553b9121014d111ac9347132cae3a9f4702b16) )
	ROM_LOAD( "bx12.124",    0x08000, 0x2000, CRC(102c0e1c) SHA1(08b665131c799605711226c4907ec596c7962f70) )
	ROM_LOAD( "bx13.123",    0x0a000, 0x2000, CRC(ca47de34) SHA1(208fb15e5e6606bdcd26cfeb407979d0ada8154d) )
	ROM_LOAD( "bx14.122",    0x0c000, 0x2000, CRC(51b95bb4) SHA1(ed368eba64ccb035b13a0ba8bde8956b73c989c1) )
	ROM_LOAD( "bx15.121",    0x0e000, 0x2000, CRC(cf45d025) SHA1(e6974138ce8a796c77d220b2ab0d931cbd8e7280) )
	ROM_LOAD( "bx16.120",    0x10000, 0x2000, CRC(d1933837) SHA1(7fcb1cc66235f70db47b1e174eaca41fa8fcbb41) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bx20.52",     0x0000, 0x0020, CRC(4e475f05) SHA1(3e077c7d2471c29dccdfbba9934024739a8d0586) )
	ROM_LOAD( "bx21.64",     0x0020, 0x0020, CRC(5de4319f) SHA1(f70e116b80627d3eccc27c1964b08a0c8cdfff44) )
ROM_END

ROM_START( dogfgtu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bx00.52",     0x8000, 0x2000, CRC(e602a21c) SHA1(12c659608d04ffc35ea9c1c0e1e82a8aab9f24bb) )
	ROM_LOAD( "bx01-6.37",   0xa000, 0x2000, CRC(8bb66399) SHA1(3f7b7b6b5c5363c35d19b797ae9016bab63e3b96) )
	ROM_LOAD( "bx02-7.36",   0xc000, 0x2000, CRC(afaf10e6) SHA1(67b1e0722ae0e8110acc44bc582b308e86743d60) )
	ROM_LOAD( "bx03-6.33",   0xe000, 0x2000, CRC(51b20e8b) SHA1(0247345b3f9736e9140faf765cb33f97660f0ddd) )

	ROM_REGION( 0x10000, "sub", 0 )	/* 64k for audio code */
	ROM_LOAD( "bx04-7.117",  0x8000, 0x2000, CRC(c4c2183b) SHA1(27cf40d6f03b078c5cc4497f393309bf33dea9dc) )
	ROM_LOAD( "bx05-7.118",  0xa000, 0x2000, CRC(d9a705ab) SHA1(7a49769d6c32e3d9840e4b24e3cbcd84a075d36d) )
	ROM_LOAD( "bx06.119",    0xc000, 0x2000, CRC(4a3b34cf) SHA1(f2e0bf9923a288b8137840f46fd90a23010f8018) )
	ROM_LOAD( "bx07-7.120",  0xe000, 0x2000, CRC(868df3dd) SHA1(482a461bce5555ac02b5ffa7723ee671139a2d54) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "bx17.56",     0x0000, 0x2000, CRC(fd3245d7) SHA1(4cbe887e0988382a38b7376c41ec1406fa66d18d) )
	ROM_LOAD( "bx18.57",     0x2000, 0x2000, CRC(03a5ef06) SHA1(44931222c722dec91516577d732478d01734efb3) )
	ROM_LOAD( "bx19.58",     0x4000, 0x2000, CRC(f62a16f4) SHA1(e7f2891aba1cf708d765229b76e36ee9c91596ea) )

	ROM_REGION( 0x12000, "gfx2", 0 )
	ROM_LOAD( "bx08.128",    0x00000, 0x2000, CRC(8bf41b27) SHA1(346da090ba216c182530df40bd8d0af96c7a705b) )
	ROM_LOAD( "bx09.127",    0x02000, 0x2000, CRC(c3ea6509) SHA1(4ebe36f1cc59f44808d2975fe3d30ee089535bbc) )
	ROM_LOAD( "bx10.126",    0x04000, 0x2000, CRC(474a1c64) SHA1(14b2967d23903175e8e0b9340fcd11f7ce9d15dd) )
	ROM_LOAD( "bx11.125",    0x06000, 0x2000, CRC(ba67e382) SHA1(7f553b9121014d111ac9347132cae3a9f4702b16) )
	ROM_LOAD( "bx12.124",    0x08000, 0x2000, CRC(102c0e1c) SHA1(08b665131c799605711226c4907ec596c7962f70) )
	ROM_LOAD( "bx13.123",    0x0a000, 0x2000, CRC(ca47de34) SHA1(208fb15e5e6606bdcd26cfeb407979d0ada8154d) )
	ROM_LOAD( "bx14.122",    0x0c000, 0x2000, CRC(51b95bb4) SHA1(ed368eba64ccb035b13a0ba8bde8956b73c989c1) )
	ROM_LOAD( "bx15.121",    0x0e000, 0x2000, CRC(cf45d025) SHA1(e6974138ce8a796c77d220b2ab0d931cbd8e7280) )
	ROM_LOAD( "bx16.120",    0x10000, 0x2000, CRC(d1933837) SHA1(7fcb1cc66235f70db47b1e174eaca41fa8fcbb41) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bx20.52",     0x0000, 0x0020, CRC(4e475f05) SHA1(3e077c7d2471c29dccdfbba9934024739a8d0586) )
	ROM_LOAD( "bx21.64",     0x0020, 0x0020, CRC(5de4319f) SHA1(f70e116b80627d3eccc27c1964b08a0c8cdfff44) )
ROM_END

ROM_START( dogfgtj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bx00.52",     0x8000, 0x2000, CRC(e602a21c) SHA1(12c659608d04ffc35ea9c1c0e1e82a8aab9f24bb) )
	ROM_LOAD( "bx01.37",     0xa000, 0x2000, CRC(4921c4fb) SHA1(995b9ac123110c5c6d34d90f706ed72afdeaa231) )
	ROM_LOAD( "bx02.36",     0xc000, 0x2000, CRC(91f1b9b3) SHA1(dd939538abf615d3a0271fd561038acc6a2a616d) )
	ROM_LOAD( "bx03.22",     0xe000, 0x2000, CRC(959ebf93) SHA1(de79dd44c68a232278b8d251e39c0ad35d160595) )

	ROM_REGION( 0x10000, "sub", 0 )	/* 64k for audio code */
	ROM_LOAD( "bx04.117",    0x8000, 0x2000, CRC(f8945f9d) SHA1(a0782a5007dc5efc302c4fd61827e1b68475e7ab) )
	ROM_LOAD( "bx05.118",    0xa000, 0x2000, CRC(3ade57ad) SHA1(cc0a35257c00c463614a6718a24cc6dee75c2e5d) )
	ROM_LOAD( "bx06.119",    0xc000, 0x2000, CRC(4a3b34cf) SHA1(f2e0bf9923a288b8137840f46fd90a23010f8018) )
	ROM_LOAD( "bx07.120",    0xe000, 0x2000, CRC(ae21f907) SHA1(6374619f930a1ea8a222d95435158a0847450aac) )

	ROM_REGION( 0x06000, "gfx1", 0 )
	ROM_LOAD( "bx17.56",     0x0000, 0x2000, CRC(fd3245d7) SHA1(4cbe887e0988382a38b7376c41ec1406fa66d18d) )
	ROM_LOAD( "bx18.57",     0x2000, 0x2000, CRC(03a5ef06) SHA1(44931222c722dec91516577d732478d01734efb3) )
	ROM_LOAD( "bx19.58",     0x4000, 0x2000, CRC(f62a16f4) SHA1(e7f2891aba1cf708d765229b76e36ee9c91596ea) )

	ROM_REGION( 0x12000, "gfx2", 0 )
	ROM_LOAD( "bx08.128",    0x00000, 0x2000, CRC(8bf41b27) SHA1(346da090ba216c182530df40bd8d0af96c7a705b) )
	ROM_LOAD( "bx09.127",    0x02000, 0x2000, CRC(c3ea6509) SHA1(4ebe36f1cc59f44808d2975fe3d30ee089535bbc) )
	ROM_LOAD( "bx10.126",    0x04000, 0x2000, CRC(474a1c64) SHA1(14b2967d23903175e8e0b9340fcd11f7ce9d15dd) )
	ROM_LOAD( "bx11.125",    0x06000, 0x2000, CRC(ba67e382) SHA1(7f553b9121014d111ac9347132cae3a9f4702b16) )
	ROM_LOAD( "bx12.124",    0x08000, 0x2000, CRC(102c0e1c) SHA1(08b665131c799605711226c4907ec596c7962f70) )
	ROM_LOAD( "bx13.123",    0x0a000, 0x2000, CRC(ca47de34) SHA1(208fb15e5e6606bdcd26cfeb407979d0ada8154d) )
	ROM_LOAD( "bx14.122",    0x0c000, 0x2000, CRC(51b95bb4) SHA1(ed368eba64ccb035b13a0ba8bde8956b73c989c1) )
	ROM_LOAD( "bx15.121",    0x0e000, 0x2000, CRC(cf45d025) SHA1(e6974138ce8a796c77d220b2ab0d931cbd8e7280) )
	ROM_LOAD( "bx16.120",    0x10000, 0x2000, CRC(d1933837) SHA1(7fcb1cc66235f70db47b1e174eaca41fa8fcbb41) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bx20.52",     0x0000, 0x0020, CRC(4e475f05) SHA1(3e077c7d2471c29dccdfbba9934024739a8d0586) )
	ROM_LOAD( "bx21.64",     0x0020, 0x0020, CRC(5de4319f) SHA1(f70e116b80627d3eccc27c1964b08a0c8cdfff44) )
ROM_END



GAME( 1984, dogfgt,  0,      dogfgt, dogfgt, 0, ROT0, "Technos Japan",       "Acrobatic Dog-Fight", GAME_SUPPORTS_SAVE )
GAME( 1985, dogfgtu, dogfgt, dogfgt, dogfgt, 0, ROT0, "Data East USA, Inc.", "Acrobatic Dog-Fight (USA)", GAME_SUPPORTS_SAVE )
GAME( 1984, dogfgtj, dogfgt, dogfgt, dogfgt, 0, ROT0, "Technos Japan",       "Dog-Fight (Japan)", GAME_SUPPORTS_SAVE )
