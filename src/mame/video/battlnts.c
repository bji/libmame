#include "emu.h"
#include "video/konicdev.h"
#include "includes/battlnts.h"

/***************************************************************************

  Callback for the K007342

***************************************************************************/

void battlnts_tile_callback(running_machine *machine, int layer, int bank, int *code, int *color, int *flags)
{
	battlnts_state *state = machine->driver_data<battlnts_state>();

	*code |= ((*color & 0x0f) << 9) | ((*color & 0x40) << 2);
	*color = state->layer_colorbase[layer];
}

/***************************************************************************

  Callback for the K007420

***************************************************************************/

void battlnts_sprite_callback(running_machine *machine, int *code,int *color)
{
	battlnts_state *state = machine->driver_data<battlnts_state>();

	*code |= ((*color & 0xc0) << 2) | state->spritebank;
	*code = (*code << 2) | ((*color & 0x30) >> 4);
	*color = 0;
}

WRITE8_HANDLER( battlnts_spritebank_w )
{
	battlnts_state *state = space->machine->driver_data<battlnts_state>();
	state->spritebank = 1024 * (data & 1);
}

/***************************************************************************

  Screen Refresh

***************************************************************************/

SCREEN_UPDATE( battlnts )
{
	battlnts_state *state = screen->machine->driver_data<battlnts_state>();

	k007342_tilemap_update(state->k007342);

	k007342_tilemap_draw(state->k007342, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE ,0);
	k007420_sprites_draw(state->k007420, bitmap, cliprect, screen->machine->gfx[1]);
	k007342_tilemap_draw(state->k007342, bitmap, cliprect, 0, 1 | TILEMAP_DRAW_OPAQUE ,0);
	return 0;
}
