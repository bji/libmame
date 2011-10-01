/**********************************************************************************

  Major Poker / Major Joker
  1994 PAL System.

  Driver by Roberto Fresca.


  Special thanks to Tomasz Slanina & Rob Ragon for their invaluable help.

***********************************************************************************

  Hardware Notes:
  --------------

  1x Z80 @ 6 MHz. (main CPU)

  1x M6295 @ 1.5 MHz. (4 channel mixing ADPCM voice synthesis). Pin7 high.

  1x 6845 (CRT Controller) @ 750 kHz.
          Hs 15625 Hz
          Vs 52.786 Hz

  2x MB8464 (Video?)8kx8
  1x MB8416 (NVRAM) 2Kx8
  1x MB8464 (?) 8kx8

  2x 27C040 (Roms P1 & P2) (GFX ROMs).
  2x 27C010 (Roms 3 & 4) (GFX ROMs).
  1x 27C020 (Rom 5) (sound)
  1x 27C512 (Rom 6) (program)

  1x Xtal @ 12 MHz.

  4x 8 DIP switches banks.
  2x switches (SW1, SW2).

  1x 2x10-pin edge connector.
  1x 2x22-pin edge connector.


***********************************************************************************

  PORTS:
  ------

  00  W ---> ROM bank.
  01  W ---> RAM bank.
  02  W ---> RAM bank.

  10 R  ---> Regular inputs (holds) multiplexed with credits out.
  10  W ---> Writes a kind of watchdog (bit4) and mech counters pulses (bits 0-1-2-3).
  11 R  ---> Read input port (remaining controls).
  11  W ---> Writes mux selector.
  12 R  ---> Read coin/credits port.
  12  W ---> Video registers: Normal or Up Down screen (bit6 off/on). Also writes 0xFC & 0x11 at CRTC offsets 0x0C & 0x0D (0's if normal).
  13 R  ---> Multiplexed port for DIP switches banks.
  13  W ---> Lamps out (array a)
  14 R  ---> Freeze. Switch in bit0 that pause the game when active. Just once active, the code loops till the bit resets.
  14  W ---> Lamps out (array b).

  50 RW ---> OKI6295 (R/W)

  60  W ---> PSG SN76489/96 initialization routines. (Maybe a leftover for different hardware)
             The offset is initialized with the following sequence: 0x9f, 0xbf, 0xdf, 0xff.


  Pulses - Port 10h.
  ------------------
    - bits -
    7654 3210
    ---- ---x   Credits Out mech counter.
    ---- --x-   Credits 3 mech counter.
    ---- -x--   Credits 1 mech counter.
    ---- x---   Credits 2 mech counter.
    ---x ----   Watchdog? (constant writes). (*)
    xxx- ----   Unknown.

  (*) Tied to a ULN Opto Triac (pin3 solder side edge connector - Lock Out (100V))


  Lamps - Array A - Port 13h.
  ---------------------------
    - bits -
    7654 3210
    ---- ---x   Hold 1 lamp.
    ---- --x-   Hold 2 lamp.
    ---- -x--   Hold 3 lamp.
    ---- x---   Hold 4 lamp.
    ---x ----   Hold 5 lamp.
    --x- ----   Big lamp.
    -x-- ----   Small lamp.
    x--- ----   Unknown.


  Lamps - Array B - Port 14h.
  ---------------------------
    - bits -
    7654 3210
    ---- ---x   Bet lamp.
    ---- --x-   Draw lamp.
    ---- -x--   Cancel lamp.
    ---- x---   Take lamp.
    ---x ----   Double-Up lamp.
    --x- ----   Fever lamp.
    xx-- ----   Unknown.

***********************************************************************************

  Instructions...
  (translated from a Japanese flyer)

  Bonus stage:
  - 10 fever awarded by three of a kind of 1, 3, or 7.
  - 15 fever awarded by four of a kind.

  Slot stage:
  - 10 bonus stage awarded by three consecutive 1, 3, or 7.
  -  5 bonus stage awarded by four consecutive numbers other than 1, 3, or 7.

  If bonus is awarded while you are in bonus stage, odds will be x4.
  Bonus stage & slot stage: You can get triple & fourth fever points, and you
  can challenge 3 or 4 times of double-up game by this point.

  ....

  Service Mode:

  F2 to enter the Book/Settings Mode.
  HOLD1 to move UP.
  HOLD2 to move DOWN.
  HOLD3 to move LEFT.
  HOLD4 to move RIGHT.
  HOLD5 to move next screen / exit.

  You can shortcut the DIP settings pressing "0" in Book Mode.
  HOLD5 to exit.

***********************************************************************************

   Major Poker: Things that you should know!!! (from "engrish" manual)
  -------------------------------------------------------------------

  The game not stabilise until 10,000 games have been played. As a result, the
  actual payout % may exceed the set payout % during this period. After the 10,000
  games the machine will follow the set payout %.

  Original game for your location.

  Analyzer screen #1: Provides you with data regarding games played so far such as
  numbers of games, number of fevers, etc...

  Analyzer screen #2: Allows you to adjust various aspects of the game to suit your
  location. For example you can increase the number of Fever (this does not affect
  the payout %). Also you can make the dealing speed quicker or slower.

  Even if you do not adjust the game it is ready to be played.


  DIP Switches....

  .-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switches #1               |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | OFF Fixed                     | OFF |                                         |
  +-------------------------------+-----+-----+-----------------------------------+
  | Screen Direction   Normal     |     | OFF |                                   |
  |                    Up Down    |     | ON  |                                   |
  +-------------------------------+-----+-----+-----+-----------------------------+
  | Hopper             No         |           | OFF |                             |
  |                    Yes        |           | ON  |                             |
  +-------------------------------+-----------+-----+-----+-----------------------+
  | Hopper SW Active   High       |                 | OFF |                       |
  |                    Low        |                 | ON  |                       |
  +-------------------------------+-----------------+-----+-----+-----------------+
  | Coin Payout        Payout SW  |                       | OFF |                 |
  |                    Automatic  |                       | ON  |                 |
  +-------------------------------+-----------------------+-----+-----+-----------+
  | Hold Cancel        No         |                             | OFF |           |
  |                    Yes        |                             | ON  |           |
  +-------------------------------+-----------------------------+-----+-----+-----+
  | Auto Hold          No         |                                   | OFF |     |
  |                    Yes        |                                   | ON  |     |
  +-------------------------------+-----------------------------------+-----+-----+
  | Fever Mode         Yes        |                                         | OFF |
  |                    No         |                                         | ON  |
  '-------------------------------+-----------------------------------------+-----'

  .-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switches #2               |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | Payout %           95%        | OFF | OFF | OFF |                             |
  |                    90%        | ON  | OFF | OFF |                             |
  |                    85%        | OFF | ON  | OFF |                             |
  |                    80%        | ON  | ON  | OFF |                             |
  |                    75%        | OFF | OFF | ON  |                             |
  |                    70%        | ON  | OFF | ON  |                             |
  |                    65%        | OFF | ON  | ON  |                             |
  |                    60%        | ON  | ON  | ON  |                             |
  +-------------------------------+-----+-----+-----+-----+-----+-----------------+
  | Min Bet size        1         |                 | OFF | OFF |                 |
  | for Fever           3         |                 | ON  | OFF |                 |
  |                     5         |                 | OFF | ON  |                 |
  |                    10         |                 | ON  | ON  |                 |
  +-------------------------------+-----------------+-----+-----+-----+-----+-----+
  | Double-Up          Weak       |                             | OFF | OFF |     |
  | Game Difficulty    ...        |                             | ON  | OFF |     |
  |                    ...        |                             | OFF | ON  |     |
  |                    Strong     |                             | ON  | ON  |     |
  +-------------------------------+-----------------------------+-----+-----+-----+
  | OFF Fixed                     |                                         | OFF |
  '-------------------------------+-----------------------------------------+-----'

  .-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switches #3               |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | Key In   1 Coin - 50 Credits  | OFF | OFF | OFF |                             |
  |          1 Coin - 5 Credits   | ON  | OFF | OFF |                             |
  |          1 Coin - 10 Credits  | OFF | ON  | OFF |                             |
  |          1 Coin - 20 Credits  | ON  | ON  | OFF |                             |
  |          1 Coin - 25 Credits  | OFF | OFF | ON  |                             |
  |          1 Coin - 40 Credits  | ON  | OFF | ON  |                             |
  |          1 Coin - 60 Credits  | OFF | ON  | ON  |                             |
  |          1 Coin - 100 Credits | ON  | ON  | ON  |                             |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----------+
  | Coin-A   1 Coin - 5 Credits   |                 | OFF | OFF | OFF |           |
  |          1 Coin - 1 Credit    |                 | ON  | OFF | OFF |           |
  |          1 Coin - 2 Credits   |                 | OFF | ON  | OFF |           |
  |          1 Coin - 10 Credits  |                 | ON  | ON  | OFF |           |
  |          1 Coin - 20 Credits  |                 | OFF | OFF | ON  |           |
  |          1 Coin - 25 Credits  |                 | ON  | OFF | ON  |           |
  |          1 Coin - 40 Credits  |                 | OFF | ON  | ON  |           |
  |          1 Coin - 50 Credits  |                 | ON  | ON  | ON  |           |
  +-------------------------------+-----------------+-----+-----+-----+-----+-----+
  | Credit Limit        5000      |                                   | OFF | OFF |
  |                    10000      |                                   | ON  | OFF |
  |                    20000      |                                   | OFF | ON  |
  |                    30000      |                                   | ON  | ON  |
  '-------------------------------+-----------------------------------+-----+-----'

  .-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switches #4               |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | Coin-B   1 Coin - 50 Credits  | OFF | OFF | OFF | OFF |                       |
  |          1 Coin - 1 Credit    | ON  | OFF | OFF | OFF |                       |
  |          1 Coin - 2 Credits   | OFF | ON  | OFF | OFF |                       |
  |          1 Coin - 4 Credits   | ON  | ON  | OFF | OFF |                       |
  |          1 Coin - 5 Credits   | OFF | OFF | ON  | OFF |                       |
  |          1 Coin - 10 Credits  | ON  | OFF | ON  | OFF |                       |
  |          1 Coin - 20 Credits  | OFF | ON  | ON  | OFF |                       |
  |          1 Coin - 25 Credits  | ON  | ON  | ON  | OFF |                       |
  |          1 Coin - 100 Credits | OFF | OFF | OFF | ON  |                       |
  |          2 Coins - 1 Credit   | ON  | OFF | OFF | ON  |                       |
  |          2 Coins - 5 Credits  | OFF | ON  | OFF | ON  |                       |
  |          4 Coins - 1 Credit   | ON  | ON  | OFF | ON  |                       |
  |          5 Coins - 1 Credit   | OFF | OFF | ON  | ON  |                       |
  |          5 Coins - 2 Credits  | ON  | OFF | ON  | ON  |                       |
  |          10 Coins - 1 Credit  | OFF | ON  | ON  | ON  |                       |
  |          20 Coins - 1 Credit  | ON  | ON  | ON  | ON  |                       |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----------+
  | Max Bet            20         |                       | OFF | OFF |           |
  |                    10         |                       | ON  | OFF |           |
  |                    30         |                       | OFF | ON  |           |
  |                    50         |                       | ON  | ON  |           |
  +-------------------------------+-----------------------+-----+-----+-----+-----+
  | Credit-In Limit    No Limit   |                                   | OFF | OFF |
  |                    1000       |                                   | ON  | OFF |
  |                    2000       |                                   | OFF | ON  |
  |                    5000       |                                   | ON  | ON  |
  '-------------------------------+-----------------------------------+-----+-----'


***********************************************************************************

  Power Supply edge connector Layout...

  .-----------------------+-+--+---------------------------.
  |       Components side |L|PN| Solder side               |
  +-----------------------+-+--+---------------------------+
  |                   GND |A|01| GND                       |
  |                   GND |B|02| GND                       |
  |                   +5V |C|03| +5V                       |
  |                   +5V |D|04| +5V                       |
  |              Lamp BET |E|05| Lamp HOLD 1               |
  |                  +12V |F|06| +12V                      |
  |             Lamp DEAL |H|07| Lamp HOLD 2               |
  |           Lamp CANCEL |J|08| Lamp HOLD 3               |
  |       Lamp TAKE SCORE |K|09| Lamp HOLD 4               |
  |        Lamp DOUBLE UP |L|10| Lamp HOLD 5               |
  '-----------------------+-+--+---------------------------'

  Edge connector Layout...

  .-----------------------+-+--+---------------------------.
  |       Components side |L|PN| Solder side               |
  +-----------------------+-+--+---------------------------+
  |           Out Counter |A|01| AC 100V In                |
  |        Key In Counter |B|02| AC Out to Hopper          |
  |     Coin In Counter A |C|03| Lock Out (100V)           |
  |             Sound GND |D|04| Sound Out                 |
  |        HOLD 5 / SMALL |E|05| Payout                    |
  |                Key In |F|06| HOLD 2                    |
  |                HOLD 3 |H|07| Coin B In                 |
  |                HOLD 4 |J|08| HOLD 1 / BIG              |
  |                       |K|09| RED (TV)                  |
  |                       |L|10| GREEN (TV)                |
  |            TAKE SCORE |M|11| Sync (TV)                 |
  |             DOUBLE UP |N|12| BLUE (TV)                 |
  |             Coin A In |P|13| DEAL                      |
  |                CANCEL |R|14| Analyzer (Books/Settings) |
  |  * SW2 (Clear Meters) |S|15| BET                       |
  |       Coin In Counter |T|16| Out of Fever Lamp         |
  |                ** GND |U|17| GND **                    |
  |                ** GND |V|18| GND **                    |
  |                       |W|19| GND                       |
  |                       |X|20|                           |
  |     Hopper Limited SW |Y|21| Hopper Payout             |
  |                       |Z|22|                           |
  '-----------------------+-+--+---------------------------'

  *   Soft SW2 Extension. Traced (undocumented).
  **  Edge connector pins U, V, 17 and 18 (GND), are tied
      to Power Supply edge connector pins A, B, 01 and 02.

***********************************************************************************

  Book / Settings Mode:
  ---------------------

  First Screen (Books & Meters):

  5C     0         F5C     0
  RF     0         FRF     0
  SF     0         FSF     0
  4C     0         F4C     0
  FH     0         FFH     0
  FL     0         FFL     0
  ST     0         FST     0
  3C     0         F3C     0
  2P     0         F2P     0

  MAIN GAMES               0
  FEVER GAMES              0
  FEVER OUT                0
  S-FEVER GAMES            0
  S-FEVER OUT              0
  TOTAL BET                0
  TOTAL SCORE              0
  SCORE%BET                0%
  W-UP IN                  0
  W-UP OUT                 0
  W-UP OUT%IN              0%
  CREDIT IN                0
  CREDIT OUT               0
  CREDIT OUT%IN            0%
  DOWN COUNT               0
  TOTAL-CREDIT IN          0
  TOTAL-CREDIT OUT         0
  TOTAL-CREDIT IN/OUT      0

  DATA RESET: In this first screen, press the SW2 to clear all data,
  except for the Total Credits meters.


  Second Screen (Settings):

                  MIN                NTL                MAX
  FEVER           --------------------*--------------------
  5 CARD          --------------------*--------------------
  ROYAL           --------------------*--------------------
  STR-FLUSH       --------------------*--------------------
  4 CARD          --------------------*--------------------
  FULL HOUSE      --------------------*--------------------
  FLUSH           --------------------*--------------------
  STRAIGHT        --------------------*--------------------
  3 CARD          --------------------*--------------------
  2 PAIR          --------------------*--------------------
  NORMAL JKR      --------------------*--------------------
  FEVER JKR       --------------------*--------------------
  DEAL SPEED      --------------------*--------------------
                  difficult appear              easy appear

  HOLD 1 (Up)
  HOLD 2 (Down)
  HOLD 3 (Left)
  HOLD 4 (Right)
  HOLD 5 (Next)


  Third Screen (Settings):

  Memory Switches...

  DEALER                OFF  ON         <--- Girl Dealer.
  FVR-ANIMATION         OFF  ON         <--- Fever Screen animation.
  DUP-NUDE              OFF  ON         <--- Double-Up Nude.
  SLOW-ACTION           OFF  ON         <--- Speed of Cards.
  KEY-LOCK              OFF  ON         <--- Lock settings leaving only Books. Other revisions may have "JQKA-1PAIR" (Jacks or Better).
  FVR-SLOT              OFF  ON         <--- Fever's number slot.
  AMUSEMENT-MODE        OFF  ON         <--- Self explanatory....

  1ST BET           1  5  10  20  30    <--- First Bet.
  CNT BET           1  5  10  20  30    <--- Second Bet.

  BACK-RGB R        0 1 2 3 4 5 6 7     <--- Background R component (0 = light, 7 = strong)
  BACK-RGB G        0 1 2 3 4 5 6 7     <--- Background G component (0 = light, 7 = strong)
  BACK-RGB B        0 1 2 3 4 5 6 7     <--- Background B component (0 = light, 7 = strong)

  BACK-PATTERN      0 1 2 3 4 5 6       <--- Background Pattern (0 = None, 1-6 available when dealer is off)

  HOLD 1 (Up)
  HOLD 2 (Down)
  HOLD 3 (Off)
  HOLD 4 (On)
  HOLD 5 (Exit)


  Fourth Screen (DIP switches test):

  DIP Switch test...

  DIPSW-1    00000000    00000000
  DIPSW-2    00000000    00000000
  DIPSW-3    00000000    00000000
  DIPSW-4    00000000    00000000

  HOLD 5 (Exit)

  Created of DIP Switch (Green)       <--- ?????
  Test Mode of DIP Switch (White)     <--- Show on the fly each DIP switch state.

***********************************************************************************

  To Do:

  - Workaround to hide the unused last column (16 pixels), but forced on the 6845.
  - Find the input that unlocks the "KEY-LOCK" mode in the settings.
  - Proper flip mode.
  - Resistors Network.

**********************************************************************************/

