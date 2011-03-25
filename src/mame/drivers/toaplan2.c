/*****************************************************************************

        ToaPlan      game hardware from 1991 - 1994
        Raizing/8ing game hardware from 1993 onwards
        -------------------------------------------------
        Driver by: Quench and Yochizo

   Raizing games and Truxton 2 are heavily dependent on the Raine source -
   many thanks to Richard Bush and the Raine team. [Yochizo]



Supported games:

    Name        Board No      Maker         Game name
    ----------------------------------------------------------------------------
    tekipaki    TP-020        Toaplan       Teki Paki
    ghox        TP-021        Toaplan       Ghox (Spinner with single up/down axis control)
    ghoxj       TP-021        Toaplan       Ghox (8-Way Joystick controls)
    dogyuun     TP-022        Toaplan       Dogyuun
    dogyuunk    TP-022        Toaplan       Dogyuun (Licensed to Unite Trading For Korea)
    kbash       TP-023        Toaplan       Knuckle Bash
    kbash2      bootleg       Toaplan       Knuckle Bash 2
    truxton2    TP-024        Toaplan       Truxton 2 / Tatsujin Oh
    pipibibs    TP-025        Toaplan       Pipi & Bibis
    whoopee     TP-025        Toaplan       Whoopee
    pipibibi    bootleg       Toaplan       Pipi & Bibis
    fixeight    TP-026        Toaplan       FixEight
    fixeighb    bootleg       Toaplan       FixEight
    grindstm    TP-027        Toaplan       Grind Stormer  (1992)
    grindsta    TP-027        Toaplan       Grind Stormer  (1992) (older)
    vfive       TP-027        Toaplan       V-V  (V-Five)  (1993 - Japan only)
    batsugun    TP-030        Toaplan       Batsugun
    batugnsp    TP-030        Toaplan       Batsugun  (Special Version)
    snowbro2    ??????        Toaplan       Snow Bros. 2 - With New Elves

    sstriker    RA-MA7893-01  Raizing       Sorcer Striker
    mahoudai    RA-MA7893-01  Raizing       Mahou Daisakusen
    kingdmgp    RA-MA9402-03  Raizing/8ing  Kingdom Grandprix
    shippumd    RA-MA9402-03  Raizing/8ing  Shippu Mahou Daisakusen
    bgaregga    RA9503        Raizing/8ing  Battle Garegga (World - Sat Feb 3 1996)
    bgareghk    RA9503        Raizing/8ing  Battle Garegga (Hong Kong (and Austria?) - Sat Feb 3 1996)
    bgaregtw    RA9503        Raizing/8ing  Battle Garegga (Taiwan and Germany - Thu Feb 1 1996)
    bgaregnv    RA9503        Raizing/8ing  Battle Garegga - New Version (Hong Kong (and Austria?) - Sat Mar 2 1996)
    bgaregt2    RA9503        Raizing/8ing  Battle Garegga - Type 2 (World - Sat Mar 2 1996)
    bgaregcn    RA9503        Raizing/8ing  Battle Garegga - Type 2 (China (and Denmark?) - Tue Apr 2 1996)
    batrider    RA9704        Raizing/8ing  Armed Police Batrider - B Version (Japan - Fri Feb 13 1998)
    batridra    RA9704        Raizing/8ing  Armed Police Batrider (Japan - Mon Dec 22 1997)
    batridrk    RA9704        Raizing/8ing  Armed Police Batrider (Korea - Fri Feb 13 1998)
    bbakraid    ET68-V99      8ing          Battle Bakraid (Japan - Wed Apr 7th, 1999)
    bbakradu    ET68-V99      8ing          Battle Bakraid - Unlimited Version (Japan - Tue Jun 8th, 1999)

    SET NOTES:

    sstriker - The region jumper is read with this set but even when set to Japan you get all English
               text etc. which seems a little odd. However, this has been verified correct on two boards.

    kingdmgp - Might be a bootleg / hack, some of the tiles needed for the credits screen have been
               stripped out, doesn't seem very professional, since it's rare and should probably only
               have a different graphics rom, it's nearly impossible for us to verify. A lot of boards
               being sold as 'Kingdom Grand Prix' are infact conversions using Neill Corlett's hack.

            ** The above two look like genuine Korean release boards, Raizing probably just missed a few things

    bgaregga - The clones have fewer types of enemy bullets (not fewer bullets, just fewer types!)
               and Stage Edit is disabled - the dipswitch is still listed in service mode but it
               doesn't do anything. In addition to these changes, bgaregt2 has no third button!
               Instead of being able to change the formation of your options with a button press,
               each of the selectable ships has a different, fixed option formation.

    batrider - The original release was marketed as a two button game, but actually needed a third button
               to fully use some of the ships' weaponry! This error was rectified in the B Version
               (which is fully playable with either two or three buttons).

    bbakraid - The Unlimited Version can display more score digits (as players managed to counter stop
               the original within days of its release!) and adds a "team" mode ala Batrider where you
               select a different ship for each of your three lives.

    dogyuunk - PCB says "TOAPLAN CO.,LTD. TP-022-1 MADE IN KOREA". It contains 3 custom Toaplan chips
               (2 gfx chips and TS-002-MACH sound MCU). Uses original Toaplan-badged MASKROMs for GFX
               and sound and original 'TP-022 01 TOAPLAN' sticker on program EPROM.


 ****************************************************************************
 * Battle Garegga and Armed Police Batrider have secret characters.         *
 * Try to input the following commands to use them.                         *
 * ======================================================================== *
 * Battle Garegga                                                           *
 *      After inserting a coin (pushing a credit button), input             *
 *      UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B  C  START        *
 *      then you can use Mahou Daisakusen characters.                       *
 *                                                                          *
 * ------------------------------------------------------------------------ *
 * Armed Police Batrider                                                    *
 *      After inserting a coin (pushing a credit button), input             *
 *      UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B  START           *
 *      then you can use Mahou Daisakusen and Battle Garegga characters.    *
 *                                                                          *
 ****************************************************************************


 ############################################################################
 # Battle Bakraid has unlocking codes to gain access to extra players       #
 # and game features.                                                       #
 # Special thanks go to the 'R8ZING Shooter Tribute' page for finding       #
 # and publishing this info.                                                #
 #                                                                          #
 # ======================================================================== #
 #      PLAYER SELECT: PHASE 2                                              #
 # Result:  3 more fighter planes available:                                #
 # Code:    UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B  Start       #
 # Conditions:                                                              #
 #      1. Start from the title screen                                      #
 #      2. Insert Coin                                                      #
 #      3. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 # Example: 12,up,11,up,10,down,9,down,8,left,7,right,6.left,5,r..          #
 # After entering the [B] button a chime should sound. Phase 2 unlocked!    #
 #                                                                          #
 # ------------------------------------------------------------------------ #
 #      PLAYER SELECT: PHASE 3                                              #
 # Result:  2 more fighter planes available:                                #
 # Code:    UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  B  A  Start       #
 # Conditions:                                                              #
 #      1. Start from the title screen                                      #
 #      2. Insert Coin                                                      #
 #      3. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 # Important: The entering of this code has to be finished before the       #
 # counter passes 10 ! To do so, you will have to start after coin          #
 # insertion, right before it starts to count:                              #
 # Example: up,19,up,18,down,17,down,16,left,15,right,14.left,..            #
 # After entering the [A] button a chime should sound. Phase 3 unlocked!    #
 #                                                                          #
 # ------------------------------------------------------------------------ #
 #      TEAM EDIT: ENABLE (Unlimited Version only)                          #
 # Result:  Unlocks the 'team edit' feature to select a team of different   #
 #          ships. See selection secrets on how to enter the team edit mode.#
 # Code:    UP  DOWN  UP  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B  Start       #
 # Conditions:                                                              #
 #      1. Start from the title screen                                      #
 #      2. Insert Coin                                                      #
 #      3. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 # Important: This code hast to be entered so that the counter is at 0 when #
 # you press the final button [B]. To do so, start after second 9:          #
 # Example: 9,up,8,down,7,up,6,down,5,left,4,right,3,left,2,right,1,A,0,B   #
 # After entering the [B] button a chime should sound. Team edit unlocked!  #
 #                                                                          #
 # ------------------------------------------------------------------------ #
 #      SPECIAL COURSE: ENABLE                                              #
 # Result:  Unlocks the boss mode, a game mode where you fight the bosses   #
 #          only.                                                           #
 # Code:    UP  DOWN  UP  DOWN  LEFT  RIGHT  LEFT  RIGHT  B  A  Start       #
 # Conditions:                                                              #
 #      1. Start from the title screen                                      #
 #      2. Hold [C] button                                                  #
 #      3. Insert Coin                                                      #
 #      4. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 #      5. Release [C] button                                               #
 # After entering the [A] button a chime should sound. Special course       #
 # unlocked!                                                                #
 ############################################################################



*************** Hardware Info ***************

CPU:
 MC68000P10
 TMP68HC000N-16

Sound CPU/MCU:
 HD647180X0FS6 (Hitachi Z180 Compatible CPU with internal 16k ROM)
 Z84C0006PEC (Z80)
 NEC V25

Sound Chips:
 YM3812
 YM2151
 YM2151 + YM3014
 YM2151 + M6295
 YM2151 + M6295 + M6295
 YMZ280B-F + YAC516-E (Digital to Analog Converter)


Graphics Custom 208pin QFP:
 GP9001 L7A0498 TOA PLAN

Found on....
Fixeight
Grind Stormer
Truxton II
Ghox
Armed Police Batrider
Battle Garegga
Mahou Daisakusen
Battle Bakraid
(emulated in video/gp9001.c)

*********************************************************************

Game status:

Teki Paki                      Working, but no sound. Missing sound MCU dump. Chip is protected. It's a QFP80 Hitachi HD647180.
Ghox                           Working, but no sound. Missing sound MCU dump. It's a QFP80 Hitachi HD647180.
Dogyuun                        Working, MCU type is a NEC V25. Chip is a PLCC94 stamped 'TS-002-MACH'.*
Knuckle Bash                   Working, MCU type is a NEC V25. Chip is a PLCC94 stamped 'TS-004-DASH'.*
Truxton 2                      Working.
Pipi & Bibis                   Working.
Whoopee                        Working, but no sound. Missing sound MCU dump. It's a Hitachi HD647180.
Pipi & Bibis (Ryouta Kikaku)   Working.
FixEight                       Working, MCU type is a NEC V25. Chip is a PLCC94 stamped 'TS-001-TURBO'
FixEight bootleg               Working. One unknown ROM (same as pipibibi one). Region hardcoded to Korea (@ $4d8)
Grind Stormer / VFive          Working, MCU type is a NEC V25. Chip is a PLCC94 stamped 'TS-007-SPY'.*
Batsugun / Batsugun Sp'        Working, MCU type is a NEC V25. Chip is a PLCC94 stamped 'TS-007-SPY'.*
Snow Bros. 2                   Working.
Mahou Daisakusen               Working.
Shippu Mahou Daisakusen        Working.
Battle Garegga                 Working.
Armed Police Batrider          Working.
Battle Bakraid                 Working.


* Some PCBs use another version stamped 'NITRO' which is the same chip type. MACH, DASH and SPY seem to be the same chip (same encryption table)
  Batsugun has the CPU hooked up in non-encrypted mode.

Notes:
    See Input Port definition header below, for instructions
      on how to enter pause/slow motion modes.
    Code at $20A26 forces territory to Japan in V-Five. Some stuff
      NOP'd at reset vector, and NEC V25 CPU post test is skipped (bootleg?)

To Do / Unknowns:
    - Whoopee/Teki Paki sometimes tests bit 5 of the territory port
        just after testing for vblank. Why ?
    - Fix top character text layer (implement the line position table).
    - Priority problem on 2nd player side of selection screen in FixEight (both original and bootleg)
    - Fixeight bootleg text in sound check mode does not display properly
        with the CPU set to 10MHz (ok at 16MHz). Possible error in video_count_r routine.

    - Need to sort out the video status register.
    - Find out how exactly how sound CPU communication really works in bgaregga/batrider/bbakraid
        current emulation seems to work (plays all sounds), but there are still some unknown reads/writes


*****************************************************************************/


#include "emu.h"
#include "cpu/z180/z180.h"
#include "cpu/nec/nec.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "machine/nmk112.h"
#include "sound/2151intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "sound/ymz280b.h"
#include "includes/toaplan2.h"
#include "rendlay.h"

static MACHINE_RESET(batsugun);
static READ16_HANDLER( batsugun_share_r );
static WRITE16_HANDLER( batsugun_share_w );

/***************************************************************************
  Initialisation handlers
***************************************************************************/

static void toaplan2_reset(device_t *device)
{
	toaplan2_state *state = device->machine->driver_data<toaplan2_state>();

	if (state->sub_cpu != NULL)
		cpu_set_input_line(state->sub_cpu, INPUT_LINE_RESET, PULSE_LINE);
}

static MACHINE_RESET( toaplan2 )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->mcu_data = 0x00;

	/*
      All games execute a RESET instruction on init, presumably to reset the sound CPU.
      This is important for games with common RAM; the RAM test will fail
      when leaving service mode if the sound CPU is not reset.
    */
	m68k_set_reset_callback(machine->device("maincpu"), toaplan2_reset);
}

static MACHINE_RESET( ghox )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	MACHINE_RESET_CALL(toaplan2);
	state->old_p1_paddle_h = 0;
	state->old_p2_paddle_h = 0;
}

static MACHINE_RESET( dogyuun )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	MACHINE_RESET_CALL(batsugun);
	state->mcu_data = 0xffaa;
}

static MACHINE_RESET( vfive )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	MACHINE_RESET_CALL(batsugun);
	state->mcu_data = 0xffaa;
}

static MACHINE_RESET( bgaregga )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();
	UINT8 *Z80 = (UINT8 *)machine->region("audiocpu")->base();

	// Set Z80 bank switch - default bank is 2
	state->current_bank = 4;
	memory_configure_bank(machine, "bank1", 0, 16, Z80, 0x4000);
	memory_set_bank(machine, "bank1", 4);

	MACHINE_RESET_CALL(toaplan2);
}

static void register_state_save(running_machine *machine)
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->save_item(NAME(state->mcu_data));
	state->save_item(NAME(state->video_status));
	state->save_item(NAME(state->old_p1_paddle_h));
	state->save_item(NAME(state->old_p2_paddle_h));
	state->save_item(NAME(state->current_bank));
	state->save_item(NAME(state->z80_busreq));
}

static DRIVER_INIT( T2_Z80 )		/* init_t2_Z80(); */
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->sub_cpu_type = CPU_2_Z80;
	state->sub_cpu = machine->device("audiocpu");
	register_state_save(machine);
}

static DRIVER_INIT( T2_Z180 )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->sub_cpu_type = CPU_2_HD647180;
	state->sub_cpu = machine->device("mcu");
	register_state_save(machine);
}

static DRIVER_INIT( T2_V25 )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->sub_cpu_type = CPU_2_V25;
	state->sub_cpu = machine->device("audiocpu");
	state->v25_reset_line = 0x20;
	register_state_save(machine);
}

static DRIVER_INIT( T2_noZ80 )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->sub_cpu_type = CPU_2_NONE;
	state->sub_cpu = NULL;
	register_state_save(machine);
}

static DRIVER_INIT( fixeight )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->sub_cpu_type = CPU_2_V25;
	state->sub_cpu = machine->device("audiocpu");
	state->v25_reset_line = 0x08;

	register_state_save(machine);
}

static DRIVER_INIT( fixeighb )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();
	UINT16 *bgdata = (UINT16 *)machine->region("maincpu")->base();

	memory_set_bankptr(machine, "bank1", &bgdata[0x40000]); /* $80000 - $fffff */
	state->sub_cpu_type = CPU_2_NONE;
	state->sub_cpu = NULL;
	register_state_save(machine);
}

static DRIVER_INIT( vfive )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->sub_cpu_type = CPU_2_V25;
	state->sub_cpu = machine->device("audiocpu");
	state->v25_reset_line = 0x10;
	register_state_save(machine);
}

static DRIVER_INIT( pipibibsbl )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();
	int A;
	int oldword, newword;
	UINT16 *pipibibi_68k_rom = (UINT16 *)(machine->region("maincpu")->base());

	/* unscramble the 68K ROM data. */

	for (A = 0; A < (0x040000/2); A+=4)
	{
		newword = 0;
		oldword = pipibibi_68k_rom[A];
		newword |= ((oldword & 0x0001) << 9);
		newword |= ((oldword & 0x0002) << 14);
		newword |= ((oldword & 0x0004) << 8);
		newword |= ((oldword & 0x0018) << 1);
		newword |= ((oldword & 0x0020) << 9);
		newword |= ((oldword & 0x0040) << 7);
		newword |= ((oldword & 0x0080) << 5);
		newword |= ((oldword & 0x0100) << 3);
		newword |= ((oldword & 0x0200) >> 1);
		newword |= ((oldword & 0x0400) >> 8);
		newword |= ((oldword & 0x0800) >> 10);
		newword |= ((oldword & 0x1000) >> 12);
		newword |= ((oldword & 0x6000) >> 7);
		newword |= ((oldword & 0x8000) >> 12);
		pipibibi_68k_rom[A] = newword;

		newword = 0;
		oldword = pipibibi_68k_rom[A+1];
		newword |= ((oldword & 0x0001) << 8);
		newword |= ((oldword & 0x0002) << 12);
		newword |= ((oldword & 0x0004) << 5);
		newword |= ((oldword & 0x0008) << 11);
		newword |= ((oldword & 0x0010) << 2);
		newword |= ((oldword & 0x0020) << 10);
		newword |= ((oldword & 0x0040) >> 1);
		newword |= ((oldword & 0x0080) >> 7);
		newword |= ((oldword & 0x0100) >> 4);
		newword |= ((oldword & 0x0200) << 0);
		newword |= ((oldword & 0x0400) >> 7);
		newword |= ((oldword & 0x0800) >> 1);
		newword |= ((oldword & 0x1000) >> 10);
		newword |= ((oldword & 0x2000) >> 2);
		newword |= ((oldword & 0x4000) >> 13);
		newword |= ((oldword & 0x8000) >> 3);
		pipibibi_68k_rom[A+1] = newword;

		newword = 0;
		oldword = pipibibi_68k_rom[A+2];
		newword |= ((oldword & 0x000f) << 4);
		newword |= ((oldword & 0x00f0) >> 4);
		newword |= ((oldword & 0x0100) << 3);
		newword |= ((oldword & 0x0200) << 1);
		newword |= ((oldword & 0x0400) >> 1);
		newword |= ((oldword & 0x0800) >> 3);
		newword |= ((oldword & 0x1000) << 3);
		newword |= ((oldword & 0x2000) << 1);
		newword |= ((oldword & 0x4000) >> 1);
		newword |= ((oldword & 0x8000) >> 3);
		pipibibi_68k_rom[A+2] = newword;

		newword = 0;
		oldword = pipibibi_68k_rom[A+3];
		newword |= ((oldword & 0x000f) << 4);
		newword |= ((oldword & 0x00f0) >> 4);
		newword |= ((oldword & 0x0100) << 7);
		newword |= ((oldword & 0x0200) << 5);
		newword |= ((oldword & 0x0400) << 3);
		newword |= ((oldword & 0x0800) << 1);
		newword |= ((oldword & 0x1000) >> 1);
		newword |= ((oldword & 0x2000) >> 3);
		newword |= ((oldword & 0x4000) >> 5);
		newword |= ((oldword & 0x8000) >> 7);
		pipibibi_68k_rom[A+3] = newword;
	}

	state->sub_cpu_type = CPU_2_Z80;
	state->sub_cpu = machine->device("audiocpu");
	register_state_save(machine);
}

static DRIVER_INIT( batrider )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->sndirq_line = 4;
	state->sub_cpu_type = CPU_2_Z80;
	state->sub_cpu = machine->device("audiocpu");
	register_state_save(machine);
}

static DRIVER_INIT( bbakraid )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->unlimited_ver = 0;
	state->sndirq_line = 2;
	state->sub_cpu_type = CPU_2_Z80;
	state->sub_cpu = machine->device("audiocpu");
	register_state_save(machine);
}

static DRIVER_INIT( bbakradu )
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	state->unlimited_ver = 1;
	state->sndirq_line = 2;
	state->sub_cpu_type = CPU_2_Z80;
	state->sub_cpu = machine->device("audiocpu");
	register_state_save(machine);
}


/***************************************************************************
  Toaplan games
***************************************************************************/




static TIMER_CALLBACK( toaplan2_raise_irq )
{
	cputag_set_input_line(machine, "maincpu", param, HOLD_LINE);
}

static void toaplan2_vblank_irq(running_machine *machine, int irq_line)
{
	/* the IRQ appears to fire at line 0xe6 */
	machine->scheduler().timer_set(machine->primary_screen->time_until_pos(0xe6), FUNC(toaplan2_raise_irq), irq_line);
}

static INTERRUPT_GEN( toaplan2_vblank_irq1 ) { toaplan2_vblank_irq(device->machine, 1); }
static INTERRUPT_GEN( toaplan2_vblank_irq2 ) { toaplan2_vblank_irq(device->machine, 2); }
static INTERRUPT_GEN( toaplan2_vblank_irq4 ) { toaplan2_vblank_irq(device->machine, 4); }

static READ16_HANDLER( video_count_r )
{
	/* +---------+---------+--------+---------------------------+ */
	/* | /H-Sync | /V-Sync | /Blank |       Scanline Count      | */
	/* | Bit 15  | Bit 14  | Bit 8  |  Bit 7-0 (count from #EF) | */
	/* +---------+---------+--------+---------------------------+ */
	/*************** Control Signals are active low ***************/

	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
	int hpos = space->machine->primary_screen->hpos();
	int vpos = space->machine->primary_screen->vpos();

	state->video_status = 0xff00;						/* Set signals inactive */

	vpos = (vpos + 15) % 262;

	bool hblank, vblank;

	hblank = (hpos > 325) && (hpos < 380);
	vblank = (vpos >= 247) && (vpos <= 250);

	if (hblank)
		state->video_status &= ~0x8000;
	if (vblank)
		state->video_status &= ~0x4000;
	if (vblank || hblank) // ?? Dogyuun is too slow if this is wrong
		state->video_status &= ~0x0100;
	if (vpos < 256)
		state->video_status |= (vpos & 0xff);
	else
		state->video_status |= 0xff;

//  logerror("VC: vpos=%04x hpos=%04x VBL=%04x\n",vpos,hpos,space->machine->primary_screen->vblank());

	return state->video_status;
}

static WRITE8_HANDLER( toaplan2_coin_w )
{
	/* +----------------+------ Bits 7-5 not used ------+--------------+ */
	/* | Coin Lockout 2 | Coin Lockout 1 | Coin Count 2 | Coin Count 1 | */
	/* |     Bit 3      |     Bit 2      |     Bit 1    |     Bit 0    | */

	if (data & 0x0f)
	{
		coin_lockout_w( space->machine, 0, ((data & 4) ? 0 : 1) );
		coin_lockout_w( space->machine, 1, ((data & 8) ? 0 : 1) );
		coin_counter_w( space->machine, 0, (data & 1) );
		coin_counter_w( space->machine, 1, (data & 2) );
	}
	else
	{
		coin_lockout_global_w(space->machine, 1); /* Lock all coin slots */
	}
	if (data & 0xe0)
	{
		logerror("Writing unknown upper bits (%02x) to coin control\n",data);
	}
}

static WRITE16_HANDLER( toaplan2_coin_word_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan2_coin_w(space, offset, data & 0xff);
	}
	if (ACCESSING_BITS_8_15 && (data & 0xff00) )
	{
		logerror("Writing unknown upper MSB command (%04x) to coin control\n",data & 0xff00);
	}
}


static WRITE16_HANDLER( toaplan2_v25_coin_word_w )
{
	logerror("toaplan2_v25_coin_word_w %04x\n",data);

	if (ACCESSING_BITS_0_7)
	{
		toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

		toaplan2_coin_w(space, offset, data & 0x0f);

		cpu_set_input_line(state->sub_cpu, INPUT_LINE_RESET,  (data & state->v25_reset_line) ? CLEAR_LINE : ASSERT_LINE);
	}
	if (ACCESSING_BITS_8_15 && (data & 0xff00) )
	{
		logerror("Writing unknown upper MSB command (%04x) to coin control\n",data & 0xff00);
	}
}

static WRITE16_HANDLER( shippumd_coin_word_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan2_coin_w(space, offset, data & 0xff);
		space->machine->device<okim6295_device>("oki")->set_bank_base(((data & 0x10) >> 4) * 0x40000);
	}
	if (ACCESSING_BITS_8_15 && (data & 0xff00) )
	{
		logerror("Writing unknown upper MSB command (%04x) to coin control\n",data & 0xff00);
	}
}

static READ16_HANDLER( z80_shared_r )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	return state->z80_shared_ram[offset] & 0xff;
}

static WRITE16_HANDLER( z80_shared_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

		state->z80_shared_ram[offset] = data & 0xff;
	}
}

static WRITE16_HANDLER( toaplan2_hd647180_cpu_w )
{
	/* Command sent to secondary CPU. Support for HD647180 will be
       required when a ROM dump becomes available for this hardware */

	if (ACCESSING_BITS_0_7)
	{
		toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

		if (state->sub_cpu_type == CPU_2_Z80)			/* Whoopee */
		{
			state->z80_shared_ram[0] = data & 0xff;
		}
		else										/* Teki Paki */
		{
			state->mcu_data = data & 0xff;
			logerror("PC:%08x Writing command (%04x) to secondary CPU shared port\n", cpu_get_previouspc(space->cpu), state->mcu_data);
		}
	}
}

static CUSTOM_INPUT( c2map_r )
{
	toaplan2_state *state = field->port->machine->driver_data<toaplan2_state>();

	/* For Teki Paki hardware */
	/* bit 4 high signifies secondary CPU is ready */
	/* bit 5 is tested low before V-Blank bit ??? */
	switch (state->sub_cpu_type)
	{
		case CPU_2_Z80:			state->mcu_data = state->z80_shared_ram[0]; break; /* Whoopee */
		case CPU_2_HD647180:	state->mcu_data = 0xff; break;					  /* Teki Paki */
		default:				state->mcu_data = 0x00; break;
	}

	return (state->mcu_data == 0xff) ? 0x01 : 0x00;
}

