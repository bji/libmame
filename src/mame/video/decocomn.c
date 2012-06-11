/* Common DECO video functions (general, not sorted by IC) */
/* I think most of this stuff is driver specific and really shouldn't be in a device at all.
   It was only put here because I wanted to split deco_tilegen1 to just be the device for the
   tilemap chips, and not contain all this extra unrelated stuff */


#include "emu.h"
#include "video/decocomn.h"
#include "ui.h"


typedef struct _decocomn_state decocomn_state;
struct _decocomn_state
{
	screen_device *screen;
	UINT8 *dirty_palette;
	UINT16 priority;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE decocomn_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == DECOCOMN);

	return (decocomn_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const decocomn_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == DECOCOMN));
	return (const decocomn_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

/* Later games have double buffered paletteram - the real palette ram is
only updated on a DMA call */

WRITE16_DEVICE_HANDLER( decocomn_nonbuffered_palette_w )
{
	int r,g,b;

	driver_device *state = device->machine().driver_data();
	COMBINE_DATA(&state->m_generic_paletteram_16[offset]);
	if (offset&1) offset--;

	b = (state->m_generic_paletteram_16[offset] >> 0) & 0xff;
	g = (state->m_generic_paletteram_16[offset + 1] >> 8) & 0xff;
	r = (state->m_generic_paletteram_16[offset + 1] >> 0) & 0xff;

	palette_set_color(device->machine(), offset / 2, MAKE_RGB(r,g,b));
}

WRITE16_DEVICE_HANDLER( decocomn_buffered_palette_w )
{
	decocomn_state *decocomn = get_safe_token(device);

	driver_device *state = device->machine().driver_data();
	COMBINE_DATA(&state->m_generic_paletteram_16[offset]);

	decocomn->dirty_palette[offset / 2] = 1;
}

WRITE16_DEVICE_HANDLER( decocomn_palette_dma_w )
{
	decocomn_state *decocomn = get_safe_token(device);
	driver_device *state = device->machine().driver_data();
	const int m = device->machine().total_colors();
	int r, g, b, i;

	for (i = 0; i < m; i++)
	{
		if (decocomn->dirty_palette[i])
		{
			decocomn->dirty_palette[i] = 0;

			b = (state->m_generic_paletteram_16[i * 2] >> 0) & 0xff;
			g = (state->m_generic_paletteram_16[i * 2 + 1] >> 8) & 0xff;
			r = (state->m_generic_paletteram_16[i * 2 + 1] >> 0) & 0xff;

			palette_set_color(device->machine(), i, MAKE_RGB(r,g,b));
		}
	}
}

/*****************************************************************************************/

/* */
READ16_DEVICE_HANDLER( decocomn_71_r )
{
	return 0xffff;
}

WRITE16_DEVICE_HANDLER( decocomn_priority_w )
{
	decocomn_state *decocomn = get_safe_token(device);
	decocomn->priority = data;
}

READ16_DEVICE_HANDLER( decocomn_priority_r )
{
	decocomn_state *decocomn = get_safe_token(device);
	return decocomn->priority;
}


/******************************************************************************/

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( decocomn )
{
	decocomn_state *decocomn = get_safe_token(device);
	const decocomn_interface *intf = get_interface(device);
//  int width, height;

	decocomn->screen = device->machine().device<screen_device>(intf->screen);
//  width = decocomn->screen->width();
//  height = decocomn->screen->height();

	decocomn->dirty_palette = auto_alloc_array_clear(device->machine(), UINT8, 4096);

	device->save_item(NAME(decocomn->priority));
	device->save_pointer(NAME(decocomn->dirty_palette), 4096);
}

static DEVICE_RESET( decocomn )
{
	decocomn_state *decocomn = get_safe_token(device);
	decocomn->priority = 0;
}


DEVICE_GET_INFO( decocomn )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(decocomn_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(decocomn);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(decocomn);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Data East Common Video Functions");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Data East Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEFINE_LEGACY_DEVICE(DECOCOMN, decocomn);
