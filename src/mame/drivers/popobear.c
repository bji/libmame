/* Popo Bear - BMC-A00211
(c) 2000 - Bao Ma Technology Co., LTD

|-----------------------------------------|
| DIP2 DIP4  UM3567(YM2413)               |J
| DIP1 DIP3                               |A
|           TA-A-901                      |M
| EN-A-701  EN-A-801  U6295(OKI)          |M
| EN-A-501  EN-A-601                      |A
| EN-A-301  EN-A-401                      |
|                                         |C
|                   AIA90610              |O
|                   BMC-68pin  AIA90423   |N
|                   plcc (68k) BMC-160pin |N
|                                         |E
|                                    OSC  |C
|                                 42.000  |T
|-----------------------------------------|

1 - BMC AIA90423 - 160-Pin ASIC, FGPA, Video?
1 - BMC AIA90610 - 68 Pin CPU (Likely 16 MHz, 68-lead plastic LCC 68000)
1 - UM3567 (YM2413) Sound
1 - U6295 (OKI6295) Sound
1 - 42.000MHz XTAL
4 - 8 Position DIP switches

JAMMA CONNECTOR
Component Side   A   B   Solder Side
           GND   1   1   GND
           GND   2   2   GND
           +5v   3   3   +5v
           +5v   4   4   +5v
                 5   5
          +12v   6   6   +12v
                 7   7
    Coin Meter   8   8
                 9   9
       Speaker  10   10  GND
                11   11
           Red  12   12  Green
          Blue  13   13  Syn
           GND  14   14
          Test  15   15
         Coin1  16   16  Coin2
      1P Start  17   17  2P Start
         1P Up  18   18  2P Up
       1P Down  19   19  2P Down
       1P Left  20   20  2P Left
      1P Right  21   21  2P Right
          1P A  22   22  2P A
          1P B  23   23  2P B
          1P C  24   24  2P C
                25   25
                26   26
           GND  27   27  GND
           GND  28   28  GND
*/

// looks like some kind of blitter
// IGS-like?

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"

class popobear_state : public driver_device
{
public:
	popobear_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }


	UINT16 *popobear_vram;
	required_device<cpu_device> m_maincpu;
};

/* stub read handlers */

static READ16_HANDLER( popo_480001_r ) { static int i = 0x00;i ^=0xff;return i;}
static READ16_HANDLER( popo_48001c_r ) { static int i = 0x00;i ^=0xff;return i;}
static READ16_HANDLER( popo_480020_r ) { static int i = 0x00;i ^=0xff;return i;}
static READ16_HANDLER( popo_48003a_r ) { static int i = 0x00;i ^=0xff;return i;}

static READ16_HANDLER( popo_500000_r ) { static int i = 0x00;i ^=0xff;return i;}
static READ16_HANDLER( popo_520000_r ) { static int i = 0x00;i ^=0xff;return i;}

static READ16_HANDLER( popo_620000_r ) { static int i = 0x00;i ^=0xff;return i;}

/* stub write handlers */

static WRITE16_HANDLER( popo_480001_w ) { }
static WRITE16_HANDLER( popo_480018_w ) { }
static WRITE16_HANDLER( popo_48001a_w ) { }
static WRITE16_HANDLER( popo_48001c_w ) { }
static WRITE16_HANDLER( popo_480020_w ) { }
static WRITE16_HANDLER( popo_480028_w ) { }
static WRITE16_HANDLER( popo_48002c_w ) { }
static WRITE16_HANDLER( popo_480030_w ) { }
static WRITE16_HANDLER( popo_48003a_w ) { }

static WRITE16_HANDLER( popo_550000_w ) { }
static WRITE16_HANDLER( popo_550002_w ) { }

static WRITE16_HANDLER( popo_600000_w ) { }
static WRITE16_HANDLER( popo_620000_w ) { }


static ADDRESS_MAP_START( popobear_mem, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x210000, 0x21ffff) AM_RAM
	AM_RANGE(0x3E0000, 0x3EFFFF) AM_RAM  AM_BASE_MEMBER(popobear_state, popobear_vram)

	/* Blitter stuff? */
	AM_RANGE(0x480000, 0x480001) AM_READ(popo_480001_r) AM_WRITE(popo_480001_w)
	AM_RANGE(0x480018, 0x480019) AM_WRITE(popo_480018_w)
	AM_RANGE(0x48001a, 0x48001b) AM_WRITE(popo_48001a_w)
	AM_RANGE(0x48001c, 0x48001d) AM_READ(popo_48001c_r) AM_WRITE(popo_48001c_w)
	AM_RANGE(0x480020, 0x480021) AM_READ(popo_480020_r) AM_WRITE(popo_480020_w)
	AM_RANGE(0x480028, 0x480029) AM_WRITE(popo_480028_w)
	AM_RANGE(0x48002c, 0x48002d) AM_WRITE(popo_48002c_w)
	AM_RANGE(0x480030, 0x480031) AM_WRITE(popo_480030_w)
	AM_RANGE(0x48003a, 0x48003b) AM_READ(popo_48003a_r) AM_WRITE(popo_48003a_w)

	AM_RANGE(0x480400, 0x48041f) AM_RAM AM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram) // looks palette like at least

	/* I/O maybe? */
	AM_RANGE(0x500000, 0x500001) AM_READ(popo_500000_r)
	AM_RANGE(0x520000, 0x520001) AM_READ(popo_520000_r)
	AM_RANGE(0x550000, 0x550001) AM_WRITE(popo_550000_w)
	AM_RANGE(0x550002, 0x550003) AM_WRITE(popo_550002_w)

	/* these could be where the OKI hooks up? */
	AM_RANGE(0x600000, 0x600001) AM_WRITE(popo_600000_w)
	AM_RANGE(0x620000, 0x620001) AM_READ(popo_620000_r) AM_WRITE(popo_620000_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( popobear )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x00, "Coin_A" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0e, "Freeplay" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hard ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Arrow" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW2:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW2:5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW2:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW2:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW2:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "DSW3:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW3:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW3:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW3:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW3:5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW3:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW3:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW3:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, "DSW4:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW4:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW4:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW4:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW4:5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW4:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW4:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW4:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	4,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 24, 16, 8, 0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

// gfx don't seem to be tiles..
static GFXDECODE_START( popobear )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,  0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  0, 1 )
GFXDECODE_END


