/******************************************************************************************
  Valley Spectra IV
  -----------------
  Rotating game, like Midway's "Rotation VIII".
*******************************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"

class spectra_state : public driver_device
{
public:
	spectra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};


static ADDRESS_MAP_START( spectra_map, AS_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static INPUT_PORTS_START( spectra )
INPUT_PORTS_END

static MACHINE_RESET( spectra )
{
}

static DRIVER_INIT( spectra )
{
}

static MACHINE_CONFIG_START( spectra, spectra_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 3579545/4)
	MCFG_CPU_PROGRAM_MAP(spectra_map)

	MCFG_MACHINE_RESET( spectra )
MACHINE_CONFIG_END

/*--------------------------------
/ Spectra IV
/-------------------------------*/
ROM_START(spectra)
	ROM_REGION(0x10000, "maincpu", 0)
    ROM_LOAD("spect_u3.dat", 0x0c00, 0x0400, CRC(9ca7510f) SHA1(a87849f16903836158063d593bb4a2e90c7473c8))
    ROM_RELOAD(0xfc00, 0x0400)
    ROM_LOAD("spect_u4.dat", 0x0800, 0x0400, CRC(e6519689) SHA1(06ef3d349ea27a072889b7c379f258d29b7217be) BAD_DUMP)
    ROM_LOAD("spect_u5.dat", 0x0400, 0x0400, CRC(49e0759f) SHA1(c3badc90ff834cbc92d8c519780069310c2b1507))
ROM_END


GAME(1979,	spectra,	0,		spectra,	spectra,	spectra,	ROT0,	"Valley",		"Spectra IV",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