#define MASTER_CLOCK	XTAL_12MHz
#define CPU_CLOCK		(MASTER_CLOCK / 2)	/* 6 MHz, measured */
#define OKI_CLOCK		(MASTER_CLOCK / 8)	/* 1.5 MHz, measured */
#define CRTC_CLOCK		(MASTER_CLOCK / 16)	/* 750 kHz, measured */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "majorpkr.lh"


class majorpkr_state : public driver_device
{
public:
	majorpkr_state(const machine_config &mconfig, device_type type, const char *tag) : driver_device(mconfig, type, tag), oki(*this, "oki") { }

	int m_mux_data;
	int	m_palette_bank;
	int m_vram_bank;
	int m_flip_state;

	tilemap_t    *m_bg_tilemap,	*m_fg_tilemap;

	UINT8 *m_videoram;

	required_device<okim6295_device> oki;

};


/*************************
*     Video Hardware     *
*************************/

static TILE_GET_INFO( bg_get_tile_info )
{
	majorpkr_state *state = machine.driver_data<majorpkr_state>();

	int code = state->m_videoram[0x800 + 2 * tile_index] + (state->m_videoram[0x800 + 2 * tile_index + 1] << 8);

	SET_TILE_INFO(
			0,
			(code & 0x1fff),
			code >> 13,
			0);
}

