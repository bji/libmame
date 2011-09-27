/***************************************************************************

  Namco System 10 - Arcade PSX Hardware
  =====================================
  Driver by smf

Namco System 10 Hardware Overview
---------------------------------
Note! This document is a Work-In-Progress and will be updated from time to time when more dumps are available.

This document covers all the known Namco System 10 games, including....
*Drum Master                                     (C) Namco, 2001
*Drum Master 2                                   (C) Namco, 2001
*Drum Master 3                                   (C) Namco, 2002
*Drum Master 4                                   (C) Namco, 2003
*Drum Master 5                                   (C) Namco, 2003
*Drum Master 6                                   (C) Namco, 2004
*Drum Master 7                                   (C) Namco, 2005
*GAHAHA Ippatsu-dou                              (C) Namco/Metro, 2000
*GAHAHA Ippatsu-dou 2                            (C) Namco/Metro, 2001
Gamshara (10021 Ver.A)                           (C) Mitchell, 2003
Gekitoride-Jong Space (10011 Ver.A)              (C) Namco/Metro, 2001
*Golgo-13 3 : Juusei no Chinkonka                (C) Namco/8ing/Raizing, 2001
*Hard Puncher Hajime no Ippo 2 Ouja e no Chousen (C) Namco/Taito, 2002
*Honne Hakkenki                                  (C) Namco, 2001
Kotoba no Puzzle Mojipittan (KPM1 Ver.A)         (C) Namco, 2001
*Kurukuru Food                                   (C) Namco, 2002
Mr Driller 2 (DR21 Ver.A)                        (C) Namco, 2000
Mr Driller 2 (DR22 Ver.A)                        (C) Namco, 2000
Mr Driller G (DRG1 Ver.A)                        (C) Namco, 2001
*NFL Classic Football                            (C) Namco, 2003
*Panicuru Panekuru (PPA1 Ver.A)                  (C) Namco, 2001
*Photo Battole                                   (C) Namco, 2001
Point Blank 3 / Gunbalina (GNN2 Ver. A)          (C) Namco, 2000
*Ren-ai Quiz High School Angel                   (C) Namco, 2002
*Seishun Quiz Colorful High School               (C) Namco, 2002
Star Trigon (STT1 Ver.A)                         (C) Namco, 2002
*Taiko No Tatsujin                               (C) Namco, 2001
*Taiko No Tatsujin 2                             (C) Namco, 2001
*Taiko No Tatsujin 3                             (C) Namco, 2002
*Taiko No Tatsujin 4                             (C) Namco, 2003
*Taiko No Tatsujin 5                             (C) Namco, 2003
*Taiko No Tatsujin 6                             (C) Namco, 2004
*Tsukkomi Yousei Gips Nice Tsukkomi              (C) Namco/Metro, 2002
Uchuu Daisakusen : Chocovader Contactee (CVC1 Ver.A) (C) Namco, 2002

* - denotes not dumped yet. If you can help with the remaining undumped S10 games,
    please contact me at http://guru.mameworld.info/

The Namco System 10 system comprises 2 PCB's....
MAIN PCB - This is the mother board PCB. It holds the main CPU/GPU & SPU and all sound circuitry, program & video RAM,
           controller/input logic and video output circuitry. Basically everything except the ROMs.
           There are three known revisions of this PCB so far. The differences seem very minor. The 2nd and 3rd revision
           have an updated CPLD revision.
           The 3rd revision has an updated model Sony chip. The only other _noticable_ difference is some component
           shuffling in the sound amplification section to accomodate two extra 1000uF capacitors and one 470uF capacitor
           has been replaced by a 1000uF capacitor. Everything else, including all the PLDs appears to be identical.
           Note there are no ROMs on the Main PCB and also no custom Namco chips on System10, which seem to have been
           phased out. Instead, they have been replaced by (custom programmed) CPLDs, probably due to cost-cutting
           measures within the company, or to reduce the cost of System10 to an entry-level no-frills development platform.
MEM PCB  - There are two known revisions of this PCB (so far). They're mostly identical except for the type/number of ROMs
           used and the contents of the CPLD.  The 2nd revision also has a RAM chip on it.
           Each game has a multi-letter code assigned to it which is printed on a small sticker and placed on the top side
           of the MEM PCB.
           This code is then proceeded by a number (only '1' & '2' seen so far), then 'Ver.' then A/B/C/D/E (only 'A' seen so far)
           which denotes the software revision, and in some cases a sub-revision such as 1 or 2 (usually only listed in the
           test mode).
           The first 1 denotes a Japanese version. Other numbers denote a World version.
           For World versions, only the main program changes, the rest of the (graphics) ROMs use the Japanese version ROMs.
           Any System 10 MEM PCB can be swapped to run on any System 10 Main PCB regardless of the main board revision.
           The high scores are stored on the MEM PCB (probably inside the main program EEPROMs/FlashROMs or maybe the CPLD?).
           Also, on all System 10 games, there is a sticker with a serial number on it and the program ROMs also contain
           that same serial number. I'm not sure why, they're not exactly _easily_ tracable and no one cares either way ;-)
           See the Main PCB and ROM Daughterboard PCB texts below for more details.


Main PCB Layout
---------------

Revision 1
SYSTEM10 MAIN PCB 8906960103 (8906970103)

Revision 2
SYSTEM10 MAIN PCB 8906960104 (8906970104)

Revision 3
SYSTEM10 MAIN PCB 8906962400 (8906972400)
  |----------------------------------------------------------|
  |   LA4705    VR1                     J201                 |
  |                           |----------------------|       |
  |           NJM3414         |----------------------|       |
|-|                                                          |
|       BA3121                                               |
|             NJM3414            54V25632      54V25632    J1|
|                     CXD1178Q                               |
|J JP4                          |---------|   |-------|      |
|A                              |         |   |       |      |
|M         CXA2067AS            |CXD8561CQ|   |CY37128|      |
|M                 53.693175MHz |         |   |VP160  |      |
|A                              |         |   |       |      |
|                               |---------|   |-------|      |
|                                                            |
|                            101.4912MHz                     |
|-|          MAX734  IS41LV16100                             |
  |                              |---------|                 |
  |   DSW1           IS41LV16100 |         |                 |
  |                              |CXD8606BQ|                 |
  |        GAL16V8D              |         |                 |
  |J5      |-|           *       |         |          PST592 |
  |        | |                   |---------| |--------|      |
  |        | |                               |        |      |
  |        | |           *                   |CXD2938Q|      |
  |J4      | |J202             IS41LV16256   |        |      |
  |        | |                               |        |      |
  |        | |      EPM3064                  |--------|      |
  |        | |                                               |
  |        |-|                 PQ30RV21                      |
  |                                                  J103    |
  |----------------------------------------------------------|
Notes:
------
      CXD8606BQ   : SONY CXD8606BQ Central Processing Unit / GTE     (QFP208)
                     - replaced by CXD8606CQ on Revision 3 Main PCB
      CXD8561CQ   : SONY CXD8561CQ Graphics Processsor Unit          (QFP208)
      CXD2938Q    : SONY CXD2938Q  Sound Processor Unit              (QFP208)
      CXD1178Q    : SONY CXD1178Q  8-bit RGB 3-channel D/A converter (QFP48)
      CXA2067AS   : SONY CXA2067AS TV/Video circuit RGB Pre-Driver   (SDIP30)
      CY37128VP160: CYPRESS CY37128VP160 Complex Programmable Logic Device (TQFP160, stamped 'S10MA1')
                     - replaced by an updated revision on Revision 2 & 3 Main PCB and stamped 'S10MA1B'
      EPM3064     : Altera MAX EPM3064ATC100-10 Complex Programmable Logic Device (TQFP100, stamped 'S10MA2A')
      GAL16V8D    : GAL16V8D PAL (PLCC20, stamped 'S10MA3A')
      IS41LV16100 : ISSI IS41LV16100S-50T 1M x16 EDO DRAM (x2, TSOP50(44) Type II)
      IS41LV16256 : ISSI IS41LV16256-50T 256k x16 EDO DRAM (TSOP44(40) Type II)
      54V25632    : OKI 54V25632 256K x32 SGRAM (x2, QFP100)
      PQ30RV31    : Sharp PQ30RV31 5 Volt to 3.3 Volt Voltage Regulator
      LA4705      : LA4705 15W 2-channel Power Amplifier (SIP18)
      MAX734      : MAX734 +12V 120mA Flash Memory Programming Supply Switching Regulator (SOIC8)
      PST592      : PST592J System Reset IC with 2.7V detection circuit (SOIC4)
      BA3121      : Ground Isolation Amplifier/ Noise Eliminator (SOIC8)
      NJM3414     : 70mA Dual Op Amp (x2, SOIC8)
      DSW1        : 8 position dip switch
      JP4         : 2 position jumper, set to NC, alt. position labelled SYNC (Note: changing the jumper position has
                    no visual effect)
      J1          : 40 Pin IDC connector for plugging of a flat 40-wire cable (not used, purpose unknown, possible
                    CDROM/DVD)
      J4          : 10 pin header for extra controls etc  / (note: custom Namco 48 pin edge connector is not on
                    System10 PCBs)
      J5          : 4 pin header for stereo sound out     \
      J201        : 100 pin custom Namco connector for mounting of MEM PCB. This connector is surface-mounted, not a
                    thru-hole type.
      J202        : 80 pin custom Namco connector for mounting of another board. This connector is surface-mounted,
                    not a thru-hole type.
                    (not used, purpose unknown)
      J103        : 6-pin JAMMA2 power plug (Note none of the other JAMMA2 standard connectors are present)
      VR1         : Volume potentiometer
      *           : Unpopulated position for IS41LV16100 1M x16 EDO DRAM

Additional Notes:
                1. In test mode (Display Test) the screen can be set to interlace or non-interlace mode. The graphics in
                   interlace mode are visually much smoother with noticeable screen flickering. Non-interlace modes gives
                   a much blockier graphic display (i.e. lower resolution) but without screen flickering.
                2. There is no dedicated highscore/options EEPROM present on the PCB, the game stores it's settings on the
                   game board (probably in the program EEPROMs/FlashROMs or maybe the CPLD?).

ROM Daughterboard PCB
---------------------
This PCB holds all the ROMs.
There are two known types of ROM daughterboards used on S10 games (so far).
All of the PCBs are the same size (approx 5" x 5") containing one custom connector surface-mounted to the underside of
the PCB, some MASKROMs/FlashROMs, a CPLD (which seems to be the customary 'KEYCUS' chip, and on the 2nd type a RAM
chip also.

********
*Type 1*
********
System10 MEM(M) PCB 8906961000 (8906970700)
|-------------------------------------|
|                                     |
|                       |-------|     +-
|                       |       |     +-
|                       |CY37128|     +-
|                       |VP160  |     +-
|                       |       |     +-
|     7E     7D         |-------|     +-
|                                     |
|     6E     6D                       |
|                                     |
|     5E     5D                 5A    |
|                                     |
|     4E     4D                 4A    |
|                                     |
|     3E     3D                       |
|                                     |
|     2E     2D                 2A    |
|                                     |
|     1E     1D                 1A    |
|                                     |
|-------------------------------------|
Notes:
      CY37128VP160: CY37128VP160 Cypress Complex Programmable Logic Device (TQFP160)
      1A - 5A     : Intel Flash DA28F640J5 64MBit Flash EEPROM (SSOP56)
      1D - 7E     : Samsung Electronics K3N9V1000A-YC 128MBit MASK ROM (TSOP48) (see note 3)
      6 pin header: (purpose unknown, probably for programming the CPLD)

This PCB is used on:

              Software     MEM PCB
Game          Revision     Sticker      KEYCUS   ROMs Populated
------------------------------------------------------------------------------------
Mr Driller 2  DR21/VER.A3  DR21 Ver.A   KC001A   DR21VERA.1A, DR21MA1.1D, DR21MA2.2D
Mr Driller 2  DR22/VER.A3  DR22 Ver.A   KC001A   DR22VERA.1A, DR21MA1.1D, DR21MA2.2D

      Note
      1. The ROM PCB has locations for 4x 64MBit program ROMs, but only 1A is populated.
      2. The ROM PCB has locations for 14x 128MBit GFX ROMs (Total capacity = 2048MBits) but only 1D and 2D are populated.
      3. These ROMs are only 18mm long, dumping them requires a special custom adapter

********
*Type 2*
********
System10 MEM(N) PCB 8906961402 (8906971402)
|-------------------------------------|
|                                     |
|                    |---------|      +-
|                    |         |      +-
|                    |CY37256  |      +-
|     8E     8D      |VP208    |      +-
|                    |         |      +-
|     7E     7D      |---------|      +-
|                                     |
|     6E     6D                       |
|                                     |
|     5E     5D       CY7C1019        |
|                                     |
|     4E     4D                       |
|                                     |
|     3E     3D                       |
|                                     |
|     2E     2D                       |
|                                     |
|     1E     1D                       |
|                                     |
|-------------------------------------|
Notes:
      CY37256VP208: Cypress CY37256VP208 Complex Programmable Logic Device (TQFP208)
      CY7C1019    : Cypress CY7C1019BV33-15VC or Samsung Electronics K6R1008V1C-JC15 128k x8 bit 3.3V High Speed CMOS Static Ram (SOJ32)
      1D - 8E     : Samsung Electronics K9F2808U0B-YCBO 128MBit NAND Flash EEPROM (TSOP48)
      6 pin header: (purpose unknown, probably for programming the CPLD)

This PCB is used on:

                                      MEM PCB
Game                                  Sticker       KEYCUS   ROMs Populated
---------------------------------------------------------------------------
Gamshara                              10021 Ver.A   KC020A   8E, 8D
Gekitoride-Jong Space                 10011 Ver.A   KC003A   8E, 8D, 7E, 7D
Mr.Driller G                          DRG1  Ver.A   KC007A   8E, 8D, 7E
Kotoba no Puzzle Mojipittan           KPM1  Ver.A   KC012A   8E, 8D, 7E
Panicuru Panekuru                     PPA1  Ver.A   KC017A   8E, 8D, 7E
Star Trigon                           STT1  Ver.A   KC019A   8E, 8D
Utyuu Daisakusen Chocovader Contactee CVC1  Ver.A   KC022A

      Note
      1. The ROM PCB has locations for 16x 128MBit FlashROMs (Total capacity = 2048MBits) but usually only a few are populated.
*/

