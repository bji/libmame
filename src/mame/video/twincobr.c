/***************************************************************************
  video.c

  Functions to emulate the video hardware of these machines.
  Video is 40x30 tiles. (320x240 for Twin Cobra/Flying shark)
  Video is 40x30 tiles. (320x240 for Wardner)

  Video has 3 scrolling tile layers (Background, Foreground and Text) and
  Sprites that have 4 priorities. Lowest priority is "Off".
  Wardner has an unusual sprite priority in the shop scenes, whereby a
  middle level priority Sprite appears over a high priority Sprite ?

***************************************************************************/

#include "emu.h"
#include "video/mc6845.h"
#include "includes/twincobr.h"


static void twincobr_restore_screen(running_machine &machine);

/* 6845 used for video sync signals only */
const mc6845_interface twincobr_mc6845_intf =
{
	"screen",	/* screen we are acting on */
	2,			/* number of pixels per video memory address */ /* Horizontal Display programmed to 160 characters */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};



/***************************************************************************
    Callbacks for the TileMap code
***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	twincobr_state *state = machine.driver_data<twincobr_state>();
	int code, tile_number, color;

	code = state->m_bgvideoram16[tile_index+state->m_bg_ram_bank];
	tile_number = code & 0x0fff;
	color = (code & 0xf000) >> 12;
	SET_TILE_INFO(
			2,
			tile_number,
			color,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	twincobr_state *state = machine.driver_data<twincobr_state>();
	int code, tile_number, color;

	code = state->m_fgvideoram16[tile_index];
	tile_number = (code & 0x0fff) | state->m_fg_rom_bank;
	color = (code & 0xf000) >> 12;
	SET_TILE_INFO(
			1,
			tile_number,
			color,
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	twincobr_state *state = machine.driver_data<twincobr_state>();
	int code, tile_number, color;

	code = state->m_txvideoram16[tile_index];
	tile_number = code & 0x07ff;
	color = (code & 0xf800) >> 11;
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
}

/***************************************************************************
    Start the video hardware emulation.
***************************************************************************/

static void twincobr_create_tilemaps(running_machine &machine)
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,8,8,64,64);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,64,64);
	state->m_tx_tilemap = tilemap_create(machine, get_tx_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_transparent_pen(state->m_fg_tilemap,0);
	tilemap_set_transparent_pen(state->m_tx_tilemap,0);
}

VIDEO_START( toaplan0 )
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	/* the video RAM is accessed via ports, it's not memory mapped */
	state->m_txvideoram_size = 0x0800;
	state->m_bgvideoram_size = 0x2000;	/* banked two times 0x1000 */
	state->m_fgvideoram_size = 0x1000;

	twincobr_create_tilemaps(machine);

	state->m_txvideoram16 = auto_alloc_array_clear(machine, UINT16, state->m_txvideoram_size);
	state->m_fgvideoram16 = auto_alloc_array_clear(machine, UINT16, state->m_fgvideoram_size);
	state->m_bgvideoram16 = auto_alloc_array_clear(machine, UINT16, state->m_bgvideoram_size);

	state->m_display_on = 0;
	twincobr_display(machine, state->m_display_on);

	state_save_register_global_pointer(machine, state->m_txvideoram16, state->m_txvideoram_size);
	state_save_register_global_pointer(machine, state->m_fgvideoram16, state->m_fgvideoram_size);
	state_save_register_global_pointer(machine, state->m_bgvideoram16, state->m_bgvideoram_size);
	state_save_register_global(machine, state->m_txoffs);
	state_save_register_global(machine, state->m_fgoffs);
	state_save_register_global(machine, state->m_bgoffs);
	state_save_register_global(machine, state->m_scroll_x);
	state_save_register_global(machine, state->m_scroll_y);
	state_save_register_global(machine, state->m_txscrollx);
	state_save_register_global(machine, state->m_fgscrollx);
	state_save_register_global(machine, state->m_bgscrollx);
	state_save_register_global(machine, state->m_txscrolly);
	state_save_register_global(machine, state->m_fgscrolly);
	state_save_register_global(machine, state->m_bgscrolly);
	state_save_register_global(machine, state->m_display_on);
	state_save_register_global(machine, state->m_fg_rom_bank);
	state_save_register_global(machine, state->m_bg_ram_bank);
	state_save_register_global(machine, state->m_flip_screen);
	state_save_register_global(machine, state->m_wardner_sprite_hack);
	machine.save().register_postload(save_prepost_delegate(FUNC(twincobr_restore_screen), &machine));
}

