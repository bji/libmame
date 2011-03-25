/*******************************************************************************************

    Erotictac/Tactic (c) 1990 Sisteme
    Poizone (c) 1991 Eterna

    Actually an Acorn Archimedes-based Arcade system

    original driver by Tomasz Slanina, Steve Ellenoff, Nicola Salmoria
    rewrite to use AA functions by R.Belmont & Angelo Salese
    special thanks to Tom Walker (author of the Acorn Archimedes Arculator emulator)

    TODO (specific issues only):
    - Sound is currently ugly in both games, recognizable but still nowhere near perfection
    - ertictac: 'music' dip-sw makes the game to just hang, BGM doesn't play either for
                whatever reason (should be triggered as soon as it executes the POST)
    - poizone: video timings are off, causing various glitches.
    - Does this Arcade conversion have I2C device? It seems unused afaik.
    - Need PCB for identify the exact model of AA, available RAM, what kind of i/o "podule"
      it has etc.

*******************************************************************************************/
#include "emu.h"
#include "cpu/arm/arm.h"
#include "sound/dac.h"
#include "includes/archimds.h"
#include "machine/i2cmem.h"

static READ32_HANDLER( ertictac_podule_r )
{
	ioc_regs[IRQ_STATUS_B] &= ~ARCHIMEDES_IRQB_PODULE_IRQ;

	switch(offset)
	{
		case 0x04/4: return input_port_read(space->machine, "DSW1") & 0xff;
		case 0x08/4: return input_port_read(space->machine, "DSW2") & 0xff;
		case 0x10/4: return input_port_read(space->machine, "SYSTEM") & 0xff;
		case 0x14/4: return input_port_read(space->machine, "P2") & 0xff;
		case 0x18/4: return input_port_read(space->machine, "P1") & 0xff;
	}

	return 0;
}

static ADDRESS_MAP_START( ertictac_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x01ffffff) AM_READWRITE(archimedes_memc_logical_r, archimedes_memc_logical_w)
	AM_RANGE(0x02000000, 0x02ffffff) AM_RAM AM_BASE(&archimedes_memc_physmem) /* physical RAM - 16 MB for now, should be 512k for the A310 */

	AM_RANGE(0x03340000, 0x0334001f) AM_READ(ertictac_podule_r)
	AM_RANGE(0x033c0000, 0x033c001f) AM_READ(ertictac_podule_r)

	AM_RANGE(0x03000000, 0x033fffff) AM_READWRITE(archimedes_ioc_r, archimedes_ioc_w)
	AM_RANGE(0x03400000, 0x035fffff) AM_READWRITE(archimedes_vidc_r, archimedes_vidc_w)
	AM_RANGE(0x03600000, 0x037fffff) AM_READWRITE(archimedes_memc_r, archimedes_memc_w)
	AM_RANGE(0x03800000, 0x03ffffff) AM_ROM AM_REGION("maincpu", 0) AM_WRITE(archimedes_memc_page_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( ertictac )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00c4, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00c4, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Language ) )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x02, 0x02, "Demo Sound" )	PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Test Mode" )		PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Music" )		PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, "Game Timing" )	PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x30, "Normal Game" )
	PORT_DIPSETTING(    0x20, "3:00" )
	PORT_DIPSETTING(    0x10, "2:30" )
	PORT_DIPSETTING(    0x00, "2:00" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x05, 0x00, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_DIPNAME( 0x0a, 0x00, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("DSW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_4C ) )

	PORT_DIPNAME( 0x10, 0x00, "Sexy Views" )	PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )	PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( poizone )
	PORT_INCLUDE( ertictac )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW1:3" )
	PORT_DIPNAME( 0x30, 0x40, "Coinage 1" ) PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xC0, 0x00, "Coinage 2" ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xC0, DEF_STR( 1C_4C ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Setting 1" ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x01, "Manual" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x1A, 0x00, "Setting 2" )  PORT_DIPLOCATION("DSW2:3,4,5")
	PORT_DIPSETTING(    0x00, "Extremely Easy - 2:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, "Very Easy - 1:30")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x08, "Easy - 2:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x0A, "Normal 1 - 1:30")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, "Normal 2 - 1:45")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x12, "Difficult - 2:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x18, "Very Difficult - 2:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x1A, "Extremely Difficult - 1:30")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "Clear 20% - 1:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x02, "Clear 30% - 1:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x08, "Clear 40% - 1:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x0A, "Clear 50% - 1:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x10, "Clear 60% - 1:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x12, "Clear 70% - 1:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x18, "Clear 80% - 1:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x1A, "Clear 90% - 1:00")	PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW2:6" )