#include "emu.h"
#include "cpu/mips/psx.h"
#include "includes/psx.h"

WRITE32_HANDLER( namcos10_bank_w )
{
	memory_set_bank( space->machine(), "bank1", data & 0xf );
}

class namcos10_state : public psx_state
{
public:
	namcos10_state(running_machine &machine, const driver_device_config_base &config)
		: psx_state(machine, config) { }
};

static ADDRESS_MAP_START( namcos10_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00ffffff) AM_RAM AM_SHARE("share1") /* ram */
	AM_RANGE(0x1f800000, 0x1f8003ff) AM_RAM /* scratchpad */
	AM_RANGE(0x1f801000, 0x1f801007) AM_WRITENOP
	AM_RANGE(0x1f801008, 0x1f80100b) AM_RAM /* ?? */
	AM_RANGE(0x1f80100c, 0x1f80102f) AM_WRITENOP
	AM_RANGE(0x1f801010, 0x1f801013) AM_READNOP
	AM_RANGE(0x1f801014, 0x1f801017) AM_READNOP
	AM_RANGE(0x1f801040, 0x1f80105f) AM_READWRITE(psx_sio_r, psx_sio_w)
	AM_RANGE(0x1f801060, 0x1f80106f) AM_WRITENOP
	AM_RANGE(0x1f801070, 0x1f801077) AM_READWRITE(psx_irq_r, psx_irq_w)
	AM_RANGE(0x1f801080, 0x1f8010ff) AM_READWRITE(psx_dma_r, psx_dma_w)
	AM_RANGE(0x1f801100, 0x1f80112f) AM_READWRITE(psx_counter_r, psx_counter_w)
	AM_RANGE(0x1f801810, 0x1f801817) AM_READWRITE(psx_gpu_r, psx_gpu_w)
	AM_RANGE(0x1f801820, 0x1f801827) AM_READWRITE(psx_mdec_r, psx_mdec_w)
	AM_RANGE(0x1f801c00, 0x1f801dff) AM_NOP
	AM_RANGE(0x1f802020, 0x1f802033) AM_RAM /* ?? */
	AM_RANGE(0x1f802040, 0x1f802043) AM_WRITENOP
	AM_RANGE(0x1fc00000, 0x1fffffff) AM_ROM AM_SHARE("share2") AM_REGION("user1", 0) /* bios */
	AM_RANGE(0x80000000, 0x80ffffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fffffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xa0000000, 0xa0ffffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfffffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END

static void memm_driver_init( running_machine &machine )
{
	psx_driver_init(machine);
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler( 0x1f300000, 0x1f300003, FUNC(namcos10_bank_w) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank( 0x1f400000, 0x1f7fffff, "bank1" );
	memory_configure_bank( machine, "bank1", 0, 16, machine.region( "user2" )->base(), 4 * 1024 * 1024 );
}

UINT8 *nand_base;
UINT32 nand_address = 0;

READ32_HANDLER( nand_status_r )
{
	return 0;
}

WRITE32_HANDLER( nand_address1_w )
{
	nand_address = ( nand_address & 0x00ffffff ) | ( ( data & 0xff ) << 24 );
}

WRITE32_HANDLER( nand_address2_w )
{
	nand_address = ( nand_address & 0xff00ffff ) | ( ( data & 0xff ) << 16 );
}

WRITE32_HANDLER( nand_address3_w )
{
	nand_address = ( nand_address & 0xffff00ff ) | ( ( data & 0xff ) << 8 );
}

WRITE32_HANDLER( nand_address4_w )
{
	nand_address = ( nand_address & 0xffffff00 ) | ( ( data & 0xff ) << 0 );
}

UINT16 nand_read( UINT32 address )
{
	int index = ( ( address / 512 ) * 528 ) + ( address % 512 );
	return nand_base[ index ] | ( nand_base[ index + 1 ] << 8 );
}

UINT16 nand_read2( UINT32 address )
{
	int index = ( ( address / 512 ) * 528 ) + ( address % 512 );
	return nand_base[ index + 1 ] | ( nand_base[ index ] << 8 );
}

READ32_HANDLER( nand_data_r )
{
	UINT32 data = nand_read2( nand_address * 2 );

/*  printf( "data<-%08x (%08x)\n", data, nand_address ); */
	nand_address++;

	return data;
}

static void nand_copy( UINT32 *dst, UINT32 address, int len )
{
	while( len > 0 )
	{
		*( dst++ ) = nand_read( address ) | ( nand_read( address + 2 ) << 16 );
		address += 4;
		len -= 4;
	}
}

UINT32 block[ 0x1ff ];

WRITE32_HANDLER( nand_block_w )
{
	COMBINE_DATA( &block[ offset ] );
/*  printf( "block %d %08x\n", offset, data ); */
}

READ32_HANDLER( nand_block_r )
{
	return block[ offset ];
}

WRITE32_HANDLER( watchdog_w )
{
}

static void memn_driver_init( running_machine &machine )
{
	UINT8 *BIOS = (UINT8 *)machine.region( "user1" )->base();
	nand_base = (UINT8 *)machine.region( "user2" )->base();

	nand_copy( (UINT32 *)( BIOS + 0x0000000 ), 0x08000, 0x001c000 );
	nand_copy( (UINT32 *)( BIOS + 0x0020000 ), 0x24000, 0x03e0000 );

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler( 0x1f400000, 0x1f400003, FUNC(nand_status_r) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler( 0x1f410000, 0x1f410003, FUNC(nand_address1_w) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler( 0x1f420000, 0x1f420003, FUNC(nand_address2_w) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler( 0x1f430000, 0x1f430003, FUNC(nand_address3_w) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler( 0x1f440000, 0x1f440003, FUNC(nand_address4_w) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler( 0x1f450000, 0x1f450003, FUNC(nand_data_r) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler( 0x1fb60000, 0x1fb60003, FUNC(watchdog_w) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler( 0xbf500000, 0xbf5007d7, FUNC(nand_block_w) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler( 0xbf500000, 0xbf5007d7, FUNC(nand_block_r) );

	psx_driver_init(machine);
}

static void decrypt_bios( running_machine &machine, int b15, int b14, int b13, int b12, int b11, int b10, int b9, int b8,
	int b7, int b6, int b5, int b4, int b3, int b2, int b1, int b0 )
{
	UINT16 *BIOS = (UINT16 *)machine.region( "user1" )->base();
	int len = machine.region( "user1" )->bytes() / 2;
	int i;

	for( i = 0; i < len; i++ )
	{
		BIOS[ i ] = BITSWAP16( BIOS[ i ] ^ 0xaaaa,
			b15, b14, b13, b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0 );
	}
}

static DRIVER_INIT( mrdrilr2 )
{
	memm_driver_init(machine);
	decrypt_bios( machine, 0xc, 0xd, 0xf, 0xe, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x4, 0x1, 0x2, 0x5, 0x0, 0x3 );
}

static DRIVER_INIT( gjspace )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x0, 0x2, 0xe, 0xd, 0xf, 0x6, 0xc, 0x7, 0x5, 0x1, 0x9, 0x8, 0xa, 0x3, 0x4, 0xb );
}

static DRIVER_INIT( mrdrilrg )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x6, 0x4, 0x7, 0x5, 0x2, 0x1, 0x0, 0x3, 0xc, 0xd, 0xe, 0xf, 0x8, 0x9, 0xb, 0xa );
}

static DRIVER_INIT( knpuzzle )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x6, 0x7, 0x4, 0x5, 0x2, 0x0, 0x3, 0x1, 0xc, 0xd, 0xe, 0xf, 0x9, 0xb, 0x8, 0xa );
}

static DRIVER_INIT( startrgn )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9 );
}

static DRIVER_INIT( gamshara )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x8, 0x9, 0xa, 0xb );
}

