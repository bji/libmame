/*
  Konami Gambling Games ("Endeavour" hardware)

   unknown Hardware, unknown CPU

  I think they use CF cards for resources, one game has what appears to be a dump of one
  but the rest don't.  It's quite possibly (even likely) that all the sets here are incomplete.

*/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/arm7/arm7.h"
#include "cpu/mips/mips3.h"
#include "cpu/powerpc/ppc.h"

class konendev_state : public driver_device
{
public:
	konendev_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START( konendev_map, AS_PROGRAM, 32, konendev_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM	// main program
ADDRESS_MAP_END


static INPUT_PORTS_START( konendev )
INPUT_PORTS_END


static const powerpc_config ppc603e_config =
{
	XTAL_66_6667MHz		/* Multiplier 1.5, Bus = 66MHz, Core = 100MHz */
};


static MACHINE_CONFIG_START( konendev, konendev_state )
	MCFG_CPU_ADD("maincpu", ARM7_BE, 100000000)
	MCFG_CPU_PROGRAM_MAP(konendev_map)
MACHINE_CONFIG_END


/* Interesting sets */

ROM_START( enchlamp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "enl5rg26_01h.bin", 0x00000, 0x100000, CRC(fed5b988) SHA1(49442decd9b40f0a382c4fc7b231958f526ddbd1) )
	ROM_LOAD32_WORD( "enl5rg26_02l.bin", 0x00002, 0x100000, CRC(d0e42c9f) SHA1(10ff944ec0a626d47ec12be291ff5fe001342ed4) )

	ROM_REGION( 0x1800000, "flash", 0 )
	ROM_LOAD( "enl5r211.fmu.bin", 0x0000, 0x1800000, CRC(592c3c7f) SHA1(119b3c6223d656981c399c399d7edccfdbb50dc7) )
ROM_END


ROM_START( whiterus )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "01h whr5ra26 (c5df)", 0x00000, 0x080000, CRC(d5a1ebb6) SHA1(14a8d1d8f8ae8919eaa878660c7e97e7ea7a02d8) )
	ROM_LOAD32_WORD( "02l whr5ra26 (bc0a)", 0x00002, 0x080000, CRC(48a2277c) SHA1(965d1da31e3bcde6fda4e15e8980a69e8bce5a84) )

	ROM_REGION( 0x200000, "others", 0 )
	ROM_LOAD( "u190.4 2v02s502.ifu_rus (95 7)", 0x0000, 0x080000, CRC(36122a98) SHA1(3d2c40c9d504358d890364e26c9562e40314d8a4) )
	ROM_LOAD( "2v02s502_ifu.bin", 0x0000, 0x080000, CRC(36122a98) SHA1(3d2c40c9d504358d890364e26c9562e40314d8a4) ) // was in 2V02S502_IFU.zip looks similar to above tho
ROM_END

/* Partial sets */

ROM_START( aadvent )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "afa5re26_01h.bin", 0x00000, 0x100000, CRC(65ce6f7a) SHA1(018742f13fea4c52f822e7f12e8efd0aff61a713) )
	ROM_LOAD32_WORD( "afa5re26_02l.bin", 0x00002, 0x100000, CRC(73945b3a) SHA1(5ace9c439048f3555fe631917c15bee76362e784) )
ROM_END

ROM_START( dragnfly )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "drf5re26_01h.bin", 0x00000, 0x100000, CRC(ef6f1b69) SHA1(007a41cd1b08705184f69ce3e0e6c63bc2301e25) )
	ROM_LOAD32_WORD( "drf5re26_02l.bin", 0x00002, 0x100000, CRC(00e00c29) SHA1(a92d7220bf46655222ddc5d1c276dc469343f4c5) )
ROM_END

ROM_START( gypmagic )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "gym5rc26_01h.bin", 0x00000, 0x080000, CRC(8643be94) SHA1(fc63872a55ac2229652566bd9795ce9bf8442fee) )
	ROM_LOAD32_WORD( "gym5rc26_02l.bin", 0x00002, 0x080000, CRC(4ee33c46) SHA1(9e0ef66e9d53a47827d04e6a89d13d37429e0c16) )
ROM_END

ROM_START( incanp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "inp5rg26_01h.bin", 0x00000, 0x100000, CRC(8434222e) SHA1(d03710e18f5b9e45db32685778a21a5dc598d043) )
	ROM_LOAD32_WORD( "inp5rg26_02l.bin", 0x00002, 0x100000, CRC(50c37109) SHA1(a638587f37f63b3f63ee51f541d991c3784c09f7) )
ROM_END

ROM_START( jestmagi )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "jem5rc26_01h.bin", 0x00000, 0x080000, CRC(9145324c) SHA1(366baa22bde1b8da19dba756829305d0fd69b4ff) )
	ROM_LOAD32_WORD( "jem5rc26_02l.bin", 0x00002, 0x080000, CRC(cb49f466) SHA1(e3987de2e640fe8116d66d2c1755e6500dedf8a5) )
ROM_END

ROM_START( luckfoun )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "luf5rd26_01h.bin", 0x00000, 0x080000, CRC(68b3d50a) SHA1(9b3d2a9f5d72db091e79b036017bd5d07f9fed00) )
	ROM_LOAD32_WORD( "luf5rd26_02l.bin", 0x00002, 0x080000, CRC(e7e9b8cd) SHA1(d8c421b0d58775f5a0ccae6395a604091b0acf1d) )
