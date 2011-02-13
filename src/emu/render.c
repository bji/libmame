/***************************************************************************

    render.c

    Core rendering system.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************

    Windows-specific to-do:
        * no fallback if we run out of video memory

    Longer-term to do: (once old renderer is gone)
        * make vector updates asynchronous

****************************************************************************

    Overview of objects:

        render_target -- This represents a final rendering target. It
            is specified using integer width/height values, can have
            non-square pixels, and you can specify its rotation. It is
            what really determines the final rendering details. The OSD
            layer creates one or more of these to encapsulate the
            rendering process. Each render_target holds a list of
            layout_files that it can use for drawing. When rendering, it
            makes use of both layout_files and render_containers.

        render_container -- Containers are the top of a hierarchy that is
            not directly related to the objects above. Containers hold
            high level primitives that are generated at runtime by the
            video system. They are used currently for each screen and
            the user interface. These high-level primitives are broken down
            into low-level primitives at render time.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "render.h"
#include "rendfont.h"
#include "rendlay.h"
#include "rendutil.h"
#include "config.h"
#include "xmlfile.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define INTERNAL_FLAG_CHAR		0x00000001

enum
{
	COMPONENT_TYPE_IMAGE = 0,
	COMPONENT_TYPE_RECT,
	COMPONENT_TYPE_DISK,
	COMPONENT_TYPE_MAX
};


enum
{
	CONTAINER_ITEM_LINE = 0,
	CONTAINER_ITEM_QUAD,
	CONTAINER_ITEM_MAX
};



//**************************************************************************
//  MACROS
//**************************************************************************

#define ISWAP(var1, var2) do { int temp = var1; var1 = var2; var2 = temp; } while (0)
#define FSWAP(var1, var2) do { float temp = var1; var1 = var2; var2 = temp; } while (0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// an object_transform is used to track transformations when building an object list
struct object_transform
{
	float				xoffs, yoffs;		// offset transforms
	float				xscale, yscale;		// scale transforms
	render_color		color;				// color transform
	int					orientation;		// orientation transform
	bool				no_center;			// center the container?
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// precomputed UV coordinates for various orientations
static const render_quad_texuv oriented_texcoords[8] =
{
	{ { 0,0 }, { 1,0 }, { 0,1 }, { 1,1 } },		// 0
	{ { 1,0 }, { 0,0 }, { 1,1 }, { 0,1 } },		// ORIENTATION_FLIP_X
	{ { 0,1 }, { 1,1 }, { 0,0 }, { 1,0 } },		// ORIENTATION_FLIP_Y
	{ { 1,1 }, { 0,1 }, { 1,0 }, { 0,0 } },		// ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y
	{ { 0,0 }, { 0,1 }, { 1,0 }, { 1,1 } },		// ORIENTATION_SWAP_XY
	{ { 0,1 }, { 0,0 }, { 1,1 }, { 1,0 } },		// ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X
	{ { 1,0 }, { 1,1 }, { 0,0 }, { 0,1 } },		// ORIENTATION_SWAP_XY | ORIENTATION_FLIP_Y
	{ { 1,1 }, { 1,0 }, { 0,1 }, { 0,0 } }		// ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y
};

// layer orders
static const int layer_order_standard[] = { ITEM_LAYER_SCREEN, ITEM_LAYER_OVERLAY, ITEM_LAYER_BACKDROP, ITEM_LAYER_BEZEL };
static const int layer_order_alternate[] = { ITEM_LAYER_BACKDROP, ITEM_LAYER_SCREEN, ITEM_LAYER_OVERLAY, ITEM_LAYER_BEZEL };



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  apply_orientation - apply orientation to a
//  set of bounds
//-------------------------------------------------

inline void apply_orientation(render_bounds &bounds, int orientation)
{
	// swap first
	if (orientation & ORIENTATION_SWAP_XY)
	{
		FSWAP(bounds.x0, bounds.y0);
		FSWAP(bounds.x1, bounds.y1);
	}

	// apply X flip
	if (orientation & ORIENTATION_FLIP_X)
	{
		bounds.x0 = 1.0f - bounds.x0;
		bounds.x1 = 1.0f - bounds.x1;
	}

	// apply Y flip
	if (orientation & ORIENTATION_FLIP_Y)
	{
		bounds.y0 = 1.0f - bounds.y0;
		bounds.y1 = 1.0f - bounds.y1;
	}
}


//-------------------------------------------------
//  normalize_bounds - normalize bounds so that
//  x0/y0 are less than x1/y1
//-------------------------------------------------

inline void normalize_bounds(render_bounds &bounds)
{
	if (bounds.x0 > bounds.x1)
		FSWAP(bounds.x0, bounds.x1);
	if (bounds.y0 > bounds.y1)
		FSWAP(bounds.y0, bounds.y1);
}


//-------------------------------------------------
//  get_layer_and_blendmode - return the
//  appropriate layer index and blendmode
//-------------------------------------------------

inline item_layer get_layer_and_blendmode(const layout_view &view, int index, int &blendmode)
{
	//  if we have multiple backdrop pieces and no overlays, render:
    //      backdrop (add) + screens (add) + bezels (alpha)
    //  else render:
    //      screens (add) + overlays (RGB multiply) + backdrop (add) + bezels (alpha)

    const int *layer_order = layer_order_standard;
	if (view.first_item(ITEM_LAYER_BACKDROP) != NULL && view.first_item(ITEM_LAYER_BACKDROP)->next() != NULL && view.first_item(ITEM_LAYER_OVERLAY) == NULL)
		layer_order = layer_order_alternate;

	// select the layer
	int layer = layer_order[index];

	// pick a blendmode
	if (layer == ITEM_LAYER_SCREEN && layer_order == layer_order_standard)
		blendmode = -1;
	else if (layer == ITEM_LAYER_SCREEN || (layer == ITEM_LAYER_BACKDROP && layer_order == layer_order_standard))
		blendmode = BLENDMODE_ADD;
	else if (layer == ITEM_LAYER_OVERLAY)
		blendmode = BLENDMODE_RGB_MULTIPLY;
	else
		blendmode = BLENDMODE_ALPHA;

	return item_layer(layer);
}



//**************************************************************************
//  RENDER PRIMITIVE
//**************************************************************************

//-------------------------------------------------
//  reset - reset the state of a primitive after
//  it is re-allocated
//-------------------------------------------------

void render_primitive::reset()
{
	memset(&type, 0, FPTR(&texcoords + 1) - FPTR(&type));
}



//**************************************************************************
//  RENDER PRIMITIVE LIST
//**************************************************************************

//-------------------------------------------------
//  render_primitive_list - constructor
//-------------------------------------------------

render_primitive_list::render_primitive_list()
	: m_lock(osd_lock_alloc())
{
}


//-------------------------------------------------
//  ~render_primitive_list - destructor
//-------------------------------------------------

render_primitive_list::~render_primitive_list()
{
	release_all();
	osd_lock_free(m_lock);
}


//-------------------------------------------------
//  add_reference - add a new reference
//-------------------------------------------------

inline void render_primitive_list::add_reference(void *refptr)
{
	// skip if we already have one
	if (has_reference(refptr))
		return;

	// set the refptr and link us into the list
	reference *ref = m_reference_allocator.alloc();
	ref->m_refptr = refptr;
	m_reflist.append(*ref);
}


//-------------------------------------------------
//  has_reference - find a refptr in a reference
//  list
//-------------------------------------------------

inline bool render_primitive_list::has_reference(void *refptr) const
{
	// skip if we already have one
	for (reference *ref = m_reflist.first(); ref != NULL; ref = ref->next())
		if (ref->m_refptr == refptr)
			return true;
	return false;
}


//-------------------------------------------------
//  alloc - allocate a new empty primitive
//-------------------------------------------------

inline render_primitive *render_primitive_list::alloc(render_primitive::primitive_type type)
{
	render_primitive *result = m_primitive_allocator.alloc();
	result->reset();
	result->type = type;
	return result;
}


//-------------------------------------------------
//  release_all - release the contents of
//  a render list
//-------------------------------------------------

void render_primitive_list::release_all()
{
	// release all the live items while under the lock
	acquire_lock();
	m_primitive_allocator.reclaim_all(m_primlist);
	m_reference_allocator.reclaim_all(m_reflist);
	release_lock();
}


//-------------------------------------------------
//  append_or_return - append a primitive to the
//  end of the list, or return it to the free
//  list, based on a flag
//-------------------------------------------------

void render_primitive_list::append_or_return(render_primitive &prim, bool clipped)
{
	if (!clipped)
		m_primlist.append(prim);
	else
		m_primitive_allocator.reclaim(prim);
}



//**************************************************************************
//  RENDER TEXTURE
//**************************************************************************

//-------------------------------------------------
//  render_texture - constructor
//-------------------------------------------------

render_texture::render_texture()
	: m_manager(NULL),
	  m_next(NULL),
	  m_bitmap(NULL),
	  m_palette(NULL),
	  m_format(TEXFORMAT_ARGB32),
	  m_scaler(NULL),
	  m_param(NULL),
	  m_curseq(0),
	  m_bcglookup(NULL),
	  m_bcglookup_entries(0)
{
	m_sbounds.min_x = m_sbounds.min_y = m_sbounds.max_x = m_sbounds.max_y = 0;
	memset(m_scaled, 0, sizeof(m_scaled));
}


//-------------------------------------------------
//  ~render_texture - destructor
//-------------------------------------------------

render_texture::~render_texture()
{
	release();
}


//-------------------------------------------------
//  reset - reset the state of a texture after
//  it has been re-allocated
//-------------------------------------------------

void render_texture::reset(render_manager &manager, texture_scaler_func scaler, void *param)
{
	m_manager = &manager;
	m_scaler = scaler;
	m_param = param;
}


//-------------------------------------------------
//  release - release resources when we are freed
//-------------------------------------------------

void render_texture::release()
{
	// free all scaled versions
	for (int scalenum = 0; scalenum < ARRAY_LENGTH(m_scaled); scalenum++)
	{
		m_manager->invalidate_all(m_scaled[scalenum].bitmap);
		auto_free(&m_manager->machine(), m_scaled[scalenum].bitmap);
		m_scaled[scalenum].bitmap = NULL;
		m_scaled[scalenum].seqid = 0;
	}

	// invalidate references to the original bitmap as well
	m_manager->invalidate_all(m_bitmap);
	m_bitmap = NULL;
	m_sbounds.min_x = m_sbounds.min_y = m_sbounds.max_x = m_sbounds.max_y = 0;
	m_format = TEXFORMAT_ARGB32;
	m_curseq = 0;

	// release palette references
	if (m_palette != NULL)
		palette_deref(m_palette);
	m_palette = NULL;

	// free any B/C/G lookup tables
	auto_free(&m_manager->machine(), m_bcglookup);
	m_bcglookup = NULL;
	m_bcglookup_entries = 0;
}


//-------------------------------------------------
//  set_bitmap - set a new source bitmap
//-------------------------------------------------

void render_texture::set_bitmap(bitmap_t *bitmap, const rectangle *sbounds, int format, palette_t *palette)
{
	// ensure we have a valid palette for palettized modes
	if (format == TEXFORMAT_PALETTE16 || format == TEXFORMAT_PALETTEA16)
		assert(palette != NULL);

	// invalidate references to the old bitmap
	if (bitmap != m_bitmap && m_bitmap != NULL)
		m_manager->invalidate_all(m_bitmap);

	// if the palette is different, adjust references
	if (palette != m_palette)
	{
		if (m_palette != NULL)
			palette_deref(m_palette);
		if (palette != NULL)
			palette_ref(palette);
	}

	// set the new bitmap/palette
	m_bitmap = bitmap;
	m_sbounds.min_x = (sbounds != NULL) ? sbounds->min_x : 0;
	m_sbounds.min_y = (sbounds != NULL) ? sbounds->min_y : 0;
	m_sbounds.max_x = (sbounds != NULL) ? sbounds->max_x : (bitmap != NULL) ? bitmap->width : 1000;
	m_sbounds.max_y = (sbounds != NULL) ? sbounds->max_y : (bitmap != NULL) ? bitmap->height : 1000;
	m_palette = palette;
	m_format = format;

	// invalidate all scaled versions
	for (int scalenum = 0; scalenum < ARRAY_LENGTH(m_scaled); scalenum++)
	{
		if (m_scaled[scalenum].bitmap != NULL)
		{
			m_manager->invalidate_all(m_scaled[scalenum].bitmap);
			auto_free(&m_manager->machine(), m_scaled[scalenum].bitmap);
		}
		m_scaled[scalenum].bitmap = NULL;
		m_scaled[scalenum].seqid = 0;
	}
}


//-------------------------------------------------
//  hq_scale - generic high quality resampling
//  scaler
//-------------------------------------------------

void render_texture::hq_scale(bitmap_t &dest, const bitmap_t &source, const rectangle &sbounds, void *param)
{
	render_color color = { 1.0f, 1.0f, 1.0f, 1.0f };
	render_resample_argb_bitmap_hq(dest.base, dest.rowpixels, dest.width, dest.height, &source, &sbounds, &color);
}


//-------------------------------------------------
//  get_scaled - get a scaled bitmap (if we can)
//-------------------------------------------------

bool render_texture::get_scaled(UINT32 dwidth, UINT32 dheight, render_texinfo &texinfo, render_primitive_list &primlist)
{
	// source width/height come from the source bounds
	int swidth = m_sbounds.max_x - m_sbounds.min_x;
	int sheight = m_sbounds.max_y - m_sbounds.min_y;

	// ensure height/width are non-zero
	if (dwidth < 1) dwidth = 1;
	if (dheight < 1) dheight = 1;

	// are we scaler-free? if so, just return the source bitmap
	const rgb_t *palbase = (m_format == TEXFORMAT_PALETTE16 || m_format == TEXFORMAT_PALETTEA16) ? palette_entry_list_adjusted(m_palette) : NULL;
	if (m_scaler == NULL || (m_bitmap != NULL && swidth == dwidth && sheight == dheight))
	{
		// add a reference and set up the source bitmap
		primlist.add_reference(m_bitmap);
		UINT8 bpp = (m_format == TEXFORMAT_PALETTE16 || m_format == TEXFORMAT_PALETTEA16 || m_format == TEXFORMAT_RGB15 || m_format == TEXFORMAT_YUY16) ? 16 : 32;
		texinfo.base = (UINT8 *)m_bitmap->base + (m_sbounds.min_y * m_bitmap->rowpixels + m_sbounds.min_x) * (bpp / 8);
		texinfo.rowpixels = m_bitmap->rowpixels;
		texinfo.width = swidth;
		texinfo.height = sheight;
		texinfo.palette = palbase;
		texinfo.seqid = ++m_curseq;
		return true;
	}

	// is it a size we already have?
	scaled_texture *scaled = NULL;
	int scalenum;
	for (scalenum = 0; scalenum < ARRAY_LENGTH(m_scaled); scalenum++)
	{
		scaled = &m_scaled[scalenum];

		// we need a non-NULL bitmap with matching dest size
		if (scaled->bitmap != NULL && dwidth == scaled->bitmap->width && dheight == scaled->bitmap->height)
			break;
	}

	// did we get one?
	if (scalenum == ARRAY_LENGTH(m_scaled))
	{
		int lowest = -1;

		// didn't find one -- take the entry with the lowest seqnum
		for (scalenum = 0; scalenum < ARRAY_LENGTH(m_scaled); scalenum++)
			if ((lowest == -1 || m_scaled[scalenum].seqid < m_scaled[lowest].seqid) && !primlist.has_reference(m_scaled[scalenum].bitmap))
				lowest = scalenum;
		assert_always(lowest != -1, "Too many live texture instances!");

		// throw out any existing entries
		scaled = &m_scaled[lowest];
		if (scaled->bitmap != NULL)
		{
			m_manager->invalidate_all(scaled->bitmap);
			auto_free(&m_manager->machine(), scaled->bitmap);
		}

		// allocate a new bitmap
		scaled->bitmap = auto_alloc(&m_manager->machine(), bitmap_t(dwidth, dheight, BITMAP_FORMAT_ARGB32));
		scaled->seqid = ++m_curseq;

		// let the scaler do the work
		(*m_scaler)(*scaled->bitmap, *m_bitmap, m_sbounds, m_param);
	}

	// finally fill out the new info
	primlist.add_reference(scaled->bitmap);
	texinfo.base = scaled->bitmap->base;
	texinfo.rowpixels = scaled->bitmap->rowpixels;
	texinfo.width = dwidth;
	texinfo.height = dheight;
	texinfo.palette = palbase;
	texinfo.seqid = scaled->seqid;
	return true;
}


//-------------------------------------------------
//  get_adjusted_palette - return the adjusted
//  palette for a texture
//-------------------------------------------------

const rgb_t *render_texture::get_adjusted_palette(render_container &container)
{
	const rgb_t *adjusted;
	int numentries;

	// override the palette with our adjusted palette
	switch (m_format)
	{
		case TEXFORMAT_PALETTE16:
		case TEXFORMAT_PALETTEA16:

			// if no adjustment necessary, return the raw palette
			assert(m_palette != NULL);
			adjusted = palette_entry_list_adjusted(m_palette);
			if (!container.has_brightness_contrast_gamma_changes())
				return adjusted;

			// if this is the machine palette, return our precomputed adjusted palette
			adjusted = container.bcg_lookup_table(m_format, m_palette);
			if (adjusted != NULL)
				return adjusted;

			// otherwise, ensure we have memory allocated and compute the adjusted result ourself
			numentries = palette_get_num_colors(m_palette) * palette_get_num_groups(m_palette);
			if (m_bcglookup == NULL || m_bcglookup_entries < numentries)
			{
				rgb_t *newlookup = auto_alloc_array(&m_manager->machine(), rgb_t, numentries);
				memcpy(newlookup, m_bcglookup, m_bcglookup_entries * sizeof(rgb_t));
				auto_free(&m_manager->machine(), m_bcglookup);
				m_bcglookup = newlookup;
				m_bcglookup_entries = numentries;
			}
			for (int index = 0; index < numentries; index++)
			{
				UINT8 r = container.apply_brightness_contrast_gamma(RGB_RED(adjusted[index]));
				UINT8 g = container.apply_brightness_contrast_gamma(RGB_GREEN(adjusted[index]));
				UINT8 b = container.apply_brightness_contrast_gamma(RGB_BLUE(adjusted[index]));
				m_bcglookup[index] = MAKE_ARGB(RGB_ALPHA(adjusted[index]), r, g, b);
			}
			return m_bcglookup;

		case TEXFORMAT_RGB15:

			// if no adjustment necessary, return NULL
			if (!container.has_brightness_contrast_gamma_changes() && m_palette == NULL)
				return NULL;

			// if no palette, return the standard lookups
			if (m_palette == NULL)
				return container.bcg_lookup_table(m_format);

			// otherwise, ensure we have memory allocated and compute the adjusted result ourself
			assert(palette_get_num_colors(m_palette) == 32);
			adjusted = palette_entry_list_adjusted(m_palette);
			if (m_bcglookup == NULL || m_bcglookup_entries < 4 * 32)
			{
				rgb_t *newlookup = auto_alloc_array(&m_manager->machine(), rgb_t, 4 * 32);
				memcpy(newlookup, m_bcglookup, m_bcglookup_entries * sizeof(rgb_t));
				auto_free(&m_manager->machine(), m_bcglookup);
				m_bcglookup = newlookup;
				m_bcglookup_entries = 4 * 32;
			}

			// otherwise, return the 32-entry BCG lookups
			for (int index = 0; index < 32; index++)
			{
				UINT8 val = container.apply_brightness_contrast_gamma(RGB_GREEN(adjusted[index]));
				m_bcglookup[0x00 + index] = val << 0;
				m_bcglookup[0x20 + index] = val << 8;
				m_bcglookup[0x40 + index] = val << 16;
				m_bcglookup[0x60 + index] = val << 24;
			}
			return m_bcglookup;

		case TEXFORMAT_RGB32:
		case TEXFORMAT_ARGB32:
		case TEXFORMAT_YUY16:

			// if no adjustment necessary, return NULL
			if (!container.has_brightness_contrast_gamma_changes() && m_palette == NULL)
				return NULL;

			// if no palette, return the standard lookups
			if (m_palette == NULL)
				return container.bcg_lookup_table(m_format);

			// otherwise, ensure we have memory allocated and compute the adjusted result ourself
			assert(palette_get_num_colors(m_palette) == 256);
			adjusted = palette_entry_list_adjusted(m_palette);
			if (m_bcglookup == NULL || m_bcglookup_entries < 4 * 256)
			{
				rgb_t *newlookup = auto_alloc_array(&m_manager->machine(), rgb_t, 4 * 256);
				memcpy(newlookup, m_bcglookup, m_bcglookup_entries * sizeof(rgb_t));
				auto_free(&m_manager->machine(), m_bcglookup);
				m_bcglookup = newlookup;
				m_bcglookup_entries = 4 * 256;
			}

			// otherwise, return the 32-entry BCG lookups
			for (int index = 0; index < 256; index++)
			{
				UINT8 val = container.apply_brightness_contrast_gamma(RGB_GREEN(adjusted[index]));
				m_bcglookup[0x000 + index] = val << 0;
				m_bcglookup[0x100 + index] = val << 8;
				m_bcglookup[0x200 + index] = val << 16;
				m_bcglookup[0x300 + index] = val << 24;
			}
			return m_bcglookup;

		default:
			assert(FALSE);
	}

	return NULL;
}



//**************************************************************************
//  RENDER CONTAINER
//**************************************************************************

//-------------------------------------------------
//  render_container - constructor
//-------------------------------------------------

render_container::render_container(render_manager &manager, screen_device *screen)
	: m_next(NULL),
	  m_manager(manager),
	  m_itemlist(manager.machine().m_respool),
	  m_item_allocator(manager.machine().m_respool),
	  m_screen(screen),
	  m_overlaybitmap(NULL),
	  m_overlaytexture(NULL),
	  m_palclient(NULL)
{
	// all palette entries are opaque by default
	for (int color = 0; color < ARRAY_LENGTH(m_bcglookup); color++)
		m_bcglookup[color] = MAKE_ARGB(0xff,0x00,0x00,0x00);

	// make sure it is empty
	empty();

	// if we have a screen, read and apply the options
	if (screen != NULL)
	{
		// set the initial orientation and brightness/contrast/gamma
		m_user.m_orientation = manager.machine().gamedrv->flags & ORIENTATION_MASK;
		m_user.m_brightness = options_get_float(manager.machine().options(), OPTION_BRIGHTNESS);
		m_user.m_contrast = options_get_float(manager.machine().options(), OPTION_CONTRAST);
		m_user.m_gamma = options_get_float(manager.machine().options(), OPTION_GAMMA);
	}

	// allocate a client to the main palette
	if (manager.machine().palette != NULL)
		m_palclient = palette_client_alloc(manager.machine().palette);
	recompute_lookups();
}


//-------------------------------------------------
//  ~render_container - destructor
//-------------------------------------------------

render_container::~render_container()
{
	// free all the container items
	empty();

	// free the overlay texture
	m_manager.texture_free(m_overlaytexture);

	// release our palette client
	if (m_palclient != NULL)
		palette_client_free(m_palclient);
}


//-------------------------------------------------
//  set_overlay - set the overlay bitmap for the
//  container
//-------------------------------------------------

void render_container::set_overlay(bitmap_t *bitmap)
{
	// free any existing texture
	m_manager.texture_free(m_overlaytexture);

	// set the new data and allocate the texture
	m_overlaybitmap = bitmap;
	if (m_overlaybitmap != NULL)
	{
		m_overlaytexture = m_manager.texture_alloc(render_container::overlay_scale);
		m_overlaytexture->set_bitmap(bitmap, NULL, TEXFORMAT_ARGB32);
	}
}


//-------------------------------------------------
//  set_user_settings - set the current user
//  settings for a container
//-------------------------------------------------

void render_container::set_user_settings(const user_settings &settings)
{
	m_user = settings;
	recompute_lookups();
}


//-------------------------------------------------
//  add_line - add a line item to this container
//-------------------------------------------------

void render_container::add_line(float x0, float y0, float x1, float y1, float width, rgb_t argb, UINT32 flags)
{
	item &newitem = add_generic(CONTAINER_ITEM_LINE, x0, y0, x1, y1, argb);
	newitem.m_width = width;
	newitem.m_flags = flags;
}


//-------------------------------------------------
//  add_quad - add a quad item to this container
//-------------------------------------------------

void render_container::add_quad(float x0, float y0, float x1, float y1, rgb_t argb, render_texture *texture, UINT32 flags)
{
	item &newitem = add_generic(CONTAINER_ITEM_QUAD, x0, y0, x1, y1, argb);
	newitem.m_texture = texture;
	newitem.m_flags = flags;
}


//-------------------------------------------------
//  add_char - add a char item to this container
//-------------------------------------------------

void render_container::add_char(float x0, float y0, float height, float aspect, rgb_t argb, render_font &font, UINT16 ch)
{
	// compute the bounds of the character cell and get the texture
	render_bounds bounds;
	bounds.x0 = x0;
	bounds.y0 = y0;
	render_texture *texture = font.get_char_texture_and_bounds(height, aspect, ch, bounds);

	// add it like a quad
	item &newitem = add_generic(CONTAINER_ITEM_QUAD, bounds.x0, bounds.y0, bounds.x1, bounds.y1, argb);
	newitem.m_texture = texture;
	newitem.m_flags = PRIMFLAG_TEXORIENT(ROT0) | PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);
	newitem.m_internal = INTERNAL_FLAG_CHAR;
}


//-------------------------------------------------
//  apply_brightness_contrast_gamma - apply the
//  container's brightess, contrast, and gamma to
//  an 8-bit value
//-------------------------------------------------

UINT8 render_container::apply_brightness_contrast_gamma(UINT8 value)
{
	return ::apply_brightness_contrast_gamma(value, m_user.m_brightness, m_user.m_contrast, m_user.m_gamma);
}


//-------------------------------------------------
//  apply_brightness_contrast_gamma_fp - apply the
//  container's brightess, contrast, and gamma to
//  a floating-point value
//-------------------------------------------------

float render_container::apply_brightness_contrast_gamma_fp(float value)
{
	return ::apply_brightness_contrast_gamma_fp(value, m_user.m_brightness, m_user.m_contrast, m_user.m_gamma);
}


//-------------------------------------------------
//  bcg_lookup_table - return the appropriate
//  brightness/contrast/gamma lookup table for a
//  given texture mode
//-------------------------------------------------

const rgb_t *render_container::bcg_lookup_table(int texformat, palette_t *palette)
{
	switch (texformat)
	{
		case TEXFORMAT_PALETTE16:
		case TEXFORMAT_PALETTEA16:
			return (palette != NULL && palette == palette_client_get_palette(m_palclient)) ? m_bcglookup : NULL;

		case TEXFORMAT_RGB15:
			return m_bcglookup32;

		case TEXFORMAT_RGB32:
		case TEXFORMAT_ARGB32:
		case TEXFORMAT_YUY16:
			return m_bcglookup256;

		default:
			return NULL;
	}
}


//-------------------------------------------------
//  overlay_scale - scaler for an overlay
//-------------------------------------------------

void render_container::overlay_scale(bitmap_t &dest, const bitmap_t &source, const rectangle &sbounds, void *param)
{
	// simply replicate the source bitmap over the target
	for (int y = 0; y < dest.height; y++)
	{
		UINT32 *src = (UINT32 *)source.base + (y % source.height) * source.rowpixels;
		UINT32 *dst = (UINT32 *)dest.base + y * dest.rowpixels;
		int sx = 0;

		// loop over columns
		for (int x = 0; x < dest.width; x++)
		{
			*dst++ = src[sx++];
			if (sx >= source.width)
				sx = 0;
		}
	}
}


//-------------------------------------------------
//  add_generic - add a generic item to a
//  container
//-------------------------------------------------

render_container::item &render_container::add_generic(UINT8 type, float x0, float y0, float x1, float y1, rgb_t argb)
{
	item *newitem = m_item_allocator.alloc();

	// copy the data into the new item
	newitem->m_type = type;
	newitem->m_bounds.x0 = x0;
	newitem->m_bounds.y0 = y0;
	newitem->m_bounds.x1 = x1;
	newitem->m_bounds.y1 = y1;
	newitem->m_color.r = (float)RGB_RED(argb) * (1.0f / 255.0f);
	newitem->m_color.g = (float)RGB_GREEN(argb) * (1.0f / 255.0f);
	newitem->m_color.b = (float)RGB_BLUE(argb) * (1.0f / 255.0f);
	newitem->m_color.a = (float)RGB_ALPHA(argb) * (1.0f / 255.0f);
	newitem->m_flags = 0;
	newitem->m_internal = 0;
	newitem->m_width = 0;
	newitem->m_texture = NULL;

	// add the item to the container
	return m_itemlist.append(*newitem);
}


//-------------------------------------------------
//  recompute_lookups - recompute the lookup table
//  for the render container
//-------------------------------------------------

void render_container::recompute_lookups()
{
	// recompute the 256 entry lookup table
	for (int i = 0; i < 0x100; i++)
	{
		UINT8 adjustedval = apply_brightness_contrast_gamma(i);
		m_bcglookup256[i + 0x000] = adjustedval << 0;
		m_bcglookup256[i + 0x100] = adjustedval << 8;
		m_bcglookup256[i + 0x200] = adjustedval << 16;
		m_bcglookup256[i + 0x300] = adjustedval << 24;
	}

	// recompute the 32 entry lookup table
	for (int i = 0; i < 0x20; i++)
	{
		UINT8 adjustedval = apply_brightness_contrast_gamma(pal5bit(i));
		m_bcglookup32[i + 0x000] = adjustedval << 0;
		m_bcglookup32[i + 0x020] = adjustedval << 8;
		m_bcglookup32[i + 0x040] = adjustedval << 16;
		m_bcglookup32[i + 0x060] = adjustedval << 24;
	}

	// recompute the palette entries
	if (m_palclient != NULL)
	{
		palette_t *palette = palette_client_get_palette(m_palclient);
		const pen_t *adjusted_palette = palette_entry_list_adjusted(palette);
		int colors = palette_get_num_colors(palette) * palette_get_num_groups(palette);

		for (int i = 0; i < colors; i++)
		{
			pen_t newval = adjusted_palette[i];
			m_bcglookup[i] = (newval & 0xff000000) |
									  m_bcglookup256[0x200 + RGB_RED(newval)] |
									  m_bcglookup256[0x100 + RGB_GREEN(newval)] |
									  m_bcglookup256[0x000 + RGB_BLUE(newval)];
		}
	}
}


//-------------------------------------------------
//  update_palette - update any dirty palette
//  entries
//-------------------------------------------------

void render_container::update_palette()
{
	// skip if no client
	if (m_palclient == NULL)
		return;

	// get the dirty list
	UINT32 mindirty, maxdirty;
	const UINT32 *dirty = palette_client_get_dirty_list(m_palclient, &mindirty, &maxdirty);

	// iterate over dirty items and update them
	if (dirty != NULL)
	{
		palette_t *palette = palette_client_get_palette(m_palclient);
		const pen_t *adjusted_palette = palette_entry_list_adjusted(palette);

		// loop over chunks of 32 entries, since we can quickly examine 32 at a time
		for (UINT32 entry32 = mindirty / 32; entry32 <= maxdirty / 32; entry32++)
		{
			UINT32 dirtybits = dirty[entry32];
			if (dirtybits != 0)

				// this chunk of 32 has dirty entries; fix them up
				for (UINT32 entry = 0; entry < 32; entry++)
					if (dirtybits & (1 << entry))
					{
						UINT32 finalentry = entry32 * 32 + entry;
						rgb_t newval = adjusted_palette[finalentry];
						m_bcglookup[finalentry] = (newval & 0xff000000) |
													  m_bcglookup256[0x200 + RGB_RED(newval)] |
													  m_bcglookup256[0x100 + RGB_GREEN(newval)] |
													  m_bcglookup256[0x000 + RGB_BLUE(newval)];
					}
		}
	}
}


//-------------------------------------------------
//  user_settings - constructor
//-------------------------------------------------

render_container::user_settings::user_settings()
	: m_orientation(0),
	  m_brightness(1.0f),
	  m_contrast(1.0f),
	  m_gamma(1.0f),
	  m_xscale(1.0f),
	  m_yscale(1.0f),
	  m_xoffset(0.0f),
	  m_yoffset(0.0f)
{
}



//**************************************************************************
//  RENDER TARGET
//**************************************************************************

//-------------------------------------------------
//  render_target - constructor
//-------------------------------------------------

render_target::render_target(render_manager &manager, const char *layoutfile, UINT32 flags)
	: m_next(NULL),
	  m_manager(manager),
	  m_curview(NULL),
	  m_filelist(*auto_alloc(&manager.machine(), simple_list<layout_file>(manager.machine().m_respool))),
	  m_flags(flags),
	  m_listindex(0),
	  m_width(640),
	  m_height(480),
	  m_pixel_aspect(0.0f),
	  m_max_refresh(0),
	  m_orientation(0),
	  m_base_view(NULL),
	  m_base_orientation(ROT0),
	  m_maxtexwidth(65536),
	  m_maxtexheight(65536),
	  m_debug_containers(manager.machine().m_respool)
{
	// determine the base layer configuration based on options
	m_base_layerconfig.set_backdrops_enabled(options_get_bool(manager.machine().options(), OPTION_USE_BACKDROPS));
	m_base_layerconfig.set_overlays_enabled(options_get_bool(manager.machine().options(), OPTION_USE_OVERLAYS));
	m_base_layerconfig.set_bezels_enabled(options_get_bool(manager.machine().options(), OPTION_USE_BEZELS));
	m_base_layerconfig.set_zoom_to_screen(options_get_bool(manager.machine().options(), OPTION_ARTWORK_CROP));

	// determine the base orientation based on options
	m_orientation = ROT0;
	if (!options_get_bool(manager.machine().options(), OPTION_ROTATE))
		m_base_orientation = orientation_reverse(manager.machine().gamedrv->flags & ORIENTATION_MASK);

	// rotate left/right
	if (options_get_bool(manager.machine().options(), OPTION_ROR) || (options_get_bool(manager.machine().options(), OPTION_AUTOROR) && (manager.machine().gamedrv->flags & ORIENTATION_SWAP_XY)))
		m_base_orientation = orientation_add(ROT90, m_base_orientation);
	if (options_get_bool(manager.machine().options(), OPTION_ROL) || (options_get_bool(manager.machine().options(), OPTION_AUTOROL) && (manager.machine().gamedrv->flags & ORIENTATION_SWAP_XY)))
		m_base_orientation = orientation_add(ROT270, m_base_orientation);

	// flip X/Y
	if (options_get_bool(manager.machine().options(), OPTION_FLIPX))
		m_base_orientation ^= ORIENTATION_FLIP_X;
	if (options_get_bool(manager.machine().options(), OPTION_FLIPY))
		m_base_orientation ^= ORIENTATION_FLIP_Y;

	// set the orientation and layerconfig equal to the base
	m_orientation = m_base_orientation;
	m_layerconfig = m_base_layerconfig;

	// load the layout files
	load_layout_files(layoutfile, flags & RENDER_CREATE_SINGLE_FILE);

	// set the current view to the first one
	set_view(0);

	// make us the UI target if there is none
	if (!hidden() && manager.m_ui_target == NULL)
		manager.set_ui_target(*this);
}


//-------------------------------------------------
//  ~render_target - destructor
//-------------------------------------------------

render_target::~render_target()
{
	auto_free(&m_manager.machine(), &m_filelist);
}


//-------------------------------------------------
//  is_ui_target - return true if this is the
//  UI target
//-------------------------------------------------

bool render_target::is_ui_target() const
{
	return (this == &m_manager.ui_target());
}


//-------------------------------------------------
//  index - return the index of this target
//-------------------------------------------------

int render_target::index() const
{
	return m_manager.m_targetlist.indexof(*this);
}


//-------------------------------------------------
//  set_bounds - set the bounds and pixel aspect
//  of a target
//-------------------------------------------------

void render_target::set_bounds(INT32 width, INT32 height, float pixel_aspect)
{
	m_width = width;
	m_height = height;
	m_bounds.x0 = m_bounds.y0 = 0;
	m_bounds.x1 = (float)width;
	m_bounds.y1 = (float)height;
	m_pixel_aspect = pixel_aspect;
}


//-------------------------------------------------
//  set_view - dynamically change the view for
//  a target
//-------------------------------------------------

void render_target::set_view(int viewindex)
{
	layout_view *view = view_by_index(viewindex);
	if (view != NULL)
	{
		m_curview = view;
		view->recompute(m_layerconfig);
	}
}


//-------------------------------------------------
//  set_max_texture_size - set the upper bound on
//  the texture size
//-------------------------------------------------

void render_target::set_max_texture_size(int maxwidth, int maxheight)
{
	m_maxtexwidth = maxwidth;
	m_maxtexheight = maxheight;
}


//-------------------------------------------------
//  configured_view - select a view for this
//  target based on the configuration parameters
//-------------------------------------------------

int render_target::configured_view(const char *viewname, int targetindex, int numtargets)
{
	layout_view *view = NULL;
	int viewindex;

	// auto view just selects the nth view
	if (strcmp(viewname, "auto") != 0)
	{
		// scan for a matching view name
		for (view = view_by_index(viewindex = 0); view != NULL; view = view_by_index(++viewindex))
			if (mame_strnicmp(view->name(), viewname, strlen(viewname)) == 0)
				break;
	}

	// if we don't have a match, default to the nth view
	int scrcount = m_manager.machine().m_devicelist.count(SCREEN);
	if (view == NULL && scrcount > 0)
	{
		// if we have enough targets to be one per screen, assign in order
		if (numtargets >= scrcount)
		{
			int ourindex = index() % scrcount;
			screen_device *screen;
			for (screen = m_manager.machine().first_screen(); screen != NULL; screen = screen->next_screen())
				if (ourindex-- == 0)
					break;

			// find the first view with this screen and this screen only
			for (view = view_by_index(viewindex = 0); view != NULL; view = view_by_index(++viewindex))
			{
				const render_screen_list &viewscreens = view->screens();
				if (viewscreens.count() == 1 && viewscreens.contains(*screen))
					break;
				if (viewscreens.count() == 0)
				{
					view = NULL;
					break;
				}
			}
		}

		// otherwise, find the first view that has all the screens
		if (view == NULL)
		{
			for (view = view_by_index(viewindex = 0); view != NULL; view = view_by_index(++viewindex))
			{
				const render_screen_list &viewscreens = view->screens();
				if (viewscreens.count() == 0)
					break;
				if (viewscreens.count() >= scrcount)
				{
					screen_device *screen;
					for (screen = m_manager.machine().first_screen(); screen != NULL; screen = screen->next_screen())
						if (!viewscreens.contains(*screen))
							break;
					if (screen == NULL)
						break;
				}
			}
		}
	}

	// make sure it's a valid view
	return (view != NULL) ? view_index(*view) : 0;
}


//-------------------------------------------------
//  view_name - return the name of the given view
//-------------------------------------------------

const char *render_target::view_name(int viewindex)
{
	layout_view *view = view_by_index(viewindex);
	return (view != NULL) ? view->name() : NULL;
}


//-------------------------------------------------
//  render_target_get_view_screens - return a
//  bitmask of which screens are visible on a
//  given view
//-------------------------------------------------

const render_screen_list &render_target::view_screens(int viewindex)
{
	layout_view *view = view_by_index(viewindex);
	return (view != NULL) ? view->screens() : s_empty_screen_list;
}


//-------------------------------------------------
//  compute_visible_area - compute the visible
//  area for the given target with the current
//  layout and proposed new parameters
//-------------------------------------------------

void render_target::compute_visible_area(INT32 target_width, INT32 target_height, float target_pixel_aspect, int target_orientation, INT32 &visible_width, INT32 &visible_height)
{
	float width, height;
	float scale;

	// constrained case
	if (target_pixel_aspect != 0.0f)
	{
		// start with the aspect ratio of the square pixel layout
		width = m_curview->effective_aspect(m_layerconfig);
		height = 1.0f;

		// first apply target orientation
		if (target_orientation & ORIENTATION_SWAP_XY)
			FSWAP(width, height);

		// apply the target pixel aspect ratio
		height *= target_pixel_aspect;

		// based on the height/width ratio of the source and target, compute the scale factor
		if (width / height > (float)target_width / (float)target_height)
			scale = (float)target_width / width;
		else
			scale = (float)target_height / height;
	}

	// stretch-to-fit case
	else
	{
		width = (float)target_width;
		height = (float)target_height;
		scale = 1.0f;
	}

	// set the final width/height
	visible_width = render_round_nearest(width * scale);
	visible_height = render_round_nearest(height * scale);
}


//-------------------------------------------------
//  compute_minimum_size - compute the "minimum"
//  size of a target, which is the smallest bounds
//  that will ensure at least 1 target pixel per
//  source pixel for all included screens
//-------------------------------------------------

void render_target::compute_minimum_size(INT32 &minwidth, INT32 &minheight)
{
	float maxxscale = 1.0f, maxyscale = 1.0f;
	int screens_considered = 0;

	// early exit in case we are called between device teardown and render teardown
	if (m_manager.machine().m_devicelist.count() == 0)
	{
		minwidth = 640;
		minheight = 480;
		return;
	}

	// scan the current view for all screens
	for (item_layer layer = ITEM_LAYER_FIRST; layer < ITEM_LAYER_MAX; layer++)

		// iterate over items in the layer
		for (layout_view::item *curitem = m_curview->first_item(layer); curitem != NULL; curitem = curitem->next())
			if (curitem->screen() != NULL)
			{
				// use a hard-coded default visible area for vector screens
				screen_device *screen = curitem->screen();
				const rectangle vectorvis = { 0, 639, 0, 479 };
				const rectangle &visarea = (screen->screen_type() == SCREEN_TYPE_VECTOR) ? vectorvis : screen->visible_area();

				// apply target orientation to the bounds
				render_bounds bounds = curitem->bounds();
				apply_orientation(bounds, m_orientation);
				normalize_bounds(bounds);

				// based on the orientation of the screen container, check the bitmap
				float xscale, yscale;
				if (!(orientation_add(m_orientation, screen->container().orientation()) & ORIENTATION_SWAP_XY))
				{
					xscale = (float)(visarea.max_x + 1 - visarea.min_x) / (bounds.x1 - bounds.x0);
					yscale = (float)(visarea.max_y + 1 - visarea.min_y) / (bounds.y1 - bounds.y0);
				}
				else
				{
					xscale = (float)(visarea.max_y + 1 - visarea.min_y) / (bounds.x1 - bounds.x0);
					yscale = (float)(visarea.max_x + 1 - visarea.min_x) / (bounds.y1 - bounds.y0);
				}

				// pick the greater
				maxxscale = MAX(xscale, maxxscale);
				maxyscale = MAX(yscale, maxyscale);
				screens_considered++;
			}

	// if there were no screens considered, pick a nominal default
	if (screens_considered == 0)
	{
		maxxscale = 640.0f;
		maxyscale = 480.0f;
	}

	// round up
	minwidth = render_round_nearest(maxxscale);
	minheight = render_round_nearest(maxyscale);
}


//-------------------------------------------------
//  get_primitives - return a list of primitives
//  for a given render target
//-------------------------------------------------

render_primitive_list &render_target::get_primitives()
{
	// remember the base values if this is the first frame
	if (m_base_view == NULL)
		m_base_view = m_curview;

	// switch to the next primitive list
	render_primitive_list &list = m_primlist[m_listindex];
	m_listindex = (m_listindex + 1) % ARRAY_LENGTH(m_primlist);
	list.acquire_lock();

	// free any previous primitives
	list.release_all();

	// compute the visible width/height
	INT32 viswidth, visheight;
	compute_visible_area(m_width, m_height, m_pixel_aspect, m_orientation, viswidth, visheight);

	// create a root transform for the target
	object_transform root_xform;
	root_xform.xoffs = (float)(m_width - viswidth) / 2;
	root_xform.yoffs = (float)(m_height - visheight) / 2;
	root_xform.xscale = (float)viswidth;
	root_xform.yscale = (float)visheight;
	root_xform.color.r = root_xform.color.g = root_xform.color.b = root_xform.color.a = 1.0f;
	root_xform.orientation = m_orientation;
    root_xform.no_center = false;

	// iterate over layers back-to-front, but only if we're running
	if (m_manager.machine().phase() >= MACHINE_PHASE_RESET)
		for (item_layer layernum = ITEM_LAYER_FIRST; layernum < ITEM_LAYER_MAX; layernum++)
		{
			int blendmode;
			item_layer layer = get_layer_and_blendmode(*m_curview, layernum, blendmode);
			if (m_curview->layer_enabled(layer))
			{
				// iterate over items in the layer
				for (layout_view::item *curitem = m_curview->first_item(layer); curitem != NULL; curitem = curitem->next())
				{
					// first apply orientation to the bounds
					render_bounds bounds = curitem->bounds();
					apply_orientation(bounds, root_xform.orientation);
					normalize_bounds(bounds);

					// apply the transform to the item
					object_transform item_xform;
					item_xform.xoffs = root_xform.xoffs + bounds.x0 * root_xform.xscale;
					item_xform.yoffs = root_xform.yoffs + bounds.y0 * root_xform.yscale;
					item_xform.xscale = (bounds.x1 - bounds.x0) * root_xform.xscale;
					item_xform.yscale = (bounds.y1 - bounds.y0) * root_xform.yscale;
					item_xform.color.r = curitem->color().r * root_xform.color.r;
					item_xform.color.g = curitem->color().g * root_xform.color.g;
					item_xform.color.b = curitem->color().b * root_xform.color.b;
					item_xform.color.a = curitem->color().a * root_xform.color.a;
					item_xform.orientation = orientation_add(curitem->orientation(), root_xform.orientation);
                    item_xform.no_center = false;

					// if there is no associated element, it must be a screen element
					if (curitem->screen() != NULL)
						add_container_primitives(list, item_xform, curitem->screen()->container(), blendmode);
					else
						add_element_primitives(list, item_xform, *curitem->element(), curitem->state(), blendmode);
				}
			}
		}

	// if we are not in the running stage, draw an outer box
	else
	{
		render_primitive *prim = list.alloc(render_primitive::QUAD);
		set_render_bounds_xy(&prim->bounds, 0.0f, 0.0f, (float)m_width, (float)m_height);
		set_render_color(&prim->color, 1.0f, 1.0f, 1.0f, 1.0f);
		prim->texture.base = NULL;
		prim->flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);
		list.append(*prim);

		if (m_width > 1 && m_height > 1)
		{
			prim = list.alloc(render_primitive::QUAD);
			set_render_bounds_xy(&prim->bounds, 1.0f, 1.0f, (float)(m_width - 1), (float)(m_height - 1));
			set_render_color(&prim->color, 1.0f, 0.0f, 0.0f, 0.0f);
			prim->texture.base = NULL;
			prim->flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);
			list.append(*prim);
		}
	}

	// process the debug containers
	for (render_container *debug = m_debug_containers.first(); debug != NULL; debug = debug->next())
	{
		object_transform ui_xform;
		ui_xform.xoffs = 0;
		ui_xform.yoffs = 0;
		ui_xform.xscale = (float)m_width;
		ui_xform.yscale = (float)m_height;
		ui_xform.color.r = ui_xform.color.g = ui_xform.color.b = ui_xform.color.a = 1.0f;
		ui_xform.color.a = 0.9f;
		ui_xform.orientation = m_orientation;
		ui_xform.no_center = true;

		// add UI elements
		add_container_primitives(list, ui_xform, *debug, BLENDMODE_ALPHA);
	}

	// process the UI if we are the UI target
	if (is_ui_target())
	{
		// compute the transform for the UI
		object_transform ui_xform;
		ui_xform.xoffs = 0;
		ui_xform.yoffs = 0;
		ui_xform.xscale = (float) m_width;
		ui_xform.yscale = (float) m_height;
		ui_xform.color.r = ui_xform.color.g = ui_xform.color.b = ui_xform.color.a = 1.0f;
		ui_xform.orientation = m_orientation;
        ui_xform.no_center = false;

		// add UI elements
		add_container_primitives(list, ui_xform, m_manager.ui_container(), BLENDMODE_ALPHA);
	}

	// optimize the list before handing it off
	add_clear_and_optimize_primitive_list(list);
	list.release_lock();
	return list;
}


//-------------------------------------------------
//  map_point_container - attempts to map a point
//  on the specified render_target to the
//  specified container, if possible
//-------------------------------------------------

bool render_target::map_point_container(INT32 target_x, INT32 target_y, render_container &container, float &container_x, float &container_y)
{
	const char *input_tag;
	UINT32 input_mask;
	return map_point_internal(target_x, target_y, &container, container_x, container_y, input_tag, input_mask);
}


//-------------------------------------------------
//  map_point_input - attempts to map a point on
//  the specified render_target to the specified
//  container, if possible
//-------------------------------------------------

bool render_target::map_point_input(INT32 target_x, INT32 target_y, const char *&input_tag, UINT32 &input_mask, float &input_x, float &input_y)
{
	return map_point_internal(target_x, target_y, NULL, input_x, input_y, input_tag, input_mask);
}


//-------------------------------------------------
//  invalidate_all - if any of our primitive lists
//  contain a reference to the given pointer,
//  clear them
//-------------------------------------------------

void render_target::invalidate_all(void *refptr)
{
	// iterate through all our primitive lists
	for (int listnum = 0; listnum < ARRAY_LENGTH(m_primlist); listnum++)
	{
		render_primitive_list &list = m_primlist[listnum];

		// if we have a reference to this object, release our list
		list.acquire_lock();
		if (list.has_reference(refptr))
			list.release_all();
		list.release_lock();
	}
}


//-------------------------------------------------
//  debug_alloc - allocate a container for a debug
//  view
//-------------------------------------------------

render_container *render_target::debug_alloc()
{
	return &m_debug_containers.append(*m_manager.container_alloc());
}


//-------------------------------------------------
//  debug_free - free a container for a debug view
//-------------------------------------------------

void render_target::debug_free(render_container &container)
{
	m_debug_containers.remove(container);
}


//-------------------------------------------------
//  debug_top - move a debug view container to
//  the top of the list
//-------------------------------------------------

void render_target::debug_top(render_container &container)
{
	m_debug_containers.prepend(m_debug_containers.detach(container));
}


//-------------------------------------------------
//  update_layer_config - recompute after a layer
//  config change
//-------------------------------------------------

void render_target::update_layer_config()
{
	m_curview->recompute(m_layerconfig);
}


//-------------------------------------------------
//  load_layout_files - load layout files for a
//  given render target
//-------------------------------------------------

void render_target::load_layout_files(const char *layoutfile, bool singlefile)
{
	// if there's an explicit file, load that first
	const char *basename = m_manager.machine().basename();
	if (layoutfile != NULL)
		load_layout_file(basename, layoutfile);

	// if we're only loading this file, we know our final result
	if (singlefile)
		return;

	// try to load a file based on the driver name
	const game_driver *gamedrv = m_manager.machine().gamedrv;
	if (!load_layout_file(basename, gamedrv->name))
		load_layout_file(basename, "default");

	// if a default view has been specified, use that as a fallback
	if (gamedrv->default_layout != NULL)
		load_layout_file(NULL, gamedrv->default_layout);
	if (m_manager.machine().m_config.m_default_layout != NULL)
		load_layout_file(NULL, m_manager.machine().m_config.m_default_layout);

	// try to load another file based on the parent driver name
	const game_driver *cloneof = driver_get_clone(gamedrv);
	if (cloneof != NULL)
		if (!load_layout_file(cloneof->name, cloneof->name))
			load_layout_file(cloneof->name, "default");

	// now do the built-in layouts for single-screen games
	if (m_manager.machine().m_devicelist.count(SCREEN) == 1)
	{
		if (gamedrv->flags & ORIENTATION_SWAP_XY)
			load_layout_file(NULL, layout_vertical);
		else
			load_layout_file(NULL, layout_horizont);
		assert_always(m_filelist.count() > 0, "Couldn't parse default layout??");
	}
}


//-------------------------------------------------
//  load_layout_file - load a single layout file
//  and append it to our list
//-------------------------------------------------

bool render_target::load_layout_file(const char *dirname, const char *filename)
{
	// if the first character of the "file" is an open brace, assume it is an XML string
	xml_data_node *rootnode;
	if (filename[0] == '<')
		rootnode = xml_string_read(filename, NULL);

	// otherwise, assume it is a file
	else
	{
		// build the path and optionally prepend the directory
		astring fname(filename, ".lay");
		if (dirname != NULL)
			fname.ins(0, PATH_SEPARATOR).ins(0, dirname);

		// attempt to open the file; bail if we can't
		mame_file *layoutfile;
		file_error filerr = mame_fopen(SEARCHPATH_ARTWORK, fname, OPEN_FLAG_READ, &layoutfile);
		if (filerr != FILERR_NONE)
			return false;

		// read the file
		rootnode = xml_file_read(mame_core_file(layoutfile), NULL);
		mame_fclose(layoutfile);
	}

	// if we didn't get a properly-formatted XML file, record a warning and exit
	if (rootnode == NULL)
	{
		if (filename[0] != '<')
			mame_printf_warning("Improperly formatted XML file '%s', ignorning\n", filename);
		else
			mame_printf_warning("Improperly formatted XML string, ignorning");
		return false;
	}

	// parse and catch any errors
	bool result = true;
	try
	{
		m_filelist.append(*auto_alloc(&m_manager.machine(), layout_file(m_manager.machine(), *rootnode, dirname)));
	}
	catch (emu_fatalerror &err)
	{
		if (filename[0] != '<')
			mame_printf_warning("Error in XML file '%s': %s\n", filename, err.string());
		else
			mame_printf_warning("Error in XML string: %s", err.string());
		result = false;
	}

	// free the root node
	xml_file_free(rootnode);
	return result;
}


//-------------------------------------------------
//  add_container_primitives - add primitives
//  based on the container
//-------------------------------------------------

void render_target::add_container_primitives(render_primitive_list &list, const object_transform &xform, render_container &container, int blendmode)
{
	// first update the palette for the container, if it is dirty
	container.update_palette();

	// compute the clip rect
	render_bounds cliprect;
	cliprect.x0 = xform.xoffs;
	cliprect.y0 = xform.yoffs;
	cliprect.x1 = xform.xoffs + xform.xscale;
	cliprect.y1 = xform.yoffs + xform.yscale;
	sect_render_bounds(&cliprect, &m_bounds);

	// compute the container transform
	object_transform container_xform;
	container_xform.orientation = orientation_add(container.orientation(), xform.orientation);
	{
		float xscale = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.yscale() : container.xscale();
		float yscale = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.xscale() : container.yscale();
		float xoffs = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.yoffset() : container.xoffset();
		float yoffs = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.xoffset() : container.yoffset();
		if (container_xform.orientation & ORIENTATION_FLIP_X) xoffs = -xoffs;
		if (container_xform.orientation & ORIENTATION_FLIP_Y) yoffs = -yoffs;
		container_xform.xscale = xform.xscale * xscale;
		container_xform.yscale = xform.yscale * yscale;
		if (xform.no_center)
		{
			container_xform.xoffs = xform.xscale * (xoffs) + xform.xoffs;
			container_xform.yoffs = xform.yscale * (yoffs) + xform.yoffs;
		}
		else
		{
			container_xform.xoffs = xform.xscale * (0.5f - 0.5f * xscale + xoffs) + xform.xoffs;
			container_xform.yoffs = xform.yscale * (0.5f - 0.5f * yscale + yoffs) + xform.yoffs;
		}
		container_xform.color = xform.color;
	}

	// iterate over elements
	for (render_container::item *curitem = container.first_item(); curitem != NULL; curitem = curitem->next())
	{
		// compute the oriented bounds
		render_bounds bounds = curitem->bounds();
		apply_orientation(bounds, container_xform.orientation);

		// allocate the primitive and set the transformed bounds/color data
		render_primitive *prim = list.alloc(render_primitive::INVALID);
		prim->bounds.x0 = render_round_nearest(container_xform.xoffs + bounds.x0 * container_xform.xscale);
		prim->bounds.y0 = render_round_nearest(container_xform.yoffs + bounds.y0 * container_xform.yscale);
		if (curitem->internal() & INTERNAL_FLAG_CHAR)
		{
			prim->bounds.x1 = prim->bounds.x0 + render_round_nearest((bounds.x1 - bounds.x0) * container_xform.xscale);
			prim->bounds.y1 = prim->bounds.y0 + render_round_nearest((bounds.y1 - bounds.y0) * container_xform.yscale);
		}
		else
		{
			prim->bounds.x1 = render_round_nearest(container_xform.xoffs + bounds.x1 * container_xform.xscale);
			prim->bounds.y1 = render_round_nearest(container_xform.yoffs + bounds.y1 * container_xform.yscale);
		}

		// compute the color of the primitive
		prim->color.r = container_xform.color.r * curitem->color().r;
		prim->color.g = container_xform.color.g * curitem->color().g;
		prim->color.b = container_xform.color.b * curitem->color().b;
		prim->color.a = container_xform.color.a * curitem->color().a;

		// now switch off the type
		bool clipped = true;
		switch (curitem->type())
		{
			case CONTAINER_ITEM_LINE:
				// adjust the color for brightness/contrast/gamma
				prim->color.a = container.apply_brightness_contrast_gamma_fp(prim->color.a);
				prim->color.r = container.apply_brightness_contrast_gamma_fp(prim->color.r);
				prim->color.g = container.apply_brightness_contrast_gamma_fp(prim->color.g);
				prim->color.b = container.apply_brightness_contrast_gamma_fp(prim->color.b);

				// set the line type
				prim->type = render_primitive::LINE;

				// scale the width by the minimum of X/Y scale factors
				prim->width = curitem->width() * MIN(container_xform.xscale, container_xform.yscale);
				prim->flags = curitem->flags();

				// clip the primitive
				clipped = render_clip_line(&prim->bounds, &cliprect);
				break;

			case CONTAINER_ITEM_QUAD:
				// set the quad type
				prim->type = render_primitive::QUAD;

				// normalize the bounds
				normalize_bounds(prim->bounds);

				// get the scaled bitmap and set the resulting palette
				if (curitem->texture() != NULL)
				{
					// determine the final orientation
					int finalorient = orientation_add(PRIMFLAG_GET_TEXORIENT(curitem->flags()), container_xform.orientation);

					// based on the swap values, get the scaled final texture
					int width = (finalorient & ORIENTATION_SWAP_XY) ? (prim->bounds.y1 - prim->bounds.y0) : (prim->bounds.x1 - prim->bounds.x0);
					int height = (finalorient & ORIENTATION_SWAP_XY) ? (prim->bounds.x1 - prim->bounds.x0) : (prim->bounds.y1 - prim->bounds.y0);
					width = MIN(width, m_maxtexwidth);
					height = MIN(height, m_maxtexheight);
					if (curitem->texture()->get_scaled(width, height, prim->texture, list))
					{
						// set the palette
						prim->texture.palette = curitem->texture()->get_adjusted_palette(container);

						// determine UV coordinates and apply clipping
						prim->texcoords = oriented_texcoords[finalorient];
						clipped = render_clip_quad(&prim->bounds, &cliprect, &prim->texcoords);

						// apply the final orientation from the quad flags and then build up the final flags
						prim->flags = (curitem->flags() & ~(PRIMFLAG_TEXORIENT_MASK | PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) |
										PRIMFLAG_TEXORIENT(finalorient) |
										PRIMFLAG_TEXFORMAT(curitem->texture()->format());
						if (blendmode != -1)
							prim->flags |= PRIMFLAG_BLENDMODE(blendmode);
						else
							prim->flags |= PRIMFLAG_BLENDMODE(PRIMFLAG_GET_BLENDMODE(curitem->flags()));
					}
				}
				else
				{
					// adjust the color for brightness/contrast/gamma
					prim->color.r = container.apply_brightness_contrast_gamma_fp(prim->color.r);
					prim->color.g = container.apply_brightness_contrast_gamma_fp(prim->color.g);
					prim->color.b = container.apply_brightness_contrast_gamma_fp(prim->color.b);

					// no texture -- set the basic flags
					prim->texture.base = NULL;
					prim->flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);

					// apply clipping
					clipped = render_clip_quad(&prim->bounds, &cliprect, NULL);
				}
				break;
		}

		// add to the list or free if we're clipped out
		list.append_or_return(*prim, clipped);
	}

	// add the overlay if it exists
	if (container.overlay() != NULL && m_layerconfig.screen_overlay_enabled())
	{
		INT32 width, height;

		// allocate a primitive
		render_primitive *prim = list.alloc(render_primitive::QUAD);
		set_render_bounds_wh(&prim->bounds, xform.xoffs, xform.yoffs, xform.xscale, xform.yscale);
		prim->color = container_xform.color;
		width = render_round_nearest(prim->bounds.x1) - render_round_nearest(prim->bounds.x0);
		height = render_round_nearest(prim->bounds.y1) - render_round_nearest(prim->bounds.y0);

		bool got_scaled = container.overlay()->get_scaled(
				(container_xform.orientation & ORIENTATION_SWAP_XY) ? height : width,
				(container_xform.orientation & ORIENTATION_SWAP_XY) ? width : height, prim->texture, list);
		if (got_scaled)
		{
			// determine UV coordinates
			prim->texcoords = oriented_texcoords[container_xform.orientation];

			// set the flags and add it to the list
			prim->flags = PRIMFLAG_TEXORIENT(container_xform.orientation) |
							PRIMFLAG_BLENDMODE(BLENDMODE_RGB_MULTIPLY) |
							PRIMFLAG_TEXFORMAT(container.overlay()->format());
		}
		list.append_or_return(*prim, !got_scaled);
	}
}


//-------------------------------------------------
//  add_element_primitives - add the primitive
//  for an element in the current state
//-------------------------------------------------

void render_target::add_element_primitives(render_primitive_list &list, const object_transform &xform, layout_element &element, int state, int blendmode)
{
	// if we're out of range, bail
	if (state > element.maxstate())
		return;
	if (state < 0)
		state = 0;

	// get a pointer to the relevant texture
	render_texture *texture = element.state_texture(state);
	if (texture != NULL)
	{
		render_primitive *prim = list.alloc(render_primitive::QUAD);

		// configure the basics
		prim->color = xform.color;
		prim->flags = PRIMFLAG_TEXORIENT(xform.orientation) | PRIMFLAG_BLENDMODE(blendmode) | PRIMFLAG_TEXFORMAT(texture->format());

		// compute the bounds
		INT32 width = render_round_nearest(xform.xscale);
		INT32 height = render_round_nearest(xform.yscale);
		set_render_bounds_wh(&prim->bounds, render_round_nearest(xform.xoffs), render_round_nearest(xform.yoffs), (float) width, (float) height);
		if (xform.orientation & ORIENTATION_SWAP_XY)
			ISWAP(width, height);
		width = MIN(width, m_maxtexwidth);
		height = MIN(height, m_maxtexheight);

		// get the scaled texture and append it
		bool clipped = true;
		if (texture->get_scaled(width, height, prim->texture, list))
		{
			// compute the clip rect
			render_bounds cliprect;
			cliprect.x0 = render_round_nearest(xform.xoffs);
			cliprect.y0 = render_round_nearest(xform.yoffs);
			cliprect.x1 = render_round_nearest(xform.xoffs + xform.xscale);
			cliprect.y1 = render_round_nearest(xform.yoffs + xform.yscale);
			sect_render_bounds(&cliprect, &m_bounds);

			// determine UV coordinates and apply clipping
			prim->texcoords = oriented_texcoords[xform.orientation];
			clipped = render_clip_quad(&prim->bounds, &cliprect, &prim->texcoords);
		}

		// add to the list or free if we're clipped out
		list.append_or_return(*prim, clipped);
	}
}


//-------------------------------------------------
//  map_point_internal - internal logic for
//  mapping points
//-------------------------------------------------

bool render_target::map_point_internal(INT32 target_x, INT32 target_y, render_container *container, float &mapped_x, float &mapped_y, const char *&mapped_input_tag, UINT32 &mapped_input_mask)
{
	// default to point not mapped
	mapped_x = -1.0;
	mapped_y = -1.0;
	mapped_input_tag = NULL;
	mapped_input_mask = 0;

	// convert target coordinates to float
	float target_fx = (float)target_x / m_width;
	float target_fy = (float)target_y / m_height;

	// explicitly check for the UI container
	if (container != NULL && container == &m_manager.ui_container())
	{
		// this hit test went against the UI container
		if (target_fx >= 0.0 && target_fx < 1.0 && target_fy >= 0.0 && target_fy < 1.0)
		{
			// this point was successfully mapped
			mapped_x = target_fx;
			mapped_y = target_fy;
			return true;
		}
		return false;
	}

	// loop through each layer
	for (item_layer layernum = ITEM_LAYER_FIRST; layernum < ITEM_LAYER_MAX; layernum++)
	{
		int blendmode;
		item_layer layer = get_layer_and_blendmode(*m_curview, layernum, blendmode);
		if (m_curview->layer_enabled(layer))
		{
			// iterate over items in the layer
			for (layout_view::item *item = m_curview->first_item(layer); item != NULL; item = item->next())
			{
				bool checkit;

				// if we're looking for a particular container, verify that we have the right one
				if (container != NULL)
					checkit = (item->screen() != NULL && &item->screen()->container() == container);

				// otherwise, assume we're looking for an input
				else
					checkit = item->has_input();

				// this target is worth looking at; now check the point
				if (checkit && target_fx >= item->bounds().x0 && target_fx < item->bounds().x1 && target_fy >= item->bounds().y0 && target_fy < item->bounds().y1)
				{
					// point successfully mapped
					mapped_x = (target_fx - item->bounds().x0) / (item->bounds().x1 - item->bounds().x0);
					mapped_y = (target_fy - item->bounds().y0) / (item->bounds().y1 - item->bounds().y0);
					mapped_input_tag = item->input_tag_and_mask(mapped_input_mask);
					return true;
				}
			}
		}
	}
	return false;
}


//-------------------------------------------------
//  view_name - return the name of the indexed
//  view, or NULL if it doesn't exist
//-------------------------------------------------

layout_view *render_target::view_by_index(int index) const
{
	// scan the list of views within each layout, skipping those that don't apply
	for (layout_file *file = m_filelist.first(); file != NULL; file = file->next())
		for (layout_view *view = file->first_view(); view != NULL; view = view->next())
			if (!(m_flags & RENDER_CREATE_NO_ART) || !view->has_art())
				if (index-- == 0)
					return view;
	return NULL;
}


//-------------------------------------------------
//  view_index - return the index of the given
//  view
//-------------------------------------------------

int render_target::view_index(layout_view &targetview) const
{
	// find the first named match
	int index = 0;

	// scan the list of views within each layout, skipping those that don't apply
	for (layout_file *file = m_filelist.first(); file != NULL; file = file->next())
		for (layout_view *view = file->first_view(); view != NULL; view = view->next())
			if (!(m_flags & RENDER_CREATE_NO_ART) || !view->has_art())
			{
				if (&targetview == view)
					return index;
				index++;
			}
	return 0;
}


//-------------------------------------------------
//  config_load - process config information
//-------------------------------------------------

void render_target::config_load(xml_data_node &targetnode)
{
	// find the view
	const char *viewname = xml_get_attribute_string(&targetnode, "view", NULL);
	if (viewname != NULL)
		for (int viewnum = 0; viewnum < 1000; viewnum++)
		{
			const char *testname = view_name(viewnum);
			if (testname == NULL)
				break;
			if (!strcmp(viewname, testname))
			{
				set_view(viewnum);
				break;
			}
		}

	// modify the artwork config
	int tmpint = xml_get_attribute_int(&targetnode, "backdrops", -1);
	if (tmpint == 0 || tmpint == 1)
		set_backdrops_enabled(tmpint);

	tmpint = xml_get_attribute_int(&targetnode, "overlays", -1);
	if (tmpint == 0 || tmpint == 1)
		set_overlays_enabled(tmpint);

	tmpint = xml_get_attribute_int(&targetnode, "bezels", -1);
	if (tmpint == 0 || tmpint == 1)
		set_bezels_enabled(tmpint);

	tmpint = xml_get_attribute_int(&targetnode, "zoom", -1);
	if (tmpint == 0 || tmpint == 1)
		set_zoom_to_screen(tmpint);

	// apply orientation
	tmpint = xml_get_attribute_int(&targetnode, "rotate", -1);
	if (tmpint != -1)
	{
		if (tmpint == 90)
			tmpint = ROT90;
		else if (tmpint == 180)
			tmpint = ROT180;
		else if (tmpint == 270)
			tmpint = ROT270;
		else
			tmpint = ROT0;
		set_orientation(orientation_add(tmpint, orientation()));

		// apply the opposite orientation to the UI
		if (is_ui_target())
		{
			render_container::user_settings settings;
			render_container &ui_container = m_manager.ui_container();

			ui_container.get_user_settings(settings);
			settings.m_orientation = orientation_add(orientation_reverse(tmpint), settings.m_orientation);
			ui_container.set_user_settings(settings);
		}
	}
}


//-------------------------------------------------
//  config_save - save our configuration, or
//  return false if we are the same as the default
//-------------------------------------------------

bool render_target::config_save(xml_data_node &targetnode)
{
	bool changed = false;

	// output the basics
	xml_set_attribute_int(&targetnode, "index", index());

	// output the view
	if (m_curview != m_base_view)
	{
		xml_set_attribute(&targetnode, "view", m_curview->name());
		changed = true;
	}

	// output the layer config
	if (m_layerconfig != m_base_layerconfig)
	{
		xml_set_attribute_int(&targetnode, "backdrops", m_layerconfig.backdrops_enabled());
		xml_set_attribute_int(&targetnode, "overlays", m_layerconfig.overlays_enabled());
		xml_set_attribute_int(&targetnode, "bezels", m_layerconfig.bezels_enabled());
		xml_set_attribute_int(&targetnode, "zoom", m_layerconfig.zoom_to_screen());
		changed = true;
	}

	// output rotation
	if (m_orientation != m_base_orientation)
	{
		int rotate = 0;
		if (orientation_add(ROT90, m_base_orientation) == m_orientation)
			rotate = 90;
		else if (orientation_add(ROT180, m_base_orientation) == m_orientation)
			rotate = 180;
		else if (orientation_add(ROT270, m_base_orientation) == m_orientation)
			rotate = 270;
		assert(rotate != 0);
		xml_set_attribute_int(&targetnode, "rotate", rotate);
		changed = true;
	}

	return changed;
}


//-------------------------------------------------
//  init_clear_extents - reset the extents list
//-------------------------------------------------

void render_target::init_clear_extents()
{
	m_clear_extents[0] = -m_height;
	m_clear_extents[1] = 1;
	m_clear_extents[2] = m_width;
	m_clear_extent_count = 3;
}


//-------------------------------------------------
//  remove_clear_extent - remove a quad from the
//  list of stuff to clear, unless it overlaps
//  a previous quad
//-------------------------------------------------

bool render_target::remove_clear_extent(const render_bounds &bounds)
{
	INT32 *max = &m_clear_extents[MAX_CLEAR_EXTENTS];
	INT32 *last = &m_clear_extents[m_clear_extent_count];
	INT32 *ext = &m_clear_extents[0];
	INT32 boundsx0 = ceil(bounds.x0);
	INT32 boundsx1 = floor(bounds.x1);
	INT32 boundsy0 = ceil(bounds.y0);
	INT32 boundsy1 = floor(bounds.y1);
	INT32 y0, y1 = 0;

	// loop over Y extents
	while (ext < last)
	{
		INT32 *linelast;

		// first entry of each line should always be negative
		assert(ext[0] < 0.0f);
		y0 = y1;
		y1 = y0 - ext[0];

		// do we intersect this extent?
		if (boundsy0 < y1 && boundsy1 > y0)
		{
			INT32 *xext;
			INT32 x0, x1 = 0;

			// split the top
			if (y0 < boundsy0)
			{
				int diff = boundsy0 - y0;

				// make a copy of this extent
				memmove(&ext[ext[1] + 2], &ext[0], (last - ext) * sizeof(*ext));
				last += ext[1] + 2;
				assert_always(last < max, "Ran out of clear extents!\n");

				// split the extent between pieces
				ext[ext[1] + 2] = -(-ext[0] - diff);
				ext[0] = -diff;

				// advance to the new extent
				y0 -= ext[0];
				ext += ext[1] + 2;
				y1 = y0 - ext[0];
			}

			// split the bottom
			if (y1 > boundsy1)
			{
				int diff = y1 - boundsy1;

				// make a copy of this extent
				memmove(&ext[ext[1] + 2], &ext[0], (last - ext) * sizeof(*ext));
				last += ext[1] + 2;
				assert_always(last < max, "Ran out of clear extents!\n");

				// split the extent between pieces
				ext[ext[1] + 2] = -diff;
				ext[0] = -(-ext[0] - diff);

				// recompute y1
				y1 = y0 - ext[0];
			}

			// now remove the X extent
			linelast = &ext[ext[1] + 2];
			xext = &ext[2];
			while (xext < linelast)
			{
				x0 = x1;
				x1 = x0 + xext[0];

				// do we fully intersect this extent?
				if (boundsx0 >= x0 && boundsx1 <= x1)
				{
					// yes; split it
					memmove(&xext[2], &xext[0], (last - xext) * sizeof(*xext));
					last += 2;
					linelast += 2;
					assert_always(last < max, "Ran out of clear extents!\n");

					// split this extent into three parts
					xext[0] = boundsx0 - x0;
					xext[1] = boundsx1 - boundsx0;
					xext[2] = x1 - boundsx1;

					// recompute x1
					x1 = boundsx1;
					xext += 2;
				}

				// do we partially intersect this extent?
				else if (boundsx0 < x1 && boundsx1 > x0)
					goto abort;

				// advance
				xext++;

				// do we partially intersect the next extent (which is a non-clear extent)?
				if (xext < linelast)
				{
					x0 = x1;
					x1 = x0 + xext[0];
					if (boundsx0 < x1 && boundsx1 > x0)
						goto abort;
					xext++;
				}
			}

			// update the count
			ext[1] = linelast - &ext[2];
		}

		// advance to the next row
		ext += 2 + ext[1];
	}

	// update the total count
	m_clear_extent_count = last - &m_clear_extents[0];
	return true;

abort:
	// update the total count even on a failure as we may have split extents
	m_clear_extent_count = last - &m_clear_extents[0];
	return false;
}


//-------------------------------------------------
//  add_clear_extents - add the accumulated
//  extents as a series of quads to clear
//-------------------------------------------------

void render_target::add_clear_extents(render_primitive_list &list)
{
	simple_list<render_primitive> clearlist;
	INT32 *last = &m_clear_extents[m_clear_extent_count];
	INT32 *ext = &m_clear_extents[0];
	INT32 y0, y1 = 0;

	// loop over all extents
	while (ext < last)
	{
		INT32 *linelast = &ext[ext[1] + 2];
		INT32 *xext = &ext[2];
		INT32 x0, x1 = 0;

		// first entry should always be negative
		assert(ext[0] < 0);
		y0 = y1;
		y1 = y0 - ext[0];

		// now remove the X extent
		while (xext < linelast)
		{
			x0 = x1;
			x1 = x0 + *xext++;

			// only add entries for non-zero widths
			if (x1 - x0 > 0)
			{
				render_primitive *prim = list.alloc(render_primitive::QUAD);
				set_render_bounds_xy(&prim->bounds, (float)x0, (float)y0, (float)x1, (float)y1);
				set_render_color(&prim->color, 1.0f, 0.0f, 0.0f, 0.0f);
				prim->texture.base = NULL;
				prim->flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);
				clearlist.append(*prim);
			}

			// skip the non-clearing extent
			x0 = x1;
			x1 = x0 + *xext++;
		}

		// advance to the next part
		ext += 2 + ext[1];
	}

	// we know that the first primitive in the list will be the global clip
	// so we insert the clears immediately after
	list.m_primlist.prepend_list(clearlist);
}


//-------------------------------------------------
//  add_clear_and_optimize_primitive_list -
//  optimize the primitive list
//-------------------------------------------------

void render_target::add_clear_and_optimize_primitive_list(render_primitive_list &list)
{
	// start with the assumption that we need to clear the whole screen
	init_clear_extents();

	// scan the list until we hit an intersection quad or a line
	for (render_primitive *prim = list.first(); prim != NULL; prim = prim->next())
	{
		// switch off the type
		switch (prim->type)
		{
			case render_primitive::LINE:
				goto done;

			case render_primitive::QUAD:
			{
				// stop when we hit an alpha texture
				if (PRIMFLAG_GET_TEXFORMAT(prim->flags) == TEXFORMAT_ARGB32 || PRIMFLAG_GET_TEXFORMAT(prim->flags) == TEXFORMAT_PALETTEA16)
					goto done;

				// if this quad can't be cleanly removed from the extents list, we're done
				if (!remove_clear_extent(prim->bounds))
					goto done;

				// change the blendmode on the first primitive to be NONE
				if (PRIMFLAG_GET_BLENDMODE(prim->flags) == BLENDMODE_RGB_MULTIPLY)
				{
					// RGB multiply will multiply against 0, leaving nothing
					set_render_color(&prim->color, 1.0f, 0.0f, 0.0f, 0.0f);
					prim->texture.base = NULL;
					prim->flags = (prim->flags & ~PRIMFLAG_BLENDMODE_MASK) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE);
				}
				else
				{
					// for alpha or add modes, we will blend against 0 or add to 0; treat it like none
					prim->flags = (prim->flags & ~PRIMFLAG_BLENDMODE_MASK) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE);
				}

				// since alpha is disabled, premultiply the RGB values and reset the alpha to 1.0
				prim->color.r *= prim->color.a;
				prim->color.g *= prim->color.a;
				prim->color.b *= prim->color.a;
				prim->color.a = 1.0f;
				break;
			}

			default:
				throw emu_fatalerror("Unexpected primitive type");
		}
	}

done:
	// now add the extents to the clear list
	add_clear_extents(list);
}



//**************************************************************************
//  CORE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  render_manager - constructor
//-------------------------------------------------

render_manager::render_manager(running_machine &machine)
	: m_machine(machine),
	  m_targetlist(machine.m_respool),
	  m_ui_target(NULL),
	  m_live_textures(0),
	  m_texture_allocator(machine.m_respool),
	  m_ui_container(auto_alloc(&machine, render_container(*this))),
	  m_screen_container_list(machine.m_respool)
{
	// register callbacks
	config_register(&machine, "video", config_load_static, config_save_static);

	// create one container per screen
	for (screen_device *screen = machine.first_screen(); screen != NULL; screen = screen->next_screen())
		screen->set_container(*container_alloc(screen));
}


//-------------------------------------------------
//  ~render_manager - destructor
//-------------------------------------------------

render_manager::~render_manager()
{
	// free all the containers since they may own textures
	container_free(m_ui_container);
	m_screen_container_list.reset();

	// better not be any outstanding textures when we die
	assert(m_live_textures == 0);
}


//-------------------------------------------------
//  is_live - return if the screen is 'live'
//-------------------------------------------------

bool render_manager::is_live(screen_device &screen) const
{
	// iterate over all live targets and or together their screen masks
	for (render_target *target = m_targetlist.first(); target != NULL; target = target->next())
		if (target->view_screens(target->view()).contains(screen))
			return true;
	return false;
}


//-------------------------------------------------
//  max_update_rate - return the smallest maximum
//  update rate across all targets
//-------------------------------------------------

float render_manager::max_update_rate() const
{
	// iterate over all live targets and or together their screen masks
	float minimum = 0;
	for (render_target *target = m_targetlist.first(); target != NULL; target = target->next())
		if (target->max_update_rate() != 0)
		{
			if (minimum == 0)
				minimum = target->max_update_rate();
			else
				minimum = MIN(target->max_update_rate(), minimum);
		}

	return minimum;
}


//-------------------------------------------------
//  target_alloc - allocate a new target
//-------------------------------------------------

render_target *render_manager::target_alloc(const char *layoutfile, UINT32 flags)
{
	return &m_targetlist.append(*auto_alloc(&m_machine, render_target(*this, layoutfile, flags)));
}


//-------------------------------------------------
//  target_free - free a target
//-------------------------------------------------

void render_manager::target_free(render_target *target)
{
	if (target != NULL)
		m_targetlist.remove(*target);
}


//-------------------------------------------------
//  target_by_index - get a render_target by index
//-------------------------------------------------

render_target *render_manager::target_by_index(int index) const
{
	// count up the targets until we hit the requested index
	for (render_target *target = m_targetlist.first(); target != NULL; target = target->next())
		if (!target->hidden())
			if (index-- == 0)
				return target;
	return NULL;
}


//-------------------------------------------------
//  ui_aspect - return the aspect ratio for UI
//  fonts
//-------------------------------------------------

float render_manager::ui_aspect()
{
	int orient = orientation_add(m_ui_target->orientation(), m_ui_container->orientation());

	// based on the orientation of the target, compute height/width or width/height
	float aspect;
	if (!(orient & ORIENTATION_SWAP_XY))
		aspect = (float)m_ui_target->height() / (float)m_ui_target->width();
	else
		aspect = (float)m_ui_target->width() / (float)m_ui_target->height();

	// if we have a valid pixel aspect, apply that and return
	if (m_ui_target->pixel_aspect() != 0.0f)
		return aspect / m_ui_target->pixel_aspect();

	// if not, clamp for extreme proportions
	if (aspect < 0.66f)
		aspect = 0.66f;
	if (aspect > 1.5f)
		aspect = 1.5f;
	return aspect;
}


//-------------------------------------------------
//  texture_alloc - allocate a new texture
//-------------------------------------------------

render_texture *render_manager::texture_alloc(texture_scaler_func scaler, void *param)
{
	// allocate a new texture and reset it
	render_texture *tex = m_texture_allocator.alloc();
	tex->reset(*this, scaler, param);
	m_live_textures++;
	return tex;
}


//-------------------------------------------------
//  texture_free - release a texture
//-------------------------------------------------

void render_manager::texture_free(render_texture *texture)
{
	if (texture != NULL)
	{
		m_live_textures--;
		texture->release();
	}
	m_texture_allocator.reclaim(texture);
}


//-------------------------------------------------
//  font_alloc - allocate a new font instance
//-------------------------------------------------

render_font *render_manager::font_alloc(const char *filename)
{
	return auto_alloc(&m_machine, render_font(*this, filename));
}


//-------------------------------------------------
//  font_free - release a font instance
//-------------------------------------------------

void render_manager::font_free(render_font *font)
{
	auto_free(&m_machine, font);
}


//-------------------------------------------------
//  invalidate_all - remove all refs to a
//  particular reference pointer
//-------------------------------------------------

void render_manager::invalidate_all(void *refptr)
{
	// permit NULL
	if (refptr == NULL)
		return;

	// loop over targets
	for (render_target *target = m_targetlist.first(); target != NULL; target = target->next())
		target->invalidate_all(refptr);
}


//-------------------------------------------------
//  container_alloc - allocate a new container
//-------------------------------------------------

render_container *render_manager::container_alloc(screen_device *screen)
{
	render_container *container = auto_alloc(&m_machine, render_container(*this, screen));
	if (screen != NULL)
		m_screen_container_list.append(*container);
	return container;
}


//-------------------------------------------------
//  container_free - release a container
//-------------------------------------------------

void render_manager::container_free(render_container *container)
{
	m_screen_container_list.detach(*container);
	auto_free(&m_machine, container);
}


//-------------------------------------------------
//  config_load - read and apply data from the
//  configuration file
//-------------------------------------------------

void render_manager::config_load_static(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	machine->render().config_load(config_type, parentnode);
}

void render_manager::config_load(int config_type, xml_data_node *parentnode)
{
	// we only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// might not have any data
	if (parentnode == NULL)
		return;

	// check the UI target
	xml_data_node *uinode = xml_get_sibling(parentnode->child, "interface");
	if (uinode != NULL)
	{
		render_target *target = target_by_index(xml_get_attribute_int(uinode, "target", 0));
		if (target != NULL)
			set_ui_target(*target);
	}

	// iterate over target nodes
	for (xml_data_node *targetnode = xml_get_sibling(parentnode->child, "target"); targetnode; targetnode = xml_get_sibling(targetnode->next, "target"))
	{
		render_target *target = target_by_index(xml_get_attribute_int(targetnode, "index", -1));
		if (target != NULL)
			target->config_load(*targetnode);
	}

	// iterate over screen nodes
	for (xml_data_node *screennode = xml_get_sibling(parentnode->child, "screen"); screennode; screennode = xml_get_sibling(screennode->next, "screen"))
	{
		int index = xml_get_attribute_int(screennode, "index", -1);
		render_container *container = m_screen_container_list.find(index);
		render_container::user_settings settings;

		// fetch current settings
		container->get_user_settings(settings);

		// fetch color controls
		settings.m_brightness = xml_get_attribute_float(screennode, "brightness", settings.m_brightness);
		settings.m_contrast = xml_get_attribute_float(screennode, "contrast", settings.m_contrast);
		settings.m_gamma = xml_get_attribute_float(screennode, "gamma", settings.m_gamma);

		// fetch positioning controls
		settings.m_xoffset = xml_get_attribute_float(screennode, "hoffset", settings.m_xoffset);
		settings.m_xscale = xml_get_attribute_float(screennode, "hstretch", settings.m_xscale);
		settings.m_yoffset = xml_get_attribute_float(screennode, "voffset", settings.m_yoffset);
		settings.m_yscale = xml_get_attribute_float(screennode, "vstretch", settings.m_yscale);

		// set the new values
		container->set_user_settings(settings);
	}
}


//-------------------------------------------------
//  config_save - save data to the configuration
//  file
//-------------------------------------------------

void render_manager::config_save_static(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	machine->render().config_save(config_type, parentnode);
}

void render_manager::config_save(int config_type, xml_data_node *parentnode)
{
	// we only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// write out the interface target
	if (m_ui_target->index() != 0)
	{
		// create a node for it
		xml_data_node *uinode = xml_add_child(parentnode, "interface", NULL);
		if (uinode != NULL)
			xml_set_attribute_int(uinode, "target", m_ui_target->index());
	}

	// iterate over targets
	for (int targetnum = 0; targetnum < 1000; targetnum++)
	{
		// get this target and break when we fail
		render_target *target = target_by_index(targetnum);
		if (target == NULL)
			break;

		// create a node
		xml_data_node *targetnode = xml_add_child(parentnode, "target", NULL);
		if (targetnode != NULL && !target->config_save(*targetnode))
			xml_delete_node(targetnode);
	}

	// iterate over screen containers
	int scrnum = 0;
	for (render_container *container = m_screen_container_list.first(); container != NULL; container = container->next(), scrnum++)
	{
		// create a node
		xml_data_node *screennode = xml_add_child(parentnode, "screen", NULL);
		if (screennode != NULL)
		{
			bool changed = false;

			// output the basics
			xml_set_attribute_int(screennode, "index", scrnum);

			render_container::user_settings settings;
			container->get_user_settings(settings);

			// output the color controls
			if (settings.m_brightness != options_get_float(m_machine.options(), OPTION_BRIGHTNESS))
			{
				xml_set_attribute_float(screennode, "brightness", settings.m_brightness);
				changed = true;
			}

			if (settings.m_contrast != options_get_float(m_machine.options(), OPTION_CONTRAST))
			{
				xml_set_attribute_float(screennode, "contrast", settings.m_contrast);
				changed = true;
			}

			if (settings.m_gamma != options_get_float(m_machine.options(), OPTION_GAMMA))
			{
				xml_set_attribute_float(screennode, "gamma", settings.m_gamma);
				changed = true;
			}

			// output the positioning controls
			if (settings.m_xoffset != 0.0f)
			{
				xml_set_attribute_float(screennode, "hoffset", settings.m_xoffset);
				changed = true;
			}

			if (settings.m_xscale != 1.0f)
			{
				xml_set_attribute_float(screennode, "hstretch", settings.m_xscale);
				changed = true;
			}

			if (settings.m_yoffset != 0.0f)
			{
				xml_set_attribute_float(screennode, "voffset", settings.m_yoffset);
				changed = true;
			}

			if (settings.m_yscale != 1.0f)
			{
				xml_set_attribute_float(screennode, "vstretch", settings.m_yscale);
				changed = true;
			}

			// if nothing changed, kill the node
			if (!changed)
				xml_delete_node(screennode);
		}
	}
}
