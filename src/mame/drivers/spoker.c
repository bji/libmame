/***************************************************************************
Super Poker (IGS)
Driver by Mirko Buffoni

TODO:
- Understand how to reset NVRAM
- Map DSW (Operator mode doesn't help)
- Map Leds and Coin counters
- 3super8 randomly crashes
- 3super8 doesn't have the 8x32 tilemap, change the video emulation accordingly
***************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "cpu/z80/z80.h"
#include "sound/2413intf.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"


/***************************************************************************
                                Video Hardware
***************************************************************************/

class spoker_state : public driver_device
{
public:
	spoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg_tile_ram(*this, "bg_tile_ram"),
		m_fg_tile_ram(*this, "fg_tile_ram"),
		m_fg_color_ram(*this, "fg_color_ram"){ }

	required_shared_ptr<UINT8> m_bg_tile_ram;
	tilemap_t *m_bg_tilemap;

	required_shared_ptr<UINT8> m_fg_tile_ram;
	required_shared_ptr<UINT8> m_fg_color_ram;
	tilemap_t *m_fg_tilemap;

	int m_video_enable;
	int m_nmi_ack;
	int m_hopper;
	UINT8 m_igs_magic[2];
	UINT8 m_out[3];
	DECLARE_WRITE8_MEMBER(bg_tile_w);
	DECLARE_WRITE8_MEMBER(fg_tile_w);
	DECLARE_WRITE8_MEMBER(fg_color_w);
	DECLARE_WRITE8_MEMBER(spoker_nmi_and_coins_w);
	DECLARE_WRITE8_MEMBER(spoker_video_and_leds_w);
	DECLARE_WRITE8_MEMBER(spoker_leds_w);
	DECLARE_WRITE8_MEMBER(spoker_magic_w);
	DECLARE_READ8_MEMBER(spoker_magic_r);
	DECLARE_CUSTOM_INPUT_MEMBER(hopper_r);
};