static TILE_GET_INFO( fg_get_tile_info )
{
	majorpkr_state *state = machine.driver_data<majorpkr_state>();

	int code = state->m_videoram[2 * tile_index] + (state->m_videoram[2 * tile_index + 1] << 8);

	SET_TILE_INFO(
			1,
			(code & 0x07ff),
			code >> 13,
			(code & (1 << 12)) ? (TILE_FLIPX|TILE_FLIPY) : 0);
}


static VIDEO_START(majorpkr)
{
	majorpkr_state *state = machine.driver_data<majorpkr_state>();

	state->m_bg_tilemap = tilemap_create(machine, bg_get_tile_info, tilemap_scan_rows, 16, 8, 36, 28);
	state->m_fg_tilemap = tilemap_create(machine, fg_get_tile_info, tilemap_scan_rows, 16, 8, 36, 28);
	tilemap_set_transparent_pen(state->m_fg_tilemap, 0);

	state->machine().generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 4 * 0x800);

	state->m_videoram = auto_alloc_array(machine, UINT8, 4 * 0x800); /* 2 banks in use, but the game clears 4 after boot ... */
}


static SCREEN_UPDATE(majorpkr)
{
	majorpkr_state *state = screen->machine().driver_data<majorpkr_state>();

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));

	rectangle custom_clip;

	/* The following custom_clip is to exclude the last char column (unused)
       form the render. We need more proof about how the video is working.
    */
	custom_clip.min_x=cliprect->min_x;
	custom_clip.max_x=cliprect->max_x-16;
	custom_clip.min_y=cliprect->min_y;
	custom_clip.max_y=cliprect->max_y;

	tilemap_draw(bitmap, &custom_clip, state->m_bg_tilemap, 0, 0);
	tilemap_draw(bitmap, &custom_clip, state->m_fg_tilemap, 0, 0);

	if (state->m_flip_state == 1)
	{
		tilemap_set_flip(state->m_bg_tilemap, TILEMAP_FLIPX | TILEMAP_FLIPY);
		tilemap_set_flip(state->m_fg_tilemap, TILEMAP_FLIPX | TILEMAP_FLIPY);
	}

	return 0;
}


