#include "emu.h"
#include "cpu/m6809/m6809.h"

extern const char layout_pinball[];

class wico_state : public driver_device
{
public:
	wico_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};


static ADDRESS_MAP_START( wico_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( wico )
INPUT_PORTS_END

static MACHINE_RESET( wico )
{
}

static DRIVER_INIT( wico )
{
}

static MACHINE_CONFIG_START( wico, wico_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 10000000 / 8)
	MCFG_CPU_PROGRAM_MAP(wico_map)

	MCFG_MACHINE_RESET( wico )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Af-Tor (1984)
/-------------------------------------------------------------------*/
ROM_START(aftor)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u25.bin", 0xf000, 0x1000, CRC(d66e95ff) SHA1(f7e8c51f1b37e7ef560406f1968c12a2043646c5))
	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u52.bin", 0x8000, 0x2000, CRC(8035b446) SHA1(3ec59015e259c315bf09f4e2046f9d98e2d7a732))
	ROM_LOAD("u48.bin", 0xe000, 0x2000, CRC(b4406563) SHA1(6d1a9086eb1f6f947eae3a92ccf7a9b7375d85d3))
ROM_END

/*-------------------------------------------------------------------
/ Big Top  (1977)
/-------------------------------------------------------------------*/

GAME(1984,	aftor,		0,		wico,	wico,	wico,	ROT0,	"Wico",		"Af-Tor",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
