/*
    Super Real Mahjong P6 (JPN Ver.)
    (c)1996 Seta

WIP driver by Sebastien Volpe, Tomasz Slanina and David Haywood

Emulation Notes:
The graphics are compressed, using the same 8bpp RLE scheme as CPS3 uses
for the background on Sean's stage of Street Fighter III.

DMA Operations are not fully understood


according prg ROM (offset $0fff80):

    S12 SYSTEM
    SUPER REAL MAJAN P6
    SETA CO.,LTD
    19960410
    V1.00

TODO:
 - fix sound emulation
 - fix DMA operations
 - fix video emulation

Are there other games on this 'System S12' hardware ???

---------------- dump infos ----------------

[Jun/15/2000]

Super Real Mahjong P6 (JPN Ver.)
(c)1996 Seta

SX011
E47-REV01B

CPU:    68000-16
Sound:  NiLe
OSC:    16.0000MHz
        42.9545MHz
        56.0000MHz

Chips:  ST-0026 NiLe (video, sound)
        ST-0017


SX011-01.22  chr, samples (?)
SX011-02.21
SX011-03.20
SX011-04.19
SX011-05.18
SX011-06.17
SX011-07.16
SX011-08.15

SX011-09.10  68000 data

SX011-10.4   68000 prg.
SX011-11.5


Dumped 06/15/2000

*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/nile.h"

class srmp6_state : public driver_device
{
public:
	srmp6_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16* m_tileram;
	UINT16* m_dmaram;
	UINT16* m_chrram;

	UINT16 *m_sprram;
	UINT16 *m_sprram_old;

	int m_brightness;
	UINT16 m_input_select;
	UINT16 *m_video_regs;

	unsigned short m_lastb;
	unsigned short m_lastb2;
	int m_destl;
};

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

static const gfx_layout tiles8x8_layout =
{
	8,8,
	(0x100000*16)/0x40,
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	{ 0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64 },
	8*64
};

static void update_palette(running_machine &machine)
{
	srmp6_state *state = machine.driver_data<srmp6_state>();
	INT8 r, g ,b;
	int brg = state->m_brightness - 0x60;
	int i;

	for(i = 0; i < 0x800; i++)
	{
		r = machine.generic.paletteram.u16[i] >>  0 & 0x1F;
		g = machine.generic.paletteram.u16[i] >>  5 & 0x1F;
		b = machine.generic.paletteram.u16[i] >> 10 & 0x1F;

		if(brg < 0) {
			r += (r * brg) >> 5;
			if(r < 0) r = 0;
			g += (g * brg) >> 5;
			if(g < 0) g = 0;
			b += (b * brg) >> 5;
			if(b < 0) b = 0;
		}
		else if(brg > 0) {
			r += ((0x1F - r) * brg) >> 5;
			if(r > 0x1F) r = 0x1F;
			g += ((0x1F - g) * brg) >> 5;
			if(g > 0x1F) g = 0x1F;
			b += ((0x1F - b) * brg) >> 5;
			if(b > 0x1F) b = 0x1F;
		}
		palette_set_color(machine, i, MAKE_RGB(r << 3, g << 3, b << 3));
	}
}

static VIDEO_START(srmp6)
{
	srmp6_state *state = machine.driver_data<srmp6_state>();

	state->m_tileram = auto_alloc_array_clear(machine, UINT16, 0x100000*16/2);
	state->m_dmaram = auto_alloc_array(machine, UINT16, 0x100/2);
	state->m_sprram_old = auto_alloc_array_clear(machine, UINT16, 0x80000/2);

	/* create the char set (gfx will then be updated dynamically from RAM) */
	machine.gfx[0] = gfx_element_alloc(machine, &tiles8x8_layout, (UINT8*)state->m_tileram, machine.total_colors() / 256, 0);
	machine.gfx[0]->color_granularity=256;

	state->m_brightness = 0x60;
}

#if 0
static int xixi=0;
#endif

