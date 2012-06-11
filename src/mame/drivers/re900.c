/***********************************************************************************

    re900.c

    Ruleta RE-900 - Entretenimientos GEMINIS & GENATRON (C) 1993

    Driver by Grull Osgo.
    Additional work by Roberto Fresca.

    Games running on this hardware:

    * Ruleta RE-900,    1993, Entretenimientos GEMINIS.
    * Buena Suerte '94  1994, Entretenimientos GEMINIS.


************************************************************************************

    Hardware Info
    =============

    * Main board:

    1x AMD P80C31BH                 Main CPU.
    1x AY-3-8910                    Sound & I/O System.
    1x TMS-9129-NL                  Video System.

    1x 27C512 EPROM                 Program ROM.
    1x MS6264L-10PC (8Kx8) SRAM     Data Memory (Battery Backed RAM).
    2x TMS4416-15NL (64Kx4) DRAM    Video Memory.

    1x 11.0592 MHz Crystal          CPU clock.
    1x 10.738635 MHz. Crystal       Video System clock.
    1x MAX691CPE                    Power supervisor system, Data RAM Protect and Watchdog.

    1x 3.6 Ni-Cd Battery            Data Memory.
    1x LM380N-8                     1 Channel Audio Amplifier.


************************************************************************************

    Game Info
    =========

    * RE-900 Electronic Roulette.- 6 Players - W/ Random Bonus Multiplier (up to 10x)

    How to play...

    This Roulette allow up to 6 players. To start the machine, turn the Operator Key
    (the the Operator Key light will turn green). Whilest this key is turned ON, you
    can insert credits, play, and payout. Once the key is turned OFF (red light), you
    can play, but credits can't be entered/taken.

    You can select the player number through key "L" (the respective player light will
    lite on). Key-In for all 6 players are keys 1-2-3-4-5-6 and Key-Out are Q-W-E-R-T-Y
    respectively. Up-Down-Left-Right to place cursor, and left CTRL to place a bet.
    After a short time without activity, the roulette start to play, simulating the ball
    with an array of leds...

    We made a full artwork that allow you to play this game with bells and whistles.


    * Buena Suerte ?94 Video Poker Game w/ Double Up feature - 1 Player.

    This game is a reprogrammed version of the Buena Suerte! poker game, to run on this
    GEMINIS RE900 hardware.

    Graphics are worse than original BS, but sounds are improved through the AY-8910.


***********************************************************************************/


#define MAIN_CLOCK		XTAL_11_0592MHz
#define VDP_CLOCK		XTAL_10_730MHz
#define TMS_CLOCK		VDP_CLOCK / 24

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "re900.lh"


class re900_state : public driver_device
{
public:
	re900_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_rom;
	UINT8 m_psg_pa;
	UINT8 m_psg_pb;
	UINT8 m_mux_data;
	UINT8 m_ledant;
	UINT8 m_player;
	UINT8 m_stat_a;
};


/****************
* Read Handlers *
****************/

static READ8_DEVICE_HANDLER (re_psg_portA_r)
{
	if ((input_port_read(device->machine(), "IN0") & 0x01) == 0)
	{
		output_set_lamp_value(0,1);		// Operator Key ON
	}

	else
	{
		output_set_lamp_value(0,0);		// Operator Key OFF
	}

	return input_port_read(device->machine(), "IN0");
}