static DRIVER_INIT( gunbalna )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x9, 0x8, 0xa, 0xb );
}

static DRIVER_INIT( chocovdr )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x8, 0x9, 0xa, 0xb );
}

static MACHINE_RESET( namcos10 )
{
	psx_machine_init(machine);
}

static MACHINE_CONFIG_START( namcos10, namcos10_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8661R, XTAL_101_4912MHz )
	MCFG_CPU_PROGRAM_MAP( namcos10_map)
	MCFG_CPU_VBLANK_INT("screen", psx_vblank)

	MCFG_MACHINE_RESET( namcos10 )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE( 1024, 1024 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 639, 0, 479 )
	MCFG_SCREEN_UPDATE( psx )

	MCFG_PALETTE_LENGTH( 65536 )

	MCFG_PALETTE_INIT( psx )
	MCFG_VIDEO_START( psx_type2 )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

static INPUT_PORTS_START( namcos10 )
	/* IN 0 */
	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x8000, IP_ACTIVE_HIGH )
	PORT_BIT( 0x7fff, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 1 */
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 2 */
	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 3 */
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
INPUT_PORTS_END

ROM_START( mrdrilr2 )
	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* main prg */
	ROM_LOAD( "dr21vera.1a",  0x000000, 0x800000, CRC(f93532a2) SHA1(8b72f2868978be1f0e0abd11425a3c8b2b0c4e99) )

	ROM_REGION( 0x4000000, "user2", 0 ) /* main prg */
	ROM_LOAD( "dr21ma1.1d", 0x0000000, 0x1000000, CRC(26dc6f55) SHA1(a9cedf547fa7a4d5850b9b3b867d46e577a035e0) )
	ROM_LOAD( "dr21ma2.2d", 0x1000000, 0x1000000, CRC(702556ff) SHA1(c95defd5fd6a9b406fc8d8f28ecfab732ef1ff42) )
ROM_END

ROM_START( mrdrlr2a )
	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* main prg */
	ROM_LOAD( "dr22vera.1a",  0x000000, 0x800000, CRC(f2633388) SHA1(42e56c9758ee833390003d4b41956f75f5a22760) )

	ROM_REGION( 0x4000000, "user2", 0 ) /* main prg */
	ROM_LOAD( "dr21ma1.1d", 0x0000000, 0x1000000, CRC(26dc6f55) SHA1(a9cedf547fa7a4d5850b9b3b867d46e577a035e0) )
	ROM_LOAD( "dr21ma2.2d", 0x1000000, 0x1000000, CRC(702556ff) SHA1(c95defd5fd6a9b406fc8d8f28ecfab732ef1ff42) )
ROM_END

ROM_START( gjspace )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x4200000, "user2", 0 ) /* main prg */
	ROM_LOAD( "10011a_0.bin", 0x0000000, 0x1080000, CRC(df862033) SHA1(4141357ed315adb4de636d7bf752354e953e8cbf) )
	ROM_LOAD( "10011a_1.bin", 0x1080000, 0x1080000, CRC(734c7ac0) SHA1(2f325236a4e4f2dba886682e9a7e8e243b5fbb3d) )
	ROM_LOAD( "10011a_2.bin", 0x2100000, 0x1080000, CRC(3bbbc0b7) SHA1(ad02ec2e5f401f0f5d40a413038649ebd25d5343) )
	ROM_LOAD( "10011a_3.bin", 0x3180000, 0x1080000, CRC(fb0de5ca) SHA1(50a462a52ff4a0bc112b9d89f2b2d032c60cf59c) )