SCREEN_UPDATE_IND16( popobear )
{
	popobear_state *state = screen.machine().driver_data<popobear_state>();
	bitmap.fill(0, cliprect);

	UINT16* popobear_vram = state->popobear_vram;
	int count = 0;
	for (int y=0;y<256;y++)
	{
		for (int x=0;x<128;x++)
		{
			UINT8 dat;
			dat = (popobear_vram[count]&0xf000)>>12;
			bitmap.pix16(y, (x*4)+0) =dat;

			dat = (popobear_vram[count]&0x0f00)>>8;
			bitmap.pix16(y, (x*4)+1) =dat;

			dat = (popobear_vram[count]&0x00f0)>>4;
			bitmap.pix16(y, (x*4)+2) =dat;

			dat = (popobear_vram[count]&0x000f)>>0;
			bitmap.pix16(y, (x*4)+3) =dat;

			count++;
		}
	}

	return 0;
}

VIDEO_START(popobear)
{

}

static TIMER_DEVICE_CALLBACK( popobear_irq )
{
	popobear_state *state = timer.machine().driver_data<popobear_state>();
	int scanline = param;

	if(scanline == 240)
		device_set_input_line(state->m_maincpu, 2, HOLD_LINE);

	if(scanline == 128)
		device_set_input_line(state->m_maincpu, 3, HOLD_LINE);

	if(scanline == 32)
		device_set_input_line(state->m_maincpu, 5, HOLD_LINE);
}

static MACHINE_CONFIG_START( popobear, popobear_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_42MHz/4)  // XTAL CORRECT, DIVISOR GUESSED
	MCFG_CPU_PROGRAM_MAP(popobear_mem)
	// levels 2,3,5 look interesting
	//MCFG_CPU_VBLANK_INT("screen",irq3_line_hold)
	MCFG_TIMER_ADD_SCANLINE("scantimer", popobear_irq, "screen", 0, 1)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_STATIC(popobear)

	MCFG_GFXDECODE(popobear)

	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_PALETTE_LENGTH(256)

	MCFG_VIDEO_START(popobear)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ym2413", YM2413, XTAL_42MHz/10)  // XTAL CORRECT, DIVISOR GUESSED
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)

	MCFG_OKIM6295_ADD("oki", XTAL_42MHz/10/4, OKIM6295_PIN7_LOW)  // XTAL CORRECT, DIVISOR GUESSED
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END


ROM_START( popobear )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "popobear_en-a-301_1.6.u3", 0x000001, 0x20000, CRC(b934adf6) SHA1(93431c7a19af812b549aad35cc1176a81805ffab) )
	ROM_LOAD16_BYTE( "popobear_en-a-401_1.6.u4", 0x000000, 0x20000, CRC(0568af9c) SHA1(920531dbc4bbde2d1db062bd5c48b97dd50b7185) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE(	"popobear_en-a-501.u5", 0x000000, 0x100000, CRC(185901a9) SHA1(7ff82b5751645df53435eaa66edce589684cc5c7) )
	ROM_LOAD16_BYTE(	"popobear_en-a-601.u6", 0x000001, 0x100000, CRC(84fa9f3f) SHA1(34dd7873f88b0dae5fb81fe84e82d2b6b49f7332) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE(	"popobear_en-a-701.u7", 0x000000, 0x100000, CRC(45eba6d0) SHA1(0278602ed57ac45040619d590e6cc85e2cfeed31) )
	ROM_LOAD16_BYTE(	"popobear_en-a-801.u8", 0x000001, 0x100000, CRC(2760f2e6) SHA1(58af59f486c9df930f7c124f89154f8f389a5bd7) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "popobear_ta-a-901.u9", 0x00000, 0x40000,  CRC(f1e94926) SHA1(f4d6f5b5811d90d0069f6efbb44d725ff0d07e1c) )
ROM_END

GAME( 2000, popobear,    0, popobear,    popobear,    0, ROT0,  "BMC", "PoPo Bear", GAME_NOT_WORKING | GAME_IS_SKELETON )
