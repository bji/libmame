/***************************************************************************

    rendlay.c

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

****************************************************************************

    Overview of objects:

        layout_file -- A layout_file comprises a list of elements and a
            list of views. The elements are reusable items that the views
            reference.

        layout_view -- A layout_view describes a single view within a
            layout_file. The view is described using arbitrary coordinates
            that are scaled to fit within the render target. Pixels within
            a view are assumed to be square.

        view_item -- Each view has four lists of view_items, one for each
            "layer." Each view item is specified using floating point
            coordinates in arbitrary units, and is assumed to have square
            pixels. Each view item can control its orientation independently.
            Each item can also have an optional name, and can be set at
            runtime into different "states", which control how the embedded
            elements are displayed.

        layout_element -- A layout_element is a description of a piece of
            visible artwork. Most view_items (except for those in the screen
            layer) have exactly one layout_element which describes the
            contents of the item. Elements are separate from items because
            they can be re-used multiple times within a layout. Even though
            an element can contain a number of components, they are treated
            as if they were a single bitmap.

        element_component -- Each layout_element contains one or more
            components. Each component can describe either an image or
            a rectangle/disk primitive. Each component also has a "state"
            associated with it, which controls whether or not the component
            is visible (if the owning item has the same state, it is
            visible).

***************************************************************************/

#include <ctype.h>

#include "emu.h"
#include "emuopts.h"
#include "render.h"
#include "rendfont.h"
#include "rendlay.h"
#include "rendutil.h"
#include "output.h"
#include "xmlfile.h"
#include "png.h"



/***************************************************************************
    STANDARD LAYOUTS
***************************************************************************/

// single screen layouts
#include "horizont.lh"
#include "vertical.lh"

// dual screen layouts
#include "dualhsxs.lh"
#include "dualhovu.lh"
#include "dualhuov.lh"

// triple screen layouts
#include "triphsxs.lh"

// generic color overlay layouts
#include "ho20ffff.lh"
#include "ho2eff2e.lh"
#include "ho4f893d.lh"
#include "ho88ffff.lh"
#include "hoa0a0ff.lh"
#include "hoffe457.lh"
#include "voffff20.lh"
#include "hoffff20.lh"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int LAYOUT_VERSION = 2;