WRITE8_MEMBER(spoker_state::bg_tile_w)
{

	m_bg_tile_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	spoker_state *state = machine.driver_data<spoker_state>();
	int code = state->m_bg_tile_ram[tile_index];
	SET_TILE_INFO(1 + (tile_index & 3), code & 0xff, 0, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	spoker_state *state = machine.driver_data<spoker_state>();
	int code = state->m_fg_tile_ram[tile_index] | (state->m_fg_color_ram[tile_index] << 8);
	SET_TILE_INFO(0, code, (4*(code >> 14)+3), 0);
}

WRITE8_MEMBER(spoker_state::fg_tile_w)
{

	m_fg_tile_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(spoker_state::fg_color_w)
{

	m_fg_color_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

static VIDEO_START(spoker)
{
	spoker_state *state = machine.driver_data<spoker_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,	8,  32,	128, 8);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,	8,  8,	128, 32);
	state->m_fg_tilemap->set_transparent_pen(0);
}

static SCREEN_UPDATE_IND16(spoker)
{
	spoker_state *state = screen.machine().driver_data<spoker_state>();

	bitmap.fill(get_black_pen(screen.machine()), cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

CUSTOM_INPUT_MEMBER(spoker_state::hopper_r)
{
	if (m_hopper) return !(machine().primary_screen->frame_number()%10);
	return machine().input().code_pressed(KEYCODE_H);
}

static void show_out(UINT8 *out)
{
#ifdef MAME_DEBUG
	popmessage("%02x %02x %02x", out[0], out[1], out[2]);
#endif
}

WRITE8_MEMBER(spoker_state::spoker_nmi_and_coins_w)
{

	if ((data) & (0x22))
	{
		logerror("PC %06X: nmi_and_coins = %02x\n",cpu_get_pc(&space.device()),data);
//      popmessage("%02x",data);
	}

	coin_counter_w(machine(), 0,		data & 0x01);	// coin_a
	coin_counter_w(machine(), 1,		data & 0x04);	// coin_c
	coin_counter_w(machine(), 2,		data & 0x08);	// key in
	coin_counter_w(machine(), 3,		data & 0x10);	// coin out mech

	set_led_status(machine(), 6,		data & 0x40);	// led for coin out / hopper active

	if(((m_nmi_ack & 0x80) == 0) && data & 0x80)
		cputag_set_input_line(machine(), "maincpu", INPUT_LINE_NMI, CLEAR_LINE);

	m_nmi_ack = data & 0x80;     // nmi acknowledge, 0 -> 1

	m_out[0] = data;
	show_out(m_out);
}

WRITE8_MEMBER(spoker_state::spoker_video_and_leds_w)
{

	set_led_status(machine(), 4,	  data & 0x01);	// start?
	set_led_status(machine(), 5,	  data & 0x04);	// l_bet?

	m_video_enable	=	  data & 0x40;
	m_hopper			=	(~data)& 0x80;

	m_out[1] = data;
	show_out(m_out);
}

WRITE8_MEMBER(spoker_state::spoker_leds_w)
{

	set_led_status(machine(), 0, data & 0x01);	// stop_1
	set_led_status(machine(), 1, data & 0x02);	// stop_2
	set_led_status(machine(), 2, data & 0x04);	// stop_3
	set_led_status(machine(), 3, data & 0x08);	// stop
	// data & 0x10?

	m_out[2] = data;
	show_out(m_out);
}

WRITE8_MEMBER(spoker_state::spoker_magic_w)
{

	m_igs_magic[offset] = data;

	if (offset == 0)
		return;

	switch(m_igs_magic[0])
	{
		case 0x01:
			break;

		default:
//          popmessage("magic %x <- %04x",igs_magic[0],data);
			logerror("%06x: warning, writing to igs_magic %02x = %02x\n", cpu_get_pc(&space.device()), m_igs_magic[0], data);
	}
}

READ8_MEMBER(spoker_state::spoker_magic_r)
{

	switch(m_igs_magic[0])
	{
		case 0x00:
			if ( !(m_igs_magic[1] & 0x01) )	return ioport("DSW1")->read();
			if ( !(m_igs_magic[1] & 0x02) )	return ioport("DSW2")->read();
			if ( !(m_igs_magic[1] & 0x04) )	return ioport("DSW3")->read();
			if ( !(m_igs_magic[1] & 0x08) )	return ioport("DSW4")->read();
			if ( !(m_igs_magic[1] & 0x10) )	return ioport("DSW5")->read();
			logerror("%06x: warning, reading dsw with igs_magic[1] = %02x\n", cpu_get_pc(&space.device()), m_igs_magic[1]);
			break;

		default:
			logerror("%06x: warning, reading with igs_magic = %02x\n", cpu_get_pc(&space.device()), m_igs_magic[0]);
	}

	return 0;
}




static ADDRESS_MAP_START( spoker_map, AS_PROGRAM, 8, spoker_state )
	AM_RANGE( 0x00000, 0x0f3ff ) AM_ROM
	AM_RANGE( 0x0f400, 0x0ffff ) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( spoker_portmap, AS_IO, 8, spoker_state )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // Z180 internal regs

	AM_RANGE( 0x2000, 0x23ff ) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_byte_split_lo_w ) AM_SHARE("paletteram")
	AM_RANGE( 0x2400, 0x27ff ) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_byte_split_hi_w ) AM_SHARE("paletteram2")

	AM_RANGE( 0x3000, 0x33ff ) AM_RAM_WRITE(bg_tile_w ) AM_SHARE("bg_tile_ram")

	AM_RANGE( 0x5000, 0x5fff ) AM_RAM_WRITE(fg_tile_w )  AM_SHARE("fg_tile_ram")

	/* TODO: ppi #1 */
	AM_RANGE( 0x6480, 0x6480 ) AM_WRITE(spoker_nmi_and_coins_w )
	AM_RANGE( 0x6481, 0x6481 ) AM_READ_PORT( "SERVICE" )
	AM_RANGE( 0x6482, 0x6482 ) AM_READ_PORT( "COINS" )

	/* TODO: ppi #2 */
	AM_RANGE( 0x6490, 0x6490 ) AM_READ_PORT( "BUTTONS1" )
	AM_RANGE( 0x6491, 0x6491 ) AM_WRITE(spoker_video_and_leds_w )
	AM_RANGE( 0x6492, 0x6492 ) AM_WRITE(spoker_leds_w )

	AM_RANGE( 0x64a0, 0x64a0 ) AM_READ_PORT( "BUTTONS2" )

	AM_RANGE( 0x64b0, 0x64b1 ) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w )

	AM_RANGE( 0x64c0, 0x64c0 ) AM_DEVREADWRITE("oki", okim6295_device, read, write)

	AM_RANGE( 0x64d0, 0x64d1 ) AM_READWRITE(spoker_magic_r, spoker_magic_w )	// DSW1-5

	AM_RANGE( 0x7000, 0x7fff ) AM_RAM_WRITE(fg_color_w ) AM_SHARE("fg_color_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( 3super8_portmap, AS_IO, 8, spoker_state )
//  AM_RANGE( 0x1000, 0x1fff ) AM_WRITENOP

	AM_RANGE( 0x2000, 0x27ff ) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_byte_split_lo_w ) AM_SHARE("paletteram")
	AM_RANGE( 0x2800, 0x2fff ) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_byte_split_hi_w ) AM_SHARE("paletteram2")

	AM_RANGE( 0x3000, 0x33ff ) AM_RAM_WRITE(bg_tile_w ) AM_SHARE("bg_tile_ram")

	AM_RANGE( 0x4000, 0x4000 ) AM_READ_PORT( "DSW1" )
	AM_RANGE( 0x4001, 0x4001 ) AM_READ_PORT( "DSW2" )
	AM_RANGE( 0x4002, 0x4002 ) AM_READ_PORT( "DSW3" )
	AM_RANGE( 0x4003, 0x4003 ) AM_READ_PORT( "DSW4" )
	AM_RANGE( 0x4004, 0x4004 ) AM_READ_PORT( "DSW5" )