static SCREEN_UPDATE(srmp6)
{
	srmp6_state *state = screen->machine().driver_data<srmp6_state>();
	int alpha;
	int x,y,tileno,height,width,xw,yw,sprite,xb,yb;
	UINT16 *sprite_list = state->m_sprram_old;
	UINT16 mainlist_offset = 0;

	union
	{
		INT16  a;
		UINT16 b;
	} temp;

	bitmap_fill(bitmap,cliprect,0);

#if 0
	/* debug */
	if(screen->machine().input().code_pressed_once(KEYCODE_Q))
	{
		++xixi;
		printf("%x\n",xixi);
	}

	if(screen->machine().input().code_pressed_once(KEYCODE_W))
	{
		--xixi;
		printf("%x\n",xixi);
	}
#endif

	/* Main spritelist is 0x0000 - 0x1fff in spriteram, sublists follow */
	while (mainlist_offset<0x2000/2)
	{

		UINT16 *sprite_sublist = &state->m_sprram_old[sprite_list[mainlist_offset+1]<<3];
		UINT16 sublist_length=sprite_list[mainlist_offset+0]&0x7fff; //+1 ?
		INT16 global_x,global_y, flip_x, flip_y;
		UINT16 global_pal;

		/* end of list marker */
		if (sprite_list[mainlist_offset+0] == 0x8000)
			break;


		if(sprite_list[mainlist_offset+0]!=0)
		{
			temp.b=sprite_list[mainlist_offset+2];
			global_x=temp.a;
			temp.b=sprite_list[mainlist_offset+3];
			global_y=temp.a;

			global_pal = sprite_list[mainlist_offset+4] & 0x7;

			if((sprite_list[mainlist_offset+5] & 0x700) == 0x700)
			{
				alpha = (sprite_list[mainlist_offset+5] & 0x1F) << 3;
			}
			else
			{
				alpha = 255;
			}
			//  printf("%x %x \n",sprite_list[mainlist_offset+1],sublist_length);

			while(sublist_length)
			{
				sprite=sprite_sublist[0]&0x7fff;
				flip_x=sprite_sublist[1]>>8&1;
				flip_y=sprite_sublist[1]>>9&1;
				temp.b=sprite_sublist[2];
				x=temp.a;
				temp.b=sprite_sublist[3];
				y=temp.a;
				//x+=global_x;
				//y+=global_y;

				width=((sprite_sublist[1])&0x3);
				height=((sprite_sublist[1]>>2)&0x3);

				height = 1 << height;
				width = 1 << width;

				y-=height*8;
				tileno = sprite;
				//tileno += (sprite_list[4]&0xf)*0x4000; // this makes things worse in places (title screen for example)

				for(xw=0;xw<width;xw++)
				{
					for(yw=0;yw<height;yw++)
					{

						if(!flip_x)
							xb=x+xw*8+global_x;
						else
							xb=x+(width-xw-1)*8+global_x;

						if(!flip_y)
							yb=y+yw*8+global_y;
						else
							yb=y+(height-yw-1)*8+global_y;

						drawgfx_alpha(bitmap,cliprect,screen->machine().gfx[0],tileno,global_pal,flip_x,flip_y,xb,yb,0,alpha);
						tileno++;
					}
				}

				sprite_sublist+=8;
				--sublist_length;
			}
		}
		mainlist_offset+=8;
	}

	memcpy(state->m_sprram_old, state->m_sprram, 0x80000);

	if(screen->machine().input().code_pressed_once(KEYCODE_Q))
	{
		FILE *p=fopen("tileram.bin","wb");
		fwrite(state->m_tileram, 1, 0x100000*16, p);
		fclose(p);
	}


	return 0;
}

/***************************************************************************
    Main CPU memory handlers
***************************************************************************/

static WRITE16_HANDLER( srmp6_input_select_w )
{
	srmp6_state *state = space->machine().driver_data<srmp6_state>();

	state->m_input_select = data & 0x0f;
}

static READ16_HANDLER( srmp6_inputs_r )
{
	srmp6_state *state = space->machine().driver_data<srmp6_state>();

	if (offset == 0)			// DSW
		return input_port_read(space->machine(), "DSW");

	switch (state->m_input_select)	// inputs
	{
		case 1<<0: return input_port_read(space->machine(), "KEY0");
		case 1<<1: return input_port_read(space->machine(), "KEY1");
		case 1<<2: return input_port_read(space->machine(), "KEY2");
		case 1<<3: return input_port_read(space->machine(), "KEY3");
	}

	return 0;
}