/******************************
*         R/W Handlers        *
******************************/

static WRITE8_HANDLER(rom_bank_w)
{
	memory_set_bank(space->machine(), "rom_bank", data & 0x3);
}


static WRITE8_HANDLER(palette_bank_w)
{
	majorpkr_state *state = space->machine().driver_data<majorpkr_state>();

	state->m_palette_bank=data;
}


static READ8_HANDLER(paletteram_r)
{
	majorpkr_state *state = space->machine().driver_data<majorpkr_state>();
	return state->machine().generic.paletteram.u8[state->m_palette_bank * 0x800 + offset];
}

static WRITE8_HANDLER(paletteram_w)
{
	majorpkr_state *state = space->machine().driver_data<majorpkr_state>();
	state->machine().generic.paletteram.u8[state->m_palette_bank * 0x800 + offset] = data;

	offset >>= 1;
	int color = state->machine().generic.paletteram.u8[state->m_palette_bank * 0x800 + offset * 2] + state->machine().generic.paletteram.u8[state->m_palette_bank * 0x800 + offset * 2 + 1] * 256;
	palette_set_color(space->machine(), offset + state->m_palette_bank * 256 * 4, MAKE_RGB(pal5bit(color >> 5), pal5bit(color >> 10), pal5bit(color)));
}


