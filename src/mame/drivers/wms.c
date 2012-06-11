/*

WMS Russian Fruit Machines (Mechanical?)

x86 based

*/


#include "emu.h"
#include "cpu/i386/i386.h"

class wms_state : public driver_device
{
public:
	wms_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START( wms_map, AS_PROGRAM, 32, wms_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM AM_REGION("maincpu", 0 )

	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("maincpu", 0 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( wms_io, AS_IO, 32, wms_state )
ADDRESS_MAP_END


static INPUT_PORTS_START( wms )
INPUT_PORTS_END



static MACHINE_CONFIG_START( wms, wms_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I486, 40000000 ) // ??
	MCFG_CPU_PROGRAM_MAP(wms_map)
	MCFG_CPU_IO_MAP(wms_io)
MACHINE_CONFIG_END

ROM_START( wms )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "10.bin",      0x0e0000, 0x020000, CRC(cf901f7d) SHA1(009a28fede06d2ff7f476ff643bf27cddd2adbab) )
	ROM_REGION(0x100000, "rom", ROMREGION_ERASE00)
ROM_END

ROM_START( wmsa )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "10cver4.010", 0x0e0000, 0x020000, CRC(fd310b97) SHA1(5745549258a1cefec4b3dddbe9d9a0d6281278e9) )
	ROM_REGION(0x100000, "rom", ROMREGION_ERASE00)
ROM_END


ROM_START( wmsb )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "50cver4.010", 0x0e0000, 0x020000, CRC(eeeeab29) SHA1(898c05c0674a9978caaad4a0fe3650a9d9a56715) )
	ROM_REGION(0x100000, "rom", ROMREGION_ERASE00)
ROM_END


ROM_START( btippers )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(96e362e3) SHA1(a0c35e9aa6bcbc5ffbf8750fa728294ef1e21b02) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(5468a57c) SHA1(3cb87c288bef1782b086a9d6d17f5c3a04aca3c8) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(460ce5b6) SHA1(a4e22fff508db1e36e30ce0ec2c4fefaee67dcfc) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(442ed657) SHA1(e4d33c85c22c44908a016521af53fc234a836b63) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(2d7a9a0e) SHA1(0ab5752ca3bf360180caec219b7bfd478bb09cf4) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(5d767b66) SHA1(fb0866408657db540b85641ad5624885d7ef58ef) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(d4f533a9) SHA1(5ec53fed535fe6409481f99561c13e1fb98385ed) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(c845e18a) SHA1(3e20fbf6ac127a780a7a1517347e3cf7e951e5eb) )
ROM_END

ROM_START( wmsboom )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(5008854d) SHA1(8d9d11775b6cbdef1c71683c4d92e64af26e8939) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(b1fc3e98) SHA1(372aab282905f1fe5781a87f2791d34c93aa0492) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x080000, CRC(ba1b2ab6) SHA1(087a360c1260484ad3bc0b2601003da9581a92b5) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x080000, CRC(3ba6c20d) SHA1(2324a7bc83f695541bbf1a66c0559fea30f3d007) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x080000, CRC(948e8104) SHA1(04bffd1bb2dc9b96550424e8be64a75907b4cbe4) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x080000, CRC(68187707) SHA1(9a85fe5737ae372999f2bfaf50263c00fc9b22f4) )
ROM_END

ROM_START( cashcrop )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(e1918f25) SHA1(55c33c1f604a44caef65e712e69d21792161dfbc) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(22eb1718) SHA1(cc99997d446efd9df99a3e4f04589d1a47d2b638) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(4ce5b630) SHA1(84495ea66d7636a956324f7e8d334fd6aa74724f) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(3073a171) SHA1(7758500fcf8a8c0c43c464644de885a95b8ae152) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(7090eefd) SHA1(f0bc2ab2da956ab3921774c839cab065a59e1daa) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(a8b01689) SHA1(15916b7f98a49dac848a6010f96d9ccac4a9e8f2) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(5fc10445) SHA1(722bf476d0efc6fc77395f2d2cc3913f61a137ac) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(17cdabce) SHA1(d54e6d1697ccd424922c712f4a1f75e8d5f44288) )
ROM_END