static READ16_HANDLER( ghox_p1_h_analog_r )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
	INT8 value, new_value;

	new_value = input_port_read(space->machine, "PAD1");
	if (new_value == state->old_p1_paddle_h) return 0;
	value = new_value - state->old_p1_paddle_h;
	state->old_p1_paddle_h = new_value;
	return value;
}

static READ16_HANDLER( ghox_p2_h_analog_r )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
	INT8 value, new_value;

	new_value = input_port_read(space->machine, "PAD2");
	if (new_value == state->old_p2_paddle_h) return 0;
	value = new_value - state->old_p2_paddle_h;
	state->old_p2_paddle_h = new_value;
	return value;
}

static READ16_HANDLER( ghox_mcu_r )
{
	return 0xff;
}

static WRITE16_HANDLER( ghox_mcu_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
		UINT16 *toaplan2_shared_ram16 = state->shared_ram16;

		state->mcu_data = data;
		if ((data >= 0xd0) && (data < 0xe0))
		{
			offset = ((data & 0x0f) * 2) + (0x38 / 2);
			toaplan2_shared_ram16[offset  ] = 0x0005;	/* Return address for */
			toaplan2_shared_ram16[offset-1] = 0x0056;	/*   RTS instruction */
		}
		else
		{
			logerror("PC:%08x Writing %08x to HD647180 cpu shared ram status port\n", cpu_get_previouspc(space->cpu), state->mcu_data);
		}
		toaplan2_shared_ram16[0x56 / 2] = 0x004e;	/* Return a RTS instruction */
		toaplan2_shared_ram16[0x58 / 2] = 0x0075;

		if (data == 0xd3)
		{
		toaplan2_shared_ram16[0x56 / 2] = 0x003a;	//  move.w  d1,d5
		toaplan2_shared_ram16[0x58 / 2] = 0x0001;
		toaplan2_shared_ram16[0x5a / 2] = 0x0008;	//  bclr.b  #0,d5
		toaplan2_shared_ram16[0x5c / 2] = 0x0085;
		toaplan2_shared_ram16[0x5e / 2] = 0x0000;
		toaplan2_shared_ram16[0x60 / 2] = 0x0000;
		toaplan2_shared_ram16[0x62 / 2] = 0x00cb;	//  muls.w  #3,d5
		toaplan2_shared_ram16[0x64 / 2] = 0x00fc;
		toaplan2_shared_ram16[0x66 / 2] = 0x0000;
		toaplan2_shared_ram16[0x68 / 2] = 0x0003;
		toaplan2_shared_ram16[0x6a / 2] = 0x0090;	//  sub.w   d5,d0
		toaplan2_shared_ram16[0x6c / 2] = 0x0045;
		toaplan2_shared_ram16[0x6e / 2] = 0x00e5;	//  lsl.b   #2,d1
		toaplan2_shared_ram16[0x70 / 2] = 0x0009;
		toaplan2_shared_ram16[0x72 / 2] = 0x004e;	//  rts
		toaplan2_shared_ram16[0x74 / 2] = 0x0075;
		}
	}
}

static READ16_HANDLER( ghox_shared_ram_r )
{
	/* Ghox 68K reads data from MCU shared RAM and writes it to main RAM.
       It then subroutine jumps to main RAM and executes this code.
       Here, we're just returning a RTS instruction for now.
       See above ghox_mcu_w routine.

       Offset $56 and $58 are accessed from around PC:0F814

       Offset $38 and $36 are accessed from around PC:0DA7C
       Offset $3c and $3a are accessed from around PC:02E3C
       Offset $40 and $3E are accessed from around PC:103EE
       Offset $44 and $42 are accessed from around PC:0FB52
       Offset $48 and $46 are accessed from around PC:06776
    */

	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	return state->shared_ram16[offset] & 0xff;
}

static WRITE16_HANDLER( ghox_shared_ram_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

		state->shared_ram16[offset] = data & 0xff;
	}
}

static WRITE16_HANDLER( fixeight_subcpu_ctrl_w )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	cpu_set_input_line(state->sub_cpu, INPUT_LINE_RESET, (data & state->v25_reset_line) ? CLEAR_LINE : ASSERT_LINE);
}

static WRITE16_DEVICE_HANDLER( oki_bankswitch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		downcast<okim6295_device *>(device)->set_bank_base((data & 1) * 0x40000);
	}
}

static WRITE16_DEVICE_HANDLER( fixeighb_oki_bankswitch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 7;
		if (data <= 4)
		{
			UINT8 *fixeighb_oki = device->machine->region("oki")->base();
			memcpy(&fixeighb_oki[0x30000], &fixeighb_oki[(data * 0x10000) + 0x40000], 0x10000);
		}
	}
}

static READ8_HANDLER( v25_dswa_read )
{
	return input_port_read(space->machine, "DSWA") ^ 0xff;
}

static READ8_HANDLER( v25_dswb_read )
{
	return input_port_read(space->machine, "DSWB") ^ 0xff;
}

static READ8_HANDLER( v25_jmpr_read )
{
	return input_port_read(space->machine, "JMPR") ^ 0xff;
}


/***************************************************************************
  Raizing games
***************************************************************************/


static WRITE16_HANDLER( bgaregga_soundlatch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

		soundlatch_w(space, offset, data & 0xff);
		cpu_set_input_line(state->sub_cpu, 0, HOLD_LINE);
	}
}


static READ8_HANDLER( bgaregga_E01D_r )
{
	// the Z80 reads this address during its IRQ routine,
	// and reads the soundlatch only if the lowest bit is clear.
	return 0;
}


static WRITE8_HANDLER( bgaregga_E00C_w )
{
	// the Z80 writes here after reading the soundlatch.
	// I would think that this was an acknowledge latch like
	// batrider and bbakraid have, except that on the 68000 side
	// there's no corresponding read...
}


static WRITE8_HANDLER( bgaregga_bankswitch_w )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
	int bank = (data & 0x0f) - 10 + 4;

	if (bank != state->current_bank)
	{
		state->current_bank = bank;
		memory_set_bank(space->machine, "bank1", bank);
	}
}


// bgaregga and batrider don't actually have a NMK112, but rather a GAL
// programmed to bankswitch the sound ROMs in a similar fashion.
// it may not be a coincidence that the composer and sound designer for
// these two games, Manabu "Santaruru" Namiki, came to Raizing from NMK...

static WRITE8_HANDLER( raizing_okim6295_bankselect_0 )
{
	nmk112_device *nmk112 = space->machine->device<nmk112_device>("nmk112");

	nmk112_okibank_w(nmk112, 0,  data		& 0x0f);	// chip 0 bank 0
	nmk112_okibank_w(nmk112, 1, (data >> 4)	& 0x0f);	// chip 0 bank 1
}

static WRITE8_HANDLER( raizing_okim6295_bankselect_1 )
{
	nmk112_device *nmk112 = space->machine->device<nmk112_device>("nmk112");

	nmk112_okibank_w(nmk112, 2,  data		& 0x0f);	// chip 0 bank 2
	nmk112_okibank_w(nmk112, 3, (data >> 4)	& 0x0f);	// chip 0 bank 3
}

static WRITE8_HANDLER( raizing_okim6295_bankselect_2 )
{
	nmk112_device *nmk112 = space->machine->device<nmk112_device>("nmk112");

	nmk112_okibank_w(nmk112, 4,  data		& 0x0f);	// chip 1 bank 0
	nmk112_okibank_w(nmk112, 5, (data >> 4)	& 0x0f);	// chip 1 bank 1
}

static WRITE8_HANDLER( raizing_okim6295_bankselect_3 )
{
	nmk112_device *nmk112 = space->machine->device<nmk112_device>("nmk112");

	nmk112_okibank_w(nmk112, 6,  data		& 0x0f);	// chip 1 bank 2
	nmk112_okibank_w(nmk112, 7, (data >> 4)	& 0x0f);	// chip 1 bank 3
}


static WRITE8_HANDLER( batrider_bankswitch_w )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
	int bank = data & 0x0f;

	bank = (bank > 1) ? bank + 2 : bank;

	if (bank != state->current_bank)
	{
		state->current_bank = bank;
		memory_set_bank(space->machine, "bank1", bank);
	}
}


static READ16_HANDLER( batrider_z80_busack_r )
{
	/* Bit 1 returns the status of BUSAK from the Z80.
       BUSRQ is activated via bit 0x10 on the NVRAM write port.
       These accesses are made when the 68K wants to read the Z80
       ROM code. Failure to return the correct status incurrs a Sound Error.
    */

	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	return state->z80_busreq;			/* Loop BUSRQ to BUSAK */
}


static WRITE16_HANDLER( batrider_z80_busreq_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

		state->z80_busreq = (data & 0xff);
	}
}


static READ16_HANDLER( raizing_z80rom_r )
{
	UINT8 *Z80_ROM_test = (UINT8 *)space->machine->region("audiocpu")->base();

	if (offset < 0x8000)
		return Z80_ROM_test[offset] & 0xff;

	return Z80_ROM_test[offset + 0x8000] & 0xff;
}


// these two latches are always written together, via a single move.l instruction
static WRITE16_HANDLER( batrider_soundlatch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

		soundlatch_w(space, offset, data & 0xff);
		cpu_set_input_line(state->sub_cpu, INPUT_LINE_NMI, ASSERT_LINE);
	}
}


static WRITE16_HANDLER( batrider_soundlatch2_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

		soundlatch2_w(space, offset, data & 0xff);
		cpu_set_input_line(state->sub_cpu, INPUT_LINE_NMI, ASSERT_LINE);
	}
}


static WRITE16_HANDLER( raizing_unknown_w )
{
	// the 68K writes here when it wants a sound acknowledge IRQ from the Z80
	// for bbakraid this is on every sound command; for batrider, only on certain commands
}


static WRITE16_HANDLER( raizing_clear_sndirq_w )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	// not sure whether this is correct
	// the 68K writes here during the sound IRQ handler, and nowhere else...
	cputag_set_input_line(space->machine, "maincpu", state->sndirq_line, CLEAR_LINE);
}


static WRITE8_HANDLER( raizing_sndirq_w )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	// if raizing_clear_sndirq_w() is correct, should this be ASSERT_LINE?
	cputag_set_input_line(space->machine, "maincpu", state->sndirq_line, HOLD_LINE);
}


static WRITE8_HANDLER( raizing_clear_nmi_w )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	cpu_set_input_line(state->sub_cpu, INPUT_LINE_NMI, CLEAR_LINE);
}


/*###################### Battle Bakraid ##############################*/

/* EEPROM contents with Battle Bakraid Unlimited version features unlocked */
static const UINT8 bbakraid_unlim_default[512] = {
	0xc2,0x49,0x00,0x07,0xa1,0x20,0x2a,0x2a,0x2a,0x90,0x90,0x90,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x06,0x1a,0x80,0x2a,0x2a,0x2a,0x94,
	0x94,0x94,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x04,
	0x93,0xe0,0x2a,0x2a,0x2a,0x98,0x98,0x98,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x02,0x00,0x03,0x0d,0x40,0x2a,0x2a,0x2a,0x9c,0x9c,0x9c,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x01,0x86,0xa0,0x2a,0x2a,
	0x2a,0xa0,0xa0,0xa0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
	0x00,0x07,0xa1,0x20,0x2b,0x2b,0x2b,0x90,0x90,0x90,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x06,0x1a,0x80,0x2b,0x2b,0x2b,0x94,0x94,0x94,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x04,0x93,0xe0,
	0x2b,0x2b,0x2b,0x98,0x98,0x98,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x02,0x00,0x03,0x0d,0x40,0x2b,0x2b,0x2b,0x9c,0x9c,0x9c,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x01,0x86,0xa0,0x2b,0x2b,0x2b,0xa0,
	0xa0,0xa0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x07,
	0xa1,0x20,0x23,0x23,0x23,0x90,0x90,0x90,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x02,0x00,0x06,0x1a,0x80,0x23,0x23,0x23,0x94,0x94,0x94,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x04,0x93,0xe0,0x23,0x23,
	0x23,0x98,0x98,0x98,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
	0x00,0x03,0x0d,0x40,0x23,0x23,0x23,0x9c,0x9c,0x9c,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x01,0x86,0xa0,0x23,0x23,0x23,0xa0,0xa0,0xa0,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x07,0xa1,0x20,
	0x3f,0x3f,0x3f,0x90,0x90,0x90,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x02,0x00,0x06,0x1a,0x80,0x3f,0x3f,0x3f,0x94,0x94,0x94,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x04,0x93,0xe0,0x3f,0x3f,0x3f,0x98,
	0x98,0x98,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x03,
	0x0d,0x40,0x3f,0x3f,0x3f,0x9c,0x9c,0x9c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x02,0x00,0x01,0x86,0xa0,0x3f,0x3f,0x3f,0xa0,0xa0,0xa0,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x04,0xc2,0x49,0xc2,0x49,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0xcf,0x00,0x04,0x00,0x00,0x21,0xd5,
	0x00,0x05,0x06,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xc2,0x49
};



static const eeprom_interface bbakraid_93C66_intf =
{
	/* Pin 6 of the 93C66 is connected to Gnd!
       So it's configured for 512 bytes */

	9,			// address bits
	8,			// data bits
	"*110",		// read         110 aaaaaaaaa
	"*101",		// write        101 aaaaaaaaa dddddddd
	"*111",		// erase        111 aaaaaaaaa
	"*10000xxxxxxx",// lock         100x 00xxxx
	"*10011xxxxxxx",// unlock       100x 11xxxx
//  "*10001xxxx",   // write all    1 00 01xxxx dddddddd
//  "*10010xxxx"    // erase all    1 00 10xxxx
};

static READ16_HANDLER( bbakraid_nvram_r )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();
	eeprom_device *eeprom = space->machine->device<eeprom_device>("eeprom");

	/* Bit 1 returns the status of BUSAK from the Z80.
       BUSRQ is activated via bit 0x10 on the NVRAM write port.
       These accesses are made when the 68K wants to read the Z80
       ROM code. Failure to return the correct status incurrs a Sound Error.
    */

	int data;
	data  = ((eeprom_read_bit(eeprom) & 0x01) << 4);
	data |= ((state->z80_busreq >> 4) & 0x01);	/* Loop BUSRQ to BUSAK */

	return data;
}


static WRITE16_HANDLER( bbakraid_nvram_w )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	if (data & ~0x001f)
		logerror("CPU #0 PC:%06X - Unknown EEPROM data being written %04X\n",cpu_get_pc(space->cpu),data);

	if ( ACCESSING_BITS_0_7 )
		input_port_write(space->machine, "EEPROMOUT", data, 0xff);

	state->z80_busreq = data & 0x10;	/* see bbakraid_nvram_r above */
}


static void bbakraid_irqhandler(device_t *device, int state)
{
	/* Not used ???  Connected to a test pin (TP082) */
	logerror("YMZ280 is generating an interrupt. State=%08x\n",state);
}


static INTERRUPT_GEN( bbakraid_snd_interrupt )
{
	cpu_set_input_line(device, 0, HOLD_LINE);
}



