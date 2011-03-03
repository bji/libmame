/***************************************************************************

    rendfont.c

    Rendering system font management.

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

#include "emu.h"
#include "rendfont.h"
#include "rendutil.h"
#include "emuopts.h"
#include "zlib.h"

#include "uismall.fh"


//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  next_line - return a pointer to the start of
//  the next line
//-------------------------------------------------

inline const char *next_line(const char *ptr)
{
	// scan forward until we hit the end or a carriage return
	while (*ptr != 13 && *ptr != 10 && *ptr != 0) ptr++;

	// if we hit the end, return NULL
	if (*ptr == 0)
		return NULL;

	// eat the trailing linefeed if present
	if (*++ptr == 10)
		ptr++;
	return ptr;
}


//-------------------------------------------------
//  get_char - return a pointer to a character
//  in a font, expanding if necessary
//-------------------------------------------------

inline render_font::glyph &render_font::get_char(unicode_char chnum)
{
	static glyph dummy_glyph;

	// grab the table; if none, return the dummy character
	glyph *glyphtable = m_glyphs[chnum / 256];
	if (glyphtable == NULL && m_format == FF_OSD)
		glyphtable = m_glyphs[chnum / 256] = auto_alloc_array_clear(&m_manager.machine(), glyph, 256);
	if (glyphtable == NULL)
		return dummy_glyph;

	// if the character isn't generated yet, do it now
	glyph &gl = glyphtable[chnum % 256];
	if (gl.bitmap == NULL)
		char_expand(chnum, gl);

	// return the resulting character
	return gl;
}



//**************************************************************************
//  RENDER FONT
//**************************************************************************

//-------------------------------------------------
//  render_font - constructor
//-------------------------------------------------

render_font::render_font(render_manager &manager, const char *filename)
	: m_manager(manager),
	  m_format(FF_UNKNOWN),
	  m_height(0),
	  m_yoffs(0),
	  m_scale(1.0f),
	  m_rawdata(NULL),
	  m_rawsize(0),
	  m_osdfont(NULL)
{
	memset(m_glyphs, 0, sizeof(m_glyphs));

	// if this is an OSD font, we're done
	if (filename != NULL)
	{
		m_osdfont = manager.machine().osd().font_open(filename, m_height);
		if (m_osdfont != NULL)
		{
			m_scale = 1.0f / (float)m_height;
			m_format = FF_OSD;
			return;
		}
	}

	// if the filename is 'default' default to 'ui.bdf' for backwards compatibility
	if (filename != NULL && mame_stricmp(filename, "default") == 0)
		filename = "ui.bdf";

	// attempt to load the cached version of the font first
	if (filename != NULL && load_cached_bdf(filename))
		return;

	// load the raw data instead
	emu_file ramfile(manager.machine().options(), NULL, OPEN_FLAG_READ);
	file_error filerr = ramfile.open_ram(font_uismall, sizeof(font_uismall));
	if (filerr == FILERR_NONE)
		load_cached(ramfile, 0);
}


//-------------------------------------------------
//  ~render_font - destructor
//-------------------------------------------------

render_font::~render_font()
{
	// free all the subtables
	for (int tablenum = 0; tablenum < 256; tablenum++)
		if (m_glyphs[tablenum] != NULL)
		{
			// loop over characters
			for (int charnum = 0; charnum < 256; charnum++)
			{
				glyph &gl = m_glyphs[tablenum][charnum];
				m_manager.texture_free(gl.texture);
				auto_free(&m_manager.machine(), gl.bitmap);
			}

			// free the subtable itself
			auto_free(&m_manager.machine(), m_glyphs[tablenum]);
		}

	// free the raw data and the size itself
	auto_free(&m_manager.machine(), m_rawdata);

	// release the OSD font
	if (m_osdfont != NULL)
		m_manager.machine().osd().font_close(m_osdfont);
}


//-------------------------------------------------
//  char_expand - expand the raw data for a
//  character into a bitmap
//-------------------------------------------------

void render_font::char_expand(unicode_char chnum, glyph &gl)
{
	// if we're an OSD font, query the info
	if (m_format == FF_OSD)
	{
		// we set bmwidth to -1 if we've previously queried and failed
		if (gl.bmwidth == -1)
			return;

		// attempt to get the font bitmap; if we fail, set bmwidth to -1
		gl.bitmap = m_manager.machine().osd().font_get_bitmap(m_osdfont, chnum, gl.width, gl.xoffs, gl.yoffs);
		if (gl.bitmap == NULL)
		{
			gl.bmwidth = -1;
			return;
		}

		// populate the bmwidth/bmheight fields
		gl.bmwidth = gl.bitmap->width;
		gl.bmheight = gl.bitmap->height;
	}

	// other formats need to parse their data
	else
	{
		// punt if nothing there
		if (gl.bmwidth == 0 || gl.bmheight == 0 || gl.rawdata == NULL)
			return;

		// allocate a new bitmap of the size we need
		gl.bitmap = auto_alloc(&m_manager.machine(), bitmap_t(gl.bmwidth, m_height, BITMAP_FORMAT_ARGB32));
		bitmap_fill(gl.bitmap, NULL, 0);

		// extract the data
		const char *ptr = gl.rawdata;
		UINT8 accum = 0, accumbit = 7;
		for (int y = 0; y < gl.bmheight; y++)
		{
			int desty = y + m_height + m_yoffs - gl.yoffs - gl.bmheight;
			UINT32 *dest = (desty >= 0 && desty < m_height) ? BITMAP_ADDR32(gl.bitmap, desty, 0) : NULL;

			// text format
			if (m_format == FF_TEXT)
			{
				// loop over bytes
				for (int x = 0; x < gl.bmwidth; x += 4)
				{
					// scan for the next hex digit
					int bits = -1;
					while (*ptr != 13 && bits == -1)
					{
						if (*ptr >= '0' && *ptr <= '9')
							bits = *ptr++ - '0';
						else if (*ptr >= 'A' && *ptr <= 'F')
							bits = *ptr++ - 'A' + 10;
						else if (*ptr >= 'a' && *ptr <= 'f')
							bits = *ptr++ - 'a' + 10;
						else
							ptr++;
					}

					// expand the four bits
					if (dest != NULL)
					{
						*dest++ = (bits & 8) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
						*dest++ = (bits & 4) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
						*dest++ = (bits & 2) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
						*dest++ = (bits & 1) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					}
				}

				// advance to the next line
				ptr = next_line(ptr);
			}

			// cached format
			else if (m_format == FF_CACHED)
			{
				for (int x = 0; x < gl.bmwidth; x++)
				{
					if (accumbit == 7)
						accum = *ptr++;
					if (dest != NULL)
						*dest++ = (accum & (1 << accumbit)) ? MAKE_ARGB(0xff,0xff,0xff,0xff) : MAKE_ARGB(0x00,0xff,0xff,0xff);
					accumbit = (accumbit - 1) & 7;
				}
			}
		}
	}

	// wrap a texture around the bitmap
	gl.texture = m_manager.texture_alloc(render_texture::hq_scale);
	gl.texture->set_bitmap(gl.bitmap, NULL, TEXFORMAT_ARGB32);
}


//-------------------------------------------------
//  get_char_texture_and_bounds - return the
//  texture for a character and compute the
//  bounds of the final bitmap
//-------------------------------------------------

render_texture *render_font::get_char_texture_and_bounds(float height, float aspect, unicode_char chnum, render_bounds &bounds)
{
	glyph &gl = get_char(chnum);

	// on entry, assume x0,y0 are the top,left coordinate of the cell and add
	// the character bounding box to that position
	float scale = m_scale * height;
	bounds.x0 += (float)gl.xoffs * scale * aspect;

	// compute x1,y1 from there based on the bitmap size
	bounds.x1 = bounds.x0 + (float)gl.bmwidth * scale * aspect;
	bounds.y1 = bounds.y0 + (float)m_height * scale;

	// return the texture
	return gl.texture;
}


//-------------------------------------------------
//  get_scaled_bitmap_and_bounds - return a
//  scaled bitmap and bounding rect for a char
//-------------------------------------------------

void render_font::get_scaled_bitmap_and_bounds(bitmap_t &dest, float height, float aspect, unicode_char chnum, rectangle &bounds)
{
	glyph &gl = get_char(chnum);

	// on entry, assume x0,y0 are the top,left coordinate of the cell and add
	// the character bounding box to that position
	float scale = m_scale * height;
	bounds.min_x = (float)gl.xoffs * scale * aspect;
	bounds.min_y = 0;

	// compute x1,y1 from there based on the bitmap size
	bounds.max_x = bounds.min_x + (float)gl.bmwidth * scale * aspect;
	bounds.max_y = bounds.min_y + (float)m_height * scale;

	// if the bitmap isn't big enough, bail
	if (dest.width < bounds.max_x - bounds.min_x || dest.height < bounds.max_y - bounds.min_y)
		return;

	// if no texture, fill the target
	if (gl.texture == NULL)
	{
		bitmap_fill(&dest, NULL, 0);
		return;
	}

	// scale the font
	INT32 origwidth = dest.width;
	INT32 origheight = dest.height;
	dest.width = bounds.max_x - bounds.min_x;
	dest.height = bounds.max_y - bounds.min_y;
	rectangle clip;
	clip.min_x = clip.min_y = 0;
	clip.max_x = gl.bitmap->width - 1;
	clip.max_y = gl.bitmap->height - 1;
	render_texture::hq_scale(dest, *gl.bitmap, clip, NULL);
	dest.width = origwidth;
	dest.height = origheight;
}


//-------------------------------------------------
//  char_width - return the width of a character
//  at the given height
//-------------------------------------------------

float render_font::char_width(float height, float aspect, unicode_char ch)
{
	return (float)get_char(ch).width * m_scale * height * aspect;
}


//-------------------------------------------------
//  string_width - return the width of a string
//  at the given height
//-------------------------------------------------

float render_font::string_width(float height, float aspect, const char *string)
{
	// loop over the string and accumulate widths
	int totwidth = 0;
	for (const unsigned char *ptr = (const unsigned char *)string; *ptr != 0; ptr++)
		totwidth += get_char(*ptr).width;

	// scale the final result based on height
	return (float)totwidth * m_scale * height * aspect;
}


//-------------------------------------------------
//  utf8string_width - return the width of a
//  UTF8-encoded string at the given height
//-------------------------------------------------

float render_font::utf8string_width(float height, float aspect, const char *utf8string)
{
	int length = strlen(utf8string);

	// loop over the string and accumulate widths
	int count;
	int totwidth = 0;
	for (int offset = 0; offset < length; offset += count)
	{
		unicode_char uchar;
		count = uchar_from_utf8(&uchar, utf8string + offset, length - offset);
		if (count == -1)
			break;
		if (uchar < 0x10000)
			totwidth += get_char(uchar).width;
	}

	// scale the final result based on height
	return (float)totwidth * m_scale * height * aspect;
}


//-------------------------------------------------
//  load_cached_bdf - attempt to load a cached
//  version of the BDF font 'filename'; if that
//  fails, fall back on the regular BDF loader
//  and create a new cached version
//-------------------------------------------------

bool render_font::load_cached_bdf(const char *filename)
{
	// first try to open the BDF itself
	emu_file file(manager().machine().options(), SEARCHPATH_FONT, OPEN_FLAG_READ);
	file_error filerr = file.open(filename);
	if (filerr != FILERR_NONE)
		return false;

	// determine the file size and allocate memory
	m_rawsize = file.size();
	char *data = auto_alloc_array_clear(&m_manager.machine(), char, m_rawsize + 1);

	// read the first chunk
	UINT32 bytes = file.read(data, MIN(CACHED_BDF_HASH_SIZE, m_rawsize));
	if (bytes != MIN(CACHED_BDF_HASH_SIZE, m_rawsize))
		return false;

	// has the chunk
	UINT32 hash = crc32(0, (const UINT8 *)data, bytes) ^ (UINT32)m_rawsize;

	// create the cached filename, changing the 'F' to a 'C' on the extension
	astring cachedname(filename);
	cachedname.del(cachedname.len() - 3, 3).cat("bdc");

	// attempt to open the cached version of the font
	{
		emu_file cachefile(manager().machine().options(), SEARCHPATH_FONT, OPEN_FLAG_READ);
		filerr = cachefile.open(cachedname);
		if (filerr == FILERR_NONE)
		{
			// if we have a cached version, load it
			bool result = load_cached(cachefile, hash);

			// if that worked, we're done
			if (result)
			{
				auto_free(&m_manager.machine(), data);
				return true;
			}
		}
	}

	// read in the rest of the font
	if (bytes < m_rawsize)
	{
		UINT32 read = file.read(data + bytes, m_rawsize - bytes);
		if (read != m_rawsize - bytes)
		{
			auto_free(&m_manager.machine(), data);
			return false;
		}
	}

	// NULL-terminate the data and attach it to the font
	data[m_rawsize] = 0;
	m_rawdata = data;

	// load the BDF
	bool result = load_bdf();

	// if we loaded okay, create a cached one
	if (result)
		save_cached(cachedname, hash);

	// close the file
	return result;
}


//-------------------------------------------------
//  load_bdf - parse and load a BDF font
//-------------------------------------------------

bool render_font::load_bdf()
{
	// set the format to text
	m_format = FF_TEXT;

	// first find the FONTBOUNDINGBOX tag
	const char *ptr;
	for (ptr = m_rawdata; ptr != NULL; ptr = next_line(ptr))
	{
		// we only care about a tiny few fields
		if (strncmp(ptr, "FONTBOUNDINGBOX ", 16) == 0)
		{
			int dummy1, dummy2;
			if (sscanf(ptr + 16, "%d %d %d %d", &dummy1, &m_height, &dummy2, &m_yoffs) != 4)
				return false;
			break;
		}
	}

	// compute the scale factor
	m_scale = 1.0f / (float)m_height;

	// now scan for characters
	int charcount = 0;
	for ( ; ptr != NULL; ptr = next_line(ptr))
	{
		// stop at ENDFONT
		if (strncmp(ptr, "ENDFONT", 7) == 0)
			break;

		// once we hit a STARTCHAR, parse until the end
		if (strncmp(ptr, "STARTCHAR ", 10) == 0)
		{
			int bmwidth = -1, bmheight = -1, xoffs = -1, yoffs = -1;
			const char *rawdata = NULL;
			int charnum = -1;
			int width = -1;

			// scan for interesting per-character tags
			for ( ; ptr != NULL; ptr = next_line(ptr))
			{
				// ENCODING tells us which character
				if (strncmp(ptr, "ENCODING ", 9) == 0)
				{
					if (sscanf(ptr + 9, "%d", &charnum) != 1)
						return 1;
				}

				// DWIDTH tells us the width to the next character
				else if (strncmp(ptr, "DWIDTH ", 7) == 0)
				{
					int dummy1;
					if (sscanf(ptr + 7, "%d %d", &width, &dummy1) != 2)
						return 1;
				}

				// BBX tells us the height/width of the bitmap and the offsets
				else if (strncmp(ptr, "BBX ", 4) == 0)
				{
					if (sscanf(ptr + 4, "%d %d %d %d", &bmwidth, &bmheight, &xoffs, &yoffs) != 4)
						return 1;
				}

				// BITMAP is the start of the data
				else if (strncmp(ptr, "BITMAP", 6) == 0)
				{
					// stash the raw pointer and scan for the end of the character
					for (rawdata = ptr = next_line(ptr); ptr != NULL && strncmp(ptr, "ENDCHAR", 7) != 0; ptr = next_line(ptr)) ;
					break;
				}
			}

			// if we have everything, allocate a new character
			if (charnum >= 0 && charnum < 65536 && rawdata != NULL && bmwidth >= 0 && bmheight >= 0)
			{
				// if we don't have a subtable yet, make one
				if (m_glyphs[charnum / 256] == NULL)
					m_glyphs[charnum / 256] = auto_alloc_array_clear(&m_manager.machine(), glyph, 256);

				// fill in the entry
				glyph &gl = m_glyphs[charnum / 256][charnum % 256];
				gl.width = width;
				gl.bmwidth = bmwidth;
				gl.bmheight = bmheight;
				gl.xoffs = xoffs;
				gl.yoffs = yoffs;
				gl.rawdata = rawdata;
			}

			// some progress for big fonts
			if (++charcount % 256 == 0)
				mame_printf_warning("Loading BDF font... (%d characters loaded)\n", charcount);
		}
	}

	// make sure all the numbers are the same width
	if (m_glyphs[0] != NULL)
	{
		int maxwidth = 0;
		for (int ch = '0'; ch <= '9'; ch++)
			if (m_glyphs[0][ch].bmwidth > maxwidth)
				maxwidth = m_glyphs[0][ch].width;
		for (int ch = '0'; ch <= '9'; ch++)
			m_glyphs[0][ch].width = maxwidth;
	}

	return true;
}


//-------------------------------------------------
//  load_cached - load a font in cached format
//-------------------------------------------------

bool render_font::load_cached(emu_file &file, UINT32 hash)
{
	// get the file size
	UINT64 filesize = file.size();

	// first read the header
	UINT8 header[CACHED_HEADER_SIZE];
	UINT32 bytes_read = file.read(header, CACHED_HEADER_SIZE);
	if (bytes_read != CACHED_HEADER_SIZE)
		return false;

	// validate the header
	if (header[0] != 'f' || header[1] != 'o' || header[2] != 'n' || header[3] != 't')
		return false;
	if (header[4] != (UINT8)(hash >> 24) || header[5] != (UINT8)(hash >> 16) || header[6] != (UINT8)(hash >> 8) || header[7] != (UINT8)hash)
		return false;
	m_height = (header[8] << 8) | header[9];
	m_scale = 1.0f / (float)m_height;
	m_yoffs = (INT16)((header[10] << 8) | header[11]);
	int numchars = (header[12] << 24) | (header[13] << 16) | (header[14] << 8) | header[15];
	if (filesize - CACHED_HEADER_SIZE < numchars * CACHED_CHAR_SIZE)
		return false;

	// now read the rest of the data
	UINT8 *data = auto_alloc_array(&m_manager.machine(), UINT8, filesize - CACHED_HEADER_SIZE);
	bytes_read = file.read(data, filesize - CACHED_HEADER_SIZE);
	if (bytes_read != filesize - CACHED_HEADER_SIZE)
	{
		auto_free(&m_manager.machine(), data);
		return false;
	}

	// extract the data from the data
	UINT64 offset = numchars * CACHED_CHAR_SIZE;
	for (int chindex = 0; chindex < numchars; chindex++)
	{
		const UINT8 *info = &data[chindex * CACHED_CHAR_SIZE];
		int chnum = (info[0] << 8) | info[1];

		// if we don't have a subtable yet, make one
		if (m_glyphs[chnum / 256] == NULL)
			m_glyphs[chnum / 256] = auto_alloc_array_clear(&m_manager.machine(), glyph, 256);

		// fill in the entry
		glyph &gl = m_glyphs[chnum / 256][chnum % 256];
		gl.width = (info[2] << 8) | info[3];
		gl.xoffs = (INT16)((info[4] << 8) | info[5]);
		gl.yoffs = (INT16)((info[6] << 8) | info[7]);
		gl.bmwidth = (info[8] << 8) | info[9];
		gl.bmheight = (info[10] << 8) | info[11];
		gl.rawdata = (char *)data + offset;

		// advance the offset past the character
		offset += (gl.bmwidth * gl.bmheight + 7) / 8;
		if (offset > filesize - CACHED_HEADER_SIZE)
		{
			auto_free(&m_manager.machine(), data);
			return false;
		}
	}

	// reuse the chartable as a temporary buffer
	m_format = FF_CACHED;
	m_rawdata = (char *)data;
	return true;
}


//-------------------------------------------------
//  save_cached - save a font in cached format
//-------------------------------------------------

bool render_font::save_cached(const char *filename, UINT32 hash)
{
	mame_printf_warning("Generating cached BDF font...\n");

	// attempt to open the file
	emu_file file(manager().machine().options(), SEARCHPATH_FONT, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);
	file_error filerr = file.open(filename);
	if (filerr != FILERR_NONE)
		return false;

	// determine the number of characters
	int numchars = 0;
	for (int chnum = 0; chnum < 65536; chnum++)
	{
		glyph *chtable = m_glyphs[chnum / 256];
		if (chtable != NULL)
		{
			glyph &gl = chtable[chnum % 256];
			if (gl.width > 0)
				numchars++;
		}
	}

	UINT8 *chartable = NULL;
	UINT8 *tempbuffer = NULL;
	try
	{
		// allocate an array to hold the character data
		UINT8 *chartable = auto_alloc_array_clear(&m_manager.machine(), UINT8, numchars * CACHED_CHAR_SIZE);

		// allocate a temp buffer to compress into
		UINT8 *tempbuffer = auto_alloc_array(&m_manager.machine(), UINT8, 65536);

		// write the header
		UINT8 *dest = tempbuffer;
		*dest++ = 'f';
		*dest++ = 'o';
		*dest++ = 'n';
		*dest++ = 't';
		*dest++ = hash >> 24;
		*dest++ = hash >> 16;
		*dest++ = hash >> 8;
		*dest++ = hash & 0xff;
		*dest++ = m_height >> 8;
		*dest++ = m_height & 0xff;
		*dest++ = m_yoffs >> 8;
		*dest++ = m_yoffs & 0xff;
		*dest++ = numchars >> 24;
		*dest++ = numchars >> 16;
		*dest++ = numchars >> 8;
		*dest++ = numchars & 0xff;
		assert(dest - tempbuffer == CACHED_HEADER_SIZE);
		UINT32 bytes_written = file.write(tempbuffer, dest - tempbuffer);
		if (bytes_written != dest - tempbuffer)
			throw emu_fatalerror("Error writing cached file");

		// write the empty table to the beginning of the file
		bytes_written = file.write(chartable, numchars * CACHED_CHAR_SIZE);
		if (bytes_written != numchars * CACHED_CHAR_SIZE)
			throw emu_fatalerror("Error writing cached file");

		// loop over all characters
		int tableindex = 0;
		for (int chnum = 0; chnum < 65536; chnum++)
		{
			glyph &gl = get_char(chnum);
			if (gl.width > 0)
			{
				// write out a bit-compressed bitmap if we have one
				if (gl.bitmap != NULL)
				{
					// write the data to the tempbuffer
					dest = tempbuffer;
					UINT8 accum = 0;
					UINT8 accbit = 7;

					// bit-encode the character data
					for (int y = 0; y < gl.bmheight; y++)
					{
						int desty = y + m_height + m_yoffs - gl.yoffs - gl.bmheight;
						const UINT32 *src = (desty >= 0 && desty < m_height) ? BITMAP_ADDR32(gl.bitmap, desty, 0) : NULL;
						for (int x = 0; x < gl.bmwidth; x++)
						{
							if (src != NULL && RGB_ALPHA(src[x]) != 0)
								accum |= 1 << accbit;
							if (accbit-- == 0)
							{
								*dest++ = accum;
								accum = 0;
								accbit = 7;
							}
						}
					}

					// flush any extra
					if (accbit != 7)
						*dest++ = accum;

					// write the data
					bytes_written = file.write(tempbuffer, dest - tempbuffer);
					if (bytes_written != dest - tempbuffer)
						throw emu_fatalerror("Error writing cached file");

					// free the bitmap and texture
					m_manager.texture_free(gl.texture);
					auto_free(&m_manager.machine(), gl.bitmap);
					gl.texture = NULL;
					gl.bitmap = NULL;
				}

				// compute the table entry
				dest = &chartable[tableindex++ * CACHED_CHAR_SIZE];
				*dest++ = chnum >> 8;
				*dest++ = chnum & 0xff;
				*dest++ = gl.width >> 8;
				*dest++ = gl.width & 0xff;
				*dest++ = gl.xoffs >> 8;
				*dest++ = gl.xoffs & 0xff;
				*dest++ = gl.yoffs >> 8;
				*dest++ = gl.yoffs & 0xff;
				*dest++ = gl.bmwidth >> 8;
				*dest++ = gl.bmwidth & 0xff;
				*dest++ = gl.bmheight >> 8;
				*dest++ = gl.bmheight & 0xff;
			}
		}

		// seek back to the beginning and rewrite the table
		file.seek(CACHED_HEADER_SIZE, SEEK_SET);
		bytes_written = file.write(chartable, numchars * CACHED_CHAR_SIZE);
		if (bytes_written != numchars * CACHED_CHAR_SIZE)
			throw emu_fatalerror("Error writing cached file");

		// all done
		auto_free(&m_manager.machine(), tempbuffer);
		auto_free(&m_manager.machine(), chartable);
		return true;
	}
	catch (...)
	{
		file.remove_on_close();
		auto_free(&m_manager.machine(), tempbuffer);
		auto_free(&m_manager.machine(), chartable);
		return false;
	}
}
