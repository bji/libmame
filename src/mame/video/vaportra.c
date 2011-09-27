/***************************************************************************

   Vapour Trail Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************

    2 Data East 55 chips for playfields (same as Dark Seal, etc)
    1 Data East MXC-06 chip for sprites (same as Bad Dudes, etc)

***************************************************************************/

#include "emu.h"
#include "video/deco16ic.h"
#include "includes/vaportra.h"
#include "video/decmxc06.h"

/******************************************************************************/

WRITE16_HANDLER( vaportra_priority_w )
{
	vaportra_state *state = space->machine().driver_data<vaportra_state>();
	COMBINE_DATA(&state->m_priority[offset]);
}

/******************************************************************************/

static void update_24bitcol( running_machine &machine, int offset )
{
	UINT8 r, g, b;

	r = (machine.generic.paletteram.u16[offset] >> 0) & 0xff;
	g = (machine.generic.paletteram.u16[offset] >> 8) & 0xff;
	b = (machine.generic.paletteram2.u16[offset] >> 0) & 0xff;

	palette_set_color(machine, offset, MAKE_RGB(r,g,b));
}

WRITE16_HANDLER( vaportra_palette_24bit_rg_w )
{
	COMBINE_DATA(&space->machine().generic.paletteram.u16[offset]);
	update_24bitcol(space->machine(), offset);
}

WRITE16_HANDLER( vaportra_palette_24bit_b_w )
{
	COMBINE_DATA(&space->machine().generic.paletteram2.u16[offset]);
	update_24bitcol(space->machine(), offset);
}

/******************************************************************************/


SCREEN_UPDATE( vaportra )
{
	vaportra_state *state = screen->machine().driver_data<vaportra_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);
	int pri = state->m_priority[0] & 0x03;

	flip_screen_set(screen->machine(), !BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, 0, 0);
	deco16ic_pf_update(state->m_deco_tilegen2, 0, 0);

	screen->machine().device<deco_mxc06_device>("spritegen")->set_pri_type(1); // force priorities to be handled in a different way for this driver for now

	/* Draw playfields */
	if (pri == 0)
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0);
		screen->machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen->machine(), bitmap, cliprect, screen->machine().generic.buffered_spriteram.u16, 0, state->m_priority[1], 0x0f);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	}
	else if (pri == 1)
	{
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0);
		screen->machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen->machine(), bitmap, cliprect, screen->machine().generic.buffered_spriteram.u16, 0, state->m_priority[1], 0x0f);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	}
	else if (pri == 2)
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
		screen->machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen->machine(), bitmap, cliprect, screen->machine().generic.buffered_spriteram.u16, 0, state->m_priority[1], 0x0f);
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0);
	}
	else
	{
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
		screen->machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen->machine(), bitmap, cliprect, screen->machine().generic.buffered_spriteram.u16, 0, state->m_priority[1], 0x0f);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0);
	}

	screen->machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen->machine(), bitmap, cliprect, screen->machine().generic.buffered_spriteram.u16, 1, state->m_priority[1], 0x0f);
	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}