static ADDRESS_MAP_START( tekipaki_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x020000, 0x03ffff) AM_ROM							/* extra for Whoopee */
	AM_RANGE(0x080000, 0x082fff) AM_RAM
	AM_RANGE(0x0c0000, 0x0c0fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x140000, 0x14000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x180000, 0x180001) AM_READ_PORT("DSWA")
	AM_RANGE(0x180010, 0x180011) AM_READ_PORT("DSWB")
	AM_RANGE(0x180020, 0x180021) AM_READ_PORT("SYS")
	AM_RANGE(0x180030, 0x180031) AM_READ_PORT("JMPR")			/* CPU 2 busy and Territory Jumper block */
	AM_RANGE(0x180040, 0x180041) AM_WRITE(toaplan2_coin_word_w)	/* Coin count/lock */
	AM_RANGE(0x180050, 0x180051) AM_READ_PORT("IN1")
	AM_RANGE(0x180060, 0x180061) AM_READ_PORT("IN2")
	AM_RANGE(0x180070, 0x180071) AM_WRITE(toaplan2_hd647180_cpu_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( ghox_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x040001) AM_READ(ghox_p2_h_analog_r)	/* Paddle 2 */
	AM_RANGE(0x080000, 0x083fff) AM_RAM
	AM_RANGE(0x0c0000, 0x0c0fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x100000, 0x100001) AM_READ(ghox_p1_h_analog_r)	/* Paddle 1 */
	AM_RANGE(0x140000, 0x14000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x180000, 0x180001) AM_READWRITE(ghox_mcu_r, ghox_mcu_w)	/* really part of shared RAM */
	AM_RANGE(0x180006, 0x180007) AM_READ_PORT("DSWA")
	AM_RANGE(0x180008, 0x180009) AM_READ_PORT("DSWB")
	AM_RANGE(0x180010, 0x180011) AM_READ_PORT("SYS")
	AM_RANGE(0x18000c, 0x18000d) AM_READ_PORT("IN1")
	AM_RANGE(0x18000e, 0x18000f) AM_READ_PORT("IN2")
	AM_RANGE(0x180500, 0x180fff) AM_READWRITE(ghox_shared_ram_r, ghox_shared_ram_w) AM_BASE_MEMBER(toaplan2_state, shared_ram16)
	AM_RANGE(0x181000, 0x181001) AM_WRITE(toaplan2_coin_word_w)
	AM_RANGE(0x18100c, 0x18100d) AM_READ_PORT("JMPR")
ADDRESS_MAP_END


static ADDRESS_MAP_START( dogyuun_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x200010, 0x200011) AM_READ_PORT("IN1")
	AM_RANGE(0x200014, 0x200015) AM_READ_PORT("IN2")
	AM_RANGE(0x200018, 0x200019) AM_READ_PORT("SYS")
	AM_RANGE(0x20001c, 0x20001d) AM_WRITE(toaplan2_v25_coin_word_w)
	AM_RANGE(0x210000, 0x21ffff) AM_READWRITE( batsugun_share_r, batsugun_share_w )
	/***** The following locations in 0x30000x are for video controller 1 ******/
	AM_RANGE(0x300000, 0x30000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	/***** The following loctions in 0x50000x are for video controller 2 ******/
	AM_RANGE(0x500000, 0x50000d) AM_DEVREADWRITE("gp9001vdp1", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x700000, 0x700001) AM_READ(video_count_r)			/* test bit 8 */
ADDRESS_MAP_END


static ADDRESS_MAP_START( kbash_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x200000, 0x200fff) AM_READWRITE( batsugun_share_r, batsugun_share_w )
	AM_RANGE(0x208010, 0x208011) AM_READ_PORT("IN1")
	AM_RANGE(0x208014, 0x208015) AM_READ_PORT("IN2")
	AM_RANGE(0x208018, 0x208019) AM_READ_PORT("SYS")
	AM_RANGE(0x20801c, 0x20801d) AM_WRITE(toaplan2_coin_word_w)
	AM_RANGE(0x300000, 0x30000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x700000, 0x700001) AM_READ(video_count_r)			/* test bit 8 */
ADDRESS_MAP_END


static ADDRESS_MAP_START( kbash2_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x104000, 0x10401f) AM_RAM			/* Sound related? */
	AM_RANGE(0x200000, 0x200001) AM_NOP			/* Left over from original code - Sound Number write, Status read */
	AM_RANGE(0x200002, 0x200003) AM_WRITENOP	/* Left over from original code - Reset Sound */
	AM_RANGE(0x200004, 0x200005) AM_READ_PORT("DSWA")
	AM_RANGE(0x200008, 0x200009) AM_READ_PORT("DSWB")
	AM_RANGE(0x20000c, 0x20000d) AM_READ_PORT("JMPR")
	AM_RANGE(0x200010, 0x200011) AM_READ_PORT("IN1")
	AM_RANGE(0x200014, 0x200015) AM_READ_PORT("IN2")
	AM_RANGE(0x200018, 0x200019) AM_READ_PORT("SYS")
	AM_RANGE(0x200020, 0x200021) AM_DEVREADWRITE8_MODERN("oki2", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x200024, 0x200025) AM_DEVREADWRITE8_MODERN("oki1", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x200028, 0x200029) AM_DEVWRITE("oki1", oki_bankswitch_w)
	AM_RANGE(0x20002c, 0x20002d) AM_READ(video_count_r)
	AM_RANGE(0x300000, 0x30000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( truxton2_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x200000, 0x20000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x300000, 0x300fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x400000, 0x401fff) AM_READWRITE(toaplan2_txvideoram16_r, toaplan2_txvideoram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16, tx_vram_size)
	AM_RANGE(0x402000, 0x4021ff) AM_READWRITE(toaplan2_txvideoram16_offs_r, toaplan2_txvideoram16_offs_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16_offs, tx_offs_vram_size)
	AM_RANGE(0x402200, 0x402fff) AM_RAM
	AM_RANGE(0x403000, 0x4031ff) AM_READWRITE(toaplan2_txscrollram16_r, toaplan2_txscrollram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txscrollram16, tx_scroll_vram_size)
	AM_RANGE(0x403200, 0x403fff) AM_RAM
	AM_RANGE(0x500000, 0x50ffff) AM_READWRITE(toaplan2_tx_gfxram16_r, toaplan2_tx_gfxram16_w) AM_BASE_MEMBER(toaplan2_state, tx_gfxram16)
	AM_RANGE(0x600000, 0x600001) AM_READ(video_count_r)
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("DSWA")
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("DSWB")
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("JMPR")
	AM_RANGE(0x700006, 0x700007) AM_READ_PORT("IN1")
	AM_RANGE(0x700008, 0x700009) AM_READ_PORT("IN2")
	AM_RANGE(0x70000a, 0x70000b) AM_READ_PORT("SYS")
	AM_RANGE(0x700010, 0x700011) AM_DEVREADWRITE8_MODERN("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x700014, 0x700017) AM_DEVREADWRITE8("ymsnd", ym2151_r, ym2151_w, 0x00ff)
	AM_RANGE(0x70001e, 0x70001f) AM_WRITE(toaplan2_coin_word_w)		/* Coin count/lock */
ADDRESS_MAP_END


static ADDRESS_MAP_START( pipibibs_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x082fff) AM_RAM
	AM_RANGE(0x0c0000, 0x0c0fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x140000, 0x14000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)

	AM_RANGE(0x190000, 0x190fff) AM_READWRITE(z80_shared_r, z80_shared_w)
	AM_RANGE(0x19c01c, 0x19c01d) AM_WRITE(toaplan2_coin_word_w)		/* Coin count/lock */
	AM_RANGE(0x19c020, 0x19c021) AM_READ_PORT("DSWA")
	AM_RANGE(0x19c024, 0x19c025) AM_READ_PORT("DSWB")
	AM_RANGE(0x19c028, 0x19c029) AM_READ_PORT("JMPR")
	AM_RANGE(0x19c02c, 0x19c02d) AM_READ_PORT("SYS")
	AM_RANGE(0x19c030, 0x19c031) AM_READ_PORT("IN1")
	AM_RANGE(0x19c034, 0x19c035) AM_READ_PORT("IN2")
ADDRESS_MAP_END

// odd scroll registers
static ADDRESS_MAP_START( pipibibi_bootleg_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x082fff) AM_RAM
	AM_RANGE(0x083000, 0x0837ff) AM_DEVREADWRITE("gp9001vdp0", pipibibi_bootleg_spriteram16_r, pipibibi_bootleg_spriteram16_w)	/* SpriteRAM */
	AM_RANGE(0x083800, 0x087fff) AM_RAM				/* SpriteRAM (unused) */
	AM_RANGE(0x0c0000, 0x0c0fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x120000, 0x120fff) AM_RAM				/* Copy of SpriteRAM ? */
	//AM_RANGE(0x13f000, 0x13f001) AM_WRITENOP        /* ??? */
	AM_RANGE(0x180000, 0x182fff) AM_DEVREADWRITE("gp9001vdp0", pipibibi_bootleg_videoram16_r, pipibibi_bootleg_videoram16_w)		/* TileRAM */
	AM_RANGE(0x188000, 0x18800f) AM_DEVWRITE("gp9001vdp0", pipibibi_bootleg_scroll_w)
	AM_RANGE(0x190002, 0x190003) AM_READ(z80_shared_r)	/* Z80 ready ? */
	AM_RANGE(0x190010, 0x190011) AM_WRITE(z80_shared_w)	/* Z80 task to perform */
	AM_RANGE(0x19c01c, 0x19c01d) AM_WRITE(toaplan2_coin_word_w)	/* Coin count/lock */
	AM_RANGE(0x19c020, 0x19c021) AM_READ_PORT("DSWA")
	AM_RANGE(0x19c024, 0x19c025) AM_READ_PORT("DSWB")
	AM_RANGE(0x19c028, 0x19c029) AM_READ_PORT("JMPR")
	AM_RANGE(0x19c02c, 0x19c02d) AM_READ_PORT("SYS")
	AM_RANGE(0x19c030, 0x19c031) AM_READ_PORT("IN1")
	AM_RANGE(0x19c034, 0x19c035) AM_READ_PORT("IN2")
ADDRESS_MAP_END


/* this one is rather different to the other v25 based ones, shared ram is moved for example */
static ADDRESS_MAP_START( fixeight_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("IN1")
	AM_RANGE(0x200004, 0x200005) AM_READ_PORT("IN2")
	AM_RANGE(0x200008, 0x200009) AM_READ_PORT("IN3")
	AM_RANGE(0x200010, 0x200011) AM_READ_PORT("SYS")
	AM_RANGE(0x20001c, 0x20001d) AM_WRITE(toaplan2_coin_word_w)	/* Coin count/lock */
	AM_RANGE(0x280000, 0x28ffff) AM_READWRITE( batsugun_share_r, batsugun_share_w )
	AM_RANGE(0x300000, 0x30000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x500000, 0x501fff) AM_READWRITE(toaplan2_txvideoram16_r, toaplan2_txvideoram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16, tx_vram_size)
	AM_RANGE(0x502000, 0x5021ff) AM_READWRITE(toaplan2_txvideoram16_offs_r, toaplan2_txvideoram16_offs_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16_offs, tx_offs_vram_size)
	AM_RANGE(0x503000, 0x5031ff) AM_READWRITE(toaplan2_txscrollram16_r, toaplan2_txscrollram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txscrollram16, tx_scroll_vram_size)
	AM_RANGE(0x600000, 0x60ffff) AM_READWRITE(toaplan2_tx_gfxram16_r, toaplan2_tx_gfxram16_w) AM_BASE_MEMBER(toaplan2_state, tx_gfxram16)
	AM_RANGE(0x700000, 0x700001) AM_WRITE(fixeight_subcpu_ctrl_w)
	AM_RANGE(0x800000, 0x800001) AM_READ(video_count_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( fixeighb_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM		/* 0-$7ffff ?*/
	AM_RANGE(0x100000, 0x10ffff) AM_RAM		/* 100000 - 107fff  105000-105xxx 106000-106xxx 108000 - related to sound ?*/
	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("IN1")
	AM_RANGE(0x200004, 0x200005) AM_READ_PORT("IN2")
	AM_RANGE(0x200008, 0x200009) AM_READ_PORT("IN3")
	AM_RANGE(0x20000c, 0x20000d) AM_READ_PORT("DSWB")
	AM_RANGE(0x200010, 0x200011) AM_READ_PORT("SYS")
	AM_RANGE(0x200014, 0x200015) AM_DEVWRITE("oki", fixeighb_oki_bankswitch_w)	/* Sound banking. Code at $4084c, $5070 */
	AM_RANGE(0x200018, 0x200019) AM_DEVREADWRITE8_MODERN("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x20001c, 0x20001d) AM_READ_PORT("DSWA")
	AM_RANGE(0x300000, 0x30000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x500000, 0x501fff) AM_READWRITE(toaplan2_txvideoram16_r, toaplan2_txvideoram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16, tx_vram_size)
	AM_RANGE(0x502000, 0x5021ff) AM_READWRITE(toaplan2_txvideoram16_offs_r, toaplan2_txvideoram16_offs_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16_offs, tx_offs_vram_size)
	AM_RANGE(0x503000, 0x5031ff) AM_READWRITE(toaplan2_txscrollram16_r, toaplan2_txscrollram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txscrollram16, tx_scroll_vram_size)
	AM_RANGE(0x700000, 0x700001) AM_READ(video_count_r)
	AM_RANGE(0x800000, 0x87ffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END


static ADDRESS_MAP_START( vfive_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
//  AM_RANGE(0x200000, 0x20ffff) AM_ROM                         /* Sound ROM is here ??? */
	AM_RANGE(0x200010, 0x200011) AM_READ_PORT("IN1")
	AM_RANGE(0x200014, 0x200015) AM_READ_PORT("IN2")
	AM_RANGE(0x200018, 0x200019) AM_READ_PORT("SYS")
	AM_RANGE(0x20001c, 0x20001d) AM_WRITE(toaplan2_v25_coin_word_w)	/* Coin count/lock */
	AM_RANGE(0x210000, 0x21ffff) AM_READWRITE( batsugun_share_r, batsugun_share_w )
	AM_RANGE(0x300000, 0x30000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x700000, 0x700001) AM_READ(video_count_r)
ADDRESS_MAP_END


static READ16_HANDLER( batsugun_share_r )
{
	toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

	return state->batsugun_share[offset] | state->batsugun_share[offset]<<8;
}

static WRITE16_HANDLER( batsugun_share_w )
{
#if 0
	if (ACCESSING_BITS_8_15)
	{
		toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

		state->batsugun_share[offset] = data >> 8;
	}
#endif

	if (ACCESSING_BITS_0_7)
	{
		toaplan2_state *state = space->machine->driver_data<toaplan2_state>();

		state->batsugun_share[offset] = data;
	}
}

static ADDRESS_MAP_START( batsugun_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x200010, 0x200011) AM_READ_PORT("IN1")
	AM_RANGE(0x200014, 0x200015) AM_READ_PORT("IN2")
	AM_RANGE(0x200018, 0x200019) AM_READ_PORT("SYS")
	AM_RANGE(0x20001c, 0x20001d) AM_WRITE(toaplan2_v25_coin_word_w)	/* Coin count/lock + v25 reset/hold control lines? */
	AM_RANGE(0x210000, 0x21ffff) AM_READWRITE( batsugun_share_r, batsugun_share_w )

	/***** The following in 0x30000x are for video controller 1 ******/
	AM_RANGE(0x300000, 0x30000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	/***** The following in 0x50000x are for video controller 2 ******/
	AM_RANGE(0x500000, 0x50000d) AM_DEVREADWRITE("gp9001vdp1", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x700000, 0x700001) AM_READ(video_count_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( snowbro2_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x300000, 0x30000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x500000, 0x500003) AM_DEVREADWRITE8("ymsnd", ym2151_r, ym2151_w, 0x00ff)
	AM_RANGE(0x600000, 0x600001) AM_DEVREADWRITE8_MODERN("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("JMPR")
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("DSWA")
	AM_RANGE(0x700008, 0x700009) AM_READ_PORT("DSWB")
	AM_RANGE(0x70000c, 0x70000d) AM_READ_PORT("IN1")
	AM_RANGE(0x700010, 0x700011) AM_READ_PORT("IN2")
	AM_RANGE(0x700014, 0x700015) AM_READ_PORT("IN3")
	AM_RANGE(0x700018, 0x700019) AM_READ_PORT("IN4")
	AM_RANGE(0x70001c, 0x70001d) AM_READ_PORT("SYS")
	AM_RANGE(0x700030, 0x700031) AM_DEVWRITE("oki", oki_bankswitch_w)		/* Sample bank switch */
	AM_RANGE(0x700034, 0x700035) AM_WRITE(toaplan2_coin_word_w)	/* Coin count/lock */
ADDRESS_MAP_END


static ADDRESS_MAP_START( mahoudai_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x218000, 0x21bfff) AM_READWRITE(z80_shared_r, z80_shared_w)
	AM_RANGE(0x21c01c, 0x21c01d) AM_WRITE(toaplan2_coin_word_w)
	AM_RANGE(0x21c020, 0x21c021) AM_READ_PORT("IN1")
	AM_RANGE(0x21c024, 0x21c025) AM_READ_PORT("IN2")
	AM_RANGE(0x21c028, 0x21c029) AM_READ_PORT("SYS")
	AM_RANGE(0x21c02c, 0x21c02d) AM_READ_PORT("DSWA")
	AM_RANGE(0x21c030, 0x21c031) AM_READ_PORT("DSWB")
	AM_RANGE(0x21c034, 0x21c035) AM_READ_PORT("JMPR")
	AM_RANGE(0x21c03c, 0x21c03d) AM_READ(video_count_r)
	AM_RANGE(0x300000, 0x30000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x401000, 0x4017ff) AM_RAM							/* Unused PaletteRAM */
	AM_RANGE(0x500000, 0x501fff) AM_READWRITE(toaplan2_txvideoram16_r, toaplan2_txvideoram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16, tx_vram_size)
	AM_RANGE(0x502000, 0x5021ff) AM_READWRITE(toaplan2_txvideoram16_offs_r, toaplan2_txvideoram16_offs_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16_offs, tx_offs_vram_size)
	AM_RANGE(0x502200, 0x502fff) AM_RAM
	AM_RANGE(0x503000, 0x5031ff) AM_READWRITE(toaplan2_txscrollram16_r, toaplan2_txscrollram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txscrollram16, tx_scroll_vram_size)
	AM_RANGE(0x503200, 0x503fff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( shippumd_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x218000, 0x21bfff) AM_READWRITE(z80_shared_r, z80_shared_w)
//  AM_RANGE(0x21c008, 0x21c009) AM_WRITENOP                    /* ??? */
	AM_RANGE(0x21c01c, 0x21c01d) AM_WRITE(shippumd_coin_word_w)
	AM_RANGE(0x21c020, 0x21c021) AM_READ_PORT("IN1")
	AM_RANGE(0x21c024, 0x21c025) AM_READ_PORT("IN2")
	AM_RANGE(0x21c028, 0x21c029) AM_READ_PORT("SYS")
	AM_RANGE(0x21c02c, 0x21c02d) AM_READ_PORT("DSWA")
	AM_RANGE(0x21c030, 0x21c031) AM_READ_PORT("DSWB")
	AM_RANGE(0x21c034, 0x21c035) AM_READ_PORT("JMPR")
	AM_RANGE(0x21c03c, 0x21c03d) AM_READ(video_count_r)
	AM_RANGE(0x300000, 0x30000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x401000, 0x4017ff) AM_RAM							/* Unused PaletteRAM */
	AM_RANGE(0x500000, 0x501fff) AM_READWRITE(toaplan2_txvideoram16_r, toaplan2_txvideoram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16, tx_vram_size)
	AM_RANGE(0x502000, 0x5021ff) AM_READWRITE(toaplan2_txvideoram16_offs_r, toaplan2_txvideoram16_offs_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16_offs, tx_offs_vram_size)
	AM_RANGE(0x502200, 0x502fff) AM_RAM
	AM_RANGE(0x503000, 0x5031ff) AM_READWRITE(toaplan2_txscrollram16_r, toaplan2_txscrollram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txscrollram16, tx_scroll_vram_size)
	AM_RANGE(0x503200, 0x503fff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( bgaregga_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x218000, 0x21bfff) AM_READWRITE(z80_shared_r, z80_shared_w)
	AM_RANGE(0x21c01c, 0x21c01d) AM_WRITE(toaplan2_coin_word_w)
	AM_RANGE(0x21c020, 0x21c021) AM_READ_PORT("IN1")
	AM_RANGE(0x21c024, 0x21c025) AM_READ_PORT("IN2")
	AM_RANGE(0x21c028, 0x21c029) AM_READ_PORT("SYS")
	AM_RANGE(0x21c02c, 0x21c02d) AM_READ_PORT("DSWA")
	AM_RANGE(0x21c030, 0x21c031) AM_READ_PORT("DSWB")
	AM_RANGE(0x21c034, 0x21c035) AM_READ_PORT("JMPR")
	AM_RANGE(0x21c03c, 0x21c03d) AM_READ(video_count_r)
	AM_RANGE(0x300000, 0x30000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_r, gp9001_vdp_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x500000, 0x501fff) AM_READWRITE(toaplan2_txvideoram16_r, toaplan2_txvideoram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16, tx_vram_size)
	AM_RANGE(0x502000, 0x5021ff) AM_READWRITE(toaplan2_txvideoram16_offs_r, toaplan2_txvideoram16_offs_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16_offs, tx_offs_vram_size)
	AM_RANGE(0x502200, 0x502fff) AM_RAM
	AM_RANGE(0x503000, 0x5031ff) AM_READWRITE(toaplan2_txscrollram16_r, toaplan2_txscrollram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txscrollram16, tx_scroll_vram_size)
	AM_RANGE(0x503200, 0x503fff) AM_RAM
	AM_RANGE(0x600000, 0x600001) AM_WRITE(bgaregga_soundlatch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( batrider_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x201fff) AM_READWRITE(toaplan2_txvideoram16_r, toaplan2_txvideoram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16, tx_vram_size)	/* Text VideoRAM */
	AM_RANGE(0x202000, 0x202fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram) AM_SIZE_MEMBER(toaplan2_state, paletteram16_size)
	AM_RANGE(0x203000, 0x2031ff) AM_READWRITE(toaplan2_txvideoram16_offs_r, toaplan2_txvideoram16_offs_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16_offs, tx_offs_vram_size)
	AM_RANGE(0x203200, 0x2033ff) AM_READWRITE(toaplan2_txscrollram16_r, toaplan2_txscrollram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txscrollram16, tx_scroll_vram_size)
	AM_RANGE(0x203400, 0x207fff) AM_READWRITE(raizing_tx_gfxram16_r, raizing_tx_gfxram16_w) /* Main RAM actually */
	AM_RANGE(0x208000, 0x20ffff) AM_RAM
	AM_RANGE(0x300000, 0x37ffff) AM_READ(raizing_z80rom_r)
	AM_RANGE(0x400000, 0x40000d) AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_alt_r, gp9001_vdp_alt_w)
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("IN")
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("SYS-DSW")
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("DSW")
	AM_RANGE(0x500006, 0x500007) AM_READ(video_count_r)
	AM_RANGE(0x500008, 0x500009) AM_READ(soundlatch3_word_r)
	AM_RANGE(0x50000a, 0x50000b) AM_READ(soundlatch4_word_r)
	AM_RANGE(0x50000c, 0x50000d) AM_READ(batrider_z80_busack_r)
	AM_RANGE(0x500010, 0x500011) AM_WRITE(toaplan2_coin_word_w)
	AM_RANGE(0x500020, 0x500021) AM_WRITE(batrider_soundlatch_w)
	AM_RANGE(0x500022, 0x500023) AM_WRITE(batrider_soundlatch2_w)
	AM_RANGE(0x500024, 0x500025) AM_WRITE(raizing_unknown_w)
	AM_RANGE(0x500026, 0x500027) AM_WRITE(raizing_clear_sndirq_w)
	AM_RANGE(0x500060, 0x500061) AM_WRITE(batrider_z80_busreq_w)
	AM_RANGE(0x500080, 0x500081) AM_WRITE(batrider_textdata_decode)
	AM_RANGE(0x5000c0, 0x5000cf) AM_WRITE(batrider_objectbank_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( bbakraid_68k_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x201fff) AM_READWRITE(toaplan2_txvideoram16_r, toaplan2_txvideoram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16, tx_vram_size)	/* Text VideoRAM */
	AM_RANGE(0x202000, 0x202fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram) AM_SIZE_MEMBER(toaplan2_state, paletteram16_size)
	AM_RANGE(0x203000, 0x2031ff) AM_READWRITE(toaplan2_txvideoram16_offs_r, toaplan2_txvideoram16_offs_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txvideoram16_offs, tx_offs_vram_size)
	AM_RANGE(0x203200, 0x2033ff) AM_READWRITE(toaplan2_txscrollram16_r, toaplan2_txscrollram16_w) AM_BASE_SIZE_MEMBER(toaplan2_state, txscrollram16, tx_scroll_vram_size)
	AM_RANGE(0x203400, 0x207fff) AM_READWRITE(raizing_tx_gfxram16_r, raizing_tx_gfxram16_w)	/* Main RAM actually */
	AM_RANGE(0x208000, 0x20ffff) AM_RAM
	AM_RANGE(0x300000, 0x33ffff) AM_READ(raizing_z80rom_r)
	AM_RANGE(0x400000, 0x40000d)  AM_DEVREADWRITE("gp9001vdp0", gp9001_vdp_alt_r, gp9001_vdp_alt_w)
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("IN")
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("SYS-DSW")
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("DSW")
	AM_RANGE(0x500006, 0x500007) AM_READ(video_count_r)
	AM_RANGE(0x500008, 0x500009) AM_WRITE(toaplan2_coin_word_w)
	AM_RANGE(0x500010, 0x500011) AM_READ(soundlatch3_word_r)
	AM_RANGE(0x500012, 0x500013) AM_READ(soundlatch4_word_r)
	AM_RANGE(0x500014, 0x500015) AM_WRITE(batrider_soundlatch_w)
	AM_RANGE(0x500016, 0x500017) AM_WRITE(batrider_soundlatch2_w)
	AM_RANGE(0x500018, 0x500019) AM_READ(bbakraid_nvram_r)
	AM_RANGE(0x50001a, 0x50001b) AM_WRITE(raizing_unknown_w)
	AM_RANGE(0x50001c, 0x50001d) AM_WRITE(raizing_clear_sndirq_w)
	AM_RANGE(0x50001e, 0x50001f) AM_WRITE(bbakraid_nvram_w)
	AM_RANGE(0x500080, 0x500081) AM_WRITE(batrider_textdata_decode)
	AM_RANGE(0x5000c0, 0x5000cf) AM_WRITE(batrider_objectbank_w)
ADDRESS_MAP_END



static ADDRESS_MAP_START( pipibibs_sound_z80_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_BASE_MEMBER(toaplan2_state, z80_shared_ram)
	AM_RANGE(0xe000, 0xe001) AM_DEVREADWRITE("ymsnd", ym3812_r, ym3812_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( raizing_sound_z80_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_BASE_MEMBER(toaplan2_state, z80_shared_ram)
	AM_RANGE(0xe000, 0xe001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xe004, 0xe004) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0xe00e, 0xe00e) AM_WRITE(toaplan2_coin_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( bgaregga_sound_z80_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_BASE_MEMBER(toaplan2_state, z80_shared_ram)
	AM_RANGE(0xe000, 0xe001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xe004, 0xe004) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0xe006, 0xe006) AM_WRITE(raizing_okim6295_bankselect_0)
	AM_RANGE(0xe008, 0xe008) AM_WRITE(raizing_okim6295_bankselect_1)
	AM_RANGE(0xe00a, 0xe00a) AM_WRITE(bgaregga_bankswitch_w)
	AM_RANGE(0xe00c, 0xe00c) AM_WRITE(bgaregga_E00C_w)
	AM_RANGE(0xe01c, 0xe01c) AM_READ(soundlatch_r)
	AM_RANGE(0xe01d, 0xe01d) AM_READ(bgaregga_E01D_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( batrider_sound_z80_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( batrider_sound_z80_port, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_WRITE(soundlatch3_w)
	AM_RANGE(0x42, 0x42) AM_WRITE(soundlatch4_w)
	AM_RANGE(0x44, 0x44) AM_WRITE(raizing_sndirq_w)
	AM_RANGE(0x46, 0x46) AM_WRITE(raizing_clear_nmi_w)
	AM_RANGE(0x48, 0x48) AM_READ(soundlatch_r)
	AM_RANGE(0x4a, 0x4a) AM_READ(soundlatch2_r)
	AM_RANGE(0x80, 0x81) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x82, 0x82) AM_DEVREADWRITE_MODERN("oki1", okim6295_device, read, write)
	AM_RANGE(0x84, 0x84) AM_DEVREADWRITE_MODERN("oki2", okim6295_device, read, write)
	AM_RANGE(0x88, 0x88) AM_WRITE(batrider_bankswitch_w)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(raizing_okim6295_bankselect_0)
	AM_RANGE(0xc2, 0xc2) AM_WRITE(raizing_okim6295_bankselect_1)
	AM_RANGE(0xc4, 0xc4) AM_WRITE(raizing_okim6295_bankselect_2)
	AM_RANGE(0xc6, 0xc6) AM_WRITE(raizing_okim6295_bankselect_3)
ADDRESS_MAP_END


static ADDRESS_MAP_START( bbakraid_sound_z80_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM			/* Only 2FFFh valid code */
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( bbakraid_sound_z80_port, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_WRITE(soundlatch3_w)
	AM_RANGE(0x42, 0x42) AM_WRITE(soundlatch4_w)
	AM_RANGE(0x44, 0x44) AM_WRITE(raizing_sndirq_w)
	AM_RANGE(0x46, 0x46) AM_WRITE(raizing_clear_nmi_w)
	AM_RANGE(0x48, 0x48) AM_READ(soundlatch_r)
	AM_RANGE(0x4a, 0x4a) AM_READ(soundlatch2_r)
	AM_RANGE(0x80, 0x81) AM_DEVREADWRITE("ymz", ymz280b_r, ymz280b_w)
ADDRESS_MAP_END



#ifdef USE_HD64x180
static ADDRESS_MAP_START( hd647180_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xfe00, 0xffff) AM_RAM			/* Internal 512 bytes of RAM */
ADDRESS_MAP_END
#endif


static ADDRESS_MAP_START( V25_kbash_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x007ff) AM_RAM AM_BASE_MEMBER(toaplan2_state, batsugun_share) /* 2KB shared with M68K */
	AM_RANGE(0x04000, 0x04001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x04002, 0x04002) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0x80000, 0x87fff) AM_MIRROR(0x78000) AM_ROM AM_REGION("audiocpu", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( V25_rambased_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x00001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x00004, 0x00004) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0x80000, 0x87fff) AM_MIRROR(0x78000) AM_RAM AM_BASE_MEMBER(toaplan2_state, batsugun_share)
ADDRESS_MAP_END


static ADDRESS_MAP_START( V25_rambased_nooki_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x00001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	//AM_RANGE(0x00004, 0x00004) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0x80000, 0x87fff) AM_MIRROR(0x78000) AM_RAM AM_BASE_MEMBER(toaplan2_state, batsugun_share)
ADDRESS_MAP_END

READ8_DEVICE_HANDLER( fixeight_eeprom_r )
{
	int bit = eeprom_read_bit(device);

	return bit<<7;
}

static WRITE8_DEVICE_HANDLER(fixeight_eeprom_w )
{
		// latch the bit
		eeprom_write_bit(device, data & 0x0040);

		// reset line asserted: reset.
		eeprom_set_cs_line(device, (data & 0x0010) ? CLEAR_LINE : ASSERT_LINE);

		// clock line asserted: write latch or select next bit to read
		eeprom_set_clock_line(device, (data & 0x0020) ? ASSERT_LINE : CLEAR_LINE);
}

static READ8_HANDLER( fixeight_region_r )
{
	// this must match the eeprom!
	// however there is no valid value that makes the dumped eeprom boot
	// this makes me wonder if there are decryption errors, therefore
	// this code, and the default eeproms use should be considered subject
	// to change

	if (!strcmp(space->machine->gamedrv->name,"fixeightkt"))	return 0x00;
	if (!strcmp(space->machine->gamedrv->name,"fixeightk"))	return 0x01;
	if (!strcmp(space->machine->gamedrv->name,"fixeightht"))	return 0x02;
	if (!strcmp(space->machine->gamedrv->name,"fixeighth"))	return 0x03;
	if (!strcmp(space->machine->gamedrv->name,"fixeighttwt"))	return 0x04;
	if (!strcmp(space->machine->gamedrv->name,"fixeighttw"))	return 0x05;
	if (!strcmp(space->machine->gamedrv->name,"fixeightat"))	return 0x06;
	if (!strcmp(space->machine->gamedrv->name,"fixeighta"))	return 0x07;
	if (!strcmp(space->machine->gamedrv->name,"fixeightt"))	return 0x08;
	if (!strcmp(space->machine->gamedrv->name,"fixeight9"))	return 0x09;
	if (!strcmp(space->machine->gamedrv->name,"fixeighta"))	return 0x0a;
	if (!strcmp(space->machine->gamedrv->name,"fixeightu"))	return 0x0b;
//  if (!strcmp(space->machine->gamedrv->name,"fixeightc")) return 0x0c; // invalid
//  if (!strcmp(space->machine->gamedrv->name,"fixeightd")) return 0x0d; // invalid
	if (!strcmp(space->machine->gamedrv->name,"fixeightj"))	return 0x0e;
	if (!strcmp(space->machine->gamedrv->name,"fixeightjt"))	return 0x0f;

	return 0x00;
}

static ADDRESS_MAP_START( V25_fixeight_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00004, 0x00004) AM_READ(fixeight_region_r)
	AM_RANGE(0x0000a, 0x0000b) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x0000c, 0x0000c) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0x80000, 0x87fff) AM_MIRROR(0x78000) AM_RAM AM_BASE_MEMBER(toaplan2_state, batsugun_share)
ADDRESS_MAP_END

static ADDRESS_MAP_START( V25_fixeight_port, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(V25_PORT_P0, V25_PORT_P0) AM_DEVREADWRITE("eeprom", fixeight_eeprom_r, fixeight_eeprom_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( V25_port, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(V25_PORT_PT, V25_PORT_PT) AM_READ(v25_dswa_read)
	AM_RANGE(V25_PORT_P0, V25_PORT_P0) AM_READ(v25_dswb_read)
	AM_RANGE(V25_PORT_P1, V25_PORT_P1) AM_READ(v25_jmpr_read)
	AM_RANGE(V25_PORT_P2, V25_PORT_P2) AM_WRITENOP	/* bit 0 is FAULT according to kbash schematics */
ADDRESS_MAP_END

static ADDRESS_MAP_START( V25_port_abswap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(V25_PORT_PT, V25_PORT_PT) AM_READ(v25_dswb_read)
	AM_RANGE(V25_PORT_P0, V25_PORT_P0) AM_READ(v25_dswa_read)
	AM_RANGE(V25_PORT_P1, V25_PORT_P1) AM_READ(v25_jmpr_read)
	AM_RANGE(V25_PORT_P2, V25_PORT_P2) AM_WRITENOP	/* bit 0 is FAULT according to kbash schematics */
ADDRESS_MAP_END

/*****************************************************************************
    Input Port definitions
    Service input of the TOAPLAN2_SYSTEM_INPUTS is used as a Pause type input.
    If you press then release the following buttons, the following occurs:
    Service & P2 start            : The game will pause.
    P1 start                      : The game will continue.
    Service & P1 start & P2 start : The game will play in slow motion.
*****************************************************************************/


#define TOAPLAN2_COINAGE( mask, value)																				\
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:5,6")								\
	PORT_DIPSETTING(		0x0030, DEF_STR( 4C_1C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_EQUALS,value)		\
	PORT_DIPSETTING(		0x0020, DEF_STR( 3C_1C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_EQUALS,value)		\
	PORT_DIPSETTING(		0x0010, DEF_STR( 2C_1C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_EQUALS,value)		\
	PORT_DIPSETTING(		0x0020, DEF_STR( 2C_1C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_NOTEQUALS,value)	\
	PORT_DIPSETTING(		0x0000, DEF_STR( 1C_1C ) )																\
	PORT_DIPSETTING(		0x0030, DEF_STR( 2C_3C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_NOTEQUALS,value)	\
	PORT_DIPSETTING(		0x0010, DEF_STR( 1C_2C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_NOTEQUALS,value)	\
	PORT_DIPNAME( 0x00c0,	0x0000, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:7,8")								\
	PORT_DIPSETTING(		0x0080, DEF_STR( 2C_1C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_NOTEQUALS,value)	\
	PORT_DIPSETTING(		0x0000, DEF_STR( 1C_1C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_NOTEQUALS,value)	\
	PORT_DIPSETTING(		0x00c0, DEF_STR( 2C_3C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_NOTEQUALS,value)	\
	PORT_DIPSETTING(		0x0040, DEF_STR( 1C_2C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_NOTEQUALS,value)	\
	PORT_DIPSETTING(		0x0000, DEF_STR( 1C_2C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_EQUALS,value)		\
	PORT_DIPSETTING(		0x0040, DEF_STR( 1C_3C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_EQUALS,value)		\
	PORT_DIPSETTING(		0x0080, DEF_STR( 1C_4C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_EQUALS,value)		\
	PORT_DIPSETTING(		0x00c0, DEF_STR( 1C_6C ) )		PORT_CONDITION("JMPR",mask,PORTCOND_EQUALS,value)




static INPUT_PORTS_START( toaplan2 )
	PORT_START("VBL")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("DSWA")
	PORT_DIPNAME( 0x0001,	0x0000, DEF_STR( Unused ) )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002,	0x0000, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0002, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x0004, IP_ACTIVE_HIGH, "SW1:3")
	PORT_DIPNAME( 0x0008,	0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(		0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( On ) )
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Modified below */
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("DSWB")
	PORT_DIPNAME( 0x0003,	0x0000, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(		0x0003, DEF_STR( Hardest ) )
	PORT_DIPSETTING(		0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(		0x0001, DEF_STR( Easy ) )
	/* Various features on bit mask 0x00fc */
	PORT_BIT( 0x00fc, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Modified below */
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("JMPR")	/* (6) Territory Jumper block */
	/* Territory settings on bit mask 0x000f */
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Modified below */
	PORT_BIT( 0xfff0, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */
INPUT_PORTS_END



static INPUT_PORTS_START( tekipaki )
	PORT_INCLUDE( toaplan2 )

	PORT_MODIFY("DSWA")
	/* Various features on bit mask 0x000f - see above */
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	TOAPLAN2_COINAGE( 0x000f, 0x0002 )

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x0004,	0x0000, DEF_STR( Unused ) )		PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008,	0x0000, DEF_STR( Unused ) )		PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010,	0x0000, DEF_STR( Unused ) )		PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020,	0x0000, DEF_STR( Unused ) )		PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040,	0x0000, "Game Mode" )			PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0040, "Stop" )
	PORT_DIPSETTING(		0x0000, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Unused ) )		PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0080, DEF_STR( On ) )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x000f,	0x0002, "Territory" )
	PORT_DIPSETTING(		0x0002, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x0001, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x0007, "USA (Romstar)" )
	PORT_DIPSETTING(		0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x000f, "Japan (Distributed by Tecmo)" )
	PORT_DIPSETTING(		0x0004, "Korea" )
	PORT_DIPSETTING(		0x0003, "Hong Kong" )
	PORT_DIPSETTING(		0x0008, "Hong Kong (Honest Trading Co.)" )
	PORT_DIPSETTING(		0x0005, "Taiwan" )
	PORT_DIPSETTING(		0x0006, "Taiwan (Spacy Co. Ltd)" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(c2map_r, NULL)
INPUT_PORTS_END


static INPUT_PORTS_START( ghox )
	PORT_INCLUDE( toaplan2 )

	PORT_MODIFY("DSWA")
	/* Various features on bit mask 0x000f - see above */
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	TOAPLAN2_COINAGE( 0x80000, 0x80000 )

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x000c,	0x0000, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(		0x0008, "100k only" )
	PORT_DIPSETTING(		0x0004, "100k and 300k" )
	PORT_DIPSETTING(		0x0000, "100k and every 200k" )
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x0030, "1" )
	PORT_DIPSETTING(		0x0020, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x0010, "5" )
	PORT_DIPNAME( 0x0040,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Unused ) )			PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0080, DEF_STR( On ) )

	PORT_MODIFY("JMPR")
	/* Bit Mask 0x80000 is used here to signify European Coinage for MAME purposes - not read on the real board! */
	PORT_DIPNAME( 0x8000f,	0x80002, "Territory" )
	PORT_DIPSETTING(		0x80002, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x8000a, "Europe (Nova Apparate GMBH & Co)" )
	PORT_DIPSETTING(		0x8000d, "Europe (Taito Corporation Japan)" )
	PORT_DIPSETTING(		0x00001, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x00009, "USA (Romstar)" )
	PORT_DIPSETTING(		0x0000b, "USA (Taito America Corporation)" )
	PORT_DIPSETTING(		0x0000c, "USA (Taito Corporation Japan)" )
	PORT_DIPSETTING(		0x00000, DEF_STR( Japan ) )
//  PORT_DIPSETTING(        0x0000e, "Japan (Taito Corporation)" )
	PORT_DIPSETTING(		0x00004, "Korea" )
	PORT_DIPSETTING(		0x00003, "Hong Kong (Honest Trading Co.)" )
	PORT_DIPSETTING(		0x00005, "Taiwan" )
	PORT_DIPSETTING(		0x80006, "Spain & Portugal (APM Electronics SA)" )
	PORT_DIPSETTING(		0x80007, "Italy (Star Electronica SRL)" )
	PORT_DIPSETTING(		0x80008, "UK (JP Leisure Ltd)" )

	PORT_START("PAD1")		/* Paddle 1 (left-right)  read at $100000 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("PAD2")		/* Paddle 2 (left-right)  read at $040000 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */
INPUT_PORTS_END


static INPUT_PORTS_START( dogyuun )
	PORT_INCLUDE( toaplan2 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,	0x0000, DEF_STR( Free_Play) )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0001, DEF_STR( On ) )
	/* Various features on bit mask 0x000e - see above */
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	TOAPLAN2_COINAGE( 0x000f, 0x0003 )

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x000c,	0x0000, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(		0x0008, "400k only" )
	PORT_DIPSETTING(		0x0000, "200k only" )
	PORT_DIPSETTING(		0x0004, "200k, 400k and 600k" )
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x0030, "1" )
	PORT_DIPSETTING(		0x0020, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x0010, "5" )
	PORT_DIPNAME( 0x0040,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Yes ) )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x00f0,	0x0030, "Territory" )
	PORT_DIPSETTING(		0x0030, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x0010, DEF_STR( USA ) )
//  PORT_DIPSETTING(        0x0070, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x0020, "USA (Atari Games Corp license)" )
//  PORT_DIPSETTING(        0x00c0, "USA (Atari Games Corp license)" )
	PORT_DIPSETTING(		0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x00f0, "Japan (Taito Corp license)" )
	PORT_DIPSETTING(		0x0080, "South East Asia (Charterfield license)" )
//  PORT_DIPSETTING(        0x00d0, "South East Asia (Charterfield license)" )
	PORT_DIPSETTING(		0x0050, "Korea (Unite Trading license)" )
//  PORT_DIPSETTING(        0x00a0, "Korea (Unite Trading license)" )
	PORT_DIPSETTING(		0x0040, "Hong Kong (Charterfield license)" )
//  PORT_DIPSETTING(        0x0090, "Hong Kong (Charterfield license)" )
	PORT_DIPSETTING(		0x0060, "Taiwan" )
//  PORT_DIPSETTING(        0x00b0, "Taiwan" )
	//PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* Sound ready */
INPUT_PORTS_END


static INPUT_PORTS_START( dogyuunk )
	PORT_INCLUDE( dogyuun )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x00f0,	0x0050, "Territory" )
	PORT_DIPSETTING(		0x0030, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x0010, DEF_STR( USA ) )
//  PORT_DIPSETTING(        0x0070, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x0020, "USA (Atari Games Corp license)" )
//  PORT_DIPSETTING(        0x00c0, "USA (Atari Games Corp license)" )
	PORT_DIPSETTING(		0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x00f0, "Japan (Taito Corp license)" )
	PORT_DIPSETTING(		0x0080, "South East Asia (Charterfield license)" )
//  PORT_DIPSETTING(        0x00d0, "South East Asia (Charterfield license)" )
	PORT_DIPSETTING(		0x0050, "Korea (Unite Trading license)" )
//  PORT_DIPSETTING(        0x00a0, "Korea (Unite Trading license)" )
	PORT_DIPSETTING(		0x0040, "Hong Kong (Charterfield license)" )
//  PORT_DIPSETTING(        0x0090, "Hong Kong (Charterfield license)" )
	PORT_DIPSETTING(		0x0060, "Taiwan" )
//  PORT_DIPSETTING(        0x00b0, "Taiwan" )
	//PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* Sound ready */
INPUT_PORTS_END

static INPUT_PORTS_START( dogyuunt )
	PORT_INCLUDE( dogyuun )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x00f0,	0x0000, "Territory" )
	PORT_DIPSETTING(		0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x0010, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x0020, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x0030, "Hong Kong" )
	PORT_DIPSETTING(		0x0040, "Korea" )
	PORT_DIPSETTING(		0x0050, "Taiwan" )
	PORT_DIPSETTING(		0x0060, "South East Asia (Charterfield license)" )
	PORT_DIPSETTING(		0x0070, "USA (Romstar license)" )
	PORT_DIPSETTING(		0x0080, "Hong Kong and China (Honest Trading Co. license)" )
	PORT_DIPSETTING(		0x0090, "Korea (JC Trading Corp. license)" )
	PORT_DIPSETTING(		0x00a0, "USA (Fabtek license)" )
	PORT_DIPSETTING(		0x00f0, "Japan (Taito Corp license)" )
INPUT_PORTS_END

static INPUT_PORTS_START( kbash )
	PORT_INCLUDE( toaplan2 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,	0x0000, "Continue Mode" )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(		0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(		0x0001, "Discount" )
	/* Various features on bit mask 0x000e - see above */
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	TOAPLAN2_COINAGE( 0x0007, 0x0002 )

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x000c,	0x0000, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(		0x0008, "200k only" )
	PORT_DIPSETTING(		0x0004, "100k only" )
	PORT_DIPSETTING(		0x0000, "100k and 400k" )
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x0030, "1" )
	PORT_DIPSETTING(		0x0000, "2" )
	PORT_DIPSETTING(		0x0020, "3" )
	PORT_DIPSETTING(		0x0010, "4" )
	PORT_DIPNAME( 0x0040,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Yes ) )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x00f0,	0x0020, "Territory" )
	PORT_DIPSETTING(		0x00a0, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x0020, "Europe, USA (Atari license)" )
	PORT_DIPSETTING(		0x0090, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x0010, "USA, Europe (Atari license)" )
	PORT_DIPSETTING(		0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x0060, "South East Asia" )	/*Service Mode lists European Coinage */
	PORT_DIPSETTING(		0x0030, "Korea" )
	PORT_DIPSETTING(		0x0040, "Hong Kong" )
INPUT_PORTS_END


static INPUT_PORTS_START( kbash2 )
	PORT_INCLUDE(kbash)

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x000f,	0x0006, "Territory" )
	PORT_DIPSETTING(		0x0000, "Japan (Taito Corp license)" )
	PORT_DIPSETTING(		0x000e, "South East Asia" )	/*Service Mode lists European Coinage */
	PORT_DIPSETTING(		0x0006, "South East Asia (Charterfield license)" )	/*Service Mode lists European Coinage */
	PORT_DIPSETTING(		0x000b, "Korea" )
	PORT_DIPSETTING(		0x0003, "Korea (Unite license)" )
	PORT_DIPSETTING(		0x0004, "Hong Kong" )
	PORT_DIPSETTING(		0x0005, "Taiwan" )
INPUT_PORTS_END


static INPUT_PORTS_START( truxton2 )
	PORT_INCLUDE( toaplan2 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,	0x0000, "Rapid Fire" )				PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(		0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( On ) )
	/* Various features on bit mask 0x000e - see above */
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	TOAPLAN2_COINAGE( 0x0007, 0x0002 )

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x000c,	0x0000, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x000c, "200k only" )
	PORT_DIPSETTING(		0x0008, "100k only" )
	PORT_DIPSETTING(		0x0004, "100k and 250k" )
	PORT_DIPSETTING(		0x0000, "70k and 200k" )
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x0030, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x0020, "4" )
	PORT_DIPSETTING(		0x0010, "5" )
	PORT_DIPNAME( 0x0040,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Yes ) )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x0007,	0x0002, "Territory" )
	PORT_DIPSETTING(		0x0002, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x0001, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x0006, "South East Asia" )
	PORT_DIPSETTING(		0x0004, "Korea" )
	PORT_DIPSETTING(		0x0003, "Hong Kong" )
	PORT_DIPSETTING(		0x0005, "Taiwan" )
INPUT_PORTS_END


static INPUT_PORTS_START( pipibibs )
	PORT_INCLUDE( toaplan2 )

	PORT_MODIFY("DSWA")
	/* Various features on bit mask 0x000f - see above */
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	TOAPLAN2_COINAGE( 0x0006, 0x0006 )

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x000c,	0x0000, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(		0x0008, "200k only" )
	PORT_DIPSETTING(		0x0000, "200k and every 300k" )
	PORT_DIPSETTING(		0x0004, "150k and every 200k" )
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x0030, "1" )
	PORT_DIPSETTING(		0x0020, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x0010, "5" )
	PORT_DIPNAME( 0x0040,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Unused ) )			PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0080, DEF_STR( On ) )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x0008,	0x0000, "Nudity" )
	PORT_DIPSETTING(		0x0008, DEF_STR( Low ) )
	PORT_DIPSETTING(		0x0000, "High, but censored" )
	PORT_DIPNAME( 0x0007,	0x0006, "Territory" )
	PORT_DIPSETTING(		0x0006, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x0007, "Europe (Nova Apparate GMBH & Co)" )
	PORT_DIPSETTING(		0x0004, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x0005, "USA (Romstar)" )
	PORT_DIPSETTING(		0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x0001, DEF_STR( Asia ) )
	PORT_DIPSETTING(		0x0002, "Hong Kong (Honest Trading Co.)" )
	PORT_DIPSETTING(		0x0003, "Taiwan" )
INPUT_PORTS_END


static INPUT_PORTS_START( whoopee )
	PORT_INCLUDE(pipibibs)

	PORT_MODIFY("JMPR")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(c2map_r, NULL)	/* bit 0x10 sound ready */
INPUT_PORTS_END


static INPUT_PORTS_START( pipibibsbl )
	PORT_INCLUDE(pipibibs)

	PORT_MODIFY("VBL")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )		/* This video HW doesn't use this VBlank input */

	PORT_MODIFY("DSWA")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )	PORT_DIPLOCATION("SW1:2")	/* This video HW doesn't support flip screen */
	/* Various features on bit mask 0x000d - see above */
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	TOAPLAN2_COINAGE( 0x80000, 0x80000 )

	PORT_MODIFY("JMPR")
	/* Bit Mask 0x80000 is used here to signify European Coinage for MAME purposes - not read on the real board! */
	PORT_DIPNAME( 0x80007,	0x00007, "Territory" )
	PORT_DIPSETTING(		0x00002, DEF_STR( World ) )
//  PORT_DIPSETTING(        0x00003, DEF_STR( World ) )
	PORT_DIPSETTING(		0x00007, "World (Ryouta Kikaku)" )
	PORT_DIPSETTING(		0x80005, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x00004, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x00000, "Japan (Ryouta Kikaku)" )
	PORT_DIPSETTING(		0x00001, "Hong Kong (Honest Trading Co.)" )
	PORT_DIPSETTING(		0x80006, "Spain & Portugal (APM Electronics SA)" )
INPUT_PORTS_END


static INPUT_PORTS_START( fixeight )
	PORT_START("VBL")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_SERVICE_NO_TOGGLE(0x0004, IP_ACTIVE_HIGH)	/* Service input is a push-button marked 'Test SW' */
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */
INPUT_PORTS_END


static INPUT_PORTS_START( fixeightbl )
	PORT_INCLUDE( toaplan2 )

	PORT_MODIFY("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_SERVICE_NO_TOGGLE(0x0004, IP_ACTIVE_HIGH)	/* Service input is a push-button marked 'Test SW' */

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,	0x0000, "Maximum Players" )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(		0x0000, "2" )
	PORT_DIPSETTING(		0x0001, "3" )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )	PORT_DIPLOCATION("SW1:2")	/* This video HW doesn't support flip screen */
	PORT_DIPNAME( 0x0004,	0x0004, "Shooting style" )		PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(		0x0004, "Semi-auto" )
	PORT_DIPSETTING(		0x0000, "Fully-auto" )
	/* Various features on bit mask 0x000a - see above */
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	TOAPLAN2_COINAGE( 0x000e, 0x0008 )

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x000c,	0x0000, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(		0x0000, "500k and every 500k" )
	PORT_DIPSETTING(		0x0008, "300k only" )
	PORT_DIPSETTING(		0x0004, "300k and every 300k" )
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x0030, "1" )
	PORT_DIPSETTING(		0x0020, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x0010, "5" )
	PORT_DIPNAME( 0x0040,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Yes ) )

	PORT_MODIFY("JMPR")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )		/* Doesn't exist? */
INPUT_PORTS_END


static INPUT_PORTS_START( grindstm )
	PORT_INCLUDE( toaplan2 )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,	0x0000, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(		0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(		0x0001, DEF_STR( Cocktail ) )
	/* Various features on bit mask 0x000e - see above */
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	TOAPLAN2_COINAGE( 0x000f, 0x0009 )

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x000c,	0x0000, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(		0x0008, "200k only" )
	PORT_DIPSETTING(		0x0000, "300k and 800k" )
	PORT_DIPSETTING(		0x0004, "300k and every 800k" )
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x0030, "1" )
	PORT_DIPSETTING(		0x0020, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x0010, "5" )
	PORT_DIPNAME( 0x0040,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Yes ) )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x00f0,	0x0090, "Territory" )
	PORT_DIPSETTING(		0x0090, DEF_STR( Europe ) )
