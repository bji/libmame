/*******************************************************************************************

Notes:
-To init chsuper3, just soft-reset and keep pressed both service keys (9 & 0)

TODO:
-sound;
-inputs are grossly mapped;
-lamps;

*******************************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "sound/dac.h"
#include "machine/nvram.h"


class chsuper_state : public driver_device
{
public:
	chsuper_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_tilexor;
	struct { int r,g,b,offs,offs_internal; } m_pal;
};



static VIDEO_START(chsuper)
{
}

static SCREEN_UPDATE(chsuper)
{
	//chsuper_state *state = screen->machine().driver_data<chsuper_state>();
	const gfx_element *gfx = screen->machine().gfx[0];
	UINT8 *vram = screen->machine().region("vram")->base();
	int count = 0x0000;
	int y,x;

	for (y=0;y<64;y++)
	{
		for (x=0;x<128;x++)
		{
			int tile = ((vram[count+1]<<8) | vram[count]) & 0xffff;

			//tile ^=state->m_tilexor;

			drawgfx_opaque(bitmap,cliprect,gfx,tile,0,0,0,x*4,y*8);
			count+=2;
		}
	}

	return 0;
}

static WRITE8_HANDLER( paletteram_io_w )
{
	chsuper_state *state = space->machine().driver_data<chsuper_state>();
	switch(offset)
	{
		case 0:
			state->m_pal.offs = data;
			break;
		case 2:
			state->m_pal.offs_internal = 0;
			break;
		case 1:
			switch(state->m_pal.offs_internal)
			{
				case 0:
					state->m_pal.r = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					state->m_pal.offs_internal++;
					break;
				case 1:
					state->m_pal.g = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					state->m_pal.offs_internal++;
					break;
				case 2:
					state->m_pal.b = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					palette_set_color(space->machine(), state->m_pal.offs, MAKE_RGB(state->m_pal.r, state->m_pal.g, state->m_pal.b));
					state->m_pal.offs_internal = 0;
					state->m_pal.offs++;
					break;
			}

			break;
	}
}

static READ8_HANDLER( ff_r )
{
	return 0xff;
}

static WRITE8_HANDLER( chsuper_vram_w )
{
	UINT8 *vram = space->machine().region("vram")->base();

	vram[offset] = data;
}

static ADDRESS_MAP_START( chsuper_prg_map, AS_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x0efff) AM_ROM
	AM_RANGE(0x00000, 0x01fff) AM_WRITE( chsuper_vram_w )
	AM_RANGE(0x0f000, 0x0ffff) AM_RAM AM_REGION("maincpu", 0xf000)
	AM_RANGE(0xfb000, 0xfbfff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

//  AM_RANGE(0xaff8, 0xaff8) AM_DEVWRITE_MODERN("oki", okim6295_device, write)

static ADDRESS_MAP_START( chsuper_portmap, AS_IO, 8 )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // Z180 internal regs
	AM_RANGE( 0x00e8, 0x00e8 ) AM_READ_PORT("IN0")
	AM_RANGE( 0x00e9, 0x00e9 ) AM_READ_PORT("IN1")
	AM_RANGE( 0x00ea, 0x00ea ) AM_READ_PORT("DSW0")
	AM_RANGE( 0x00ed, 0x00ef ) AM_WRITENOP //lamps
	AM_RANGE( 0x00fc, 0x00fe ) AM_WRITE( paletteram_io_w )
	AM_RANGE( 0x8300, 0x8300 ) AM_READ( ff_r ) //probably data for the dac
	AM_RANGE( 0xff20, 0xff3f ) AM_DEVWRITE("dac", dac_w) // unk writes
ADDRESS_MAP_END

static INPUT_PORTS_START( chsuper )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Cancel All")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Analyzer")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Credit clear" ) //hopper?
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
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

	PORT_START("DSW1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	4,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7},
	{ 2*8,3*8,0*8,1*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32},
	8*32
};

static GFXDECODE_START( chsuper )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,   0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( chsuper, chsuper_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz / 2)	/* HD64180RP8, 8 MHz? */
	MCFG_CPU_PROGRAM_MAP(chsuper_prg_map)
	MCFG_CPU_IO_MAP(chsuper_portmap)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0, 30*8-1)
	MCFG_SCREEN_UPDATE(chsuper)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_GFXDECODE(chsuper)
	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_START(chsuper)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/*  ROM Regions definition
 */

ROM_START( chsuper3 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "c.bin",  0x0000, 0x80000, CRC(e987ed1f) SHA1(8d1ee01914356714c7d1f8437d98b41a707a174a) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "a.bin",  0x00000, 0x80000, CRC(ace8b591) SHA1(e9ba5efebdc9b655056ed8b2621f062f50e0528f) )
	ROM_LOAD( "b.bin",  0x80000, 0x80000, CRC(5f58c722) SHA1(d339ae27af010b058eae9084fba85fb2fbed3952) )

	ROM_REGION( 0x10000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x80000, "adpcm", 0 )
	ROM_COPY( "maincpu", 0x10000, 0x00000, 0x70000 )