enum
{
	LINE_CAP_NONE = 0,
	LINE_CAP_START = 1,
	LINE_CAP_END = 2
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const render_screen_list render_target::s_empty_screen_list;



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  gcd - compute the greatest common divisor (GCD)
//  of two integers using the Euclidean algorithm
//-------------------------------------------------

inline int gcd(int a, int b)
{
	while (b != 0)
	{
		int t = b;
		b = a % b;
		a = t;
	}
	return a;
}


//-------------------------------------------------
//  reduce_fraction - reduce a fraction by
//  dividing out common factors
//-------------------------------------------------

inline void reduce_fraction(int &num, int &den)
{
	// search the greatest common divisor
	int div = gcd(num, den);

	// reduce the fraction if a common divisor has been found
	if (div > 1)
	{
		num /= div;
		den /= div;
	}
}



//**************************************************************************
//  SHARED PARSING HELPERS
//**************************************************************************

//-------------------------------------------------
//  get_variable_value - compute the value of
//  a variable in an XML attribute
//-------------------------------------------------

static int get_variable_value(running_machine &machine, const char *string, char **outputptr)
{
	char temp[100];

	// screen 0 parameters
	for (const screen_device_config *devconfig = machine.m_config.first_screen(); devconfig != NULL; devconfig = devconfig->next_screen())
	{
		int scrnum = machine.m_config.m_devicelist.index(SCREEN, devconfig->tag());

		// native X aspect factor
		sprintf(temp, "~scr%dnativexaspect~", scrnum);
		if (!strncmp(string, temp, strlen(temp)))
		{
			int num = devconfig->visible_area().max_x + 1 - devconfig->visible_area().min_x;
			int den = devconfig->visible_area().max_y + 1 - devconfig->visible_area().min_y;
			reduce_fraction(num, den);
			*outputptr += sprintf(*outputptr, "%d", num);
			return strlen(temp);
		}

		// native Y aspect factor
		sprintf(temp, "~scr%dnativeyaspect~", scrnum);
		if (!strncmp(string, temp, strlen(temp)))
		{
			int num = devconfig->visible_area().max_x + 1 - devconfig->visible_area().min_x;
			int den = devconfig->visible_area().max_y + 1 - devconfig->visible_area().min_y;
			reduce_fraction(num, den);
			*outputptr += sprintf(*outputptr, "%d", den);
			return strlen(temp);
		}

		// native width
		sprintf(temp, "~scr%dwidth~", scrnum);
		if (!strncmp(string, temp, strlen(temp)))
		{
			*outputptr += sprintf(*outputptr, "%d", devconfig->visible_area().max_x + 1 - devconfig->visible_area().min_x);
			return strlen(temp);
		}

		// native height
		sprintf(temp, "~scr%dheight~", scrnum);
		if (!strncmp(string, temp, strlen(temp)))
		{
			*outputptr += sprintf(*outputptr, "%d", devconfig->visible_area().max_y + 1 - devconfig->visible_area().min_y);
			return strlen(temp);
		}
	}

	// default: copy the first character and continue
	**outputptr = *string;
	*outputptr += 1;
	return 1;
}


//-------------------------------------------------
//  xml_get_attribute_string_with_subst - analog
//  to xml_get_attribute_string but with variable
//  substitution
//-------------------------------------------------

static const char *xml_get_attribute_string_with_subst(running_machine &machine, xml_data_node &node, const char *attribute, const char *defvalue)
{
	const char *str = xml_get_attribute_string(&node, attribute, NULL);
	static char buffer[1000];

	// if nothing, just return the default
	if (str == NULL)
		return defvalue;

	// if no tildes, don't worry
	if (strchr(str, '~') == NULL)
		return str;

	// make a copy of the string, doing substitutions along the way
	const char *s;
	char *d;
	for (s = str, d = buffer; *s != 0; )
	{
		// if not a variable, just copy
		if (*s != '~')
			*d++ = *s++;

		// extract the variable
		else
			s += get_variable_value(machine, s, &d);
	}
	*d = 0;
	return buffer;
}


//-------------------------------------------------
//  xml_get_attribute_int_with_subst - analog
//  to xml_get_attribute_int but with variable
//  substitution
//-------------------------------------------------

static int xml_get_attribute_int_with_subst(running_machine &machine, xml_data_node &node, const char *attribute, int defvalue)
{
	const char *string = xml_get_attribute_string_with_subst(machine, node, attribute, NULL);
	int value;

	if (string == NULL)
		return defvalue;
	if (string[0] == '$')
		return (sscanf(&string[1], "%X", &value) == 1) ? value : defvalue;
	if (string[0] == '0' && string[1] == 'x')
		return (sscanf(&string[2], "%X", &value) == 1) ? value : defvalue;
	if (string[0] == '#')
		return (sscanf(&string[1], "%d", &value) == 1) ? value : defvalue;
	return (sscanf(&string[0], "%d", &value) == 1) ? value : defvalue;
}


//-------------------------------------------------
//  xml_get_attribute_float_with_subst - analog
//  to xml_get_attribute_float but with variable
//  substitution
//-------------------------------------------------

static float xml_get_attribute_float_with_subst(running_machine &machine, xml_data_node &node, const char *attribute, float defvalue)
{
	const char *string = xml_get_attribute_string_with_subst(machine, node, attribute, NULL);
	float value;

	if (string == NULL || sscanf(string, "%f", &value) != 1)
		return defvalue;
	return value;
}


//-------------------------------------------------
//  parse_bounds - parse a bounds XML node
//-------------------------------------------------

void parse_bounds(running_machine &machine, xml_data_node *boundsnode, render_bounds &bounds)
{
	// skip if nothing
	if (boundsnode == NULL)
	{
		bounds.x0 = bounds.y0 = 0.0f;
		bounds.x1 = bounds.y1 = 1.0f;
		return;
	}

	// parse out the data
	if (xml_get_attribute(boundsnode, "left") != NULL)
	{
		// left/right/top/bottom format
		bounds.x0 = xml_get_attribute_float_with_subst(machine, *boundsnode, "left", 0.0f);
		bounds.x1 = xml_get_attribute_float_with_subst(machine, *boundsnode, "right", 1.0f);
		bounds.y0 = xml_get_attribute_float_with_subst(machine, *boundsnode, "top", 0.0f);
		bounds.y1 = xml_get_attribute_float_with_subst(machine, *boundsnode, "bottom", 1.0f);
	}
	else if (xml_get_attribute(boundsnode, "x") != NULL)
	{
		// x/y/width/height format
		bounds.x0 = xml_get_attribute_float_with_subst(machine, *boundsnode, "x", 0.0f);
		bounds.x1 = bounds.x0 + xml_get_attribute_float_with_subst(machine, *boundsnode, "width", 1.0f);
		bounds.y0 = xml_get_attribute_float_with_subst(machine, *boundsnode, "y", 0.0f);
		bounds.y1 = bounds.y0 + xml_get_attribute_float_with_subst(machine, *boundsnode, "height", 1.0f);
	}
	else
		throw emu_fatalerror("Illegal bounds value in XML");

	// check for errors
	if (bounds.x0 > bounds.x1 || bounds.y0 > bounds.y1)
		throw emu_fatalerror("Illegal bounds value in XML: (%f-%f)-(%f-%f)", bounds.x0, bounds.x1, bounds.y0, bounds.y1);
}


//-------------------------------------------------
//  parse_color - parse a color XML node
//-------------------------------------------------

void parse_color(running_machine &machine, xml_data_node *colornode, render_color &color)
{
	// skip if nothing
	if (colornode == NULL)
	{
		color.r = color.g = color.b = color.a = 1.0f;
		return;
	}

	// parse out the data
	color.r = xml_get_attribute_float_with_subst(machine, *colornode, "red", 1.0);
	color.g = xml_get_attribute_float_with_subst(machine, *colornode, "green", 1.0);
	color.b = xml_get_attribute_float_with_subst(machine, *colornode, "blue", 1.0);
	color.a = xml_get_attribute_float_with_subst(machine, *colornode, "alpha", 1.0);

	// check for errors
	if (color.r < 0.0 || color.r > 1.0 || color.g < 0.0 || color.g > 1.0 ||
		color.b < 0.0 || color.b > 1.0 || color.a < 0.0 || color.a > 1.0)
		throw emu_fatalerror("Illegal ARGB color value in XML: %f,%f,%f,%f", color.r, color.g, color.b, color.a);
}


//-------------------------------------------------
//  parse_orientation - parse an orientation XML
//  node
//-------------------------------------------------

static void parse_orientation(running_machine &machine, xml_data_node *orientnode, int &orientation)
{
	// skip if nothing
	if (orientnode == NULL)
	{
		orientation = ROT0;
		return;
	}

	// parse out the data
	int rotate = xml_get_attribute_int_with_subst(machine, *orientnode, "rotate", 0);
	switch (rotate)
	{
		case 0:		orientation = ROT0;		break;
		case 90:	orientation = ROT90;	break;
		case 180:	orientation = ROT180;	break;
		case 270:	orientation = ROT270;	break;
		default:	throw emu_fatalerror("Invalid rotation in XML orientation node: %d", rotate); break;
	}
	if (strcmp("yes", xml_get_attribute_string_with_subst(machine, *orientnode, "swapxy", "no")) == 0)
		orientation ^= ORIENTATION_SWAP_XY;
	if (strcmp("yes", xml_get_attribute_string_with_subst(machine, *orientnode, "flipx", "no")) == 0)
		orientation ^= ORIENTATION_FLIP_X;
	if (strcmp("yes", xml_get_attribute_string_with_subst(machine, *orientnode, "flipy", "no")) == 0)
		orientation ^= ORIENTATION_FLIP_Y;
}



//**************************************************************************
//  LAYOUT ELEMENT
//**************************************************************************

//-------------------------------------------------
//  layout_element - constructor
//-------------------------------------------------

layout_element::layout_element(running_machine &machine, xml_data_node &elemnode, const char *dirname)
	: m_next(NULL),
	  m_machine(machine),
	  m_complist(machine.m_respool),
	  m_defstate(0),
	  m_maxstate(0),
	  m_elemtex(NULL)
{
	// extract the name
	const char *name = xml_get_attribute_string_with_subst(machine, elemnode, "name", NULL);
	if (name == NULL)
		throw emu_fatalerror("All layout elements must have a name!\n");
	m_name = name;

	// get the default state
	m_defstate = xml_get_attribute_int_with_subst(machine, elemnode, "defstate", -1);

	// parse components in order
	bool first = true;
	render_bounds bounds = { 0 };
	for (xml_data_node *compnode = elemnode.child; compnode != NULL; compnode = compnode->next)
	{
		// allocate a new component
		component &newcomp = m_complist.append(*auto_alloc(&machine, component(machine, *compnode, dirname)));

		// accumulate bounds
		if (first)
			bounds = newcomp.m_bounds;
		else
			union_render_bounds(&bounds, &newcomp.m_bounds);
		first = false;

		// determine the maximum state
		if (newcomp.m_state > m_maxstate)
			m_maxstate = newcomp.m_state;
		if (newcomp.m_type == component::CTYPE_LED7SEG)
			m_maxstate = 255;
		if (newcomp.m_type == component::CTYPE_LED14SEG)
			m_maxstate = 16383;
		if (newcomp.m_type == component::CTYPE_LED14SEGSC || newcomp.m_type == component::CTYPE_LED16SEG)
			m_maxstate = 65535;
		if (newcomp.m_type == component::CTYPE_LED16SEGSC)
			m_maxstate = 262143;
		if (newcomp.m_type == component::CTYPE_DOTMATRIX)
			m_maxstate = 255;
	}

	// determine the scale/offset for normalization
	float xoffs = bounds.x0;
	float yoffs = bounds.y0;
	float xscale = 1.0f / (bounds.x1 - bounds.x0);
	float yscale = 1.0f / (bounds.y1 - bounds.y0);

	// normalize all the component bounds
	for (component *curcomp = m_complist.first(); curcomp != NULL; curcomp = curcomp->next())
	{
		curcomp->m_bounds.x0 = (curcomp->m_bounds.x0 - xoffs) * xscale;
		curcomp->m_bounds.x1 = (curcomp->m_bounds.x1 - xoffs) * xscale;
		curcomp->m_bounds.y0 = (curcomp->m_bounds.y0 - yoffs) * yscale;
		curcomp->m_bounds.y1 = (curcomp->m_bounds.y1 - yoffs) * yscale;
	}

	// allocate an array of element textures for the states
	m_elemtex = auto_alloc_array(&machine, texture, m_maxstate + 1);
}


//-------------------------------------------------
//  ~layout_element - destructor
//-------------------------------------------------

layout_element::~layout_element()
{
	// loop over all states and free their textures
	auto_free(&m_machine, m_elemtex);
}


//-------------------------------------------------
//  state_texture - return a pointer to a
//  render_texture for the given state, allocating
//  one if needed
//-------------------------------------------------

render_texture *layout_element::state_texture(int state)
{
	assert(state <= m_maxstate);
	if (m_elemtex[state].m_texture == NULL)
	{
		m_elemtex[state].m_element = this;
		m_elemtex[state].m_state = state;
		m_elemtex[state].m_texture = m_machine.render().texture_alloc(element_scale, &m_elemtex[state]);
	}
	return m_elemtex[state].m_texture;
}


//-------------------------------------------------
//  element_scale - scale an element by rendering
//  all the components at the appropriate
//  resolution
//-------------------------------------------------

void layout_element::element_scale(bitmap_t &dest, const bitmap_t &source, const rectangle &sbounds, void *param)
{
	texture *elemtex = (texture *)param;

	// iterate over components that are part of the current state
	for (component *curcomp = elemtex->m_element->m_complist.first(); curcomp != NULL; curcomp = curcomp->next())
		if (curcomp->m_state == -1 || curcomp->m_state == elemtex->m_state)
		{
			// get the local scaled bounds
			rectangle bounds;
			bounds.min_x = render_round_nearest(curcomp->bounds().x0 * dest.width);
			bounds.min_y = render_round_nearest(curcomp->bounds().y0 * dest.height);
			bounds.max_x = render_round_nearest(curcomp->bounds().x1 * dest.width);
			bounds.max_y = render_round_nearest(curcomp->bounds().y1 * dest.height);

			// based on the component type, add to the texture
			curcomp->draw(elemtex->m_element->m_machine, dest, bounds, elemtex->m_state);
		}
}


//**************************************************************************
//  LAYOUT ELEMENT TEXTURE
//**************************************************************************

//-------------------------------------------------
//  texture - constructor
//-------------------------------------------------

layout_element::texture::texture()
	: m_element(NULL),
	  m_texture(NULL),
	  m_state(0)
{
}


//-------------------------------------------------
//  ~texture - destructor
//-------------------------------------------------

layout_element::texture::~texture()
{
	if (m_element != NULL)
		m_element->m_machine.render().texture_free(m_texture);
}



//**************************************************************************
//  LAYOUT ELEMENT COMPONENT
//**************************************************************************

//-------------------------------------------------
//  component - constructor
//-------------------------------------------------

layout_element::component::component(running_machine &machine, xml_data_node &compnode, const char *dirname)
	: m_next(NULL),
	  m_type(CTYPE_INVALID),
	  m_state(0),
	  m_bitmap(NULL),
	  m_hasalpha(false)
{
	// fetch common data
	m_state = xml_get_attribute_int_with_subst(machine, compnode, "state", -1);
	parse_bounds(machine, xml_get_sibling(compnode.child, "bounds"), m_bounds);
	parse_color(machine, xml_get_sibling(compnode.child, "color"), m_color);

	// image nodes
	if (strcmp(compnode.name, "image") == 0)
	{
		m_type = CTYPE_IMAGE;
		m_dirname = dirname;
		m_imagefile = xml_get_attribute_string_with_subst(machine, compnode, "file", "");
		m_alphafile = xml_get_attribute_string_with_subst(machine, compnode, "alphafile", "");
	}

	// text nodes
	else if (strcmp(compnode.name, "text") == 0)
	{
		m_type = CTYPE_TEXT;
		m_string = xml_get_attribute_string_with_subst(machine, compnode, "string", "");
	}

	// dotmatrix nodes
	else if (strcmp(compnode.name, "dotmatrix") == 0)
		m_type = CTYPE_DOTMATRIX;

	// led7seg nodes
	else if (strcmp(compnode.name, "led7seg") == 0)
		m_type = CTYPE_LED7SEG;

	// led14seg nodes
	else if (strcmp(compnode.name, "led14seg") == 0)
		m_type = CTYPE_LED14SEG;

	// led14segsc nodes
	else if (strcmp(compnode.name, "led14segsc") == 0)
		m_type = CTYPE_LED14SEGSC;

	// led16seg nodes
	else if (strcmp(compnode.name, "led16seg") == 0)
		m_type = CTYPE_LED16SEG;

	// led16segsc nodes
	else if (strcmp(compnode.name, "led16segsc") == 0)
		m_type = CTYPE_LED16SEGSC;

	// rect nodes
	else if (strcmp(compnode.name, "rect") == 0)
		m_type = CTYPE_RECT;

	// disk nodes
	else if (strcmp(compnode.name, "disk") == 0)
		m_type = CTYPE_DISK;

	// error otherwise
	else
		throw emu_fatalerror("Unknown element component: %s", compnode.name);
}


//-------------------------------------------------
//  ~component - destructor
//-------------------------------------------------

layout_element::component::~component()
{
	global_free(m_bitmap);
}


//-------------------------------------------------
//  draw - draw a component
//-------------------------------------------------

void layout_element::component::draw(running_machine &machine, bitmap_t &dest, const rectangle &bounds, int state)
{
	switch (m_type)
	{
		case CTYPE_IMAGE:
			if (m_bitmap == NULL)
				m_bitmap = load_bitmap();
			render_resample_argb_bitmap_hq(
					BITMAP_ADDR32(&dest, bounds.min_y, bounds.min_x),
					dest.rowpixels,
					bounds.max_x - bounds.min_x,
					bounds.max_y - bounds.min_y,
					m_bitmap, NULL, &m_color);
			break;

		case CTYPE_RECT:
			draw_rect(dest, bounds);
			break;

		case CTYPE_DISK:
			draw_disk(dest, bounds);
			break;

		case CTYPE_TEXT:
			draw_text(machine, dest, bounds);
			break;

		case CTYPE_LED7SEG:
			draw_led7seg(dest, bounds, state);
			break;

		case CTYPE_LED14SEG:
			draw_led14seg(dest, bounds, state);
			break;

		case CTYPE_LED16SEG:
			draw_led16seg(dest, bounds, state);
			break;

		case CTYPE_LED14SEGSC:
			draw_led14segsc(dest, bounds, state);
			break;

		case CTYPE_LED16SEGSC:
			draw_led16segsc(dest, bounds, state);
			break;

		case CTYPE_DOTMATRIX:
			draw_dotmatrix(dest, bounds, state);
			break;

		default:
			throw emu_fatalerror("Unknown component type requested draw()");
	}
}


//-------------------------------------------------
//  draw_rect - draw a rectangle in the specified
//  color
//-------------------------------------------------

void layout_element::component::draw_rect(bitmap_t &dest, const rectangle &bounds)
{
	// compute premultiplied colors
	UINT32 r = m_color.r * m_color.a * 255.0;
	UINT32 g = m_color.g * m_color.a * 255.0;
	UINT32 b = m_color.b * m_color.a * 255.0;
	UINT32 inva = (1.0f - m_color.a) * 255.0;

	// iterate over X and Y
	for (UINT32 y = bounds.min_y; y < bounds.max_y; y++)
		for (UINT32 x = bounds.min_x; x < bounds.max_x; x++)
		{
			UINT32 finalr = r;
			UINT32 finalg = g;
			UINT32 finalb = b;

			// if we're translucent, add in the destination pixel contribution
			if (inva > 0)
			{
				UINT32 dpix = *BITMAP_ADDR32(&dest, y, x);
				finalr += (RGB_RED(dpix) * inva) >> 8;
				finalg += (RGB_GREEN(dpix) * inva) >> 8;
				finalb += (RGB_BLUE(dpix) * inva) >> 8;
			}

			// store the target pixel, dividing the RGBA values by the overall scale factor
			*BITMAP_ADDR32(&dest, y, x) = MAKE_ARGB(0xff, finalr, finalg, finalb);
		}
}


//-------------------------------------------------
//  draw_disk - draw an ellipse in the specified
//  color
//-------------------------------------------------

void layout_element::component::draw_disk(bitmap_t &dest, const rectangle &bounds)
{
	// compute premultiplied colors
	UINT32 r = m_color.r * m_color.a * 255.0;
	UINT32 g = m_color.g * m_color.a * 255.0;
	UINT32 b = m_color.b * m_color.a * 255.0;
	UINT32 inva = (1.0f - m_color.a) * 255.0;

	// find the center
	float xcenter = (float)(bounds.min_x + bounds.max_x) * 0.5f;
	float ycenter = (float)(bounds.min_y + bounds.max_y) * 0.5f;
	float xradius = (float)(bounds.max_x - bounds.min_x) * 0.5f;
	float yradius = (float)(bounds.max_y - bounds.min_y) * 0.5f;
	float ooyradius2 = 1.0f / (yradius * yradius);

	// iterate over y
	for (UINT32 y = bounds.min_y; y < bounds.max_y; y++)
	{
		float ycoord = ycenter - ((float)y + 0.5f);
		float xval = xradius * sqrt(1.0f - (ycoord * ycoord) * ooyradius2);

		// compute left/right coordinates
		INT32 left = (INT32)(xcenter - xval + 0.5f);
		INT32 right = (INT32)(xcenter + xval + 0.5f);

		// draw this scanline
		for (UINT32 x = left; x < right; x++)
		{
			UINT32 finalr = r;
			UINT32 finalg = g;
			UINT32 finalb = b;

			// if we're translucent, add in the destination pixel contribution
			if (inva > 0)
			{
				UINT32 dpix = *BITMAP_ADDR32(&dest, y, x);
				finalr += (RGB_RED(dpix) * inva) >> 8;
				finalg += (RGB_GREEN(dpix) * inva) >> 8;
				finalb += (RGB_BLUE(dpix) * inva) >> 8;
			}

			// store the target pixel, dividing the RGBA values by the overall scale factor
			*BITMAP_ADDR32(&dest, y, x) = MAKE_ARGB(0xff, finalr, finalg, finalb);
		}
	}
}


//-------------------------------------------------
//  draw_text - draw text in the specified color
//-------------------------------------------------

void layout_element::component::draw_text(running_machine &machine, bitmap_t &dest, const rectangle &bounds)
{
	// compute premultiplied colors
	UINT32 r = m_color.r * 255.0;
	UINT32 g = m_color.g * 255.0;
	UINT32 b = m_color.b * 255.0;
	UINT32 a = m_color.a * 255.0;

	// get the width of the string
	render_font *font = machine.render().font_alloc("default");
	float aspect = 1.0f;
	INT32 width;
	while (1)
	{
		width = font->string_width(bounds.max_y - bounds.min_y, aspect, m_string);
		if (width < bounds.max_x - bounds.min_x)
			break;
		aspect *= 0.9f;
	}
	INT32 curx = bounds.min_x + (bounds.max_x - bounds.min_x - width) / 2;

	// allocate a temporary bitmap
	bitmap_t *tempbitmap = global_alloc(bitmap_t(dest.width, dest.height, BITMAP_FORMAT_ARGB32));

	// loop over characters
	for (const char *s = m_string; *s != 0; s++)
	{
		// get the font bitmap
		rectangle chbounds;
		font->get_scaled_bitmap_and_bounds(*tempbitmap, bounds.max_y - bounds.min_y, aspect, *s, chbounds);

		// copy the data into the target
		for (int y = 0; y < chbounds.max_y - chbounds.min_y; y++)
		{
			int effy = bounds.min_y + y;
			if (effy >= bounds.min_y && effy <= bounds.max_y)
			{
				UINT32 *src = BITMAP_ADDR32(tempbitmap, y, 0);
				UINT32 *d = BITMAP_ADDR32(&dest, effy, 0);
				for (int x = 0; x < chbounds.max_x - chbounds.min_x; x++)
				{
					int effx = curx + x + chbounds.min_x;
					if (effx >= bounds.min_x && effx <= bounds.max_x)
					{
						UINT32 spix = RGB_ALPHA(src[x]);
						if (spix != 0)
						{
							UINT32 dpix = d[effx];
							UINT32 ta = (a * (spix + 1)) >> 8;
							UINT32 tr = (r * ta + RGB_RED(dpix) * (0x100 - ta)) >> 8;
							UINT32 tg = (g * ta + RGB_GREEN(dpix) * (0x100 - ta)) >> 8;
							UINT32 tb = (b * ta + RGB_BLUE(dpix) * (0x100 - ta)) >> 8;
							d[effx] = MAKE_ARGB(0xff, tr, tg, tb);
						}
					}
				}
			}
		}

		// advance in the X direction
		curx += font->char_width(bounds.max_y - bounds.min_y, aspect, *s);
	}

	// free the temporary bitmap and font
	global_free(tempbitmap);
	machine.render().font_free(font);
}


//-------------------------------------------------
//  load_bitmap - load a PNG file with artwork for
//  a component
//-------------------------------------------------

bitmap_t *layout_element::component::load_bitmap()
{
	// load the basic bitmap
	bitmap_t *bitmap = render_load_png(OPTION_ARTPATH, m_dirname, m_imagefile, NULL, &m_hasalpha);
	if (bitmap != NULL && m_alphafile)

		// load the alpha bitmap if specified
		if (render_load_png(OPTION_ARTPATH, m_dirname, m_alphafile, bitmap, &m_hasalpha) == NULL)
		{
			global_free(bitmap);
			bitmap = NULL;
		}

	// if we can't load the bitmap, allocate a dummy one and report an error
	if (bitmap == NULL)
	{
		// draw some stripes in the bitmap
		bitmap = global_alloc(bitmap_t(100, 100, BITMAP_FORMAT_ARGB32));
		bitmap_fill(bitmap, NULL, 0);
		for (int step = 0; step < 100; step += 25)
			for (int line = 0; line < 100; line++)
				*BITMAP_ADDR32(bitmap, (step + line) % 100, line % 100) = MAKE_ARGB(0xff,0xff,0xff,0xff);

		// log an error
		if (!m_alphafile)
			mame_printf_warning("Unable to load component bitmap '%s'", m_imagefile.cstr());
		else
			mame_printf_warning("Unable to load component bitmap '%s'/'%s'", m_imagefile.cstr(), m_alphafile.cstr());
	}

	return bitmap;
}


//-------------------------------------------------
//  draw_led7seg - draw a 7-segment LCD
//-------------------------------------------------

void layout_element::component::draw_led7seg(bitmap_t &dest, const rectangle &bounds, int pattern)
{
	const rgb_t onpen = MAKE_ARGB(0xff,0xff,0xff,0xff);
	const rgb_t offpen = MAKE_ARGB(0xff,0x20,0x20,0x20);

	// sizes for computation
	int bmwidth = 250;
	int bmheight = 400;
	int segwidth = 40;
	int skewwidth = 40;

	// allocate a temporary bitmap for drawing
	bitmap_t *tempbitmap = global_alloc(bitmap_t(bmwidth + skewwidth, bmheight, BITMAP_FORMAT_ARGB32));
	bitmap_fill(tempbitmap, NULL, MAKE_ARGB(0xff,0x00,0x00,0x00));

	// top bar
	draw_segment_horizontal(*tempbitmap, 0 + 2*segwidth/3, bmwidth - 2*segwidth/3, 0 + segwidth/2, segwidth, (pattern & (1 << 0)) ? onpen : offpen);

	// top-right bar
	draw_segment_vertical(*tempbitmap, 0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2, segwidth, (pattern & (1 << 1)) ? onpen : offpen);

	// bottom-right bar
	draw_segment_vertical(*tempbitmap, bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2, segwidth, (pattern & (1 << 2)) ? onpen : offpen);

	// bottom bar
	draw_segment_horizontal(*tempbitmap, 0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight - segwidth/2, segwidth, (pattern & (1 << 3)) ? onpen : offpen);

	// bottom-left bar
	draw_segment_vertical(*tempbitmap, bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2, segwidth, (pattern & (1 << 4)) ? onpen : offpen);

	// top-left bar
	draw_segment_vertical(*tempbitmap, 0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2, segwidth, (pattern & (1 << 5)) ? onpen : offpen);

	// middle bar
	draw_segment_horizontal(*tempbitmap, 0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight/2, segwidth, (pattern & (1 << 6)) ? onpen : offpen);

	// apply skew
	apply_skew(*tempbitmap, 40);

	// decimal point
	draw_segment_decimal(*tempbitmap, bmwidth + segwidth/2, bmheight - segwidth/2, segwidth, (pattern & (1 << 7)) ? onpen : offpen);

	// resample to the target size
	render_resample_argb_bitmap_hq(dest.base, dest.rowpixels, dest.width, dest.height, tempbitmap, NULL, &m_color);

	global_free(tempbitmap);
}


//-------------------------------------------------
//  draw_led14seg - draw a 14-segment LCD
//-------------------------------------------------

void layout_element::component::draw_led14seg(bitmap_t &dest, const rectangle &bounds, int pattern)
{
	const rgb_t onpen = MAKE_ARGB(0xff, 0xff, 0xff, 0xff);
	const rgb_t offpen = MAKE_ARGB(0xff, 0x20, 0x20, 0x20);

	// sizes for computation
	int bmwidth = 250;
	int bmheight = 400;
	int segwidth = 40;
	int skewwidth = 40;

	// allocate a temporary bitmap for drawing
	bitmap_t *tempbitmap = global_alloc(bitmap_t(bmwidth + skewwidth, bmheight, BITMAP_FORMAT_ARGB32));
	bitmap_fill(tempbitmap, NULL, MAKE_ARGB(0xff, 0x00, 0x00, 0x00));

	// top bar
	draw_segment_horizontal(*tempbitmap,
		0 + 2*segwidth/3, bmwidth - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 0)) ? onpen : offpen);

	// right-top bar
	draw_segment_vertical(*tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 1)) ? onpen : offpen);

	// right-bottom bar
	draw_segment_vertical(*tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 2)) ? onpen : offpen);

	// bottom bar
	draw_segment_horizontal(*tempbitmap,
		0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
		segwidth, (pattern & (1 << 3)) ? onpen : offpen);

	// left-bottom bar
	draw_segment_vertical(*tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 4)) ? onpen : offpen);

	// left-top bar
	draw_segment_vertical(*tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 5)) ? onpen : offpen);

	// horizontal-middle-left bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 6)) ? onpen : offpen);

	// horizontal-middle-right bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 7)) ? onpen : offpen);

	// vertical-middle-top bar
	draw_segment_vertical_caps(*tempbitmap,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 8)) ? onpen : offpen);

	// vertical-middle-bottom bar
	draw_segment_vertical_caps(*tempbitmap,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 9)) ? onpen : offpen);

	// diagonal-left-bottom bar
	draw_segment_diagonal_1(*tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 10)) ? onpen : offpen);

	// diagonal-left-top bar
	draw_segment_diagonal_2(*tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 11)) ? onpen : offpen);

	// diagonal-right-top bar
	draw_segment_diagonal_1(*tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 12)) ? onpen : offpen);

	// diagonal-right-bottom bar
	draw_segment_diagonal_2(*tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 13)) ? onpen : offpen);

	// apply skew
	apply_skew(*tempbitmap, 40);

	// resample to the target size
	render_resample_argb_bitmap_hq(dest.base, dest.rowpixels, dest.width, dest.height, tempbitmap, NULL, &m_color);

	global_free(tempbitmap);
}