static WRITE16_HANDLER( video_regs_w )
{
	srmp6_state *state = space->machine().driver_data<srmp6_state>();

	switch(offset)
	{

		case 0x5e/2: // bank switch, used by ROM check
		{
			const UINT8 *rom = space->machine().region("nile")->base();
			LOG(("%x\n",data));
			memory_set_bankptr(space->machine(), "bank1",(UINT16 *)(rom + (data & 0x0f)*0x200000));
			break;
		}

		// set by IT4
		case 0x5c/2: // either 0x40 explicitely in many places, or according $2083b0 (IT4)
			//Fade in/out (0x40(dark)-0x60(normal)-0x7e?(bright) reset by 0x00?
			data = (!data)?0x60:(data == 0x5e)?0x60:data;
			if (state->m_brightness != data) {
				state->m_brightness = data;
				update_palette(space->machine());
			}
			break;

		/* unknown registers - there are others */

		// set by IT4 (jsr $b3c), according flip screen dsw
		case 0x48/2: //     0 /  0xb0 if flipscreen
		case 0x52/2: //     0 / 0x2ef if flipscreen
		case 0x54/2: // 0x152 / 0x15e if flipscreen

		// set by IT4 ($82e-$846)
		case 0x56/2: // written 8,9,8,9 successively

		default:
			logerror("video_regs_w (PC=%06X): %04x = %04x & %04x\n", cpu_get_previouspc(&space->device()), offset*2, data, mem_mask);
			break;
	}
	COMBINE_DATA(&state->m_video_regs[offset]);
}

static READ16_HANDLER( video_regs_r )
{
	srmp6_state *state = space->machine().driver_data<srmp6_state>();

	logerror("video_regs_r (PC=%06X): %04x\n", cpu_get_previouspc(&space->device()), offset*2);
	return state->m_video_regs[offset];
}


/* DMA RLE stuff - the same as CPS3 */
static UINT32 process(running_machine &machine,UINT8 b,UINT32 dst_offset)
{
	srmp6_state *state = machine.driver_data<srmp6_state>();
	int l=0;

	UINT8 *tram=(UINT8*)state->m_tileram;

	if (state->m_lastb == state->m_lastb2)	//rle
	{
		int i;
		int rle=(b+1)&0xff;

		for(i=0;i<rle;++i)
		{
			tram[dst_offset + state->m_destl] = state->m_lastb;
			gfx_element_mark_dirty(machine.gfx[0], (dst_offset + state->m_destl)/0x40);

			dst_offset++;
			++l;
		}
		state->m_lastb2 = 0xffff;

		return l;
	}
	else
	{
		state->m_lastb2 = state->m_lastb;
		state->m_lastb = b;
		tram[dst_offset + state->m_destl] = b;
		gfx_element_mark_dirty(machine.gfx[0], (dst_offset + state->m_destl)/0x40);

		return 1;
	}
}


static WRITE16_HANDLER(srmp6_dma_w)
{
	srmp6_state *state = space->machine().driver_data<srmp6_state>();
	UINT16* dmaram = state->m_dmaram;

	COMBINE_DATA(&dmaram[offset]);
	if (offset==13 && dmaram[offset]==0x40)
	{
		const UINT8 *rom = space->machine().region("nile")->base();
		UINT32 srctab=2*((((UINT32)dmaram[5])<<16)|dmaram[4]);
		UINT32 srcdata=2*((((UINT32)dmaram[11])<<16)|dmaram[10]);
		UINT32 len=4*(((((UINT32)dmaram[7]&3)<<16)|dmaram[6])+1); //??? WRONG!
		int tempidx=0;

		/* show params */
		LOG(("DMA! %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
				dmaram[0x00/2],
				dmaram[0x02/2],
				dmaram[0x04/2],
				dmaram[0x06/2],
				dmaram[0x08/2],
				dmaram[0x0a/2],
				dmaram[0x0c/2],
				dmaram[0x0e/2],
				dmaram[0x10/2],
				dmaram[0x12/2],
				dmaram[0x14/2],
				dmaram[0x16/2],
				dmaram[0x18/2],
				dmaram[0x1a/2]));

		state->m_destl = dmaram[9]*0x40000;

		state->m_lastb = 0xfffe;
		state->m_lastb2 = 0xffff;

		while(1)
		{
			int i;
			UINT8 ctrl=rom[srcdata];
			++srcdata;

			for(i=0;i<8;++i)
			{
				UINT8 p=rom[srcdata];

				if(ctrl&0x80)
				{
					UINT8 real_byte;
					real_byte = rom[srctab+p*2];
					tempidx+=process(space->machine(),real_byte,tempidx);
					real_byte = rom[srctab+p*2+1];//px[DMA_XOR((current_table_address+p*2+1))];
					tempidx+=process(space->machine(),real_byte,tempidx);
				}
				else
				{
					tempidx+=process(space->machine(),p,tempidx);
				}

				ctrl<<=1;
				++srcdata;


				if(tempidx>=len)
				{
					LOG(("%x\n",srcdata));
					return;
				}
			}
		}
	}
}