ROM_END

ROM_START( mrdrilrg )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x3180000, "user2", 0 ) /* main prg */
	ROM_LOAD( "drg1a_0.bin",  0x0000000, 0x1080000, CRC(e0801878) SHA1(fbb771c1e76e0690f6dffed2287eb470b561ec20) )
	ROM_LOAD( "drg1a_1.bin",  0x1080000, 0x1080000, CRC(4d8cde73) SHA1(62a5fab8be8fd0a6bfeb101020d4cf58866a757c) )
	ROM_LOAD( "drg1a_2.bin",  0x2100000, 0x1080000, CRC(ccfabf7b) SHA1(0cbd91ce8abd6efca5d427b52279ce265f685aa9) )
ROM_END

ROM_START( knpuzzle )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x3180000, "user2", 0 ) /* main prg */
	ROM_LOAD( "kpm1a_0.bin",  0x0000000, 0x1080000, CRC(b2947eb8) SHA1(fa941bf3598bb25d2c8f0a93154e32bf78a6507c) )
	ROM_LOAD( "kpm1a_1.bin",  0x1080000, 0x1080000, CRC(f3aa855a) SHA1(87b94e22db4bc4169324bbff93c4ea19c1d99b40) )
	ROM_LOAD( "kpm1a_2.bin",  0x2100000, 0x1080000, CRC(b297cc8d) SHA1(c3494e7a8a0b4e0c8c40b99121373effbfe848eb) )
