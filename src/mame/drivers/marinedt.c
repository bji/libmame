/*
---------------------------
Marine Date by TAITO (1981)
---------------------------

Location     Device      File ID     Checksum
---------------------------------------------
LB 3D         2716        MG01         BB4B
LB 4D         2716        MG02         89B3
LB 5D         2716        MG03         A5CE
LB 6D         2716        MG04         CE20
LB 7D         2716        MG05         16B9
LB 9D         2716        MG06         39A9
LB 10D        2716        MG07         B7F1
LB 1F         2716        MG09         9934
LB 3F         2716        MG10         F185
LB 4F         2716        MG11         1603
MB 6C         2532        MG12         66C3
MB 6H         2532        MG13         23E2
MB 2A       82S123        MG14.BPR     1CB1
MB 1A       82S123        MG15.BPR     1471
MB 4E       82S123        MG16.BPR     0570
TB 5F       82S123        MG17.BPR     129B


Notes:     TB - Top PCB        MG070001  MGN00001
           MB - Middle PCB     MG070002  MGN00002
           LB - Lower PCB      AA017779  MGN00002


Brief Hardware Overview
-----------------------

Main processor    -  Z80  2.5MHz

Sound             - Discrete audio, like Space Invaders

-------------------------------------------------------------------------

a static underwater scence with obstacles in it, like seaweed,
crabs and other stuff.  You have a limited number of "strokes"
per screen as well as a timer to work against.  Your goal is
to *bounce* yourself around the screen using *Strokes* on the
trackball to try to reach a *female* octopus before your run out
of strokes or time.  You sort of bounce yourself around the screen
like a billiard ball would bounce, but once in a while bubbles
and other stuff will come up from underneath you and carry you
away from where you are trying to get.  When you reach your goal
you get another more difficult screen, etc.

I think it was manufactured by Taito, I'm not sure but I seem to
recall that it was a full blown Japanese machine.


todo:
in cocktail mopde p1 is flipped
after inking the shark on the far right octi was moved to goal?
for the colours, goal has to be black otherwise it would register
    qas a hit, is goal pen 0 or 6?
rom writes when finishing a game
    worth looking at before the collision is correct?
playing dot hit when eaten by a shark?
dont use any ints, s/b UINT8?
enemy sprite not disabled at end of game
tilemap
palette may only be around 4 colours
    is 14 the palette?
how do you know if you've got an ink left?
prom 14 is the top bits? 4 bpp? or so?
why is level 37 chosen?
should it be 30fps?
    check other taito games of the time
look at other taito 1981 games for ideas on the ports
    bking
    jhunt?
"Marine Deto" or "Marine Date"
    look in the roms for all the text
simplify gfx decode
why does the player sprite need 4 colours?
    check if more than 1 are used
check service test ram wipes for confirmation of ram spots
    anything after trackball test?
obj1 to obj2 draw order
2nd trackball
flip/cocktail issues

done:
timer?
    you get 200 for each shot, don't think it's actually a timer
have i been using x/y consistently, ie non rotated or rotated origin?
    yes, seems to be best using xy raw (ie non-rotated)
p2 ink doesn't always light up in test mode
    after p1 ink pressed, p2 ink doesn't light up
    this is correct behavior if DSW set as Upright mode
*/

#include "emu.h"
#include "cpu/z80/z80.h"