//  AM_RANGE( 0x4000, 0x40ff ) AM_WRITENOP

	AM_RANGE( 0x5000, 0x5fff ) AM_RAM_WRITE(fg_tile_w )  AM_SHARE("fg_tile_ram")

	AM_RANGE( 0x6480, 0x6480 ) AM_READ_PORT( "IN0" )
	AM_RANGE( 0x6490, 0x6490 ) AM_READ_PORT( "IN1" )
	AM_RANGE( 0x6491, 0x6491 ) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE( 0x64a0, 0x64a0 ) AM_READ_PORT( "IN2" )
	AM_RANGE( 0x64b0, 0x64b0 ) AM_WRITE(spoker_leds_w )
	AM_RANGE( 0x64c0, 0x64c0 ) AM_READNOP //irq ack?

	AM_RANGE( 0x64f0, 0x64f0 ) AM_WRITE(spoker_nmi_and_coins_w )

	AM_RANGE( 0x7000, 0x7fff ) AM_RAM_WRITE(fg_color_w ) AM_SHARE("fg_color_ram")
ADDRESS_MAP_END


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( spoker )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN( 0xff, 0xff )

	PORT_START("DSW5")
	PORT_DIPUNKNOWN( 0xff, 0xff )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Memory Clear")	// stats, memory
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL  ) PORT_CUSTOM_MEMBER(DEVICE_SELF,spoker_state,hopper_r, (void *)0 ) PORT_NAME("HPSW")	// hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( 3super8 )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN( 0xff, 0xff )

	PORT_START("DSW5")
	PORT_DIPUNKNOWN( 0xff, 0xff )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL  ) PORT_CUSTOM_MEMBER(DEVICE_SELF,spoker_state,hopper_r, (void *)0 ) PORT_NAME("HPSW")	// hopper sensor
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )
INPUT_PORTS_END

