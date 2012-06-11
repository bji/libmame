/***************************************************************************

    Sega G-80 raster hardware

    Across these games, there's a mixture of discrete sound circuitry,
    speech boards, ADPCM samples, and a TMS3617 music chip.

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "includes/segag80r.h"
#include "machine/i8255.h"
#include "machine/i8243.h"
#include "sound/samples.h"
#include "sound/tms36xx.h"
#include "sound/dac.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define SEGA005_555_TIMER_FREQ		(1.44 / ((15000 + 2 * 4700) * 1.5e-6))
#define SEGA005_COUNTER_FREQ		(100000)	/* unknown, just a guess */

DEVICE_GET_INFO( sega005_sound );

DECLARE_LEGACY_SOUND_DEVICE(SEGA005, sega005_sound);
DEFINE_LEGACY_SOUND_DEVICE(SEGA005, sega005_sound);



/*************************************
 *
 *  Astro Blaster sound hardware
 *
 *************************************/

static SOUND_START( astrob );

/*
    Description of Astro Blaster sounds (in the hope of future discrete goodness):

    CD4017 = decade counter with one output per decoded stage (10 outputs altogether)
    CD4024 = 7-bit counter with 7 outputs


    "V" signal
    ----------
        CD4017 @ U15:
            reset by RATE RESET signal = 1
            clocked by falling edge of ATTACK signal
            +12V output from here goes through a diode and one of 10 resistors:
                0 = 120k
                1 = 82k
                2 = 62k
                3 = 56k
                4 = 47k
                5 = 39k
                6 = 35k
                7 = 27k
                8 = 24k
                9 = 22k
            and then in series through a 22k resistor

        Op-amp @ U6 takes the WARP signal and the output of CD4017 @ U15
            and forms the signal "V" which is used to control the invader
            sounds


        How to calculate the output voltage at U16 labeled (V).
        (Derrick Renaud)

        First you have an inverting amp.  To get the gain you
        use G=-Rf/Ri, where Rf=R178=22k.  Ri is the selected
        resistor on the output of U15.

        The input voltage to the amp (pin 6) will always be
        about 12V - 0.5V (diode drop in low current circuit) =
        11.5V.

        Now you need to calculate the reference voltage on the
        + input (pin 5).  Depending on the state of WARP...

        If the warp data is 0, then U31 inverts it to an Open
        Collector high, meaning WARP is out of circuit. So:
        Vref = 12V * (R163)/(R162+R163)
             = 12V * 10k/(10K+4.7k)
             = 8.163V

        When warp data is 1, then U31 inverts it to low,
        grounding R164 putting it in parallel with R163,
        giving:
        Vref = 12V * (R163||R164)/(R163||R164 +R162)
             = 12V * 5k/(5k+4.7k)
             = 6.186V

        Now to get the control voltage V:
        V = (Vi - Vref) * G + Vref
          = (11.5V - Vref) * G + Vref

        That gives you the control voltage at V.  From there I
        would have to millman the voltage with the internal
        voltage/resistors of the 555 to get the actual used
        control voltage.

        But it seems you just want a range, so just use the
        above info to get the highest and lowest voltages
        generated, and create the frequency shift you desire.
        Remember as the control voltage (V) lowers, the
        frequency increases.



    INVADER-1 output
    ----------------




    INVADER-2 output
    ----------------
        555 timer @ U13 in astable mode with the following parameters:
            R1 = 10k
            R2 = 100k
            C = 0.0022u
            CV = "V" signal
            Reset = (PORT076 & 0x02)
        Output goes to CD4024 @ U12

        CD4024 @ U12:
            reset through some unknown means
            clocked by 555 timer @ U13
            +12 output from here goes through a resistor ladder:
                Q1 -> 82k
                Q2 -> 39k
                Q3 -> 22k
                Q4 -> 10k
        Summed output from here is INVADER-2


    INVADER-3 output
    ----------------
        555 timer at U17 in astable mode with the following parameters:
            R1 = 10k
            R2 = 68k
            C = 0.1u
            CV = some combination of "V" and "W" signals
            Reset = (PORT076 & 0x04)
        Output from here is INVADER-3

*/