ROM_END

ROM_START( startrgn )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "stt1a_0.bin",  0x0000000, 0x1080000, CRC(1e090644) SHA1(a7a293e2bd9eea2eb64a492a47272d9d9ee2c724) )
	ROM_LOAD( "stt1a_1.bin",  0x1080000, 0x1080000, CRC(aa527694) SHA1(a25dcbeca58a1443070848b3487a24d51d41a34b) )
ROM_END

ROM_START( gamshara )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "10021a.8e",    0x0000000, 0x1080000, CRC(6c0361fc) SHA1(7debf1f2e6bed31d59fb224a78a17a94fc573785) )
	ROM_LOAD( "10021a.8d",    0x1080000, 0x1080000, CRC(73669ff7) SHA1(eb8bbf931f1f8a049208d081d040512a3ffa9c00) )
ROM_END

ROM_START( ptblank3 )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(31b39221) SHA1(7fcb14aaa26c531928a6cd704e746d0e3ae3e031) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(82d2cfb5) SHA1(4b5e713a55e74a7b32b1b9b5811892df2df86256) )
ROM_END

ROM_START( chocovdr )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x5280000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(f265b1b6) SHA1(f327e7bac0bc1bd31aa3362e36233130a6b240ea) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(05d01cd2) SHA1(e9947ebea24d618e8b9a69f582ef0b9d97bb4cad) )
	ROM_LOAD( "2.7e",         0x2100000, 0x1080000, CRC(2e308d20) SHA1(4ff072f0d488b12f77ef7d119822f89b5b5a6712) )
	ROM_LOAD( "3.7d",         0x3180000, 0x1080000, CRC(126c9e6f) SHA1(32de87f01fd1c8c26a68bf42a062f5f44bcc5a3b) )
	ROM_LOAD( "4.6e",         0x4200000, 0x1080000, CRC(6c18678d) SHA1(410a282397e4a4b5763467811080172b462bfcef) )
