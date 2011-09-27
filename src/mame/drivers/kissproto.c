/*
    Bally Kiss 8035 prototype
*/
#include "emu.h"
#include "cpu/mcs48/mcs48.h"

extern const char layout_pinball[];

class kissp_state : public driver_device
{
public:
	kissp_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }
};


static ADDRESS_MAP_START( kissp_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( kissp )
INPUT_PORTS_END

static MACHINE_RESET( kissp )
{
}

static DRIVER_INIT( kissp )
{
}

static MACHINE_CONFIG_START( kissp, kissp_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8035, 6000000/15)
	MCFG_CPU_PROGRAM_MAP(kissp_map)

	MCFG_MACHINE_RESET( kissp )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

ROM_START(kissp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "kiss8755.bin", 0x4000, 0x0800, CRC(894c1052) SHA1(579ce3c8ec374f2cd17928ab92311f035ecee341))
	ROM_RELOAD( 0x0000, 0x0800)
	ROM_LOAD( "kissprot.u5", 0x1000, 0x1000, CRC(38a2ef5a) SHA1(4ffdb2e9aa30417d506af3bc4b6835ba1dc80e4f))
	ROM_LOAD( "kissprot.u6", 0x2000, 0x1000, CRC(bcdfaf1d) SHA1(d21bebbf702b400eb71f8c88be50a180a5ac260a))
	ROM_LOAD( "kissprot.u7", 0x3000, 0x0800, CRC(d97da1d3) SHA1(da771a08969a12105c7adc9f9e3cbd1677971e79))
	ROM_RELOAD( 0x4800, 0x0800)
ROM_END

GAME( 1979, kissp,	  kiss,		kissp, kissp, kissp, ROT0, "Bally","Kiss (prototype)", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)