INPUT_PORTS_END

static DRIVER_INIT( ertictac )
{
	archimedes_driver_init(machine);
}

static MACHINE_START( ertictac )
{
	archimedes_init(machine);

	// reset the DAC to centerline
	//dac_signed_data_w(machine->device("dac"), 0x80);
}

static MACHINE_RESET( ertictac )
{
	archimedes_reset(machine);
}

static INTERRUPT_GEN( ertictac_podule_irq )
{
	archimedes_request_irq_b(device->machine, ARCHIMEDES_IRQB_PODULE_IRQ);
}

/* TODO: Are we sure that this HW have I2C device? */
#define	NVRAM_SIZE 256
#define	NVRAM_PAGE_SIZE	0	/* max size of one write request */

static const i2cmem_interface i2cmem_interface =
{
	I2CMEM_SLAVE_ADDRESS, NVRAM_PAGE_SIZE, NVRAM_SIZE
};

static MACHINE_CONFIG_START( ertictac, driver_device )

	MCFG_CPU_ADD("maincpu", ARM, 8000000) /* guess */
	MCFG_CPU_PROGRAM_MAP(ertictac_map)
	MCFG_CPU_PERIODIC_INT(ertictac_podule_irq,60) // FIXME: timing of this

	MCFG_MACHINE_START(ertictac)
	MCFG_MACHINE_RESET(ertictac)

	MCFG_I2CMEM_ADD("i2cmem",i2cmem_interface)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(1280, 1024)
	MCFG_SCREEN_VISIBLE_AREA(0, 1280-1, 0, 1024-1)
	MCFG_SCREEN_UPDATE(archimds_vidc)

	MCFG_PALETTE_LENGTH(0x200)

	MCFG_VIDEO_START(archimds_vidc)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac0", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac3", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac4", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac5", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac6", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_SOUND_ADD("dac7", DAC, 0)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
MACHINE_CONFIG_END

ROM_START( ertictac )
	ROM_REGION(0x800000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "01", 0x00000, 0x10000, CRC(8dce677c) SHA1(9f12b1febe796038caa1ecad1d17864dc546cfd8) )
	ROM_LOAD32_BYTE( "02", 0x00001, 0x10000, CRC(7c5c916c) SHA1(d4ed5fc3a7b27253551e7d9176ed9e6513092e60) )
	ROM_LOAD32_BYTE( "03", 0x00002, 0x10000, CRC(edca5ac6) SHA1(f6c4b8030f3c1c93922c5f7232f2159e0471b93a) )
	ROM_LOAD32_BYTE( "04", 0x00003, 0x10000, CRC(959be81c) SHA1(3e8eaacf8809863fd712ad605d23fdda4e904aee) )
	ROM_LOAD32_BYTE( "05", 0x40000, 0x10000, CRC(d08a6c89) SHA1(17b0f5fb2094126146b09d69c91bf413737b0c9e) )
	ROM_LOAD32_BYTE( "06", 0x40001, 0x10000, CRC(d727bcd8) SHA1(4627f8d4d6e6f323445b3ffcfc0d9c699a9a4f89) )
	ROM_LOAD32_BYTE( "07", 0x40002, 0x10000, CRC(23b75de2) SHA1(e18f5339ca2dd362298784ff3e5479d780d823f8) )
	ROM_LOAD32_BYTE( "08", 0x40003, 0x10000, CRC(9448ccdf) SHA1(75fe3c31625f8ba1eedd806b52993e92b1f585b6) )
	ROM_LOAD32_BYTE( "09", 0x80000, 0x10000, CRC(2bfb312e) SHA1(af7a4a1926c9c3d0b3ad41a4729a17383581c070) )
	ROM_LOAD32_BYTE( "10", 0x80001, 0x10000, CRC(e7a05477) SHA1(0ec6f94a35b87e1e4529dea194fce1fde9a5b0ad) )
	ROM_LOAD32_BYTE( "11", 0x80002, 0x10000, CRC(3722591c) SHA1(e0c4075bc4b1c90a6decba3005a1f8298bd61bd1) )
	ROM_LOAD32_BYTE( "12", 0x80003, 0x10000, CRC(fe022b7e) SHA1(056f7283bc54eff555fd1db91410c020fd905063) )
	ROM_LOAD32_BYTE( "13", 0xc0000, 0x10000, CRC(83550842) SHA1(0fee78dbf13ba970e0544c48f8939550f9347822) )
	ROM_LOAD32_BYTE( "14", 0xc0001, 0x10000, CRC(3029567c) SHA1(6d49bea3a3f6f11f4182a602d37b53f1f896c154) )
	ROM_LOAD32_BYTE( "15", 0xc0002, 0x10000, CRC(500997ab) SHA1(028c7b3ca03141e5b596ab1e2ab98d0ccd9bf93a) )
	ROM_LOAD32_BYTE( "16", 0xc0003, 0x10000, CRC(70a8d136) SHA1(50b11f5701ed5b79a5d59c9a3c7d5b7528e66a4d) )

	ROM_REGION(0x200000, "vram", ROMREGION_ERASE00)