//-------------------------------------------------
//  draw_led14segsc - draw a 14-segment LCD with
//  semicolon (2 extra segments)
//-------------------------------------------------

void layout_element::component::draw_led14segsc(bitmap_t &dest, const rectangle &bounds, int pattern)
{
	const rgb_t onpen = MAKE_ARGB(0xff, 0xff, 0xff, 0xff);
	const rgb_t offpen = MAKE_ARGB(0xff, 0x20, 0x20, 0x20);

	// sizes for computation
	int bmwidth = 250;
	int bmheight = 400;
	int segwidth = 40;
	int skewwidth = 40;

	// allocate a temporary bitmap for drawing, adding some extra space for the tail
	bitmap_t *tempbitmap = global_alloc(bitmap_t(bmwidth + skewwidth, bmheight + segwidth, BITMAP_FORMAT_ARGB32));
	bitmap_fill(tempbitmap, NULL, MAKE_ARGB(0xff, 0x00, 0x00, 0x00));

	// top bar
	draw_segment_horizontal(*tempbitmap,
		0 + 2*segwidth/3, bmwidth - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 0)) ? onpen : offpen);

	// right-top bar
	draw_segment_vertical(*tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 1)) ? onpen : offpen);

	// right-bottom bar
	draw_segment_vertical(*tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 2)) ? onpen : offpen);

	// bottom bar
	draw_segment_horizontal(*tempbitmap,
		0 + 2*segwidth/3, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
		segwidth, (pattern & (1 << 3)) ? onpen : offpen);

	// left-bottom bar
	draw_segment_vertical(*tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 4)) ? onpen : offpen);

	// left-top bar
	draw_segment_vertical(*tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 5)) ? onpen : offpen);

	// horizontal-middle-left bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 6)) ? onpen : offpen);

	// horizontal-middle-right bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 7)) ? onpen : offpen);

	// vertical-middle-top bar
	draw_segment_vertical_caps(*tempbitmap,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 8)) ? onpen : offpen);

	// vertical-middle-bottom bar
	draw_segment_vertical_caps(*tempbitmap,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 9)) ? onpen : offpen);

	// diagonal-left-bottom bar
	draw_segment_diagonal_1(*tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 10)) ? onpen : offpen);

	// diagonal-left-top bar
	draw_segment_diagonal_2(*tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 11)) ? onpen : offpen);

	// diagonal-right-top bar
	draw_segment_diagonal_1(*tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 12)) ? onpen : offpen);

	// diagonal-right-bottom bar
	draw_segment_diagonal_2(*tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 13)) ? onpen : offpen);

	// apply skew
	apply_skew(*tempbitmap, 40);

	// decimal point
	draw_segment_decimal(*tempbitmap, bmwidth + segwidth/2, bmheight - segwidth/2, segwidth, (pattern & (1 << 14)) ? onpen : offpen);

	// comma tail
	draw_segment_diagonal_1(*tempbitmap,
		bmwidth - (segwidth/2), bmwidth + segwidth,
		bmheight - (segwidth), bmheight + segwidth*1.5,
		segwidth/2, (pattern & (1 << 15)) ? onpen : offpen);

	// resample to the target size
	render_resample_argb_bitmap_hq(dest.base, dest.rowpixels, dest.width, dest.height, tempbitmap, NULL, &m_color);

	global_free(tempbitmap);
}


