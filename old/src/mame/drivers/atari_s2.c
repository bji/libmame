/*
    Atari Generation/System 2
*/
#include "emu.h"
#include "cpu/m6800/m6800.h"

extern const char layout_pinball[];

class atari_s2_state : public driver_device
{
public:
	atari_s2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }
};


static ADDRESS_MAP_START( atari_s2_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM
	AM_RANGE(0x0800, 0x08ff) AM_MIRROR(0x0700) AM_RAM // NVRAM
	AM_RANGE(0x2800, 0x3fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( atari_s2 )
INPUT_PORTS_END

static MACHINE_RESET( atari_s2 )
{
}

static DRIVER_INIT( atari_s2 )
{
}

static MACHINE_CONFIG_START( atari_s2, atari_s2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 1000000)
	MCFG_CPU_PROGRAM_MAP(atari_s2_map)

	MCFG_MACHINE_RESET( atari_s2 )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Superman (03/1979)
/-------------------------------------------------------------------*/
ROM_START(supermap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("supmn_k.rom", 0x2800, 0x0800, CRC(a28091c2) SHA1(9f5e47db408da96a31cb2f3be0fa9fb1e79f8d85))
	ROM_RELOAD(0xa800, 0x0800)
	ROM_LOAD("atari_m.rom", 0x3000, 0x0800, CRC(1bb6b72c) SHA1(dd24ed54de275aadf8dc0810a6af3ac97aea4026))
	ROM_RELOAD(0xb000, 0x0800)
	ROM_LOAD("atari_j.rom", 0x3800, 0x0800, CRC(26521779) SHA1(2cf1c66441aee99b9d01859d495c12025b5ef094))
	ROM_RELOAD(0xb800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Hercules (05/1979)
/-------------------------------------------------------------------*/
ROM_START(hercules)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("herc_k.rom", 0x2800, 0x0800, CRC(65e099b1) SHA1(83a06bc82e0f8f4c0655886c6a9962bb28d00c5e))
	ROM_RELOAD(0xa800, 0x0800)
	ROM_LOAD("atari_m.rom", 0x3000, 0x0800, CRC(1bb6b72c) SHA1(dd24ed54de275aadf8dc0810a6af3ac97aea4026))
	ROM_RELOAD(0xb000, 0x0800)
	ROM_LOAD("atari_j.rom", 0x3800, 0x0800, CRC(26521779) SHA1(2cf1c66441aee99b9d01859d495c12025b5ef094))
	ROM_RELOAD(0xb800, 0x0800)
	ROM_RELOAD(0xf800, 0x0800)
ROM_END

GAME( 1979, supermap, 0, atari_s2, atari_s2, atari_s2, ROT0, "Atari","Superman (Pinball)", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME( 1979, hercules, 0, atari_s2, atari_s2, atari_s2, ROT0, "Atari","Hercules", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
