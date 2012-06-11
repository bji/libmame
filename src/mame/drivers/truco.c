/******************************************************************************************************

  Truco-Tron - (c) 198? Playtronic SRL, Argentina.

  Written by Ernesto Corvi
  Additional work by Roberto Fresca.


  Notes:

  - The board uses a battery backed ram for protection, mapped at $7c00-$7fff.
  - If the battery backup data is corrupt, it comes up with some sort of code entry screen.
    As far as I can tell, you can't do anything with it.
  - Replacing the battery backed ram with an eeprom is not really an option since the game stores the
    current credits count in the battery backed ram.
  - System clock is 12 Mhz. The CPU clock is unknown.
  - The Alternate Gfx mode is funky. Not only it has different bitmaps, but also the strings with the
    game options are truncated. Title is also truncated.


*******************************************************************************************************

  Mini-board (6"x 7") silkscreened 8901 REV.C
  JAMMA connector.

  1x Xtal 12 MHz.
  1x 3.6V. Lithium Battery (QTC85).

  2 rows of 6 holes for jumpers (JP1, JP2).
  No DIP switches banks.

  All IC's are scratched to avoid the identification.


  PCB layout:
  .--------------------------------------------------.
  |S T  .---. .---. .---. .-.    .-.     .---. .-.   |
  |E R  |   | | U | | U | |U|    |U|     | U | |U|   |
  |R U  |   | | 2 | | 3 | |1|    |5|     | 1 | |1|   |
  |I C  | U | |   | |   | |5|    | |     | 0 | |4|   |
  |E O  | 1 | |   | |   | '-'    '-'     |   | '-'   |
  |     |   | '---' '---'        .-.     '---' .-.   |
  |0 T  |   |                    |U| .-------. |U|   |
  |0 R  |   |                    |1| |BATTERY| |1|   |
  |0 O  |   |                    |2| | -   + | |9|   |
  |0 N  '---'                    '-' '-------' '-'   |
  | .-. .-,   .---. .-.   .---.  .-.   .-.     .-.   |
  | |U| |U|   |   | |U|   |   |  |U|   |U|     |U|   |
  | |1| |1|   |   | |1|   |   |  |6|   |7|     |1|   |
  | |7| |8|   | U | |6|   | U |  | |   | |     |3|  P|
  | '-' '-'   | 4 | '-'   | 9 |  '-'   '-'     | |  L|
  | .----.    |   |       |   |  .-.   .-.     '-'  A|
  | |Xtal|    |   |       |   |  |U|   |U|          Y|
  | '----'    |   |       |   |  |1|   |8|          T|
  | .-.       |   |       |   |  |1|   | |          R|
  | |U|       '---'       '---'  '-'   '-'          O|
  | |2|.---.                                        N|
  | |0||pot|                                        I|
  | '-''---'          JAMMA                         C|
  '----+++++++++++++++++++++++++++++-----------------'
       |||||||||||||||||||||||||||||
       '---------------------------'


  IC's Reverse Engineering....

  MARKED   PINS     ID    TYPE        DETAILS

  - U1 : 40-pin IC  YES   CPU         MOTOROLA M6809
  - U2 : 28-pin IC  YES   ROM U2      M27128A
  - U3 : 28-pin IC  YES   ROM U3      M27128A
  - U4 : 40-pin IC  YES   PIA         ST EF6821P
  - U5 : 16-pin IC  NO
  - U6 : 16-pin IC  NO
  - U7 : 16-pin IC  NO
  - U8 : 16-pin IC  NO
  - U9 : 40-pin IC  YES   CRTC        HD6845 / UM6845
  - U10: 28-pin IC  YES   RAM         ???? (Battery NVR)
  - U11: 14-pin IC  NO
  - U12: 14-pin IC  NO
  - U13: 20-pin IC  NO
  - U14: 20-pin IC  NO
  - U15: 20-pin IC  NO
  - U16: 20-pin IC  NO
  - U17: 14-pin IC  NO
  - U18: 14-pin IC  NO
  - U19: 16-pin IC  YES   WATCHDOG    MAXIM MAX691
  - U20: 16-pin IC  YES   DARLINGTON  ULN2003


                             M6809
                           +---\/---+
                    GND   1|        |40 !HALT <--
                --> !NMI  2|        |39 ETAL  <--
  PIA /IRQA & B --> !IRQ  3|        |38 EXTAL <--
                --> !FIRQ 4|        |37 !RES  <-- PIA /RES & U19(15) MAX691
                <-- BS    5|        |36 MRDY  <--
                <-- BA    6|        |35 Q     <--
                    Vcc   7|   U1   |34 E     <-- PIA E (25)
  PIA /RS0 (36) <-- A0    8|        |33 !DMA  <--
  PIA /RS1 (35) <-- A1    9|Motorola|32 R/!W  --> PIA R/W (21)
                <-- A2   10|  6809  |31 D0    <-> PIA D0 (33)
                <-- A3   11|        |30 D1    <-> PIA D1 (32)
                <-- A4   12|        |29 D2    <-> PIA D2 (31)
                <-- A5   13|        |28 D3    <-> PIA D3 (30)
                <-- A6   14|        |27 D4    <-> PIA D4 (29)
                <-- A7   15|        |26 D5    <-> PIA D5 (28)
                <-- A8   16|        |25 D6    <-> PIA D6 (27)
                <-- A9   17|        |24 D7    <-> PIA D7 (26)
                <-- A10  18|        |23 A15   -->
                <-- A11  19|        |22 A14   -->
                <-- A12  20|        |21 A13   -->
                           +--------+

                            PIA 6821
                          +----\/----+
                      VSS |01      40| CA1 --- PIA CB1 (*)
  JAMMA S17 (2P_ST) - PA0 |02      39| CA2 --- U19(11). Watchdog/RESET
  JAMMA S14 (SRVSW) - PA1 |03      38| /IRQA - CPU M6809 !IRQ (03)
  JAMMA C26 (2P_SL) - PA2 |04      37| /IRQB - CPU M6809 !IRQ (03)
  JAMMA S16 (COIN2) - PA3 |05  U4  36| /RS0 -- CPU M6809 A0 (08)
  JAMMA S15 (TLTSW) - PA4 |06      35| /RS1 -- CPU M6809 A1 (09)
  JAMMA C22 (P1_B1) - PA5 |07      34| /RES -- CPU M6809 !RES (37) & U19(15) MAX691
  JAMMA C18/21(U-R) - PA6 |08      33| D0 ---- CPU M6809 D0 (31)
  JAMMA C19/20(D-L) - PA7 |09      32| D1 ---- CPU M6809 D1 (30)
             JP2(4) - PB0 |10      31| D2 ---- CPU M6809 D2 (29)
             JP2(2) - PB1 |11      30| D3 ---- CPU M6809 D3 (28)
            U20(04) - PB2 |12      29| D4 ---- CPU M6809 D4 (27)
            U20(05) - PB3 |13      28| D5 ---- CPU M6809 D5 (26)
             JP1(6) - PB4 |14      27| D6 ---- CPU M6809 D6 (25)
             JP1(4) - PB5 |15      26| D7 ---- CPU M6809 D7 (24)
             JP1(2) - PB6 |16      25| E ----- CPU M6809 E (34)
            U20(07) - PB7 |17      24| CS1
        PIA CA1 (*) - CB1 |18      23| /CS2
            U20(06) - CB2 |19      22| CS0
                      VCC |20      21| R/W
                          +----------+

    (*) Lines CA1 and CB1 are tied together, being both IN.
        They are connected to JAMMA C16 (COIN1).


  U19:   *** MAX691 ***  Maxim MAX691 Microprocessor Supervisory Circuit.
                         (for battery backup power switching and watchdog).
  leg 01 [VBATT] ---->
  leg 02 [VOUT] ----->
  leg 03 [VCC] ------> VCC
  leg 04 [GND] ------> GND
  leg 05 [BATT ON] -->
  leg 06 [/LOWLINE] ->
  leg 07 [OSC IN] ---> N/C \  Set 1.6 seconds as WD timeout.
  leg 08 [OSC SEL] --> N/C /
  leg 09 [PFI] ------>
  leg 10 [/PFO] ----->
  leg 11 [WDI] ------> PIA CA2
  leg 12 [/CE OUT] -->
  leg 13 [/CE IN] ---> GND
  leg 14 [/WDO] ----->
  leg 15 [/RESET] ---> CPU /RES (37)
  leg 16 [RESET] ---->


  U20:   *** ULN2003 ***

  leg 01 --> N/C
  leg 02 --> N/C
  leg 03 --> N/C
  leg 04 --> PIA PB2 (12)
  leg 05 --> PIA PB3 (13)
  leg 06 --> PIA CB2 (19)
  leg 07 --> PIA PB7 (17)
  leg 08 --> GND
  leg 09 --> VCC
  leg 10 --> CAP --> JAMMA(S10) +Speaker
  leg 11 --> JAMMA(S26)
  leg 12 --> JAMMA(C08) Coin Counter 1
  leg 13 --> JAMMA(S08) Coin Counter 2
  leg 14 --> N/C
  leg 15 --> N/C
  leg 16 --> N/C


  JP1:
  01 --> GND
  02 --> PIA PB6 (16) & R5(1K) --> VCC
  03 --> GND
  04 --> PIA PB5 (15) & R7(1K) --> VCC
  05 --> GND
  06 --> PIA PB4 (14) & R8(1K) --> VCC

  JP2:
  01 --> GND
  02 --> PIA PB1 (11) & R9(1K) --> VCC
  03 --> GND
  04 --> PIA PB0 (10) & R10(1K) -> VCC
  05 --> GND
  06 --> Seems N/C


*******************************************************************************************************/