//-------------------------------------------------
//  draw_led16seg - draw a 16-segment LCD
//-------------------------------------------------

void layout_element::component::draw_led16seg(bitmap_t &dest, const rectangle &bounds, int pattern)
{
	const rgb_t onpen = MAKE_ARGB(0xff, 0xff, 0xff, 0xff);
	const rgb_t offpen = MAKE_ARGB(0xff, 0x20, 0x20, 0x20);

	// sizes for computation
	int bmwidth = 250;
	int bmheight = 400;
	int segwidth = 40;
	int skewwidth = 40;

	// allocate a temporary bitmap for drawing
	bitmap_t *tempbitmap = global_alloc(bitmap_t(bmwidth + skewwidth, bmheight, BITMAP_FORMAT_ARGB32));
	bitmap_fill(tempbitmap, NULL, MAKE_ARGB(0xff, 0x00, 0x00, 0x00));

	// top-left bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, 0 + segwidth/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 0)) ? onpen : offpen);

	// top-right bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, 0 + segwidth/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 1)) ? onpen : offpen);

	// right-top bar
	draw_segment_vertical(*tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 2)) ? onpen : offpen);

	// right-bottom bar
	draw_segment_vertical(*tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 3)) ? onpen : offpen);

	// bottom-right bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 4)) ? onpen : offpen);

	// bottom-left bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight - segwidth/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 5)) ? onpen : offpen);

	// left-bottom bar
	draw_segment_vertical(*tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 6)) ? onpen : offpen);

	// left-top bar
	draw_segment_vertical(*tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 7)) ? onpen : offpen);

	// horizontal-middle-left bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 8)) ? onpen : offpen);

	// horizontal-middle-right bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 9)) ? onpen : offpen);

	// vertical-middle-top bar
	draw_segment_vertical_caps(*tempbitmap,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 10)) ? onpen : offpen);

	// vertical-middle-bottom bar
	draw_segment_vertical_caps(*tempbitmap,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 11)) ? onpen : offpen);

	// diagonal-left-bottom bar
	draw_segment_diagonal_1(*tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 12)) ? onpen : offpen);

	// diagonal-left-top bar
	draw_segment_diagonal_2(*tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 13)) ? onpen : offpen);

	// diagonal-right-top bar
	draw_segment_diagonal_1(*tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 14)) ? onpen : offpen);

	// diagonal-right-bottom bar
	draw_segment_diagonal_2(*tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 15)) ? onpen : offpen);

	// apply skew
	apply_skew(*tempbitmap, 40);

	// resample to the target size
	render_resample_argb_bitmap_hq(dest.base, dest.rowpixels, dest.width, dest.height, tempbitmap, NULL, &m_color);

	global_free(tempbitmap);
}