/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x8x6 =
{
	8, 8,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
	  RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
	  RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,2*8) },
	8*8*2
};

static const gfx_layout layout_8x32x6 =
{
	8, 32,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
	  RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
	  RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP32(0,2*8) },
	8*32*2
};

static const gfx_layout layout3s8_8x8x6 =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(0,3)+2*8, RGN_FRAC(0,3)+0, RGN_FRAC(1,3)+2*8, RGN_FRAC(1,3)+0,RGN_FRAC(2,3)+2*8, RGN_FRAC(2,3)+0 },
	{ 0, 1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 4*8, 5*8, 8*8, 9*8,  12*8, 13*8 },
	16*8
};

static GFXDECODE_START( spoker )
	GFXDECODE_ENTRY( "gfx1", 0x00000, layout_8x8x6,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x04000, layout_8x32x6,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, layout_8x32x6,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0c000, layout_8x32x6,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, layout_8x32x6,  0, 16 )
GFXDECODE_END

static GFXDECODE_START( 3super8 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, layout3s8_8x8x6,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x04000, layout_8x32x6,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, layout_8x32x6,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0c000, layout_8x32x6,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, layout_8x32x6,     0, 16 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static MACHINE_RESET( spoker )
{
	spoker_state *state = machine.driver_data<spoker_state>();

	state->m_nmi_ack		=	0;
	state->m_hopper			=	0;
	state->m_video_enable	=	1;
}

static INTERRUPT_GEN( spoker_interrupt )
{
//  spoker_state *state = device->machine().driver_data<spoker_state>();

	device_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE);
}

static MACHINE_CONFIG_START( spoker, spoker_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz / 2)	/* HD64180RP8, 8 MHz? */
	MCFG_CPU_PROGRAM_MAP(spoker_map)
	MCFG_CPU_IO_MAP(spoker_portmap)
	MCFG_CPU_VBLANK_INT("screen",spoker_interrupt)

	MCFG_MACHINE_RESET(spoker)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-16-1)
	MCFG_SCREEN_UPDATE_STATIC(spoker)

	MCFG_GFXDECODE(spoker)
	MCFG_PALETTE_LENGTH(0x400)

	MCFG_VIDEO_START(spoker)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_12MHz / 12, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( 3super8, spoker )

	MCFG_CPU_REPLACE("maincpu", Z80, XTAL_24MHz / 4)	/* z840006, 24/4 MHz? */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(spoker_map)
	MCFG_CPU_IO_MAP(3super8_portmap)
	MCFG_CPU_VBLANK_INT("screen",spoker_interrupt)
	MCFG_CPU_PERIODIC_INT(irq0_line_hold,120) // this signal comes from the PIC

	MCFG_GFXDECODE(3super8)

	MCFG_DEVICE_REMOVE("ymsnd")
MACHINE_CONFIG_END

static DRIVER_INIT( spk116it )
{
	int A;
	UINT8 *rom = machine.root_device().memregion("maincpu")->base();


	for (A = 0;A < 0x10000;A++)
	{
		rom[A] ^= 0x02;
		if ((A & 0x0208) == 0x0208) rom[A] ^= 0x20;
		if ((A & 0x0228) == 0x0008) rom[A] ^= 0x20;
		if ((A & 0x04A0) == 0x04A0) rom[A] ^= 0x02;
		if ((A & 0x1208) == 0x1208) rom[A] ^= 0x01;
	}
}