ROM_END

ROM_START( mohicans )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "moh5rf26_01h.bin", 0x00000, 0x100000, CRC(527dda20) SHA1(0a71484421738517c17d76e9bf92943b57cc4cc8) )
	ROM_LOAD32_WORD( "moh5rf26_02l.bin", 0x00002, 0x100000, CRC(a9bd3846) SHA1(02d80ff6c20e3732ae582de5d4392d4d6d8ba955) )
ROM_END

ROM_START( monshow )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "tms5rc26_01h.bin", 0x00000, 0x100000, CRC(8209aafe) SHA1(e48a0524ad93a9b657d3efe67f7b5e1067b37e48) )
	ROM_LOAD32_WORD( "tms5rc26_02l.bin", 0x00002, 0x100000, CRC(78de8c59) SHA1(ad73bc926f5874d257171dfa6b727cb31e33bce9) )
ROM_END

ROM_START( romanl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "rol5rg26_01h.bin", 0x00000, 0x100000, CRC(d441d30c) SHA1(025111699a7e29781bbb4d0f4151c808e3d06235) )
	ROM_LOAD32_WORD( "rol5rg26_02l.bin", 0x00002, 0x100000, CRC(08bd72ca) SHA1(a082cffeb1bccc8ec468a618eaabba7dac89882c) )
ROM_END

ROM_START( safemon )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "sam5rj26_01h.bin", 0x00000, 0x080000, CRC(7f82693f) SHA1(1c8540d209ab17f4fca5ff74bc687c83ec315208) )
	ROM_LOAD32_WORD( "sam5rj26_02l.bin", 0x00002, 0x080000, CRC(73bd981e) SHA1(f01b97201bd877c601cf3c742a6e0963de8e48dc) )
ROM_END

ROM_START( showqn )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "shq_1h.bin", 0x00000, 0x080000, CRC(3fc44415) SHA1(f0be1b90a2a374f9fb9e059e834bbdbf714b6607) )
	ROM_LOAD32_WORD( "shq_2l.bin", 0x00002, 0x080000, CRC(38a03281) SHA1(1b4552b0ce347df4d87e398111bbf72f126a8ec1) )
ROM_END

ROM_START( spiceup )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "siu5rc26_01h.bin", 0x00000, 0x100000, CRC(373bc2b1) SHA1(af3740fdcd028f162440701c952a3a87805bc65b) )
	ROM_LOAD32_WORD( "siu5rc26_02l.bin", 0x00002, 0x100000, CRC(2e584321) SHA1(ca98092dde76338117e989e774db2db672d87bfa) )
ROM_END

ROM_START( sultanw )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "suw5rc26_01h.bin", 0x00000, 0x100000, CRC(27760529) SHA1(b8970a706df52ee5792bbd7a4e719f2be87662ac) )
	ROM_LOAD32_WORD( "suw5rc26_02l.bin", 0x00002, 0x100000, CRC(1c98fd4d) SHA1(58ff948c0deba0bffb8866b15f46518524516501) )
ROM_END

ROM_START( konzero )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_WORD( "rmclr_h.bin", 0x00000, 0x080000, CRC(b9237061) SHA1(0eb311e8e1c872d6a9c38726efb17ddf4713bc7d) )
	ROM_LOAD32_WORD( "rmclr_l.bin", 0x00002, 0x080000, CRC(2806299c) SHA1(a069f4477b310f99ff1ff48f622dc30862589127) )
ROM_END

DRIVER_INIT( konendev )
{
	UINT8 *src = machine.region( "maincpu" )->base();
	size_t  srcsize = machine.region( "maincpu" )->bytes();
	for (int i = 0; i < srcsize; i += 2)
	{
		int temp = src[i];
		src[i] = src[i + 1];
		src[i + 1] = temp;
	}
}

// has a flash dump?
GAME( 200?, enchlamp,   0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Enchanted Lamp (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
// missing flash but has other interesting files
GAME( 200?, whiterus,   0,        konendev,    konendev,    konendev, ROT0,  "Konami", "White Russia (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )

// partial sets
GAME( 200?, aadvent,    0,        konendev,    konendev,    konendev, ROT0,  "Konami", "African Adventure (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, dragnfly,   0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Dragonfly (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, gypmagic,	0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Gypsy Magic (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, incanp,		0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Incan Pyramids (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, jestmagi,	0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Jester Magic (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, luckfoun,	0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Lucky Fountain (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, mohicans,	0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Mohican Sun (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, monshow,	0,        konendev,    konendev,    konendev, ROT0,  "Konami", "The Monster Show (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, romanl,		0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Roman Legions (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, safemon,	0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Safe Money (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, showqn,		0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Show Queen (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, spiceup,	0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Spice It Up (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, sultanw,	0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Sultan's Wish (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 200?, konzero,	0,        konendev,    konendev,    konendev, ROT0,  "Konami", "Zero (Konami Endeavour)", GAME_NOT_WORKING | GAME_NO_SOUND ) // doesn't seem to have a title string in it?