static READ8_DEVICE_HANDLER (re_psg_portB_r)
{
	re900_state *state = device->machine().driver_data<re900_state>();
	UINT8 retval = 0xff;
	logerror("llamada a re_psg_portB_r\n");
	/* This is a hack to select the active player due to Keyboard size restrictions  */

	output_set_lamp_value(state->m_player,1);

	if (input_port_read(device->machine(), "IN_S"))
	{
		if (!state->m_stat_a)
		{
			output_set_lamp_value(1, 0);
			output_set_lamp_value(2, 0);
			output_set_lamp_value(3, 0);
			output_set_lamp_value(4, 0);
			output_set_lamp_value(5, 0);
			output_set_lamp_value(6, 0);
			state->m_player++;

			if (state->m_player == 7)
			{
				state->m_player = 1;
			}

			output_set_lamp_value(state->m_player, 1); /* It shows active player via layout buttons   */
			state->m_stat_a = 1;
		}
	}

	else
	{
		state->m_stat_a = 0;
	}
	/* End of Select Player Hack */

	/* "INA": Unified port to share the player Keys among all players - Key In & Key Out have their own buttons on keyboard. */
	switch( state->m_mux_data )
	{
		case 0x01: retval = (input_port_read(device->machine(), "IN6") | 0x80 ) - (( state->m_player == 6 ) ? (input_port_read(device->machine(), "INA") | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 6 */
		case 0x02: retval = (input_port_read(device->machine(), "IN5") | 0x80 ) - (( state->m_player == 5 ) ? (input_port_read(device->machine(), "INA") | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 5 */
		case 0x04: retval = (input_port_read(device->machine(), "IN4") | 0x80 ) - (( state->m_player == 4 ) ? (input_port_read(device->machine(), "INA") | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 4 */
		case 0x08: retval = (input_port_read(device->machine(), "IN3") | 0x80 ) - (( state->m_player == 3 ) ? (input_port_read(device->machine(), "INA") | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 3 */
		case 0x10: retval = (input_port_read(device->machine(), "IN2") | 0x80 ) - (( state->m_player == 2 ) ? (input_port_read(device->machine(), "INA") | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 2 */
		case 0x20: retval = (input_port_read(device->machine(), "IN1") | 0x80 ) - (( state->m_player == 1 ) ? (input_port_read(device->machine(), "INA") | 0x80 ) ^ 0xff: 0x00 ); break; /* Player 1 */
	}

	return retval;
}

static READ8_HANDLER (rom_r)
{
	re900_state *state = space->machine().driver_data<re900_state>();
	return state->m_rom[offset];
}


/***********************
*    Write Handlers    *
***********************/

static WRITE8_DEVICE_HANDLER (re_mux_port_A_w)
{
	re900_state *state = device->machine().driver_data<re900_state>();
	state->m_psg_pa = data;
	state->m_mux_data = ((data >> 2) & 0x3f) ^ 0x3f;
}

static WRITE8_DEVICE_HANDLER (re_mux_port_B_w)
{
	re900_state *state = device->machine().driver_data<re900_state>();
	UINT8 led;
	state->m_psg_pb = data;
	led = (state->m_psg_pa >> 2) & 0x3f;

	if (data == 0x7f)
	{
		output_set_lamp_value(20 + led, 1);

		if (led != state->m_ledant)
		{
			output_set_lamp_value(20 + state->m_ledant, 0);
			state->m_ledant = led;
		}
	}
}

static WRITE8_HANDLER (cpu_port_0_w)
{
//  output_set_lamp_value(7,1 ^ ( (data >> 4) & 1)); /* Cont. Sal */
//  output_set_lamp_value(8,1 ^ ( (data >> 5) & 1)); /* Cont. Ent */
}

static WRITE8_HANDLER(re900_watchdog_reset_w)
{
	//watchdog_reset_w(space,0,0); /* To do! */
}


/*******************************
*    Memory Map Information    *
*******************************/

static ADDRESS_MAP_START( mem_prg, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_BASE_MEMBER(re900_state, m_rom)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mem_io, AS_IO, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_READ (rom_r)
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE_MODERN("tms9128", tms9928a_device, vram_write)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE_MODERN("tms9128", tms9928a_device, register_write)
	AM_RANGE(0xe800, 0xe801) AM_DEVWRITE("ay_re900", ay8910_address_data_w)
	AM_RANGE(0xe802, 0xe802) AM_DEVREAD("ay_re900", ay8910_r)
	AM_RANGE(0xe000, 0xefff) AM_WRITE(re900_watchdog_reset_w)
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P0) AM_WRITE(cpu_port_0_w)
	AM_RANGE(MCS51_PORT_P2, MCS51_PORT_P2) AM_NOP
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_NOP
ADDRESS_MAP_END


static WRITE_LINE_DEVICE_HANDLER(vdp_interrupt)
{
	cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE );
}


/************************
*      Input ports      *
************************/

static INPUT_PORTS_START( re900 )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Operator Key") PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Auditor Key")  PORT_TOGGLE PORT_CODE(KEYCODE_9)

	PORT_START("INA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Left")         PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Right")        PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Up")           PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Down")         PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Key-In")   PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Key-Out")  PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P2 Key-In")   PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P2 Key-Out")  PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P3 Key-In")   PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P3 Key-Out")  PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P4 Key-In")   PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P4 Key-Out")  PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P5 Key-In")   PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P5 Key-Out")  PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P6 Key-In")   PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P6 Key-Out")  PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN_S")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Player Select")  PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( bs94 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Double")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Operator") PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Card High")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Card Low")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Auditor") PORT_CODE(KEYCODE_9)