ROM_START( filthyr )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(4b57aa15) SHA1(076ef6c1782a0ec34c15bffd5e93644ba179c1f7) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(12dfb80c) SHA1(b7cff3b6dd76102ae3b7cea51da97af9578088ef) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x080000, CRC(86b7c3b4) SHA1(d2c555aa88d141fbd17a5473631adec258b38fe2) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x080000, CRC(12d4e1f2) SHA1(f6317e39a0dd0ce7248f4e1c6a0d69f2d01eb613) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x080000, CRC(d4ce89cb) SHA1(63ddfc5a7ab4f271ebe6ef650832c3b582004acf) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x080000, CRC(fb0de283) SHA1(e1df32787a6fcf597041923d977eacbe5744b770) )
ROM_END

ROM_START( hottop )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, NO_DUMP ) // missing?

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(15ebe9a4) SHA1(3852bf9bd0976abe46102b2c237a513f6447bea6) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(8cab6576) SHA1(be1dc406cbf4fd4035f4e123c4095a0a07350035) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(6ac44a87) SHA1(37d2c07845804b5ac472a6f443d82f3c13d02d04) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(c52b0eba) SHA1(2d69b69a403619f92c35e05e90ce5a76e1cc50a1) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(371f075f) SHA1(ffebaf9d36a0b3b2b726e39b51c456cabeb6e2c4) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(bee1eae3) SHA1(e18b10e7c6dafd9df8b34ee19ce74a82b8adcec4) )
ROM_END

ROM_START( inwinner )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(f5c26579) SHA1(4ef22c0115417320ef17f0c3fc7550db24bdcb8c) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(93082de5) SHA1(5469cd5dfc2a54707cf08feae1e09d808efabf6c) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(d6d3e635) SHA1(90ac66c26fe807992117eab42e12006d9c491ffb) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(6e0c0bc9) SHA1(3f518c507a75bd3d48a3387c80936ff713a4a05d) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(0fff871b) SHA1(703cbdcbe20988d1422c89cde7c63f387af6de3c) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(8a42ac41) SHA1(3e56e369938d4789d4677f8ad0a7c0e1eb38942d) )
ROM_END



ROM_START( jptparty )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(750bfd36) SHA1(161c98455cb2ceb1d7d1a35941b5bb0784129b25) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(78c8bfcd) SHA1(a6ce68a88bcd3c74bffcc852d6f91b6f0f077a91) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(969ac077) SHA1(30207da5f9c3e1fc2a1f4d1bc66fcd2bfc760a8e) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(c76a100e) SHA1(e29b65a92fd8254f577852acfefbec40840e4962) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(53001015) SHA1(ce08db51f812a71db93026bb9d9c33e705020860) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(4f34c9be) SHA1(ef1ad53002c5232fd64715add8f5646c7c6499d6) )
ROM_END


ROM_START( leprgld )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(0985d6b6) SHA1(291c124a9ae813942b14620789370b4a7b9cf85e) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(e29c5127) SHA1(4afe22cd59fa8104b2920b5b3aca9d23ae55c83a) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(8e046831) SHA1(b16b707962c70ef71c11b9c181bdc2335fdf1f51) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(1dbcfdb8) SHA1(652e2939a87e4a68bdb9af405274cb49a7dbed00) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(6a41056f) SHA1(54168a0115dc8d232100fbbd440063dc75fdbaaf) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(0485e432) SHA1(c57a47fd27a14a2ea01400515f61be58be5fa391) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(10bfcdcf) SHA1(34dc619f33c81014120d9b6a2f8b2373451af2e4) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(a3f82467) SHA1(0971f32bff0d4bda43cd98e975e49710d73f9286) )
ROM_END



ROM_START( lol )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(28af87a0) SHA1(c94b98b6922b5f5551eff239226bfe007535fc21) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(58bb1a7c) SHA1(9b93b8d51c0d2895a0926cef6875ca9e985f3193) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(1782c8f4) SHA1(35930b3a1a3a00e7c4e2bfafeb482425a875aa18) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(defafc56) SHA1(4faa41cdf2af52a2dfe8342f0238e13608676cc5) )
ROM_END


