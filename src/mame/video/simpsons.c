#include "emu.h"
#include "video/konicdev.h"
#include "includes/simpsons.h"

/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void simpsons_tile_callback( running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	simpsons_state *state = machine.driver_data<simpsons_state>();
	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*color = state->m_layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K053247

***************************************************************************/

void simpsons_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	simpsons_state *state = machine.driver_data<simpsons_state>();
	int pri = (*color & 0x0f80) >> 6;	/* ??????? */

	if (pri <= state->m_layerpri[2])
		*priority_mask = 0;
	else if (pri > state->m_layerpri[2] && pri <= state->m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->m_layerpri[1] && pri <= state->m_layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = state->m_sprite_colorbase + (*color & 0x001f);
}


/***************************************************************************

  Extra video banking

***************************************************************************/

static READ8_HANDLER( simpsons_k052109_r )
{
	simpsons_state *state = space->machine().driver_data<simpsons_state>();
	return k052109_r(state->m_k052109, offset + 0x2000);
}

static WRITE8_HANDLER( simpsons_k052109_w )
{
	simpsons_state *state = space->machine().driver_data<simpsons_state>();
	k052109_w(state->m_k052109, offset + 0x2000, data);
}

static READ8_HANDLER( simpsons_k053247_r )
{
	simpsons_state *state = space->machine().driver_data<simpsons_state>();
	int offs;

	if (offset < 0x1000)
	{
		offs = offset >> 1;

		if (offset & 1)
			return(state->m_spriteram[offs] & 0xff);
		else
			return(state->m_spriteram[offs] >> 8);
	}
	else
		return state->m_xtraram[offset - 0x1000];
}

static WRITE8_HANDLER( simpsons_k053247_w )
{
	simpsons_state *state = space->machine().driver_data<simpsons_state>();
	int offs;

	if (offset < 0x1000)
	{
		UINT16 *spriteram = state->m_spriteram;
		offs = offset >> 1;

		if (offset & 1)
			spriteram[offs] = (spriteram[offs] & 0xff00) | data;
		else
			spriteram[offs] = (spriteram[offs] & 0x00ff) | (data << 8);
	}
	else state->m_xtraram[offset - 0x1000] = data;
}

void simpsons_video_banking( running_machine &machine, int bank )
{
	simpsons_state *state = machine.driver_data<simpsons_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	if (bank & 1)
	{
		space->install_read_bank(0x0000, 0x0fff, "bank5");
		space->install_legacy_write_handler(0x0000, 0x0fff, FUNC(paletteram_xBBBBBGGGGGRRRRR_be_w));
		memory_set_bankptr(machine, "bank5", machine.generic.paletteram.v);
	}
	else
		space->install_legacy_readwrite_handler(*state->m_k052109, 0x0000, 0x0fff, FUNC(k052109_r), FUNC(k052109_w));

	if (bank & 2)
		space->install_legacy_readwrite_handler(0x2000, 0x3fff, FUNC(simpsons_k053247_r), FUNC(simpsons_k053247_w));
	else
		space->install_legacy_readwrite_handler(0x2000, 0x3fff, FUNC(simpsons_k052109_r), FUNC(simpsons_k052109_w));
}



/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE( simpsons )
{
	simpsons_state *state = screen->machine().driver_data<simpsons_state>();
	int layer[3], bg_colorbase;

	bg_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI0);
	state->m_sprite_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI1);
	state->m_layer_colorbase[0] = k053251_get_palette_index(state->m_k053251, K053251_CI2);
	state->m_layer_colorbase[1] = k053251_get_palette_index(state->m_k053251, K053251_CI3);
	state->m_layer_colorbase[2] = k053251_get_palette_index(state->m_k053251, K053251_CI4);

	k052109_tilemap_update(state->m_k052109);

	layer[0] = 0;
	state->m_layerpri[0] = k053251_get_priority(state->m_k053251, K053251_CI2);
	layer[1] = 1;
	state->m_layerpri[1] = k053251_get_priority(state->m_k053251, K053251_CI3);
	layer[2] = 2;
	state->m_layerpri[2] = k053251_get_priority(state->m_k053251, K053251_CI4);

	konami_sortlayers3(layer, state->m_layerpri);

	bitmap_fill(screen->machine().priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 16 * bg_colorbase);
	k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, layer[0], 0, 1);
	k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, layer[1], 0, 2);
	k052109_tilemap_draw(state->m_k052109, bitmap, cliprect, layer[2], 0, 4);

	k053247_sprites_draw(state->m_k053246, bitmap, cliprect);
	return 0;
}
