#include "emu.h"
#include "cpu/i4004/i4004.h"

extern const char layout_pinball[];

class flicker_state : public driver_device
{
public:
	flicker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};


static ADDRESS_MAP_START( flicker_rom, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_NOP
	AM_RANGE(0x0000, 0x03FF) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(flicker_map, AS_DATA, 8)
	AM_RANGE(0x0000, 0xffff) AM_NOP
	AM_RANGE(0x0000, 0x00FF) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( flicker_io , AS_IO, 8)
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( flicker )
INPUT_PORTS_END

static MACHINE_RESET( flicker )
{
}

static DRIVER_INIT( flicker )
{
}

static MACHINE_CONFIG_START( flicker, flicker_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I4004, XTAL_5MHz / 8)
	MCFG_CPU_PROGRAM_MAP(flicker_rom)
	MCFG_CPU_DATA_MAP(flicker_map)
	MCFG_CPU_IO_MAP(flicker_io)

	MCFG_MACHINE_RESET( flicker )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

ROM_START(flicker)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("flicker.rom", 0x0000, 0x0400, CRC(c692e586) SHA1(5cabb28a074d18b589b5b8f700c57e1610071c68))
ROM_END

GAME(1974,	flicker,	0,	flicker,	flicker,	flicker,	ROT0,	"Nutting Associates",	"Flicker (Prototype)",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