INPUT_PORTS_END


/***************************
*    TMS9928a Interfase    *
***************************/

static TMS9928A_INTERFACE(re900_tms9928a_interface)
{
	"screen",
	0x4000,
	DEVCB_LINE(vdp_interrupt)
};


/*************************
*    AY8910 Interfase    *
*************************/

static const ay8910_interface ay8910_re900 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(re_psg_portA_r),
	DEVCB_HANDLER(re_psg_portB_r),
	DEVCB_HANDLER(re_mux_port_A_w),
	DEVCB_HANDLER(re_mux_port_B_w)
};

static const ay8910_interface ay8910_bs94 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("IN0"),
	DEVCB_INPUT_PORT("IN1"),
	DEVCB_NULL,
	DEVCB_NULL
};

/***************************
*      Machine Driver      *
***************************/

static MACHINE_CONFIG_START( re900, re900_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8051, MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(mem_prg)
	MCFG_CPU_IO_MAP(mem_io)

	/* video hardware */
	MCFG_TMS9928A_ADD( "tms9128", TMS9128, re900_tms9928a_interface )	/* TMS9128NL on pcb */
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9128", tms9128_device, screen_update )

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* sound hardware   */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay_re900", AY8910, TMS_CLOCK) /* From TMS9128NL - Pin 37 (GROMCLK) */
	MCFG_SOUND_CONFIG(ay8910_re900)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bs94, re900 )

	/* sound hardware   */
	MCFG_SOUND_MODIFY("ay_re900")
	MCFG_SOUND_CONFIG(ay8910_bs94)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( re900 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "re900.bin", 0x0000, 0x10000, CRC(967ae944) SHA1(104bab79fd50a8e38ae15058dbe47a59f1ec4b05) )
ROM_END

ROM_START( bs94 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bs94.bin",  0x0000, 0x10000, CRC(bbd484ce) SHA1(4128e488ca806842c3639e05c4c9cf4c0da2990d) )
ROM_END


/************************
*      Driver Init      *
************************/

static DRIVER_INIT( re900 )
{
	re900_state *state = machine.driver_data<re900_state>();
	state->m_player = 1;
	state->m_stat_a = 1;
	state->m_psg_pa = state->m_psg_pb = state->m_mux_data = state->m_ledant = 0;
}


/*************************
*      Game Drivers      *
*************************/

/*     YEAR  NAME   PARENT MACHINE INPUT  INIT   ROT     COMPANY                    FULLNAME            FLAGS LAYOUT */
GAMEL( 1993, re900, 0,     re900,  re900, re900, ROT90, "Entretenimientos GEMINIS", "Ruleta RE-900",    0,    layout_re900)
GAME ( 1994, bs94 , 0,     bs94,   bs94 , re900, ROT0,  "Entretenimientos GEMINIS", "Buena Suerte '94", 0)