//  PORT_DIPSETTING(        0x0080, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x00d0, DEF_STR( USA ) )
//  PORT_DIPSETTING(        0x00b0, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x00c0, "USA (American Sammy Corporation license)" )
//  PORT_DIPSETTING(        0x00a0, "USA (American Sammy Corporation license)" )
	PORT_DIPSETTING(		0x0070, "South East Asia" )
	PORT_DIPSETTING(		0x0060, "South East Asia (Charterfield license)" )
//  PORT_DIPSETTING(        0x00f0, "Korea" )
//  PORT_DIPSETTING(        0x00e0, "Korea" )
	PORT_DIPSETTING(		0x0010, "Korea" )
	PORT_DIPSETTING(		0x0000, "Korea (Unite Trading license)" )
	PORT_DIPSETTING(		0x0030, "Hong Kong" )
	PORT_DIPSETTING(		0x0020, "Hong Kong (Charterfield license)" )
	PORT_DIPSETTING(		0x0050, "Taiwan" )
	PORT_DIPSETTING(		0x0040, "Taiwan (Anomoto International Inc license)" )
INPUT_PORTS_END


static INPUT_PORTS_START( vfive )
	PORT_INCLUDE(grindstm)

	PORT_MODIFY("JMPR")		/* (6) Territory Jumper block */
	/* Territory is forced to Japan in this set. */
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( batsugun )
	PORT_INCLUDE( toaplan2 )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,	0x0000, "Continue Mode" )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(		0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(		0x0001, "Discount" )
	/* Various features on bit mask 0x000e - see above */
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	TOAPLAN2_COINAGE( 0x00ff, 0x00ff )	/* Actually European coinage isn't used */

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x000c,	0x0000, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(		0x0008, "1500k only" )
	PORT_DIPSETTING(		0x0000, "1000k only" )
	PORT_DIPSETTING(		0x0004, "500k and every 600k" )
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x0030, "1" )
	PORT_DIPSETTING(		0x0020, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x0010, "5" )
	PORT_DIPNAME( 0x0040,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Yes ) )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x00f0,	0x0090, "Territory" )
	PORT_DIPSETTING(		0x0090, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x0080, "Europe (Taito Corp license)" )
	PORT_DIPSETTING(		0x00b0, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x00a0, "USA (Taito Corp license)" )
	PORT_DIPSETTING(		0x00f0, DEF_STR( Japan ) )
//  PORT_DIPSETTING(        0x00e0, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x00d0, "Japan (Taito Corp license)" )
//  PORT_DIPSETTING(        0x00c0, "Japan (Taito Corp license)" )
	PORT_DIPSETTING(		0x0070, "South East Asia" )
	PORT_DIPSETTING(		0x0060, "South East Asia (Taito Corp license)" )
	PORT_DIPSETTING(		0x0010, "Korea" )
	PORT_DIPSETTING(		0x0000, "Korea (Unite Trading license)" )
	PORT_DIPSETTING(		0x0030, "Hong Kong" )
	PORT_DIPSETTING(		0x0020, "Hong Kong (Taito Corp license)" )
	PORT_DIPSETTING(		0x0050, "Taiwan" )
	PORT_DIPSETTING(		0x0040, "Taiwan (Taito Corp license)" )
INPUT_PORTS_END


static INPUT_PORTS_START( snowbro2 )
	PORT_INCLUDE( toaplan2 )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("IN4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,	0x0000, "Continue Mode" )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(		0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(		0x0001, "Discount" )
	/* Various features on bit mask 0x000e - see above */
	/* Coinage on bit mask 0x00f0 - see TOAPLAN2_COINAGE above */
	TOAPLAN2_COINAGE( 0x1C00, 0x0800 )	/* European coinage is shown in service mode, but not used */

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x000c,	0x0000, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(		0x0008, "200k only" )
	PORT_DIPSETTING(		0x0000, "100k only" )
	PORT_DIPSETTING(		0x0004, "100k and every 500k" )
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x0030, "1" )
	PORT_DIPSETTING(		0x0020, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x0010, "4" )
	PORT_DIPNAME( 0x0040,	0x0000, "Invulnerability" )		PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,	0x0000, "Maximum Players" )		PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0080, "2" )
	PORT_DIPSETTING(		0x0000, "4" )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x2000,	0x0000, "Show All Rights Reserved" )
	PORT_DIPSETTING(		0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(		0x2000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c00,	0x0800, "Territory" )
	PORT_DIPSETTING(		0x0800, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x0400, DEF_STR( USA ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x1800, "South East Asia" )
	PORT_DIPSETTING(		0x0c00, "Korea" )
	PORT_DIPSETTING(		0x1000, "Hong Kong" )
	PORT_DIPSETTING(		0x1400, "Taiwan" )
	PORT_BIT( 0xc3ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( sstriker )
	PORT_INCLUDE( toaplan2 )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,	0x0000, DEF_STR( Free_Play ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0001, DEF_STR( On ) )
	/* Various features on bit mask 0x000e - see above */
	/* Coinage on bit mask 0x00f0 - see RAIZING8_COINAGE above */
	TOAPLAN2_COINAGE( 0x000e, 0x0004 )

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x000c,	0x0000, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(		0x0008, "200k only" )
	PORT_DIPSETTING(		0x0000, "Every 300k" )
	PORT_DIPSETTING(		0x0004, "200k and 500k" )
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x0030, "1" )
	PORT_DIPSETTING(		0x0020, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x0010, "5" )
	PORT_DIPNAME( 0x0040,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Yes ) )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x000e,	0x0004, "Territory" )
	PORT_DIPSETTING(		0x0004, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x0002, DEF_STR( USA ) )
//  PORT_DIPSETTING(        0x0000, DEF_STR( Japan ) )  /* Wrong Title screen - use Mahoudai*/
	PORT_DIPSETTING(		0x0006, "South East Asia" )
	PORT_DIPSETTING(		0x0008, "China" )
	PORT_DIPSETTING(		0x000a, "Korea (Unite Trading license)" )
	PORT_DIPSETTING(		0x000c, "Hong Kong" )
	PORT_DIPSETTING(		0x000e, "Taiwan" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( mahoudai )
	PORT_INCLUDE(sstriker)

	PORT_MODIFY("JMPR")
	/* Not used, it seems. This setting forces Japan for Territory */
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( kingdmgp )
	PORT_INCLUDE( toaplan2 )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,	0x0000, DEF_STR( Free_Play ) )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0001, DEF_STR( On ) )
	/* Various features on bit mask 0x000e - see above */
	/* Coinage on bit mask 0x00f0 - see RAIZING8_COINAGE above */
	TOAPLAN2_COINAGE( 0x000e, 0x0004 )

	PORT_MODIFY("DSWB")
	/* Difficulty on bit mask 0x0003 - see above */
	PORT_DIPNAME( 0x000c,	0x0000, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(		0x0008, "200k only" )
	PORT_DIPSETTING(		0x0000, "Every 300k" )
	PORT_DIPSETTING(		0x0004, "200k and 500k" )
	PORT_DIPNAME( 0x0030,	0x0000, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x0030, "1" )
	PORT_DIPSETTING(		0x0020, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x0010, "5" )
	PORT_DIPNAME( 0x0040,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Yes ) )

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x000e,	0x0004, "Territory" )
	PORT_DIPSETTING(		0x0004, DEF_STR( Europe ) )
	PORT_DIPSETTING(		0x0002, DEF_STR( USA ) )