static const char *const astrob_sample_names[] =
{
	"*astrob",
	"invadr1",		/* 0 */
	"winvadr1",		/* 1 */
	"invadr2",		/* 2 */
	"winvadr2",		/* 3 */
	"invadr3",		/* 4 */
	"winvadr3",		/* 5 */
	"invadr4",		/* 6 */
	"winvadr4",		/* 7 */
	"asteroid",		/* 8 */
	"refuel",		/* 9 */
	"pbullet",		/* 10 */
	"ebullet",		/* 11 */
	"eexplode",		/* 12 */
	"pexplode",		/* 13 */
	"deedle",		/* 14 */
	"sonar",		/* 15 */
	0
};


static const samples_interface astrob_samples_interface =
{
	11,
	astrob_sample_names
};


MACHINE_CONFIG_FRAGMENT( astrob_sound_board )

	MCFG_SOUND_START(astrob)

	/* sound hardware */
	MCFG_SAMPLES_ADD("samples", astrob_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/*************************************
 *
 *  Startup configuration
 *
 *************************************/

static SOUND_START( astrob )
{
	segag80r_state *state = machine.driver_data<segag80r_state>();
	state_save_register_global_array(machine, state->m_sound_state);
	state_save_register_global(machine, state->m_sound_rate);
}



/*************************************
 *
 *  Astro Blaster sound triggers
 *
 *************************************/

WRITE8_MEMBER(segag80r_state::astrob_sound_w)
{
	static const float attack_resistor[10] =
	{
		120.0f, 82.0f, 62.0f, 56.0f, 47.0f, 39.0f, 33.0f, 27.0f, 24.0f, 22.0f
	};
	samples_device *samples = machine().device<samples_device>("samples");
	float freq_factor;

	UINT8 diff = data ^ m_sound_state[offset];
	m_sound_state[offset] = data;

	switch (offset)
	{
		case 0:
			/* INVADER-1: channel 0 */
			if ((diff & 0x01) && !(data & 0x01)) samples->start(0, (data & 0x80) ? 0 : 1, true);
			if ((data & 0x01) && samples->playing(0)) samples->stop(0);

			/* INVADER-2: channel 1 */
			if ((diff & 0x02) && !(data & 0x02)) samples->start(1, (data & 0x80) ? 2 : 3, true);
			if ((data & 0x02) && samples->playing(1)) samples->stop(1);

			/* INVADER-3: channel 2 */
			if ((diff & 0x04) && !(data & 0x04)) samples->start(2, (data & 0x80) ? 4 : 5, true);
			if ((data & 0x04) && samples->playing(2)) samples->stop(2);

			/* INVADER-4: channel 3 */
			if ((diff & 0x08) && !(data & 0x08)) samples->start(3, (data & 0x80) ? 6 : 7, true);
			if ((data & 0x08) && samples->playing(3)) samples->stop(3);

			/* ASTROIDS: channel 4 */
			if ((diff & 0x10) && !(data & 0x10)) samples->start(4, 8, true);
			if ((data & 0x10) && samples->playing(4)) samples->stop(4);

			/* MUTE */
			machine().sound().system_mute(data & 0x20);

			/* REFILL: channel 5 */
			if (!(data & 0x40) && !samples->playing(5)) samples->start(5, 9);
			if ( (data & 0x40) && samples->playing(5))  samples->stop(5);

			/* WARP: changes which sample is played for the INVADER samples above */
			if (diff & 0x80)
			{
				if (samples->playing(0)) samples->start(0, (data & 0x80) ? 0 : 1, true);
				if (samples->playing(1)) samples->start(1, (data & 0x80) ? 2 : 3, true);
				if (samples->playing(2)) samples->start(2, (data & 0x80) ? 4 : 5, true);
				if (samples->playing(3)) samples->start(3, (data & 0x80) ? 6 : 7, true);
			}
			break;

		case 1:
			/* LASER #1: channel 6 */
			if ((diff & 0x01) && !(data & 0x01)) samples->start(6, 10);

			/* LASER #2: channel 7 */
			if ((diff & 0x02) && !(data & 0x02)) samples->start(7, 11);

			/* SHORT EXPL: channel 8 */
			if ((diff & 0x04) && !(data & 0x04)) samples->start(8, 12);

			/* LONG EXPL: channel 8 */
			if ((diff & 0x08) && !(data & 0x08)) samples->start(8, 13);

			/* ATTACK RATE */
			if ((diff & 0x10) && !(data & 0x10)) m_sound_rate = (m_sound_rate + 1) % 10;

			/* RATE RESET */
			if (!(data & 0x20)) m_sound_rate = 0;

			/* BONUS: channel 9 */
			if ((diff & 0x40) && !(data & 0x40)) samples->start(9, 14);

			/* SONAR: channel 10 */
			if ((diff & 0x80) && !(data & 0x80)) samples->start(10, 15);
			break;
	}

	/* the samples were recorded with sound_rate = 0, so we need to scale */
	/* the frequency as a fraction of that; these equations come from */
	/* Derrick's analysis above; we compute the inverted scale factor to */
	/* account for the fact that frequency goes up as CV goes down */
	/* WARP is already taken into account by the differing samples above */
	freq_factor  = (11.5f - 8.163f) * (-22.0f / attack_resistor[0]) + 8.163f;
	freq_factor /= (11.5f - 8.163f) * (-22.0f / attack_resistor[m_sound_rate]) + 8.163f;

	/* adjust the sample rate of invader sounds based the sound_rate */
	/* this is an approximation */
	if (samples->playing(0)) samples->set_frequency(0, samples->base_frequency(0) * freq_factor);
	if (samples->playing(1)) samples->set_frequency(1, samples->base_frequency(1) * freq_factor);
	if (samples->playing(2)) samples->set_frequency(2, samples->base_frequency(2) * freq_factor);
	if (samples->playing(3)) samples->set_frequency(3, samples->base_frequency(3) * freq_factor);
}



/*************************************
 *
 *  005 sound hardware
 *
 *************************************/

static SOUND_START( sega005 );
static STREAM_UPDATE( sega005_stream_update );
static TIMER_CALLBACK( sega005_auto_timer );
static WRITE8_DEVICE_HANDLER( sega005_sound_a_w );
static WRITE8_DEVICE_HANDLER( sega005_sound_b_w );

/*
    005

    The Sound Board consists of the following:

    An 8255:
        Port A controls the sounds that use discrete circuitry
            A0 - Large Expl. Sound Trig
            A1 - Small Expl. Sound Trig
            A2 - Drop Sound Bomb Trig
            A3 - Shoot Sound Pistol Trig
            A4 - Missile Sound Trig
            A5 - Helicopter Sound Trig
            A6 - Whistle Sound Trig
            A7 - <unused>

      Port B controls the melody generator (described below)

      Port C is apparently unused


    Melody Generator:

        555 timer frequency = 1.44/((R1 + 2R2)*C)
        R1 = 15e3
        R2 = 4.7e3
        C=1.5e-6
        Frequency = 39.344 Hz

        Auto timer is enabled if port B & 0x20 == 1
        Auto timer is reset if 2716 value & 0x20 == 0

        Manual timer is enabled if port B & 0x20 == 0
        Manual timer is clocked if port B & 0x40 goes from 0 to 1

        Both auto and manual timers clock LS393 counter
        Counter is held to 0 if port B & 0x10 == 1

        Output of LS393 >> 1 selects low 7 bits of lookup in 2716.
        High 4 bits come from port B bits 0-3.

        Low 5 bits of output from 2716 look up value in 6331 PROM at U8 (32x8)

        8-bit output of 6331 at U8 is loaded into pair of LS161 counters whenever they overflow.
        LS161 counters are clocked somehow (not clear how)

        Carry output from LS161 counters (overflowing 8 bits) goes to the B
            input on the LS293 counter at U14.
        Rising edge of B input clocks bit 1 of counter (effectively adding 2).
        Output B (bit 1) is mixed with output D (bit 3) with different weights
            through a small RC circuit and fed into the 4391 input at U32.

        The 4391 output is the final output.
*/

static const char *const sega005_sample_names[] =
{
	"*005",
	"lexplode",		/* 0 */
	"sexplode",		/* 1 */
	"dropbomb",		/* 2 */
	"shoot",		/* 3 */
	"missile",		/* 4 */
	"helicopt",		/* 5 */
	"whistle",		/* 6 */
	0
};


static const samples_interface sega005_samples_interface =
{
	7,
	sega005_sample_names
};


static I8255A_INTERFACE( ppi8255_005_intf )
{
	DEVCB_NULL,							/* Port A read */
	DEVCB_HANDLER(sega005_sound_a_w),	/* Port A write */
	DEVCB_NULL,							/* Port B read */
	DEVCB_HANDLER(sega005_sound_b_w),	/* Port B write */
	DEVCB_NULL,							/* Port C read */
	DEVCB_NULL							/* Port C write */
};


MACHINE_CONFIG_FRAGMENT( 005_sound_board )

	MCFG_I8255A_ADD( "ppi8255", ppi8255_005_intf )

	/* sound hardware */
	MCFG_SOUND_START(sega005)

	MCFG_SAMPLES_ADD("samples", sega005_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("005", SEGA005, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/*************************************
 *
 *  Startup configuration
 *
 *************************************/

static SOUND_START( sega005 )
{
	segag80r_state *state = machine.driver_data<segag80r_state>();
	state_save_register_global_array(machine, state->m_sound_state);
	state_save_register_global(machine, state->m_sound_addr);
	state_save_register_global(machine, state->m_sound_data);
	state_save_register_global(machine, state->m_square_state);
	state_save_register_global(machine, state->m_square_count);
}



/*************************************
 *
 *  005 sound triggers
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( sega005_sound_a_w )
{
	segag80r_state *state = device->machine().driver_data<segag80r_state>();
	samples_device *samples = device->machine().device<samples_device>("samples");
	UINT8 diff = data ^ state->m_sound_state[0];
	state->m_sound_state[0] = data;

	/* LARGE EXPL: channel 0 */
	if ((diff & 0x01) && !(data & 0x01)) samples->start(0, 0);

	/* SMALL EXPL: channel 1 */
	if ((diff & 0x02) && !(data & 0x02)) samples->start(1, 1);

	/* DROP BOMB: channel 2 */
	if ((diff & 0x04) && !(data & 0x04)) samples->start(2, 2);

	/* SHOOT PISTOL: channel 3 */
	if ((diff & 0x08) && !(data & 0x08)) samples->start(3, 3);

	/* MISSILE: channel 4 */
	if ((diff & 0x10) && !(data & 0x10)) samples->start(4, 4);

	/* HELICOPTER: channel 5 */
	if ((diff & 0x20) && !(data & 0x20) && !samples->playing(5)) samples->start(5, 5, true);
	if ((diff & 0x20) &&  (data & 0x20)) samples->stop(5);

	/* WHISTLE: channel 6 */
	if ((diff & 0x40) && !(data & 0x40) && !samples->playing(6)) samples->start(6, 6, true);
	if ((diff & 0x40) &&  (data & 0x40)) samples->stop(6);
}


INLINE void sega005_update_sound_data(running_machine &machine)
{
	segag80r_state *state = machine.driver_data<segag80r_state>();
	UINT8 newval = state->memregion("005")->base()[state->m_sound_addr];
	UINT8 diff = newval ^ state->m_sound_data;

	//mame_printf_debug("  [%03X] = %02X\n", state->m_sound_addr, newval);

	/* latch the new value */
	state->m_sound_data = newval;

	/* if bit 5 goes high, we reset the timer */
	if ((diff & 0x20) && !(newval & 0x20))
	{
		//mame_printf_debug("Stopping timer\n");
		state->m_sega005_sound_timer->adjust(attotime::never);
	}

	/* if bit 5 goes low, we start the timer again */
	if ((diff & 0x20) && (newval & 0x20))
	{
		//mame_printf_debug("Starting timer\n");
		state->m_sega005_sound_timer->adjust(attotime::from_hz(SEGA005_555_TIMER_FREQ), 0, attotime::from_hz(SEGA005_555_TIMER_FREQ));
	}
}


static WRITE8_DEVICE_HANDLER( sega005_sound_b_w )
{
	segag80r_state *state = device->machine().driver_data<segag80r_state>();
	/*
           D6: manual timer clock (0->1)
           D5: 0 = manual timer, 1 = auto timer
           D4: 1 = hold/reset address counter to 0
        D3-D0: upper 4 bits of ROM address
    */
	UINT8 diff = data ^ state->m_sound_state[1];
	state->m_sound_state[1] = data;

	//mame_printf_debug("sound[%d] = %02X\n", 1, data);

	/* force a stream update */
	state->m_sega005_stream->update();

	/* ROM address */
	state->m_sound_addr = ((data & 0x0f) << 7) | (state->m_sound_addr & 0x7f);

	/* reset both sound address and square wave counters */
	if (data & 0x10)
	{
		state->m_sound_addr &= 0x780;
		state->m_square_state = 0;
	}

	/* manual clock */
	if ((diff & 0x40) && (data & 0x40) && !(data & 0x20) && !(data & 0x10))
		state->m_sound_addr = (state->m_sound_addr & 0x780) | ((state->m_sound_addr + 1) & 0x07f);

	/* update the sound data */
	sega005_update_sound_data(device->machine());
}



/*************************************
 *
 *  005 custom sound generation
 *
 *************************************/

static DEVICE_START( sega005_sound )
{
	segag80r_state *state = device->machine().driver_data<segag80r_state>();
	running_machine &machine = device->machine();

	/* create the stream */
	state->m_sega005_stream = device->machine().sound().stream_alloc(*device, 0, 1, SEGA005_COUNTER_FREQ, NULL, sega005_stream_update);

	/* create a timer for the 555 */
	state->m_sega005_sound_timer = machine.scheduler().timer_alloc(FUNC(sega005_auto_timer));

	/* set the initial sound data */
	state->m_sound_data = 0x00;
	sega005_update_sound_data(machine);
}


DEVICE_GET_INFO( sega005_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(sega005_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "005 Custom");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


static STREAM_UPDATE( sega005_stream_update )
{
	segag80r_state *state = device->machine().driver_data<segag80r_state>();
	const UINT8 *sound_prom = state->memregion("proms")->base();
	int i;

	/* no implementation yet */
	for (i = 0; i < samples; i++)
	{
		if (!(state->m_sound_state[1] & 0x10) && (++state->m_square_count & 0xff) == 0)
		{
			state->m_square_count = sound_prom[state->m_sound_data & 0x1f];

			/* hack - the RC should filter this out */
			if (state->m_square_count != 0xff)
				state->m_square_state += 2;
		}

		outputs[0][i] = (state->m_square_state & 2) ? 0x7fff : 0x0000;
	}
}


static TIMER_CALLBACK( sega005_auto_timer )
{
	segag80r_state *state = machine.driver_data<segag80r_state>();
	/* force an update then clock the sound address if not held in reset */
	state->m_sega005_stream->update();
	if ((state->m_sound_state[1] & 0x20) && !(state->m_sound_state[1] & 0x10))
	{
		state->m_sound_addr = (state->m_sound_addr & 0x780) | ((state->m_sound_addr + 1) & 0x07f);
		sega005_update_sound_data(machine);
	}
}



/*************************************
 *
 *  Space Odyssey sound hardware
 *
 *************************************/

static SOUND_START( spaceod );

static const char *const spaceod_sample_names[] =
{
	"*spaceod",
	"fire",			/* 0 */
	"bomb",			/* 1 */
	"eexplode", 	/* 2 */
	"pexplode",		/* 3 */
	"warp", 		/* 4 */
	"birth",		/* 5 */
	"scoreup",		/* 6 */
	"ssound",		/* 7 */
	"accel",		/* 8 */
	"damaged",		/* 9 */
	"erocket",		/* 10 */
	0
};


static const samples_interface spaceod_samples_interface =
{
	11,
	spaceod_sample_names
};


MACHINE_CONFIG_FRAGMENT( spaceod_sound_board )

	/* sound hardware */
	MCFG_SOUND_START(spaceod)

	MCFG_SAMPLES_ADD("samples", spaceod_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/*************************************
 *
 *  Startup configuration
 *
 *************************************/

static SOUND_START( spaceod )
{
	segag80r_state *state = machine.driver_data<segag80r_state>();
	state_save_register_global_array(machine, state->m_sound_state);
}



/*************************************
 *
 *  Space Odyssey sound triggers
 *
 *************************************/

WRITE8_MEMBER(segag80r_state::spaceod_sound_w)
{
	samples_device *samples = machine().device<samples_device>("samples");
	UINT8 diff = data ^ m_sound_state[offset];
	m_sound_state[offset] = data;

	switch (offset)
	{
		case 0:
			/* BACK G: channel 0 */
			if ((diff & 0x01) && !(data & 0x01) && !samples->playing(0)) samples->start(0, 7, true);
			if ((diff & 0x01) &&  (data & 0x01)) samples->stop(0);

			/* SHORT EXP: channel 1 */
			if ((diff & 0x04) && !(data & 0x04)) samples->start(1, 2);

			/* ACCELERATE: channel 2 */
			if ((diff & 0x10) && !(data & 0x10)) samples->start(2, 8);

			/* BATTLE STAR: channel 3 */
			if ((diff & 0x20) && !(data & 0x20)) samples->start(3, 10);

			/* D BOMB: channel 4 */
			if ((diff & 0x40) && !(data & 0x40)) samples->start(4, 1);

			/* LONG EXP: channel 5 */
			if ((diff & 0x80) && !(data & 0x80)) samples->start(5, 3);
			break;

		case 1:
			/* SHOT: channel 6 */
			if ((diff & 0x01) && !(data & 0x01)) samples->start(6, 0);

			/* BONUS UP: channel 7 */
			if ((diff & 0x02) && !(data & 0x02)) samples->start(7, 6);

			/* WARP: channel 8 */
			if ((diff & 0x08) && !(data & 0x08)) samples->start(8, 4);

			/* APPEARANCE UFO: channel 9 */
			if ((diff & 0x40) && !(data & 0x40)) samples->start(9, 5);

			/* BLACK HOLE: channel 10 */
			if ((diff & 0x80) && !(data & 0x80)) samples->start(10, 9);
			break;
	}
}



/*************************************
 *
 *  Monster Bash sound hardware
 *
 *************************************/

static SOUND_START( monsterb );
static WRITE8_DEVICE_HANDLER( monsterb_sound_a_w );
static WRITE8_DEVICE_HANDLER( monsterb_sound_b_w );
static READ8_DEVICE_HANDLER( n7751_status_r );
static WRITE8_DEVICE_HANDLER( n7751_command_w );
static WRITE8_DEVICE_HANDLER( n7751_rom_control_w );


static WRITE8_DEVICE_HANDLER( n7751_p2_w );


/*
    Monster Bash

    The Sound Board is a fairly complex mixture of different components.
    An 8255A-5 controls the interface to/from the sound board.
    Port A connects to a TMS3617 (basic music synthesizer) circuit.
    Port B connects to two sounds generated by discrete circuitry.
    Port C connects to a NEC7751 (8048 CPU derivative) to control four "samples".
*/


static const char *const monsterb_sample_names[] =
{
	"*monsterb",
	"zap",
	"jumpdown",
	0
};


static const samples_interface monsterb_samples_interface =
{
	2,
	monsterb_sample_names
};


static const tms36xx_interface monsterb_tms3617_interface =
{
	TMS3617,
	{0.5,0.5,0.5,0.5,0.5,0.5}  /* decay times of voices */
};



/*************************************
 *
 *  N7751 memory maps
 *
 *************************************/

static ADDRESS_MAP_START( monsterb_7751_portmap, AS_IO, 8, segag80r_state )
	AM_RANGE(MCS48_PORT_T1,   MCS48_PORT_T1) AM_READ(n7751_t1_r)
	AM_RANGE(MCS48_PORT_P2,   MCS48_PORT_P2) AM_READ(n7751_command_r)
	AM_RANGE(MCS48_PORT_BUS,  MCS48_PORT_BUS) AM_READ(n7751_rom_r)
	AM_RANGE(MCS48_PORT_P1,   MCS48_PORT_P1) AM_DEVWRITE_LEGACY("dac", dac_w)
	AM_RANGE(MCS48_PORT_P2,   MCS48_PORT_P2) AM_DEVWRITE_LEGACY("audio_8243", n7751_p2_w)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_DEVWRITE_LEGACY("audio_8243", i8243_prog_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static I8255A_INTERFACE( monsterb_ppi_intf )
{
	DEVCB_NULL,							/* Port A read */
	DEVCB_HANDLER(monsterb_sound_a_w),	/* Port A write */
	DEVCB_NULL,							/* Port B read */
	DEVCB_HANDLER(monsterb_sound_b_w),	/* Port B write */
	DEVCB_HANDLER(n7751_status_r),		/* Port C read */
	DEVCB_HANDLER(n7751_command_w)		/* Port C write */
};


MACHINE_CONFIG_FRAGMENT( monsterb_sound_board )

	MCFG_I8255A_ADD( "ppi8255", monsterb_ppi_intf )

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", N7751, 6000000)
	MCFG_CPU_IO_MAP(monsterb_7751_portmap)

	MCFG_I8243_ADD("audio_8243", NULL, n7751_rom_control_w)

	/* sound hardware */
	MCFG_SOUND_START(monsterb)

	MCFG_SAMPLES_ADD("samples", monsterb_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("music", TMS36XX, 247)
	MCFG_SOUND_CONFIG(monsterb_tms3617_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  Startup configuration
 *
 *************************************/

static SOUND_START( monsterb )
{
	segag80r_state *state = machine.driver_data<segag80r_state>();
	state_save_register_global_array(machine, state->m_sound_state);
	state_save_register_global(machine, state->m_sound_addr);
	state_save_register_global(machine, state->m_n7751_command);
	state_save_register_global(machine, state->m_n7751_busy);
}



/*************************************
 *
 *  TMS3617 access
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( monsterb_sound_a_w )
{
	device_t *tms = device->machine().device("music");
	int enable_val;

	/* Lower four data lines get decoded into 13 control lines */
	tms36xx_note_w(tms, 0, data & 15);

	/* Top four data lines address an 82S123 ROM that enables/disables voices */
	enable_val = device->machine().root_device().memregion("prom")->base()[(data & 0xF0) >> 4];
	tms3617_enable_w(tms, enable_val >> 2);
}



/*************************************
 *
 *  Discrete sound triggers
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( monsterb_sound_b_w )
{
	segag80r_state *state = device->machine().driver_data<segag80r_state>();
	samples_device *samples = device->machine().device<samples_device>("samples");
	UINT8 diff = data ^ state->m_sound_state[1];
	state->m_sound_state[1] = data;

	/* SHOT: channel 0 */
	if ((diff & 0x01) && !(data & 0x01)) samples->start(0, 0);

	/* DIVE: channel 1 */
	if ((diff & 0x02) && !(data & 0x02)) samples->start(1, 1);

    /* TODO: D7 on Port B might affect TMS3617 output (mute?) */
}



/*************************************
 *
 *  N7751 connections
 *
 *************************************/

static READ8_DEVICE_HANDLER( n7751_status_r )
{
	segag80r_state *state = device->machine().driver_data<segag80r_state>();
	return state->m_n7751_busy << 4;
}


static WRITE8_DEVICE_HANDLER( n7751_command_w )
{
	segag80r_state *state = device->machine().driver_data<segag80r_state>();
	/*
        Z80 7751 control port

        D0-D2 = connected to 7751 port C
        D3    = /INT line
    */
	state->m_n7751_command = data & 0x07;
	cputag_set_input_line(device->machine(), "audiocpu", 0, ((data & 0x08) == 0) ? ASSERT_LINE : CLEAR_LINE);
	device->machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}


static WRITE8_DEVICE_HANDLER( n7751_rom_control_w )
{
	segag80r_state *state = device->machine().driver_data<segag80r_state>();
	/* P4 - address lines 0-3 */
	/* P5 - address lines 4-7 */
	/* P6 - address lines 8-11 */
	/* P7 - ROM selects */
	switch (offset)
	{
		case 0:
			state->m_sound_addr = (state->m_sound_addr & ~0x00f) | ((data & 0x0f) << 0);
			break;

		case 1:
			state->m_sound_addr = (state->m_sound_addr & ~0x0f0) | ((data & 0x0f) << 4);
			break;

		case 2:
			state->m_sound_addr = (state->m_sound_addr & ~0xf00) | ((data & 0x0f) << 8);
			break;

		case 3:
			state->m_sound_addr &= 0xfff;
			{
				int numroms = state->memregion("n7751")->bytes() / 0x1000;
				if (!(data & 0x01) && numroms >= 1) state->m_sound_addr |= 0x0000;
				if (!(data & 0x02) && numroms >= 2) state->m_sound_addr |= 0x1000;
				if (!(data & 0x04) && numroms >= 3) state->m_sound_addr |= 0x2000;
				if (!(data & 0x08) && numroms >= 4) state->m_sound_addr |= 0x3000;
			}
			break;
	}
}


READ8_MEMBER(segag80r_state::n7751_rom_r)
{
	/* read from BUS */
	return memregion("n7751")->base()[m_sound_addr];
}


READ8_MEMBER(segag80r_state::n7751_command_r)
{
	/* read from P2 - 8255's PC0-2 connects to 7751's S0-2 (P24-P26 on an 8048) */
	/* bit 0x80 is an alternate way to control the sample on/off; doesn't appear to be used */
	return 0x80 | ((m_n7751_command & 0x07) << 4);
}


static WRITE8_DEVICE_HANDLER( n7751_p2_w )
{
	segag80r_state *state = device->machine().driver_data<segag80r_state>();
	/* write to P2; low 4 bits go to 8243 */
	i8243_p2_w(device, offset, data & 0x0f);

	/* output of bit $80 indicates we are ready (1) or busy (0) */
	/* no other outputs are used */
	state->m_n7751_busy = data >> 7;
}


READ8_MEMBER(segag80r_state::n7751_t1_r)
{
	/* T1 - labelled as "TEST", connected to ground */
	return 0;
}
