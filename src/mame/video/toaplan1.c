/***************************************************************************

  Functions to emulate the video hardware of some Toaplan games,
  which use the BCU-2 tile controller, and the FCU-2 Sprite controller -
  and SCU Sprite controller (Only Rally Bike uses the SCU controller).


  There are 4 scrolling layers of graphics, stored in planes of 64x64 tiles.
  Each tile in each plane is assigned a priority between 1 and 15, higher
  numbers have greater priority.

 BCU controller. Each tile takes up 32 bits - the format is:

  0         1         2         3
  ---- ---- ---- ---- -ttt tttt tttt tttt = Tile number (0 - $7fff)
  ---- ---- ---- ---- h--- ---- ---- ---- = Hidden
  ---- ---- --cc cccc ---- ---- ---- ---- = Color (0 - $3f)
  pppp ---- ---- ---- ---- ---- ---- ---- = Priority (0-$f)
  ---- ???? ??-- ---- ---- ---- ---- ---- = Unknown / Unused

  Scroll Reg

  0         1         2         3
  xxxx xxxx x--- ---- ---- ---- ---- ---- = X position
  ---- ---- ---- ---- yyyy yyyy y--- ---- = Y position
  ---- ---- -??? ???? ---- ---- -??? ???? = Unknown / Unused



 FCU controller. Sprite RAM format  (except Rally Bike)

  0         1         2         3
  -sss ssss ssss ssss ---- ---- ---- ---- = Sprite number (0 - $7fff)
  h--- ---- ---- ---- ---- ---- ---- ---- = Hidden
  ---- ---- ---- ---- ---- ---- --cc cccc = Color (0 - $3f)
  ---- ---- ---- ---- ---- dddd dd-- ---- = Dimension (pointer to Size RAM)
  ---- ---- ---- ---- pppp ---- ---- ---- = Priority (0-$f)

  4         5         6         7
  ---- ---- ---- ---- xxxx xxxx x--- ---- = X position
  yyyy yyyy y--- ---- ---- ---- ---- ---- = Y position
  ---- ---- -??? ???? ---- ---- -??? ???? = Unknown



 SCU controller. Sprite RAM format  (Rally Bike)

  0         1         2         3
  ---- -sss ssss ssss ---- ---- ---- ---- = Sprite number (0 - $7FF)
  ---- ---- ---- ---- ---- ---- --cc cccc = Color (0 - $3F)
  ---- ---- ---- ---- ---- ---x ---- ---- = Flip X
  ---- ---- ---- ---- ---- --y- ---- ---- = Flip Y
  ---- ---- ---- ---- ---- pp-- ---- ---- = Priority (0h,4h,8h,Ch (shifted < 2 places))
  ???? ?--- ---- ---- ???? ---- ??-- ---- = Unknown / Unused

  4         5         6         7
  xxxx xxxx x--- ---- ---- ---- ---- ---- = X position
  ---- ---- ---- ---- yyyy yyyy y--- ---- = Y position
  ---- ---- -??? ???? ---- ---- -??? ???? = Unknown



  The tiles use a palette of 1024 colors, the sprites use a different palette
  of 1024 colors.


           BCU Controller writes                Tile Offsets
 Game      reg0  reg1  reg2  reg3         X     Y     flip-X  flip-Y
RallyBik   41e0  2e1e  148c  0f09        01e6  00fc     <- same --
Truxton    41e0  2717  0e86  0c06        01b7  00f2     0188  01fd
HellFire   41e0  2717  0e86  0c06        01b7  0102     0188  000d
ZeroWing   41e0  2717  0e86  0c06        01b7  0102     0188  000d
DemonWld   41e0  2e1e  148c  0f09        01a9  00fc     0196  0013
FireShrk   41e0  2717  0e86  0c06        01b7  00f2     0188  01fd
Out-Zone   41e0  2e1e  148c  0f09        01a9  00ec     0196  0003
Vimana     41e0  2717  0e86  0c06        01b7  00f2     0188  01fd


Sprites are of varying sizes between 8x8 and 128x128 with any variation
in between, in multiples of 8 either way.
Here we draw the first 8x8 part of the sprite, then by using the sprite
dimensions, we draw the rest of the 8x8 parts to produce the complete
sprite.


Abnormalities:
 How/when do priority 0 Tile layers really get displayed ?

 What are the video PROMs for ? Priority maybe ?

 Zerowing flashes red when an enemy is shot, and this is done by flipping
 layer 2 into its oversized second half which is all red. In flipscreen
 mode, this doesn't work properly as the flip scroll value doesn't equate
 properly.
 Possibly a bug with the game itself using a wrong scroll value ??
 Here's some notes:
 First values are non red flash scrolls. Second values are red flash scrolls.

 Scrolls    PF1-X  PF1-Y    PF2-X  PF2-Y    PF3-X  PF3-Y    PF4-X  PF4-Y
 ------>    #4180  #f880    #1240  #f880    #4380  #f880    #e380  #f880
 -flip->    #1500  #7f00    #e8c0  #7f00    #1300  #7f00    #bb00  #7f00

 ------>    #4100  #f880    #1200  #7880    #4300  #f880    #e380  #f880
 -flip->    #1500  #7f00    #e8c0  #8580??  #1300  #7f00    #bb00  #7f00
                                      |
                                      |
                                    f880 = 111110001xxxxxxx -> 1f1 scroll
                                    7f00 = 011111110xxxxxxx -> 0fe scroll
                                    7880 = 011110001xxxxxxx -> 0f1 scroll
                                    8580 = 100001011xxxxxxx -> 10b scroll

 So a snapshot of the scroll equations become: (from the functions below)
    1f1 - (102 - 101) == 1f0   star background
    0fe - (00d - 1ef) == 0e0   star background (flipscreen)
    0f1 - (102 - 101) == 0f0   red  background
    10b - (00d - 1ef) == 0ed   red  background (flipscreen) wrong!
    10b - (00d - 0ef) == 1ed   red  background (flipscreen) should somehow equate to this


***************************************************************************/