ROM_START( lovewin )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(b1b2c7da) SHA1(96430bfa062467bba1daaf2e6180068b3211a739) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(9cdff164) SHA1(58d8692476e73db12674ed986f8ce17183c687e9) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(e0c44410) SHA1(2a95a3ee68cf9c19d6427379d3754506d8a89e88) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(75fd9131) SHA1(db6e6099ccc1c5bca3953a2322eaf154d61dea2b) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(fda225f2) SHA1(757d9118657fa6e033e1449ed5cae25079eb2616) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(c923d180) SHA1(358383458f04950c346a1af07b0c88a3dcd90bf4) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(a2b19335) SHA1(f078582d0be2c79e973b2f1472f1288ac1e73ae1) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(e003b31c) SHA1(41e7fb83041a86a1f6b5d73eb5310739584aeaeb) )
ROM_END



ROM_START( mtburn )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(a329aff0) SHA1(2f631783bd15b8a695d92d0861e167130f914046) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(a0f63408) SHA1(5740ecf6413208c468cdfb84e36887a7ca01262f) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(dc238bf3) SHA1(f649d20fa7b6ce9ee3c9ff392e78ddd66dff8638) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(36b773ac) SHA1(1ce30518da56989ceae3389988b671d9e319c66e) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(20c28e90) SHA1(54ffb735bea5b68513eb6831a749cd3dc94af15f) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(336e2b23) SHA1(ee47d41c95e418f7c2c647b5f6e47e7bb203c50e) )
ROM_END



ROM_START( otchart )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(b1baee77) SHA1(9f8e87bb10499fa73d961bce4fd75d31c880ea51) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(a329aff0) SHA1(2f631783bd15b8a695d92d0861e167130f914046) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(3b2ebe91) SHA1(c2576b303242f75489b96ba90a24d8ad737b5064) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(f0e69a31) SHA1(82552972b8769ec836c3e84110fa6ed2efdcf970) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(d9ab5d90) SHA1(880dab563268ac230c61647bb1ced666606e0f80) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(1f875649) SHA1(4e8e1f860d01f72539f3965c730cc69c3789f898) )
ROM_END



ROM_START( perfect )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(1e90dc52) SHA1(6ac1762ce979e1259e03025d89271034565d8f4a) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(f6f09482) SHA1(55d9395d473ccafbb655bc852a0ea8e9b11957ae) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(9bfc079f) SHA1(99dda328e5796b5e8fd1f43251bbfa5573d8991e) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(f9cf7ee6) SHA1(da74df8fdd1312e1a2dbaa74bb9bc8d97b3fc08e) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(7c04579d) SHA1(6aed1dd61efb58c551db1eccde21935ff5fb517e) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(beb1992e) SHA1(94a6d1179a8a9755cb8c3469f9dff81551e3a845) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(af9270a5) SHA1(e5ab2039d02749a0847f1712d2e0d610a5146c60) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(03fe1d5f) SHA1(87c7b8d77b4654bc938440516035a63704cf108d) )
ROM_END


ROM_START( reelemin )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(a3840965) SHA1(1e782bb5eaeabce0d1a07eb895d344028364cd4f) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(04658d90) SHA1(1180b2596175394e079c8498bfc5e956426d0e1c) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(742590b2) SHA1(65df9d346b1f69f79e6e79c91724813f052c64ee) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(bf3aabb8) SHA1(251c1bcf28f117152902b37915c41be1e912dda0) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(f99bf909) SHA1(4dac7aac0cf815d0b67f4c8d55d84f529e45f27d) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(1e4b64f1) SHA1(f082b2a7b6e029ffded1eec6fbc5f755bdd8d9eb) )
ROM_END


ROM_START( sonoth )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(a5afc1dd) SHA1(5743c630d382d1cc2f7720a2038b105f0dd49790) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(66aff9dd) SHA1(d5eced91421957f9ff5929a3bed10a81d52916b9) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(57e139d7) SHA1(7973ab1cbfcd23b8d812ca843c1e8ebfdd17599b) )
ROM_END