#define MASTER_CLOCK	XTAL_12MHz			/* confirmed */
#define CPU_CLOCK		(MASTER_CLOCK/16)	/* guess */
#define CRTC_CLOCK		(MASTER_CLOCK/8)	/* guess */

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "includes/truco.h"


/*******************************************
*           Read/Write Handlers            *
*******************************************/

static WRITE8_DEVICE_HANDLER( porta_w )
{
	logerror("Port A writes: %2x\n", data);
}

static WRITE8_DEVICE_HANDLER( pia_ca2_w )
{
/*  PIA CA2 line is connected to IC U19, leg 11.
    The IC was successfully identified as MAX691.
    The leg 11 is WDI...

    The code toggles 0's & 1's on this line.
    Legs 07 [OSC IN] and 08 [OSC SEL] aren't connected,
    setting 1.6 seconds as WD timeout.
*/
	watchdog_reset(device->machine());
}

static WRITE8_DEVICE_HANDLER( portb_w )
{
	if ((data & 0x80) | (data == 0))
	{
		dac_data_w(device->machine().device("dac"), data & 0x80);	/* Isolated the bit for Delta-Sigma DAC */
	}
	else
		logerror("Port B writes: %2x\n", data);
}

static WRITE8_DEVICE_HANDLER( pia_irqa_w )
{
		logerror("PIA irq A: %2x\n", data);
}