#include "emu.h"
#include "includes/toaplan1.h"
#include "cpu/m68000/m68000.h"


#define TOAPLAN1_TILEVRAM_SIZE       0x4000	/* 4 tile layers each this RAM size */
#define TOAPLAN1_SPRITERAM_SIZE      0x800	/* sprite ram */
#define TOAPLAN1_SPRITESIZERAM_SIZE  0x80	/* sprite size ram */


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_pf1_tile_info )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();
	int color, tile_number, attrib;

	tile_number = state->pf1_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = state->pf1_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
	if (state->pf1_tilevram16[2*tile_index+1] & 0x8000) tileinfo->pen_data = state->empty_tile;
	tileinfo->category = (attrib & 0xf000) >> 12;
}

static TILE_GET_INFO( get_pf2_tile_info )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();
	int color, tile_number, attrib;

	tile_number = state->pf2_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = state->pf2_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
	if (state->pf2_tilevram16[2*tile_index+1] & 0x8000) tileinfo->pen_data = state->empty_tile;
	tileinfo->category = (attrib & 0xf000) >> 12;
}

static TILE_GET_INFO( get_pf3_tile_info )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();
	int color, tile_number, attrib;

	tile_number = state->pf3_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = state->pf3_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
	if (state->pf3_tilevram16[2*tile_index+1] & 0x8000) tileinfo->pen_data = state->empty_tile;
	tileinfo->category = (attrib & 0xf000) >> 12;
}

static TILE_GET_INFO( get_pf4_tile_info )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();
	int color, tile_number, attrib;

	tile_number = state->pf4_tilevram16[2*tile_index+1] & 0x7fff;
	attrib = state->pf4_tilevram16[2*tile_index];
	color = attrib & 0x3f;
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
	if (state->pf4_tilevram16[2*tile_index+1] & 0x8000) tileinfo->pen_data = state->empty_tile;
	tileinfo->category = (attrib & 0xf000) >> 12;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void toaplan1_create_tilemaps(running_machine *machine)
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	state->pf1_tilemap = tilemap_create(machine, get_pf1_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->pf2_tilemap = tilemap_create(machine, get_pf2_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->pf3_tilemap = tilemap_create(machine, get_pf3_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->pf4_tilemap = tilemap_create(machine, get_pf4_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	tilemap_set_transparent_pen(state->pf1_tilemap, 0);
	tilemap_set_transparent_pen(state->pf2_tilemap, 0);
	tilemap_set_transparent_pen(state->pf3_tilemap, 0);
	tilemap_set_transparent_pen(state->pf4_tilemap, 0);

	memset(state->empty_tile, 0x00, sizeof(state->empty_tile));
}


static void toaplan1_paletteram_alloc(running_machine *machine)
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	machine->generic.paletteram.u16 = auto_alloc_array(machine, UINT16, (state->colorram1_size + state->colorram2_size)/2);

	state_save_register_global_pointer(machine, machine->generic.paletteram.u16, (state->colorram1_size + state->colorram2_size)/2);
}

static void toaplan1_vram_alloc(running_machine *machine)
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	state->pf1_tilevram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_TILEVRAM_SIZE/2);
	state->pf2_tilevram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_TILEVRAM_SIZE/2);
	state->pf3_tilevram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_TILEVRAM_SIZE/2);
	state->pf4_tilevram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_TILEVRAM_SIZE/2);

	state_save_register_global_pointer(machine, state->pf1_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);
	state_save_register_global_pointer(machine, state->pf2_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);
	state_save_register_global_pointer(machine, state->pf3_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);
	state_save_register_global_pointer(machine, state->pf4_tilevram16, TOAPLAN1_TILEVRAM_SIZE/2);

#ifdef MAME_DEBUG
	state->display_pf1 = 1;
	state->display_pf2 = 1;
	state->display_pf3 = 1;
	state->display_pf4 = 1;
	state->displog = 0;
#endif
}

static void toaplan1_spritevram_alloc(running_machine *machine)
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	state->spriteram = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_SPRITERAM_SIZE/2);
	state->buffered_spriteram = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_SPRITERAM_SIZE/2);
	state->spritesizeram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_SPRITESIZERAM_SIZE/2);
	state->buffered_spritesizeram16 = auto_alloc_array_clear(machine, UINT16, TOAPLAN1_SPRITESIZERAM_SIZE/2);

	state_save_register_global_pointer(machine, state->spriteram, TOAPLAN1_SPRITERAM_SIZE/2);
	state_save_register_global_pointer(machine, state->buffered_spriteram, TOAPLAN1_SPRITERAM_SIZE/2);
	state_save_register_global_pointer(machine, state->spritesizeram16, TOAPLAN1_SPRITESIZERAM_SIZE/2);
	state_save_register_global_pointer(machine, state->buffered_spritesizeram16, TOAPLAN1_SPRITESIZERAM_SIZE/2);

	state->spriteram_size = TOAPLAN1_SPRITERAM_SIZE;
}