/* if tileram is actually bigger than the mapped area, how do we access the rest? */
static READ16_HANDLER(tileram_r)
{
	srmp6_state *state = space->machine().driver_data<srmp6_state>();

	return state->m_chrram[offset];
}

static WRITE16_HANDLER(tileram_w)
{
	srmp6_state *state = space->machine().driver_data<srmp6_state>();

	//UINT16 tmp;
	COMBINE_DATA(&state->m_chrram[offset]);

	/* are the DMA registers enabled some other way, or always mapped here, over RAM? */
	if (offset >= 0xfff00/2 && offset <= 0xfff1a/2 )
	{
		offset &=0x1f;
		srmp6_dma_w(space,offset,data,mem_mask);
	}
}

static WRITE16_HANDLER(paletteram_w)
{
	srmp6_state *state = space->machine().driver_data<srmp6_state>();
	INT8 r, g, b;
	int brg = state->m_brightness - 0x60;

	paletteram16_xBBBBBGGGGGRRRRR_word_w(space, offset, data, mem_mask);

	if(brg)
	{
		r = data >>  0 & 0x1F;
		g = data >>  5 & 0x1F;
		b = data >> 10 & 0x1F;

		if(brg < 0) {
			r += (r * brg) >> 5;
			if(r < 0) r = 0;
			g += (g * brg) >> 5;
			if(g < 0) g = 0;
			b += (b * brg) >> 5;
			if(b < 0) b = 0;
		}
		else if(brg > 0) {
			r += ((0x1F - r) * brg) >> 5;
			if(r > 0x1F) r = 0x1F;
			g += ((0x1F - g) * brg) >> 5;
			if(g > 0x1F) g = 0x1F;
			b += ((0x1F - b) * brg) >> 5;
			if(b > 0x1F) b = 0x1F;
		}

		palette_set_color(space->machine(), offset, MAKE_RGB(r << 3, g << 3, b << 3));
	}
}

static READ16_HANDLER( srmp6_irq_ack_r )
{
	cputag_set_input_line(space->machine(), "maincpu", 4, CLEAR_LINE);
	return 0; // value read doesn't matter
}

static ADDRESS_MAP_START( srmp6_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x23ffff) AM_RAM					// work RAM
	AM_RANGE(0x600000, 0x7fffff) AM_ROMBANK("bank1")		// banked ROM (used by ROM check)
	AM_RANGE(0x800000, 0x9fffff) AM_ROM AM_REGION("user1", 0)

	AM_RANGE(0x300000, 0x300005) AM_READWRITE(srmp6_inputs_r, srmp6_input_select_w)		// inputs
	AM_RANGE(0x480000, 0x480fff) AM_RAM_WRITE(paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x4d0000, 0x4d0001) AM_READ(srmp6_irq_ack_r)

	// OBJ RAM: checked [$400000-$47dfff]
	AM_RANGE(0x400000, 0x47ffff) AM_RAM AM_BASE_MEMBER(srmp6_state,m_sprram)

	// CHR RAM: checked [$500000-$5fffff]
	AM_RANGE(0x500000, 0x5fffff) AM_READWRITE(tileram_r,tileram_w) AM_BASE_MEMBER(srmp6_state,m_chrram)
	//AM_RANGE(0x5fff00, 0x5fffff) AM_WRITE(dma_w) AM_BASE_MEMBER(srmp6_state,m_dmaram)

	AM_RANGE(0x4c0000, 0x4c006f) AM_READWRITE(video_regs_r, video_regs_w) AM_BASE_MEMBER(srmp6_state,m_video_regs)	// ? gfx regs ST-0026 NiLe
	AM_RANGE(0x4e0000, 0x4e00ff) AM_DEVREADWRITE("nile", nile_snd_r, nile_snd_w)
	AM_RANGE(0x4e0100, 0x4e0101) AM_DEVREADWRITE("nile", nile_sndctrl_r, nile_sndctrl_w)
	//AM_RANGE(0x4e0110, 0x4e0111) AM_NOP // ? accessed once ($268dc, written $b.w)
	//AM_RANGE(0x5fff00, 0x5fff1f) AM_RAM // ? see routine $5ca8, video_regs related ???

	//AM_RANGE(0xf00004, 0xf00005) AM_RAM // ?
	//AM_RANGE(0xf00006, 0xf00007) AM_RAM // ?

ADDRESS_MAP_END