static WRITE8_DEVICE_HANDLER( pia_irqb_w )
{
		logerror("PIA irq B: %2x\n", data);
}


/*******************************************
*                Memory Map                *
*******************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x17ff) AM_RAM														/* General purpose RAM */
	AM_RANGE(0x1800, 0x7bff) AM_RAM AM_BASE_MEMBER(truco_state, m_videoram)				/* Video RAM */
	AM_RANGE(0x7c00, 0x7fff) AM_RAM AM_BASE_MEMBER(truco_state, m_battery_ram)			/* Battery backed RAM */
	AM_RANGE(0x8000, 0x8003) AM_DEVREADWRITE_MODERN("pia0", pia6821_device, read, write)
	AM_RANGE(0x8004, 0x8004) AM_DEVWRITE_MODERN("crtc", mc6845_device, address_w)
	AM_RANGE(0x8005, 0x8005) AM_DEVREADWRITE_MODERN("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x8008, 0xffff) AM_ROM
ADDRESS_MAP_END
/*
CRTC:

00: 5f
01: 40
02: 4d
03: 06
04: 0f
05: 04
06: 0c
07: 0e
08: 00
09: 0f
0a: 00
0b: 00
0c: 00
0d: c0
*/

/*******************************************
*         Input Ports Definition           *
*******************************************/

static INPUT_PORTS_START( truco )
	PORT_START("P1")	/* IN0 */
	PORT_DIPNAME( 0x01, 0x01, "IN0-1 (P2 START)" )
	PORT_DIPSETTING (	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN0-2 (SERVICE SW)" )
	PORT_DIPSETTING (	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN0-3 (P2 SELECT)" )
	PORT_DIPSETTING (	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN0-4 (COIN2)" )
	PORT_DIPSETTING (	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN0-5 (TILT SW)" )
	PORT_DIPSETTING (	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
//  PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )        /* Connected to JAMMA S17 (P2 START) */
//  PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )        /* Connected to JAMMA S14 (SERVICE SW) */
//  PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )        /* Connected to JAMMA C26 (P2 SELECT) */
//  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        /* Connected to JAMMA S16 (COIN2) */
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )        /* Connected to JAMMA S15 (TILT SW) */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )		/* Connected to JAMMA C22 (P1 BUTTON1) */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )	/* Connected to JAMMA C18/21 (JOY UP & JOY RIGHT) */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )	/* Connected to JAMMA C19/20 (JOY DOWN & JOY LEFT) */

	PORT_START("JMPRS")	/* JP1-2 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING (	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Alt. Graphics" )
	PORT_DIPSETTING (	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING (	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING (	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING (	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING (	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING (	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING (	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING (	0x00, DEF_STR( On ) )

	PORT_START("COIN")	/* IN1 - FAKE - Used for coinup */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*******************************************
*       Machine Reset & Interrupts         *
*******************************************/

static MACHINE_RESET( truco )
{
	truco_state *state = machine.driver_data<truco_state>();
	int a;

	/* Setup the data on the battery backed RAM */

	/* IRQ check */
	state->m_battery_ram[0x002] = 0x51;
	state->m_battery_ram[0x024] = 0x49;
	state->m_battery_ram[0x089] = 0x04;
	state->m_battery_ram[0x170] = 0x12;
	state->m_battery_ram[0x1a8] = 0xd5;

	/* Mainloop check */
	state->m_battery_ram[0x005] = 0x04;
	state->m_battery_ram[0x22B] = 0x46;
	state->m_battery_ram[0x236] = 0xfb;
	state->m_battery_ram[0x2fe] = 0x1D;
	state->m_battery_ram[0x359] = 0x5A;

	/* Boot check */
	a = ( state->m_battery_ram[0x000] << 8 ) | state->m_battery_ram[0x001];

	a += 0x4d2;

	state->m_battery_ram[0x01d] = ( a >> 8 ) & 0xff;
	state->m_battery_ram[0x01e] = a & 0xff;
	state->m_battery_ram[0x020] = state->m_battery_ram[0x011];
}

static INTERRUPT_GEN( truco_interrupt )
{
	truco_state *state = device->machine().driver_data<truco_state>();
	/* coinup */

	if ( input_port_read(device->machine(),  "COIN") & 1 )
	{
		if ( state->m_trigger == 0 )
		{
			generic_pulse_irq_line(device, M6809_IRQ_LINE);
			state->m_trigger++;
		}
	} else
		state->m_trigger = 0;
}


/*******************************************
*              PIA Interfaces              *
*******************************************/
/*

*/
static const pia6821_interface pia0_intf =
{
	DEVCB_INPUT_PORT("P1"),		/* port A in */
	DEVCB_INPUT_PORT("JMPRS"),	/* port B in */
	DEVCB_NULL,					/* line CA1 in ??? */
	DEVCB_NULL,					/* line CB1 in ??? */
	DEVCB_NULL,					/* line CA2 in */
	DEVCB_NULL,					/* line CB2 in */
	DEVCB_HANDLER(porta_w),		/* port A out */
	DEVCB_HANDLER(portb_w),		/* port B out */
	DEVCB_HANDLER(pia_ca2_w),	/* line CA2 out */
	DEVCB_NULL,					/* port CB2 out */
	DEVCB_HANDLER(pia_irqa_w),	/* IRQA */
	DEVCB_HANDLER(pia_irqb_w)	/* IRQB */
};


/*******************************************
*              CRTC Interface              *
*******************************************/

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	4,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


/*******************************************
*              Machine Driver              *
*******************************************/

static MACHINE_CONFIG_START( truco, truco_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT("screen", truco_interrupt)
    MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(1.6))	/* 1.6 seconds */

	MCFG_PIA6821_ADD("pia0", pia0_intf)

	MCFG_MACHINE_RESET(truco)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 192)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 192-1)
	MCFG_SCREEN_UPDATE_STATIC(truco)

	MCFG_PALETTE_LENGTH(16)
	MCFG_PALETTE_INIT(truco)

	MCFG_MC6845_ADD("crtc", MC6845, CRTC_CLOCK, mc6845_intf)	/* Identified as UM6845 */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( truco )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "truco.u3",   0x08000, 0x4000, CRC(4642fb96) SHA1(e821f6fd582b141a5ca2d5bd53f817697048fb81) )
	ROM_LOAD( "truco.u2",   0x0c000, 0x4000, CRC(ff355750) SHA1(1538f20b1919928ffca439e4046a104ddfbc756c) )
ROM_END

/*    YEAR  NAME     PARENT  MACHINE  INPUT    INIT  ROT    COMPANY           FULLNAME     FLAGS  */
GAME( 198?, truco,   0,      truco,   truco,   0,    ROT0, "Playtronic SRL", "Truco-Tron", 0 )