ROM_START( swingin )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(f0ee2385) SHA1(8269cf0a765228fd8a2512cefde715e3a7219d7a) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(7857764b) SHA1(07eeb9f134ee52a03bda72f7df037a81b32a70a4) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(fed9c637) SHA1(64dd3f53f844ce3be69f941c7b7a9f6804f4181f) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(fb56775b) SHA1(283e8c5abd3e8d7c45aededa85182b60aed781d8) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(e3dc27f5) SHA1(0ef43b4a87c624fd8e008b4346065de1731b8c9b) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(85da08d9) SHA1(cd60b524d353be2094e403263ea7d3d1d9ded472) )
ROM_END


ROM_START( wmstopb )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(dbbabd02) SHA1(6e85bb11222c593a326e282e6edfff3647379c78) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(74212101) SHA1(d8e25de334e451a56e9a7cd0f9eca8d1bea4b6b4) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(44e7a0ae) SHA1(8558ec01b3d3c96a4a47869d4e69cbe66ee1b804) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(7b034ad8) SHA1(55d2a5ee42dcf06557ce7ff40977393c70a00123) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(60f6f2fd) SHA1(5d3dd6f5a2b10b45d3281b49608cd24a110e0541) )
ROM_END


ROM_START( wdun )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(9c5f2a50) SHA1(8c591eb80276d460dd66bdf817778aed11d013ef) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(e272e5c5) SHA1(a51c723d51e0632a192597d2ab8ec09bcf1e54e7) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(9a5c34c7) SHA1(680c504868d6850df1f450c0c4f5345c112f7151) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(fa942bad) SHA1(faac6fd68c3ec7d3da564e9bb19ebb2210ad275f) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(fd98076e) SHA1(7a19e9c66df099ac1315efa62575c4d53a7f4582) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(59aaceb0) SHA1(2f7506b634291cc1f7654115fe6be7d34ba16c8c) )
ROM_END


ROM_START( winbid )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(873eecfb) SHA1(35960c9a2a4efeac8b8cea94f1eac6ffb175b9a9) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(628d4bc6) SHA1(7f812addacd5e93843e2a4dc852fbc543dbc4cec) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(0882d890) SHA1(23b388477078c6a1b2756eb3cacb4c146109bb90) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(6dd36b10) SHA1(d595b9cf4f26e55f6aa1cdf18482d9808cf923bd) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(711f6064) SHA1(4dfd12df5594dda46754b12cf6e8399da25051b9) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(74d7272a) SHA1(127fd29a0edb86a2c893a50d4da3cd961673c8b4) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(2ad34442) SHA1(a20b56c63d4b7a41ff9bb44aadca9fd297276b58) )
ROM_END


ROM_START( wldstrek )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(6200c5e3) SHA1(fcf542875a11972974a9c72e171d4ca0686b0c0e) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(d194924a) SHA1(2880ddf2950bb9a4d325e4f13fdad91b3978b7b7) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(6023bc00) SHA1(b53ac52bc4d323168d081aece520743f03833dd6) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(d2f34426) SHA1(14174a6babba97d17dddd54de14ef5cffead79d1) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(f0865c98) SHA1(c93d256388357bc782caaba2e1cc917e248b640d) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(49917f88) SHA1(e21ba71fbce2e52e5a65104be4f7624e1b38f16e) )
ROM_END


ROM_START( yukongld )
	ROM_REGION32_LE(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(7c12f7c7) SHA1(20eb1a5ff85e0fd36e669c7dc53b677f12446af3) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(a1330976) SHA1(d1307f2bd2f4d407b38557507ecbf94e3a5b43d1) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(e5d722cd) SHA1(eacbd9bac5f1f931ae951f3f746851ac3237442a) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(cae8cb45) SHA1(64647f1392e7ac870058461cb9f128ac61182906) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(0c4fd3a0) SHA1(6371cb9d108be20d717a92c26a87666b6bb676fc) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(4d30c67c) SHA1(feda6b0e09bd39e871685c89a6566986012c4099) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(56f03f5d) SHA1(93c5a7698dd10a97f1f386c27560a816a779beb1) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(5978592d) SHA1(7716070894e2b201e37a290b3a8bcf44ce54ea6c) )