/***************************************************************************
    Port definitions
***************************************************************************/

static INPUT_PORTS_START( srmp6 )

	PORT_START("KEY0")
	PORT_BIT( 0xfe01, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x0100, IP_ACTIVE_LOW )

	PORT_START("KEY1")
	PORT_BIT( 0xfe41, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0180, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0xfe41, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0180, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0xfe61, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0180, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")	/* 16-bit DSW1+DSW2 */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )		// DSW1
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Re-Clothe" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Nudity" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Difficulty ) )	// DSW2
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPSETTING(      0x0100, "7" )
	PORT_DIPSETTING(      0x0200, "6" )
	PORT_DIPSETTING(      0x0300, "5" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0500, "2" )
	PORT_DIPSETTING(      0x0600, "1" )
	PORT_DIPSETTING(      0x0700, "4" )
	PORT_DIPNAME( 0x0800, 0x0000, "Kuitan" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Continues ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )
INPUT_PORTS_END

/***************************************************************************
    Machine driver
***************************************************************************/

static MACHINE_CONFIG_START( srmp6, srmp6_state )

	MCFG_CPU_ADD("maincpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(srmp6_map)
	MCFG_CPU_VBLANK_INT("screen",irq4_line_assert) // irq3 is a timer irq, but it's never enabled

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 42*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE(srmp6)

	MCFG_PALETTE_LENGTH(0x800)

	MCFG_VIDEO_START(srmp6)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("nile", NILE, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/***************************************************************************
    ROM definition(s)
***************************************************************************/

ROM_START( srmp6 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "sx011-10.4", 0x000001, 0x080000, CRC(8f4318a5) SHA1(44160968cca027b3d42805f2dd42662d11257ef6) )
	ROM_LOAD16_BYTE( "sx011-11.5", 0x000000, 0x080000, CRC(7503d9cf) SHA1(03ab35f13b6166cb362aceeda18e6eda8d3abf50) )

	ROM_REGION( 0x200000, "user1", 0 ) /* 68000 Data */
	ROM_LOAD( "sx011-09.10", 0x000000, 0x200000, CRC(58f74438) SHA1(a256e39ca0406e513ab4dbd812fb0b559b4f61f2) )

	/* these are accessed directly by the 68k, DMA device etc.  NOT decoded */
	ROM_REGION( 0x2000000, "nile", 0)	/* Banked ROM */
	ROM_LOAD16_WORD_SWAP( "sx011-08.15", 0x0000000, 0x0400000, CRC(01b3b1f0) SHA1(bbd60509c9ba78358edbcbb5953eafafd6e2eaf5) ) // CHR00
	ROM_LOAD16_WORD_SWAP( "sx011-07.16", 0x0400000, 0x0400000, CRC(26e57dac) SHA1(91272268977c5fbff7e8fbe1147bf108bd2ed321) ) // CHR01
	ROM_LOAD16_WORD_SWAP( "sx011-06.17", 0x0800000, 0x0400000, CRC(220ee32c) SHA1(77f39b54891c2381b967534b0f6d380962eadcae) ) // CHR02
	ROM_LOAD16_WORD_SWAP( "sx011-05.18", 0x0c00000, 0x0400000, CRC(87e5fea9) SHA1(abd751b5744d6ac7e697774ea9a7f7455bf3ac7c) ) // CHR03
	ROM_LOAD16_WORD_SWAP( "sx011-04.19", 0x1000000, 0x0400000, CRC(e90d331e) SHA1(d8afb1497cec8fe6de10d23d49427e11c4c57910) ) // CHR04
	ROM_LOAD16_WORD_SWAP( "sx011-03.20", 0x1400000, 0x0400000, CRC(f1f24b35) SHA1(70d6848f77940331e1be8591a33d62ac22a3aee9) ) // CHR05
	ROM_LOAD16_WORD_SWAP( "sx011-02.21", 0x1800000, 0x0400000, CRC(c56d7e50) SHA1(355c64b38e7b266f386b9c0b906c8581fc15374b) ) // CHR06
	ROM_LOAD16_WORD_SWAP( "sx011-01.22", 0x1c00000, 0x0400000, CRC(785409d1) SHA1(3e31254452a30d929161a1ea3a3daa69de058364) ) // CHR07
ROM_END



/***************************************************************************
    Game driver(s)
***************************************************************************/

/*GAME( YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS)*/
GAME( 1995, srmp6, 0, srmp6, srmp6, 0, ROT0, "Seta", "Super Real Mahjong P6 (Japan)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND)
