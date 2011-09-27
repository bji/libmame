/*
    Atari Bartop 5200 prototype system

    Driver by Mariusz Wojcieszek
    Based on Atari 400/800 MESS Driver by Juergen Buchmueller

    Hardware was based on Atari 5200 game console with additional coin and timer hardware.
    System was intented to run in barrooms and taverns. Software was based on Atari 5200
    Realsports cartridges.

    Barroom Baseball is modified version of Realsports Baseball.

    Coins and timer were handled by Bartop system bios. Since bios dump is not available,
    regular Atari 5200 bios is used and game is marked as not working. Game cart is also
    marked as bad dump as it is done in "console style", i.e. one file for all game data.
*/

#include "emu.h"
#include "deprecat.h"
#include "cpu/m6502/m6502.h"
#include "includes/atari.h"
#include "sound/speaker.h"
#include "sound/pokey.h"
#include "video/gtia.h"


class bartop52_state : public driver_device
{
public:
	bartop52_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

};


static ADDRESS_MAP_START(a5200_mem, AS_PROGRAM, 8)
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc0ff) AM_READWRITE(atari_gtia_r, atari_gtia_w)
	AM_RANGE(0xd400, 0xd5ff) AM_READWRITE(atari_antic_r, atari_antic_w)
	AM_RANGE(0xe800, 0xe8ff) AM_DEVREADWRITE("pokey", pokey_r, pokey_w)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END

#define JOYSTICK_DELTA			10
#define JOYSTICK_SENSITIVITY	200

static INPUT_PORTS_START(bartop52)
	PORT_START("djoy_b")	/* IN3 digital joystick buttons (GTIA button bits) */
	PORT_BIT(0x01, 0x01, IPT_BUTTON1) PORT_CODE(JOYCODE_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x02, 0x02, IPT_BUTTON1) PORT_CODE(JOYCODE_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x04, 0x04, IPT_UNUSED)
	PORT_BIT(0x08, 0x08, IPT_UNUSED)
	PORT_BIT(0x10, 0x10, IPT_BUTTON2) PORT_CODE(JOYCODE_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, 0x20, IPT_BUTTON2) PORT_CODE(JOYCODE_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, 0x40, IPT_UNUSED)
	PORT_BIT(0x80, 0x80, IPT_UNUSED)

    PORT_START("keypad_0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("(Break)") PORT_CODE(KEYCODE_PAUSE)	// is this correct?
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Wind-Up") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[0]") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("No Wind-Up") PORT_CODE(KEYCODE_PLUS_PAD)
    PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

    PORT_START("keypad_1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Fast Ball Low") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Change-Up Low") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Curve Low") PORT_CODE(KEYCODE_7_PAD)
    PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

    PORT_START("keypad_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Fast Ball Med.") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Change-Up Med.") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Curve Med") PORT_CODE(KEYCODE_4_PAD)
    PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

    PORT_START("keypad_3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START)    PORT_NAME("Start")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Fast Ball High") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Change-Up High") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Curve High") PORT_CODE(KEYCODE_1_PAD)
    PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

    PORT_START("analog_0")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(1)

    PORT_START("analog_1")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(1)

    PORT_START("analog_2")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(2)

    PORT_START("analog_3")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(2)

INPUT_PORTS_END


static const pokey_interface atari_pokey_interface =
{
	{
		DEVCB_INPUT_PORT("analog_0"),
		DEVCB_INPUT_PORT("analog_1"),
		DEVCB_INPUT_PORT("analog_2"),
		DEVCB_INPUT_PORT("analog_3"),
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_NULL,
	},
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	atari_interrupt_cb,
};

static MACHINE_CONFIG_START( a5200, bartop52_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, FREQ_17_EXACT)
	MCFG_CPU_PROGRAM_MAP(a5200_mem)
	MCFG_CPU_VBLANK_INT_HACK(a5200_interrupt, TOTAL_LINES_60HZ)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_VISIBLE_AREA(MIN_X, MAX_X, MIN_Y, MAX_Y)
	MCFG_SCREEN_REFRESH_RATE(FRAME_RATE_60HZ)
	MCFG_SCREEN_SIZE(HWIDTH*8, TOTAL_LINES_60HZ)
	MCFG_SCREEN_UPDATE(atari)

	MCFG_PALETTE_LENGTH(256)
	MCFG_PALETTE_INIT(atari)

	MCFG_VIDEO_START(atari)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("pokey", POKEY, FREQ_17_EXACT)
	MCFG_SOUND_CONFIG(atari_pokey_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

MACHINE_CONFIG_END

ROM_START(barbball)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "barbball.bin", 0x4000, 0x8000, BAD_DUMP CRC(21d19c8f) SHA1(510ccb20df2ecdbe7f8373de6a9fc11493e8c3f2) )
	ROM_LOAD( "5200.rom",     0xf800, 0x0800, BAD_DUMP CRC(4248d3e3) SHA1(6ad7a1e8c9fad486fbec9498cb48bf5bc3adc530) )
ROM_END

GAME( 1983, barbball, 0, a5200, bartop52, 0, ROT0, "Atari", "Barroom Baseball (prototype)", GAME_NOT_WORKING )
