/*

    Gottlieb System 80A

*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/i86/i86.h"

extern const char layout_pinball[];

class gts80a_state : public driver_device
{
public:
	gts80a_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }
};

static ADDRESS_MAP_START( gts80a_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x1000, 0x17ff) AM_MIRROR(0xc000) AM_ROM	/* PROM */
	AM_RANGE(0x2000, 0x2fff) AM_MIRROR(0xc000) AM_ROM	/* u2 ROM */
	AM_RANGE(0x3000, 0x3fff) AM_MIRROR(0xc000) AM_ROM	/* u3 ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( caveman_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x1000, 0x17ff) AM_MIRROR(0xc000) AM_ROM	/* PROM */
	AM_RANGE(0x2000, 0x2fff) AM_MIRROR(0xc000) AM_ROM	/* u2 ROM */
	AM_RANGE(0x3000, 0x3fff) AM_MIRROR(0xc000) AM_ROM	/* u3 ROM */
ADDRESS_MAP_END

static INPUT_PORTS_START( gts80a )
INPUT_PORTS_END

static INPUT_PORTS_START( caveman )
INPUT_PORTS_END

static MACHINE_RESET( gts80a )
{
}

static DRIVER_INIT( gts80a )
{
}

static MACHINE_CONFIG_START( gts80a_s, gts80a_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 850000)
	MCFG_CPU_PROGRAM_MAP(gts80a_map)

	MCFG_MACHINE_RESET( gts80a )

	/* related to src/mame/audio/gottlieb.c */
//  MCFG_IMPORT_FROM(gts80s_s)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( gts80a_ss, gts80a_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 850000)
	MCFG_CPU_PROGRAM_MAP(gts80a_map)

	MCFG_MACHINE_RESET( gts80a )

	/* related to src/mame/audio/gottlieb.c */
//  MCFG_IMPORT_FROM(gts80s_ss)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