ROM_END

GAME( 2000, mrdrilr2,  0,        namcos10, namcos10, mrdrilr2, ROT0, "Namco", "Mr. Driller 2 (Japan, DR21 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2000, mrdrlr2a,  mrdrilr2, namcos10, namcos10, mrdrilr2, ROT0, "Namco", "Mr. Driller 2 (Japan, DR22 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2000, ptblank3,  0,        namcos10, namcos10, gunbalna, ROT0, "Namco", "Point Blank 3 (GNN2 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2001, gjspace,   0,        namcos10, namcos10, gjspace,  ROT0, "Namco / Metro", "Gekitoride-Jong Space (10011 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2001, mrdrilrg,  0,        namcos10, namcos10, mrdrilrg, ROT0, "Namco", "Mr. Driller G (Japan, DRG1 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2001, knpuzzle,  0,        namcos10, namcos10, knpuzzle, ROT0, "Namco", "Kotoba no Puzzle Mojipittan (Japan, KPM1 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2002, chocovdr,  0,        namcos10, namcos10, chocovdr, ROT0, "Namco", "Uchuu Daisakusen: Chocovader Contactee (Japan, CVC1 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2002, startrgn,  0,        namcos10, namcos10, startrgn, ROT0, "Namco", "Star Trigon (Japan, STT1 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2003, gamshara,  0,        namcos10, namcos10, gamshara, ROT0, "Mitchell", "Gamshara (10021 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