//-------------------------------------------------
//  draw_led16segsc - draw a 16-segment LCD with
//  semicolon (2 extra segments)
//-------------------------------------------------

void layout_element::component::draw_led16segsc(bitmap_t &dest, const rectangle &bounds, int pattern)
{
	const rgb_t onpen = MAKE_ARGB(0xff, 0xff, 0xff, 0xff);
	const rgb_t offpen = MAKE_ARGB(0xff, 0x20, 0x20, 0x20);

	// sizes for computation
	int bmwidth = 250;
	int bmheight = 400;
	int segwidth = 40;
	int skewwidth = 40;

	// allocate a temporary bitmap for drawing
	bitmap_t *tempbitmap = global_alloc(bitmap_t(bmwidth + skewwidth, bmheight + segwidth, BITMAP_FORMAT_ARGB32));
	bitmap_fill(tempbitmap, NULL, MAKE_ARGB(0xff, 0x00, 0x00, 0x00));

	// top-left bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, 0 + segwidth/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 0)) ? onpen : offpen);

	// top-right bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, 0 + segwidth/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 1)) ? onpen : offpen);

	// right-top bar
	draw_segment_vertical(*tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 2)) ? onpen : offpen);

	// right-bottom bar
	draw_segment_vertical(*tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, bmwidth - segwidth/2,
		segwidth, (pattern & (1 << 3)) ? onpen : offpen);

	// bottom-right bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight - segwidth/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 4)) ? onpen : offpen);

	// bottom-left bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight - segwidth/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 5)) ? onpen : offpen);

	// left-bottom bar
	draw_segment_vertical(*tempbitmap,
		bmheight/2 + segwidth/3, bmheight - 2*segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 6)) ? onpen : offpen);

	// left-top bar
	draw_segment_vertical(*tempbitmap,
		0 + 2*segwidth/3, bmheight/2 - segwidth/3, 0 + segwidth/2,
		segwidth, (pattern & (1 << 7)) ? onpen : offpen);

	// horizontal-middle-left bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + 2*segwidth/3, bmwidth/2 - segwidth/10, bmheight/2,
		segwidth, LINE_CAP_START, (pattern & (1 << 8)) ? onpen : offpen);

	// horizontal-middle-right bar
	draw_segment_horizontal_caps(*tempbitmap,
		0 + bmwidth/2 + segwidth/10, bmwidth - 2*segwidth/3, bmheight/2,
		segwidth, LINE_CAP_END, (pattern & (1 << 9)) ? onpen : offpen);

	// vertical-middle-top bar
	draw_segment_vertical_caps(*tempbitmap,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 10)) ? onpen : offpen);

	// vertical-middle-bottom bar
	draw_segment_vertical_caps(*tempbitmap,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3, bmwidth/2,
		segwidth, LINE_CAP_NONE, (pattern & (1 << 11)) ? onpen : offpen);

	// diagonal-left-bottom bar
	draw_segment_diagonal_1(*tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 12)) ? onpen : offpen);

	// diagonal-left-top bar
	draw_segment_diagonal_2(*tempbitmap,
		0 + segwidth + segwidth/5, bmwidth/2 - segwidth/2 - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 13)) ? onpen : offpen);

	// diagonal-right-top bar
	draw_segment_diagonal_1(*tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		0 + segwidth + segwidth/3, bmheight/2 - segwidth/2 - segwidth/3,
		segwidth, (pattern & (1 << 14)) ? onpen : offpen);

	// diagonal-right-bottom bar
	draw_segment_diagonal_2(*tempbitmap,
		bmwidth/2 + segwidth/2 + segwidth/5, bmwidth - segwidth - segwidth/5,
		bmheight/2 + segwidth/2 + segwidth/3, bmheight - segwidth - segwidth/3,
		segwidth, (pattern & (1 << 15)) ? onpen : offpen);

	// decimal point
	draw_segment_decimal(*tempbitmap, bmwidth + segwidth/2, bmheight - segwidth/2, segwidth, (pattern & (1 << 16)) ? onpen : offpen);

	// comma tail
	draw_segment_diagonal_1(*tempbitmap,
		bmwidth - (segwidth/2), bmwidth + segwidth,
		bmheight - (segwidth), bmheight + segwidth*1.5,
		segwidth/2, (pattern & (1 << 17)) ? onpen : offpen);

	// apply skew
	apply_skew(*tempbitmap, 40);

	// resample to the target size
	render_resample_argb_bitmap_hq(dest.base, dest.rowpixels, dest.width, dest.height, tempbitmap, NULL, &m_color);

	global_free(tempbitmap);
}