ROM_START( spk116it )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v.bin",   0x0000, 0x10000, CRC(e44e943a)  SHA1(78e32d07e2be9a452be10735641cbcf269068c55) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "6.bin",  0x80000, 0x40000, CRC(55b54b11) SHA1(decf27d40ec842374af02c93d761375690be83a3) )
	ROM_LOAD( "5.bin",  0x40000, 0x40000, CRC(163f5b64) SHA1(5d3a5c2a64691ee9e2bb3a7c283aa9efa53fb35e) )
	ROM_LOAD( "4.bin",  0x00000, 0x40000, CRC(ec2c6ac3) SHA1(e0a38da26202d2b9a481060fe5b88a38e284201e) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "3.bin",  0x20000, 0x10000, CRC(5f18b012) SHA1(c9a96237eaf3138f136bbaffb29dde0ef568ce73) )
	ROM_LOAD( "2.bin",  0x10000, 0x10000, CRC(50fc3505) SHA1(ca1e4ee7e0bb59c3bd67727f65054a48000ae7fe) )
	ROM_LOAD( "1.bin",  0x00000, 0x10000, CRC(28ce630a) SHA1(9b597073d33841e7db2c68bbe9f30b734d7f7b41) )

	ROM_REGION( 0x40000, "oki", 0 )	/* expansion rom - contains backgrounds and pictures charmaps */
	ROM_LOAD( "7.bin",   0x0000, 0x40000, CRC(67789f1c) SHA1(1bef621b4d6399f76020c6310e2e1c2f861679de) )
ROM_END

ROM_START( spk115it )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v(__115).bin",   0x0000, 0x10000, CRC(df52997b) SHA1(72a76e84aeedfdebd4c6cb47809117a28b5d3892) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "6(__115).bin",  0x80000, 0x40000, CRC(f9b027f8) SHA1(c4686a4024062482f9864e0445087e32899fc775) )
	ROM_LOAD( "5(__115).bin",  0x40000, 0x40000, CRC(baca51b6) SHA1(c97322c814729332378b6304a79062fea385ca97) )
	ROM_LOAD( "4(__115).bin",  0x00000, 0x40000, CRC(1172c790) SHA1(43f1d019ecae5c605722e3fe77ae2f022b01260b) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "3.bin",  0x20000, 0x10000, CRC(5f18b012) SHA1(c9a96237eaf3138f136bbaffb29dde0ef568ce73) )
	ROM_LOAD( "2.bin",  0x10000, 0x10000, CRC(50fc3505) SHA1(ca1e4ee7e0bb59c3bd67727f65054a48000ae7fe) )
	ROM_LOAD( "1.bin",  0x00000, 0x10000, CRC(28ce630a) SHA1(9b597073d33841e7db2c68bbe9f30b734d7f7b41) )

	ROM_REGION( 0x40000, "oki", 0 )	/* expansion rom - contains backgrounds and pictures charmaps */
	ROM_LOAD( "7.bin",   0x0000, 0x40000, CRC(67789f1c) SHA1(1bef621b4d6399f76020c6310e2e1c2f861679de) )
ROM_END

/*

Produttore  ?Italy?
N.revisione
CPU

1x 24mhz osc
2x fpga
1x z840006
1x PIC16c65a-20/p
1x 6295 oki

ROMs

Note

4x 8 dipswitch
1x 4 dispwitch

*/

// all gfx / sound roms are bad.  they're definitely meant to have different data
//  in each half, and maybe even be twice the size.
//  in all cases the first half is missing (the sample table in the samples rom for example)
//1.bin                                           1ST AND 2ND HALF IDENTICAL
//2.bin                                           1ST AND 2ND HALF IDENTICAL
//3.bin                                           1ST AND 2ND HALF IDENTICAL
//sound.bin                                       1ST AND 2ND HALF IDENTICAL