class marinedt_state : public driver_device
{
public:
	marinedt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_tx_tileram(*this, "tx_tileram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_tx_tileram;

	/* video-related */
	bitmap_ind16 *m_tile;
	bitmap_ind16 *m_obj1;
	bitmap_ind16 *m_obj2;
	tilemap_t *m_tx_tilemap;

	UINT8 m_obj1_a;
	UINT8 m_obj1_x;
	UINT8 m_obj1_y;
	UINT8 m_obj2_a;
	UINT8 m_obj2_x;
	UINT8 m_obj2_y;
	UINT8 m_pd;
	UINT8 m_pf;
	UINT8 m_music;
	UINT8 m_sound;
	UINT8 m_coll;
	UINT8 m_cx;
	UINT8 m_cyr;
	UINT8 m_cyq;
	UINT8 m_collh;
	UINT8 m_cxh;
	UINT8 m_cyrh;
	UINT8 m_cyqh;
	DECLARE_WRITE8_MEMBER(tx_tileram_w);
	DECLARE_READ8_MEMBER(marinedt_port1_r);
	DECLARE_READ8_MEMBER(marinedt_coll_r);
	DECLARE_READ8_MEMBER(marinedt_obj1_x_r);
	DECLARE_READ8_MEMBER(marinedt_obj1_yr_r);
	DECLARE_READ8_MEMBER(marinedt_obj1_yq_r);
	DECLARE_WRITE8_MEMBER(marinedt_obj1_a_w);
	DECLARE_WRITE8_MEMBER(marinedt_obj1_x_w);
	DECLARE_WRITE8_MEMBER(marinedt_obj1_y_w);
	DECLARE_WRITE8_MEMBER(marinedt_obj2_a_w);
	DECLARE_WRITE8_MEMBER(marinedt_obj2_x_w);
	DECLARE_WRITE8_MEMBER(marinedt_obj2_y_w);
	DECLARE_WRITE8_MEMBER(marinedt_music_w);
	DECLARE_WRITE8_MEMBER(marinedt_sound_w);
	DECLARE_WRITE8_MEMBER(marinedt_pd_w);
	DECLARE_WRITE8_MEMBER(marinedt_pf_w);
};


WRITE8_MEMBER(marinedt_state::tx_tileram_w)
{

	m_tx_tileram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

READ8_MEMBER(marinedt_state::marinedt_port1_r)
{

	//might need to be reversed for cocktail stuff

	/* x/y multiplexed */
	return ioport(((m_pf & 0x08) >> 3) ? "TRACKY" : "TRACKX")->read();
}

READ8_MEMBER(marinedt_state::marinedt_coll_r)
{
	//76543210
	//x------- obj1 to obj2 collision
	//-xxx---- unused
	//----x--- obj1 to playfield collision
	//-----xxx unused

	return m_coll | m_collh;
}

//are these returning only during a collision?
//id imagine they are returning the pf char where the collission took place?
//what about where there is lots of colls?
//maybe the first on a scanline basis
READ8_MEMBER(marinedt_state::marinedt_obj1_x_r)
{
	//76543210
	//xxxx---- unknown
	//----xxxx x pos in tile ram

	UINT8 *RAM = memregion("maincpu")->base();

	if (RAM[0x430e])
		--m_cx;
	else
		++m_cx;

	//figure out why inc/dec based on 430e?
	return m_cx | (m_cxh << 4);
}

READ8_MEMBER(marinedt_state::marinedt_obj1_yr_r)
{
	//76543210
	//xxxx---- unknown
	//----xxxx row in current screen quarter


	//has to be +1 if cx went over?
	if (m_cx == 0x10)
		m_cyr++;

	return m_cyr | (m_cyrh << 4);
}

READ8_MEMBER(marinedt_state::marinedt_obj1_yq_r)
{
	//76543210
	//xx------ unknown
	//--xx---- screen quarter when flipped?
	//----xx-- unknown
	//------xx screen quarter

	return m_cyq | (m_cyqh << 4);
}

WRITE8_MEMBER(marinedt_state::marinedt_obj1_a_w){ m_obj1_a = data; }
WRITE8_MEMBER(marinedt_state::marinedt_obj1_x_w){ m_obj1_x = data; }
WRITE8_MEMBER(marinedt_state::marinedt_obj1_y_w){ m_obj1_y = data; }
WRITE8_MEMBER(marinedt_state::marinedt_obj2_a_w){ m_obj2_a = data; }
WRITE8_MEMBER(marinedt_state::marinedt_obj2_x_w){ m_obj2_x = data; }
WRITE8_MEMBER(marinedt_state::marinedt_obj2_y_w){ m_obj2_y = data; }

WRITE8_MEMBER(marinedt_state::marinedt_music_w){ m_music = data; }

WRITE8_MEMBER(marinedt_state::marinedt_sound_w)
{
	//76543210
	//xx------ ??
	//--x----- jet sound
	//---x---- foam
	//----x--- ink
	//-----x-- collision
	//------x- dots hit
	//-------x ??

	m_sound = data;
}

WRITE8_MEMBER(marinedt_state::marinedt_pd_w)
{
	//76543210
	//xxx----- ?? unused
	//---x---- ?? the rest should be used based on the table
	//----x--- ??
	//-----x-- ??
	//------x- obj2 enable
	//-------x obj1 enable

	m_pd = data;
}

/*
upright
marinedt_pf_w: 00   // upright
marinedt_pf_w: 01   // ??

cocktail
marinedt_pf_w: 02   // cocktail
marinedt_pf_w: 03   // ??

marinedt_pf_w: 01   // upright
marinedt_pf_w: 05   // flip sprite?

marinedt_pf_w: 07   // cocktail
marinedt_pf_w: 03   // non-flip sprite?
*/
WRITE8_MEMBER(marinedt_state::marinedt_pf_w)
{
	//76543210
	//xxxx---- ?? unused (will need to understand table of written values)
	//----x--- xy trackball select
	//-----x-- ?? flip screen/controls
	//------x- ?? upright/cocktail
	//-------x ?? service mode (coin lockout??)


	//if ((m_pf & 0x07) != (data & 0x07))
	//  mame_printf_debug("marinedt_pf_w: %02x\n", data & 0x07);

	if ((m_pf & 0x02) != (data & 0x02))
	{
		if (data & 0x02)
			mame_printf_debug("tile flip\n");
		else
			mame_printf_debug("tile non-flip\n");

		if (data & 0x02)
			m_tx_tilemap->set_flip(TILEMAP_FLIPX | TILEMAP_FLIPY);
		else
			m_tx_tilemap->set_flip(0);
	}

	m_pf = data;

	//if (data & 0xf0)
	//    logerror("pf:%02x %d\n", m_pf);
	//logerror("pd:%02x %d\n", m_pd, machine().primary_screen->frame_number());

}

static ADDRESS_MAP_START( marinedt_map, AS_PROGRAM, 8, marinedt_state )
	AM_RANGE(0x0000, 0x37ff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x4400, 0x47ff) AM_RAM				//unused, vram mirror?
	AM_RANGE(0x4800, 0x4bff) AM_RAM_WRITE(tx_tileram_w) AM_SHARE("tx_tileram")
	AM_RANGE(0x4c00, 0x4c00) AM_WRITENOP	//?? maybe off by one error
ADDRESS_MAP_END

static ADDRESS_MAP_START( marinedt_io_map, AS_IO, 8, marinedt_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW0")		//dips coinage
	AM_RANGE(0x01, 0x01) AM_READ(marinedt_port1_r)	//trackball xy muxed
	AM_RANGE(0x02, 0x02) AM_READWRITE(marinedt_obj1_x_r, marinedt_obj1_a_w)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("IN0") AM_WRITE(marinedt_obj1_x_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW1") AM_WRITE(marinedt_obj1_y_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(marinedt_music_w)
	AM_RANGE(0x06, 0x06) AM_READWRITE(marinedt_obj1_yr_r, marinedt_sound_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(marinedt_obj2_a_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(marinedt_obj2_x_w)
	AM_RANGE(0x0a, 0x0a) AM_READWRITE(marinedt_obj1_yq_r, marinedt_obj2_y_w)
	AM_RANGE(0x0d, 0x0d) AM_WRITE(marinedt_pd_w)
	AM_RANGE(0x0e, 0x0e) AM_READWRITE(marinedt_coll_r, watchdog_reset_w)
	AM_RANGE(0x0f, 0x0f) AM_WRITE(marinedt_pf_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( marinedt )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )
//cheat?
	PORT_DIPNAME( 0x02, 0x00, "ignore internal bounce?" )	//maybe die/bounce of rocks/coral?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
//freezes the game before the reset
//doesn't seem to be done as a dip, but what about mixing with diops like this?
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x00, "Coin Chutes" )
	PORT_DIPSETTING(    0x00, "Common" )
	PORT_DIPSETTING(    0x10, "Individual" )
	PORT_DIPNAME( 0x20, 0x00, "Year Display" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "6" )

	PORT_START("TRACKX")	/* FAKE MUXED */
//check all bits are used
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("TRACKY")	/* FAKE MUXED */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)
INPUT_PORTS_END

static const gfx_layout marinedt_charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },	//maybe 120
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout marinedt_objlayout =
{
	32,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(32*8*7,1), STEP4(32*8*6,1), STEP4(32*8*5,1), STEP4(32*8*4,1), STEP4(32*8*3,1), STEP4(32*8*2,1), STEP4(32*8*1,1), STEP4(32*8*0,1) },
	{ STEP16(0,8), STEP16(16*8,8) },
	32*32*2
};

static GFXDECODE_START( marinedt )
	GFXDECODE_ENTRY( "gfx1", 0, marinedt_charlayout, 0,  4 )	//really only 1 colour set?
	GFXDECODE_ENTRY( "gfx2", 0, marinedt_objlayout,  48, 4 )
	GFXDECODE_ENTRY( "gfx3", 0, marinedt_objlayout,  32, 4 )
GFXDECODE_END

static PALETTE_INIT( marinedt )
{
	const UINT8 *color_prom = machine.root_device().memregion("proms")->base();
	int i,r,b,g;

	for (i = 0; i < machine.total_colors(); i++)
	{
		int bit0, bit1, bit2;

		/* red component */
		bit0 = (~color_prom[i] >> 0) & 0x01;
		bit1 = (~color_prom[i] >> 1) & 0x01;
		bit2 = (~color_prom[i] >> 2) & 0x01;
//      *(palette++) = 0x92 * bit0 + 0x46 * bit1 + 0x27 * bit2;
		r = 0x27 * bit0 + 0x46 * bit1 + 0x92 * bit2;
		/* green component */
		bit0 = (~color_prom[i] >> 3) & 0x01;
		bit1 = (~color_prom[i] >> 4) & 0x01;
		bit2 = (~color_prom[i] >> 5) & 0x01;
//      *(palette++) = 0x92 * bit0 + 0x46 * bit1 + 0x27 * bit2;
		g = 0x27 * bit0 + 0x46 * bit1 + 0x92 * bit2;
		/* blue component */
		bit0 = (~color_prom[i] >> 5) & 0x01;
		bit1 = (~color_prom[i] >> 6) & 0x01;
		bit2 = (~color_prom[i] >> 7) & 0x01;
bit0 = 0;
//      *(palette++) = 0x92 * bit0 + 0x46 * bit1 + 0x27 * bit2;
		b = 0x27 * bit0 + 0x46 * bit1 + 0x92 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


static TILE_GET_INFO( get_tile_info )
{
	marinedt_state *state = machine.driver_data<marinedt_state>();
	int code = state->m_tx_tileram[tile_index];
	int color = 0;
	int flags = TILE_FLIPX;

	SET_TILE_INFO(0, code, color, flags);
}

static VIDEO_START( marinedt )
{
	marinedt_state *state = machine.driver_data<marinedt_state>();
	state->m_tx_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_tx_tilemap->set_transparent_pen(0);
	state->m_tx_tilemap->set_scrolldx(0, 4*8);
	state->m_tx_tilemap->set_scrolldy(0, -4*8);

	state->m_tile = auto_bitmap_ind16_alloc(machine, 32 * 8, 32 * 8);
	state->m_obj1 = auto_bitmap_ind16_alloc(machine, 32, 32);
	state->m_obj2 = auto_bitmap_ind16_alloc(machine, 32, 32);
}


// x------- flipy
// -x------ unused ??
// --xxx--- sprite code
// -----x-- bank
// ------xx colour

#define OBJ_CODE(a)	((((a) & 0x04) << 1) + (((a) & 0x38) >> 3))
#define OBJ_COLOR(a)	((a) & 0x03)
#define OBJ_X(x)	(256 - 32 - (x))
#define OBJ_Y(y)	(256 - 1 - (y))
#define OBJ_FLIPX(a)	((state->m_pf & 0x02) == 0)
#define OBJ_FLIPY(a)	((a) & 0x80)

static SCREEN_UPDATE_IND16( marinedt )
{
	marinedt_state *state = screen.machine().driver_data<marinedt_state>();
	int sx, sy;

	state->m_tile->fill(0);
	state->m_tx_tilemap->draw(*state->m_tile, cliprect, 0, 0);

	state->m_obj1->fill(0);
	drawgfx_transpen(*state->m_obj1, state->m_obj1->cliprect(), screen.machine().gfx[1],
			OBJ_CODE(state->m_obj1_a),
			OBJ_COLOR(state->m_obj1_a),
			OBJ_FLIPX(state->m_obj1_a), OBJ_FLIPY(state->m_obj1_a),
			0, 0, 0);

	state->m_obj2->fill(0);
	drawgfx_transpen(*state->m_obj2, state->m_obj2->cliprect(), screen.machine().gfx[2],
			OBJ_CODE(state->m_obj2_a),
			OBJ_COLOR(state->m_obj2_a),
			OBJ_FLIPX(state->m_obj2_a), OBJ_FLIPY(state->m_obj2_a),
			0, 0, 0);

	bitmap.fill(0);

	if (state->m_pd & 0x02)
		copybitmap_trans(bitmap, *state->m_obj2, 0, 0, OBJ_X(state->m_obj2_x), OBJ_Y(state->m_obj2_y), cliprect, 0);

	if (state->m_pd & 0x01)
		copybitmap_trans(bitmap, *state->m_obj1, 0, 0, OBJ_X(state->m_obj1_x), OBJ_Y(state->m_obj1_y), cliprect, 0);

	copybitmap_trans(bitmap, *state->m_tile, 0, 0, 0, 0, cliprect, 0);

	state->m_coll = state->m_cx = state->m_cyr = state->m_cyq = 0;
	if (state->m_pd & 0x01)
	{
		for (sx = 0; sx < 32; sx++)
			for (sy = 0; sy < 32; sy++)
			{
				int x = OBJ_X(state->m_obj1_x) + sx;
				int y = OBJ_Y(state->m_obj1_y) + sy;

				if (!cliprect.contains(x, y))
					continue;

				if (state->m_obj1->pix16(sy, sx) == 0)
					continue;

				if (state->m_tile->pix16(y, x) != 0)
				{
					state->m_coll = 0x08;

					state->m_cx = (x % 128) / 8;
					state->m_cx &= 0x0f;

					state->m_cyr = ((y % 64) / 8) * 2 + (x > 127 ? 1 : 0);
					state->m_cyr &= 0x0f;

					state->m_cyq = y / 64;
					state->m_cyq &= 0x0f;

					break;
				}
			}
	}

	state->m_collh = state->m_cxh = state->m_cyrh = state->m_cyqh = 0;
	if ((state->m_pd & 0x03) == 0x03)
	{
		for (sx = 0; sx < 32; sx++)
			for (sy = 0; sy < 32; sy++)
			{
				int x = OBJ_X(state->m_obj1_x + sx);
				int y = OBJ_Y(state->m_obj1_y + sy);

				int xx = OBJ_X(state->m_obj2_x) - x;
				int yy = OBJ_Y(state->m_obj2_y) - y;

				if (xx < 0 || xx >= 32 || yy < 0 || yy >= 32)
					continue;

				if (state->m_obj1->pix16(sy, sx) == 0)
					continue;

				if (state->m_obj2->pix16(yy, xx) != 0)
				{
					state->m_collh = 0x80;

					state->m_cxh = (x % 128) / 8;
					state->m_cxh &= 0x0f;

					state->m_cyrh = ((y % 64) / 8) * 2 + (x > 127 ? 1 : 0);
					state->m_cyrh &= 0x0f;

					state->m_cyqh= y / 64;
					state->m_cyqh &= 0x0f;

					break;
				}
			}
	}
	return 0;
}

static MACHINE_START( marinedt )
{
	marinedt_state *state = machine.driver_data<marinedt_state>();

	state->save_item(NAME(state->m_obj1_a));
	state->save_item(NAME(state->m_obj1_x));
	state->save_item(NAME(state->m_obj1_y));
	state->save_item(NAME(state->m_obj2_a));
	state->save_item(NAME(state->m_obj2_x));
	state->save_item(NAME(state->m_obj2_y));
	state->save_item(NAME(state->m_pd));
	state->save_item(NAME(state->m_pf));
	state->save_item(NAME(state->m_music));
	state->save_item(NAME(state->m_sound));
	state->save_item(NAME(state->m_coll));
	state->save_item(NAME(state->m_cx));
	state->save_item(NAME(state->m_cyr));
	state->save_item(NAME(state->m_cyq));
	state->save_item(NAME(state->m_collh));
	state->save_item(NAME(state->m_cxh));
	state->save_item(NAME(state->m_cyrh));
	state->save_item(NAME(state->m_cyqh));
}

static MACHINE_RESET( marinedt )
{
	marinedt_state *state = machine.driver_data<marinedt_state>();

	state->m_obj1_a = 0;
	state->m_obj1_x = 0;
	state->m_obj1_y = 0;
	state->m_obj2_a = 0;
	state->m_obj2_x = 0;
	state->m_obj2_y = 0;
	state->m_pd = 0;
	state->m_pf = 0;
	state->m_music = 0;
	state->m_sound = 0;
	state->m_coll = 0;
	state->m_cx = 0;
	state->m_cyr = 0;
	state->m_cyq = 0;
	state->m_collh = 0;
	state->m_cxh = 0;
	state->m_cyrh = 0;
	state->m_cyqh = 0;
}

static MACHINE_CONFIG_START( marinedt, marinedt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,10000000/4)
	MCFG_CPU_PROGRAM_MAP(marinedt_map)
	MCFG_CPU_IO_MAP(marinedt_io_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_MACHINE_START(marinedt)
	MCFG_MACHINE_RESET(marinedt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(4*8+32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(marinedt)

	MCFG_GFXDECODE(marinedt)
	MCFG_PALETTE_LENGTH(64)

	MCFG_PALETTE_INIT(marinedt)
	MCFG_VIDEO_START(marinedt)

	/* sound hardware */
	//discrete sound
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( marinedt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mg01",     0x0000, 0x0800, CRC(ad09f04d) SHA1(932fc973b4a2fbbebd7e6437ed30c8444e3d4afb))
	ROM_LOAD( "mg02",     0x0800, 0x0800, CRC(555a2b0f) SHA1(143a8953ce5070c31dc4c1f623833b2a5a2cf657))
	ROM_LOAD( "mg03",     0x1000, 0x0800, CRC(2abc79b3) SHA1(1afb331a2c0e320b6d026bc5cb47a53ac3356c2a))
	ROM_LOAD( "mg04",     0x1800, 0x0800, CRC(be928364) SHA1(8d9ae71e2751c009187e41d84fbad9519ab551e1) )
	ROM_LOAD( "mg05",     0x2000, 0x0800, CRC(44cd114a) SHA1(833165c5c00c6e505acf29fef4a3ae3f9647b443) )
	ROM_LOAD( "mg06",     0x2800, 0x0800, CRC(a7e2c69b) SHA1(614fc479d13c1726382fe7b4b0379c1dd4915af0) )
	ROM_LOAD( "mg07",     0x3000, 0x0800, CRC(b85d1f9a) SHA1(4fd3e76b1816912df84477dba4655d395f5e7072) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "mg09",     0x0000, 0x0800, CRC(f4c349ca) SHA1(077f65eeac616a778d6c42bb95677fa2892ab697) )
	ROM_LOAD( "mg10",     0x0800, 0x0800, CRC(b41251e3) SHA1(e125a971b401c78efeb4b03d0fab43e392d3fc14) )
	ROM_LOAD( "mg11",     0x1000, 0x0800, CRC(50d66dd7) SHA1(858d1d2a75e091b0e382d964c5e4ddcd8e6f07dd))

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "mg12",     0x0000, 0x1000, CRC(7c6486d5) SHA1(a7f17a803937937f05fc90621883a0fd44b297a0) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "mg13",     0x0000, 0x1000, CRC(17817044) SHA1(8c9b96620e3c414952e6d85c6e81b0df85c88e7a) )

	ROM_REGION( 0x0080, "proms", 0 )
	ROM_LOAD( "mg14.bpr", 0x0000, 0x0020, CRC(f75f4e3a) SHA1(36e665987f475c57435fa8c224a2a3ce0c5e672b) )	//char clr
	ROM_LOAD( "mg15.bpr", 0x0020, 0x0020, CRC(cd3ab489) SHA1(a77478fb94d0cf8f4317f89cc9579def7c294b4f) )	//obj clr
	ROM_LOAD( "mg16.bpr", 0x0040, 0x0020, CRC(92c868bc) SHA1(483ae6f47845ddacb701528e82bd388d7d66a0fb) )	//?? collisions
	ROM_LOAD( "mg17.bpr", 0x0060, 0x0020, CRC(13261a02) SHA1(050edd18e4f79d19d5206f55f329340432fd4099) )	//?? table of increasing values
ROM_END

GAME( 1981, marinedt, 0, marinedt, marinedt, 0, ROT270, "Taito", "Marine Date", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
