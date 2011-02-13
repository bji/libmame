/***************************************************************************

    rendlay.h

    Core rendering layout parser and manager.

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

***************************************************************************/

#ifndef __RENDLAY_H__
#define __RENDLAY_H__

#include "render.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum item_layer
{
	ITEM_LAYER_FIRST = 0,
	ITEM_LAYER_BACKDROP = ITEM_LAYER_FIRST,
	ITEM_LAYER_SCREEN,
	ITEM_LAYER_OVERLAY,
	ITEM_LAYER_BEZEL,
	ITEM_LAYER_MAX
};
DECLARE_ENUM_OPERATORS(item_layer);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> layout_element

// a layout_element is a single named element, which may have multiple components
class layout_element
{
	friend class simple_list<layout_element>;

public:
	// construction/destruction
	layout_element(running_machine &machine, xml_data_node &elemnode, const char *dirname);
	virtual ~layout_element();

	// getters
	layout_element *next() const { return m_next; }
	const char *name() const { return m_name; }
	running_machine &machine() const { return m_machine; }
	int default_state() const { return m_defstate; }
	int maxstate() const { return m_maxstate; }
	render_texture *state_texture(int state);

private:
	// a component represents an image, rectangle, or disk in an element
	class component
	{
		friend class layout_element;
		friend class simple_list<component>;

	public:
		// construction/destruction
		component(running_machine &machine, xml_data_node &compnode, const char *dirname);
		~component();

		// getters
		component *next() const { return m_next; }
		const render_bounds &bounds() const { return m_bounds; }

		// operations
		void draw(running_machine &machine, bitmap_t &dest, const rectangle &bounds, int state);

	private:
		// component types
		enum component_type
		{
			CTYPE_INVALID = 0,
			CTYPE_IMAGE,
			CTYPE_RECT,
			CTYPE_DISK,
			CTYPE_TEXT,
			CTYPE_LED7SEG,
			CTYPE_LED14SEG,
			CTYPE_LED16SEG,
			CTYPE_LED14SEGSC,
			CTYPE_LED16SEGSC,
			CTYPE_DOTMATRIX,
			CTYPE_MAX
		};

		// helpers
		void draw_rect(bitmap_t &dest, const rectangle &bounds);
		void draw_disk(bitmap_t &dest, const rectangle &bounds);
		void draw_text(running_machine &machine, bitmap_t &dest, const rectangle &bounds);
		bitmap_t *load_bitmap();
		void draw_led7seg(bitmap_t &dest, const rectangle &bounds, int pattern);
		void draw_led14seg(bitmap_t &dest, const rectangle &bounds, int pattern);
		void draw_led14segsc(bitmap_t &dest, const rectangle &bounds, int pattern);
		void draw_led16seg(bitmap_t &dest, const rectangle &bounds, int pattern);
		void draw_led16segsc(bitmap_t &dest, const rectangle &bounds, int pattern);
		void draw_dotmatrix(bitmap_t &dest, const rectangle &bounds, int pattern);
		void draw_segment_horizontal_caps(bitmap_t &dest, int minx, int maxx, int midy, int width, int caps, rgb_t color);
		void draw_segment_horizontal(bitmap_t &dest, int minx, int maxx, int midy, int width, rgb_t color);
		void draw_segment_vertical_caps(bitmap_t &dest, int miny, int maxy, int midx, int width, int caps, rgb_t color);
		void draw_segment_vertical(bitmap_t &dest, int miny, int maxy, int midx, int width, rgb_t color);
		void draw_segment_diagonal_1(bitmap_t &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		void draw_segment_diagonal_2(bitmap_t &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		void draw_segment_decimal(bitmap_t &dest, int midx, int midy, int width, rgb_t color);
		void draw_segment_comma(bitmap_t &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color);
		void apply_skew(bitmap_t &dest, int skewwidth);

		// internal state
		component *			m_next;			// link to next component
		component_type		m_type;			// type of component
		int					m_state;		// state where this component is visible (-1 means all states)
		render_bounds		m_bounds;		// bounds of the element
		render_color		m_color;		// color of the element
		astring				m_string;		// string for text components
		bitmap_t *			m_bitmap;		// source bitmap for images
		astring				m_dirname;		// directory name of image file (for lazy loading)
		astring				m_imagefile;	// name of the image file (for lazy loading)
		astring				m_alphafile;	// name of the alpha file (for lazy loading)
		bool				m_hasalpha;		// is there any alpha component present?
	};

	// a texture encapsulates a texture for a given element in a given state
	class texture
	{
	public:
		texture();
		~texture();

		layout_element *	m_element;		// pointer back to the element
		render_texture *	m_texture;		// texture for this state
		int					m_state;		// associated state number
	};

	// internal helpers
	static void element_scale(bitmap_t &dest, const bitmap_t &source, const rectangle &sbounds, void *param);

	// internal state
	layout_element *	m_next;				// link to next element
	running_machine &	m_machine;			// reference to the owning machine
	astring				m_name;				// name of this element
	simple_list<component> m_complist;		// list of components
	int					m_defstate;			// default state of this element
	int					m_maxstate;			// maximum state value for all components
	texture *			m_elemtex;			// array of element textures used for managing the scaled bitmaps
};


// ======================> layout_view

// a layout_view encapsulates a named list of items
class layout_view
{
	friend class simple_list<layout_view>;

public:
	// an item is a single backdrop, screen, overlay, or bezel item
	class item
	{
		friend class layout_view;
		friend class simple_list<item>;