//-------------------------------------------------
//  draw_dotmatrix - draw a row of 8 dots for a
//  dotmatrix
//-------------------------------------------------

void layout_element::component::draw_dotmatrix(bitmap_t &dest, const rectangle &bounds, int pattern)
{
	const rgb_t onpen = MAKE_ARGB(0xff, 0xff, 0xff, 0xff);
	const rgb_t offpen = MAKE_ARGB(0xff, 0x20, 0x20, 0x20);

	// sizes for computation
	int bmwidth = 2000;
	int bmheight = 300;
	int dotwidth = 250;

	// allocate a temporary bitmap for drawing
	bitmap_t *tempbitmap = global_alloc(bitmap_t(bmwidth, bmheight, BITMAP_FORMAT_ARGB32));
	bitmap_fill(tempbitmap, NULL, MAKE_ARGB(0xff, 0x00, 0x00, 0x00));

	for (int i = 0; i < 8; i++)
		draw_segment_decimal(*tempbitmap, ((dotwidth/2 )+ (i * dotwidth)), bmheight/2, dotwidth, (pattern & (1 << i))?onpen:offpen);

	// resample to the target size
	render_resample_argb_bitmap_hq(dest.base, dest.rowpixels, dest.width, dest.height, tempbitmap, NULL, &m_color);

	global_free(tempbitmap);
}


//-------------------------------------------------
//  draw_segment_horizontal_caps - draw a
//  horizontal LED segment with definable end
//  and start points
//-------------------------------------------------

void layout_element::component::draw_segment_horizontal_caps(bitmap_t &dest, int minx, int maxx, int midy, int width, int caps, rgb_t color)
{
	// loop over the width of the segment
	for (int y = 0; y < width / 2; y++)
	{
		UINT32 *d0 = BITMAP_ADDR32(&dest, midy - y, 0);
		UINT32 *d1 = BITMAP_ADDR32(&dest, midy + y, 0);
		int ty = (y < width / 8) ? width / 8 : y;

		// loop over the length of the segment
		for (int x = minx + ((caps & LINE_CAP_START) ? ty : 0); x < maxx - ((caps & LINE_CAP_END) ? ty : 0); x++)
			d0[x] = d1[x] = color;
	}
}


//-------------------------------------------------
//  draw_segment_horizontal - draw a horizontal
//  LED segment
//-------------------------------------------------

void layout_element::component::draw_segment_horizontal(bitmap_t &dest, int minx, int maxx, int midy, int width, rgb_t color)
{
	draw_segment_horizontal_caps(dest, minx, maxx, midy, width, LINE_CAP_START | LINE_CAP_END, color);
}


//-------------------------------------------------
//  draw_segment_vertical_caps - draw a
//  vertical LED segment with definable end
//  and start points
//-------------------------------------------------

void layout_element::component::draw_segment_vertical_caps(bitmap_t &dest, int miny, int maxy, int midx, int width, int caps, rgb_t color)
{
	// loop over the width of the segment
	for (int x = 0; x < width / 2; x++)
	{
		UINT32 *d0 = BITMAP_ADDR32(&dest, 0, midx - x);
		UINT32 *d1 = BITMAP_ADDR32(&dest, 0, midx + x);
		int tx = (x < width / 8) ? width / 8 : x;

		// loop over the length of the segment
		for (int y = miny + ((caps & LINE_CAP_START) ? tx : 0); y < maxy - ((caps & LINE_CAP_END) ? tx : 0); y++)
			d0[y * dest.rowpixels] = d1[y * dest.rowpixels] = color;
	}
}


//-------------------------------------------------
//  draw_segment_vertical - draw a vertical
//  LED segment
//-------------------------------------------------