static void toaplan1_set_scrolls(running_machine *machine)
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	tilemap_set_scrollx(state->pf1_tilemap, 0, (state->pf1_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs1));
	tilemap_set_scrollx(state->pf2_tilemap, 0, (state->pf2_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs2));
	tilemap_set_scrollx(state->pf3_tilemap, 0, (state->pf3_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs3));
	tilemap_set_scrollx(state->pf4_tilemap, 0, (state->pf4_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs4));
	tilemap_set_scrolly(state->pf1_tilemap, 0, (state->pf1_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
	tilemap_set_scrolly(state->pf2_tilemap, 0, (state->pf2_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
	tilemap_set_scrolly(state->pf3_tilemap, 0, (state->pf3_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
	tilemap_set_scrolly(state->pf4_tilemap, 0, (state->pf4_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
}

static STATE_POSTLOAD( rallybik_flipscreen )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	rallybik_bcu_flipscreen_w(space, 0, state->bcu_flipscreen, 0xffff);
}

static STATE_POSTLOAD( toaplan1_flipscreen )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	toaplan1_bcu_flipscreen_w(space, 0, state->bcu_flipscreen, 0xffff);
}

static void register_common(running_machine *machine)
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	state_save_register_global(machine, state->scrollx_offs1);
	state_save_register_global(machine, state->scrollx_offs2);
	state_save_register_global(machine, state->scrollx_offs3);
	state_save_register_global(machine, state->scrollx_offs4);
	state_save_register_global(machine, state->scrolly_offs);

	state_save_register_global(machine, state->bcu_flipscreen);
	state_save_register_global(machine, state->fcu_flipscreen);
	state_save_register_global(machine, state->reset);

	state_save_register_global(machine, state->pf1_scrollx);
	state_save_register_global(machine, state->pf1_scrolly);
	state_save_register_global(machine, state->pf2_scrollx);
	state_save_register_global(machine, state->pf2_scrolly);
	state_save_register_global(machine, state->pf3_scrollx);
	state_save_register_global(machine, state->pf3_scrolly);
	state_save_register_global(machine, state->pf4_scrollx);
	state_save_register_global(machine, state->pf4_scrolly);

	state_save_register_global(machine, state->tiles_offsetx);
	state_save_register_global(machine, state->tiles_offsety);
	state_save_register_global(machine, state->pf_voffs);
	state_save_register_global(machine, state->spriteram_offs);
}


VIDEO_START( rallybik )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	toaplan1_create_tilemaps(machine);
	toaplan1_paletteram_alloc(machine);
	toaplan1_vram_alloc(machine);

	state->buffered_spriteram = auto_alloc_array_clear(machine, UINT16, state->spriteram_size/2);
	state_save_register_global_pointer(machine, state->buffered_spriteram, state->spriteram_size/2);

	state->scrollx_offs1 = 0x00d + 6;
	state->scrollx_offs2 = 0x00d + 4;
	state->scrollx_offs3 = 0x00d + 2;
	state->scrollx_offs4 = 0x00d + 0;
	state->scrolly_offs  = 0x111;

	state->bcu_flipscreen = -1;
	state->fcu_flipscreen = 0;
	state->reset = 0;

	register_common(machine);

	state_save_register_postload(machine, rallybik_flipscreen, NULL);
}

VIDEO_START( toaplan1 )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	toaplan1_create_tilemaps(machine);
	toaplan1_paletteram_alloc(machine);
	toaplan1_vram_alloc(machine);
	toaplan1_spritevram_alloc(machine);

	state->scrollx_offs1 = 0x1ef + 6;
	state->scrollx_offs2 = 0x1ef + 4;
	state->scrollx_offs3 = 0x1ef + 2;
	state->scrollx_offs4 = 0x1ef + 0;
	state->scrolly_offs  = 0x101;

	state->bcu_flipscreen = -1;
	state->fcu_flipscreen = 0;
	state->reset = 1;

	register_common(machine);

	state_save_register_postload(machine, toaplan1_flipscreen, NULL);
}


/***************************************************************************

  Video I/O port hardware.

***************************************************************************/

READ16_HANDLER( toaplan1_frame_done_r )
{
	return space->machine->primary_screen->vblank();
}

WRITE16_HANDLER( toaplan1_tile_offsets_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	if ( offset == 0 )
	{
		COMBINE_DATA(&state->tiles_offsetx);
		logerror("Tiles_offsetx now = %08x\n", state->tiles_offsetx);
	}
	else
	{
		COMBINE_DATA(&state->tiles_offsety);
		logerror("Tiles_offsety now = %08x\n", state->tiles_offsety);
	}
	state->reset = 1;
	toaplan1_set_scrolls(space->machine);
}

WRITE16_HANDLER( rallybik_bcu_flipscreen_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	if (ACCESSING_BITS_0_7 && (data != state->bcu_flipscreen))
	{
		logerror("Setting BCU controller flipscreen port to %04x\n",data);
		state->bcu_flipscreen = data & 0x01;		/* 0x0001 = flip, 0x0000 = no flip */
		tilemap_set_flip_all(space->machine, (data ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0));
		if (state->bcu_flipscreen)
		{
			state->scrollx_offs1 = 0x1c0 - 6;
			state->scrollx_offs2 = 0x1c0 - 4;
			state->scrollx_offs3 = 0x1c0 - 2;
			state->scrollx_offs4 = 0x1c0 - 0;
			state->scrolly_offs  = 0x0e8;
		}
		else
		{
			state->scrollx_offs1 = 0x00d + 6;
			state->scrollx_offs2 = 0x00d + 4;
			state->scrollx_offs3 = 0x00d + 2;
			state->scrollx_offs4 = 0x00d + 0;
			state->scrolly_offs  = 0x111;
		}
		toaplan1_set_scrolls(space->machine);
	}
}

WRITE16_HANDLER( toaplan1_bcu_flipscreen_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	if (ACCESSING_BITS_0_7 && (data != state->bcu_flipscreen))
	{
		logerror("Setting BCU controller flipscreen port to %04x\n",data);
		state->bcu_flipscreen = data & 0x01;		/* 0x0001 = flip, 0x0000 = no flip */
		tilemap_set_flip_all(space->machine, (data ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0));
		if (state->bcu_flipscreen)
		{
			const rectangle &visarea = space->machine->primary_screen->visible_area();

			state->scrollx_offs1 = 0x151 - 6;
			state->scrollx_offs2 = 0x151 - 4;
			state->scrollx_offs3 = 0x151 - 2;
			state->scrollx_offs4 = 0x151 - 0;
			state->scrolly_offs  = 0x1ef;
			state->scrolly_offs += ((visarea.max_y + 1) - ((visarea.max_y + 1) - visarea.min_y)) * 2;	/* Horizontal games are offset so adjust by +0x20 */
		}
		else
		{
			state->scrollx_offs1 = 0x1ef + 6;
			state->scrollx_offs2 = 0x1ef + 4;
			state->scrollx_offs3 = 0x1ef + 2;
			state->scrollx_offs4 = 0x1ef + 0;
			state->scrolly_offs  = 0x101;
		}
		toaplan1_set_scrolls(space->machine);
	}
}

WRITE16_HANDLER( toaplan1_fcu_flipscreen_w )
{
	if (ACCESSING_BITS_8_15)
	{
		toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

		logerror("Setting FCU controller flipscreen port to %04x\n",data);
		state->fcu_flipscreen = data & 0x8000;	/* 0x8000 = flip, 0x0000 = no flip */
	}
}

READ16_HANDLER( toaplan1_spriteram_offs_r ) /// this aint really needed ?
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	return state->spriteram_offs;
}

WRITE16_HANDLER( toaplan1_spriteram_offs_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	COMBINE_DATA(&state->spriteram_offs);
}


/* tile palette */
READ16_HANDLER( toaplan1_colorram1_r )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	return state->colorram1[offset];
}

