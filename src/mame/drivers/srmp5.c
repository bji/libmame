/*

Super Real Mahjong P5
(c)1994 SETA

preliminary driver by Tomasz Slanina


--

CPU   : R3560 (IDT MIPS R3000 derivative)
SOUND : TC6210AF (ST-0016)
OSC.  : 50.0000MHz 42.9545MHz

SX008-01.BIN ; CHR ROM
SX008-02.BIN ;  |
SX008-03.BIN ;  |
SX008-04.BIN ;  |
SX008-05.BIN ;  |
SX008-06.BIN ;  |
SX008-07.BIN ; /
SX008-08.BIN ; SOUND DATA
SX008-09.BIN ; /
SX008-11.BIN ; MAIN PRG
SX008-12.BIN ;  |
SX008-13.BIN ;  |
SX008-14.BIN ; /


Note:

attract sound ON/OFF of DIPSW doesn't work.
This is not a bug (real machine behaves the same).
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mips/r3000.h"
#include "sound/st0016.h"
#include "includes/st0016.h"

#define DEBUG_CHAR

#define SPRITE_GLOBAL_X 0
#define SPRITE_GLOBAL_Y 1
#define SUBLIST_OFFSET	2
#define SUBLIST_LENGTH	3

#define SUBLIST_OFFSET_SHIFT 3
#define SPRITE_LIST_END_MARKER 0x8000

#define SPRITE_TILE    0
#define SPRITE_PALETTE 1
#define SPRITE_LOCAL_X 2
#define SPRITE_LOCAL_Y 3
#define SPRITE_SIZE    4

#define SPRITE_SUBLIST_ENTRY_LENGTH 8
#define SPRITE_LIST_ENTRY_LENGTH    4

#define SPRITE_DATA_GRANULARITY 0x80

class srmp5_state : public st0016_state
{
public:
	srmp5_state(const machine_config &mconfig, device_type type, const char *tag)
		: st0016_state(mconfig, type, tag) { }

	UINT32 m_databank;
	UINT16 *m_tileram;
	UINT16 *m_palram;
	UINT16 *m_sprram;

	UINT8 m_input_select;

	UINT8 m_cmd1;
	UINT8 m_cmd2;
	UINT8 m_cmd_stat;

	UINT32 m_vidregs[0x120 / 4];
#ifdef DEBUG_CHAR
	UINT8 m_tileduty[0x2000];
#endif
	DECLARE_READ32_MEMBER(srmp5_palette_r);
	DECLARE_WRITE32_MEMBER(srmp5_palette_w);
	DECLARE_WRITE32_MEMBER(bank_w);
	DECLARE_READ32_MEMBER(tileram_r);
	DECLARE_WRITE32_MEMBER(tileram_w);
	DECLARE_READ32_MEMBER(spr_r);
	DECLARE_WRITE32_MEMBER(spr_w);
	DECLARE_READ32_MEMBER(data_r);
	DECLARE_WRITE32_MEMBER(input_select_w);
	DECLARE_READ32_MEMBER(srmp5_inputs_r);
	DECLARE_WRITE32_MEMBER(cmd1_w);
	DECLARE_WRITE32_MEMBER(cmd2_w);
	DECLARE_READ32_MEMBER(cmd_stat32_r);
	DECLARE_READ32_MEMBER(srmp5_vidregs_r);
	DECLARE_WRITE32_MEMBER(srmp5_vidregs_w);
	DECLARE_READ32_MEMBER(irq_ack_clear);
	DECLARE_READ8_MEMBER(cmd1_r);
	DECLARE_READ8_MEMBER(cmd2_r);
	DECLARE_READ8_MEMBER(cmd_stat8_r);
};


static SCREEN_UPDATE_RGB32( srmp5 )
{
	srmp5_state *state = screen.machine().driver_data<srmp5_state>();
	int x,y,address,xs,xs2,ys,ys2,height,width,xw,yw,xb,yb,sizex,sizey;
	UINT16 *sprite_list=state->m_sprram;
	UINT16 *sprite_list_end=&state->m_sprram[0x4000]; //guess
	UINT8 *pixels=(UINT8 *)state->m_tileram;

//Table surface seems to be tiles, but display corrupts when switching the scene if always ON.
//Currently the tiles are OFF.
#ifdef BG_ENABLE
	UINT8 tile_width  = (state->m_vidregs[2] >> 0) & 0xFF;
	UINT8 tile_height = (state->m_vidregs[2] >> 8) & 0xFF;
	if(tile_width && tile_height)
	{
		// 16x16 tile
		UINT16 *map = &sprram[0x2000];
		for(yw = 0; yw < tile_height; yw++)
		{
			for(xw = 0; xw < tile_width; xw++)
			{
				UINT16 tile = map[yw * 128 + xw * 2];
				if(tile >= 0x2000) continue;

				address = tile * SPRITE_DATA_GRANULARITY;
				for(y = 0; y < 16; y++)
				{
					for(x = 0; x < 16; x++)
					{
						UINT8 pen = pixels[address];
						if(pen)
						{
							UINT16 pixdata=state->m_palram[pen];
							bitmap.pix32(yw * 16 + y, xw * 16 + x) = ((pixdata&0x7c00)>>7) | ((pixdata&0x3e0)<<6) | ((pixdata&0x1f)<<19);
						}
						address++;
					}
				}
			}
		}
	}
	else
#endif
		bitmap.fill(0, cliprect);

	while((sprite_list[SUBLIST_OFFSET]&SPRITE_LIST_END_MARKER)==0 && sprite_list<sprite_list_end)
	{
		UINT16 *sprite_sublist=&state->m_sprram[sprite_list[SUBLIST_OFFSET]<<SUBLIST_OFFSET_SHIFT];
		UINT16 sublist_length=sprite_list[SUBLIST_LENGTH];
		INT16 global_x,global_y;

		if(0!=sprite_list[SUBLIST_OFFSET])
		{
			global_x=(INT16)sprite_list[SPRITE_GLOBAL_X];
			global_y=(INT16)sprite_list[SPRITE_GLOBAL_Y];
			while(sublist_length)
			{
				x=(INT16)sprite_sublist[SPRITE_LOCAL_X]+global_x;
				y=(INT16)sprite_sublist[SPRITE_LOCAL_Y]+global_y;
				width =(sprite_sublist[SPRITE_SIZE]>> 4)&0xf;
				height=(sprite_sublist[SPRITE_SIZE]>>12)&0xf;

				sizex=(sprite_sublist[SPRITE_SIZE]>>0)&0xf;
				sizey=(sprite_sublist[SPRITE_SIZE]>>8)&0xf;

				address=(sprite_sublist[SPRITE_TILE] & ~(sprite_sublist[SPRITE_SIZE] >> 11 & 7))*SPRITE_DATA_GRANULARITY;
				y -= (height + 1) * (sizey + 1)-1;
				for(xw=0;xw<=width;xw++)
				{
					xb = (sprite_sublist[SPRITE_PALETTE] & 0x8000) ? (width-xw)*(sizex+1)+x: xw*(sizex+1)+x;
					for(yw=0;yw<=height;yw++)
					{
						yb = yw*(sizey+1)+y;
						for(ys=0;ys<=sizey;ys++)
						{
							ys2 = (sprite_sublist[SPRITE_PALETTE] & 0x4000) ? ys : (sizey - ys);
							for(xs=0;xs<=sizex;xs++)
							{
								UINT8 pen=pixels[address&(0x100000-1)];
								xs2 = (sprite_sublist[SPRITE_PALETTE] & 0x8000) ? (sizex - xs) : xs;
								if(pen)
								{
									if(cliprect.contains(xb+xs2, yb+ys2))
									{
										UINT16 pixdata=state->m_palram[pen+((sprite_sublist[SPRITE_PALETTE]&0xff)<<8)];
										bitmap.pix32(yb+ys2, xb+xs2) = ((pixdata&0x7c00)>>7) | ((pixdata&0x3e0)<<6) | ((pixdata&0x1f)<<19);
									}
								}
								++address;
							}
						}
					}
				}
				sprite_sublist+=SPRITE_SUBLIST_ENTRY_LENGTH;
				--sublist_length;
			}
		}
		sprite_list+=SPRITE_LIST_ENTRY_LENGTH;
	}

#ifdef DEBUG_CHAR
	{
		int i;
		for(i = 0; i < 0x2000; i++)
		{
			if (state->m_tileduty[i] == 1)
			{
				gfx_element_decode(screen.machine().gfx[0], i);
				state->m_tileduty[i] = 0;
			}
		}
	}
#endif
	return 0;
}

READ32_MEMBER(srmp5_state::srmp5_palette_r)
{

	return m_palram[offset];
}

WRITE32_MEMBER(srmp5_state::srmp5_palette_w)
{

	COMBINE_DATA(&m_palram[offset]);
	palette_set_color(machine(), offset, MAKE_RGB(data << 3 & 0xFF, data >> 2 & 0xFF, data >> 7 & 0xFF));
}
WRITE32_MEMBER(srmp5_state::bank_w)
{

	COMBINE_DATA(&m_databank);
}

READ32_MEMBER(srmp5_state::tileram_r)
{

	return m_tileram[offset];
}

WRITE32_MEMBER(srmp5_state::tileram_w)
{

	m_tileram[offset] = data & 0xFFFF; //lower 16bit only
#ifdef DEBUG_CHAR
	m_tileduty[offset >> 6] = 1;
#endif
}

READ32_MEMBER(srmp5_state::spr_r)
{

	return m_sprram[offset];
}

WRITE32_MEMBER(srmp5_state::spr_w)
{

	m_sprram[offset] = data & 0xFFFF; //lower 16bit only
}

READ32_MEMBER(srmp5_state::data_r)
{
	UINT32 data;
	const UINT8 *usr = memregion("user2")->base();

	data=((m_databank>>4)&0xf)*0x100000; //guess
	data=usr[data+offset*2]+usr[data+offset*2+1]*256;
	return data|(data<<16);
}

WRITE32_MEMBER(srmp5_state::input_select_w)
{

	m_input_select = data & 0x0F;
}

READ32_MEMBER(srmp5_state::srmp5_inputs_r)
{
	UINT32 ret = 0;

	switch (m_input_select)
	{
	case 0x01:
		ret = ioport("IN0")->read();
		break;
	case 0x02:
		ret = ioport("IN1")->read();
		break;
	case 0x04:
		ret = ioport("IN2")->read();
		break;
	case 0x00:
	case 0x08:
		ret = ioport("IN3")->read();
		break;
	}
	return ret;
}

//almost all cmds are sound related
WRITE32_MEMBER(srmp5_state::cmd1_w)
{

	m_cmd1 = data & 0xFF;
	logerror("cmd1_w %08X\n", data);
}

WRITE32_MEMBER(srmp5_state::cmd2_w)
{

	m_cmd2 = data & 0xFF;
	m_cmd_stat = 5;
	logerror("cmd2_w %08X\n", data);
}

READ32_MEMBER(srmp5_state::cmd_stat32_r)
{

	return m_cmd_stat;
}

READ32_MEMBER(srmp5_state::srmp5_vidregs_r)
{

	logerror("vidregs read  %08X %08X\n", offset << 2, m_vidregs[offset]);
	return m_vidregs[offset];
}

WRITE32_MEMBER(srmp5_state::srmp5_vidregs_w)
{

	COMBINE_DATA(&m_vidregs[offset]);
	if(offset != 0x10C / 4)
		logerror("vidregs write %08X %08X\n", offset << 2, m_vidregs[offset]);
}

READ32_MEMBER(srmp5_state::irq_ack_clear)
{
	cputag_set_input_line(machine(), "sub", R3000_IRQ4, CLEAR_LINE);
	return 0;
}

static ADDRESS_MAP_START( srmp5_mem, AS_PROGRAM, 32, srmp5_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_RAM //maybe 0 - 2fffff ?
	AM_RANGE(0x002f0000, 0x002f7fff) AM_RAM
	AM_RANGE(0x01000000, 0x01000003) AM_WRITEONLY  // 0xaa .. watchdog ?
	AM_RANGE(0x01800000, 0x01800003) AM_RAM //?1
	AM_RANGE(0x01800004, 0x01800007) AM_READ_PORT("DSW1")
	AM_RANGE(0x01800008, 0x0180000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x0180000c, 0x0180000f) AM_WRITE(bank_w)
	AM_RANGE(0x01800010, 0x01800013) AM_READ(srmp5_inputs_r) //multiplexed controls (selected by writes to 1c)
	AM_RANGE(0x01800014, 0x01800017) AM_READ_PORT("TEST")
	AM_RANGE(0x0180001c, 0x0180001f) AM_WRITE(input_select_w)//c1 c2 c4 c8 => mahjong inputs (at $10) - bits 0-3
	AM_RANGE(0x01800200, 0x01800203) AM_RAM  //sound related ? only few writes after boot
	AM_RANGE(0x01802000, 0x01802003) AM_WRITE(cmd1_w)
	AM_RANGE(0x01802004, 0x01802007) AM_WRITE(cmd2_w)
	AM_RANGE(0x01802008, 0x0180200b) AM_READ(cmd_stat32_r)
	AM_RANGE(0x01a00000, 0x01bfffff) AM_READ(data_r)
	AM_RANGE(0x01c00000, 0x01c00003) AM_READNOP // debug? 'Toru'

	AM_RANGE(0x0a000000, 0x0a0fffff) AM_READWRITE(spr_r, spr_w)
	AM_RANGE(0x0a100000, 0x0a17ffff) AM_READWRITE(srmp5_palette_r, srmp5_palette_w)
	//0?N???A?????????i??????????
	AM_RANGE(0x0a180000, 0x0a180003) AM_READNOP // write 0x00000400
	AM_RANGE(0x0a180000, 0x0a18011f) AM_READWRITE(srmp5_vidregs_r, srmp5_vidregs_w)
	AM_RANGE(0x0a200000, 0x0a3fffff) AM_READWRITE(tileram_r, tileram_w)

	AM_RANGE(0x1eff0000, 0x1eff001f) AM_WRITEONLY
	AM_RANGE(0x1eff003c, 0x1eff003f) AM_READ(irq_ack_clear)
	AM_RANGE(0x1fc00000, 0x1fdfffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x2fc00000, 0x2fdfffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( st0016_mem, AS_PROGRAM, 8, srmp5_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe900, 0xe9ff) AM_DEVREADWRITE_LEGACY("stsnd", st0016_snd_r, st0016_snd_w)
	AM_RANGE(0xec00, 0xec1f) AM_READ(st0016_character_ram_r) AM_WRITE(st0016_character_ram_w)
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

READ8_MEMBER(srmp5_state::cmd1_r)
{

	m_cmd_stat = 0;
	return m_cmd1;
}

READ8_MEMBER(srmp5_state::cmd2_r)
{

	return m_cmd2;
}

READ8_MEMBER(srmp5_state::cmd_stat8_r)
{

	return m_cmd_stat;
}

static ADDRESS_MAP_START( st0016_io, AS_IO, 8, srmp5_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xbf) AM_READ(st0016_vregs_r) AM_WRITE(st0016_vregs_w)
	AM_RANGE(0xc0, 0xc0) AM_READ(cmd1_r)
	AM_RANGE(0xc1, 0xc1) AM_READ(cmd2_r)
	AM_RANGE(0xc2, 0xc2) AM_READ(cmd_stat8_r)
	AM_RANGE(0xe1, 0xe1) AM_WRITE(st0016_rom_bank_w)
	AM_RANGE(0xe7, 0xe7) AM_WRITE(st0016_rom_bank_w)
	AM_RANGE(0xf0, 0xf0) AM_READ(st0016_dma_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( srmp5 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "PUT" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPSETTING(      0x0001, "7" )
	PORT_DIPSETTING(      0x0002, "6" )
	PORT_DIPSETTING(      0x0003, "5" )
	PORT_DIPSETTING(      0x0007, "4" )
	PORT_DIPSETTING(      0x0006, "1" )
	PORT_DIPSETTING(      0x0005, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPNAME( 0x0008, 0x0008, "Kuitan" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Test ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT ( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT ( 0xfffffff0, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )

	PORT_START("IN1")
	PORT_BIT ( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT ( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT ( 0x00000020, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT ( 0xffffffe0, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT ( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )

	PORT_START("IN3")
	PORT_BIT ( 0xffffff60, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT ( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT ( 0x00000080, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("TEST")
	PORT_BIT ( 0x00000080, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F2)
	PORT_BIT ( 0xffffff7f, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded

INPUT_PORTS_END

static const st0016_interface st0016_config =
{
	&st0016_charram
};

static const r3000_cpu_core r3000_config =
{
	1,	/* 1 if we have an FPU, 0 otherwise */
	4096,	/* code cache size */
	4096	/* data cache size */
};