ROM_END



static DRIVER_INIT(wms)
{

}

GAME( 200?, wms,	    0,		 wms, wms, wms, ROT0, "WMS", "WMS SetUp/Clear Chips (set 1)", GAME_IS_SKELETON )
GAME( 200?, wmsa,	    wms,	 wms, wms, wms, ROT0, "WMS", "WMS SetUp/Clear Chips (set 2)", GAME_IS_SKELETON )
GAME( 200?, wmsb,	    wms,	 wms, wms, wms, ROT0, "WMS", "WMS SetUp/Clear Chips (set 3)", GAME_IS_SKELETON )

GAME( 200?, btippers,   0,		 wms, wms, wms, ROT0, "WMS", "Big Tippers (Russia)", GAME_IS_SKELETON )
GAME( 200?, wmsboom,    0,		 wms, wms, wms, ROT0, "WMS", "Boom (Russia)", GAME_IS_SKELETON )
GAME( 200?, cashcrop,   0,		 wms, wms, wms, ROT0, "WMS", "Cash Crop (Russia)", GAME_IS_SKELETON )
GAME( 200?, filthyr,	0,		 wms, wms, wms, ROT0, "WMS", "Filthy Rich (Russia)", GAME_IS_SKELETON )
GAME( 200?, hottop,		0,		 wms, wms, wms, ROT0, "WMS", "Hot Toppings (Russia)", GAME_IS_SKELETON )
GAME( 200?, inwinner,	0,		 wms, wms, wms, ROT0, "WMS", "Instant Winner (Russia)", GAME_IS_SKELETON )
GAME( 200?, jptparty,	0,		 wms, wms, wms, ROT0, "WMS", "Jackpot Party (Russia)", GAME_IS_SKELETON )
GAME( 200?, leprgld,	0,		 wms, wms, wms, ROT0, "WMS", "Leprechaun's Gold (Russia)", GAME_IS_SKELETON )
GAME( 200?, lol,		0,		 wms, wms, wms, ROT0, "WMS", "Life of Luxury (Russia)", GAME_IS_SKELETON )
GAME( 200?, lovewin,	0,		 wms, wms, wms, ROT0, "WMS", "Love To Win (Russia)", GAME_IS_SKELETON )
GAME( 200?, mtburn,		0,		 wms, wms, wms, ROT0, "WMS", "Money To Burn (Russia)", GAME_IS_SKELETON )
GAME( 200?, otchart,	0,		 wms, wms, wms, ROT0, "WMS", "Off The Charts (Russia)", GAME_IS_SKELETON )
GAME( 200?, perfect,	0,		 wms, wms, wms, ROT0, "WMS", "Perfect Game (Russia)", GAME_IS_SKELETON )
GAME( 200?, reelemin,	0,		 wms, wms, wms, ROT0, "WMS", "Reel 'Em In (Russia)", GAME_IS_SKELETON )
GAME( 200?, sonoth,		0,		 wms, wms, wms, ROT0, "WMS", "Something For Nothing (Russia)", GAME_IS_SKELETON )
GAME( 200?, swingin,	0,		 wms, wms, wms, ROT0, "WMS", "Swingin In The Green (Russia)", GAME_IS_SKELETON )
GAME( 200?, wmstopb,	0,		 wms, wms, wms, ROT0, "WMS", "Top Banana (Russia)", GAME_IS_SKELETON )
GAME( 200?, wdun,		0,		 wms, wms, wms, ROT0, "WMS", "Who Dunnit (Russia)", GAME_IS_SKELETON )
GAME( 200?, winbid,		0,		 wms, wms, wms, ROT0, "WMS", "Winning Bid (Russia)", GAME_IS_SKELETON )
GAME( 200?, wldstrek,	0,		 wms, wms, wms, ROT0, "WMS", "Wild Streak (Russia)", GAME_IS_SKELETON )
GAME( 200?, yukongld,	0,		 wms, wms, wms, ROT0, "WMS", "Yukon Gold (Russia)", GAME_IS_SKELETON )