void layout_element::component::draw_segment_vertical(bitmap_t &dest, int miny, int maxy, int midx, int width, rgb_t color)
{
	draw_segment_vertical_caps(dest, miny, maxy, midx, width, LINE_CAP_START | LINE_CAP_END, color);
}


//-------------------------------------------------
//  draw_segment_diagonal_1 - draw a diagonal
//  LED segment that looks like this: /
//-------------------------------------------------

void layout_element::component::draw_segment_diagonal_1(bitmap_t &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color)
{
	// compute parameters
	width *= 1.5;
	float ratio = (maxy - miny - width) / (float)(maxx - minx);

	// draw line
	for (int x = minx; x < maxx; x++)
		if (x >= 0 && x < dest.width)
		{
			UINT32 *d = BITMAP_ADDR32(&dest, 0, x);
			int step = (x - minx) * ratio;

			for (int y = maxy - width - step; y < maxy - step; y++)
				if (y >= 0 && y < dest.height)
					d[y * dest.rowpixels] = color;
		}
}


//-------------------------------------------------
//  draw_segment_diagonal_2 - draw a diagonal
//  LED segment that looks like this:
//-------------------------------------------------

void layout_element::component::draw_segment_diagonal_2(bitmap_t &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color)
{
	// compute parameters
	width *= 1.5;
	float ratio = (maxy - miny - width) / (float)(maxx - minx);

	// draw line
	for (int x = minx; x < maxx; x++)
		if (x >= 0 && x < dest.width)
		{
			UINT32 *d = BITMAP_ADDR32(&dest, 0, x);
			int step = (x - minx) * ratio;

			for (int y = miny + step; y < miny + step + width; y++)
				if (y >= 0 && y < dest.height)
					d[y * dest.rowpixels] = color;
		}
}


//-------------------------------------------------
//  draw_segment_decimal - draw a decimal point
//-------------------------------------------------

void layout_element::component::draw_segment_decimal(bitmap_t &dest, int midx, int midy, int width, rgb_t color)
{
	// compute parameters
	width /= 2;
	float ooradius2 = 1.0f / (float)(width * width);

	// iterate over y
	for (UINT32 y = 0; y <= width; y++)
	{
		UINT32 *d0 = BITMAP_ADDR32(&dest, midy - y, 0);
		UINT32 *d1 = BITMAP_ADDR32(&dest, midy + y, 0);
		float xval = width * sqrt(1.0f - (float)(y * y) * ooradius2);
		INT32 left, right;

		// compute left/right coordinates
		left = midx - (INT32)(xval + 0.5f);
		right = midx + (INT32)(xval + 0.5f);

		// draw this scanline
		for (UINT32 x = left; x < right; x++)
			d0[x] = d1[x] = color;
	}
}


//-------------------------------------------------
//  draw_segment_comma - draw a comma tail
//-------------------------------------------------

void layout_element::component::draw_segment_comma(bitmap_t &dest, int minx, int maxx, int miny, int maxy, int width, rgb_t color)
{
	// compute parameters
	width *= 1.5;
	float ratio = (maxy - miny - width) / (float)(maxx - minx);

	// draw line
	for (int x = minx; x < maxx; x++)
	{
		UINT32 *d = BITMAP_ADDR32(&dest, 0, x);
		int step = (x - minx) * ratio;

		for (int y = maxy; y < maxy  - width - step; y--)
			d[y * dest.rowpixels] = color;
	}
}


//-------------------------------------------------
//  apply_skew - apply skew to a bitmap_t
//-------------------------------------------------

void layout_element::component::apply_skew(bitmap_t &dest, int skewwidth)
{
	for (int y = 0; y < dest.height; y++)
	{
		UINT32 *destrow = BITMAP_ADDR32(&dest, y, 0);
		int offs = skewwidth * (dest.height - y) / dest.height;
		for (int x = dest.width - skewwidth - 1; x >= 0; x--)
			destrow[x + offs] = destrow[x];
		for (int x = 0; x < offs; x++)
			destrow[x] = 0;
	}
}



//**************************************************************************
//  LAYOUT VIEW
//**************************************************************************

//-------------------------------------------------
//  layout_view - constructor
//-------------------------------------------------

layout_view::layout_view(running_machine &machine, xml_data_node &viewnode, simple_list<layout_element> &elemlist)
	: m_next(NULL),
	  m_aspect(1.0f),
	  m_scraspect(1.0f),
	  m_screens(machine.m_respool),
	  m_backdrop_list(machine.m_respool),
	  m_screen_list(machine.m_respool),
	  m_overlay_list(machine.m_respool),
	  m_bezel_list(machine.m_respool)
{
	// allocate a copy of the name
	m_name = xml_get_attribute_string_with_subst(machine, viewnode, "name", "");

	// if we have a bounds item, load it
	xml_data_node *boundsnode = xml_get_sibling(viewnode.child, "bounds");
	m_expbounds.x0 = m_expbounds.y0 = m_expbounds.x1 = m_expbounds.y1 = 0;
	if (boundsnode != NULL)
		parse_bounds(machine, xml_get_sibling(boundsnode, "bounds"), m_expbounds);

	// load backdrop items
	for (xml_data_node *itemnode = xml_get_sibling(viewnode.child, "backdrop"); itemnode != NULL; itemnode = xml_get_sibling(itemnode->next, "backdrop"))
		m_backdrop_list.append(*auto_alloc(&machine, item(machine, *itemnode, elemlist)));

	// load screen items
	for (xml_data_node *itemnode = xml_get_sibling(viewnode.child, "screen"); itemnode != NULL; itemnode = xml_get_sibling(itemnode->next, "screen"))
		m_screen_list.append(*auto_alloc(&machine, item(machine, *itemnode, elemlist)));

	// load overlay items
	for (xml_data_node *itemnode = xml_get_sibling(viewnode.child, "overlay"); itemnode != NULL; itemnode = xml_get_sibling(itemnode->next, "overlay"))
		m_overlay_list.append(*auto_alloc(&machine, item(machine, *itemnode, elemlist)));

	// load bezel items
	for (xml_data_node *itemnode = xml_get_sibling(viewnode.child, "bezel"); itemnode != NULL; itemnode = xml_get_sibling(itemnode->next, "bezel"))
		m_bezel_list.append(*auto_alloc(&machine, item(machine, *itemnode, elemlist)));

	// recompute the data for the view based on a default layer config
	recompute(render_layer_config());
}


//-------------------------------------------------
//  layout_view - destructor
//-------------------------------------------------

layout_view::~layout_view()
{
}


//-------------------------------------------------
//  first_item - return the first item in the
//  appropriate list
//-------------------------------------------------

layout_view::item *layout_view::first_item(item_layer layer) const
{
	switch (layer)
	{
		case ITEM_LAYER_BACKDROP:	return m_backdrop_list.first();
		case ITEM_LAYER_SCREEN:		return m_screen_list.first();
		case ITEM_LAYER_OVERLAY:	return m_overlay_list.first();
		case ITEM_LAYER_BEZEL:		return m_bezel_list.first();
		default:					return NULL;
	}
}


//-------------------------------------------------
//  recompute - recompute the bounds and aspect
//  ratio of a view and all of its contained items
//-------------------------------------------------