static void twincobr_restore_screen(running_machine &machine)
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	twincobr_display(machine, state->m_display_on);
	twincobr_flipscreen(machine, state->m_flip_screen);
}


/***************************************************************************
    Video I/O interface
***************************************************************************/

void twincobr_display(running_machine &machine, int enable)
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	state->m_display_on = enable;
	tilemap_set_enable(state->m_bg_tilemap, enable);
	tilemap_set_enable(state->m_fg_tilemap, enable);
	tilemap_set_enable(state->m_tx_tilemap, enable);
}

void twincobr_flipscreen(running_machine &machine, int flip)
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	tilemap_set_flip_all(machine, (flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0));
	state->m_flip_screen = flip;
	if (flip) {
		state->m_scroll_x = 0x008;
		state->m_scroll_y = 0x0c5;
	}
	else {
		state->m_scroll_x = 0x037;
		state->m_scroll_y = 0x01e;
	}
}


WRITE16_HANDLER( twincobr_txoffs_w )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	COMBINE_DATA(&state->m_txoffs);
	state->m_txoffs %= state->m_txvideoram_size;
}
READ16_HANDLER( twincobr_txram_r )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	return state->m_txvideoram16[state->m_txoffs];
}
WRITE16_HANDLER( twincobr_txram_w )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	COMBINE_DATA(&state->m_txvideoram16[state->m_txoffs]);
	tilemap_mark_tile_dirty(state->m_tx_tilemap,state->m_txoffs);
}

WRITE16_HANDLER( twincobr_bgoffs_w )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	COMBINE_DATA(&state->m_bgoffs);
	state->m_bgoffs %= (state->m_bgvideoram_size >> 1);
}
READ16_HANDLER( twincobr_bgram_r )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	return state->m_bgvideoram16[state->m_bgoffs+state->m_bg_ram_bank];
}
WRITE16_HANDLER( twincobr_bgram_w )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	COMBINE_DATA(&state->m_bgvideoram16[state->m_bgoffs+state->m_bg_ram_bank]);
	tilemap_mark_tile_dirty(state->m_bg_tilemap,(state->m_bgoffs+state->m_bg_ram_bank));
}

WRITE16_HANDLER( twincobr_fgoffs_w )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	COMBINE_DATA(&state->m_fgoffs);
	state->m_fgoffs %= state->m_fgvideoram_size;
}
READ16_HANDLER( twincobr_fgram_r )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	return state->m_fgvideoram16[state->m_fgoffs];
}
WRITE16_HANDLER( twincobr_fgram_w )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	COMBINE_DATA(&state->m_fgvideoram16[state->m_fgoffs]);
	tilemap_mark_tile_dirty(state->m_fg_tilemap,state->m_fgoffs);
}


WRITE16_HANDLER( twincobr_txscroll_w )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	if (offset == 0) {
		COMBINE_DATA(&state->m_txscrollx);
		tilemap_set_scrollx(state->m_tx_tilemap,0,(state->m_txscrollx+state->m_scroll_x) & 0x1ff);
	}
	else {
		COMBINE_DATA(&state->m_txscrolly);
		tilemap_set_scrolly(state->m_tx_tilemap,0,(state->m_txscrolly+state->m_scroll_y) & 0x1ff);
	}
}

WRITE16_HANDLER( twincobr_bgscroll_w )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	if (offset == 0) {
		COMBINE_DATA(&state->m_bgscrollx);
		tilemap_set_scrollx(state->m_bg_tilemap,0,(state->m_bgscrollx+state->m_scroll_x) & 0x1ff);
	}
	else {
		COMBINE_DATA(&state->m_bgscrolly);
		tilemap_set_scrolly(state->m_bg_tilemap,0,(state->m_bgscrolly+state->m_scroll_y) & 0x1ff);
	}
}

WRITE16_HANDLER( twincobr_fgscroll_w )
{
	twincobr_state *state = space->machine().driver_data<twincobr_state>();

	if (offset == 0) {
		COMBINE_DATA(&state->m_fgscrollx);
		tilemap_set_scrollx(state->m_fg_tilemap,0,(state->m_fgscrollx+state->m_scroll_x) & 0x1ff);
	}
	else {
		COMBINE_DATA(&state->m_fgscrolly);
		tilemap_set_scrolly(state->m_fg_tilemap,0,(state->m_fgscrolly+state->m_scroll_y) & 0x1ff);
	}
}