static WRITE8_HANDLER(vram_bank_w)
{
	majorpkr_state *state = space->machine().driver_data<majorpkr_state>();
	state->m_vram_bank = data;
}

static READ8_HANDLER(vram_r)
{
	majorpkr_state *state = space->machine().driver_data<majorpkr_state>();
	return state->m_videoram[state->m_vram_bank * 0x800 + offset];
}

static WRITE8_HANDLER(vram_w)
{
	majorpkr_state *state = space->machine().driver_data<majorpkr_state>();
	state->m_videoram[state->m_vram_bank * 0x800 + offset] = data;

	if (state->m_vram_bank == 0)
	{
		tilemap_mark_tile_dirty(state->m_fg_tilemap, offset >> 1);
	}
	else
	{
		if (state->m_vram_bank == 1)
		{
			tilemap_mark_tile_dirty(state->m_bg_tilemap, offset >> 1);
		}
		else
		{
			//logerror("accessing vram bank %d (offset = %x , data = %x )\n", state->m_vram_bank, offset,data);
		}
	}
}

static WRITE8_HANDLER(vidreg_w)
{
	majorpkr_state *state = space->machine().driver_data<majorpkr_state>();

/*  If bit6 is active, the screen is drawn upside down.
    (also 0xfc and 0x11 are written to the CRTC registers 0xc0 and 0xd0)
    So, the CRTC display start address = 0xfc11
*/
	if (data & 0x40)
	{
		/* upside down screen */
		state->m_flip_state = 1;
	}

/*  If bit6 is not active, the screen is drawn normally.
    (also 0x00 is written to the CRTC registers 0xc0 and 0xd0)
    So, the CRTC display start address = 0x0000
*/
	else
	{
		/* normal screen */
		state->m_flip_state = 0;
	}
}


/***** Multiplexed Ports *****/

static READ8_HANDLER( mux_port_r )
{
	majorpkr_state *state = space->machine().driver_data<majorpkr_state>();

	switch( (state->m_mux_data & 0xf0) )		/* 00-10-20-30-0F-1F-2F-3F */
	{
		case 0x00: return input_port_read(space->machine(), "DSW1");	/* confirmed */
		case 0x10: return input_port_read(space->machine(), "DSW2");	/* confirmed */
		case 0x20: return input_port_read(space->machine(), "DSW3");	/* confirmed */
		case 0x30: return input_port_read(space->machine(), "DSW4");	/* confirmed */
	}

	return 0xff;
}