void layout_view::recompute(render_layer_config layerconfig)
{
	// reset the bounds
	m_bounds.x0 = m_bounds.y0 = m_bounds.x1 = m_bounds.y1 = 0.0f;
	m_scrbounds.x0 = m_scrbounds.y0 = m_scrbounds.x1 = m_scrbounds.y1 = 0.0f;
	m_screens.reset();

	// loop over all layers
	bool first = true;
	bool scrfirst = true;
	for (item_layer layer = ITEM_LAYER_FIRST; layer < ITEM_LAYER_MAX; layer++)
	{
		// determine if this layer should be visible
		switch (layer)
		{
			case ITEM_LAYER_BACKDROP:	m_layenabled[layer] = layerconfig.backdrops_enabled();	break;
			case ITEM_LAYER_OVERLAY:	m_layenabled[layer] = layerconfig.overlays_enabled();	break;
			case ITEM_LAYER_BEZEL:		m_layenabled[layer] = layerconfig.bezels_enabled();		break;
			default:					m_layenabled[layer] = true;								break;
		}

		// only do it if requested
		if (m_layenabled[layer])
			for (item *curitem = first_item(layer); curitem != NULL; curitem = curitem->next())
			{
				// accumulate bounds
				if (first)
					m_bounds = curitem->m_rawbounds;
				else
					union_render_bounds(&m_bounds, &curitem->m_rawbounds);
				first = false;

				// accumulate screen bounds
				if (curitem->m_screen != NULL)
				{
					if (scrfirst)
						m_scrbounds = curitem->m_rawbounds;
					else
						union_render_bounds(&m_scrbounds, &curitem->m_rawbounds);
					scrfirst = false;

					// accumulate the screens in use while we're scanning
					m_screens.add(*curitem->m_screen);
				}
			}
	}

	// if we have an explicit bounds, override it
	if (m_expbounds.x1 > m_expbounds.x0)
		m_bounds = m_expbounds;

	// compute the aspect ratio of the view
	m_aspect = (m_bounds.x1 - m_bounds.x0) / (m_bounds.y1 - m_bounds.y0);
	m_scraspect = (m_scrbounds.x1 - m_scrbounds.x0) / (m_scrbounds.y1 - m_scrbounds.y0);

	// if we're handling things normally, the target bounds are (0,0)-(1,1)
	render_bounds target_bounds;
	if (!layerconfig.zoom_to_screen() || m_screens.count() == 0)
	{
		target_bounds.x0 = target_bounds.y0 = 0.0f;
		target_bounds.x1 = target_bounds.y1 = 1.0f;
	}

	// if we're cropping, we want the screen area to fill (0,0)-(1,1)
	else
	{
		float targwidth = (m_bounds.x1 - m_bounds.x0) / (m_scrbounds.x1 - m_scrbounds.x0);
		float targheight = (m_bounds.y1 - m_bounds.y0) / (m_scrbounds.y1 - m_scrbounds.y0);
		target_bounds.x0 = (m_bounds.x0 - m_scrbounds.x0) / (m_bounds.x1 - m_bounds.x0) * targwidth;
		target_bounds.y0 = (m_bounds.y0 - m_scrbounds.y0) / (m_bounds.y1 - m_bounds.y0) * targheight;
		target_bounds.x1 = target_bounds.x0 + targwidth;
		target_bounds.y1 = target_bounds.y0 + targheight;
	}

	// determine the scale/offset for normalization
	float xoffs = m_bounds.x0;
	float yoffs = m_bounds.y0;
	float xscale = (target_bounds.x1 - target_bounds.x0) / (m_bounds.x1 - m_bounds.x0);
	float yscale = (target_bounds.y1 - target_bounds.y0) / (m_bounds.y1 - m_bounds.y0);

	// normalize all the item bounds
	for (item_layer layer = ITEM_LAYER_FIRST; layer < ITEM_LAYER_MAX; layer++)
		for (item *curitem = first_item(layer); curitem != NULL; curitem = curitem->next())
		{
			curitem->m_bounds.x0 = target_bounds.x0 + (curitem->m_rawbounds.x0 - xoffs) * xscale;
			curitem->m_bounds.x1 = target_bounds.x0 + (curitem->m_rawbounds.x1 - xoffs) * xscale;
			curitem->m_bounds.y0 = target_bounds.y0 + (curitem->m_rawbounds.y0 - yoffs) * yscale;
			curitem->m_bounds.y1 = target_bounds.y0 + (curitem->m_rawbounds.y1 - yoffs) * yscale;
		}
}



//**************************************************************************
//  LAYOUT VIEW ITEM
//**************************************************************************

//-------------------------------------------------
//  item - constructor
//-------------------------------------------------

layout_view::item::item(running_machine &machine, xml_data_node &itemnode, simple_list<layout_element> &elemlist)
	: m_next(NULL),
	  m_element(NULL),
	  m_input_mask(0),
	  m_screen(NULL),
	  m_orientation(ROT0)
{
	// allocate a copy of the output name
	m_output_name = xml_get_attribute_string_with_subst(machine, itemnode, "name", "");

	// allocate a copy of the input tag
	m_input_tag = xml_get_attribute_string_with_subst(machine, itemnode, "inputtag", "");

	// find the associated element
	const char *name = xml_get_attribute_string_with_subst(machine, itemnode, "element", NULL);
	if (name != NULL)
	{
		// search the list of elements for a match
		for (m_element = elemlist.first(); m_element != NULL; m_element = m_element->next())
			if (strcmp(name, m_element->name()) == 0)
				break;

		// error if not found
		if (m_element == NULL)
			throw emu_fatalerror("Unable to find layout element %s", name);
	}

	// fetch common data
	int index = xml_get_attribute_int_with_subst(machine, itemnode, "index", -1);
	if (index != -1)
		m_screen = downcast<screen_device *>(machine.m_devicelist.find(SCREEN, index));
	m_input_mask = xml_get_attribute_int_with_subst(machine, itemnode, "inputmask", 0);
	if (m_output_name[0] != 0 && m_element != NULL)
		output_set_value(m_output_name, m_element->default_state());
	parse_bounds(machine, xml_get_sibling(itemnode.child, "bounds"), m_rawbounds);
	parse_color(machine, xml_get_sibling(itemnode.child, "color"), m_color);
	parse_orientation(machine, xml_get_sibling(itemnode.child, "orientation"), m_orientation);

	// sanity checks
	if (strcmp(itemnode.name, "screen") == 0)
	{
		if (m_screen == NULL)
			throw emu_fatalerror("Layout references invalid screen index %d", index);
	}
	else
	{
		if (m_element == NULL)
			throw emu_fatalerror("Layout item of type %s require an element tag", itemnode.name);
	}
}


//-------------------------------------------------
//  item - destructor
//-------------------------------------------------

layout_view::item::~item()
{
}


//-------------------------------------------------
//  state - fetch state based on configured source
//-------------------------------------------------

int layout_view::item::state() const
{
	int state = 0;

	assert(m_element != NULL);

	// if configured to an output, fetch the output value
	if (m_output_name[0] != 0)
		state = output_get_value(m_output_name);

	// if configured to an input, fetch the input value
	else if (m_input_tag[0] != 0)
	{
		const input_field_config *field = input_field_by_tag_and_mask(m_element->machine().m_portlist, m_input_tag, m_input_mask);
		if (field != NULL)
			state = ((input_port_read_safe(&m_element->machine(), m_input_tag, 0) ^ field->defvalue) & m_input_mask) ? 1 : 0;
	}
	return state;
}



//**************************************************************************
//  LAYOUT FILE
//**************************************************************************

//-------------------------------------------------
//  layout_file - constructor
//-------------------------------------------------

layout_file::layout_file(running_machine &machine, xml_data_node &rootnode, const char *dirname)
	: m_next(NULL),
	  m_elemlist(machine.m_respool),
	  m_viewlist(machine.m_respool)
{
	// find the layout node
	xml_data_node *mamelayoutnode = xml_get_sibling(rootnode.child, "mamelayout");
	if (mamelayoutnode == NULL)
		throw emu_fatalerror("Invalid XML file: missing mamelayout node");

	// validate the config data version
	int version = xml_get_attribute_int(mamelayoutnode, "version", 0);
	if (version != LAYOUT_VERSION)
		throw emu_fatalerror("Invalid XML file: unsupported version");

	// parse all the elements
	for (xml_data_node *elemnode = xml_get_sibling(mamelayoutnode->child, "element"); elemnode != NULL; elemnode = xml_get_sibling(elemnode->next, "element"))
		m_elemlist.append(*auto_alloc(&machine, layout_element(machine, *elemnode, dirname)));

	// parse all the views
	for (xml_data_node *viewnode = xml_get_sibling(mamelayoutnode->child, "view"); viewnode != NULL; viewnode = xml_get_sibling(viewnode->next, "view"))
		m_viewlist.append(*auto_alloc(&machine, layout_view(machine, *viewnode, m_elemlist)));
}


//-------------------------------------------------
//  ~layout_file - destructor
//-------------------------------------------------

layout_file::~layout_file()
{
}
