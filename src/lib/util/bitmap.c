/***************************************************************************

    bitmap.c

    Core bitmap routines.

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

#include "bitmap.h"

#include <new>



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// alignment values; 128 bytes is the largest cache line on typical
// architectures today
const UINT32 BITMAP_OVERALL_ALIGN = 128;
const UINT32 BITMAP_ROWBYTES_ALIGN = 128;



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  compute_rowpixels - compute an aligned
//  rowpixels value
//-------------------------------------------------

inline INT32 bitmap_t::compute_rowpixels(int width, int xslop)
{
	int rowpixels_align = BITMAP_ROWBYTES_ALIGN / (m_bpp / 8);
	return ((width + 2 * xslop + (rowpixels_align - 1)) / rowpixels_align) * rowpixels_align;
}


//-------------------------------------------------
//  compute_base - compute an aligned bitmap base
//  address with the given slop values
//-------------------------------------------------

inline void bitmap_t::compute_base(int xslop, int yslop)
{
	m_base = m_alloc + (m_rowpixels * yslop + xslop) * (m_bpp / 8);
	UINT64 aligned_base = ((reinterpret_cast<UINT64>(m_base) + (BITMAP_OVERALL_ALIGN - 1)) / BITMAP_OVERALL_ALIGN) * BITMAP_OVERALL_ALIGN;
	m_base = reinterpret_cast<void *>(aligned_base);
}



//**************************************************************************
//  BITMAP ALLOCATION/CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  bitmap_t - basic constructor
//-------------------------------------------------

bitmap_t::bitmap_t(bitmap_format format, int bpp, int width, int height, int xslop, int yslop)
	: m_alloc(NULL),
	  m_allocbytes(0),
	  m_format(format),
	  m_bpp(bpp),
	  m_palette(NULL)
{
	// allocate intializes all other fields
	allocate(width, height, xslop, yslop);
}


bitmap_t::bitmap_t(bitmap_format format, int bpp, void *base, int width, int height, int rowpixels)
	: m_alloc(NULL),
	  m_allocbytes(0),
	  m_base(base),
	  m_rowpixels(rowpixels),
	  m_width(width),
	  m_height(height),
	  m_format(format),
	  m_bpp(bpp),
	  m_palette(NULL),
	  m_cliprect(0, width - 1, 0, height - 1)
{
}


bitmap_t::bitmap_t(bitmap_format format, int bpp, bitmap_t &source, const rectangle &subrect)
	: m_alloc(NULL),
	  m_allocbytes(0),
	  m_base(source.raw_pixptr(subrect.min_y, subrect.min_x)),
	  m_rowpixels(source.m_rowpixels),
	  m_width(subrect.width()),
	  m_height(subrect.height()),
	  m_format(format),
	  m_bpp(bpp),
	  m_palette(NULL),
	  m_cliprect(0, subrect.width() - 1, 0, subrect.height() - 1)
{
	assert(format == source.m_format);
	assert(bpp == source.m_bpp);
	assert(source.cliprect().contains(subrect));
}


//-------------------------------------------------
//  ~bitmap_t - basic destructor
//-------------------------------------------------

bitmap_t::~bitmap_t()
{
	// delete any existing stuff
	reset();
}


//-------------------------------------------------
//  allocate -- (re)allocate memory for the bitmap
//  at the given size, destroying anything that
//  already exists
//-------------------------------------------------

void bitmap_t::allocate(int width, int height, int xslop, int yslop)
{
	assert(m_format != BITMAP_FORMAT_INVALID);
	assert(m_bpp == 8 || m_bpp == 16 || m_bpp == 32 || m_bpp == 64);

	// delete any existing stuff
	reset();

	// handle empty requests cleanly
	if (width <= 0 || height <= 0)
		return;

	// initialize fields
	m_rowpixels = compute_rowpixels(width, xslop);
	m_width = width;
	m_height = height;
	m_cliprect.set(0, width - 1, 0, height - 1);

	// allocate memory for the bitmap itself
	m_allocbytes = m_rowpixels * (m_height + 2 * yslop) * m_bpp / 8;
	m_allocbytes += BITMAP_OVERALL_ALIGN - 1;
	m_alloc = new UINT8[m_allocbytes];

	// clear to 0 by default
	memset(m_alloc, 0, m_allocbytes);

	// compute the base
	compute_base(xslop, yslop);
}


//-------------------------------------------------
//  resize -- resize a bitmap, reusing existing
//  memory if the new size is smaller than the
//  current size
//-------------------------------------------------

void bitmap_t::resize(int width, int height, int xslop, int yslop)
{
	assert(m_format != BITMAP_FORMAT_INVALID);
	assert(m_bpp == 8 || m_bpp == 16 || m_bpp == 32 || m_bpp == 64);

	// handle empty requests cleanly
	if (width <= 0 || height <= 0)
		width = height = 0;

	// determine how much memory we need for the new bitmap
	int new_rowpixels = compute_rowpixels(width, xslop);
	UINT32 new_allocbytes = new_rowpixels * (height + 2 * yslop) * m_bpp / 8;
	new_allocbytes += BITMAP_OVERALL_ALIGN - 1;

	// if we need more memory, just realloc
	if (new_allocbytes > m_allocbytes)
	{
		palette_t *palette = m_palette;
		allocate(width, height, xslop, yslop);
		set_palette(palette);
		return;
	}

	// otherwise, reconfigure
	m_rowpixels = new_rowpixels;
	m_width = width;
	m_height = height;
	m_cliprect.set(0, width - 1, 0, height - 1);

	// re-compute the base
	compute_base(xslop, yslop);
}


//-------------------------------------------------
//  reset -- reset to an invalid bitmap, deleting
//  all allocated stuff
//-------------------------------------------------

void bitmap_t::reset()
{
	// delete any existing stuff
	set_palette(NULL);
	delete[] m_alloc;
	m_alloc = NULL;
	m_base = NULL;

	// reset all fields
	m_rowpixels = 0;
	m_width = 0;
	m_height = 0;
	m_cliprect.set(0, -1, 0, -1);
}


//-------------------------------------------------
//  wrap -- wrap an array of memory; the target
//  bitmap does not own the memory
//-------------------------------------------------

void bitmap_t::wrap(void *base, int width, int height, int rowpixels)
{
	// delete any existing stuff
	reset();

	// initialize relevant fields
	m_base = base;
	m_rowpixels = rowpixels;
	m_width = width;
	m_height = height;
	m_cliprect.set(0, m_width - 1, 0, m_height - 1);
}


//-------------------------------------------------
//  wrap -- wrap a subrectangle of an existing
//  bitmap by copying its fields; the target
//  bitmap does not own the memory
//-------------------------------------------------

void bitmap_t::wrap(const bitmap_t &source, const rectangle &subrect)
{
	assert(m_format == source.m_format);
	assert(m_bpp == source.m_bpp);
	assert(source.cliprect().contains(subrect));

	// delete any existing stuff
	reset();

	// copy relevant fields
	m_base = source.raw_pixptr(subrect.min_y, subrect.min_x);
	m_rowpixels = source.m_rowpixels;
	m_width = subrect.width();
	m_height = subrect.height();
	set_palette(source.m_palette);
	m_cliprect.set(0, m_width - 1, 0, m_height - 1);
}


//-------------------------------------------------
//  set_palette -- associate a palette with a
//  bitmap
//-------------------------------------------------

void bitmap_t::set_palette(palette_t *palette)
{
	// first dereference any existing palette
	if (m_palette != NULL)
	{
		palette_deref(m_palette);
		m_palette = NULL;
	}

	// then reference any new palette
	if (palette != NULL)
	{
		palette_ref(palette);
		m_palette = palette;
	}
}


//-------------------------------------------------
//  fill -- fill a bitmap with a solid color
//-------------------------------------------------

void bitmap_t::fill(UINT32 color, const rectangle &cliprect)
{
	// if we have a cliprect, intersect with that
	rectangle fill = cliprect;
	fill &= m_cliprect;
	if (fill.empty())
		return;

	// based on the bpp go from there
	switch (m_bpp)
	{
		case 8:
			// 8bpp always uses memset
			for (INT32 y = fill.min_y; y <= fill.max_y; y++)
				memset(raw_pixptr(y, fill.min_x), (UINT8)color, fill.width());
			break;

		case 16:
			// 16bpp can use memset if the bytes are equal
			if ((UINT8)(color >> 8) == (UINT8)color)
			{
				for (INT32 y = fill.min_y; y <= fill.max_y; y++)
					memset(raw_pixptr(y, fill.min_x), (UINT8)color, fill.width() * 2);
			}
			else
			{
				// Fill the first line the hard way
				UINT16 *destrow = &pixt<UINT16>(fill.min_y);
				for (INT32 x = fill.min_x; x <= fill.max_x; x++)
					destrow[x] = (UINT16)color;

				// For the other lines, just copy the first one
				void *destrow0 = &pixt<UINT16>(fill.min_y, fill.min_x);
				for (INT32 y = fill.min_y + 1; y <= fill.max_y; y++)
				{
					destrow = &pixt<UINT16>(y, fill.min_x);
					memcpy(destrow, destrow0, fill.width() * 2);
				}
			}
			break;

		case 32:
			// 32bpp can use memset if the bytes are equal
			if ((UINT8)(color >> 8) == (UINT8)color && (UINT16)(color >> 16) == (UINT16)color)
			{
				for (INT32 y = fill.min_y; y <= fill.max_y; y++)
					memset(&pixt<UINT32>(y, fill.min_x), (UINT8)color, fill.width() * 4);
			}
			else
			{
				// Fill the first line the hard way
				UINT32 *destrow  = &pixt<UINT32>(fill.min_y);
				for (INT32 x = fill.min_x; x <= fill.max_x; x++)
					destrow[x] = (UINT32)color;

				// For the other lines, just copy the first one
				UINT32 *destrow0 = &pixt<UINT32>(fill.min_y, fill.min_x);
				for (INT32 y = fill.min_y + 1; y <= fill.max_y; y++)
				{
					destrow = &pixt<UINT32>(y, fill.min_x);
					memcpy(destrow, destrow0, fill.width() * 4);
				}
			}
			break;

		case 64:
			// 64bpp can use memset if the bytes are equal
			if ((UINT8)(color >> 8) == (UINT8)color && (UINT16)(color >> 16) == (UINT16)color)
			{
				for (INT32 y = fill.min_y; y <= fill.max_y; y++)
					memset(&pixt<UINT64>(y, fill.min_x), (UINT8)color, fill.width() * 8);
			}
			else
			{
				// Fill the first line the hard way
				UINT64 *destrow  = &pixt<UINT64>(fill.min_y);
				for (INT32 x = fill.min_x; x <= fill.max_x; x++)
					destrow[x] = (UINT64)color;

				// For the other lines, just copy the first one
				UINT64 *destrow0 = &pixt<UINT64>(fill.min_y, fill.min_x);
				for (INT32 y = fill.min_y + 1; y <= fill.max_y; y++)
				{
					destrow = &pixt<UINT64>(y, fill.min_x);
					memcpy(destrow, destrow0, fill.width() * 8);
				}
			}
			break;
	}
}