static READ8_HANDLER( mux_port2_r )
{
	majorpkr_state *state = space->machine().driver_data<majorpkr_state>();

	if ((state->m_mux_data & 0x0f) == 4)
	{
		return input_port_read(space->machine(), "IN0-1");
	}
	else
	{
		return input_port_read(space->machine(), "IN0-0");
	}
}

static WRITE8_HANDLER( mux_sel_w )
{
	majorpkr_state *state = space->machine().driver_data<majorpkr_state>();

	state->m_mux_data = data;	/* 00-10-20-30-0F-1F-2F-3F */
}


/*************************
*    Lamps and Pulses    *
*************************/

static WRITE8_HANDLER( lamps_a_w )
{
/*  Lamps - Array A.

    - bits -
    7654 3210
    ---- ---x   Hold 1 lamp.
    ---- --x-   Hold 2 lamp.
    ---- -x--   Hold 3 lamp.
    ---- x---   Hold 4 lamp.
    ---x ----   Hold 5 lamp.
    --x- ----   Big lamp.
    -x-- ----   Small lamp.
    x--- ----   Unknown.
*/
	output_set_lamp_value(0, (data) & 1);		/* Lamp 0: Hold 1 */
	output_set_lamp_value(1, (data >> 1) & 1);	/* Lamp 1: Hold 2 */
	output_set_lamp_value(2, (data >> 2) & 1);	/* Lamp 2: Hold 3 */
	output_set_lamp_value(3, (data >> 3) & 1);	/* Lamp 3: Hold 4 */
	output_set_lamp_value(4, (data >> 4) & 1);	/* Lamp 4: Hold 5 */
	output_set_lamp_value(5, (data >> 5) & 1);	/* Lamp 5: Big or Small (need identification) */
	output_set_lamp_value(6, (data >> 6) & 1);	/* Lamp 6: Big or Small (need identification) */

	if (data & 0x80)
		logerror("Lamps A: Write to 13h: %02x\n", data);
}

static WRITE8_HANDLER( lamps_b_w )
{
/*  Lamps - Array B.

    - bits -
    7654 3210
    ---- ---x   Bet lamp.
    ---- --x-   Draw lamp.
    ---- -x--   Cancel lamp.
    ---- x---   Take lamp.
    ---x ----   Double-Up lamp.
    --x- ----   Fever lamp.
    xx-- ----   Unknown.
*/
	output_set_lamp_value(7, (data) & 1);		/* Lamp 7: Bet */
	output_set_lamp_value(8, (data >> 1) & 1);	/* Lamp 8: Draw */
	output_set_lamp_value(9, (data >> 2) & 1);	/* Lamp 9: Cancel */
	output_set_lamp_value(10, (data >> 3) & 1);	/* Lamp 10: Take */
	output_set_lamp_value(11, (data >> 4) & 1);	/* Lamp 11: D-UP */
	output_set_lamp_value(12, (data >> 5) & 1);	/* Lamp 12: Fever */

	if (data & 0xc0)
		logerror("Lamps B: Write to 14h: %02x\n", data);
}

static WRITE8_HANDLER( pulses_w )
{
/*  Pulses...

    - bits -
    7654 3210
    ---- ---x   Credits Out mech counter.
    ---- --x-   Credits 3 mech counter.
    ---- -x--   Credits 1 mech counter.
    ---- x---   Credits 2 mech counter.
    ---x ----   Watchdog? (constant writes).
    xxx- ----   Unknown.
*/
	coin_counter_w(space->machine(), 3, data & 0x01);		/* Credits Out (all) */
	coin_counter_w(space->machine(), 2, data & 0x02);		/* Credits 3 */
	coin_counter_w(space->machine(), 0, data & 0x04);		/* Credits 1 */
	coin_counter_w(space->machine(), 1, data & 0x08);		/* Credits 2 */

	if (data & 0xe0)
		logerror("Pulse: Write to 10h: %02x\n", data);
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xe7ff) AM_ROM AM_ROMBANK("rom_bank")
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf000, 0xf7ff) AM_READWRITE(paletteram_r, paletteram_w)	/* 4*4 palettes - 4 banks? */
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(vram_r, vram_w)				/* two tilemaps - 2 banks? */
ADDRESS_MAP_END