ROM_START( 3super8 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "prgrom.bin", 0x00000, 0x20000, CRC(37c85dfe) SHA1(56bd2fb859b17dda1e675a385b6bcd6867ecceb0)  )

	ROM_REGION( 0x1000, "pic", 0 )
	ROM_LOAD( "pic16c65a-20-p", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "1(__baddump).bin", 0x00000, 0x40000, BAD_DUMP CRC(d9d3e21e) SHA1(2f3f07ca427d9f56f0ff143d15d95cbf15255e33) )
	ROM_LOAD( "2(__baddump).bin", 0x40000, 0x40000, BAD_DUMP CRC(fbb50ab1) SHA1(50a7ef9219c38d59117c510fe6d53fb3ba1fa456) )
	ROM_LOAD( "3(__baddump).bin", 0x80000, 0x40000, BAD_DUMP CRC(545aa4e6) SHA1(3348d4b692900c9e9cd4a52b20922a84e596cd35) )
	ROM_FILL( 0x00000 ,0x20000, 0x00 )
	ROM_FILL( 0x40000 ,0x20000, 0x00 )
	ROM_FILL( 0x80000 ,0x20000, 0x00 )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )

	ROM_REGION( 0xc0000, "rep_gfx", 0 ) //not real, taken from spk116it
	ROM_LOAD( "4.bin",  0x00000, 0x40000, BAD_DUMP CRC(ec2c6ac3) SHA1(e0a38da26202d2b9a481060fe5b88a38e284201e) )
	ROM_LOAD( "5.bin",  0x40000, 0x40000, BAD_DUMP CRC(163f5b64) SHA1(5d3a5c2a64691ee9e2bb3a7c283aa9efa53fb35e) )
	ROM_LOAD( "6.bin",  0x80000, 0x40000, BAD_DUMP CRC(55b54b11) SHA1(decf27d40ec842374af02c93d761375690be83a3) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sound.bin", 0x00000, 0x40000, BAD_DUMP CRC(230b31c3) SHA1(38c107325d3a4e9781912078b1317dc9ba3e1ced) )
ROM_END

static DRIVER_INIT( 3super8 )
{
	UINT8 *ROM = machine.root_device().memregion("maincpu")->base();
	int i;

	/* Decryption is probably done using one macrocell/output on an address decoding pal which we do not have a dump of */
	/* The encryption is quite awful actually, especially since the program rom is entirely blank/0xFF but encrypted on its second half, exposing the entire function in plaintext */
	/* Input: A6, A7, A8, A9, A11; Output: D5 XOR */
	/* function: (A6&A8)&((!A7&A11)|(A9&!A11)); */
	/* nor-reduced: !(!(!(!A6|!A8))|!(!(A7|!A11)|!(!A9|A11))); */
	for(i=0;i<0x20000;i++)
	{
		UINT8 a6, a7, a8, a9, a11, d5 = 0;
		a6 = BIT(i,6); a7 = BIT(i,7); a8 = BIT(i,8); a9 = BIT(i,9); a11 = BIT(i,11);
		d5 = (a6&a8)&((!a7&a11)|(a9&!a11));
		ROM[i] ^= d5*0x20;
	}

	/* cheesy hack: take gfx roms from spk116it and rearrange them for this game needs */
	{
		UINT8 *src = machine.root_device().memregion("rep_gfx")->base();
		UINT8 *dst = machine.root_device().memregion("gfx1")->base();
		UINT8 x;

		for(x=0;x<3;x++)
		{
			for(i=0;i<0x20000;i+=4)
			{
				dst[i+0+x*0x40000] = src[i+0+x*0x40000];
				dst[i+1+x*0x40000] = src[i+2+x*0x40000];
				dst[i+2+x*0x40000] = src[i+1+x*0x40000];
				dst[i+3+x*0x40000] = src[i+3+x*0x40000];
			}
		}
	}
}

GAME( 1993?, spk116it, 0,        spoker, spoker,  spk116it, ROT0, "IGS",       "Super Poker (v116IT)", 0 )
GAME( 1993?, spk115it, spk116it, spoker, spoker,  spk116it, ROT0, "IGS",       "Super Poker (v115IT)", 0 )
GAME( 1993?, 3super8,  spk116it, 3super8,3super8, 3super8,  ROT0, "<unknown>", "3 Super 8 (Italy)",    GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND ) //roms are badly dumped