WRITE16_HANDLER( toaplan1_colorram1_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	COMBINE_DATA(&state->colorram1[offset]);
	paletteram16_xBBBBBGGGGGRRRRR_word_w(space, offset, data, mem_mask);
}

/* sprite palette */
READ16_HANDLER( toaplan1_colorram2_r )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	return state->colorram2[offset];
}

WRITE16_HANDLER( toaplan1_colorram2_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	COMBINE_DATA(&state->colorram2[offset]);
	paletteram16_xBBBBBGGGGGRRRRR_word_w(space, offset+(state->colorram1_size/2), data, mem_mask);
}

READ16_HANDLER( toaplan1_spriteram16_r )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	return state->spriteram[state->spriteram_offs & ((TOAPLAN1_SPRITERAM_SIZE/2)-1)];
}

WRITE16_HANDLER( toaplan1_spriteram16_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	COMBINE_DATA(&state->spriteram[state->spriteram_offs & ((TOAPLAN1_SPRITERAM_SIZE/2)-1)]);

#ifdef MAME_DEBUG
	if (state->spriteram_offs >= (TOAPLAN1_SPRITERAM_SIZE/2))
	{
		logerror("Sprite_RAM_word_w, %08x out of range !\n", state->spriteram_offs);
		return;
	}
#endif

	state->spriteram_offs++;
}

READ16_HANDLER( toaplan1_spritesizeram16_r )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	return state->spritesizeram16[state->spriteram_offs & ((TOAPLAN1_SPRITESIZERAM_SIZE/2)-1)];
}

WRITE16_HANDLER( toaplan1_spritesizeram16_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	COMBINE_DATA(&state->spritesizeram16[state->spriteram_offs & ((TOAPLAN1_SPRITESIZERAM_SIZE/2)-1)]);

#ifdef MAME_DEBUG
	if (state->spriteram_offs >= (TOAPLAN1_SPRITESIZERAM_SIZE/2))
	{
		logerror("Sprite_Size_RAM_word_w, %08x out of range !\n", state->spriteram_offs);
		return;
	}
#endif

	state->spriteram_offs++;	/// really ? shouldn't happen on the sizeram
}



WRITE16_HANDLER( toaplan1_bcu_control_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	logerror("BCU tile controller register:%02x now = %04x\n",offset,data);

	/*** Hack for Zero Wing and OutZone, to reset the sound system on */
	/*** soft resets. These two games don't have a sound reset port,  */
	/*** unlike the other games */

	if (state->unk_reset_port && state->reset)
	{
		state->reset = 0;
		toaplan1_reset_sound(space,0,0,0);
	}
}

READ16_HANDLER( toaplan1_tileram_offs_r )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	return state->pf_voffs;
}

WRITE16_HANDLER( toaplan1_tileram_offs_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	if (data >= 0x4000)
		logerror("Hmmm, unknown video layer being selected (%08x)\n",data);
	COMBINE_DATA(&state->pf_voffs);
}