/*
  00  W ---> ROM bank.
  01  W ---> RAM bank.
  02  W ---> RAM bank.

  10 R  ---> Regular inputs (holds) multiplexed with credits out.
  10  W ---> Writes a kind of watchdog (bit4) and mech counters pulses (bits 0-1-2-3).
  11 R  ---> Read input port (remaining controls).
  11  W ---> Writes mux selector.
  12 R  ---> Read coin/credits port.
  12  W ---> Video registers: Normal or Up Down screen (bit6 off/on). Also writes 0xFC & 0x11 at CRTC offsets 0x0C & 0x0D (0's if normal).
  13 R  ---> Multiplexed port for DIP switches banks.
  13  W ---> Lamps out (array a)
  14 R  ---> Freeze. Switch in bit0 that pause the game when active. Just once active, the code loops till the bit resets.
  14  W ---> Lamps out (array b).

  50 RW ---> OKI6295 (RW)

  60  W ---> PSG SN76489/96 initialization routines.
             (Maybe a leftover for different hardware).
*/
static ADDRESS_MAP_START( portmap, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(rom_bank_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(palette_bank_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(vram_bank_w)

	AM_RANGE(0x10, 0x10) AM_READ(mux_port2_r)	/* muxed set of controls */
	AM_RANGE(0x10, 0x10) AM_WRITE(pulses_w)		/* kind of watchdog on bit4... mech counters on bits 0-1-2-3 */
	AM_RANGE(0x11, 0x11) AM_READ_PORT("IN1")
	AM_RANGE(0x11, 0x11) AM_WRITE(mux_sel_w)	/* multiplexer selector */
	AM_RANGE(0x12, 0x12) AM_READ_PORT("IN2")
	AM_RANGE(0x12, 0x12) AM_WRITE(vidreg_w)		/* video registers: normal or up down screen */
	AM_RANGE(0x13, 0x13) AM_READ(mux_port_r)	/* all 4 DIP switches banks multiplexed */
	AM_RANGE(0x13, 0x13) AM_WRITE(lamps_a_w)	/* lamps a out */
	AM_RANGE(0x14, 0x14) AM_READ_PORT("TEST")	/* "freeze" switch */
	AM_RANGE(0x14, 0x14) AM_WRITE(lamps_b_w)	/* lamps b out */

	AM_RANGE(0x30, 0x30) AM_DEVWRITE_MODERN("crtc", mc6845_device, address_w)
	AM_RANGE(0x31, 0x31) AM_DEVREADWRITE_MODERN("crtc", mc6845_device, register_r, register_w)

	AM_RANGE(0x50, 0x50) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0x60, 0x60) AM_WRITENOP	/* leftover from a PSG SN76489/96? */
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( majorpkr )
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )	/* muxed with Key Out */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH )  PORT_NAME("Big")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )   PORT_NAME("Small / DIP Test (In Book Mode)")

	PORT_START("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT )	/* muxed with HOLD 2 */
	PORT_BIT( 0xfd, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_BET )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE )       PORT_NAME("Book/Settings Mode")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_CODE(KEYCODE_1_PAD)  PORT_NAME("UNK 1-PAD")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_CODE(KEYCODE_2_PAD)  PORT_NAME("UNK 2-PAD")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYIN)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE )       PORT_CODE(KEYCODE_0)      PORT_NAME("SW2 (Clear Short-Term Meters)")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_CODE(KEYCODE_4_PAD)  PORT_NAME("UNK 4-PAD")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE )       PORT_CODE(KEYCODE_E)      PORT_NAME("Manual Payout")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_CODE(KEYCODE_5_PAD)  PORT_NAME("UNK 5-PAD")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER )         PORT_CODE(KEYCODE_6_PAD)  PORT_NAME("UNK 6-PAD")

	PORT_START("DSW1")	/* multiplexed x4 & inverted */
	PORT_DIPNAME( 0x01, 0x00, "OFF Fixed" )			PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Screen Direction" )	PORT_DIPLOCATION("DSW1:2")	/* Activates bit6 at port 0x10 & change CRTC registers */
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, "Up Down" )
	PORT_DIPNAME( 0x04, 0x00, "Payout" )			PORT_DIPLOCATION("DSW1:3")	/* Hopper: No / Yes (in the manual) */
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPSETTING(    0x04, "Manual" )
	PORT_DIPNAME( 0x08, 0x00, "Hopper SW Active" )	PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Low ) )
	PORT_DIPNAME( 0x10, 0x00, "Auto Max Bet" )		PORT_DIPLOCATION("DSW1:5")	/* Coin Payout: Payout SW / Automatic (in the manual) */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Hold Cancel" )		PORT_DIPLOCATION("DSW1:6")	/* Inverse from the manual */
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Auto Hold" )			PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Fever Mode" )		PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW2")	/* multiplexed x4 & inverted */
	PORT_DIPNAME( 0x07, 0x00, "Payout %" )					PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "65%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x18, 0x00, "Min Bet Size for Fever" )	PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPNAME( 0x60, 0x00, "D-UP Game Difficulty" )		PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(    0x00, "0 (Weak)" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x60, "3 (Strong)" )
	PORT_DIPNAME( 0x80, 0x00, "OFF Fixed" )					PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")	/* multiplexed x4 & inverted */
	PORT_DIPNAME( 0x07, 0x00, "Key In" )				PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(    0x01, "5 Credits / Pulse" )
	PORT_DIPSETTING(    0x02, "10 Credits / Pulse" )
	PORT_DIPSETTING(    0x03, "20 Credits / Pulse" )
	PORT_DIPSETTING(    0x04, "25 Credits / Pulse" )
	PORT_DIPSETTING(    0x05, "40 Credits / Pulse" )
	PORT_DIPSETTING(    0x00, "50 Credits / Pulse" )
	PORT_DIPSETTING(    0x06, "60 Credits / Pulse" )
	PORT_DIPSETTING(    0x07, "100 Credits / Pulse" )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("DSW3:4,5,6")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x30, "1 Coin/40 Credits" )
	PORT_DIPSETTING(    0x38, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0xc0, 0x00, "Credit Limit" )			PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x40, "10000" )
	PORT_DIPSETTING(    0x80, "20000" )
	PORT_DIPSETTING(    0xc0, "30000" )

	PORT_START("DSW4")	/* multiplexed x4 & inverted */
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("DSW4:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "20 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0e, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0d, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x30, 0x00, "Max Bet" )				PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPSETTING(    0x20, "30" )
	PORT_DIPSETTING(    0x30, "50" )
	PORT_DIPNAME( 0xc0, 0x00, "Credit-In Limit" )		PORT_DIPLOCATION("DSW4:7,8")
	PORT_DIPSETTING(    0x00, "No Limit" )
	PORT_DIPSETTING(    0x40, "1000" )
	PORT_DIPSETTING(    0x80, "2000" )
	PORT_DIPSETTING(    0xc0, "5000" )

	PORT_START("TEST")
	PORT_DIPNAME( 0x01, 0x00, "Freeze" )		/* Freeze the execution */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tilelayout =
{
	16, 8,
	RGN_FRAC(1,2),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{
		0*8, 0*8+RGN_FRAC(1,2), 1*8, 1*8+RGN_FRAC(1,2), 2*8, 2*8+RGN_FRAC(1,2), 3*8, 3*8+RGN_FRAC(1,2),
		4*8, 4*8+RGN_FRAC(1,2), 5*8, 5*8+RGN_FRAC(1,2), 6*8, 6*8+RGN_FRAC(1,2), 7*8, 7*8+RGN_FRAC(1,2)
	},
	{ 0*8*8, 1*8*8, 2*8*8, 3*8*8, 4*8*8, 5*8*8, 6*8*8, 7*8*8 },
	8*8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( majorpkr )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 8*256, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0, 8 )
