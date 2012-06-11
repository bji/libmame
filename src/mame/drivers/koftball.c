/*
King Of Football (c)1995 BMC

preliminary driver by Tomasz Slanina

--

MC68000P10
M28 (OKI 6295, next to rom C9)
BMC ADB40817(80 Pin PQFP - google hits, but no datasheet or description)
RAMDAC TRC1710-80PCA (Monolithic 256-word by 18bit Look-up Table & Triple Video DAC with 6-bit DACs)
File 89C67 (MCU?? Next to 3.57954MHz OSC)
OSC: 21.47727MHz & 3.57954MHz
2 8-way dipswitchs
part # scratched 64 pin PLCC (soccer ball sticker over this chip ;-)

ft5_v16_c5.u14 \
ft5_v16_c6.u15 | 68000 program code

ft5_v6_c9.u21 - Sound samples

ft5_v6_c1.u59 \
ft5_v6_c2.u60 | Graphics
ft5_v6_c3.u61 |
ft5_v6_c4.u58 /

*/

#define NVRAM_HACK 1

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"


class koftball_state : public driver_device
{
public:
	koftball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_main_ram(*this, "main_ram"),
		m_bmc_1_videoram(*this, "bmc_1_videoram"),
		m_bmc_2_videoram(*this, "bmc_2_videoram"){ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT16> m_main_ram;
	required_shared_ptr<UINT16> m_bmc_1_videoram;
	required_shared_ptr<UINT16> m_bmc_2_videoram;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;
	UINT8 *m_bmc_colorram;
	int m_clr_offset;
	UINT16 m_prot_data;

	DECLARE_WRITE16_MEMBER(bmc_RAMDAC_offset_w);
	DECLARE_WRITE16_MEMBER(bmc_RAMDAC_color_w);
	DECLARE_READ16_MEMBER(bmc_RAMDAC_color_r);
	DECLARE_READ16_MEMBER(random_number_r);
	DECLARE_READ16_MEMBER(prot_r);
	DECLARE_WRITE16_MEMBER(prot_w);
	DECLARE_WRITE16_MEMBER(bmc_1_videoram_w);
	DECLARE_WRITE16_MEMBER(bmc_2_videoram_w);
};


static TILE_GET_INFO( get_t1_tile_info )
{
	koftball_state *state = machine.driver_data<koftball_state>();
	int data = state->m_bmc_1_videoram[tile_index];
	SET_TILE_INFO(
			0,
			data,
			0,
			0);
}

static TILE_GET_INFO( get_t2_tile_info )
{
	koftball_state *state = machine.driver_data<koftball_state>();
	int data = state->m_bmc_2_videoram[tile_index];
	SET_TILE_INFO(
			0,
			data,
			0,
			0);
}

static VIDEO_START( koftball )
{
	koftball_state *state = machine.driver_data<koftball_state>();
	state->m_tilemap_1 = tilemap_create(machine, get_t1_tile_info,tilemap_scan_rows,8,8,64,32);
	state->m_tilemap_2 = tilemap_create(machine, get_t2_tile_info,tilemap_scan_rows,8,8,64,32);

	state->m_tilemap_1->set_transparent_pen(0);
}

static SCREEN_UPDATE_IND16( koftball )
{
	koftball_state *state = screen.machine().driver_data<koftball_state>();
	state->m_tilemap_2->draw(bitmap, cliprect, 0, 0);
	state->m_tilemap_1->draw(bitmap, cliprect, 0, 0);
	return 0;
}

WRITE16_MEMBER(koftball_state::bmc_RAMDAC_offset_w)
{
	m_clr_offset=data*3;
}

WRITE16_MEMBER(koftball_state::bmc_RAMDAC_color_w)
{
	m_bmc_colorram[m_clr_offset]=data;
	palette_set_color_rgb(machine(),m_clr_offset/3,pal6bit(m_bmc_colorram[(m_clr_offset/3)*3]),pal6bit(m_bmc_colorram[(m_clr_offset/3)*3+1]),pal6bit(m_bmc_colorram[(m_clr_offset/3)*3+2]));
	m_clr_offset=(m_clr_offset+1)%768;
}

READ16_MEMBER(koftball_state::bmc_RAMDAC_color_r)
{
	return m_bmc_colorram[m_clr_offset];
}

READ16_MEMBER(koftball_state::random_number_r)
{
	return machine().rand();
}


READ16_MEMBER(koftball_state::prot_r)
{
	switch(m_prot_data)
	{
		case 0x0000: return 0x0d00;
		case 0xff00: return 0x8d00;

		case 0x8000: return 0x0f0f;
	}

	logerror("unk prot r %x %x\n",m_prot_data,	cpu_get_previouspc(&space.device()));
	return machine().rand();
}

WRITE16_MEMBER(koftball_state::prot_w)
{
	COMBINE_DATA(&m_prot_data);
}

WRITE16_MEMBER(koftball_state::bmc_1_videoram_w)
{
	COMBINE_DATA(&m_bmc_1_videoram[offset]);
	m_tilemap_1->mark_tile_dirty(offset);
}

WRITE16_MEMBER(koftball_state::bmc_2_videoram_w)
{
	COMBINE_DATA(&m_bmc_2_videoram[offset]);
	m_tilemap_2->mark_tile_dirty(offset);
}

static ADDRESS_MAP_START( koftball_mem, AS_PROGRAM, 16, koftball_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x220000, 0x22ffff) AM_RAM AM_SHARE("main_ram")

