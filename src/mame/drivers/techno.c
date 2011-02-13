/*
    Technoplay "2-2C 8008 LS" (68000 CPU)
*/
#include "emu.h"
#include "cpu/m68000/m68000.h"

extern const char layout_pinball[];

class techno_state : public driver_device
{
public:
	techno_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }
};


static ADDRESS_MAP_START( techno_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static INPUT_PORTS_START( techno )
INPUT_PORTS_END

static MACHINE_RESET( techno )
{
}

static DRIVER_INIT( techno )
{
}

static MACHINE_CONFIG_START( techno, techno_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(techno_map)

	MCFG_MACHINE_RESET( techno )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

ROM_START(xforce)
	ROM_REGION(0x1000000, "maincpu", 0)
	ROM_LOAD16_BYTE("ic15", 0x000001, 0x8000, CRC(fb8d2853) SHA1(0b0004abfe32edfd3ac15d66f90695d264c97eba))
	ROM_LOAD16_BYTE("ic17", 0x000000, 0x8000, CRC(122ef649) SHA1(0b425f81869bc359841377a91c39f44395502bff))
ROM_END

GAME(1987,	xforce,		0,	techno,	techno,	techno,	ROT0,	"Tecnoplay",	"X Force",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