GFXDECODE_END


/***********************
*    CRTC Interface    *
***********************/

static const mc6845_interface mc6845_intf =
{
	"screen",		/* screen we are acting on */
	16,				/* number of pixels per video memory address */
	NULL,			/* before pixel update callback */
	NULL,			/* row update callback */
	NULL,			/* after pixel update callback */
	DEVCB_NULL,		/* callback for display state changes */
	DEVCB_NULL,		/* callback for cursor state changes */
	DEVCB_NULL,		/* HSYNC callback */
	DEVCB_NULL,		/* VSYNC callback */
	NULL			/* update address callback */
};


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( majorpkr, majorpkr_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK)	/* 6 MHz */
	MCFG_CPU_PROGRAM_MAP(map)
	MCFG_CPU_IO_MAP(portmap)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(52.786)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE((47+1)*16, (36+1)*8)				/* through CRTC registers: 768 x 296 */
	MCFG_SCREEN_VISIBLE_AREA(0, (36*16)-1, 0, (28*8)-1) /* through CRTC registers: 560(+16) x 224 */

	MCFG_GFXDECODE(majorpkr)
	MCFG_PALETTE_LENGTH(0x100 * 16)

	MCFG_VIDEO_START(majorpkr)
	MCFG_SCREEN_UPDATE(majorpkr)

	MCFG_MC6845_ADD("crtc", MC6845, CRTC_CLOCK, mc6845_intf) /* verified */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", OKI_CLOCK, OKIM6295_PIN7_HIGH)	/* clock frequency & pin 7 verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( majorpkr )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "6_27c512_823b.bin", 0x00000, 0x0e000, CRC(a3d5475e) SHA1(cb41508b55da8b8c658a2f2ccc6ebda09db29040)  )
	ROM_CONTINUE(0x10000,0x2000)

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "p1_27c040_7d3b.bin", 0x00000, 0x80000, CRC(67299eff) SHA1(34d3d8baf08dea495b699dd63272b445e2acb42d) )
	ROM_LOAD( "p2_27c040_6039.bin", 0x80000, 0x80000, CRC(2d68b177) SHA1(01c934e0383991f2208b915cc5015463a8b6a8fd) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "3_27c010_af18.bin", 0x00000, 0x20000, CRC(54452bb8) SHA1(9d13c17b85dd0185ba64fc6f90425e0c75363960) )
	ROM_LOAD( "4_27c010_92d6.bin", 0x20000, 0x20000, CRC(2e1e0972) SHA1(729dba2ef6ae8a7299c7ceb38835bebb0c42d28e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "5_27c020_8630.bin", 0x00000, 0x40000, CRC(4843858e) SHA1(27629829cf7753d7801a6eb42bb77ca2a467bebd) )
ROM_END


/*************************
*      Driver Init       *
*************************/

static DRIVER_INIT( majorpkr )
{
	UINT8 * ROM = (UINT8 *)machine.region("maincpu")->base();
	memory_configure_bank(machine, "rom_bank", 0, 4, &ROM[0x10000], 0x800);
}


/*************************
*      Game Drivers      *
*************************/

/*     YEAR  NAME      PARENT  MACHINE   INPUT     INIT      ROT    COMPANY       FULLNAME             FLAGS  LAYOUT */
GAMEL( 1994, majorpkr, 0,      majorpkr, majorpkr, majorpkr, ROT0, "PAL System", "Major Poker (v2.0)", 0,     layout_majorpkr )
