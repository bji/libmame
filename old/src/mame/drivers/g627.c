/*
    Midway A084-91313-G627
           A080-91313-G627
           A082-91320-C000

    Only one of it's kind
*/
#include "emu.h"
#include "cpu/z80/z80.h"

extern const char layout_pinball[];

class g627_state : public driver_device
{
public:
	g627_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }
};


static ADDRESS_MAP_START( g627_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static INPUT_PORTS_START( g627 )
INPUT_PORTS_END

static MACHINE_RESET( g627 )
{
}

static DRIVER_INIT( g627 )
{
}

static MACHINE_CONFIG_START( g627, g627_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 14138000/8)
	MCFG_CPU_PROGRAM_MAP(g627_map)

	MCFG_MACHINE_RESET( g627 )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Rotation VIII (09/1978)
/-------------------------------------------------------------------*/
ROM_START(rotation)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rot-a117.dat", 0x0000, 0x0800, CRC(7bb6beb3) SHA1(5ee62246032158c68d426c11a4a9a889ee7655d7))
	ROM_LOAD("rot-b117.dat", 0x0800, 0x0800, CRC(538e37b2) SHA1(d283ac4d0024388b92b6494fcde63957b705bf48))
	ROM_LOAD("rot-c117.dat", 0x1000, 0x0800, CRC(3321ff08) SHA1(d6d94fea27ef58ca648b2829b32d62fcec108c9b))
ROM_END

GAME(1978,	rotation,	0,	g627,	g627,	g627,	ROT0,	"Midway",	"Rotation VIII",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