ROM_END

ROM_START( poizone )
	ROM_REGION(0x800000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "p_son01.bin", 0x00000, 0x10000, CRC(28793c9f) SHA1(2d9f7d667203e745b47cd2cc97501ae961ae1a66) )
	ROM_LOAD32_BYTE( "p_son02.bin", 0x00001, 0x10000, CRC(2d4b6f4b) SHA1(8df2680d6e5dc41787b3a72e594f01f5e732d0ec) )
	ROM_LOAD32_BYTE( "p_son03.bin", 0x00002, 0x10000, CRC(0834d46e) SHA1(bf1cc9b47759ef39ed8fd8f334ed8f2902be3bf8) )
	ROM_LOAD32_BYTE( "p_son04.bin", 0x00003, 0x10000, CRC(9e9b1f6e) SHA1(d95067f3ecca3c079a67bd0b80e3b45c5b42151e) )
	ROM_LOAD32_BYTE( "p_son05.bin", 0x40000, 0x10000, CRC(be62ad42) SHA1(5eb51ad277ec7b7f1b5995bcdea35114f805baae) )
	ROM_LOAD32_BYTE( "p_son06.bin", 0x40001, 0x10000, CRC(c2f9141c) SHA1(e910fefcd6f0b99ab299b3a5f099b9ef84e1cc23) )
	ROM_LOAD32_BYTE( "p_son07.bin", 0x40002, 0x10000, CRC(8929c748) SHA1(35c108170590fbe97fdd4a1db7d660b4ee0adac8) )
	ROM_LOAD32_BYTE( "p_son08.bin", 0x40003, 0x10000, CRC(0ef5b14f) SHA1(425f130b2a94a4152fab763e0734e71f2913b25f) )
	ROM_LOAD32_BYTE( "p_son09.bin", 0x80000, 0x10000, CRC(e8cd75a6) SHA1(386a4ff576574e49711e72640dd3f33c8b7e04b3) )
	ROM_LOAD32_BYTE( "p_son10.bin", 0x80001, 0x10000, CRC(1dc01da7) SHA1(d37456e3407cab5eff5bbd9735c3a54e73b27545) )
	ROM_LOAD32_BYTE( "p_son11.bin", 0x80002, 0x10000, CRC(85e973ad) SHA1(850cd0dbda42eab78625038c6ea1f5b31674018a) )
	ROM_LOAD32_BYTE( "p_son12.bin", 0x80003, 0x10000, CRC(b89376d1) SHA1(cff29c2a8db88d4d104bae19a90de034158fe9e7) )

	ROM_LOAD32_BYTE( "p_son21.bin", 0x140000, 0x10000, CRC(a0c06c1e) SHA1(8d065117788e96ecd147d3d7ffdd273d4b69bb7a) )
	ROM_LOAD32_BYTE( "p_son22.bin", 0x140001, 0x10000, CRC(16f0bb52) SHA1(893ab1e72b84de7a38f88f9d713769968ebd4553) )
	ROM_LOAD32_BYTE( "p_son23.bin", 0x140002, 0x10000, CRC(e9c118b2) SHA1(110d9a204e701b9b54d89f027f8892c3f3a819c7) )
	ROM_LOAD32_BYTE( "p_son24.bin", 0x140003, 0x10000, CRC(a09d7f55) SHA1(e0d562c655c16034b40db93de801b98b7948beb2) )

	ROM_REGION(0x200000, "vram", ROMREGION_ERASE00)
ROM_END

GAME( 1990, ertictac, 0, ertictac, ertictac, ertictac, ROT0, "Sisteme", "Erotictac/Tactic" ,GAME_IMPERFECT_SOUND)
GAME( 1991, poizone,  0, ertictac, poizone, ertictac,  ROT0, "Eterna" ,"Poizone" ,GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS)