static ADDRESS_MAP_START( video_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x08000, 0x0ffff) AM_ROM
	AM_RANGE(0xf8000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( video_io_map, ADDRESS_SPACE_IO, 16 )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static MACHINE_CONFIG_START( caveman, gts80a_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 850000)
	MCFG_CPU_PROGRAM_MAP(caveman_map)

	MCFG_MACHINE_RESET( gts80a )

	/* related to src/mame/audio/gottlieb.c */
//  MCFG_IMPORT_FROM(gts80s_ss)

	MCFG_CPU_ADD("video_cpu", I8086, 5000000)
	MCFG_CPU_PROGRAM_MAP(video_map)
	MCFG_CPU_IO_MAP(video_io_map)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_pinball)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Alien Star (#689)
/-------------------------------------------------------------------*/
ROM_START(alienstr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("689.cpu", 0x1000, 0x0800, CRC(4262006b) SHA1(66520b66c31efd0dc654630b2d3567da799b4d89))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("689-s.snd", 0x0800, 0x0800, CRC(e1e7a610) SHA1(d4eddfc970127cf3a7d086ad46cbc7b95fdc269d))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Amazon Hunt (#684)
/-------------------------------------------------------------------*/
ROM_START(amazonh)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("684-2.cpu", 0x1000, 0x0800, CRC(b0d0c4af) SHA1(e81f568983d95cecb62d34598c40c5a5e6dcb3e2))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("684-s1.snd", 0x7000, 0x0800, CRC(86d239df) SHA1(f18efdc6b84d18b1cf01e79224284c5180c57d22))
	ROM_LOAD("684-s2.snd", 0x7800, 0x0800, CRC(4d8ea26c) SHA1(d76d535bf29297247f1e5abd080a52b7dfc3811b))
ROM_END

/*-------------------------------------------------------------------
/ Caveman (#PV-810) Pinball/Video Combo
/-------------------------------------------------------------------*/
ROM_START(caveman)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("pv810-1.cpu", 0x1000, 0x0800, CRC(dd8d516c) SHA1(011d8744a7984ed4c7ceb1f57dcbd8fdb22e21fe))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("pv810-s1.snd", 0x7000, 0x0800, CRC(a491664d) SHA1(45031bcbddb75b4f3a5c3b623a0f2723fb95f92f))
	ROM_LOAD("pv810-s2.snd", 0x7800, 0x0800, CRC(d8654e6e) SHA1(75d4f1f966ed5a1632536723229166b9cc7d77c7))

	ROM_REGION(0x1000000, "video_cpu", 0)
	ROM_LOAD16_BYTE("v810-u8.bin", 0x08000, 0x1000, CRC(514aa152) SHA1(f61a98bbc95f202417cf97b35fe9835108200477))
	ROM_RELOAD( 0xf8000, 0x1000)
	ROM_LOAD16_BYTE("v810-u7.bin", 0x08001, 0x1000, CRC(74c6533e) SHA1(8fe373c28dc4089bd9e573c69682113315236c72))
	ROM_RELOAD( 0xf8001, 0x1000)
	ROM_LOAD16_BYTE("v810-u6.bin", 0x0a000, 0x1000, CRC(2fd0ee95) SHA1(8374b7729b2de9e73784617ada6f9d895f54cc8d))
	ROM_RELOAD( 0xfa000, 0x1000)
	ROM_LOAD16_BYTE("v810-u5.bin", 0x0a001, 0x1000, CRC(2fb15da3) SHA1(ba2927bc88c1ee1b8dd682234b2616d2013c7e7c))
	ROM_RELOAD( 0xfa001, 0x1000)
	ROM_LOAD16_BYTE("v810-u4.bin", 0x0c000, 0x1000, CRC(2dfe8492) SHA1(a29604cda968504f95577e36c715ae97034bb5f8))
	ROM_RELOAD( 0xfc000, 0x1000)
	ROM_LOAD16_BYTE("v810-u3.bin", 0x0c001, 0x1000, CRC(740e9ec3) SHA1(ba4839680694bf5acff540147af4319c64c313e8))
	ROM_RELOAD( 0xfc001, 0x1000)
	ROM_LOAD16_BYTE("v810-u2.bin", 0x0e000, 0x1000, CRC(b793baf9) SHA1(cf1618cd0134529d057bc8245b9b366c3aae2326))
	ROM_RELOAD( 0xfe000, 0x1000)
	ROM_LOAD16_BYTE("v810-u1.bin", 0x0e001, 0x1000, CRC(0a283b15) SHA1(4a57ae5be36500c22b55ac17dc71968bd833298b))
	ROM_RELOAD( 0xfe001, 0x1000)
ROM_END

ROM_START(cavemana)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("pv810-1.cpu", 0x1000, 0x0800, CRC(dd8d516c) SHA1(011d8744a7984ed4c7ceb1f57dcbd8fdb22e21fe))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("pv810-s1.snd", 0x7000, 0x0800, CRC(a491664d) SHA1(45031bcbddb75b4f3a5c3b623a0f2723fb95f92f))
	ROM_LOAD("pv810-s2.snd", 0x7800, 0x0800, CRC(d8654e6e) SHA1(75d4f1f966ed5a1632536723229166b9cc7d77c7))

	ROM_REGION(0x1000000, "video_cpu", 0)
	ROM_LOAD16_BYTE("v810-u8.bin", 0x08000, 0x1000, CRC(514aa152) SHA1(f61a98bbc95f202417cf97b35fe9835108200477))
	ROM_RELOAD( 0xf8000, 0x1000)
	ROM_LOAD16_BYTE("v810-u7.bin", 0x08001, 0x1000, CRC(74c6533e) SHA1(8fe373c28dc4089bd9e573c69682113315236c72))
	ROM_RELOAD( 0xf8001, 0x1000)
	ROM_LOAD16_BYTE("v810-u6.bin", 0x0a000, 0x1000, CRC(2fd0ee95) SHA1(8374b7729b2de9e73784617ada6f9d895f54cc8d))
	ROM_RELOAD( 0xfa000, 0x1000)
	ROM_LOAD16_BYTE("v810-u5.bin", 0x0a001, 0x1000, CRC(2fb15da3) SHA1(ba2927bc88c1ee1b8dd682234b2616d2013c7e7c))
	ROM_RELOAD( 0xfa001, 0x1000)
	ROM_LOAD16_BYTE("v810-u4a.bin", 0x0c000, 0x1000, CRC(3437c697) SHA1(e35822ed04eeb7f8a54a0bfdd2b63d54fa9b2263))
	ROM_RELOAD( 0xfc000, 0x1000)
	ROM_LOAD16_BYTE("v810-u3a.bin", 0x0c001, 0x1000, CRC(729819f6) SHA1(6f684d05d1dcdbb975d3b97cfa0b1d657e7a98a5))
	ROM_RELOAD( 0xfc001, 0x1000)
	ROM_LOAD16_BYTE("v810-u2a.bin", 0x0e000, 0x1000, CRC(ab6193c2) SHA1(eb898b3a3dfef15f992f7ef6f2d636a3e124ca13))
	ROM_RELOAD( 0xfe000, 0x1000)
	ROM_LOAD16_BYTE("v810-u1a.bin", 0x0e001, 0x1000, CRC(7c6410fb) SHA1(6606d853d4955ce18ace71814bd2ae3d25e0c046))
	ROM_RELOAD( 0xfe001, 0x1000)
ROM_END

/*-------------------------------------------------------------------
/ Devil's Dare
/-------------------------------------------------------------------*/
ROM_START(dvlsdre)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("670-1.cpu", 0x1000, 0x0800, CRC(6318bce2) SHA1(1b13a87d18693fe7986fdd79bd00a80d877940c3))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("670-s1.snd", 0x7000, 0x0800, CRC(506bc22a) SHA1(3c69f8d0c38c51796c31fb38c02d00afe8a4b8c5))
	ROM_LOAD("670-s2.snd", 0x7800, 0x0800, CRC(f662ee4b) SHA1(0f63e01672b7c07a4913e150f0bbe07ecfc06e7c))
ROM_END

ROM_START(dvlsdre2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("670-a.cpu", 0x1000, 0x0800, CRC(353b2e18) SHA1(270365ea8276b64e38939f0bf88ddb955d59cd4d))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("670-a-s.snd", 0x0400, 0x0400, CRC(f141d535) SHA1(91e4ab9ce63b5ff3e395b6447a104286327b5533))
	ROM_RELOAD( 0x0800, 0x0400)
	ROM_LOAD("6530sy80.bin", 0x0c00, 0x0400, CRC(c8ba951d) SHA1(e4aa152b36695a0205c19a8914e4d77373f64c6c))
	ROM_RELOAD( 0xfc00, 0x0400)
ROM_END

/*-------------------------------------------------------------------
/ El Dorado City of Gold (#692)
/-------------------------------------------------------------------*/
ROM_START(eldorado)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("692-2.cpu", 0x1000, 0x0800, CRC(4ee6d09b) SHA1(5da0556204e76029380366f9fbb5662715cc3257))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("692-s.snd", 0x0800, 0x0800, CRC(9bfbf400) SHA1(58aed9c0b1f52bcd0b53edcdf7af576bb175e3d6))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Goin' Nuts (#682)
/-------------------------------------------------------------------*/
ROM_START(goinnuts)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("682.cpu", 0x1000, 0x0800, CRC(51c7c6de) SHA1(31accbc8d29038679f2b0396202490233657e538))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("682-s1.snd", 0x7000, 0x0800, CRC(f00dabf3) SHA1(a6e3078220ab23dc41fd48fd528e679aefec3693))
	ROM_LOAD("682-s2.snd", 0x7800, 0x0800, CRC(3be8ac5f) SHA1(0112d3417c0793e672733eff58058d8c9ad10421))
ROM_END

/*-------------------------------------------------------------------
/ Ice Fever (#695)
/-------------------------------------------------------------------*/
ROM_START(icefever)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("695.cpu", 0x1000, 0x0800, CRC(2f6e9caf) SHA1(4f9eeafcbaf758ee6bbad74611b4912ff75b8576))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("695-s.snd", 0x0800, 0x0800, CRC(daededc2) SHA1(b43303c1e39b21f3fcbc339d440ea051ced1ea26))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Jacks To Open (#687)
/-------------------------------------------------------------------*/
ROM_START(jack2opn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("687.cpu", 0x1000, 0x0800, CRC(0080565e) SHA1(c08412ba24d2ffccf11431e80bd2fc95fc4ce02b))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("687-s.snd", 0x0800, 0x0800, CRC(f9d10b7a) SHA1(db255711ed6cb46d183c0ae3894df447f3d8a8e3))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Krull (#676)
/-------------------------------------------------------------------*/
ROM_START(krullp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("676-3.cpu", 0x1000, 0x0800, CRC(71507430) SHA1(cbd7dd186ec928829585d3166ec10956d708d850))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("676-s1.snd", 0x7000, 0x0800, CRC(b1989d8f) SHA1(f1a7eac8aa9c7685f4d37f1c73bba27f4fa8b6ae))
	ROM_LOAD("676-s2.snd", 0x7800, 0x0800, CRC(05fade11) SHA1(538f6225235b5338504597acdf6bafd1de24284e))
ROM_END

/*-------------------------------------------------------------------
/ Punk! (#674)
/-------------------------------------------------------------------*/
ROM_START(punk)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("674.cpu", 0x1000, 0x0800, CRC(70cccc57) SHA1(c2446ecf072174ce3e8524c1a01b1eea72875226))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("674-s1.snd", 0x7000, 0x0800, CRC(b75f79d5) SHA1(921774dacccb025c9465ea7e24534aca2d29d6f1))
	ROM_LOAD("674-s2.snd", 0x7800, 0x0800, CRC(005d123a) SHA1(ebe258786de09488ec0a104a47e208c66b3613b5))
ROM_END

/*-------------------------------------------------------------------
/ Q*Bert's Quest (#677)
/-------------------------------------------------------------------*/
ROM_START(qbquest)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("677.cpu", 0x1000, 0x0800, CRC(fd885874) SHA1(d4414949eca45fd063c4f31079e9fa095044ab9c))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("677-s1.snd", 0x7000, 0x0800, CRC(af7bc8b7) SHA1(33100d63629be7a5b768efd82a1ed1280c845d25))
	ROM_LOAD("677-s2.snd", 0x7800, 0x0800, CRC(820aa26f) SHA1(7181ceedcf61204277d7b9fdba621915960999ad))
ROM_END

/*-------------------------------------------------------------------
/ Rack 'Em Up (#685)
/-------------------------------------------------------------------*/
ROM_START(rackempp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("685.cpu", 0x1000, 0x0800, CRC(4754d68d) SHA1(2af743287c1a021f3e130d3d6e191ec9724d640c))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("685-s.snd", 0x0800, 0x0800, CRC(d4219987) SHA1(7385d8723bdc937e7c9d6bf7f26ca06f64a9a212))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Ready...Aim...Fire! (#686)
/-------------------------------------------------------------------*/
ROM_START(raimfire)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("686.cpu", 0x1000, 0x0800, CRC(d1e7a0de) SHA1(b9af2fcaadc55d37c7d9d22621c3817eb751de6b))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("686-s.snd", 0x0800, 0x0800, CRC(09740682) SHA1(4f36d78207bd5b8e7abb7118f03acbb3885173c2))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Rocky (#672)
/-------------------------------------------------------------------*/
ROM_START(rocky)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("672-2x.cpu", 0x1000, 0x0800, CRC(8e2f0d39) SHA1(eb0982d2bfa910b3c95d6d55c04dc58395789411))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("672-s1.snd", 0x7000, 0x0800, CRC(10ba523c) SHA1(4289acd1437d7bf69fb442884a98290dc1b5f493))
	ROM_LOAD("672-s2.snd", 0x7800, 0x0800, CRC(5e77117a) SHA1(7836b1ee0b2afe621ae414d5710111b550db0e63))
ROM_END

/*-------------------------------------------------------------------
/ Royal Flush Deluxe (#681)
/-------------------------------------------------------------------*/
ROM_START(rflshdlx)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("681-2.cpu", 0x1000, 0x0800, CRC(0b048658) SHA1(c68ce525cbb44194090df17401b220d6a070eccb))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("681-s1.snd", 0x7000, 0x0800, CRC(33455bbd) SHA1(04db645060d93d7d9faff56ead9fa29a9c4723ec))
	ROM_LOAD("681-s2.snd", 0x7800, 0x0800, CRC(639c93f9) SHA1(1623fea6681a009e7a755357fa85206cf2ce6897))
ROM_END

/*-------------------------------------------------------------------
/ Spirit (#673)
/-------------------------------------------------------------------*/
ROM_START(spirit)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("673-2.cpu", 0x1000, 0x0800, CRC(a7dc2207) SHA1(9098e740639af364a12857f89bdc4e2c7c89ff23))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("673-s1.snd", 0x7000, 0x0800, CRC(fd3062ae) SHA1(6eae04ec470afd4363ca448ee106e3e89fbf471e))
	ROM_LOAD("673-s2.snd", 0x7800, 0x0800, CRC(7cf923f1) SHA1(2182324c30e8cb22735e59b74d4f6b268d3750e6))
ROM_END

/*-------------------------------------------------------------------
/ Striker (#675)
/-------------------------------------------------------------------*/
ROM_START(striker)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("675.cpu", 0x1000, 0x0800, CRC(06b66ce8) SHA1(399d98753e2da5c835c629a673069e853a4ce3c3))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("675-s1.snd", 0x7000, 0x0800, CRC(cc11c487) SHA1(fe880dd7dc03f368b2c7ea81059c4b176018b86e))
	ROM_LOAD("675-s2.snd", 0x7800, 0x0800, CRC(ec30a3d9) SHA1(895be373598786d618bed635fe43daae7245c8ac))
ROM_END

/*-------------------------------------------------------------------
/ Super Orbit (#680)
/-------------------------------------------------------------------*/
ROM_START(sorbit)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("680.cpu", 0x1000, 0x0800, CRC(decf84e6) SHA1(0c6f5e1abac58aede15016b5e30db72d1a3f6c11))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("680-s1.snd", 0x7000, 0x0800, CRC(fccbbbdd) SHA1(089f2b15ab1cc46550351614e18d8915b3d6a8bf))
	ROM_LOAD("680-s2.snd", 0x7800, 0x0800, CRC(d883d63d) SHA1(1777a16bc9df7e5be2643ed18754ba120c7a954b))
ROM_END


/*-------------------------------------------------------------------
/ The Games (#691)
/-------------------------------------------------------------------*/
ROM_START(thegames)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("691.cpu", 0x1000, 0x0800, CRC(50f620ea) SHA1(2f997a637eba4eb362586d3aa8caac44acccc795))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("691-s.snd", 0x0800, 0x0800, CRC(d7011a31) SHA1(edf5de6cf5ddc1eb577dd1d8dcc9201522df8315))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/*-------------------------------------------------------------------
/ Touchdown (#688)
/-------------------------------------------------------------------*/
ROM_START(touchdn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2_80a.bin", 0x2000, 0x1000, CRC(241de1d4) SHA1(9d5942704cbdec6565d6335e33e9f7e4c60a41ac))
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0xa000, 0x1000)
	ROM_RELOAD(0xe000, 0x1000)
	ROM_LOAD("u3_80a.bin", 0x3000, 0x1000, CRC(2d77ccdc) SHA1(47241ccd365e8d74d5aa5b775acf6445cc95b8a8))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_RELOAD(0xb000, 0x1000)
	ROM_RELOAD(0xf000, 0x1000)
	ROM_LOAD("688.cpu", 0x1000, 0x0800, CRC(e531ab3f) SHA1(695aef0dd911fee27ac2d1493a9646b5430a07d5))
	ROM_RELOAD(0x5000, 0x0800)
	ROM_RELOAD(0x9000, 0x0800)
	ROM_RELOAD(0xd000, 0x0800)

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("688-s.snd", 0x0800, 0x0800, CRC(5e9988a6) SHA1(5f531491722d3c30cf4a7c17982813a7c548387a))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

/* disp3 */GAME(1984,	alienstr,	0,		gts80a_s,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Alien Star",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1983,	amazonh,	0,		gts80a_ss,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Amazon Hunt",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* cust  */GAME(1981,	dvlsdre,	0,		gts80a_ss,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Devil's Dare",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* cust  */GAME(1981,	dvlsdre2,	0,		gts80a_s,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Devil's Dare (Sound Only)",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1984,	eldorado,	0,		gts80a_s,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"El Dorado City of Gold",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* cust  */GAME(1983,	goinnuts,	0,		gts80a_ss,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Goin' Nuts",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1985,	icefever,	0,		gts80a_s,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Ice Fever",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1984,	jack2opn,	0,		gts80a_s,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Jacks to Open",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* cust  */GAME(1983,	krullp,		0,		gts80a_ss,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Krull (Pinball)",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1982,	punk,		0,		gts80a_ss,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Punk!",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1983,	qbquest,	0,		gts80a_ss,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Q*Bert's Quest",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1983,	rackempp,	0,		gts80a_s,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Rack 'Em Up (Pinball)",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1983,	raimfire,	0,		gts80a_s,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Ready...Aim...Fire!",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* cust  */GAME(1982,	rocky,		0,		gts80a_ss,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Rocky",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1983,	rflshdlx,	0,		gts80a_ss,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Royal Flush Deluxe",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* cust  */GAME(1982,	spirit,		0,		gts80a_ss,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Spirit",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* cust  */GAME(1982,	striker,	0,		gts80a_ss,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Striker",				GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1983,	sorbit,		0,		gts80a_ss,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Super Orbit",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1984,	thegames,	0,		gts80a_s,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"The Games",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
/* disp3 */GAME(1984,	touchdn,	0,		gts80a_s,	gts80a,	gts80a,	ROT0,	"Gottlieb",		"Touchdown",			GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)

/* custom (+video) */
GAME(1981,	caveman,	0,			caveman,	caveman,	gts80a,	ROT0,	"Gottlieb",		"Caveman (Pinball/Video Combo)",		GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
GAME(1981,	cavemana,	caveman,	caveman,	caveman,	gts80a,	ROT0,	"Gottlieb",		"Caveman (Pinball/Video Combo, Set 2)",	GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
