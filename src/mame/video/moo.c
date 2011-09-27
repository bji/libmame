/***************************************************************************

 Wild West C.O.W.boys of Moo Mesa
 Bucky O'Hare
 (c) 1992 Konami

 Video hardware emulation.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/moo.h"

void moo_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	moo_state *state = machine.driver_data<moo_state>();
	int pri = (*color & 0x03e0) >> 4;

	if (pri <= state->m_layerpri[2])
		*priority_mask = 0;
	else if (pri <= state->m_layerpri[1])
		*priority_mask = 0xf0;
	else if (pri <= state->m_layerpri[0])
		*priority_mask = 0xf0|0xcc;
	else
		*priority_mask = 0xf0|0xcc|0xaa;

	*color = state->m_sprite_colorbase | (*color & 0x001f);
}

void moo_tile_callback( running_machine &machine, int layer, int *code, int *color, int *flags )
{
	moo_state *state = machine.driver_data<moo_state>();
	*color = state->m_layer_colorbase[layer] | (*color >> 2 & 0x0f);
}

VIDEO_START(moo)
{
	moo_state *state = machine.driver_data<moo_state>();

	assert(machine.primary_screen->format() == BITMAP_FORMAT_RGB32);

	state->m_alpha_enabled = 0;

	if (!strcmp(machine.system().name, "bucky") || !strcmp(machine.system().name, "buckyua") || !strcmp(machine.system().name, "buckyaa"))
	{
		// Bucky doesn't chain tilemaps
		k056832_set_layer_association(state->m_k056832, 0);

		k056832_set_layer_offs(state->m_k056832, 0, -2, 0);
		k056832_set_layer_offs(state->m_k056832, 1,  2, 0);
		k056832_set_layer_offs(state->m_k056832, 2,  4, 0);
		k056832_set_layer_offs(state->m_k056832, 3,  6, 0);
	}
	else
	{
		// other than the intro showing one blank line alignment is good through the game
		k056832_set_layer_offs(state->m_k056832, 0, -2 + 1, 0);
		k056832_set_layer_offs(state->m_k056832, 1,  2 + 1, 0);
		k056832_set_layer_offs(state->m_k056832, 2,  4 + 1, 0);
		k056832_set_layer_offs(state->m_k056832, 3,  6 + 1, 0);
	}
}

SCREEN_UPDATE(moo)
{
	moo_state *state = screen->machine().driver_data<moo_state>();
	static const int K053251_CI[4] = { K053251_CI1, K053251_CI2, K053251_CI3, K053251_CI4 };
	int layers[3];
	int new_colorbase, plane, dirty, alpha;

	state->m_sprite_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI0);
	state->m_layer_colorbase[0] = 0x70;

	if (k056832_get_layer_association(state->m_k056832))
	{
		for (plane = 1; plane < 4; plane++)
		{
			new_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI[plane]);
			if (state->m_layer_colorbase[plane] != new_colorbase)
			{
				state->m_layer_colorbase[plane] = new_colorbase;
				k056832_mark_plane_dirty(state->m_k056832, plane);
			}
		}
	}
	else
	{
		for (dirty = 0, plane = 1; plane < 4; plane++)
		{
			new_colorbase = k053251_get_palette_index(state->m_k053251, K053251_CI[plane]);
			if (state->m_layer_colorbase[plane] != new_colorbase)
			{
				state->m_layer_colorbase[plane] = new_colorbase;
				dirty = 1;
			}
		}
		if (dirty)
			k056832_mark_all_tmaps_dirty(state->m_k056832);
	}

	layers[0] = 1;
	state->m_layerpri[0] = k053251_get_priority(state->m_k053251, K053251_CI2);
	layers[1] = 2;
	state->m_layerpri[1] = k053251_get_priority(state->m_k053251, K053251_CI3);
	layers[2] = 3;
	state->m_layerpri[2] = k053251_get_priority(state->m_k053251, K053251_CI4);

	konami_sortlayers3(layers, state->m_layerpri);

	k054338_update_all_shadows(state->m_k054338, 0);
	k054338_fill_backcolor(state->m_k054338, bitmap, 0);

	bitmap_fill(screen->machine().priority_bitmap, cliprect, 0);

	if (state->m_layerpri[0] < k053251_get_priority(state->m_k053251, K053251_CI1))	/* bucky hides back layer behind background */
		k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, layers[0], 0, 1);

	k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, layers[1], 0, 2);

	// Enabling alpha improves fog and fading in Moo but causes other things to disappear.
	// There is probably a control bit somewhere to turn off alpha blending.
	state->m_alpha_enabled = k054338_register_r(state->m_k054338, K338_REG_CONTROL) & K338_CTL_MIXPRI; // DUMMY

	alpha = (state->m_alpha_enabled) ? k054338_set_alpha_level(state->m_k054338, 1) : 255;

	if (alpha > 0)
		k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, layers[2], TILEMAP_DRAW_ALPHA(alpha), 4);

	k053247_sprites_draw(state->m_k053246, bitmap, cliprect);

	k056832_tilemap_draw(state->m_k056832, bitmap, cliprect, 0, 0, 0);
	return 0;
}