//  PORT_DIPSETTING(        0x0000, DEF_STR( Japan ) )  /* Title screen is wrong when set to Japan */
	PORT_DIPSETTING(		0x0006, "South East Asia" )
	PORT_DIPSETTING(		0x0008, "China" )
	PORT_DIPSETTING(		0x000a, "Korea (Unite Trading license)" )
	PORT_DIPSETTING(		0x000c, "Hong Kong" )
	PORT_DIPSETTING(		0x000e, "Taiwan" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( shippumd )
	PORT_INCLUDE(kingdmgp)

	PORT_MODIFY("JMPR")
	 /* Title screen is corrupt for anything but Japan setting so this forces it to Japan */
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( bgaregga )
	PORT_START("VBL")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* Unknown/Unused */

	PORT_START("DSWA")
	PORT_SERVICE_DIPLOC(0x0001, IP_ACTIVE_HIGH, "SW1:1")
	PORT_DIPNAME( 0x0002,	0x0000, "Credits to Start" )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(		0x0000, "1" )
	PORT_DIPSETTING(		0x0002, "2" )
	PORT_DIPNAME( 0x001c,	0x0000, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(		0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(		0x0014, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(		0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(		0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(		0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(		0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(		0x001c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0,	0x0000, DEF_STR( Coin_B ) )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_NOTEQUALS, 0x001c)	PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(		0x00c0, DEF_STR( 4C_1C ) )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x00a0, DEF_STR( 3C_1C ) )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0080, DEF_STR( 2C_1C ) )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0000, DEF_STR( 1C_1C ) )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
//  PORT_DIPSETTING(        0x00e0, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSWA", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0020, DEF_STR( 1C_2C ) )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0040, DEF_STR( 1C_3C ) )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0060, DEF_STR( 1C_4C ) )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	/* When Coin_A is set to Free_Play, Coin_A becomes Coin_A and Coin_B, and the following dips occur */
	PORT_DIPNAME( 0x0020,	0x0000, "Joystick Mode" )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_EQUALS, 0x001c)	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(		0x0000, "90 degrees ACW" )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPSETTING(		0x0020, DEF_STR( Normal ) )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPNAME( 0x0040,	0x0000, "Effect" )				PORT_CONDITION("DSWA", 0x001c, PORTCOND_EQUALS, 0x001c)	PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )			PORT_CONDITION("DSWA", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPNAME( 0x0080,	0x0000, "Music" )				PORT_CONDITION("DSWA", 0x001c, PORTCOND_EQUALS, 0x001c)	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )		PORT_CONDITION("DSWA", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPSETTING(		0x0080, DEF_STR( On ) )			PORT_CONDITION("DSWA", 0x001c, PORTCOND_EQUALS, 0x001c)

	PORT_START("DSWB")
	PORT_DIPNAME( 0x0003,	0x0000, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(		0x0003, DEF_STR( Hardest ) )
	PORT_DIPSETTING(		0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(		0x0001, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x0004,	0x0000, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008,	0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(		0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0070,	0x0000, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(		0x0030, "1" )
	PORT_DIPSETTING(		0x0020, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x0010, "4" )
	PORT_DIPSETTING(		0x0040, "5" )
	PORT_DIPSETTING(		0x0050, "6" )
	PORT_DIPSETTING(		0x0060, DEF_STR( Infinite ) )
	PORT_DIPSETTING(		0x0070, "Invulnerability (Cheat)" )
	PORT_DIPNAME( 0x0080,	0x0000, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(		0x0000, DEF_STR( None ) )		PORT_CONDITION("JMPR",0x0003,PORTCOND_NOTEQUALS,0x0000)	/* Non-Japan */
	PORT_DIPSETTING(		0x0080, "Every 2000k" )			PORT_CONDITION("JMPR",0x0003,PORTCOND_NOTEQUALS,0x0000)	/* Non-Japan */
	PORT_DIPSETTING(		0x0080, "1000k and 2000k" )		PORT_CONDITION("JMPR",0x0003,PORTCOND_EQUALS,0x0000)	/* Japan */
	PORT_DIPSETTING(		0x0000, "Every 1000k" )			PORT_CONDITION("JMPR",0x0003,PORTCOND_EQUALS,0x0000)	/* Japan */

	PORT_START("JMPR")
	PORT_DIPNAME( 0x0008,	0x0000, "Stage Edit" )
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004,	0x0000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(		0x0004, DEF_STR( No ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0003,	0x0001, "Territory" )
	PORT_DIPSETTING(		0x0001, "Europe (German Tuning license)" )
	PORT_DIPSETTING(		0x0002, "USA (Fabtek license)" )
	PORT_DIPSETTING(		0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x0003, DEF_STR( Asia ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareghk )
	PORT_INCLUDE(bgaregga)

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x0003,	0x0003, "Territory" )
	PORT_DIPSETTING(		0x0001, "Austria (German Tuning license)" )
	/* These two settings end up reporting ROM-0 as BAD */
//  PORT_DIPSETTING(        0x0002, "USA (Fabtek license)" )
//  PORT_DIPSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x0003, "Hong Kong (Metrotainment license)" )
INPUT_PORTS_END


static INPUT_PORTS_START( bgaregtw )
	PORT_INCLUDE(bgaregga)

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x0003,	0x0003, "Territory" )
	PORT_DIPSETTING(		0x0001, "Germany (German Tuning license)" )
	/* These two settings end up reporting ROM-0 as BAD */
//  PORT_DIPSETTING(        0x0002, "USA (Fabtek license)" )
//  PORT_DIPSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x0003, "Taiwan (Liang Hwa license)" )
INPUT_PORTS_END


static INPUT_PORTS_START( bgaregcn )
	PORT_INCLUDE(bgaregga)

	PORT_MODIFY("JMPR")
	PORT_DIPNAME( 0x0003,	0x0003, "Territory" )
	PORT_DIPSETTING(		0x0001, "Denmark (German Tuning license)" )
	// These two settings end up reporting ROM-0 as BAD
//  PORT_DIPSETTING(        0x0002, "USA (Fabtek license)" )
//  PORT_DIPSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(		0x0003, "China" )
INPUT_PORTS_END


static INPUT_PORTS_START( batrider )
	PORT_START("VBL")		/* VBlank */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN")		/* Player Inputs */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")		/* DSWA and DSWB */
	PORT_SERVICE_DIPLOC(0x0001, IP_ACTIVE_HIGH, "SW1:1")
	PORT_DIPNAME( 0x0002,	0x0000, "Credits to Start" )	PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(		0x0000, "1" )					PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0002, "2" )					PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPNAME( 0x0002,	0x0000, "Joystick Mode" )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)		PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(		0x0000, DEF_STR( Normal ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPSETTING(		0x0002, "90 degrees ACW" )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPNAME( 0x001c,	0x0000, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:5,4,3")
	PORT_DIPSETTING(		0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(		0x0014, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(		0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(		0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(		0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(		0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(		0x001c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0,	0x0000, DEF_STR( Coin_B ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)	PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(		0x00c0, DEF_STR( 4C_1C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x00a0, DEF_STR( 3C_1C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0080, DEF_STR( 2C_1C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0000, DEF_STR( 1C_1C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
//  PORT_DIPSETTING(        0x00e0, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0020, DEF_STR( 1C_2C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0040, DEF_STR( 1C_3C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0060, DEF_STR( 1C_4C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	/* When Coin_A is set to Free_Play, Coin_A becomes Coin_A and Coin_B, and the following dips occur */
	PORT_DIPNAME( 0x0020,	0x0000, "Hit Score" )			PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)		PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPSETTING(		0x0020, DEF_STR( On ) )			PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPNAME( 0x0040,	0x0000, "Sound Effect" )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)		PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )			PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPNAME( 0x0080,	0x0000, "Music" )				PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)		PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPSETTING(		0x0080, DEF_STR( On ) )			PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPNAME( 0x0300,	0x0000, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(		0x0300, DEF_STR( Hardest ) )
	PORT_DIPSETTING(		0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(		0x0100, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x0c00,	0x0000, "Timer" )				PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x0c00, DEF_STR( Highest ) )
	PORT_DIPSETTING(		0x0800, DEF_STR( High ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(		0x0400, DEF_STR( Low ) )
	PORT_DIPNAME( 0x3000,	0x0000, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x3000, "1" )
	PORT_DIPSETTING(		0x2000, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x1000, "4" )
	PORT_DIPNAME( 0xc000,	0x0000, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(		0xc000, DEF_STR( None ) )
	PORT_DIPSETTING(		0x8000, "Every 2000k" )
	PORT_DIPSETTING(		0x0000, "Every 1500k" )
	PORT_DIPSETTING(		0x4000, "Every 1000k" )

	PORT_START("SYS-DSW")	/* Coin/System and DSWC */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* Service */
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Test */
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0100,	0x0000, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200,	0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(		0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400,	0x0000, "Stage Edit" )				PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800,	0x0000, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(		0x0800, DEF_STR( No ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1000,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x1000, DEF_STR( On ) )
	/* These Dips are showed only when Coin_A is set to Free_Play, but they work in normal play mode too. */
	PORT_DIPNAME( 0x2000,	0x0000, "Guest Player" )			PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(		0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000,	0x0000, "Player Select" )			PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(		0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000,	0x0000, "Special Course" )			PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(		0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( batridrj )
	PORT_INCLUDE( batrider )

	PORT_MODIFY("SYS-DSW")	/* Coin/System and DSWC */
	/* These Dips are showed only when Coin_A is set to Free_Play, but they work in normal play mode too. */
	PORT_DIPNAME( 0x2000,	0x0000, "Guest Player" )			PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000,	0x0000, "Player Select" )			PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000,	0x0000, "Special Course" )			PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x8000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bbakraid )
	PORT_START("VBL")		/* VBlank */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN")		/* Player Inputs */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")		/* DSWA and DSWB */
	PORT_SERVICE_DIPLOC(0x0001, IP_ACTIVE_HIGH, "SW1:1")
	PORT_DIPNAME( 0x0002,	0x0000, "Credits to Start" )	PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(		0x0000, "1" )					PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0002, "2" )					PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPNAME( 0x0002,	0x0000, "Joystick Mode" )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)		PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(		0x0000, DEF_STR( Normal ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPSETTING(		0x0002, "90 degrees ACW" )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPNAME( 0x001c,	0x0000, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:5,4,3")
	PORT_DIPSETTING(		0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(		0x0014, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(		0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(		0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(		0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(		0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(		0x001c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0,	0x0000, DEF_STR( Coin_B ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)	PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(		0x00c0, DEF_STR( 4C_1C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x00a0, DEF_STR( 3C_1C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0080, DEF_STR( 2C_1C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0000, DEF_STR( 1C_1C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
//  PORT_DIPSETTING(        0x00e0, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0020, DEF_STR( 1C_2C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0040, DEF_STR( 1C_3C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	PORT_DIPSETTING(		0x0060, DEF_STR( 1C_4C ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_NOTEQUALS, 0x001c)
	/* When Coin_A is set to Free_Play, Coin_A becomes Coin_A and Coin_B, and the following dips occur */
	PORT_DIPNAME( 0x0020,	0x0000, "Hit Score" )			PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPSETTING(		0x0020, DEF_STR( On ) )			PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPNAME( 0x0040,	0x0000, "Sound Effect" )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)	PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPSETTING(		0x0040, DEF_STR( On ) )			PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPNAME( 0x0080,	0x0000, "Music" )				PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )		PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPSETTING(		0x0080, DEF_STR( On ) )			PORT_CONDITION("DSW", 0x001c, PORTCOND_EQUALS, 0x001c)
	PORT_DIPNAME( 0x0300,	0x0000, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(		0x0300, DEF_STR( Hardest ) )
	PORT_DIPSETTING(		0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(		0x0100, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x0c00,	0x0000, "Timer" )				PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(		0x0c00, DEF_STR( Highest ) )
	PORT_DIPSETTING(		0x0800, DEF_STR( High ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(		0x0400, DEF_STR( Low ) )
	PORT_DIPNAME( 0x3000,	0x0000, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(		0x3000, "1" )
	PORT_DIPSETTING(		0x2000, "2" )
	PORT_DIPSETTING(		0x0000, "3" )
	PORT_DIPSETTING(		0x1000, "4" )
	PORT_DIPNAME( 0xc000,	0x0000, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(		0xc000, DEF_STR( None ) )
	PORT_DIPSETTING(		0x8000, "Every 4000k" )
	PORT_DIPSETTING(		0x4000, "Every 3000k" )
	PORT_DIPSETTING(		0x0000, "Every 2000k" )

	PORT_START("SYS-DSW")	/* Coin/System and DSW-3 */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* Service */
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Test */
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_DIPNAME( 0x0100,	0x0000, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200,	0x0000, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(		0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400,	0x0000, "Stage Edit" )				PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800,	0x0000, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(		0x0800, DEF_STR( No ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1000,	0x0000, "Invulnerability" )			PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000,	0x0000, "Save Scores" )				PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(		0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000,	0x0000, DEF_STR( Unused ) )			PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000,	0x0000, DEF_STR( Unused ) )			PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(		0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(		0x8000, DEF_STR( On ) )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
INPUT_PORTS_END



static const gfx_layout tilelayout =
{
	16,16,			/* 16x16 */
	RGN_FRAC(1,2),	/* Number of tiles */
	4,				/* 4 bits per pixel */
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*16+0, 8*16+1, 8*16+2, 8*16+3, 8*16+4, 8*16+5, 8*16+6, 8*16+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	8*4*16
};

static const gfx_layout spritelayout =
{
	8,8,			/* 8x8 */
	RGN_FRAC(1,2),	/* Number of 8x8 sprites */
	4,				/* 4 bits per pixel */
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout raizing_textlayout =
{
	8,8,	/* 8x8 characters */
	1024,	/* 1024 characters */
	4,		/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

#define XOR(a) WORD_XOR_LE(a)
#define LOC(x) (x+XOR(0))

static const gfx_layout truxton2_tx_tilelayout =
{
	8,8,	/* 8x8 characters */
	1024,	/* 1024 characters */
	4,		/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ LOC(0)*4, LOC(1)*4, LOC(4)*4, LOC(5)*4, LOC(8)*4, LOC(9)*4, LOC(12)*4, LOC(13)*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};

static const gfx_layout batrider_tx_tilelayout =
{
	8,8,	/* 8x8 characters */
	1024,	/* 1024 characters */
	4,		/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ XOR(0)*4, XOR(1)*4, XOR(2)*4, XOR(3)*4, XOR(4)*4, XOR(5)*4, XOR(6)*4, XOR(7)*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout fixeighblayout =
{
   8,8,
   RGN_FRAC(1,1),
   4,
   { 0,1,2,3 },
   { 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4},
   { 0*4*8, 1*4*8, 2*4*8, 3*4*8, 4*4*8, 5*4*8, 6*4*8, 7*4*8},
   8*8*4
};

static GFXDECODE_START( toaplan2 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   0, 0x1000 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0, 0x1000 )
GFXDECODE_END

static GFXDECODE_START( t2dualvdp )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   0, 0x1000 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0, 0x1000 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,   0, 0x1000 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 0x1000 )
GFXDECODE_END

static GFXDECODE_START( truxton2 )
	GFXDECODE_ENTRY( "gfx1", 0,       tilelayout            , 0, 0x1000 )
	GFXDECODE_ENTRY( "gfx1", 0,       spritelayout          , 0, 0x1000 )
	GFXDECODE_ENTRY( NULL, 0, truxton2_tx_tilelayout,  0, 128 )
GFXDECODE_END

static GFXDECODE_START( raizing )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,         0, 0x1000 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,       0, 0x1000 )
	GFXDECODE_ENTRY( "gfx2", 0, raizing_textlayout, 0, 128 )		/* Extra-text layer */
GFXDECODE_END

static GFXDECODE_START( batrider )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,             0, 0x1000 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,           0, 0x1000 )
	GFXDECODE_ENTRY( NULL,           0, batrider_tx_tilelayout, 0,  16 )
GFXDECODE_END

static GFXDECODE_START( fixeighb )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout     , 0, 0x1000 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout   , 0, 0x1000 )
	GFXDECODE_ENTRY( "gfx2", 0, fixeighblayout , 0, 128 )
GFXDECODE_END

static void irqhandler(device_t *device, int linestate)
{
	toaplan2_state *state = device->machine->driver_data<toaplan2_state>();

	if (state->sub_cpu != NULL)		// wouldn't tekipaki have problem without this? "mcu" is not generally added
		cpu_set_input_line(state->sub_cpu, 0, linestate);
}

static const ym3812_interface ym3812_config =
{
	irqhandler
};

static const ymz280b_interface ymz280b_config =
{
	bbakraid_irqhandler
};


static MACHINE_CONFIG_START( tekipaki, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)			/* 10MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(tekipaki_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

#ifdef USE_HD64x180
	MCFG_CPU_ADD("mcu", Z180, XTAL_10MHz)			/* HD647180 CPU actually */
	MCFG_CPU_PROGRAM_MAP(hd647180_mem)
#endif

	MCFG_MACHINE_RESET(toaplan2)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(toaplan2)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(toaplan2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_27MHz/8)
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( ghox, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)			/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(ghox_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

#ifdef USE_HD64x180
	MCFG_CPU_ADD("mcu", Z180, XTAL_10MHz)			/* HD647180 CPU actually */
	MCFG_CPU_PROGRAM_MAP(hd647180_mem)
#endif

	MCFG_MACHINE_RESET(ghox)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(toaplan2)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(toaplan2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_27MHz/8) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/* probably dogyuun, vfive and kbash use the same decryption table;
those 3 games have been seen with the NITRO905 chip, other alias are
ts002mach for dogyuun, ts004dash for kbash and ts007spy for vfive */

static const UINT8 nitro_decryption_table[256] = {
	0x1b,0x56,0x75,0x88,0x8c,0x06,0x58,0x72, 0x83,0x86,0x36,0x1a,0x5f,0xd3,0x8c,0xe9, /* 00 */
	/* *//* *//* *//* *//* *//* *//* *//* */ /* *//* *//* *//* *//* *//* *//* *//* */
	0x22,0x0f,0x03,0x2a,0xeb,0x2a,0xf9,0x0f, 0xa4,0xbd,0x75,0xf3,0x4f,0x53,0x8e,0xfe, /* 10 */
	/*W*//*W*//*r*//*W*//*r*//*W*//*W*//*r*/ /*r*//*a*//*r*//*r*//*r*//*W*//*x*//*r*/
	0x87,0xe8,0xb1,0x8d,0x36,0xb5,0x43,0x73, 0x2a,0x5b,0xf9,0x02,0x24,0x8a,0x03,0x80, /* 20 */
	/*a*//*r*//*r*//*r*//*x*//*r*//*r*//*r*/ /*W*//*r*//*W*//*W*//*a*//*r*//*a*//*r*/
	0x86,0x8b,0xd1,0x3e,0x8d,0x3e,0x58,0xfb, 0xc3,0x79,0xbd,0xb7,0x8a,0xe8,0x0f,0x81, /* 30 */
	/*a*//*a*//*a*//*a*//*r*//*r*//*a*//*r*/ /*r*//*a*//*a*//*W*//*r*//*r*//*r*//*r*/
	0xb7,0xd0,0x8b,0xeb,0xff,0xb8,0x90,0x8b, 0x5e,0xa2,0x90,0x90,0xab,0xb4,0x80,0x59, /* 40 */
	/*r*//*r*//*a*//*r*//*a*//*x*/     /*a*/ /*W*//*W*/          /*r*//*W*//*r*//*a*/
	0x87,0x72,0xb5,0xbd,0xb0,0x88,0x50,0x0f, 0xfe,0xd2,0xc3,0x90,0x8a,0x90,0xf9,0x75, /* 50 */
	/*W*//*a*//*a*//*r*//*r*//*a*//*a*//*a*/ /*r*//*W*//*r*/     /*r*/     /*W*//*r*/
	0x1a,0xb3,0x74,0x0a,0x68,0x24,0xbb,0x90, 0x75,0x47,0xfe,0x2c,0xbe,0xc3,0x88,0xd2, /* 60 */
	/*W*//*r*//*a*//*r*//*a*//*a*//*W*/      /*r*//*a*//*r*//*W*//*W*//*a*//*r*//*a*/
	0x3e,0xc1,0x8c,0x33,0x0f,0x90,0x8b,0x90, 0xb9,0x1e,0xff,0xa2,0x3e,0x22,0xbe,0x57, /* 70 */
	/*r*//*W*//*r*//*r*//*a*/     /*a*/      /*r*//*r*//*a*//*r*//*a*//*W*//*r*//*a*/
	0x81,0x3a,0xf6,0x88,0xeb,0xb1,0x89,0x8a, 0x32,0x80,0x0f,0xb1,0x48,0xc3,0x68,0x72, /* 80 */
	/*r*//*r*//*r*//*r*//*a*//*W*//*a*//*r*/ /*r*//*r*//*r*//*a*//*x*//*a*//*a*//*r*/
	0x53,0x02,0xc0,0x02,0xe8,0xb4,0x74,0xbc, 0x90,0x58,0x0a,0xf3,0x75,0xc6,0x90,0xe8, /* 90 */
	/*a*//*W*//*r*//*W*//*r*//*r*//*r*//*x*/      /*a*//*r*//*r*//*r*//*x*/     /*r*/
	0x26,0x50,0xfc,0x8c,0x90,0xb1,0xc3,0xd1, 0xeb,0x83,0xa4,0xbf,0x26,0x4b,0x46,0xfe, /* a0 */
	/*r*//*a*//*a*//*r*/     /*a*//*r*//*W*/ /*a*//*r*//*r*//*r*//*r*//*W*//*a*//*r*/
	0xe2,0x89,0xb3,0x88,0x03,0x56,0x0f,0x38, 0xbb,0x0c,0x90,0x0f,0x07,0x8a,0x8a,0x33, /* b0 */
	/*r*//*a*//*W*//*r*//*a*//*W*//*r*//*W*/ /*W*//*W*/     /*a*//*r*//*r*//*r*//*x*/
	0xfe,0xf9,0xb1,0xa0,0x45,0x36,0x22,0x5e, 0x8a,0xbe,0xc6,0xea,0x3c,0xb2,0x1e,0xe8, /* c0 */
	/*r*//*W*//*r*//*r*//*r*//*r*//*W*//*r*/ /*r*//*W*//*x*//*x*//*r*//*?*//*r*//*r*/
	0x90,0xeb,0x55,0xf6,0x8a,0xb0,0x5d,0xc0, 0xbb,0x8d,0xf6,0xd0,0xd1,0x88,0x4d,0x90, /* d0 */
	     /*a*//*r*//*r*//*a*//*a*//*r*//*W*/ /*x*//*r*//*r*//*a*//*W*//*r*//*W*/
	0x51,0x51,0x74,0xbd,0x32,0xd1,0x90,0xd2, 0x53,0xc7,0xab,0x36,0x50,0xe9,0x33,0xb3, /* e0 */
	/*r*//*a*//*r*//*r*//*r*//*W*/     /*a*/ /*r*//*x*//*r*//*r*//*W*//*a*//*r*//*W*/
	0x2e,0x05,0x88,0x59,0x74,0x74,0x22,0x8e, 0x8a,0x8a,0x36,0x08,0x0f,0x45,0x90,0x2e, /* f0 */
	/*r*//*W*//*r*//*r*//*a*//*a*//*W*//*x*/ /*r*//*r*//*x*//*a*//*r*//*a*/     /*r*/
};

/*
dogyuun
a5272 cd

kbash

vfive
a4849 cd

*/

static const nec_config nitro_config ={ nitro_decryption_table, };

static MACHINE_CONFIG_START( dogyuun, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_25MHz/2)			/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(dogyuun_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	MCFG_CPU_ADD("audiocpu", V25, XTAL_25MHz/2)			/* NEC V25 type Toaplan marked CPU ??? */
	MCFG_CPU_PROGRAM_MAP(V25_rambased_mem)
	MCFG_CPU_IO_MAP(V25_port_abswap)
	MCFG_CPU_CONFIG(nitro_config)

	MCFG_MACHINE_RESET(dogyuun)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( (XTAL_27MHz / 4) / (432 * 263) )	/* 27MHz Oscillator */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(dogyuun)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(t2dualvdp)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0
	MCFG_DEVICE_ADD_VDP1

	MCFG_VIDEO_START(toaplan2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_27MHz/8) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_OKIM6295_ADD("oki", XTAL_25MHz/24, OKIM6295_PIN7_HIGH) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( kbash, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)			/* 16MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(kbash_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	/* ROM based v25 */
	MCFG_CPU_ADD("audiocpu", V25, XTAL_16MHz)			/* NEC V25 type Toaplan marked CPU ??? */
	MCFG_CPU_PROGRAM_MAP(V25_kbash_mem)
	MCFG_CPU_IO_MAP(V25_port)
	MCFG_CPU_CONFIG(nitro_config)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(toaplan2)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(toaplan2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_27MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_OKIM6295_ADD("oki", XTAL_32MHz/32, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( kbash2, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)			/* 16MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(kbash2_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	MCFG_MACHINE_RESET(toaplan2)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(toaplan2)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(toaplan2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki1", XTAL_16MHz/16, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki2", XTAL_16MHz/16, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( truxton2, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)			/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(truxton2_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq2)

	MCFG_MACHINE_RESET(toaplan2)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( (XTAL_27MHz / 4) / (432 * 263) )	/* 27MHz Oscillator */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(truxton2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(truxton2)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(truxton2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_27MHz/8) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_16MHz/4, OKIM6295_PIN7_LOW) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( pipibibs, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)			/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(pipibibs_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_27MHz/8)			/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(pipibibs_sound_z80_mem)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(toaplan2)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(toaplan2)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(toaplan2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_27MHz/8)			/* verified on pcb */
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( whoopee, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)			/* 10MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(tekipaki_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

#ifdef USE_HD64x180
	MCFG_CPU_ADD("mcu", Z180, XTAL_10MHz)			/* HD647180 CPU actually */
	MCFG_CPU_PROGRAM_MAP(hd647180_mem)
#endif

	MCFG_MACHINE_RESET(toaplan2)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(toaplan2)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(toaplan2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_27MHz/8)
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( pipibibi_bootleg, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)			/* 10MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(pipibibi_bootleg_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_27MHz/8)			/* ??? 3.37MHz */
	MCFG_CPU_PROGRAM_MAP(pipibibs_sound_z80_mem)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(toaplan2)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(toaplan2)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(toaplan2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_27MHz/8)
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/* x = modified to match batsugun 'unencrypted' code - '?' likewise, but not so sure about them */
/* this one seems more different to the other tables */
static const UINT8 ts001turbo_decryption_table[256] = {
	0x90,0x05,0x57,0x5f,0xfe,0x4f,0xbd,0x36, 0x80,0x8b,0x8a,0x0a,0x89,0x90,0x47,0x80, /* 00 */
	     /*r*//*r*//*r*//*r*//*r*//*r*//*r*/ /*r*//*r*//*r*//*r*//*r*/     /*r*//*r*/
	0x22,0x90,0x90,0x5d,0x81,0x3c,0xb5,0x83, 0x68,0xff,0x75,0x75,0x8d,0x5b,0x8a,0x38, /* 10 */
	/*r*/          /*r*//*r*//*r*//*r*//*r*/ /*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x8b,0xeb,0xd2,0x0a,0xb4,0xc7,0x46,0xd1, 0x0a,0x53,0xbd,0x90,0x22,0xff,0x1f,0x03, /* 20 */
	/*a*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/ /*r*//*r*//*r*/     /*r*//*r*//*?*//*r*/
	0xfb,0x45,0xc3,0x02,0x90,0x0f,0x90,0x02, 0x0f,0xb7,0x90,0x24,0xc6,0xeb,0x1b,0x32, /* 30 */
	/*r*//*r*//*r*//*r*/     /*r*/     /*r*/ /*r*//*r*/     /*r*//*r*//*r*//*r*//*r*/
	0x8d,0xb9,0xfe,0x08,0x88,0x90,0x8a,0x8a, 0x75,0x8a,0xbd,0x58,0xfe,0x51,0x1e,0x8b, /* 40 */
	/*r*//*r*//*r*//*r*//*r*/     /*r*//*r*/ /*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x0f,0x22,0xf6,0x90,0xc3,0x36,0x03,0x8d, 0xbb,0x16,0xbc,0x90,0x0f,0x5e,0xf9,0x2e, /* 50 */
	/*r*//*r*//*r*/     /*r*//*r*//*r*//*r*/ /*r*//*?*//*r*/     /*r*//*r*//*r*//*r*/
	0x90,0x90,0x59,0x90,0xbb,0x1a,0x0c,0x8d, 0x89,0x72,0x83,0xa4,0xc3,0xb3,0x8b,0xe9, /* 60 */
	          /*r*/     /*r*//*r*//*r*//*r*/ /*a*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x81,0x43,0xa0,0x2c,0x0f,0x55,0xf3,0x36, 0xb0,0x59,0xe8,0x03,0x26,0xe9,0x22,0xb0, /* 70 */
	/*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/ /*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x90,0x8e,0x24,0x8a,0xd0,0x3e,0xc3,0x3a, 0x90,0x79,0x57,0x16,0x88,0x86,0x24,0x74, /* 80 */
	     /*r*//*r*//*r*//*r*//*r*//*r*//*r*/      /*a*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x33,0xc3,0x53,0xb8,0xab,0x75,0x90,0x90, 0x8e,0xb1,0xe9,0x5d,0xf9,0x02,0x3c,0x90, /* 90 */
	/*x*//*r*//*r*//*r*//*r*//*r*/           /*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x80,0xd3,0x89,0xe8,0x90,0x90,0x2a,0x74, 0x90,0x5f,0xf6,0x88,0x4f,0x56,0x8c,0x03, /* a0 */
	/*r*//*a*//*r*//*r*/          /*r*//*r*/      /*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x47,0x90,0x88,0x90,0x03,0xfe,0x90,0xfc, 0x2a,0x90,0x33,0x07,0xb1,0x50,0x0f,0x3e, /* b0 */
	/*r*/     /*r*/     /*r*//*r*/     /*r*/ /*r*/     /*r*//*r*//*r*//*r*//*r*//*r*/
	0xbd,0x4d,0xf3,0xbf,0x59,0xd2,0xea,0xc6, 0x2a,0x74,0x72,0xe2,0x3e,0x2e,0x90,0x2e, /* c0 */
	/*r*//*r*//*r*//*r*//*r*//*a*//*x*//*r*/ /*r*//*r*//*r*//*r*//*r*//*r*/     /*r*/
	0x2e,0x73,0x88,0x72,0x45,0x5d,0xc1,0xb9, 0x32,0x38,0x88,0xc1,0xa0,0x06,0x45,0x90, /* d0 */
	/*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/ /*r*//*r*//*r*//*r*//*a*//*r*//*r*/
	0x90,0x86,0x4b,0x87,0x90,0x8a,0x3b,0xab, 0x33,0xbe,0x90,0x32,0xbd,0xc7,0xb2,0x80, /* e0 */
	     /*r*//*r*//*r*/     /*r*//*?*//*r*/ /*r*//*r*/     /*r*//*r*//*r*//*?*//*r*/
	0x0f,0x75,0xc0,0xb9,0x07,0x74,0x3e,0xa2, 0x8a,0x48,0x3e,0x8d,0xeb,0x90,0xfe,0x90, /* f0 */
	/*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/ /*r*//*x*//*r*//*r*//*r*/     /*r*/
};

static const nec_config ts001turbo_config ={ ts001turbo_decryption_table, };


static MACHINE_CONFIG_START( fixeight, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)			/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(fixeight_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	MCFG_CPU_ADD("audiocpu", V25, XTAL_16MHz)			/* NEC V25 type Toaplan marked CPU ??? */
	MCFG_CPU_PROGRAM_MAP(V25_fixeight_mem)
//  IO map must be different - has eeprom but no dips or jumpers */
	MCFG_CPU_CONFIG(ts001turbo_config)
	MCFG_CPU_IO_MAP(V25_fixeight_port)

	MCFG_MACHINE_RESET(batsugun)
//  MCFG_MACHINE_RESET(toaplan2)

	//  MCFG_NVRAM_HANDLER(fixeight)        /* See 37B6 code */

	MCFG_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( (XTAL_27MHz / 4) / (432 * 263) )	/* 27MHz Oscillator */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(truxton2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(truxton2)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(truxton2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_27MHz/8) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_OKIM6295_ADD("oki", XTAL_16MHz/16, OKIM6295_PIN7_HIGH) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( fixeighb, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)			/* 10MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(fixeighb_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq2)

	MCFG_MACHINE_RESET(toaplan2)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(truxton2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(fixeighb)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(fixeighb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_14MHz/16, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( vfive, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz/2)	/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(vfive_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	MCFG_CPU_ADD("audiocpu", V25, XTAL_20MHz/2)	/* Verified on pcb, NEC V25 type Toaplan mark scratched out */
	MCFG_CPU_PROGRAM_MAP(V25_rambased_nooki_mem)
	MCFG_CPU_IO_MAP(V25_port)
	MCFG_CPU_CONFIG(nitro_config)

	MCFG_MACHINE_RESET(vfive)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( (XTAL_27MHz / 4) / (432 * 263) )	/* verified on pcb */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_DEVICE_ADD_VDP0

	MCFG_GFXDECODE(toaplan2)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_VIDEO_START(toaplan2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_27MHz/8) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_RESET(batsugun)
{
	toaplan2_state *state = machine->driver_data<toaplan2_state>();

	cpu_set_input_line(state->sub_cpu, INPUT_LINE_RESET, ASSERT_LINE);
}


static MACHINE_CONFIG_START( batsugun, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_32MHz/2)			/* 16MHz , 32MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(batsugun_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	MCFG_CPU_ADD("audiocpu", V25, XTAL_32MHz/2)			/* NEC V25 type Toaplan marked CPU ??? */
	MCFG_CPU_PROGRAM_MAP(V25_rambased_mem)
	MCFG_CPU_IO_MAP(V25_port)

	MCFG_MACHINE_RESET(batsugun)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(batsugun)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(t2dualvdp)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0
	MCFG_DEVICE_ADD_VDP1

	MCFG_VIDEO_START(toaplan2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_27MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_OKIM6295_ADD("oki", XTAL_32MHz/8, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( snowbro2, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(snowbro2_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	MCFG_MACHINE_RESET(toaplan2)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(toaplan2)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(toaplan2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_27MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_27MHz/10, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( mahoudai, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_32MHz/2)		/* 16MHz , 32MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(mahoudai_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_32MHz/8)		/* 4MHz , 32MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(raizing_sound_z80_mem)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(toaplan2)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(truxton2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(raizing)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(bgaregga)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_27MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_32MHz/32, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( shippumd, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_32MHz/2)		/* 16MHz , 32MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(shippumd_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_32MHz/8)		/* 4MHz , 32MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(raizing_sound_z80_mem)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(toaplan2)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(truxton2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_DEVICE_ADD_VDP0

	MCFG_GFXDECODE(raizing)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_VIDEO_START(bgaregga)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_27MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono",1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_32MHz/32, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static const nmk112_interface bgaregga_nmk112_intf =
{
	"oki", NULL, 0
};

static const nmk112_interface batrider_nmk112_intf =
{
	"oki1", "oki2", 0
};


static MACHINE_CONFIG_START( bgaregga, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_32MHz/2)		/* 16MHz , 32MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(bgaregga_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq4)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_32MHz/8)		/* 4MHz , 32MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(bgaregga_sound_z80_mem)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_RESET(bgaregga)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(truxton2)
	MCFG_SCREEN_EOF(toaplan2)

	MCFG_GFXDECODE(raizing)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(bgaregga)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_32MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_32MHz/16, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NMK112_ADD("nmk112", bgaregga_nmk112_intf)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( batrider, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_32MHz/2)		/* 16MHz , 32MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(batrider_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq2)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_32MHz/8)		/* 4MHz , 32MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(batrider_sound_z80_mem)
	MCFG_CPU_IO_MAP(batrider_sound_z80_port)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(bgaregga)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(batrider)

	MCFG_GFXDECODE(batrider)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(batrider)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_32MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki1", XTAL_32MHz/10, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki2", XTAL_32MHz/10, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NMK112_ADD("nmk112", batrider_nmk112_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( bbakraid, toaplan2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_32MHz/2)		/* 16MHz , 32MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(bbakraid_68k_mem)
	MCFG_CPU_VBLANK_INT("screen", toaplan2_vblank_irq1)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_32MHz/6)		/* 5.3333MHz , 32MHz Oscillator */
	MCFG_CPU_PROGRAM_MAP(bbakraid_sound_z80_mem)
	MCFG_CPU_IO_MAP(bbakraid_sound_z80_port)
	MCFG_CPU_PERIODIC_INT(bbakraid_snd_interrupt, 448)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(toaplan2)
	MCFG_EEPROM_ADD("eeprom", bbakraid_93C66_intf)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(432, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(batrider)

	MCFG_GFXDECODE(batrider)
	MCFG_PALETTE_LENGTH(T2PALETTE_LENGTH)

	MCFG_DEVICE_ADD_VDP0

	MCFG_VIDEO_START(batrider)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MCFG_SOUND_CONFIG(ymz280b_config)
	MCFG_SOUND_ROUTE(0, "mono", 1.0)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bbakradu, bbakraid )

	MCFG_DEVICE_REMOVE("eeprom")
	MCFG_EEPROM_ADD("eeprom", bbakraid_93C66_intf)
	MCFG_EEPROM_DATA(bbakraid_unlim_default, 512)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/* -------------------------- Toaplan games ------------------------- */
ROM_START( tekipaki )
	ROM_REGION( 0x040000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "tp020-1.bin", 0x000000, 0x010000, CRC(d8420bd5) SHA1(30c1ad9e053cd7e79adb42aa428ebee28e144755) )
	ROM_LOAD16_BYTE( "tp020-2.bin", 0x000001, 0x010000, CRC(7222de8e) SHA1(8352ae23efc24a2e20cc24b6d37cb8fc6b1a730c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.020", 0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "tp020-4.bin", 0x000000, 0x080000, CRC(3ebbe41e) SHA1(cea196c5f83e1a23d5b538a0db9bbbffa7af5118) )
	ROM_LOAD( "tp020-3.bin", 0x080000, 0x080000, CRC(2d5e2201) SHA1(5846c844eedd48305c1c67dc645b6e070b3f5b98) )
ROM_END


ROM_START( ghox ) /* Spinner with single axis (up/down) controls */
	ROM_REGION( 0x040000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "tp021-01.u10", 0x000000, 0x020000, CRC(9e56ac67) SHA1(daf241d9e55a6e60fc004ed61f787641595b1e62) )
	ROM_LOAD16_BYTE( "tp021-02.u11", 0x000001, 0x020000, CRC(15cac60f) SHA1(6efa3a50a5dfe6ef4072738d6a7d0d95dca8a675) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.021", 0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "tp021-03.u36", 0x000000, 0x080000, CRC(a15d8e9d) SHA1(640a33997bdce8e84bea6a944139716379839037) )
	ROM_LOAD( "tp021-04.u37", 0x080000, 0x080000, CRC(26ed1c9a) SHA1(37da8af86ea24327444c2d4ad3dfbd936208d43d) )
ROM_END


ROM_START( ghoxj ) /* 8-way joystick for controls */
	ROM_REGION( 0x040000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "tp021-01a.u10", 0x000000, 0x020000, CRC(c11b13c8) SHA1(da7defc1d3b6ddded910ba56c31fbbdb5ed57b09) )
	ROM_LOAD16_BYTE( "tp021-02a.u11", 0x000001, 0x020000, CRC(8d426767) SHA1(1ed4a8bcbf4352257e7d58cb5c2c91eb48c2f047) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.021", 0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "tp021-03.u36", 0x000000, 0x080000, CRC(a15d8e9d) SHA1(640a33997bdce8e84bea6a944139716379839037) )
	ROM_LOAD( "tp021-04.u37", 0x080000, 0x080000, CRC(26ed1c9a) SHA1(37da8af86ea24327444c2d4ad3dfbd936208d43d) )
ROM_END


ROM_START( dogyuun )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp022_01.r16", 0x000000, 0x080000, CRC(79eb2429) SHA1(088c5ed0ed77557ab71f52cafe35028e3648ae1e) )

	/* Secondary CPU is a Toaplan marked chip, (TS-002-MACH  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_3.w92", 0x000000, 0x100000, CRC(191b595f) SHA1(89344946daa18087cc83f92027cf5da659b1c7a5) )
	ROM_LOAD16_WORD_SWAP( "tp022_4.w93", 0x100000, 0x100000, CRC(d58d29ca) SHA1(90d142fef37764ef817347a2bed77892a288a077) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_5.w16", 0x000000, 0x200000, CRC(d4c1db45) SHA1(f5655467149ba737128c2f54c9c6cdaca6e4c35c) )
	ROM_LOAD16_WORD_SWAP( "tp022_6.w17", 0x200000, 0x200000, CRC(d48dc74f) SHA1(081b5a00a2ff2bd82b98b30aab3cb5b6ae1014d5) )

	ROM_REGION( 0x40000, "oki", 0 )		/* ADPCM Samples */
	ROM_LOAD( "tp022_2.w30", 0x00000, 0x40000, CRC(043271b3) SHA1(c7eaa929e55dd956579b824ea9d20a1d0129a925) )
ROM_END


ROM_START( dogyuunk )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "01.u64", 0x000000, 0x080000, CRC(fe5bd7f4) SHA1(9c725466112a514c9ed0fb074422d291c175c3f4) )

	/* Secondary CPU is a Toaplan marked chip, (TS-002-MACH  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_3.w92", 0x000000, 0x100000, CRC(191b595f) SHA1(89344946daa18087cc83f92027cf5da659b1c7a5) )
	ROM_LOAD16_WORD_SWAP( "tp022_4.w93", 0x100000, 0x100000, CRC(d58d29ca) SHA1(90d142fef37764ef817347a2bed77892a288a077) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_5.w16", 0x000000, 0x200000, CRC(d4c1db45) SHA1(f5655467149ba737128c2f54c9c6cdaca6e4c35c) )
	ROM_LOAD16_WORD_SWAP( "tp022_6.w17", 0x200000, 0x200000, CRC(d48dc74f) SHA1(081b5a00a2ff2bd82b98b30aab3cb5b6ae1014d5) )

	ROM_REGION( 0x40000, "oki", 0 )		/* ADPCM Samples */
	ROM_LOAD( "tp022_2.w30", 0x00000, 0x40000, CRC(043271b3) SHA1(c7eaa929e55dd956579b824ea9d20a1d0129a925) )
ROM_END

ROM_START( dogyuunt )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "sample10.9.u64.bin", 0x000000, 0x080000, CRC(585f5016) SHA1(18d57843f33a560a3bb4b6aef176f7ef795b742d) )

	/* Secondary CPU is a Toaplan marked chip, (TS-002-MACH  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_3.w92", 0x000000, 0x100000, CRC(191b595f) SHA1(89344946daa18087cc83f92027cf5da659b1c7a5) )
	ROM_LOAD16_WORD_SWAP( "tp022_4.w93", 0x100000, 0x100000, CRC(d58d29ca) SHA1(90d142fef37764ef817347a2bed77892a288a077) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_5.w16", 0x000000, 0x200000, CRC(d4c1db45) SHA1(f5655467149ba737128c2f54c9c6cdaca6e4c35c) )
	ROM_LOAD16_WORD_SWAP( "tp022_6.w17", 0x200000, 0x200000, CRC(d48dc74f) SHA1(081b5a00a2ff2bd82b98b30aab3cb5b6ae1014d5) )

	ROM_REGION( 0x40000, "oki", 0 )		/* ADPCM Samples */
	ROM_LOAD( "tp022_2.w30", 0x00000, 0x40000, CRC(043271b3) SHA1(c7eaa929e55dd956579b824ea9d20a1d0129a925) )
ROM_END

ROM_START( kbash )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "kbash01.bin", 0x000000, 0x080000, CRC(2965f81d) SHA1(46f2df30fa92c80ba5a37f75e756424e15534784) )

	/* Secondary CPU is a Toaplan marked chip, (TS-004-Dash  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted) */

	ROM_REGION( 0x8000, "audiocpu", 0 )			/* Sound CPU code */
	ROM_LOAD( "kbash02.bin", 0x0000, 0x8000, CRC(4cd882a1) SHA1(7199a5c384918f775f0815e09c46b2a58141814a) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "kbash03.bin", 0x000000, 0x200000, CRC(32ad508b) SHA1(e473489beaf649d3e5236770eb043327e309850c) )
	ROM_LOAD( "kbash05.bin", 0x200000, 0x200000, CRC(b84c90eb) SHA1(17a1531d884d9a9696d1b25d65f9155f02396e0e) )
	ROM_LOAD( "kbash04.bin", 0x400000, 0x200000, CRC(e493c077) SHA1(0edcfb70483ad07206695d9283031b85cd198a36) )
	ROM_LOAD( "kbash06.bin", 0x600000, 0x200000, CRC(9084b50a) SHA1(03b58278619524d2f09a4b1c152d5e057e792a56) )

	ROM_REGION( 0x40000, "oki", 0 )		/* ADPCM Samples */
	ROM_LOAD( "kbash07.bin", 0x00000, 0x40000, CRC(3732318f) SHA1(f0768459f5ad2dee53d408a0a5ae3a314864e667) )
ROM_END


/*
Knuckle Bash 2
This is a hacked version of Knuckle Bash on bootleg/Korean/Chinese
hardware showing (C)Toaplan 1999 Licensed to Charterfield

PCB Layout
----------

|--------------------------------------------|
|UPC1241  EPROM  MECAT-S                     |
|  LM324                                     |
|        M6295  M6295                        |
| PAL   62256                      M5M51008  |
|       62256    MECAT-M           M5M51008  |
|        6116                      M5M51008  |
|J       6116         14.31818MHz  M5M51008  |
|A             68000                         |
|M                    16MHz                  |
|M                  PAL                      |
|A             PAL                           |
|        |-------|                           |
|        |ACTEL  |         PAL               |
|        |A40MX04|         PAL               |
|        |       |                           |
|   DSW1 |-------|         050917-10         |
|        |ACTEL  |                           |
|   DSW2 |A40MX04| MECAT-12                  |
|62256   |       |                           |
|62256   |-------| MECAT-34                  |
|--------------------------------------------|
Notes:
      68000 clock 16.000MHz
      M6295 clock 1.000MHz [16/16]. Sample rate (Hz) 16000000/16/132
      M5M51008 - Mitsubishi M5M51008 128k x8 SRAM (SOP32)
      62256    - 32k x8 SRAM
      6116     - 2k x8 SRAM
      VSync 60Hz
      HSync 15.68kHz
*/

ROM_START( kbash2 )
	ROM_REGION( 0x80000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "mecat-m", 0x000000, 0x80000, CRC(bd2263c6) SHA1(eb794c0fc9c1fb4337114d48149283d42d22e4b3) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "mecat-34", 0x000000, 0x400000, CRC(6be7b37e) SHA1(13160ad0712fee932bb98cc226e651895b19228a) )
	ROM_LOAD( "mecat-12", 0x400000, 0x400000, CRC(49e46b1f) SHA1(d12b12696a8473eb34f3cd247ab060289a6c0e9c) )

	ROM_REGION( 0x80000, "oki1", 0 )			/* ADPCM Music */
	ROM_LOAD( "mecat-s", 0x00000, 0x80000, CRC(3eb7adf4) SHA1(b0e6e99726b854858bd0e69eb77f12b9664b35e6) )

	ROM_REGION( 0x40000, "oki2", 0 )			/* ADPCM Samples */
	ROM_LOAD( "eprom",   0x00000, 0x40000, CRC(31115cb9) SHA1(c79ea01bd865e2fc3aaab3ff05483c8fd27e5c98) )

	ROM_REGION( 0x10000, "user1", 0 )			/* ??? Some sort of table  - same as in pipibibi*/
	ROM_LOAD( "050917-10", 0x0000, 0x10000, CRC(6b213183) SHA1(599c59d155d11edb151bfaed1d24ef964462a447) )
ROM_END


ROM_START( truxton2 )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	/* program ROM is byte swapped ! */
	ROM_LOAD16_WORD( "tp024_1.bin", 0x000000, 0x080000, CRC(f5cfe6ee) SHA1(30979888a4cd6500244117748f28386a7e20a169) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "tp024_4.bin", 0x000000, 0x100000, CRC(805c449e) SHA1(fdf985344145bd320b88b9b0c25e73066c9b2ada) )
	ROM_LOAD( "tp024_3.bin", 0x100000, 0x100000, CRC(47587164) SHA1(bac493e2d5507286b984957b289c929335d27eaa) )

	ROM_REGION( 0x80000, "oki", 0 )			/* ADPCM Samples */
	ROM_LOAD( "tp024_2.bin", 0x00000, 0x80000, CRC(f2f6cae4) SHA1(bb4e8c36531bed97ced4696ca12fd40ede2531aa) )
ROM_END

ROM_START( pipibibs )
	ROM_REGION( 0x040000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "tp025-1.bin", 0x000000, 0x020000, CRC(b2ea8659) SHA1(400431b656dbfbd5a9bc5961c3ea04c4d38b6f77) )
	ROM_LOAD16_BYTE( "tp025-2.bin", 0x000001, 0x020000, CRC(dc53b939) SHA1(e4de371f97ba7c350273ad43b7f58ff31672a269) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound Z80 code */
	ROM_LOAD( "tp025-5.bin", 0x0000, 0x8000, CRC(bf8ffde5) SHA1(79c09cc9a0ea979f5af5a7e5ad671ea486f5f43e) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "tp025-4.bin", 0x000000, 0x100000, CRC(ab97f744) SHA1(c1620e614345dbd5c6567e4cb6f55c61b900d0ee) )
	ROM_LOAD( "tp025-3.bin", 0x100000, 0x100000, CRC(7b16101e) SHA1(ae0119bbfa0937d18c4fbb0a3ef7cdc3b9fa6b56) )
ROM_END

ROM_START( pipibibsa )
	ROM_REGION( 0x040000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "tp025-1.alt.bin", 0x000000, 0x020000, CRC(3e522d98) SHA1(043dd76b99e130909e47063d4cc773177a2eaccf) )
	ROM_LOAD16_BYTE( "tp025-2.alt.bin", 0x000001, 0x020000, CRC(48370485) SHA1(9895e086c9a5eeec4f454cbc6098adb2f66d4e11) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound Z80 code */
	ROM_LOAD( "tp025-5.bin", 0x0000, 0x8000, CRC(bf8ffde5) SHA1(79c09cc9a0ea979f5af5a7e5ad671ea486f5f43e) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "tp025-4.bin", 0x000000, 0x100000, CRC(ab97f744) SHA1(c1620e614345dbd5c6567e4cb6f55c61b900d0ee) )
	ROM_LOAD( "tp025-3.bin", 0x100000, 0x100000, CRC(7b16101e) SHA1(ae0119bbfa0937d18c4fbb0a3ef7cdc3b9fa6b56) )
ROM_END

ROM_START( whoopee )
	ROM_REGION( 0x040000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "whoopee.1", 0x000000, 0x020000, CRC(28882e7e) SHA1(8fcd278a7d005eb81cd9e461139c0c0f756a4fa4) )
	ROM_LOAD16_BYTE( "whoopee.2", 0x000001, 0x020000, CRC(6796f133) SHA1(d4e657be260ba3fd3f0556ade617882513b52685) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.025", 0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "tp025-4.bin", 0x000000, 0x100000, CRC(ab97f744) SHA1(c1620e614345dbd5c6567e4cb6f55c61b900d0ee) )
	ROM_LOAD( "tp025-3.bin", 0x100000, 0x100000, CRC(7b16101e) SHA1(ae0119bbfa0937d18c4fbb0a3ef7cdc3b9fa6b56) )
ROM_END


ROM_START( pipibibsbl )
	ROM_REGION( 0x040000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "ppbb06.bin", 0x000000, 0x020000, CRC(14c92515) SHA1(2d7f7c89272bb2a8115f163ad651bef3bca5107e) )
	ROM_LOAD16_BYTE( "ppbb05.bin", 0x000001, 0x020000, CRC(3d51133c) SHA1(d7bd94ad11e9aeb5a5165c5ac6f71950849bcd2f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound Z80 code */
	ROM_LOAD( "ppbb08.bin", 0x0000, 0x8000, CRC(101c0358) SHA1(162e02d00b7bdcdd3b48a0cd0527b7428435ec50) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	/* GFX data differs slightly from Toaplan boards ??? */
	ROM_LOAD16_BYTE( "ppbb01.bin", 0x000000, 0x080000, CRC(0fcae44b) SHA1(ac72bc79e3a5d0a81647c312d310d00ace017272) )
	ROM_LOAD16_BYTE( "ppbb02.bin", 0x000001, 0x080000, CRC(8bfcdf87) SHA1(4537a7d646d3014f069c6fd0be457bb32e2f18ac) )
	ROM_LOAD16_BYTE( "ppbb03.bin", 0x100000, 0x080000, CRC(abdd2b8b) SHA1(a4246dd63515f01d1227c9a9e16d9f1c739ee39e) )
	ROM_LOAD16_BYTE( "ppbb04.bin", 0x100001, 0x080000, CRC(70faa734) SHA1(4448f4dbded56c142e57293d371e0a422c3a667e) )

	ROM_REGION( 0x8000, "user1", 0 )			/* ??? Some sort of table */
	ROM_LOAD( "ppbb07.bin", 0x0000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )
ROM_END


#define ROMS_FIXEIGHT \
	ROM_REGION( 0x080000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "tp-026-1", 0x000000, 0x080000, CRC(f7b1746a) SHA1(0bbea6f111b818bc9b9b2060af4fe900f37cf7f9) ) \
	ROM_REGION( 0x400000, "gfx1", 0 ) \
	ROM_LOAD( "tp-026-3", 0x000000, 0x200000, CRC(e5578d98) SHA1(280d2b716d955e767d311fc9596823852435b6d7) ) \
	ROM_LOAD( "tp-026-4", 0x200000, 0x200000, CRC(b760cb53) SHA1(bc9c5e49e45cdda0f774be0038aa4deb21d4d285) ) \
	ROM_REGION( 0x40000, "oki", 0 )	\
	ROM_LOAD( "tp-026-2", 0x00000, 0x40000, CRC(85063f1f) SHA1(1bf4d77494de421c98f6273b9876e60d827a6826) ) \
	ROM_REGION( 0x80, "eepromdumped", 0 ) \
	ROM_LOAD16_WORD_SWAP( "93c45.u21", 0x00, 0x80, CRC(40d75df0) SHA1(a22f1cc74ce9bc9bfe53f48f6a43ab60e921052b) )\

// eeprom dumped can't be accepted by the code, but the values can't be a simple bad dump (not fixed bits and the values are present three times)
// robiza's note: probably between sound cpu and EEPROM there's something that modify the values (PAL?)
// we can get the eeprom with a value in [00004] address (1XXX dcba) -> then we need a different value in [00004] address (0XXX XXXX)
// dcba = 0 -> korea
// dcba = 1 -> korea (taito license)
// dcba = 2 -> hong kong
// dcba = 3 -> hong kong (taito license)
// dcba = 4 -> taiwan
// dcba = 5 -> taiwan (taito license)
// dcba = 6 -> south east asia
// dcba = 7 -> south east asia (taito license)
// dcba = 8 -> europe
// dcba = 9 -> europe (taito license)
// dcba = a -> u.s.a.
// dcba = b -> u.s.a. (taito license)
// dcba = c -> NO COUNTRY
// dcba = d -> NO COUNTRY (taito license)
// dcba = e -> japan
// dcba = f -> japan (taito license)

ROM_START( fixeightkt )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightkt.nv", 0x00, 0x80, CRC(08fa73ba) SHA1(b7761d3dd3f4485e55c8ef2cf1a840ca771ee2fc) )
ROM_END


ROM_START( fixeightk )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightk.nv", 0x00, 0x80, CRC(cac91c6f) SHA1(55b284f081753d60abff63493094322756b7f0c5) )
ROM_END

ROM_START( fixeightht )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightht.nv", 0x00, 0x80, CRC(57edaa51) SHA1(b8d50e82590b8cbbbcafec5f9cfbc91e4c286db5) )
ROM_END

ROM_START( fixeighth )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeighth.nv", 0x00, 0x80, CRC(95dec584) SHA1(1c309074b51da5a5263dee00403296946e41067b) )
ROM_END

ROM_START( fixeighttwt )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeighttwt.nv", 0x00, 0x80, CRC(b6d5c06c) SHA1(7fda380ac6835a983c57d093ccad7bd76893c9ba))
ROM_END

ROM_START( fixeighttw )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeighttw.nv", 0x00, 0x80, CRC(74e6afb9) SHA1(87bdc95eb0d2d54375de2c622557d503e14154be))
ROM_END

ROM_START( fixeightat )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightat.nv", 0x00, 0x80,CRC(e9c21987) SHA1(7f699e38deb84902ed62b857a3d2b4e3ea1475bb) )
ROM_END

ROM_START( fixeighta )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeighta.nv", 0x00, 0x80, CRC(2bf17652) SHA1(4ec6f188e63610d258cd6b2432d2200d61d80bed))
ROM_END

ROM_START( fixeightt )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightt.nv", 0x00, 0x80, CRC(c0da4a05) SHA1(3686161244e3e8be0e2fdb5fc5c24e39a7aeba85) )
ROM_END

ROM_START( fixeight )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeight.nv", 0x00, 0x80, CRC(02e925d0) SHA1(5839d10aceff84916ea99e9c6afcdc90eef7468b) )
ROM_END

ROM_START( fixeightut )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightut.nv", 0x00, 0x80, CRC(9fcd93ee) SHA1(4f2750f09d9b8ff358a2fd6c7a4a8ba6de67017a) )
ROM_END

ROM_START( fixeightu )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightu.nv", 0x00, 0x80, CRC(5dfefc3b) SHA1(5203525c58e2ae10575af2e277a5696bd64c5b60) )
ROM_END

ROM_START( fixeightj )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightj.nv", 0x00, 0x80, CRC(21e22038) SHA1(29fb10061e62799bb5e4171e144daac49f0cdf06) )
ROM_END

ROM_START( fixeightjt )
	ROMS_FIXEIGHT
	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightjt.nv", 0x00, 0x80, CRC(e3d14fed) SHA1(ee4982ef195240c5eaa5005ca1d591901fb01b47) )
ROM_END


/*
Fix Eight (bootleg)
Toaplan, 1992

PCB Layout
----------

|--------------------------------------------|
|   1.BIN        PAL               14MHz  PAL|
|   M6295        PAL                         |
|   PAL     6116 4.BIN          681000 681000|
|           6116                             |
|           6116                681000 681000|
|J          6116        PAL                  |
|A                             PAL           |
|M                                           |
|M   62256  62256              PAL           |
|A   2.BIN  3.BIN       PAL                  |
|                       PAL                  |
|       68000           PAL                  |
| DSW2        |------|  5.BIN                |
| DSW1   6264 |TPC   |                       |
| 3.579545MHz |1020  |  6.BIN                |
| 10MHz  6264 |------|  7.BIN                |
|--------------------------------------------|
Notes:
      68000 clock at 10.000MHz
      M6295 clock at 875kHz [14M/16]. Sample rate = 875000 / 165
      VSync at 60Hz
      6116  - 2k   x8 SRAM (x4)
      6264  - 8k   x8 SRAM (x2)
      62256 - 32k  x8 SRAM (x2)
      681000- 128k x8 SRAM (x4)
*/


ROM_START( fixeightbl )
	ROM_REGION( 0x100000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "3.bin", 0x000000, 0x80000, CRC(cc77d4b4) SHA1(4d3376cbae13d90c6314d8bb9236c2183fc6253c) )
	ROM_LOAD16_BYTE( "2.bin", 0x000001, 0x80000, CRC(ed715488) SHA1(37be9bc8ff6b54a1f660d89469c6c2da6301e9cd) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "tp-026-3", 0x000000, 0x200000, CRC(e5578d98) SHA1(280d2b716d955e767d311fc9596823852435b6d7) )
	ROM_LOAD( "tp-026-4", 0x200000, 0x200000, CRC(b760cb53) SHA1(bc9c5e49e45cdda0f774be0038aa4deb21d4d285) )

	ROM_REGION( 0x08000, "gfx2", 0)
	ROM_LOAD( "4.bin", 0x00000, 0x08000, CRC(a6aca465) SHA1(2b331faeee1832e0adc5218254a99d66331862c6) )

	ROM_REGION( 0x90000, "oki", 0 )			/* ADPCM Samples */
	ROM_LOAD( "1.bin", 0x00000, 0x30000, CRC(888f19ac) SHA1(d2f4f8b7be7a0fdb95baa0af8930e50e2f875c05) )
	ROM_CONTINUE(      0x40000, 0x50000 )

	ROM_REGION( 0x8000, "user1", 0 )			/* ??? Some sort of table  - same as in pipibibi*/
	ROM_LOAD( "5.bin", 0x0000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )
ROM_END


ROM_START( grindstm )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "01.bin", 0x000000, 0x080000, CRC(4923f790) SHA1(1c2d66b432d190d0fb6ac7ca0ec0687aea3ccbf4) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "tp027_02.bin", 0x000000, 0x100000, CRC(877b45e8) SHA1(b3ed8d8dbbe51a1919afc55d619d2b6771971493) )
	ROM_LOAD( "tp027_03.bin", 0x100000, 0x100000, CRC(b1fc6362) SHA1(5e97e3cce31be57689d394a50178cda4d80cce5f) )
ROM_END


ROM_START( grindstma )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp027-01.rom", 0x000000, 0x080000, CRC(8d8c0392) SHA1(824dde274c8bef8a87c54d8ccdda7f0feb8d11e1) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "tp027_02.bin", 0x000000, 0x100000, CRC(877b45e8) SHA1(b3ed8d8dbbe51a1919afc55d619d2b6771971493) )
	ROM_LOAD( "tp027_03.bin", 0x100000, 0x100000, CRC(b1fc6362) SHA1(5e97e3cce31be57689d394a50178cda4d80cce5f) )
ROM_END


ROM_START( vfive )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp027_01.bin", 0x000000, 0x080000, CRC(731d50f4) SHA1(794255d0a809cda9170f5bac473df9d7f0efdac8) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "tp027_02.bin", 0x000000, 0x100000, CRC(877b45e8) SHA1(b3ed8d8dbbe51a1919afc55d619d2b6771971493) )
	ROM_LOAD( "tp027_03.bin", 0x100000, 0x100000, CRC(b1fc6362) SHA1(5e97e3cce31be57689d394a50178cda4d80cce5f) )
ROM_END


ROM_START( batsugun )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp030_1a.bin", 0x000000, 0x080000,  CRC(cb1d4554) SHA1(ef31f24d77e1c13bdf5558a04a6253e2e3e6a790) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (program uploaded by main CPU) */

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "tp030_3l.bin", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "tp030_3h.bin", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "tp030_4l.bin", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "tp030_4h.bin", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "tp030_5.bin",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "tp030_6.bin",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x40000, "oki", 0 )			/* ADPCM Samples */
	ROM_LOAD( "tp030_2.bin", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )
ROM_END


ROM_START( batsuguna )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp030_01.bin", 0x000000, 0x080000, CRC(3873d7dd) SHA1(baf6187d7d554cfcf4a86b63f07fc30df7ef84c9) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (program uploaded by main CPU) */

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "tp030_3l.bin", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "tp030_3h.bin", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "tp030_4l.bin", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "tp030_4h.bin", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "tp030_5.bin",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "tp030_6.bin",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x40000, "oki", 0 )			/* ADPCM Samples */
	ROM_LOAD( "tp030_2.bin", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )
ROM_END


ROM_START( batsugunsp )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp030-sp.u69", 0x000000, 0x080000, CRC(8072a0cd) SHA1(3a0a9cdf894926a16800c4882a2b00383d981367) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (program uploaded by main CPU) */

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "tp030_3l.bin", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "tp030_3h.bin", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "tp030_4l.bin", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "tp030_4h.bin", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "tp030_5.bin",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "tp030_6.bin",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x40000, "oki", 0 )			/* ADPCM Samples */
	ROM_LOAD( "tp030_2.bin", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )
ROM_END


ROM_START( snowbro2 )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "pro-4", 0x000000, 0x080000, CRC(4c7ee341) SHA1(ad46c605a38565d0148daac301be4e4b72302fe7) )

	ROM_REGION( 0x300000, "gfx1", 0 )
	ROM_LOAD( "rom2-l", 0x000000, 0x100000, CRC(e9d366a9) SHA1(e87e3966fce3395324b90db6c134b3345104c04b) )
	ROM_LOAD( "rom2-h", 0x100000, 0x080000, CRC(9aab7a62) SHA1(611f6a15fdbac5d3063426a365538c1482e996bf) )
	ROM_LOAD( "rom3-l", 0x180000, 0x100000, CRC(eb06e332) SHA1(7cd597bfffc153d178530c0f0903bebd751c9dd1) )
	ROM_LOAD( "rom3-h", 0x280000, 0x080000, CRC(df4a952a) SHA1(b76af61c8437caca573ff1312832898666a611aa) )

	ROM_REGION( 0x80000, "oki", 0 )			/* ADPCM Samples */
	ROM_LOAD( "rom4", 0x00000, 0x80000, CRC(638f341e) SHA1(aa3fca25f099339ece1878ea730c5e9f18ec4823) )
ROM_END


/* -------------------------- Raizing games ------------------------- */


/*
For the two sets of Sorcer Striker (World) the only differences
are 2 bytes plus a corrected checksum for each set:

File Offset     sstriker   sstrikra
  0x160            17         0B   <-- Rom checksum value
  0x161            79         6D   <-- Rom checksum value

  0x92C            18         0C   <-- Unknown, but surrounding bytes are 0x18
  0x92D            18         0C   <-- Unknown, but surrounding bytes are 0x18

So it looks like sstriker was a slightly revised "final" version of the World set.
sstrikra has been verified on two boards as being correct.

Printed labels for the eproms look like:

RA-MA-01
   01
RAIZING

Both English and Japanese sets use the same labels and numbers for the roms
even if the roms contain different code / data

*/

ROM_START( sstriker )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra-ma-01_01.u65", 0x000000, 0x080000, CRC(92259f84) SHA1(127e62e407d95efd360bfe2cac9577f326abf6ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )

	ROM_REGION( 0x008000, "gfx2", 0 )
	ROM_LOAD( "ra-ma-01_05.u81",  0x000000, 0x008000, CRC(88b58841) SHA1(1d16b538c11a291bd1f46a510bfbd6259b45a0b5) )

	ROM_REGION( 0x40000, "oki", 0 )			/* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( sstrikera )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra-ma_01_01.u65", 0x000000, 0x080000, CRC(708fd51d) SHA1(167186d4cf13af37ec0fa6a59c738c54dbbf3c7c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )

	ROM_REGION( 0x008000, "gfx2", 0 )
	ROM_LOAD( "ra-ma-01_05.u81",  0x000000, 0x008000, CRC(88b58841) SHA1(1d16b538c11a291bd1f46a510bfbd6259b45a0b5) )

	ROM_REGION( 0x40000, "oki", 0 )			/* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( mahoudai )
	ROM_REGION( 0x080000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra_ma_01_01.u65", 0x000000, 0x080000, CRC(970ccc5c) SHA1(c87cab83bde0284e631f02e50068407fee81d941) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )

	ROM_REGION( 0x008000, "gfx2", 0 )
	ROM_LOAD( "ra_ma_01_05.u81",  0x000000, 0x008000, CRC(c00d1e80) SHA1(53e64c4c0c6309130b37597d13b44a9e95b717d8) )

	ROM_REGION( 0x40000, "oki", 0 )			/* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( kingdmgp )
	ROM_REGION( 0x100000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "ma02rom1.bin", 0x000000, 0x080000, CRC(a678b149) SHA1(8c1a631e023dbba0a3fa6cd1b7d10dec1663213a) )
	ROM_LOAD16_BYTE( "ma02rom0.bin", 0x000001, 0x080000, CRC(f226a212) SHA1(526acf3d05fdc88054a772fbea3de2af532bf3d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound Z80 code */
	ROM_LOAD( "ma02rom2.bin", 0x00000, 0x10000, CRC(dde8a57e) SHA1(f522a3f17e229c71512464349760a9e27778bf6a) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "ma02rom3.bin",  0x000000, 0x200000, CRC(0e797142) SHA1(a480ccd151e49b886d3175a6deff56e1f2c26c3e) )
	ROM_LOAD( "ma02rom4.bin",  0x200000, 0x200000, CRC(72a6fa53) SHA1(ce92e65205b84361cfb90305a61e9541b5c4dc2f) )

	ROM_REGION( 0x008000, "gfx2", 0 )
	ROM_LOAD( "ma02rom5.eng",  0x000000, 0x008000, CRC(8c28460b) SHA1(0aed170762f6044896a7e608df60bbd37c583a71) )

	ROM_REGION( 0x80000, "oki", 0 )			/* ADPCM Samples */
	ROM_LOAD( "ma02rom6.bin", 0x00000, 0x80000, CRC(199e7cae) SHA1(0f5e13cc8ec42c80bb4bbff90aba29cdb15213d4) )
ROM_END


ROM_START( shippumd )
	ROM_REGION( 0x100000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "ma02rom1.bin", 0x000000, 0x080000, CRC(a678b149) SHA1(8c1a631e023dbba0a3fa6cd1b7d10dec1663213a) )
	ROM_LOAD16_BYTE( "ma02rom0.bin", 0x000001, 0x080000, CRC(f226a212) SHA1(526acf3d05fdc88054a772fbea3de2af532bf3d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )			/* Sound Z80 code */
	ROM_LOAD( "ma02rom2.bin", 0x00000, 0x10000, CRC(dde8a57e) SHA1(f522a3f17e229c71512464349760a9e27778bf6a) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "ma02rom3.bin",  0x000000, 0x200000, CRC(0e797142) SHA1(a480ccd151e49b886d3175a6deff56e1f2c26c3e) )
	ROM_LOAD( "ma02rom4.bin",  0x200000, 0x200000, CRC(72a6fa53) SHA1(ce92e65205b84361cfb90305a61e9541b5c4dc2f) )

	ROM_REGION( 0x008000, "gfx2", 0 )
	ROM_LOAD( "ma02rom5.bin",  0x000000, 0x008000, CRC(116ae559) SHA1(4cc2d2a23cc0aefd457111b7990e47184e79204c) )

	ROM_REGION( 0x80000, "oki", 0 )			/* ADPCM Samples */
	ROM_LOAD( "ma02rom6.bin", 0x00000, 0x80000, CRC(199e7cae) SHA1(0f5e13cc8ec42c80bb4bbff90aba29cdb15213d4) )
ROM_END


ROM_START( bgaregga )
	ROM_REGION( 0x100000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "prg0.bin", 0x000000, 0x080000, CRC(f80c2fc2) SHA1(a9aac5c7f5439b6fe8d1b3db1fb02a27cc28fdf6) )
	ROM_LOAD16_BYTE( "prg1.bin", 0x000001, 0x080000, CRC(2ccfdd1e) SHA1(7a9f11f851854f3f8389b9c3c0906ebb8dc28712) )

	ROM_REGION( 0x40000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x08000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )
	ROM_CONTINUE(        0x10000, 0x18000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x010000, "gfx2", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x140000, "oki", 0 )		/* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x040000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggahk )
	ROM_REGION( 0x100000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "prg_0.rom", 0x000000, 0x080000, CRC(26e0019e) SHA1(5197001f5d59246b137e19ed1952a8207b25d4c0) )
	ROM_LOAD16_BYTE( "prg_1.rom", 0x000001, 0x080000, CRC(2ccfdd1e) SHA1(7a9f11f851854f3f8389b9c3c0906ebb8dc28712) )

	ROM_REGION( 0x40000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x08000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )
	ROM_CONTINUE(        0x10000, 0x18000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x010000, "gfx2", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x140000, "oki", 0 )		/* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x040000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggatw )
	ROM_REGION( 0x100000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "garegga_prg0.u123", 0x000000, 0x080000, CRC(235b7405) SHA1(a2434801df4231a6b48f6c63f47c202d25a89e79) )
	ROM_LOAD16_BYTE( "garegga_prg1.u65",  0x000001, 0x080000, CRC(c29ccf6a) SHA1(38806e0b4ff852f4bfefd80c56ca23f71623e275) )

	ROM_REGION( 0x40000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x08000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )
	ROM_CONTINUE(        0x10000, 0x18000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x010000, "gfx2", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x140000, "oki", 0 )		/* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x040000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgaregganv )
	ROM_REGION( 0x100000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "prg_0.bin", 0x000000, 0x080000, CRC(951ecc07) SHA1(a82e4b59e4a974566e59f3ab2fbae1aec7d88a2b) )
	ROM_LOAD16_BYTE( "prg_1.bin", 0x000001, 0x080000, CRC(729a60c6) SHA1(cb6f5d138bb82c32910f42d8ee16fa573a23cef3) )

	ROM_REGION( 0x40000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x08000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )
	ROM_CONTINUE(        0x10000, 0x18000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x010000, "gfx2", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x140000, "oki", 0 )		/* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x040000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggat2 )
	ROM_REGION( 0x100000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "prg0", 0x000000, 0x080000, CRC(84094099) SHA1(49fc68a8bcdae4477e20eade9dd569de88b0b798) )
	ROM_LOAD16_BYTE( "prg1", 0x000001, 0x080000, CRC(46f92fe4) SHA1(62a02cc1dbdc3ac362339aebb62368eb89b06bad) )

	ROM_REGION( 0x40000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x08000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )
	ROM_CONTINUE(        0x10000, 0x18000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x010000, "gfx2", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x140000, "oki", 0 )		/* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x040000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggacn )
	ROM_REGION( 0x100000, "maincpu", 0 )			/* Main 68K code */
	ROM_LOAD16_BYTE( "u123", 0x000000, 0x080000, CRC(88a4e66a) SHA1(ca97e564eed0c5e028b937312e55da56400d5c8c) )
	ROM_LOAD16_BYTE( "u65",  0x000001, 0x080000, CRC(5dea32a3) SHA1(59df6689e3eb5ea9e49a758604d21a64c65ca14d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x08000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )
	ROM_CONTINUE(        0x10000, 0x18000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x010000, "gfx2", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x140000, "oki", 0 )		/* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x040000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

/*
   The region of Batrider is controlled by the first byte of rom prg0.u22
   only sets which have been dumped from original PCBs are supported

   original ROM labels have no indication of the region.

   valid values are:
    ( * denotes that this set has been found on an original PCB )

   00 : Nippon *
   01 : USA *
   02 : Europe *
   03 : Asia
   04 : German
   05 : Austria
   06 : Belgium
   07 : Denmark
   08 : Finland
   09 : France
   0A : Great Britain
   0B : Greece
   0C : Holland
   0D : Italy
   0E : Norway
   0F : Portugal
   10 : Spain
   11 : Sweden
   12 : Switzerland
   13 : Australia
   14 : New Zealand
   15 : Taiwan
   16 : Hong Kong
   17 : Korea *
   18 : China *
   19 : No Region?
   1A+: Invalid

   For future reference, that would mean the following

   ROM_LOAD16_BYTE( "prg0_nippon.u22",       0x000000, 0x080000, CRC(4f3fc729) SHA1(b32d51c254741b82171a86c271679522a7aefd34) )
   ROM_LOAD16_BYTE( "prg0_usa.u22",          0x000000, 0x080000, CRC(2049d007) SHA1(f2a43547a6fc5083b03c1d59a85abbf6e1ce4cd9) )
   ROM_LOAD16_BYTE( "prg0_europe.u22",       0x000000, 0x080000, CRC(91d3e975) SHA1(682885fc17f2424d475c282f239f42faf1aae076) )
   ROM_LOAD16_BYTE( "prg0_asia.u22",         0x000000, 0x080000, CRC(fea5fe5b) SHA1(0008336ecd3886485ab1d9678880b1a0bc788f40) )
   ROM_LOAD16_BYTE( "prg0_german.u22",       0x000000, 0x080000, CRC(29969dd0) SHA1(eb8ad84b772508b6befb35afb11a0d6193c6a060) )
   ROM_LOAD16_BYTE( "prg0_austria.u22",      0x000000, 0x080000, CRC(46e08afe) SHA1(a6f46581d0f7285704fbf1ac57476c96f4dcbec2) )
   ROM_LOAD16_BYTE( "prg0_belgium.u22",      0x000000, 0x080000, CRC(f77ab38c) SHA1(8be87175250345d3e31d95ec204805071eae81f6) )
   ROM_LOAD16_BYTE( "prg0_denmark.u22",      0x000000, 0x080000, CRC(980ca4a2) SHA1(4f29eaa5ba6b94d96c527f80188657abc8f4dcd0) )
   ROM_LOAD16_BYTE( "prg0_finland.u22",      0x000000, 0x080000, CRC(826d72db) SHA1(be4bca0143f43c13361fd56974eb9b1ce7bd1740) )
   ROM_LOAD16_BYTE( "prg0_france.u22",       0x000000, 0x080000, CRC(ed1b65f5) SHA1(1e08957c0f7ed65695fb1ceb961ab765f8a97c89) )
   ROM_LOAD16_BYTE( "prg0_greatbritain.u22", 0x000000, 0x080000, CRC(5c815c87) SHA1(dea89944cd9a3fa6991b214495dc7123a505d39b) )
   ROM_LOAD16_BYTE( "prg0_greece.u22",       0x000000, 0x080000, CRC(33f74ba9) SHA1(fe770415584b037152b37a75fe468d3c52dcb3cd) )
   ROM_LOAD16_BYTE( "prg0_holland.u22",      0x000000, 0x080000, CRC(e4c42822) SHA1(8bfd286c42d7f2b3c88757b9a8b818be90b73f48) )
   ROM_LOAD16_BYTE( "prg0_italy.u22",        0x000000, 0x080000, CRC(8bb23f0c) SHA1(b448bba312a8d583a981f6633cbc14af99fdbb06) )
   ROM_LOAD16_BYTE( "prg0_norway.u22",       0x000000, 0x080000, CRC(3a28067e) SHA1(9435e6ce90b8d740a545469e6edb35d1af11ceab) )
   ROM_LOAD16_BYTE( "prg0_portugal.u22",     0x000000, 0x080000, CRC(555e1150) SHA1(5c9ae898244a23a4184f9613f42d9aa9530468b9) )
   ROM_LOAD16_BYTE( "prg0_spain.u22",        0x000000, 0x080000, CRC(0eebaa8c) SHA1(e305e90434e7f322a33e42a642362f770d3eb0e5) )
   ROM_LOAD16_BYTE( "prg0_sweden.u22",       0x000000, 0x080000, CRC(619dbda2) SHA1(9e88ba104a5cffcced3b93ca711487a82b0fddde) )
   ROM_LOAD16_BYTE( "prg0_switzerland.u22",  0x000000, 0x080000, CRC(d00784d0) SHA1(0b809414ce910684ca39216086f7d26fd2adeded) )
   ROM_LOAD16_BYTE( "prg0_australia.u22",    0x000000, 0x080000, CRC(bf7193fe) SHA1(9af50fffc6ef23e300bf7b5e90b0dee6e4f4ad05) )
   ROM_LOAD16_BYTE( "prg0_newzealand.u22",   0x000000, 0x080000, CRC(6842f075) SHA1(125b303c064d2f0b539ecadcb205756e7fd1334e) )
   ROM_LOAD16_BYTE( "prg0_taiwan.u22",       0x000000, 0x080000, CRC(0734e75b) SHA1(17a8fb4f8fda3c234ed976490193ba308cac08fe) )
   ROM_LOAD16_BYTE( "prg0_hongkong.u22",     0x000000, 0x080000, CRC(b6aede29) SHA1(580f29db6a2c2cea43966413778362694992a675) )
   ROM_LOAD16_BYTE( "prg0_korea.u22",        0x000000, 0x080000, CRC(d9d8c907) SHA1(69c197f2a41f288913f042de9eb8274c0df3ac27) )
   ROM_LOAD16_BYTE( "prg0_china.u22",        0x000000, 0x080000, CRC(c3b91f7e) SHA1(6b2376c37808dccda296d90ccd7f577ccff4e4dc) )
   ROM_LOAD16_BYTE( "prg0_none.u22",         0x000000, 0x080000, CRC(accf0850) SHA1(d93e4e80443a40c3a9575dbf21927ef0d1a039b9) )

 */


ROM_START( batrid )
	ROM_REGION( 0x200000, "maincpu", 0 )			/* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_europe.u22", 0x000000, 0x080000, CRC(91d3e975) SHA1(682885fc17f2424d475c282f239f42faf1aae076) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x48000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x08000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )
	ROM_CONTINUE(        0x10000, 0x38000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x140000, "oki1", 0 )		/* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x040000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x140000, "oki2", 0 )		/* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x040000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END

ROM_START( batridu )
	ROM_REGION( 0x200000, "maincpu", 0 )			/* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_usa.u22", 0x000000, 0x080000, CRC(2049d007) SHA1(f2a43547a6fc5083b03c1d59a85abbf6e1ce4cd9) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x48000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x08000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )
	ROM_CONTINUE(        0x10000, 0x38000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x140000, "oki1", 0 )		/* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x040000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x140000, "oki2", 0 )		/* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x040000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END

ROM_START( batridc )
	ROM_REGION( 0x200000, "maincpu", 0 )			/* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_china.u22", 0x000000, 0x080000, CRC(c3b91f7e) SHA1(6b2376c37808dccda296d90ccd7f577ccff4e4dc) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x48000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x08000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )
	ROM_CONTINUE(        0x10000, 0x38000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x140000, "oki1", 0 )		/* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x040000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x140000, "oki2", 0 )		/* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x040000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END

ROM_START( batridj )
	ROM_REGION( 0x200000, "maincpu", 0 )			/* Main 68k code */
	ROM_LOAD16_BYTE( "prg0b.u22", 0x000000, 0x080000, CRC(4f3fc729) SHA1(b32d51c254741b82171a86c271679522a7aefd34) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x48000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x08000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )
	ROM_CONTINUE(        0x10000, 0x38000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x140000, "oki1", 0 )		/* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x040000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x140000, "oki2", 0 )		/* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x040000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END

ROM_START( batridk )
	ROM_REGION( 0x200000, "maincpu", 0 )			/* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_korea.u22", 0x000000, 0x080000, CRC(d9d8c907) SHA1(69c197f2a41f288913f042de9eb8274c0df3ac27) )
	ROM_LOAD16_BYTE( "prg1.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21", 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24", 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x48000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x08000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )
	ROM_CONTINUE(        0x10000, 0x38000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x140000, "oki1", 0 )		/* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x040000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x140000, "oki2", 0 )		/* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x040000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END

/* Version A is older, might have been only released in Japan and Taiwan? */
ROM_START( batridja )
	ROM_REGION( 0x200000, "maincpu", 0 )			/* Main 68k code */
	ROM_LOAD16_BYTE( "prg0.bin", 0x000000, 0x080000, CRC(f93ea27c) SHA1(41023c2ee1efd70b5aa9c70e1ddd9e5c3d51d68a) )
	ROM_LOAD16_BYTE( "prg1.bin", 0x000001, 0x080000, CRC(8ae7f592) SHA1(8a20ebf85eca621f578d2302c3a3988647b077a7) )
	ROM_LOAD16_BYTE( "prg2.u21", 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24", 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x48000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x08000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )
	ROM_CONTINUE(        0x10000, 0x38000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x140000, "oki1", 0 )		/* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x040000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x140000, "oki2", 0 )		/* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x040000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END

ROM_START( batridta )
	ROM_REGION( 0x200000, "maincpu", 0 )			/* Main 68k code */
	ROM_LOAD16_BYTE( "u22.bin",  0x000000, 0x080000, CRC(b135820e) SHA1(c222887d18a0a3ea0fcc973b95b29d69c86f7ec3) )
	ROM_LOAD16_BYTE( "u23.bin",  0x000001, 0x080000, CRC(8ae7f592) SHA1(8a20ebf85eca621f578d2302c3a3988647b077a7) )
	ROM_LOAD16_BYTE( "prg2.u21", 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24", 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x48000, "audiocpu", 0 )			/* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x08000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )
	ROM_CONTINUE(        0x10000, 0x38000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x140000, "oki1", 0 )		/* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x040000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x140000, "oki2", 0 )		/* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x040000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END



/*
Battle Bakraid
Raizing/8ing, 1999

PCB Layout
----------

ET68-V99
|-----------------------------------------------------|
|TA8201  16.93MHz     ROM-6                   6264    |
|  YAC516                                             |
|       YMZ280B-F     ROM-7               SND_U0720   |
|                                                     |
| VOL                 ROM-8                 Z80       |
|                                                     |
|                   341256                            |
|                                               93C66 |
|                   341256               XILINX       |
|J                                       XC95108      |
|A                  27MHz     32MHz                   |
|M                                                    |
|M          DIPSW1                      341256  341256|
|A                  XILINX    XILINK                  |
|           DIPSW2  XC95144   XC95108   341256  341256|
|                                                     |
|           DIPSW3                                    |
|                                MACH211    PRG1_U023 |
| TEST_SW            68000                            |
|                                           PRG0_U022 |
|                                                     |
|                                           PRG3_U024 |
|                             L7A0498                 |
|                             GP9001        PRG2_U021 |
| ROM-0       ROM-1           (QFP208)                |
|                                                     |
|                               6264       MN414260   |
| ROM-2       ROM-3                                   |
|                               6264       MN414260   |
|-----------------------------------------------------|
Notes:
      ROM-0 to ROM-3 - 32M DIP42
      ROM-6 to ROM-8 - 32M DIP42 Byte Mode
      68000 clock - 16.000MHz (32/2)
      Z80 clock - 5.33333MHz (32/6)
      VSync - 60Hz
      HSync - 15.39kHz
*/



ROM_START( bkraidu )
	ROM_REGION( 0x200000, "maincpu", 0 )			/* Main 68k code */
	ROM_LOAD16_BYTE( "prg0u022_usa.bin", 0x000000, 0x080000, CRC(95fb2ffd) SHA1(c7f502f3945249573b66226e8bacc6a9bc230693) )
	ROM_LOAD16_BYTE( "prg1u023.new", 0x000001, 0x080000, CRC(4ae9aa64) SHA1(45fdf72141c4c9f24a38d4218c65874799b9c868) )
	ROM_LOAD16_BYTE( "prg2u021.bin", 0x100000, 0x080000, CRC(ffba8656) SHA1(6526bb65fad3384de3f301a7d1095cbf03757433) )
	ROM_LOAD16_BYTE( "prg3u024.bin", 0x100001, 0x080000, CRC(834b8ad6) SHA1(0dd6223bb0749819ad29811eeb04fd08d937abb0) )

	ROM_REGION( 0x40000, "audiocpu", 0 )			/* Sound Z80 code */
	ROM_LOAD( "sndu0720.bin", 0x00000, 0x08000, CRC(e62ab246) SHA1(00d23689dd423ecd4024c58b5903d16e890f1dff) )
	ROM_CONTINUE(             0x10000, 0x18000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "gfxu0510.bin", 0x000000, 0x400000, CRC(9cca3446) SHA1(1123f8b8bfbe59a2c572cdf61f1ad27ff37f0f0d) )
	ROM_LOAD( "gfxu0512.bin", 0x400000, 0x400000, CRC(a2a281d5) SHA1(d9a6623f9433ad682223f9780c26cd1523ebc5c5) )
	ROM_LOAD( "gfxu0511.bin", 0x800000, 0x400000, CRC(e16472c0) SHA1(6068d679a8b3b65e05acd58a7ce9ead90177049f) )
	ROM_LOAD( "gfxu0513.bin", 0xc00000, 0x400000, CRC(8bb635a0) SHA1(9064f1a2d8bb88ddbca702fb8556d0dfe6a5cadc) )

	ROM_REGION( 0x0c00000, "ymz", 0 )		/* YMZ280B Samples */
	ROM_LOAD( "rom6.829", 0x000000, 0x400000, CRC(8848b4a0) SHA1(e0dce136c5d5a4c1a92b863e57848cd5927d06f1) )
	ROM_LOAD( "rom7.830", 0x400000, 0x400000, CRC(d6224267) SHA1(5c9b7b13effbef9f707811f84bfe50ca85e605e3) )
	ROM_LOAD( "rom8.831", 0x800000, 0x400000, CRC(a101dfb0) SHA1(4b729b0d562e09df35438e9e6b457b8de2690a6e) )
ROM_END


ROM_START( bkraiduj )
	ROM_REGION( 0x200000, "maincpu", 0 )			/* Main 68k code */
	ROM_LOAD16_BYTE( "prg0u022.new", 0x000000, 0x080000, CRC(fa8d38d3) SHA1(aba91d87a8a62d3fe1139b4437b16e2f844264ad) )
	ROM_LOAD16_BYTE( "prg1u023.new", 0x000001, 0x080000, CRC(4ae9aa64) SHA1(45fdf72141c4c9f24a38d4218c65874799b9c868) )
	ROM_LOAD16_BYTE( "prg2u021.bin", 0x100000, 0x080000, CRC(ffba8656) SHA1(6526bb65fad3384de3f301a7d1095cbf03757433) )
	ROM_LOAD16_BYTE( "prg3u024.bin", 0x100001, 0x080000, CRC(834b8ad6) SHA1(0dd6223bb0749819ad29811eeb04fd08d937abb0) )

	ROM_REGION( 0x40000, "audiocpu", 0 )			/* Sound Z80 code */
	ROM_LOAD( "sndu0720.bin", 0x00000, 0x08000, CRC(e62ab246) SHA1(00d23689dd423ecd4024c58b5903d16e890f1dff) )
	ROM_CONTINUE(             0x10000, 0x18000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "gfxu0510.bin", 0x000000, 0x400000, CRC(9cca3446) SHA1(1123f8b8bfbe59a2c572cdf61f1ad27ff37f0f0d) )
	ROM_LOAD( "gfxu0512.bin", 0x400000, 0x400000, CRC(a2a281d5) SHA1(d9a6623f9433ad682223f9780c26cd1523ebc5c5) )
	ROM_LOAD( "gfxu0511.bin", 0x800000, 0x400000, CRC(e16472c0) SHA1(6068d679a8b3b65e05acd58a7ce9ead90177049f) )
	ROM_LOAD( "gfxu0513.bin", 0xc00000, 0x400000, CRC(8bb635a0) SHA1(9064f1a2d8bb88ddbca702fb8556d0dfe6a5cadc) )

	ROM_REGION( 0x0c00000, "ymz", 0 )		/* YMZ280B Samples */
	ROM_LOAD( "rom6.829", 0x000000, 0x400000, CRC(8848b4a0) SHA1(e0dce136c5d5a4c1a92b863e57848cd5927d06f1) )
	ROM_LOAD( "rom7.830", 0x400000, 0x400000, CRC(d6224267) SHA1(5c9b7b13effbef9f707811f84bfe50ca85e605e3) )
	ROM_LOAD( "rom8.831", 0x800000, 0x400000, CRC(a101dfb0) SHA1(4b729b0d562e09df35438e9e6b457b8de2690a6e) )
ROM_END


ROM_START( bkraidj )
	ROM_REGION( 0x200000, "maincpu", 0 )			/* Main 68k code */
	ROM_LOAD16_BYTE( "prg0u022.bin", 0x000000, 0x080000, CRC(0dd59512) SHA1(c6a4e6aa49c6ac3b04ae62a0a4cc8084ae048381) )
	ROM_LOAD16_BYTE( "prg1u023.bin", 0x000001, 0x080000, CRC(fecde223) SHA1(eb5ac0eda49b4b0f3d25d8a8bb356e77a453d3a7) )
	ROM_LOAD16_BYTE( "prg2u021.bin", 0x100000, 0x080000, CRC(ffba8656) SHA1(6526bb65fad3384de3f301a7d1095cbf03757433) )
	ROM_LOAD16_BYTE( "prg3u024.bin", 0x100001, 0x080000, CRC(834b8ad6) SHA1(0dd6223bb0749819ad29811eeb04fd08d937abb0) )

	ROM_REGION( 0x40000, "audiocpu", 0 )			/* Sound Z80 code */
	ROM_LOAD( "sndu0720.bin", 0x00000, 0x08000, CRC(e62ab246) SHA1(00d23689dd423ecd4024c58b5903d16e890f1dff) )
	ROM_CONTINUE(             0x10000, 0x18000 )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "gfxu0510.bin", 0x000000, 0x400000, CRC(9cca3446) SHA1(1123f8b8bfbe59a2c572cdf61f1ad27ff37f0f0d) )
	ROM_LOAD( "gfxu0512.bin", 0x400000, 0x400000, CRC(a2a281d5) SHA1(d9a6623f9433ad682223f9780c26cd1523ebc5c5) )
	ROM_LOAD( "gfxu0511.bin", 0x800000, 0x400000, CRC(e16472c0) SHA1(6068d679a8b3b65e05acd58a7ce9ead90177049f) )
	ROM_LOAD( "gfxu0513.bin", 0xc00000, 0x400000, CRC(8bb635a0) SHA1(9064f1a2d8bb88ddbca702fb8556d0dfe6a5cadc) )

	ROM_REGION( 0x0c00000, "ymz", 0 )		/* YMZ280B Samples */
	ROM_LOAD( "rom6.829", 0x000000, 0x400000, CRC(8848b4a0) SHA1(e0dce136c5d5a4c1a92b863e57848cd5927d06f1) )
	ROM_LOAD( "rom7.830", 0x400000, 0x400000, CRC(d6224267) SHA1(5c9b7b13effbef9f707811f84bfe50ca85e605e3) )
	ROM_LOAD( "rom8.831", 0x800000, 0x400000, CRC(a101dfb0) SHA1(4b729b0d562e09df35438e9e6b457b8de2690a6e) )
ROM_END


/* The following is in order of Toaplan Board/game numbers */
/* See list at top of file */

/*  ( YEAR  NAME      PARENT    MACHINE   INPUT     INIT      MONITOR COMPANY    FULLNAME     FLAGS ) */
GAME( 1991, tekipaki, 0,        tekipaki, tekipaki, T2_Z180,  ROT0,   "Toaplan", "Teki Paki", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )

GAME( 1991, ghox,     0,        ghox,     ghox,     T2_Z180,  ROT270, "Toaplan", "Ghox (Spinner with Up/Down Axis)", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1991, ghoxj,    ghox,     ghox,     ghox,     T2_Z180,  ROT270, "Toaplan", "Ghox (8-Way Joystick)", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )

GAME( 1992, dogyuun,  0,        dogyuun,  dogyuun,  T2_V25,   ROT270, "Toaplan", "Dogyuun", GAME_SUPPORTS_SAVE )
GAME( 1992, dogyuunk, dogyuun,  dogyuun,  dogyuunk, T2_V25,   ROT270, "Toaplan", "Dogyuun (Unite Trading license)", GAME_SUPPORTS_SAVE )
GAME( 1992, dogyuunt, dogyuun,  dogyuun,  dogyuunt, T2_V25,   ROT270, "Toaplan", "Dogyuun (test location version)", GAME_SUPPORTS_SAVE )

GAME( 1993, kbash,    0,        kbash,    kbash,    T2_V25,   ROT0,   "Toaplan", "Knuckle Bash", GAME_SUPPORTS_SAVE )

GAME( 1999, kbash2,   0,        kbash2,   kbash2,   T2_noZ80, ROT0,   "bootleg", "Knuckle Bash 2 (bootleg)", GAME_SUPPORTS_SAVE )

GAME( 1992, truxton2, 0,        truxton2, truxton2, T2_noZ80, ROT270, "Toaplan", "Truxton II / Tatsujin Oh", GAME_SUPPORTS_SAVE )

GAME( 1991, pipibibs,  0,        pipibibs, pipibibs, T2_Z80,   ROT0,   "Toaplan", "Pipi & Bibis / Whoopee!! (Z80 sound cpu, set 1)", GAME_SUPPORTS_SAVE )
GAME( 1991, pipibibsa, pipibibs, pipibibs, pipibibs, T2_Z80,   ROT0,   "Toaplan", "Pipi & Bibis / Whoopee!! (Z80 sound cpu, set 2)", GAME_SUPPORTS_SAVE )
GAME( 1991, whoopee,   pipibibs, whoopee,  whoopee,  T2_Z180,  ROT0,   "Toaplan", "Pipi & Bibis / Whoopee!! (Whoopee!! board)", GAME_NO_SOUND | GAME_SUPPORTS_SAVE ) // original Whoopee!! boards have a HD647180 instead of Z80
GAME( 1991, pipibibsbl,pipibibs, pipibibi_bootleg, pipibibsbl, pipibibsbl, ROT0,   "bootleg (Ryouta Kikaku)", "Pipi & Bibis / Whoopee!! (bootleg)", GAME_SUPPORTS_SAVE )

// region is in eeprom (and also requires correct return value from a v25 mapped address??)
// todo: something could be wrong here, because the _dumped_ eeprom doesn't work..
GAME( 1992, fixeight,   0,        fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Europe)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeightt,  fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Europe, Taito license)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeightkt, fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Korea, Taito license)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeightk,  fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Korea)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeightht, fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Hong Kong, Taito license)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeighth,  fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Hong Kong)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeighttwt,fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Taiwan, Taito license)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeighttw, fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Taiwan)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeightat, fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Southeast Asia, Taito license)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeighta,  fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Southeast Asia)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeightu,  fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (USA)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeightut, fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (USA, Taito license)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeightj,  fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Japan)",  GAME_SUPPORTS_SAVE )
GAME( 1992, fixeightjt, fixeight, fixeight, fixeight, fixeight, ROT270, "Toaplan", "FixEight (Japan, Taito license)",  GAME_SUPPORTS_SAVE )

GAME( 1992, fixeightbl, fixeight, fixeighb, fixeightbl, fixeighb, ROT270, "bootleg", "FixEight (Korea, bootleg)", GAME_SUPPORTS_SAVE )

GAME( 1992, grindstm, vfive,    vfive,    grindstm, vfive,    ROT270, "Toaplan", "Grind Stormer", GAME_SUPPORTS_SAVE )
GAME( 1992, grindstma,vfive,    vfive,    grindstm, vfive,    ROT270, "Toaplan", "Grind Stormer (older set)", GAME_SUPPORTS_SAVE )
GAME( 1993, vfive,    0,        vfive,    vfive,    vfive,    ROT270, "Toaplan", "V-Five (Japan)", GAME_SUPPORTS_SAVE )

GAME( 1993, batsugun,  0,        batsugun, batsugun, T2_V25,   ROT270, "Toaplan", "Batsugun", GAME_SUPPORTS_SAVE )
GAME( 1993, batsuguna, batsugun, batsugun, batsugun, T2_V25,   ROT270, "Toaplan", "Batsugun (older set)", GAME_SUPPORTS_SAVE )
GAME( 1993, batsugunsp,batsugun, batsugun, batsugun, T2_V25,   ROT270, "Toaplan", "Batsugun (Special Ver.)", GAME_SUPPORTS_SAVE )

GAME( 1994, snowbro2, 0,        snowbro2, snowbro2, T2_noZ80, ROT0,   "Hanafram", "Snow Bros. 2 - With New Elves / Otenki Paradise", GAME_SUPPORTS_SAVE )

GAME( 1993, mahoudai, 0,        mahoudai, mahoudai, T2_Z80,   ROT270, "Raizing (Able license)", "Mahou Daisakusen (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1993, sstriker, mahoudai, mahoudai, sstriker, T2_Z80,   ROT270, "Raizing", "Sorcer Striker (World)" , GAME_SUPPORTS_SAVE ) // from korean board
GAME( 1993, sstrikera,mahoudai, mahoudai, sstriker, T2_Z80,   ROT270, "Raizing", "Sorcer Striker (World, alt)" , GAME_SUPPORTS_SAVE ) // verified on two different PCBs

GAME( 1994, shippumd, 0,        shippumd, shippumd, T2_Z80,   ROT270, "Raizing / Eighting", "Shippu Mahou Daisakusen (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1994, kingdmgp, shippumd, shippumd, kingdmgp, T2_Z80,   ROT270, "Raizing / Eighting", "Kingdom Grandprix (World)" , GAME_SUPPORTS_SAVE ) // from korean board, missing letters on credits screen but this is correct

GAME( 1996, bgaregga,   0,        bgaregga, bgaregga, T2_Z80,   ROT270, "Raizing / Eighting", "Battle Garegga (Europe / USA / Japan / Asia) (Sat Feb 3 1996)", GAME_SUPPORTS_SAVE )
GAME( 1996, bgareggahk, bgaregga, bgaregga, bgareghk, T2_Z80,   ROT270, "Raizing / Eighting", "Battle Garegga (Austria / Hong Kong) (Sat Feb 3 1996)", GAME_SUPPORTS_SAVE )
GAME( 1996, bgareggatw, bgaregga, bgaregga, bgaregtw, T2_Z80,   ROT270, "Raizing / Eighting", "Battle Garegga (Taiwan / Germany) (Thu Feb 1 1996)", GAME_SUPPORTS_SAVE )
GAME( 1996, bgaregganv, bgaregga, bgaregga, bgareghk, T2_Z80,   ROT270, "Raizing / Eighting", "Battle Garegga - New Version (Austria / Hong Kong) (Sat Mar 2 1996)" , GAME_SUPPORTS_SAVE ) // displays New Version only when set to HK
GAME( 1996, bgareggat2, bgaregga, bgaregga, bgaregga, T2_Z80,   ROT270, "Raizing / Eighting", "Battle Garegga - Type 2 (Europe / USA / Japan / Asia) (Sat Mar 2 1996)" , GAME_SUPPORTS_SAVE ) // displays Type 2 only when set to Europe
GAME( 1996, bgareggacn, bgaregga, bgaregga, bgaregcn, T2_Z80,   ROT270, "Raizing / Eighting", "Battle Garegga - Type 2 (Denmark / China) (Tue Apr 2 1996)", GAME_SUPPORTS_SAVE ) // displays Type 2 only when set to Denmark

// these are all based on Version B, even if only the Japan version states 'version B'
GAME( 1998, batrid,   0,        batrider, batrider, batrider, ROT270, "Raizing / Eighting", "Armed Police Batrider (Europe) (Fri Feb 13 1998)", GAME_SUPPORTS_SAVE )
GAME( 1998, batridu,  batrid,   batrider, batrider, batrider, ROT270, "Raizing / Eighting", "Armed Police Batrider (U.S.A.) (Fri Feb 13 1998)", GAME_SUPPORTS_SAVE )
GAME( 1998, batridc,  batrid,   batrider, batrider, batrider, ROT270, "Raizing / Eighting", "Armed Police Batrider (China) (Fri Feb 13 1998)", GAME_SUPPORTS_SAVE )
GAME( 1998, batridj,  batrid,   batrider, batridrj, batrider, ROT270, "Raizing / Eighting", "Armed Police Batrider - B Version (Japan) (Fri Feb 13 1998)", GAME_SUPPORTS_SAVE )
GAME( 1998, batridk,  batrid,   batrider, batrider, batrider, ROT270, "Raizing / Eighting", "Armed Police Batrider (Korea) (Fri Feb 13 1998)", GAME_SUPPORTS_SAVE )
// version A is older code
GAME( 1998, batridja, batrid,   batrider, batridrj, batrider, ROT270, "Raizing / Eighting", "Armed Police Batrider - A Version (Japan) (Mon Dec 22 1997)", GAME_SUPPORTS_SAVE )
GAME( 1998, batridta, batrid,   batrider, batridrj, batrider, ROT270, "Raizing / Eighting", "Armed Police Batrider - A Version (Taiwan) (Mon Dec 22 1997)", GAME_SUPPORTS_SAVE )

// Battle Bakraid
// the 'unlimited' version is a newer revision of the code.
GAME( 1999, bkraidu,  0,        bbakradu, bbakraid, bbakradu, ROT270, "Eighting", "Battle Bakraid - Unlimited Version (U.S.A.) (Tue Jun 8 1999)", GAME_SUPPORTS_SAVE )
GAME( 1999, bkraiduj, bkraidu,  bbakradu, bbakraid, bbakradu, ROT270, "Eighting", "Battle Bakraid - Unlimited Version (Japan) (Tue Jun 8 1999)", GAME_SUPPORTS_SAVE )
// older revision of the code
GAME( 1999, bkraidj,  bkraidu,  bbakraid, bbakraid, bbakraid, ROT270, "Eighting", "Battle Bakraid (Japan) (Wed Apr 7 1999)", GAME_SUPPORTS_SAVE )