	public:
		// construction/destruction
		item(running_machine &machine, xml_data_node &itemnode, simple_list<layout_element> &elemlist);
		virtual ~item();

		// getters
		item *next() const { return m_next; }
		layout_element *element() const { return m_element; }
		screen_device *screen() { return m_screen; }
		const render_bounds &bounds() const { return m_bounds; }
		const render_color &color() const { return m_color; }
		int orientation() const { return m_orientation; }
		render_container *screen_container(running_machine &machine) const { return (m_screen != NULL) ? &m_screen->container() : NULL; }
		bool has_input() const { return bool(m_input_tag); }
		const char *input_tag_and_mask(UINT32 &mask) const { mask = m_input_mask; return m_input_tag; }

		// fetch state based on configured source
		int state() const;

	private:
		// internal state
		item *				m_next;				// link to next item
		layout_element *	m_element;			// pointer to the associated element (non-screens only)
		astring				m_output_name;		// name of this item
		astring				m_input_tag;		// input tag of this item
		UINT32				m_input_mask;		// input mask of this item
		screen_device *		m_screen;			// pointer to screen
		int					m_orientation;		// orientation of this item
		render_bounds		m_bounds;			// bounds of the item
		render_bounds		m_rawbounds;		// raw (original) bounds of the item
		render_color		m_color;			// color of the item
	};

	// construction/destruction
	layout_view(running_machine &machine, xml_data_node &viewnode, simple_list<layout_element> &elemlist);
	virtual ~layout_view();

	// getters
	layout_view *next() const { return m_next; }
	item *first_item(item_layer layer) const;
	const char *name() const { return m_name; }
	const render_screen_list &screens() const { return m_screens; }
	bool layer_enabled(item_layer layer) const { return m_layenabled[layer]; }

	//
	bool has_art() const { return (m_backdrop_list.count() + m_overlay_list.count() + m_bezel_list.count() != 0); }
	float effective_aspect(render_layer_config config) const { return (config.zoom_to_screen() && m_screens.count() != 0) ? m_scraspect : m_aspect; }

	// operations
	void recompute(render_layer_config layerconfig);

private:
	// internal state
	layout_view *		m_next;				// pointer to next layout in the list
	astring				m_name;				// name of the layout
	float				m_aspect;			// X/Y of the layout
	float				m_scraspect;		// X/Y of the screen areas
	render_screen_list	m_screens;			// list of active screens
	render_bounds		m_bounds;			// computed bounds of the view
	render_bounds		m_scrbounds;		// computed bounds of the screens within the view
	render_bounds		m_expbounds;		// explicit bounds of the view
	bool				m_layenabled[ITEM_LAYER_MAX]; // is this layer enabled?
	simple_list<item>	m_backdrop_list;	// list of backdrop items
	simple_list<item>	m_screen_list;		// list of screen items
	simple_list<item>	m_overlay_list;		// list of overlay items
	simple_list<item>	m_bezel_list;		// list of bezel items
};


// ======================> layout_file

// a layout_file consists of a list of elements and a list of views
class layout_file
{
	friend class simple_list<layout_file>;

public:
	// construction/destruction
	layout_file(running_machine &machine, xml_data_node &rootnode, const char *dirname);
	virtual ~layout_file();

	// getters
	layout_file *next() const { return m_next; }
	layout_element *first_element() const { return m_elemlist.first(); }
	layout_view *first_view() const { return m_viewlist.first(); }

private:
	// internal state
	layout_file *		m_next;				// pointer to the next file in the list
	simple_list<layout_element> m_elemlist;	// list of shared layout elements
	simple_list<layout_view> m_viewlist;	// list of views
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// single screen layouts
extern const char layout_horizont[];	// horizontal 4:3 screens
extern const char layout_vertical[];	// vertical 4:3 screens

// dual screen layouts
extern const char layout_dualhsxs[];	// dual 4:3 screens side-by-side
extern const char layout_dualhovu[];	// dual 4:3 screens above and below
extern const char layout_dualhuov[];	// dual 4:3 screens below and above

// triple screen layouts
extern const char layout_triphsxs[];	// triple 4:3 screens side-by-side

// generic color overlay layouts
extern const char layout_ho20ffff[];	// horizontal 4:3 with 20,FF,FF color overlay
extern const char layout_ho2eff2e[];	// horizontal 4:3 with 2E,FF,2E color overlay
extern const char layout_ho4f893d[];	// horizontal 4:3 with 4F,89,3D color overlay
extern const char layout_ho88ffff[];	// horizontal 4:3 with 88,FF,FF color overlay
extern const char layout_hoa0a0ff[];	// horizontal 4:3 with A0,A0,FF color overlay
extern const char layout_hoffe457[];	// horizontal 4:3 with FF,E4,57 color overlay
extern const char layout_hoffff20[];	// horizontal 4:3 with FF,FF,20 color overlay
extern const char layout_voffff20[];	// vertical 4:3 with FF,FF,20 color overlay


#endif	// __RENDLAY_H__