READ16_HANDLER( toaplan1_tileram16_r )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();
	offs_t vram_offset;
	UINT16 video_data = 0;

	switch (state->pf_voffs & 0xf000)	/* Locate Layer (PlayField) */
	{
		case 0x0000:
				vram_offset = ((state->pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = state->pf1_tilevram16[vram_offset];
				break;
		case 0x1000:
				vram_offset = ((state->pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = state->pf2_tilevram16[vram_offset];
				break;
		case 0x2000:
				vram_offset = ((state->pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = state->pf3_tilevram16[vram_offset];
				break;
		case 0x3000:
				vram_offset = ((state->pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				video_data = state->pf4_tilevram16[vram_offset];
				break;
		default:
				logerror("Hmmm, reading %04x from unknown playfield layer address %06x  Offset:%01x !!!\n", video_data, state->pf_voffs, offset);
				break;
	}

	return video_data;
}

READ16_HANDLER( rallybik_tileram16_r )
{
	UINT16 data = toaplan1_tileram16_r(space, offset, mem_mask);

	if (offset == 0)	/* some bit lines may be stuck to others */
	{
		data |= ((data & 0xf000) >> 4);
		data |= ((data & 0x0030) << 2);
	}
	return data;
}

WRITE16_HANDLER( toaplan1_tileram16_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();
	offs_t vram_offset;

	switch (state->pf_voffs & 0xf000)	/* Locate Layer (PlayField) */
	{
		case 0x0000:
				vram_offset = ((state->pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&state->pf1_tilevram16[vram_offset]);
				tilemap_mark_tile_dirty(state->pf1_tilemap, vram_offset/2);
				break;
		case 0x1000:
				vram_offset = ((state->pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&state->pf2_tilevram16[vram_offset]);
				tilemap_mark_tile_dirty(state->pf2_tilemap, vram_offset/2);
				break;
		case 0x2000:
				vram_offset = ((state->pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&state->pf3_tilevram16[vram_offset]);
				tilemap_mark_tile_dirty(state->pf3_tilemap, vram_offset/2);
				break;
		case 0x3000:
				vram_offset = ((state->pf_voffs * 2) + offset) & ((TOAPLAN1_TILEVRAM_SIZE/2)-1);
				COMBINE_DATA(&state->pf4_tilevram16[vram_offset]);
				tilemap_mark_tile_dirty(state->pf4_tilemap, vram_offset/2);
				break;
		default:
				logerror("Hmmm, writing %04x to unknown playfield layer address %06x  Offset:%01x\n", data, state->pf_voffs, offset);
				break;
	}
}



READ16_HANDLER( toaplan1_scroll_regs_r )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();
	UINT16 scroll = 0;

	switch(offset)
	{
		case 00: scroll = state->pf1_scrollx; break;
		case 01: scroll = state->pf1_scrolly; break;
		case 02: scroll = state->pf2_scrollx; break;
		case 03: scroll = state->pf2_scrolly; break;
		case 04: scroll = state->pf3_scrollx; break;
		case 05: scroll = state->pf3_scrolly; break;
		case 06: scroll = state->pf4_scrollx; break;
		case 07: scroll = state->pf4_scrolly; break;
		default: logerror("Hmmm, reading unknown video scroll register (%08x) !!!\n",offset);
				 break;
	}
	return scroll;
}


WRITE16_HANDLER( toaplan1_scroll_regs_w )
{
	toaplan1_state *state = space->machine->driver_data<toaplan1_state>();

	switch(offset)
	{
		case 00: COMBINE_DATA(&state->pf1_scrollx);		/* 1D3h */
				 tilemap_set_scrollx(state->pf1_tilemap, 0, (state->pf1_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs1));
				 break;
		case 01: COMBINE_DATA(&state->pf1_scrolly);		/* 1EBh */
				 tilemap_set_scrolly(state->pf1_tilemap, 0, (state->pf1_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
				 break;
		case 02: COMBINE_DATA(&state->pf2_scrollx);		/* 1D5h */
				 tilemap_set_scrollx(state->pf2_tilemap, 0, (state->pf2_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs2));
				 break;
		case 03: COMBINE_DATA(&state->pf2_scrolly);		/* 1EBh */
				 tilemap_set_scrolly(state->pf2_tilemap, 0, (state->pf2_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
				 break;
		case 04: COMBINE_DATA(&state->pf3_scrollx);		/* 1D7h */
				 tilemap_set_scrollx(state->pf3_tilemap, 0, (state->pf3_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs3));
				 break;
		case 05: COMBINE_DATA(&state->pf3_scrolly);		/* 1EBh */
				 tilemap_set_scrolly(state->pf3_tilemap, 0, (state->pf3_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
				 break;
		case 06: COMBINE_DATA(&state->pf4_scrollx);		/* 1D9h */
				 tilemap_set_scrollx(state->pf4_tilemap, 0, (state->pf4_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs4));
				 break;
		case 07: COMBINE_DATA(&state->pf4_scrolly);		/* 1EBh */
				 tilemap_set_scrolly(state->pf4_tilemap, 0, (state->pf4_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
				 break;
		default: logerror("Hmmm, writing %08x to unknown video scroll register (%08x) !!!\n",data ,offset);
				 break;
	}
}




static void toaplan1_log_vram(running_machine *machine)
{
#ifdef MAME_DEBUG
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	if ( input_code_pressed(machine, KEYCODE_M) )
	{
		UINT16 *spriteram16 = state->spriteram;
		UINT16 *buffered_spriteram16 = state->buffered_spriteram;
		offs_t sprite_voffs;
		while (input_code_pressed(machine, KEYCODE_M)) ;
		if (state->spritesizeram16)			/* FCU controller */
		{
			int schar,sattr,sxpos,sypos,bschar,bsattr,bsxpos,bsypos;
			UINT16 *size  = (UINT16 *)(state->spritesizeram16);
			UINT16 *bsize = (UINT16 *)(state->buffered_spritesizeram16);
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
				state->pf1_scrollx, state->pf1_scrolly, state->pf2_scrollx, state->pf2_scrolly, state->pf3_scrollx, state->pf3_scrolly, state->pf4_scrollx, state->pf4_scrolly);
			for ( sprite_voffs = 0; sprite_voffs < state->spriteram_size/2; sprite_voffs += 4 )
			{
				bschar = buffered_spriteram16[sprite_voffs];
				bsattr = buffered_spriteram16[sprite_voffs + 1];
				bsxpos = buffered_spriteram16[sprite_voffs + 2];
				bsypos = buffered_spriteram16[sprite_voffs + 3];
				schar = spriteram16[sprite_voffs];
				sattr = spriteram16[sprite_voffs + 1];
				sxpos = spriteram16[sprite_voffs + 2];
				sypos = spriteram16[sprite_voffs + 3];
				logerror("$(%04x)  Tile-Attr-Xpos-Ypos Now:%04x %04x %04x.%01x %04x.%01x  nxt:%04x %04x %04x.%01x %04x.%01x\n", sprite_voffs,
											 schar, sattr, sxpos, size[( sattr>>6)&0x3f]&0xf, sypos,( size[( sattr>>6)&0x3f]>>4)&0xf,
											bschar,bsattr,bsxpos,bsize[(bsattr>>6)&0x3f]&0xf,bsypos,(bsize[(bsattr>>6)&0x3f]>>4)&0xf);
			}
		}
		else									/* SCU controller */
		{
			int schar,sattr,sxpos,sypos,bschar,bsattr,bsxpos,bsypos;
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
				state->pf1_scrollx, state->pf1_scrolly, state->pf2_scrollx, state->pf2_scrolly, state->pf3_scrollx, state->pf3_scrolly, state->pf4_scrollx, state->pf4_scrolly);
			for ( sprite_voffs = 0; sprite_voffs < state->spriteram_size/2; sprite_voffs += 4 )
			{
				bschar = buffered_spriteram16[sprite_voffs];
				bsattr = buffered_spriteram16[sprite_voffs + 1];
				bsypos = buffered_spriteram16[sprite_voffs + 2];
				bsxpos = buffered_spriteram16[sprite_voffs + 3];
				schar = spriteram16[sprite_voffs];
				sattr = spriteram16[sprite_voffs + 1];
				sypos = spriteram16[sprite_voffs + 2];
				sxpos = spriteram16[sprite_voffs + 3];
				logerror("$(%04x)  Tile-Attr-Xpos-Ypos Now:%04x %04x %04x %04x  nxt:%04x %04x %04x %04x\n", sprite_voffs,
											 schar, sattr, sxpos, sypos,
											bschar,bsattr,bsxpos, bsypos);
			}
		}
	}

	if ( input_code_pressed(machine, KEYCODE_SLASH) )
	{
		UINT16 *size  = (UINT16 *)(state->spritesizeram16);
		UINT16 *bsize = (UINT16 *)(state->buffered_spritesizeram16);
		offs_t offs;
		while (input_code_pressed(machine, KEYCODE_SLASH)) ;
		if (state->spritesizeram16)			/* FCU controller */
		{
			logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
			logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
				state->pf1_scrollx, state->pf1_scrolly, state->pf2_scrollx, state->pf2_scrolly, state->pf3_scrollx, state->pf3_scrolly, state->pf4_scrollx, state->pf4_scrolly);
			for ( offs = 0; offs < (TOAPLAN1_SPRITESIZERAM_SIZE/2); offs +=4 )
			{
				logerror("SizeOffs:%04x   now:%04x %04x %04x %04x    next: %04x %04x %04x %04x\n", offs,
												bsize[offs+0], bsize[offs+1],
												bsize[offs+2], bsize[offs+3],
												size[offs+0], size[offs+1],
												size[offs+2], size[offs+3]);
			}
		}
	}

	if ( input_code_pressed(machine, KEYCODE_N) )
	{
		offs_t tile_voffs;
		int tchar[5], tattr[5];
		while (input_code_pressed(machine, KEYCODE_N)) ;	/* BCU controller */
		logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
		logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
			state->pf1_scrollx, state->pf1_scrolly, state->pf2_scrollx, state->pf2_scrolly, state->pf3_scrollx, state->pf3_scrolly, state->pf4_scrollx, state->pf4_scrolly);
		for ( tile_voffs = 0; tile_voffs < (TOAPLAN1_TILEVRAM_SIZE/2); tile_voffs += 2 )
		{
			tchar[1] = state->pf1_tilevram16[tile_voffs + 1];
			tattr[1] = state->pf1_tilevram16[tile_voffs];
			tchar[2] = state->pf2_tilevram16[tile_voffs + 1];
			tattr[2] = state->pf2_tilevram16[tile_voffs];
			tchar[3] = state->pf3_tilevram16[tile_voffs + 1];
			tattr[3] = state->pf3_tilevram16[tile_voffs];
			tchar[4] = state->pf4_tilevram16[tile_voffs + 1];
			tattr[4] = state->pf4_tilevram16[tile_voffs];
//          logerror("PF3 offs:%04x   Tile:%04x  Attr:%04x\n", tile_voffs, tchar, tattr);
			logerror("$(%04x)  Attr-Tile PF1:%04x-%04x  PF2:%04x-%04x  PF3:%04x-%04x  PF4:%04x-%04x\n", tile_voffs,
									tattr[1], tchar[1],  tattr[2], tchar[2],
									tattr[3], tchar[3],  tattr[4], tchar[4]);
		}
	}

	if ( input_code_pressed(machine, KEYCODE_W) )
	{
		while (input_code_pressed(machine, KEYCODE_W)) ;
		logerror("Mark here\n");
	}
	if ( input_code_pressed(machine, KEYCODE_E) )
	{
		while (input_code_pressed(machine, KEYCODE_E)) ;
		state->displog += 1;
		state->displog &= 1;
	}
	if (state->displog)
	{
		logerror("Scrolls    PF1-X  PF1-Y     PF2-X  PF2-Y     PF3-X  PF3-Y     PF4-X  PF4-Y\n");
		logerror("------>    #%04x  #%04x     #%04x  #%04x     #%04x  #%04x     #%04x  #%04x\n",
			state->pf1_scrollx, state->pf1_scrolly, state->pf2_scrollx, state->pf2_scrolly, state->pf3_scrollx, state->pf3_scrolly, state->pf4_scrollx, state->pf4_scrolly);
	}
	if ( input_code_pressed(machine, KEYCODE_B) )
	{
//      while (input_code_pressed(machine, KEYCODE_B)) ;
		state->scrollx_offs1 += 0x1; state->scrollx_offs2 += 0x1; state->scrollx_offs3 += 0x1; state->scrollx_offs4 += 0x1;
		logerror("Scrollx_offs now = %08x\n", state->scrollx_offs4);
		tilemap_set_scrollx(state->pf1_tilemap, 0, (state->pf1_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs1));
		tilemap_set_scrollx(state->pf2_tilemap, 0, (state->pf2_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs2));
		tilemap_set_scrollx(state->pf3_tilemap, 0, (state->pf3_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs3));
		tilemap_set_scrollx(state->pf4_tilemap, 0, (state->pf4_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs4));
	}
	if ( input_code_pressed(machine, KEYCODE_V) )
	{
//      while (input_code_pressed(machine, KEYCODE_V)) ;
		state->scrollx_offs1 -= 0x1; state->scrollx_offs2 -= 0x1; state->scrollx_offs3 -= 0x1; state->scrollx_offs4 -= 0x1;
		logerror("Scrollx_offs now = %08x\n", state->scrollx_offs4);
		tilemap_set_scrollx(state->pf1_tilemap, 0, (state->pf1_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs1));
		tilemap_set_scrollx(state->pf2_tilemap, 0, (state->pf2_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs2));
		tilemap_set_scrollx(state->pf3_tilemap, 0, (state->pf3_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs3));
		tilemap_set_scrollx(state->pf4_tilemap, 0, (state->pf4_scrollx >> 7) - (state->tiles_offsetx - state->scrollx_offs4));
	}
	if ( input_code_pressed(machine, KEYCODE_C) )
	{
//      while (input_code_pressed(machine, KEYCODE_C)) ;
		state->scrolly_offs += 0x1;
		logerror("Scrolly_offs now = %08x\n", state->scrolly_offs);
		tilemap_set_scrolly(state->pf1_tilemap, 0, (state->pf1_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
		tilemap_set_scrolly(state->pf2_tilemap, 0, (state->pf2_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
		tilemap_set_scrolly(state->pf3_tilemap, 0, (state->pf3_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
		tilemap_set_scrolly(state->pf4_tilemap, 0, (state->pf4_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
	}
	if ( input_code_pressed(machine, KEYCODE_X) )
	{
//      while (input_code_pressed(machine, KEYCODE_X)) ;
		state->scrolly_offs -= 0x1;
		logerror("Scrolly_offs now = %08x\n", state->scrolly_offs);
		tilemap_set_scrolly(state->pf1_tilemap, 0, (state->pf1_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
		tilemap_set_scrolly(state->pf2_tilemap, 0, (state->pf2_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
		tilemap_set_scrolly(state->pf3_tilemap, 0, (state->pf3_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
		tilemap_set_scrolly(state->pf4_tilemap, 0, (state->pf4_scrolly >> 7) - (state->tiles_offsety - state->scrolly_offs));
	}

	if ( input_code_pressed(machine, KEYCODE_L) )		/* Turn Playfield 4 on/off */
	{
		while (input_code_pressed(machine, KEYCODE_L)) ;
		state->display_pf4 += 1;
		state->display_pf4 &= 1;
		tilemap_set_enable(state->pf4_tilemap, state->display_pf4);
	}
	if ( input_code_pressed(machine, KEYCODE_K) )		/* Turn Playfield 3 on/off */
	{
		while (input_code_pressed(machine, KEYCODE_K)) ;
		state->display_pf3 += 1;
		state->display_pf3 &= 1;
		tilemap_set_enable(state->pf3_tilemap, state->display_pf3);
	}
	if ( input_code_pressed(machine, KEYCODE_J) )		/* Turn Playfield 2 on/off */
	{
		while (input_code_pressed(machine, KEYCODE_J)) ;
		state->display_pf2 += 1;
		state->display_pf2 &= 1;
		tilemap_set_enable(state->pf2_tilemap, state->display_pf2);
	}
	if ( input_code_pressed(machine, KEYCODE_H) )		/* Turn Playfield 1 on/off */
	{
		while (input_code_pressed(machine, KEYCODE_H)) ;
		state->display_pf1 += 1;
		state->display_pf1 &= 1;
		tilemap_set_enable(state->pf1_tilemap, state->display_pf1);
	}
#endif
}



/***************************************************************************
    Sprite Handlers
***************************************************************************/

// custom function to draw a single sprite. needed to keep correct sprites - sprites and sprites - tilemaps priorities
static void toaplan1_draw_sprite_custom(bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int sx,int sy,
		int priority)
{
	int pal_base = gfx->color_base + gfx->color_granularity * (color % gfx->total_colors);
	const UINT8 *source_base = gfx_element_get_data(gfx, code % gfx->total_elements);
	bitmap_t *priority_bitmap = gfx->machine->priority_bitmap;
	int sprite_screen_height = ((1<<16)*gfx->height+0x8000)>>16;
	int sprite_screen_width = ((1<<16)*gfx->width+0x8000)>>16;

	if (sprite_screen_width && sprite_screen_height)
	{
		/* compute sprite increment per screen pixel */
		int dx = (gfx->width<<16)/sprite_screen_width;
		int dy = (gfx->height<<16)/sprite_screen_height;

		int ex = sx+sprite_screen_width;
		int ey = sy+sprite_screen_height;

		int x_index_base;
		int y_index;

		if( flipx )
		{
			x_index_base = (sprite_screen_width-1)*dx;
			dx = -dx;
		}
		else
		{
			x_index_base = 0;
		}

		if( flipy )
		{
			y_index = (sprite_screen_height-1)*dy;
			dy = -dy;
		}
		else
		{
			y_index = 0;
		}

		if( clip )
		{
			if( sx < clip->min_x)
			{ /* clip left */
				int pixels = clip->min_x-sx;
				sx += pixels;
				x_index_base += pixels*dx;
			}
			if( sy < clip->min_y )
			{ /* clip top */
				int pixels = clip->min_y-sy;
				sy += pixels;
				y_index += pixels*dy;
			}
			/* NS 980211 - fixed incorrect clipping */
			if( ex > clip->max_x+1 )
			{ /* clip right */
				int pixels = ex-clip->max_x-1;
				ex -= pixels;
			}
			if( ey > clip->max_y+1 )
			{ /* clip bottom */
				int pixels = ey-clip->max_y-1;
				ey -= pixels;
			}
		}

		if( ex>sx )
		{ /* skip if inner loop doesn't draw anything */
			int y;

			for( y=sy; y<ey; y++ )
			{
				const UINT8 *source = source_base + (y_index>>16) * gfx->line_modulo;
				UINT16 *dest = BITMAP_ADDR16(dest_bmp, y, 0);
				UINT8 *pri = BITMAP_ADDR8(priority_bitmap, y, 0);

				int x, x_index = x_index_base;
				for( x=sx; x<ex; x++ )
				{
					int c = source[x_index>>16];
					if( c != 0 )
					{
						if (pri[x] < priority)
							dest[x] = pal_base+c;
						pri[x] = 0xff; // mark it "already drawn"
					}
					x_index += dx;
				}

				y_index += dy;
			}
		}
	}
}


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();
	UINT16 *source = (UINT16 *)state->buffered_spriteram;
	UINT16 *size   = (UINT16 *)state->buffered_spritesizeram16;
	int fcu_flipscreen = state->fcu_flipscreen;
	int offs;

	for (offs = state->spriteram_size/2 - 4; offs >= 0; offs -= 4)
	{
		if (!(source[offs] & 0x8000))
		{
			int attrib, sprite, color, priority, sx, sy;
			int sprite_sizex, sprite_sizey, dim_x, dim_y, sx_base, sy_base;
			int sizeram_ptr;

			attrib = source[offs+1];
			priority = (attrib & 0xf000) >> 12;

			sprite = source[offs] & 0x7fff;
			color = attrib & 0x3f;

			/****** find sprite dimension ******/
			sizeram_ptr = (attrib >> 6) & 0x3f;
			sprite_sizex = ( size[sizeram_ptr]       & 0x0f) * 8;
			sprite_sizey = ((size[sizeram_ptr] >> 4) & 0x0f) * 8;

			/****** find position to display sprite ******/
			sx_base = (source[offs + 2] >> 7) & 0x1ff;
			sy_base = (source[offs + 3] >> 7) & 0x1ff;

			if (sx_base >= 0x180) sx_base -= 0x200;
			if (sy_base >= 0x180) sy_base -= 0x200;

			/****** flip the sprite layer ******/
			if (fcu_flipscreen)
			{
				const rectangle &visarea = machine->primary_screen->visible_area();

				sx_base = ((visarea.max_x + 1) - visarea.min_x) - (sx_base + 8);	/* visarea.x = 320 */
				sy_base = ((visarea.max_y + 1) - visarea.min_y) - (sy_base + 8);	/* visarea.y = 240 */
				sy_base += ((visarea.max_y + 1) - ((visarea.max_y + 1) - visarea.min_y)) * 2;	/* Horizontal games are offset so adjust by +0x20 */
			}

			for (dim_y = 0; dim_y < sprite_sizey; dim_y += 8)
			{
				if (fcu_flipscreen) sy = sy_base - dim_y;
				else                sy = sy_base + dim_y;

				for (dim_x = 0; dim_x < sprite_sizex; dim_x += 8)
				{
					if (fcu_flipscreen) sx = sx_base - dim_x;
					else                sx = sx_base + dim_x;

					toaplan1_draw_sprite_custom(bitmap,cliprect,machine->gfx[1],
							                   sprite,color,
							                   fcu_flipscreen,fcu_flipscreen,
							                   sx,sy,
							                   priority);

					sprite++ ;
				}
			}
		}
	}
}


static void rallybik_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();
	UINT16 *buffered_spriteram16 = state->buffered_spriteram;
	int offs;

	for (offs = 0; offs < state->spriteram_size/2; offs += 4)
	{
		int attrib, sx, sy, flipx, flipy;
		int sprite, color;

		attrib = buffered_spriteram16[offs + 1];
		if ((attrib & 0x0c00) == priority)
		{
			sy = (buffered_spriteram16[offs + 3] >> 7) & 0x1ff;
			if (sy != 0x0100)		/* sx = 0x01a0 or 0x0040*/
			{
				sprite = buffered_spriteram16[offs] & 0x7ff;
				color  = attrib & 0x3f;
				sx = (buffered_spriteram16[offs + 2] >> 7) & 0x1ff;
				flipx = attrib & 0x100;
				if (flipx) sx -= 15;
				flipy = attrib & 0x200;
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					sprite,
					color,
					flipx,flipy,
					sx-31,sy-16,0);
			}
		}
	}
}


/***************************************************************************
    Draw the game screen in the given bitmap_t.
***************************************************************************/

VIDEO_UPDATE( rallybik )
{
	toaplan1_state *state = screen->machine->driver_data<toaplan1_state>();
	int priority;

	toaplan1_log_vram(screen->machine);

	bitmap_fill(bitmap,cliprect,0x120);

	tilemap_draw(bitmap, cliprect, state->pf1_tilemap, TILEMAP_DRAW_OPAQUE | 0, 0);
	tilemap_draw(bitmap, cliprect, state->pf1_tilemap, TILEMAP_DRAW_OPAQUE | 1, 0);

	for (priority = 1; priority < 16; priority++)
	{
		tilemap_draw(bitmap, cliprect, state->pf4_tilemap, priority, 0);
		tilemap_draw(bitmap, cliprect, state->pf3_tilemap, priority, 0);
		tilemap_draw(bitmap, cliprect, state->pf2_tilemap, priority, 0);
		tilemap_draw(bitmap, cliprect, state->pf1_tilemap, priority, 0);
		rallybik_draw_sprites(screen->machine, bitmap,cliprect,priority << 8);
	}

	return 0;
}

VIDEO_UPDATE( toaplan1 )
{
	toaplan1_state *state = screen->machine->driver_data<toaplan1_state>();
	int priority;

	toaplan1_log_vram(screen->machine);

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);
	bitmap_fill(bitmap,cliprect,0x120);

// it's really correct?
	tilemap_draw(bitmap, cliprect, state->pf1_tilemap, TILEMAP_DRAW_OPAQUE | 0, 0);
	tilemap_draw(bitmap, cliprect, state->pf1_tilemap, TILEMAP_DRAW_OPAQUE | 1, 0);

	for (priority = 1; priority < 16; priority++)
	{
		tilemap_draw_primask(bitmap, cliprect, state->pf4_tilemap, priority, priority, 0);
		tilemap_draw_primask(bitmap, cliprect, state->pf3_tilemap, priority, priority, 0);
		tilemap_draw_primask(bitmap, cliprect, state->pf2_tilemap, priority, priority, 0);
		tilemap_draw_primask(bitmap, cliprect, state->pf1_tilemap, priority, priority, 0);
	}

	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

/****************************************************************************
    Spriteram is always 1 frame ahead, suggesting spriteram buffering.
    There are no CPU output registers that control this so we
    assume it happens automatically every frame, at the end of vblank
****************************************************************************/

VIDEO_EOF( rallybik )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	memcpy(state->buffered_spriteram, state->spriteram, state->spriteram_size);
}

VIDEO_EOF( toaplan1 )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	memcpy(state->buffered_spriteram, state->spriteram, state->spriteram_size);
	memcpy(state->buffered_spritesizeram16, state->spritesizeram16, TOAPLAN1_SPRITESIZERAM_SIZE);
}

VIDEO_EOF( samesame )
{
	toaplan1_state *state = machine->driver_data<toaplan1_state>();

	memcpy(state->buffered_spriteram, state->spriteram, state->spriteram_size);
	memcpy(state->buffered_spritesizeram16, state->spritesizeram16, TOAPLAN1_SPRITESIZERAM_SIZE);
	cputag_set_input_line(machine, "maincpu", M68K_IRQ_2, HOLD_LINE);	/* Frame done */
}