	AM_RANGE(0x260000, 0x260fff) AM_WRITE(bmc_1_videoram_w) AM_SHARE("bmc_1_videoram")
	AM_RANGE(0x261000, 0x261fff) AM_WRITE(bmc_2_videoram_w) AM_SHARE("bmc_2_videoram")
	AM_RANGE(0x262000, 0x26ffff) AM_RAM

	AM_RANGE(0x280000, 0x28ffff) AM_RAM /* unused ? */
	AM_RANGE(0x2a0000, 0x2a001f) AM_WRITENOP
	AM_RANGE(0x2a0000, 0x2a001f) AM_READ(random_number_r)
	AM_RANGE(0x2b0000, 0x2b0003) AM_READ(random_number_r)
	AM_RANGE(0x2d8000, 0x2d8001) AM_READ(random_number_r)
	/*sound chip or mcu comm ? maybe just i/o (offset 0xe=lamps?)*/
	AM_RANGE(0x2da000, 0x2da001) AM_WRITENOP /* offset ? */
	AM_RANGE(0x2da002, 0x2da003) AM_WRITENOP /* data ? */

	AM_RANGE(0x2db000, 0x2db001) AM_WRITE(bmc_RAMDAC_offset_w)
	AM_RANGE(0x2db002, 0x2db003) AM_READWRITE(bmc_RAMDAC_color_r, bmc_RAMDAC_color_w)
	AM_RANGE(0x2db004, 0x2db005) AM_WRITENOP
	AM_RANGE(0x2dc000, 0x2dc001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0xff00)
	AM_RANGE(0x2f0000, 0x2f0003) AM_READ_PORT("INPUTS")
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP
	AM_RANGE(0x320000, 0x320001) AM_WRITENOP
	AM_RANGE(0x340000, 0x340001) AM_READ(prot_r)
	AM_RANGE(0x360000, 0x360001) AM_WRITE(prot_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( koftball )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("info") PORT_CODE(KEYCODE_Z)//info page
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("test2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("dec") PORT_CODE(KEYCODE_C)//dec sound test
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("inc") PORT_CODE(KEYCODE_V)//inc sound test
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("test5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("test6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("test7") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("test8") PORT_CODE(KEYCODE_A) //test mode exit

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("BET") PORT_CODE(KEYCODE_S) //bet ?
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("test12") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("test13") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("test14") PORT_CODE(KEYCODE_G)
    PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("sound test") PORT_CODE(KEYCODE_H) //test mdoe enter
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("test16") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("Select") PORT_CODE(KEYCODE_K)//test mode select
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER )	PORT_NAME("test18") PORT_CODE(KEYCODE_L)
INPUT_PORTS_END


static TIMER_DEVICE_CALLBACK( bmc_interrupt )
{
	koftball_state *state = timer.machine().driver_data<koftball_state>();
	int scanline = param;

	if(scanline == 240)
		device_set_input_line(state->m_maincpu, 2, HOLD_LINE);

	if(scanline == 128)
		device_set_input_line(state->m_maincpu, 3, HOLD_LINE);

	if(scanline == 64)
		device_set_input_line(state->m_maincpu, 6, HOLD_LINE);
}

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{0,1,2,3, RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+1,RGN_FRAC(1,2)+2,RGN_FRAC(1,2)+3 },
	{ 0, 4, 8, 12, 16,20,  24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static GFXDECODE_START( koftball )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,  0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( koftball, koftball_state )
	MCFG_CPU_ADD("maincpu", M68000, 21477270/2 )
	MCFG_CPU_PROGRAM_MAP(koftball_mem)
	MCFG_TIMER_ADD_SCANLINE("scantimer", bmc_interrupt, "screen", 0, 1)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_STATIC(koftball)

	MCFG_GFXDECODE(koftball)

	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 30*8-1)
	MCFG_PALETTE_LENGTH(256)

	MCFG_VIDEO_START(koftball)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 1122000, OKIM6295_PIN7_LOW) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END

ROM_START( koftball )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ft5_v16_c5.u14", 0x000001, 0x10000, CRC(45c856e3) SHA1(0a25cfc2b09f1bf996f9149ee2a7d0a7e51794b7) )
	ROM_LOAD16_BYTE( "ft5_v16_c6.u15", 0x000000, 0x10000, CRC(5e1784a5) SHA1(5690d315500fb533b12b598cb0a51bd1eadd0505) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE(	"ft5_v6_c3.u61", 0x00000, 0x20000, CRC(f3f747f3) SHA1(6e376d42099733e52779c089303391eeddf4fa87) )
	ROM_LOAD16_BYTE(	"ft5_v6_c4.u58", 0x00001, 0x20000, CRC(8b774574) SHA1(a79c1cf90d7b5ef0aba17770700b2fe18846f7b7) )
	ROM_LOAD16_BYTE(	"ft5_v6_c1.u59", 0x40000, 0x20000, CRC(b33a008f) SHA1(c4fd40883fa1c1cbc58f7b342fed753c52f0cf59) )
	ROM_LOAD16_BYTE(	"ft5_v6_c2.u60", 0x40001, 0x20000, CRC(3dc22223) SHA1(dc74800c51de3b6a7fbf7214a1da1d2f3d2aea84) )


	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ft5_v6_c9.u21", 0x00000, 0x10000,  CRC(f6216740) SHA1(3d1c795da2f8093e937107e3848cb96338536faf) )

ROM_END

#if NVRAM_HACK

static const UINT16 nvram[]=
{
	0x0000,0x5555,0x0000,0x5555,0x0000,0x5555,0x0000,0x5555,
	0x0000,0x5555,0x0000,0x0000,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x0467,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0xffff
};

#endif
static DRIVER_INIT(koftball)
{
	koftball_state *state = machine.driver_data<koftball_state>();
	state->m_bmc_colorram = auto_alloc_array(machine, UINT8, 768);

#if NVRAM_HACK
	{
		int offset=0;
		while(nvram[offset]!=0xffff)
		{
			state->m_main_ram[offset]=nvram[offset];
			++offset;
		}
	}
#endif
}

GAME( 1995, koftball,    0, koftball,    koftball,    koftball, ROT0,  "BMC", "King of Football", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