ROM_END

ROM_START( chsuper2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "chsuper2-c.bin",  0x0000, 0x80000, CRC(cbf59e69) SHA1(68e4b167fdf9103fd748cff401f4fe7c1d214552) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "chsuper2-a.bin",  0x00000, 0x80000, CRC(7caa8ebe) SHA1(440306a208ec8afd570b15f05b5dc542acc98510) )
	ROM_LOAD( "chsuper2-b.bin",  0x80000, 0x80000, CRC(7bb463d7) SHA1(fb3842ba53e545fa47574c91df7281a9cb417395) )

	ROM_REGION( 0x10000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x80000, "adpcm", 0 )
	ROM_COPY( "maincpu", 0x10000, 0x00000, 0x70000 )
ROM_END

ROM_START( chmpnum )
	ROM_REGION( 0x80000, "maincpu", 0 ) // code + samples
	ROM_LOAD( "3.ic11", 0x00000, 0x80000, CRC(46aa2ce7) SHA1(036d67a26c890c4dc26599bfcd2c67f12e30fb52) )

	ROM_REGION( 0x010000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "1.ic18", 0x00000, 0x80000, CRC(8e202eaa) SHA1(156b498873111e5890c00d447201ba4bcbe6e633) )
	ROM_LOAD( "2.ic19", 0x80000, 0x80000, CRC(dc0790b0) SHA1(4550f85e609338635a3987f7832517ed1d6388d4) )

	ROM_REGION( 0x80000, "adpcm", 0 )
	ROM_COPY( "maincpu", 0x10000, 0x00000, 0x70000 )
ROM_END

static DRIVER_INIT( chsuper2 )
{
	chsuper_state *state = machine.driver_data<chsuper_state>();
	UINT8 *buffer;
	UINT8 *rom = machine.region("gfx1")->base();
	int i;

	state->m_tilexor = 0x7f00;

	buffer = auto_alloc_array(machine, UINT8, 0x100000);

	for (i=0;i<0x100000;i++)
	{
		int j;

		j = i ^ (state->m_tilexor << 5);

		buffer[j] = rom[i];
	}

	memcpy(rom,buffer,0x100000);

	state->m_tilexor = 0x0000;
}

static DRIVER_INIT( chsuper3 )
{
	chsuper_state *state = machine.driver_data<chsuper_state>();
	UINT8 *buffer;
	UINT8 *rom = machine.region("gfx1")->base();
	int i;

	state->m_tilexor = 0x0e00;

	buffer = auto_alloc_array(machine, UINT8, 0x100000);

	for (i=0;i<0x100000;i++)
	{
		int j;

		j = i ^ (state->m_tilexor << 5);

		buffer[j] = rom[i];
	}

	memcpy(rom,buffer,0x100000);

	state->m_tilexor = 0x0000;
}

static DRIVER_INIT( chmpnum )
{
	chsuper_state *state = machine.driver_data<chsuper_state>();
	UINT8 *buffer;
	UINT8 *rom = machine.region("gfx1")->base();
	int i;

	state->m_tilexor = 0x1800;

	buffer = auto_alloc_array(machine, UINT8, 0x100000);

	for (i=0;i<0x100000;i++)
	{
		int j;

		j = i ^ (state->m_tilexor << 5);

		j = BITSWAP24(j,23,22,21,20,19,18,17,13, 15,14,16,12, 11,10,9,8, 7,6,5,4, 3,2,1,0);
		j = BITSWAP24(j,23,22,21,20,19,18,17,14, 15,16,13,12, 11,10,9,8, 7,6,5,4, 3,2,1,0);
		j = BITSWAP24(j,23,22,21,20,19,18,17,15, 16,14,13,12, 11,10,9,8, 7,6,5,4, 3,2,1,0);

		buffer[j] = rom[i];
	}

	memcpy(rom,buffer,0x100000);

	state->m_tilexor = 0x0000;
}


GAME( 1999, chsuper3, 0,        chsuper, chsuper,  chsuper3, ROT0, "<unknown>",    "Champion Super 3 (V0.35)", GAME_IMPERFECT_SOUND ) //24/02/99
GAME( 1999, chsuper2, chsuper3, chsuper, chsuper,  chsuper2, ROT0, "<unknown>",    "Champion Super 2 (V0.13)", GAME_IMPERFECT_SOUND ) //26/01/99
GAME( 1999, chmpnum,  chsuper3, chsuper, chsuper,  chmpnum,  ROT0, "<unknown>",    "Champion Number (V0.74)",  GAME_IMPERFECT_SOUND ) //10/11/99