static const gfx_layout tile_16x8x8_layout =
{
	16,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	{ STEP8(0, 8*16) },
	16*8*8
};

#if 0
static const gfx_layout tile_16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	{ STEP16(0, 8*16) },
	16*16*8
};
#endif

static GFXDECODE_START( srmp5 )
	GFXDECODE_ENTRY( "gfx1", 0, tile_16x8x8_layout,  0x0, 0x800  )
	//GFXDECODE_ENTRY( "gfx1", 0, tile_16x16x8_layout, 0x0, 0x800  )
GFXDECODE_END

static MACHINE_CONFIG_START( srmp5, srmp5_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,8000000)
	MCFG_CPU_PROGRAM_MAP(st0016_mem)
	MCFG_CPU_IO_MAP(st0016_io)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_CPU_ADD("sub", R3000LE, 25000000)
	MCFG_CPU_CONFIG(r3000_config)
	MCFG_CPU_PROGRAM_MAP(srmp5_mem)
	MCFG_CPU_VBLANK_INT("screen", irq4_line_assert)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(96*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 42*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(srmp5)

	MCFG_PALETTE_LENGTH(0x1800)
#ifdef DEBUG_CHAR
	MCFG_GFXDECODE( srmp5 )
#endif
	MCFG_VIDEO_START(st0016)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("stsnd", ST0016, 0)
	MCFG_SOUND_CONFIG(st0016_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

ROM_START( srmp5 )
	ROM_REGION( 0x410000, "maincpu", 0 )
	ROM_LOAD( "sx008-08.bin",   0x010000, 0x200000,   CRC(d4ac54f4) SHA1(c3dc76cd71485796a0b6a960294ea96eae8c946e) )
	ROM_LOAD( "sx008-09.bin",   0x210000, 0x200000,   CRC(5a3e6560) SHA1(92ea398f3c5e3035869f0ca5dfe7b05c90095318) )
	ROM_COPY( "maincpu",  0x10000, 0x00000, 0x08000 )

	ROM_REGION32_BE( 0x200000, "user1", 0 )
	ROM_LOAD32_BYTE( "sx008-14.bin",   0x00000, 0x80000,   CRC(b5c55120) SHA1(0a41351c9563b2c6a00709189a917757bd6e0a24) )
	ROM_LOAD32_BYTE( "sx008-13.bin",   0x00001, 0x80000,   CRC(0af475e8) SHA1(24cddffa0f8c81832ae8870823d772e3b7493194) )
	ROM_LOAD32_BYTE( "sx008-12.bin",   0x00002, 0x80000,   CRC(43e9bb98) SHA1(e46dd98d2e1babfa12ddf2fa9b31377e8691d3a1) )
	ROM_LOAD32_BYTE( "sx008-11.bin",   0x00003, 0x80000,   CRC(ca15ff45) SHA1(5ee610e0bb835568c36898210a6f8394902d5b54) )

	ROM_REGION( 0xf00000, "user2",0) /* gfx ? */
	ROM_LOAD( "sx008-01.bin",   0x000000, 0x200000,   CRC(82dabf48) SHA1(c53e9ed0056c431eab13ab362936c25d3cc5abba) )
	ROM_LOAD( "sx008-02.bin",   0x200000, 0x200000,   CRC(cfd2be0f) SHA1(a21f2928e08047c97443123aceba7ff4e95c6d3d) )
	ROM_LOAD( "sx008-03.bin",   0x400000, 0x200000,   CRC(d7323b10) SHA1(94ecc17b6b8b071cf2c61bbef4aec2c6c7693c62) )
	ROM_LOAD( "sx008-04.bin",   0x600000, 0x200000,   CRC(b10d3067) SHA1(21c36307780d4f38ec54d87cd222d65e4f8c00a5) )
	ROM_LOAD( "sx008-05.bin",   0x800000, 0x200000,   CRC(0ff5e6f5) SHA1(ab7d021757f341d28db6d7d009c20ec9d7bd83c1) )
	ROM_LOAD( "sx008-06.bin",   0xa00000, 0x200000,   CRC(ba6fd7c4) SHA1(f086195c5c647e07e77ce2a23e94d28e6ad9ff4f) )
	ROM_LOAD( "sx008-07.bin",   0xc00000, 0x200000,   CRC(3564485d) SHA1(12464de4e2b6c4df1595183996d1987f0ecffb01) )
#ifdef DEBUG_CHAR
	ROM_REGION( 0x100000, "gfx1", 0)
	ROM_FILL( 0, 0x100000, 0x00)
#endif
ROM_END

static DRIVER_INIT(srmp5)
{
	srmp5_state *state = machine.driver_data<srmp5_state>();
	st0016_game = 9;

	state->m_tileram = auto_alloc_array(machine, UINT16, 0x100000/2);
	state->m_sprram  = auto_alloc_array(machine, UINT16, 0x080000/2);
	state->m_palram  = auto_alloc_array(machine, UINT16, 0x040000/2);
#ifdef DEBUG_CHAR
	memset(state->m_tileduty, 1, 0x2000);
#endif
}

GAME( 1994, srmp5,	0,	  srmp5,    srmp5,    srmp5,    ROT0, "Seta",  "Super Real Mahjong P5", GAME_IMPERFECT_GRAPHICS)
