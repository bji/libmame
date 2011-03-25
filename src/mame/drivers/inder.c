#include "emu.h"
#include "cpu/z80/z80.h"

extern const char layout_pinball[];

class inder_state : public driver_device
{
public:
	inder_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }
};

static ADDRESS_MAP_START( inder_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( inder )
INPUT_PORTS_END

static MACHINE_RESET( inder )
{
}

static DRIVER_INIT( inder )
{
}

static MACHINE_CONFIG_START( inder, inder_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2500000)
	MCFG_CPU_PROGRAM_MAP(inder_map)

	MCFG_MACHINE_RESET( inder )

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ 250 CC (1992)
/-------------------------------------------------------------------*/
ROM_START(ind250cc)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("0-250cc.bin", 0x0000, 0x2000, CRC(753d82ec) SHA1(61950336ba571f9f75f2fc31ccb7beaf4e05dddc))
	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("a-250cc.bin", 0x00000, 0x2000, CRC(b64bdafb) SHA1(eab6d54d34b44187d454c1999e4bcf455183d5a0))
	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("b-250cc.bin", 0x00000, 0x10000, CRC(884c31c8) SHA1(23a838f1f0cb4905fa8552579b5452134f0fc9cc))
	ROM_LOAD("c-250cc.bin", 0x10000, 0x10000, CRC(5a1dfa1d) SHA1(4957431d87be0bb6d27910b718f7b7edcd405fff))
	ROM_LOAD("d-250cc.bin", 0x20000, 0x10000, CRC(a0940387) SHA1(0e06483e3e823bf4673d8e0bd120b0a6b802035d))
	ROM_LOAD("e-250cc.bin", 0x30000, 0x10000, CRC(538b3274) SHA1(eb76c41a60199bb94aec4666222e405bbcc33494))
ROM_END

/*-------------------------------------------------------------------
/ Atleta (1991)
/-------------------------------------------------------------------*/
ROM_START(atleta)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("atleta0.cpu", 0x0000, 0x2000, CRC(5f27240f) SHA1(8b77862fa311d703b3af8a1db17e13b17dca7ec6))
	ROM_LOAD("atleta1.cpu", 0x2000, 0x2000, CRC(12bef582) SHA1(45e1da318141d9228bc91a4e09fff6bf6f194235))
	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("atletaa.snd", 0x00000, 0x2000, CRC(051c5329) SHA1(339115af4a2e3f1f2c31073cbed1842518d5916e))
	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("atletab.snd", 0x0000, 0x10000, CRC(7f155828) SHA1(e459c81b2c2e47d4276344d8d6a08c2c6242f941))
	ROM_LOAD("atletac.snd", 0x10000, 0x10000, CRC(20456363) SHA1(b226400dac35dedc039a7e03cb525c6033b24ebc))
	ROM_LOAD("atletad.snd", 0x20000, 0x10000, CRC(6518e3a4) SHA1(6b1d852005dabb76c7c65b87ecc9ee1422f16737))
	ROM_LOAD("atletae.snd", 0x30000, 0x10000, CRC(1ef7b099) SHA1(08400db3e238baf1673a2da604c999db6be30ffe))
ROM_END

/*-------------------------------------------------------------------
/ Brave Team (1985)
/-------------------------------------------------------------------*/
ROM_START(brvteam)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("brv-tea.m0", 0x0000, 0x1000, CRC(1fa72160) SHA1(0fa779ce2604599adff1e124d0b161b69094a614))
	ROM_LOAD("brv-tea.m1", 0x1000, 0x1000, CRC(4f02ca47) SHA1(68ec7d48c335a1ddd808feaeccac04a4f63d1a33))
ROM_END

/*-------------------------------------------------------------------
/ Canasta '86' (1986)
/-------------------------------------------------------------------*/
ROM_START(canasta)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("c860.bin", 0x0000, 0x1000, CRC(b1f79e52) SHA1(8e9c616f9be19d056da2f86778539d62c0885bac))
	ROM_LOAD("c861.bin", 0x1000, 0x1000, CRC(25ae3994) SHA1(86dcda3278fbe0e57b8ff4858b955d067af414ce))
ROM_END

/*-------------------------------------------------------------------
/ Clown (1988)
/-------------------------------------------------------------------*/
ROM_START(pinclown)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("clown_a.bin", 0x0000, 0x2000, CRC(b7c3f9ab) SHA1(89ede10d9e108089da501b28f53cd7849f791a00))
	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("clown_b.bin", 0x00000, 0x2000, CRC(81a66302) SHA1(3d1243ae878747f20e54cd3322c5a54ded45ce21))
	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("clown_c.bin", 0x00000, 0x10000, CRC(dff89319) SHA1(3745a02c3755d11ea7fb552f7a5df2e8bbee2c29))
	ROM_LOAD("clown_d.bin", 0x10000, 0x10000, CRC(cce4e1dc) SHA1(561c9331d2d110d34cf250cd7b25be16a72a1d79))
	ROM_LOAD("clown_e.bin", 0x20000, 0x10000, CRC(98263526) SHA1(509764e65847637824ba93f7e6ce926501c431ce))
	ROM_LOAD("clown_f.bin", 0x30000, 0x10000, CRC(5f01b531) SHA1(116b1670ef4d5c054bb09dc55aa7d5d3ca047079))
ROM_END

/*-------------------------------------------------------------------
/ Corsario (1989)
/-------------------------------------------------------------------*/
ROM_START(corsario)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("0-corsar.bin", 0x0000, 0x2000, CRC(800f6895) SHA1(a222e7ea959629202686815646fc917ffc5a646c))
	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("a-corsar.bin", 0x00000, 0x2000, CRC(e14b7918) SHA1(5a5fc308b0b70fe041b81071ba4820782b6ff988))
	ROM_REGION(0x40000, "user1", 0)
	ROM_LOAD("b-corsar.bin", 0x00000, 0x10000, CRC(7f155828) SHA1(e459c81b2c2e47d4276344d8d6a08c2c6242f941))
	ROM_LOAD("c-corsar.bin", 0x10000, 0x10000, CRC(047fd722) SHA1(2385507459f85c68141adc7084cb51dfa02462f6))
	ROM_LOAD("d-corsar.bin", 0x20000, 0x10000, CRC(10d8b448) SHA1(ed1918e6c55eba07dde31b9755c9403e073cad98))
	ROM_LOAD("e-corsar.bin", 0x30000, 0x10000, CRC(918ee349) SHA1(17cded8b5626c91e400d26332a160704f2fd2b55))
ROM_END

/*-------------------------------------------------------------------
/ Lap By Lap (1986)
/-------------------------------------------------------------------*/
ROM_START(lapbylap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lblr0.bin", 0x0000, 0x1000, CRC(2970f31a) SHA1(01fb774de19944bb3a19577921f84ab5b6746afb))
	ROM_LOAD("lblr1.bin", 0x1000, 0x1000, CRC(94787c10) SHA1(f2a5b07e57222ee811982eb220c239e34a358d6f))
	ROM_REGION(0x10000, "soundcpu", 0)
	ROM_LOAD("lblsr0.bin", 0x00000, 0x2000, CRC(cbaddf02) SHA1(8207eebc414d90328bfd521190d508b88bb870a2))
ROM_END

GAME(1992,	ind250cc,	0,		inder,	inder,	inder,	ROT0,	"Inder",		"250 CC",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1991,	atleta,		0,		inder,	inder,	inder,	ROT0,	"Inder",		"Atleta",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1985,	brvteam,	0,		inder,	inder,	inder,	ROT0,	"Inder",		"Brave Team",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1986,	canasta,	0,		inder,	inder,	inder,	ROT0,	"Inder",		"Canasta '86'",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1988,	pinclown,	0,		inder,	inder,	inder,	ROT0,	"Inder",		"Clown (Inder)",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1989,	corsario,	0,		inder,	inder,	inder,	ROT0,	"Inder",		"Corsario",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1986,	lapbylap,	0,		inder,	inder,	inder,	ROT0,	"Inder",		"Lap By Lap",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