WRITE16_HANDLER( twincobr_exscroll_w )	/* Extra unused video layer */
{
	if (offset == 0) logerror("PC - write %04x to unknown video scroll Y register\n",data);
	else logerror("PC - write %04x to unknown video scroll X register\n",data);
}

/******************** Wardner interface to this hardware ********************/
WRITE8_HANDLER( wardner_txlayer_w )
{
	int shift = 8 * (offset & 1);
	twincobr_txoffs_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_bglayer_w )
{
	int shift = 8 * (offset & 1);
	twincobr_bgoffs_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_fglayer_w )
{
	int shift = 8 * (offset & 1);
	twincobr_fgoffs_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_txscroll_w )
{
	int shift = 8 * (offset & 1);
	twincobr_txscroll_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_bgscroll_w )
{
	int shift = 8 * (offset & 1);
	twincobr_bgscroll_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_fgscroll_w )
{
	int shift = 8 * (offset & 1);
	twincobr_fgscroll_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_exscroll_w )	/* Extra unused video layer */
{
	switch (offset)
	{
		case 01:	//data <<= 8;
		case 00:	logerror("PC - write %04x to unknown video scroll X register\n",data); break;
		case 03:	//data <<= 8;
		case 02:	logerror("PC - write %04x to unknown video scroll Y register\n",data); break;
	}
}

READ8_HANDLER( wardner_videoram_r )
{
	int shift = 8 * (offset & 1);
	switch (offset/2) {
		case 0: return twincobr_txram_r(space,0,0xffff) >> shift;
		case 1: return twincobr_bgram_r(space,0,0xffff) >> shift;
		case 2: return twincobr_fgram_r(space,0,0xffff) >> shift;
	}
	return 0;
}

WRITE8_HANDLER( wardner_videoram_w )
{
	int shift = 8 * (offset & 1);
	switch (offset/2) {
		case 0: twincobr_txram_w(space,0,data << shift, 0xff << shift); break;
		case 1: twincobr_bgram_w(space,0,data << shift, 0xff << shift); break;
		case 2: twincobr_fgram_w(space,0,data << shift, 0xff << shift); break;
	}
}

READ8_HANDLER( wardner_sprite_r )
{
	int shift = (offset & 1) * 8;
	return space->machine().generic.spriteram.u16[offset/2] >> shift;
}

WRITE8_HANDLER( wardner_sprite_w )
{
	UINT16 *spriteram16 = space->machine().generic.spriteram.u16;
	if (offset & 1)
		spriteram16[offset/2] = (spriteram16[offset/2] & 0x00ff) | (data << 8);
	else
		spriteram16[offset/2] = (spriteram16[offset/2] & 0xff00) | data;
}



/***************************************************************************
    Ugly sprite hack for Wardner when hero is in shop
***************************************************************************/

static void wardner_sprite_priority_hack(running_machine &machine)
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	if (state->m_fgscrollx != state->m_bgscrollx) {
		UINT16 *buffered_spriteram16 = machine.generic.buffered_spriteram.u16;
		if ((state->m_fgscrollx==0x1c9) || (state->m_flip_screen && (state->m_fgscrollx==0x17a))) {	/* in the shop ? */
			int wardner_hack = buffered_spriteram16[0x0b04/2];
		/* sprite position 0x6300 to 0x8700 -- hero on shop keeper (normal) */
		/* sprite position 0x3900 to 0x5e00 -- hero on shop keeper (flip) */
			if ((wardner_hack > 0x3900) && (wardner_hack < 0x8700)) {	/* hero at shop keeper ? */
				wardner_hack = buffered_spriteram16[0x0b02/2];
				wardner_hack |= 0x0400;			/* make hero top priority */
				buffered_spriteram16[0x0b02/2] = wardner_hack;
				wardner_hack = buffered_spriteram16[0x0b0a/2];
				wardner_hack |= 0x0400;
				buffered_spriteram16[0x0b0a/2] = wardner_hack;
				wardner_hack = buffered_spriteram16[0x0b12/2];
				wardner_hack |= 0x0400;
				buffered_spriteram16[0x0b12/2] = wardner_hack;
				wardner_hack = buffered_spriteram16[0x0b1a/2];
				wardner_hack |= 0x0400;
				buffered_spriteram16[0x0b1a/2] = wardner_hack;
			}
		}
	}
}



