/***************************************************************************

    Victory video system

***************************************************************************/

#include "emu.h"
#include "includes/victory.h"



/* number of ticks per clock of the microcode state machine   */
/* from what I can tell, this should be divided by 32, not 8  */
/* but the interrupt test does some precise timing, and fails */
/* if it's not 8 */
#define VICTORY_MICRO_STATE_CLOCK	(XTAL_11_289MHz)
#define MICRO_STATE_CLOCK_PERIOD	attotime::from_hz(VICTORY_MICRO_STATE_CLOCK / 8)


/* debugging constants */
#define LOG_MICROCODE		0
#define LOG_COLLISION		0


/* function prototypes */
static int command2(running_machine &machine);
static int command3(running_machine &machine);
static int command4(running_machine &machine);
static int command5(running_machine &machine);
static int command6(running_machine &machine);
static int command7(running_machine &machine);



/*************************************
 *
 *  Initialize the video system
 *
 *************************************/

VIDEO_START( victory )
{
	victory_state *state = machine.driver_data<victory_state>();
	/* allocate bitmapram */
	state->m_rram = auto_alloc_array(machine, UINT8, 0x4000);
	state->m_gram = auto_alloc_array(machine, UINT8, 0x4000);
	state->m_bram = auto_alloc_array(machine, UINT8, 0x4000);

	/* allocate bitmaps */
	state->m_bgbitmap = auto_alloc_array(machine, UINT8, 256 * 256);
	state->m_fgbitmap = auto_alloc_array(machine, UINT8, 256 * 256);

	/* reset globals */
	state->m_vblank_irq = 0;
	state->m_fgcoll = state->m_fgcollx = state->m_fgcolly = 0;
	state->m_bgcoll = state->m_bgcollx = state->m_bgcolly = 0;
	state->m_scrollx = state->m_scrolly = 0;
	state->m_video_control = 0;
	memset(&state->m_micro, 0, sizeof(state->m_micro));
	state->m_micro.timer = machine.scheduler().timer_alloc(FUNC(NULL));

	/* register for state saving */
	state_save_register_global_array(machine, state->m_paletteram);
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static void victory_update_irq(running_machine &machine)
{
	victory_state *state = machine.driver_data<victory_state>();
	if (state->m_vblank_irq || state->m_fgcoll || (state->m_bgcoll && (state->m_video_control & 0x20)))
		cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
	else
		cputag_set_input_line(machine, "maincpu", 0, CLEAR_LINE);
}


INTERRUPT_GEN( victory_vblank_interrupt )
{
	victory_state *state = device->machine().driver_data<victory_state>();
	state->m_vblank_irq = 1;

	victory_update_irq(device->machine());
}



/*************************************
 *
 *  Palette handling
 *
 *************************************/

WRITE8_HANDLER( victory_paletteram_w )
{
	victory_state *state = space->machine().driver_data<victory_state>();
	state->m_paletteram[offset & 0x3f] = ((offset & 0x80) << 1) | data;
}


static void set_palette(running_machine &machine)
{
	victory_state *state = machine.driver_data<victory_state>();
	offs_t offs;

	for (offs = 0; offs < 0x40; offs++)
	{
		UINT16 data = state->m_paletteram[offs];

		palette_set_color_rgb(machine, offs, pal3bit(data >> 6), pal3bit(data >> 0), pal3bit(data >> 3));
	}
}



/*************************************
 *
 *  Video control read
 *
 *************************************/

READ8_HANDLER( victory_video_control_r )
{
	victory_state *state = space->machine().driver_data<victory_state>();
	int result = 0;

	switch (offset)
	{
		case 0x00:	/* 5XFIQ */
			result = state->m_fgcollx;
			if (LOG_COLLISION) logerror("%04X:5XFIQ read = %02X\n", cpu_get_previouspc(&space->device()), result);
			return result;

		case 0x01:	/* 5CLFIQ */
			result = state->m_fgcolly;
			if (state->m_fgcoll)
			{
				state->m_fgcoll = 0;
				victory_update_irq(space->machine());
			}
			if (LOG_COLLISION) logerror("%04X:5CLFIQ read = %02X\n", cpu_get_previouspc(&space->device()), result);
			return result;

		case 0x02:	/* 5BACKX */
			result = state->m_bgcollx & 0xfc;
			if (LOG_COLLISION) logerror("%04X:5BACKX read = %02X\n", cpu_get_previouspc(&space->device()), result);
			return result;

		case 0x03:	/* 5BACKY */
			result = state->m_bgcolly;
			if (state->m_bgcoll)
			{
				state->m_bgcoll = 0;
				victory_update_irq(space->machine());
			}
			if (LOG_COLLISION) logerror("%04X:5BACKY read = %02X\n", cpu_get_previouspc(&space->device()), result);
			return result;

		case 0x04:	/* 5STAT */
			// D7 = BUSY (9A1) -- microcode
			// D6 = 5FCIRQ (3B1)
			// D5 = 5VIRQ
			// D4 = 5BCIRQ (3B1)
			// D3 = SL256
			if (state->m_micro.timer_active && state->m_micro.timer->elapsed() < state->m_micro.endtime)
				result |= 0x80;
			result |= (~state->m_fgcoll & 1) << 6;
			result |= (~state->m_vblank_irq & 1) << 5;
			result |= (~state->m_bgcoll & 1) << 4;
			result |= (space->machine().primary_screen->vpos() & 0x100) >> 5;
			if (LOG_COLLISION) logerror("%04X:5STAT read = %02X\n", cpu_get_previouspc(&space->device()), result);
			return result;

		default:
			logerror("%04X:victory_video_control_r(%02X)\n", cpu_get_previouspc(&space->device()), offset);
			break;
	}
	return 0;
}



/*************************************
 *
 *  Video control write
 *
 *************************************/

WRITE8_HANDLER( victory_video_control_w )
{
	victory_state *state = space->machine().driver_data<victory_state>();
	struct micro_t &micro = state->m_micro;
	switch (offset)
	{
		case 0x00:	/* LOAD IL */
			if (LOG_MICROCODE) logerror("%04X:IL=%02X\n", cpu_get_previouspc(&space->device()), data);
			micro.i = (micro.i & 0xff00) | (data & 0x00ff);
			break;

		case 0x01:	/* LOAD IH */
			if (LOG_MICROCODE) logerror("%04X:IH=%02X\n", cpu_get_previouspc(&space->device()), data);
			micro.i = (micro.i & 0x00ff) | ((data << 8) & 0xff00);
			if (micro.cmdlo == 5)
			{
				if (LOG_MICROCODE) logerror("  Command 5 triggered by write to IH\n");
				command5(space->machine());
			}
			break;

		case 0x02:	/* LOAD CMD */
			if (LOG_MICROCODE) logerror("%04X:CMD=%02X\n", cpu_get_previouspc(&space->device()), data);
			micro.cmd = data;
			micro.cmdlo = data & 7;
			if (micro.cmdlo == 0)
				logerror("  Command 0 triggered\n");
			else if (micro.cmdlo == 1)
				logerror("  Command 1 triggered\n");
			else if (micro.cmdlo == 6)
			{
				if (LOG_MICROCODE) logerror("  Command 6 triggered\n");
				command6(space->machine());
			}
			break;

		case 0x03:	/* LOAD G */
			if (LOG_MICROCODE) logerror("%04X:G=%02X\n", cpu_get_previouspc(&space->device()), data);
			micro.g = data;
			break;

		case 0x04:	/* LOAD X */
			if (LOG_MICROCODE) logerror("%04X:X=%02X\n", cpu_get_previouspc(&space->device()), data);
			micro.xp = data;
			if (micro.cmdlo == 3)
			{
				if (LOG_MICROCODE) logerror(" Command 3 triggered by write to X\n");
				command3(space->machine());
			}
			break;

		case 0x05:	/* LOAD Y */
			if (LOG_MICROCODE) logerror("%04X:Y=%02X\n", cpu_get_previouspc(&space->device()), data);
			micro.yp = data;
			if (micro.cmdlo == 4)
			{
				if (LOG_MICROCODE) logerror("  Command 4 triggered by write to Y\n");
				command4(space->machine());
			}
			break;

		case 0x06:	/* LOAD R */
			if (LOG_MICROCODE) logerror("%04X:R=%02X\n", cpu_get_previouspc(&space->device()), data);
			micro.r = data;
			break;

		case 0x07:	/* LOAD B */
			if (LOG_MICROCODE) logerror("%04X:B=%02X\n", cpu_get_previouspc(&space->device()), data);
			micro.b = data;
			if (micro.cmdlo == 2)
			{
				if (LOG_MICROCODE) logerror("  Command 2 triggered by write to B\n");
				command2(space->machine());
			}
			else if (micro.cmdlo == 7)
			{
				if (LOG_MICROCODE) logerror("  Command 7 triggered by write to B\n");
				command7(space->machine());
			}
			break;

		case 0x08:	/* SCROLLX */
			if (LOG_MICROCODE) logerror("%04X:SCROLLX write = %02X\n", cpu_get_previouspc(&space->device()), data);
			state->m_scrollx = data;
			break;

		case 0x09:	/* SCROLLY */
			if (LOG_MICROCODE) logerror("%04X:SCROLLY write = %02X\n", cpu_get_previouspc(&space->device()), data);
			state->m_scrolly = data;
			break;

		case 0x0a:	/* CONTROL */
			// D7 = HLMBK
			// D6 = VLMBK
			// D5 = BIRQEA
			// D4 = SEL5060
			// D3 = SINVERT
			// D2 = BIR12
			// D1 = SELOVER
			if (LOG_MICROCODE) logerror("%04X:CONTROL write = %02X\n", cpu_get_previouspc(&space->device()), data);
			state->m_video_control = data;
			break;

		case 0x0b:	/* CLRVIRQ */
			if (LOG_MICROCODE) logerror("%04X:CLRVIRQ write = %02X\n", cpu_get_previouspc(&space->device()), data);
			state->m_vblank_irq = 0;
			victory_update_irq(space->machine());
			break;

		default:
			if (LOG_MICROCODE) logerror("%04X:victory_video_control_w(%02X) = %02X\n", cpu_get_previouspc(&space->device()), offset, data);
			break;
	}
}


/***************************************************************************************************

    Victory Microcode
    -----------------

    The cool thing about this hardware is the use of microcode, which is like having a little
    graphics coprocessor around to do the hard stuff. The operations that can be performed by
    this bit of circuitry include pixel plotting, line drawing, sprite drawing, and data
    transfer, all with optional collision detection. In addition, data can be uploaded into
    the $2000-$21FF address range and then "executed" as mini subroutines.

    Commands to the microcode are written to the command register at $C102, followed by
    whatever parameters are needed. Parameters are stored in registers. There are a number
    of registers, accessed at these addresses:

        C100-C101:  I (16 bits)
        C102:       CMD (8 bits)
        C103:       G (8 bits)
        C104:       X' (8 bits)
        C105:       Y' (8 bits)
        C106:       R (8 bits)
        C107:       B (8 bits)

    Writing the last parameter triggers the command. There are a total of 6 commands supported:

        command 2: copy data
            when register B is written, take the bytes from R, G and B and transfer them
            into video RAM at address I

        command 3: draw sprite
            when register X is written, draw a sprite at location (X,Y) using the data from
            video RAM address I; the width is given by (R >> 5) * 8, and then height is
            given by (R & 31) * 2; data is XORed with the current VRAM contents

        command 4: execute program
            when register Y is written, copy Y * 2 to the PC and begin executing the commands
            at ($2000 + PC); each command loads 6 bytes from VRAM into registers CMD,X,Y,I and R;
            the program stops executing after it receives a command with the high bit off

        command 5: draw vector
            when register IH is written, draw a vector of length IL starting at location (X,Y);
            IH serves as the bresenhem increment for the minor axis; bits 4-6 of the command
            select which octant to draw into; each VRAM write XORs the data from R,G and B
            with the current VRAM contents

        command 6: copy data
            when the command is written, copy (R & 31) * 2 bytes of data from video RAM location
            I to video RAM location ($2000 + PC)

        command 7: plot pixel
            when register B is written, take the bytes from R, G and B and XOR them with the
            video RAM contents at (X,Y)

    The command register is actually broken down into bitfields as follows:

        D7    -> must be high for a program to continue execution; otherwise, it will stop
        D4-D6 -> for non-vector commands, enables VRAM writes to the red, blue and green planes
        D3    -> enable collision detection for commands 3,5,7
        D0-D2 -> command

    The microcode is actually a big state machine, driven by the 4 PROMs at 19B,19C,19D and 19E.
    Below are some of the gory details of the state machine.

***************************************************************************************************

    19E:
        D7 -> inverter -> ZERO RAM [11C8, 13D8]
        D6 -> select on the mux at 18F
        D5 -> BUSY [4B6]
        D4 -> D on flip flop at 16E
        D3 -> D3 of alternate selection from mux at 18F
        D2 -> D2 of alternate selection from mux at 18F
        D1 -> D1 of alternate selection from mux at 18F
        D0 -> D0 of alternate selection from mux at 18F

    19B:
        D7 -> S LOAD LH [11B8]
        D6 -> INC I (AND with WRITE EA) [8A8]
        D5 -> S INC Y (AND with WRITE EA) [8C8]
        D4 -> SXFERY (AND with WRITE EA) [8C8]
        D3 -> D on flip flop at 15E, output goes to SADDX [8C8]
        D2 -> S LOAD PC [8B8]
        D1 -> CPU0 [11C8, 13C7]
        D0 -> INC X (AND with WRITE EA) [8C8]

    19C:
        D7 -> SXFERX/INC X (AND with WRITE EA) [8C8, 11B8, 12C8]
        D6 -> see D5
        D5 -> selects one of 4 with D6:
                0 -> SEA VDATA
                1 -> SEA BUFF
                2 -> SEA SR 1
                3 -> SEA SR 2
        D4 -> ADD 128 [11C8, 12C8]
              also: S ACC CLEAR (AND with WRITE EA) [10B8]
        D3 -> S ACC CLK (AND with S SEQ CLK) [10B8]
        D2 -> INC PC [8B8]
        D1 -> INC L [11B8]
        D0 -> INC H [11B8]

    19D:
        D7 -> S W VRAM (AND with WRITE EA) [14A8]
        D6 -> S WRITE BUSS1 (AND with WRITE EA) [7A8]
        D5 -> S WRITE BUSS2 (AND with WRITE EA) [7A8]
        D4 -> D2 of alternate selection from mux at 18E
        D3 -> D1 of alternate selection from mux at 18E
        D2 -> D0 of alternate selection from mux at 18E
        D1 -> ASEL1 (AND with WRITE EA) [8D8]
        D0 -> ASEL0 (AND with WRITE EA) [8D8]


    Always on in non-zero states: BUSY, CPU0

    State   Next                ASEL SEA Interesting bits
    -----   ----                ---- --- --------------------------------------------
      00    /SETRDY ? 00 : 01     3   0  None
      01    CM0-2                 0   0
      02    00                    0   0  ZERORAM, INCI, SWVRAM
      03    1C                    2   0  SLOADLH, SXFERY
      04    1A                    2   0  SLOADPC
      05    0A                    1   0  SXFERY, ADD128+SACCCLEAR, SACCCLK
      06    0C                    0   0  SLOADLH, SLOADPC
      07    08                    1   0  SXFERY, SXFERX+INCX
      08    09                    1   2  INCX, SWVRAM
      09    00                    1   3  SWVRAM
      0A    VFIN ? 19 : 0B        1   0  SXFERX+INCX
      0B    0A                    1   2  INCI, SACCCLK, SWVRAM
      0C    0D                    0   1  INCI, SXFERX+INCX, INCL
      0D    /LTC ? 0C : 0E        2   2  ZERORAM, INCPC, SWVRAM
      0E    19                    2   2

      19    /CM7 ? 00 : 1A        2   0
      1A    1B                    2   0  INCPC, SWRITEBUSS1
      1B    01                    2   0  INCPC, SWRITEBUSS2
      1C    HTC ? 19 : 1D         0   1
      1D    1E                    1   2  INCX, SXFERX+INCX, INCL, SWVRAM
      1E    /LTC ? 1C : 1F        1   3  INCI, SINCY, SWVRAM
      1F    1C                    1   0  ZERORAM, SXFERY, SADDX, INCH

Registers:

    X' = 8-bit value = 2 x 4-bit counters at 11B/13B
            SADDX  -> enables clock to count
            LF/RT  -> controls direction of counting
            SLDX   -> loads data from RED VRAM or D0-D7 into X'
            OUT    -> to X

    X  = 8-bit value = 2 x 4-bit counters at 12D/13D
            SINCX  -> enables clock to count
            SXFERX -> loads data from X' into X, with an XOR of 7
            OUT    -> to X1-X128

    Y' = 8-bit value = 8-bit latch
            SLDY   -> loads data from BLUE VRAM or D0-D7 into Y'
            OUT    -> to Y

    Y  = 8-bit value = 2 x 4-bit counters at 10B/8B
            SINCY  -> enables clock to count
            SXFERY -> loads data from Y' into Y
            OUT    -> to Y1-Y128

    I  = 16-bit value = 4 x 4-bit counters at 12C/11C/12B/14B
            INCI   -> enables clock to count
            SLDIH  -> loads data from BLUE VRAM or D0-D7 into upper 8 bits of I
            SLDIL  -> loads data from RED VRAM or D0-D7 into lower 8 bits of I
            OUT    -> to I1-I32000

    PC = 9-bit value = 2 x 4-bit counters at 9B/7B plus JK flip-flop at 12E
            INCPC  -> toggles flip-flop and increments
            SLOADPC-> loads data from Y' into PC

    L  = 5-bit value = 2 x 4-bit counters at 3H/4H
            INCL   -> enables clock to count
            SLOADLH-> loads data from SEA

    H  = 3-bit value = 1 x 4-bit counter at 5H
            INCH   -> enables clock to count
            SLOADLH-> loads data from SEA

    14-bit VRAM address comes from one of several sources, depending on ASEL
        ASEL0 -> I & 0x3fff
        ASEL1 -> ((Y & 0xff) << 5) | ((X & 0xff) >> 3)
        ASEL2 -> 0x2000 | (PC & 0x1ff)
        ASEL3 -> ((L & 0xff) << 5) | ((E & 0xff) >> 3)  [video refresh]

***************************************************************************************************/


/*************************************
 *
 *  Microcode timing
 *
 *************************************/

INLINE void count_states(struct micro_t &micro, int states)
{
	attotime state_time = MICRO_STATE_CLOCK_PERIOD * states;

	if (!micro.timer)
	{
		micro.timer->adjust(attotime::never);
		micro.timer_active = 1;
		micro.endtime = state_time;
	}
	else if (micro.timer->elapsed() > micro.endtime)
	{
		micro.timer->adjust(attotime::never);
		micro.timer_active = 1;
		micro.endtime = state_time;
	}
	else
		micro.endtime += state_time;
}


/*************************************
 *
 *  Microcode command 2:
 *      Load data from R/G/B
 *
 *************************************/

static int command2(running_machine &machine)
{
	victory_state *state = machine.driver_data<victory_state>();
	struct micro_t &micro = state->m_micro;
/*
    Actual microcode:
          02    00                    0   0  ZERORAM, INCI, SWVRAM

    Basic gist of things:
        WRITE
        I++
        goto state00
*/
	int addr = micro.i++ & 0x3fff;

	if (micro.cmd & 0x10)
		state->m_gram[addr] = micro.g;
	if (micro.cmd & 0x20)
		state->m_bram[addr] = micro.b;
	if (micro.cmd & 0x40)
		state->m_rram[addr] = micro.r;

	count_states(micro, 3);
	return 0;
}


/*************************************
 *
 *  Microcode command 3:
 *      Draw sprite from I to (X,Y)
 *
 *************************************/

static int command3(running_machine &machine)
{
	victory_state *state = machine.driver_data<victory_state>();
	struct micro_t &micro = state->m_micro;
/*
    Actual microcode:
          03    1C                    2   0  SLOADLH, SXFERY
          1C    HTC ? 19 : 1D         0   1
          1D    1E                    1   2  INCX, SXFERX+INCX, INCL, SWVRAM
          1E    /LTC ? 1C : 1F        1   3  INCI, SINCY, SWVRAM
          1F    1C                    1   0  ZERORAM, SXFERY, SADDX, INCH

    Basic gist of things:
        H = R >> 5
        L = (R & 0x1f) << 1
        Y = Y'
        state1C:
            if (H & 8) goto state19
            X = X'; L++
            WRITE
            I++; Y++
            if ((L & 0x20) == 0) goto state1C
            Y = Y'; X'++; H++
            goto state1C
*/
	int ycount = 64 - (micro.r & 31) * 2;
	int xcount = 8 - (micro.r >> 5);
	int shift = micro.xp & 7;
	int nshift = 8 - shift;
	int x, y, sy;

	for (x = 0; x < xcount; x++, micro.xp += 8)
	{
		sy = micro.yp;

		for (y = 0; y < ycount; y++)
		{
			int srcoffs = micro.i++ & 0x3fff;
			int dstoffs = (sy++ & 0xff) * 32 + micro.xp / 8;
			UINT8 src;

			/* non-collision-detect case */
			if (!(micro.cmd & 0x08) || state->m_fgcoll)
			{
				if (micro.cmd & 0x10)
				{
					src = state->m_gram[srcoffs];
					state->m_gram[dstoffs + 0] ^= src >> shift;
					state->m_gram[dstoffs + 1] ^= src << nshift;
				}
				if (micro.cmd & 0x20)
				{
					src = state->m_bram[srcoffs];
					state->m_bram[dstoffs + 0] ^= src >> shift;
					state->m_bram[dstoffs + 1] ^= src << nshift;
				}
				if (micro.cmd & 0x40)
				{
					src = state->m_rram[srcoffs];
					state->m_rram[dstoffs + 0] ^= src >> shift;
					state->m_rram[dstoffs + 1] ^= src << nshift;
				}
			}

			/* collision-detect case */
			else
			{
				if (micro.cmd & 0x10)
				{
					src = state->m_gram[srcoffs];
					if ((state->m_gram[dstoffs + 0] & (src >> shift)) | (state->m_gram[dstoffs + 1] & (src << nshift)))
						state->m_fgcoll = 1, state->m_fgcollx = micro.xp, state->m_fgcolly = sy - 1;
					state->m_gram[dstoffs + 0] ^= src >> shift;
					state->m_gram[dstoffs + 1] ^= src << nshift;
				}
				if (micro.cmd & 0x20)
				{
					src = state->m_bram[srcoffs];
					if ((state->m_bram[dstoffs + 0] & (src >> shift)) | (state->m_bram[dstoffs + 1] & (src << nshift)))
						state->m_fgcoll = 1, state->m_fgcollx = micro.xp, state->m_fgcolly = sy - 1;
					state->m_bram[dstoffs + 0] ^= src >> shift;
					state->m_bram[dstoffs + 1] ^= src << nshift;
				}
				if (micro.cmd & 0x40)
				{
					src = state->m_rram[srcoffs];
					if ((state->m_rram[dstoffs + 0] & (src >> shift)) | (state->m_rram[dstoffs + 1] & (src << nshift)))
						state->m_fgcoll = 1, state->m_fgcollx = micro.xp, state->m_fgcolly = sy - 1;
					state->m_rram[dstoffs + 0] ^= src >> shift;
					state->m_rram[dstoffs + 1] ^= src << nshift;
				}
				if (state->m_fgcoll) victory_update_irq(machine);
			}
		}
	}

	count_states(micro, 3 + (2 + 2 * ycount) * xcount);

	return micro.cmd & 0x80;
}


/*************************************
 *
 *  Microcode command 4:
 *      Execute commands at (Y * 2)
 *
 *************************************/

static int command4(running_machine &machine)
{
	victory_state *state = machine.driver_data<victory_state>();
	struct micro_t &micro = state->m_micro;
/*
    Actual microcode:
          04    1A                    2   0  SLOADPC
          1A    1B                    2   0  INCPC, SWRITEBUSS1
          1B    01                    2   0  INCPC, SWRITEBUSS2

    Basic gist of things:
        PC = Y' << 1
        CM = GREEN[PC]
        I = (BLUE[PC] << 8) + RED[PC]
        PC++
        R = GREEN[PC]
        X' = RED[PC]
        Y' = BLUE[PC]
        PC++
        goto state01
*/
	int keep_going = 0;

	if (LOG_MICROCODE) logerror("================= EXECUTE BEGIN\n");

	count_states(micro, 4);

	micro.pc = micro.yp << 1;
	do
	{
		micro.cmd = state->m_gram[0x2000 + micro.pc];
		micro.cmdlo = micro.cmd & 7;
		micro.i = (state->m_bram[0x2000 + micro.pc] << 8) | state->m_rram[0x2000 + micro.pc];
		micro.r = state->m_gram[0x2001 + micro.pc];
		micro.xp = state->m_rram[0x2001 + micro.pc];
		micro.yp = state->m_bram[0x2001 + micro.pc];
		if (LOG_MICROCODE) logerror("PC=%03X  CMD=%02X I=%04X R=%02X X=%02X Y=%02X\n", micro.pc, micro.cmd, micro.i, micro.r, micro.xp, micro.yp);
		micro.pc = (micro.pc + 2) & 0x1ff;

		switch (micro.cmdlo)
		{
			case 0:												break;
			case 1:												break;
			case 2:	keep_going = command2(machine);					break;
			case 3:	keep_going = command3(machine);				break;
			case 4:	micro.pc = micro.yp << 1; keep_going = 1;	break;
			case 5:	keep_going = command5(machine);				break;
			case 6:	keep_going = command6(machine);					break;
			case 7:	keep_going = command7(machine);				break;
		}
	} while (keep_going);

	if (LOG_MICROCODE) logerror("================= EXECUTE END\n");

	return micro.cmd & 0x80;
}


/*************************************
 *
 *  Microcode command 5:
 *      Draw vector from (X,Y)
 *
 *************************************/

static int command5(running_machine &machine)
{
	victory_state *state = machine.driver_data<victory_state>();
	struct micro_t &micro = state->m_micro;
/*
    Actual microcode:
          05    0A                    1   0  SXFERY, ADD128+SACCCLEAR, SACCCLK
          0A    VFIN ? 19 : 0B        1   0  SXFERX+INCX
          0B    0A                    1   2  INCI, SACCCLK, SWVRAM

    Basic gist of things:
        Y = Y'; ACC = 128
        X = X'/CLOCK SR
        while (!(IL & 0x100))
        {
            IL++; ACC += IH
            adjust X,Y based on carry
            WRITE(X,Y)  [SR1]
        }

    line draw: one of 8 cases based on VDATA

                no carry            carry
                --------            -----
        case 0: 1011 -> X++, Y      1101 -> X++, Y--
        case 1: 0101 -> X, Y--      1101 -> X++, Y--
        case 2: 0101 -> X, Y--      1100 -> X--, Y--
        case 3: 1010 -> X--, Y      1100 -> X--, Y--
        case 4: 1010 -> X--, Y      1110 -> X--, Y++
        case 5: 0111 -> X, Y++      1110 -> X--, Y++
        case 6: 0111 -> X, Y++      1111 -> X++, Y++
        case 7: 1011 -> X++, Y      1111 -> X++, Y++

*/
	static const INT8 inctable[8][4] =
	{
		{  1, 0, 1,-1 },
		{  0,-1, 1,-1 },
		{  0,-1,-1,-1 },
		{ -1, 0,-1,-1 },
		{ -1, 0,-1, 1 },
		{  0, 1,-1, 1 },
		{  0, 1, 1, 1 },
		{  1, 0, 1, 1 }
	};

	int xinc = inctable[(micro.cmd >> 4) & 7][0];
	int yinc = inctable[(micro.cmd >> 4) & 7][1];
	int xincc = inctable[(micro.cmd >> 4) & 7][2];
	int yincc = inctable[(micro.cmd >> 4) & 7][3];
	UINT8 x = micro.xp;
	UINT8 y = micro.yp;
	int acc = 0x80;
	int i = micro.i >> 8;
	int c;

	/* non-collision-detect case */
	if (!(micro.cmd & 0x08) || state->m_fgcoll)
	{
		for (c = micro.i & 0xff; c < 0x100; c++)
		{
			int addr = y * 32 + x / 8;
			int shift = x & 7;
			int nshift = 8 - shift;

			state->m_gram[addr + 0] ^= micro.g >> shift;
			state->m_gram[addr + 1] ^= micro.g << nshift;
			state->m_bram[addr + 0] ^= micro.b >> shift;
			state->m_bram[addr + 1] ^= micro.b << nshift;
			state->m_rram[addr + 0] ^= micro.r >> shift;
			state->m_rram[addr + 1] ^= micro.r << nshift;

			acc += i;
			if (acc & 0x100)
			{
				x += xincc;
				y += yincc;
			}
			else
			{
				x += xinc;
				y += yinc;
			}
			acc &= 0xff;
		}
	}

	/* collision-detect case */
	else
	{
		for (c = micro.i & 0xff; c < 0x100; c++)
		{
			int addr = y * 32 + x / 8;
			int shift = x & 7;
			int nshift = 8 - shift;

			if ((state->m_gram[addr + 0] & (micro.g >> shift)) | (state->m_gram[addr + 1] & (micro.g << nshift)) |
				(state->m_bram[addr + 0] & (micro.b >> shift)) | (state->m_bram[addr + 1] & (micro.b << nshift)) |
				(state->m_rram[addr + 0] & (micro.r >> shift)) | (state->m_rram[addr + 1] & (micro.r << nshift)))
				state->m_fgcoll = 1, state->m_fgcollx = x, state->m_fgcolly = y;

			state->m_gram[addr + 0] ^= micro.g >> shift;
			state->m_gram[addr + 1] ^= micro.g << nshift;
			state->m_bram[addr + 0] ^= micro.b >> shift;
			state->m_bram[addr + 1] ^= micro.b << nshift;
			state->m_rram[addr + 0] ^= micro.r >> shift;
			state->m_rram[addr + 1] ^= micro.r << nshift;

			acc += i;
			if (acc & 0x100)
			{
				x += xincc;
				y += yincc;
			}
			else
			{
				x += xinc;
				y += yinc;
			}
			acc &= 0xff;
		}
		if (state->m_fgcoll) victory_update_irq(machine);
	}

	micro.xp = x;

	count_states(micro, 3 + 2 * (0x100 - (micro.i & 0xff)));

	return micro.cmd & 0x80;
}


/*************************************
 *
 *  Microcode command 6:
 *      Copy data from I to (Y * 2)
 *
 *************************************/

static int command6(running_machine &machine)
{
	victory_state *state = machine.driver_data<victory_state>();
	struct micro_t &micro = state->m_micro;
/*
    Actual microcode:
          06    0C                    0   0  SLOADLH, SLOADPC
          0C    0D                    0   1  INCI, SXFERX+INCX, INCL
          0D    /LTC ? 0C : 0E        2   2  ZERORAM, INCPC, SWVRAM
          0E    19                    2   2

    Basic gist of things:
        H = R >> 5
        L = (R & 0x1f) << 1
        PC = Y'
        state0C:
            I++; X = X'; L++
            WRITE(I, *PC)
            PC++
            if ((L & 0x20) == 0) goto state1C
*/
	int i;

	micro.pc = micro.yp << 1;
	for (i = (micro.r & 31) << 1; i < 64; i++)
	{
		int saddr = micro.i++ & 0x3fff;
		int daddr = 0x2000 + micro.pc++;
		micro.pc &= 0x1ff;

		if (micro.cmd & 0x10)
			state->m_gram[daddr] = state->m_gram[saddr];
		if (micro.cmd & 0x20)
			state->m_bram[daddr] = state->m_bram[saddr];
		if (micro.cmd & 0x40)
			state->m_rram[daddr] = state->m_rram[saddr];
	}

	count_states(micro, 3 + 2 * (64 - (micro.r & 31) * 2));

	return micro.cmd & 0x80;
}


/*************************************
 *
 *  Microcode command 7:
 *      Draw pixels to (X,Y)
 *
 *************************************/

static int command7(running_machine &machine)
{
	victory_state *state = machine.driver_data<victory_state>();
	struct micro_t &micro = state->m_micro;
/*
    Actual microcode:
          07    08                    1   0  SXFERY, SXFERX+INCX
          08    09                    1   2  INCX, SWVRAM
          09    00                    1   3  SWVRAM

    Basic gist of things:
        Y = Y'
        X = X'/CLOCK SR
        WRITE SR1
        X++
        WRITE SR2
*/
	int addr = micro.yp * 32 + micro.xp / 8;
	int shift = micro.xp & 7;
	int nshift = 8 - shift;

	/* non-collision-detect case */
	if (!(micro.cmd & 0x08) || state->m_fgcoll)
	{
		if (micro.cmd & 0x10)
		{
			state->m_gram[addr + 0] ^= micro.g >> shift;
			state->m_gram[addr + 1] ^= micro.g << nshift;
		}
		if (micro.cmd & 0x20)
		{
			state->m_bram[addr + 0] ^= micro.b >> shift;
			state->m_bram[addr + 1] ^= micro.b << nshift;
		}
		if (micro.cmd & 0x40)
		{
			state->m_rram[addr + 0] ^= micro.r >> shift;
			state->m_rram[addr + 1] ^= micro.r << nshift;
		}
	}

	/* collision-detect case */
	else
	{
		if (micro.cmd & 0x10)
		{
			if ((state->m_gram[addr + 0] & (micro.g >> shift)) | (state->m_gram[addr + 1] & (micro.g << nshift)))
				state->m_fgcoll = 1, state->m_fgcollx = micro.xp + 8, state->m_fgcolly = micro.yp;
			state->m_gram[addr + 0] ^= micro.g >> shift;
			state->m_gram[addr + 1] ^= micro.g << nshift;
		}
		if (micro.cmd & 0x20)
		{
			if ((state->m_bram[addr + 0] & (micro.b >> shift)) | (state->m_bram[addr + 1] & (micro.b << nshift)))
				state->m_fgcoll = 1, state->m_fgcollx = micro.xp + 8, state->m_fgcolly = micro.yp;
			state->m_bram[addr + 0] ^= micro.b >> shift;
			state->m_bram[addr + 1] ^= micro.b << nshift;
		}
		if (micro.cmd & 0x40)
		{
			if ((state->m_rram[addr + 0] & (micro.r >> shift)) | (state->m_rram[addr + 1] & (micro.r << nshift)))
				state->m_fgcoll = 1, state->m_fgcollx = micro.xp + 8, state->m_fgcolly = micro.yp;
			state->m_rram[addr + 0] ^= micro.r >> shift;
			state->m_rram[addr + 1] ^= micro.r << nshift;
		}
		if (state->m_fgcoll) victory_update_irq(machine);
	}

	count_states(micro, 4);

	return micro.cmd & 0x80;
}


/*************************************
 *
 *  Background update
 *
 *************************************/

static void update_background(running_machine &machine)
{
	victory_state *state = machine.driver_data<victory_state>();
	int x, y, row, offs;

	for (y = offs = 0; y < 32; y++)
		for (x = 0; x < 32; x++, offs++)
		{
			int code = state->m_videoram[offs];

			for (row = 0; row < 8; row++)
			{
				UINT8 pix2 = state->m_charram[0x0000 + 8 * code + row];
				UINT8 pix1 = state->m_charram[0x0800 + 8 * code + row];
				UINT8 pix0 = state->m_charram[0x1000 + 8 * code + row];
				UINT8 *dst = &state->m_bgbitmap[(y * 8 + row) * 256 + x * 8];

				*dst++ = ((pix2 & 0x80) >> 5) | ((pix1 & 0x80) >> 6) | ((pix0 & 0x80) >> 7);
				*dst++ = ((pix2 & 0x40) >> 4) | ((pix1 & 0x40) >> 5) | ((pix0 & 0x40) >> 6);
				*dst++ = ((pix2 & 0x20) >> 3) | ((pix1 & 0x20) >> 4) | ((pix0 & 0x20) >> 5);
				*dst++ = ((pix2 & 0x10) >> 2) | ((pix1 & 0x10) >> 3) | ((pix0 & 0x10) >> 4);
				*dst++ = ((pix2 & 0x08) >> 1) | ((pix1 & 0x08) >> 2) | ((pix0 & 0x08) >> 3);
				*dst++ = ((pix2 & 0x04)     ) | ((pix1 & 0x04) >> 1) | ((pix0 & 0x04) >> 2);
				*dst++ = ((pix2 & 0x02) << 1) | ((pix1 & 0x02)     ) | ((pix0 & 0x02) >> 1);
				*dst++ = ((pix2 & 0x01) << 2) | ((pix1 & 0x01) << 1) | ((pix0 & 0x01)     );
			}
		}
}


/*************************************
 *
 *  Foreground update
 *
 *************************************/

static void update_foreground(running_machine &machine)
{
	victory_state *state = machine.driver_data<victory_state>();
	int x, y;

	for (y = 0; y < 256; y++)
	{
		UINT8 *dst = &state->m_fgbitmap[y * 256];

		/* assemble the RGB bits for each 8-pixel chunk */
		for (x = 0; x < 256; x += 8)
		{
			UINT8 g = state->m_gram[y * 32 + x / 8];
			UINT8 b = state->m_bram[y * 32 + x / 8];
			UINT8 r = state->m_rram[y * 32 + x / 8];

			*dst++ = ((r & 0x80) >> 5) | ((b & 0x80) >> 6) | ((g & 0x80) >> 7);
			*dst++ = ((r & 0x40) >> 4) | ((b & 0x40) >> 5) | ((g & 0x40) >> 6);
			*dst++ = ((r & 0x20) >> 3) | ((b & 0x20) >> 4) | ((g & 0x20) >> 5);
			*dst++ = ((r & 0x10) >> 2) | ((b & 0x10) >> 3) | ((g & 0x10) >> 4);
			*dst++ = ((r & 0x08) >> 1) | ((b & 0x08) >> 2) | ((g & 0x08) >> 3);
			*dst++ = ((r & 0x04)     ) | ((b & 0x04) >> 1) | ((g & 0x04) >> 2);
			*dst++ = ((r & 0x02) << 1) | ((b & 0x02)     ) | ((g & 0x02) >> 1);
			*dst++ = ((r & 0x01) << 2) | ((b & 0x01) << 1) | ((g & 0x01)     );
		}
	}
}


static TIMER_CALLBACK( bgcoll_irq_callback )
{
	victory_state *state = machine.driver_data<victory_state>();
	state->m_bgcollx = param & 0xff;
	state->m_bgcolly = param >> 8;
	state->m_bgcoll = 1;
	victory_update_irq(machine);
}



/*************************************
 *
 *  Standard screen refresh callback
 *
 *************************************/

SCREEN_UPDATE( victory )
{
	victory_state *state = screen->machine().driver_data<victory_state>();
	int bgcollmask = (state->m_video_control & 4) ? 4 : 7;
	int count = 0;
	int x, y;

	/* copy the palette from palette RAM */
	set_palette(screen->machine());

	/* update the foreground & background */
	update_foreground(screen->machine());
	update_background(screen->machine());

	/* blend the bitmaps and do collision detection */
	for (y = 0; y < 256; y++)
	{
		UINT16 *scanline = BITMAP_ADDR16(bitmap, y, 0);
		UINT8 sy = state->m_scrolly + y;
		UINT8 *fg = &state->m_fgbitmap[y * 256];
		UINT8 *bg = &state->m_bgbitmap[sy * 256];

		/* do the blending */
		for (x = 0; x < 256; x++)
		{
			int fpix = *fg++;
			int bpix = bg[(x + state->m_scrollx) & 255];
			scanline[x] = bpix | (fpix << 3);
			if (fpix && (bpix & bgcollmask) && count++ < 128)
				screen->machine().scheduler().timer_set(screen->time_until_pos(y, x), FUNC(bgcoll_irq_callback), x | (y << 8));
		}
	}

	return 0;
}