static void twincobr_log_vram(running_machine &machine)
{
#ifdef MAME_DEBUG
	twincobr_state *state = machine.driver_data<twincobr_state>();

	if ( machine.input().code_pressed(KEYCODE_M) )
	{
		offs_t tile_voffs;
		int tcode[4];
		while (machine.input().code_pressed(KEYCODE_M)) ;
		logerror("Scrolls             BG-X BG-Y  FG-X FG-Y  TX-X  TX-Y\n");
		logerror("------>             %04x %04x  %04x %04x  %04x  %04x\n",state->m_bgscrollx,state->m_bgscrolly,state->m_fgscrollx,state->m_fgscrolly,state->m_txscrollx,state->m_txscrolly);
		for ( tile_voffs = 0; tile_voffs < (state->m_txvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = state->m_bgvideoram16[tile_voffs];
			tcode[2] = state->m_fgvideoram16[tile_voffs];
			tcode[3] = state->m_txvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x  FG1:%01x-%03x  TX1:%02x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff,
							tcode[2] & 0xf000 >> 12, tcode[2] & 0x0fff,
							tcode[3] & 0xf800 >> 11, tcode[3] & 0x07ff);
		}
		for ( tile_voffs = (state->m_txvideoram_size/2); tile_voffs < (state->m_fgvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = state->m_bgvideoram16[tile_voffs];
			tcode[2] = state->m_fgvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x  FG1:%01x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff,
							tcode[2] & 0xf000 >> 12, tcode[2] & 0x0fff);
		}
		for ( tile_voffs = (state->m_fgvideoram_size/2); tile_voffs < (state->m_bgvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = state->m_bgvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff);
		}
	}
#endif
}


/***************************************************************************
    Sprite Handlers
***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	twincobr_state *state = machine.driver_data<twincobr_state>();
	int offs;

	if (state->m_display_on)
	{
		UINT16 *buffered_spriteram16 = machine.generic.buffered_spriteram.u16;
		for (offs = 0;offs < machine.generic.spriteram_size/2;offs += 4)
		{
			int attribute,sx,sy,flipx,flipy;
			int sprite, color;

			attribute = buffered_spriteram16[offs + 1];
			if ((attribute & 0x0c00) == priority) {	/* low priority */
				sy = buffered_spriteram16[offs + 3] >> 7;
				if (sy != 0x0100) {		/* sx = 0x01a0 or 0x0040*/
					sprite = buffered_spriteram16[offs] & 0x7ff;
					color  = attribute & 0x3f;
					sx = buffered_spriteram16[offs + 2] >> 7;
					flipx = attribute & 0x100;
					if (flipx) sx -= 14;		/* should really be 15 */
					flipy = attribute & 0x200;
					drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
						sprite,
						color,
						flipx,flipy,
						sx-32,sy-16,0);
				}
			}
		}
	}
}


/***************************************************************************
    Draw the game screen in the given bitmap_t.
***************************************************************************/

SCREEN_UPDATE( toaplan0 )
{
	twincobr_state *state = screen->machine().driver_data<twincobr_state>();
	twincobr_log_vram(screen->machine());

	if (state->m_wardner_sprite_hack) wardner_sprite_priority_hack(screen->machine());

	bitmap_fill(bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,state->m_bg_tilemap,TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(screen->machine(), bitmap,cliprect,0x0400);
	tilemap_draw(bitmap,cliprect,state->m_fg_tilemap,0,0);
	draw_sprites(screen->machine(), bitmap,cliprect,0x0800);
	tilemap_draw(bitmap,cliprect,state->m_tx_tilemap,0,0);
	draw_sprites(screen->machine(), bitmap,cliprect,0x0c00);
	return 0;
}


SCREEN_EOF( toaplan0 )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	/* Spriteram is always 1 frame ahead, suggesting spriteram buffering.
        There are no CPU output registers that control this so we
        assume it happens automatically every frame, at the end of vblank */
	buffer_spriteram16_w(space,0,0,0xffff);
}
