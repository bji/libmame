/***************************************************************************

    fileio.c

    File access functions.

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
#include "hash.h"
#include "unzip.h"
#include "options.h"
#include "fileio.h"


const UINT32 OPEN_FLAG_HAS_CRC	= 0x10000;



//**************************************************************************
//  PATH ITERATOR
//**************************************************************************

//-------------------------------------------------
//  path_iterator - constructor
//-------------------------------------------------

path_iterator::path_iterator(core_options &opts, const char *searchpath)
	: m_base((searchpath != NULL && !osd_is_absolute_path(searchpath)) ? options_get_string(&opts, searchpath) : ""),
	  m_current(m_base),
	  m_index(0)
{
}


//-------------------------------------------------
//  path_iterator_get_next - get the next entry
//  in a multipath sequence
//-------------------------------------------------

bool path_iterator::next(astring &buffer)
{
	// if none left, return FALSE to indicate we are done
	if (m_index != 0 && *m_current == 0)
		return false;

	// copy up to the next semicolon
	const char *semi = strchr(m_current, ';');
	if (semi == NULL)
		semi = m_current + strlen(m_current);
	buffer.cpy(m_current, semi - m_current);
	m_current = (*semi == 0) ? semi : semi + 1;

	// bump the index and return TRUE
	m_index++;
	return true;
}



//**************************************************************************
//  FILE ENUMERATOR
//**************************************************************************

//-------------------------------------------------
//  file_enumerator - constructor
//-------------------------------------------------

file_enumerator::file_enumerator(core_options &options, const char *searchpath)
	: m_iterator(options, searchpath),
	  m_curdir(NULL),
	  m_buflen(0)
{
}


//-------------------------------------------------
//  ~file_enumerator - destructor
//-------------------------------------------------

file_enumerator::~file_enumerator()
{
	// close anything open
	if (m_curdir != NULL)
		osd_closedir(m_curdir);
}


//-------------------------------------------------
//  next - return information about the next file
//  in the search path
//-------------------------------------------------

const osd_directory_entry *file_enumerator::next()
{
	// loop over potentially empty directories
	while (1)
	{
		// if no open directory, get the next path
		while (m_curdir == NULL)
		{
			// if we fail to get anything more, we're done
			if (!m_iterator.next(m_pathbuffer))
				return NULL;

			// open the path
			m_curdir = osd_opendir(m_pathbuffer);
		}

		// get the next entry from the current directory
		const osd_directory_entry *result = osd_readdir(m_curdir);
		if (result != NULL)
			return result;

		// we're done; close this directory
		osd_closedir(m_curdir);
		m_curdir = NULL;
	}
}



//**************************************************************************
//  EMU FILE
//**************************************************************************

//-------------------------------------------------
//  emu_file - constructor
//-------------------------------------------------

emu_file::emu_file(core_options &options, const char *searchpath, UINT32 openflags)
	: m_file(NULL),
	  m_iterator(options, searchpath),
	  m_crc(0),
	  m_openflags(openflags),
	  m_zipfile(NULL),
	  m_zipdata(NULL),
	  m_ziplength(0),
	  m_remove_on_close(false)
{
	// sanity check the open flags
	if ((m_openflags & OPEN_FLAG_HAS_CRC) && (m_openflags & OPEN_FLAG_WRITE))
		throw emu_fatalerror("Attempted to open a file for write with OPEN_FLAG_HAS_CRC");
}


//-------------------------------------------------
//  ~emu_file - destructor
//-------------------------------------------------

emu_file::~emu_file()
{
	// close in the standard way
	close();
}


//-------------------------------------------------
//  operator core_file - automatically convert
//  ourselves to a core_file pointer
//-------------------------------------------------

emu_file::operator core_file *()
{
	// load the ZIP file now if we haven't yet
	if (m_zipfile != NULL && load_zipped_file() != FILERR_NONE)
		return NULL;

	// return the core file
	return m_file;
}


//-------------------------------------------------
//  hash - returns the hash for a file
//-------------------------------------------------

hash_collection &emu_file::hashes(const char *types)
{
	// determine which hashes we need
	astring needed;
	for (const char *scan = types; *scan != 0; scan++)
		if (m_hashes.hash(*scan) == NULL)
			needed.cat(*scan);

	// if we need nothing, skip it
	if (!needed)
		return m_hashes;

	// load the ZIP file if needed
	if (m_zipfile != NULL && load_zipped_file() != FILERR_NONE)
		return m_hashes;
	if (m_file == NULL)
		return m_hashes;

	// if we have ZIP data, just hash that directly
	if (m_zipdata != NULL)
	{
		m_hashes.compute(m_zipdata, m_ziplength, needed);
		return m_hashes;
	}

	// read the data if we can
	const UINT8 *filedata = (const UINT8 *)core_fbuffer(m_file);
	if (filedata == NULL)
		return m_hashes;

	// compute the hash
	m_hashes.compute(filedata, core_fsize(m_file), needed);
	return m_hashes;
}


//-------------------------------------------------
//  open - open a file by searching paths
//-------------------------------------------------

file_error emu_file::open(const char *name)
{
	// remember the filename and CRC info
	m_filename = name;
	m_crc = 0;
	m_openflags &= ~OPEN_FLAG_HAS_CRC;

	// reset the iterator and open_next
	m_iterator.reset();
	return open_next();
}

file_error emu_file::open(const char *name1, const char *name2)
{
	// concatenate the strings and do a standard open
	astring name(name1, name2);
	return open(name);
}

file_error emu_file::open(const char *name1, const char *name2, const char *name3)
{
	// concatenate the strings and do a standard open
	astring name(name1, name2, name3);
	return open(name);
}

file_error emu_file::open(const char *name1, const char *name2, const char *name3, const char *name4)
{
	// concatenate the strings and do a standard open
	astring name(name1, name2, name3, name4);
	return open(name);
}

file_error emu_file::open(const char *name, UINT32 crc)
{
	// remember the filename and CRC info
	m_filename = name;
	m_crc = crc;
	m_openflags |= OPEN_FLAG_HAS_CRC;

	// reset the iterator and open_next
	m_iterator.reset();
	return open_next();
}

file_error emu_file::open(const char *name1, const char *name2, UINT32 crc)
{
	// concatenate the strings and do a standard open
	astring name(name1, name2);
	return open(name, crc);
}

file_error emu_file::open(const char *name1, const char *name2, const char *name3, UINT32 crc)
{
	// concatenate the strings and do a standard open
	astring name(name1, name2, name3);
	return open(name, crc);
}

file_error emu_file::open(const char *name1, const char *name2, const char *name3, const char *name4, UINT32 crc)
{
	// concatenate the strings and do a standard open
	astring name(name1, name2, name3, name4);
	return open(name, crc);
}


//-------------------------------------------------
//  open_next - open the next file that matches
//  the filename by iterating over paths
//-------------------------------------------------

file_error emu_file::open_next()
{
	// if we're open from a previous attempt, close up now
	if (m_file != NULL)
		close();

	// loop over paths
	file_error filerr = FILERR_NOT_FOUND;
	while (m_iterator.next(m_fullpath))
	{
		// compute the full pathname
		if (m_fullpath.len() > 0)
			m_fullpath.cat(PATH_SEPARATOR);
		m_fullpath.cat(m_filename);

		// attempt to open the file directly
		filerr = core_fopen(m_fullpath, m_openflags, &m_file);
		if (filerr == FILERR_NONE)
			break;

		// if we're opening for read-only we have other options
		if ((m_openflags & (OPEN_FLAG_READ | OPEN_FLAG_WRITE)) == OPEN_FLAG_READ)
		{
			filerr = attempt_zipped();
			if (filerr == FILERR_NONE)
				break;
		}
	}
	return filerr;
}


//-------------------------------------------------
//  open_ram - open a "file" which is actually
//  just an array of data in RAM
//-------------------------------------------------

file_error emu_file::open_ram(const void *data, UINT32 length)
{
	// set a fake filename and CRC
	m_filename = "RAM";
	m_crc = 0;

	// use the core_file's built-in RAM support
	return core_fopen_ram(data, length, m_openflags, &m_file);
}


//-------------------------------------------------
//  close - close a file and free all data; also
//  remove the file if requested
//-------------------------------------------------

void emu_file::close()
{
	// close files and free memory
	if (m_zipfile != NULL)
		zip_file_close(m_zipfile);
	m_zipfile = NULL;

	if (m_file != NULL)
		core_fclose(m_file);
	m_file = NULL;

	if (m_zipdata != NULL)
		global_free(m_zipdata);
	m_zipdata = NULL;

	if (m_remove_on_close)
		osd_rmfile(m_fullpath);
	m_remove_on_close = false;

	// reset our hashes and path as well
	m_hashes.reset();
	m_fullpath.reset();
}


//-------------------------------------------------
//  compress - enable/disable streaming file
//  compression via zlib; level is 0 to disable
//  compression, or up to 9 for max compression
//-------------------------------------------------

file_error emu_file::compress(int level)
{
	return core_fcompress(m_file, level);
}


//-------------------------------------------------
//  seek - seek within a file
//-------------------------------------------------

int emu_file::seek(INT64 offset, int whence)
{
	// load the ZIP file now if we haven't yet
	if (m_zipfile != NULL && load_zipped_file() != FILERR_NONE)
		return 1;

	// seek if we can
	if (m_file != NULL)
		return core_fseek(m_file, offset, whence);

	return 1;
}


//-------------------------------------------------
//  tell - return the current file position
//-------------------------------------------------

UINT64 emu_file::tell()
{
	// load the ZIP file now if we haven't yet
	if (m_zipfile != NULL && load_zipped_file() != FILERR_NONE)
		return 0;

	// tell if we can
	if (m_file != NULL)
		return core_ftell(m_file);

	return 0;
}


//-------------------------------------------------
//  eof - return true if we're at the end of file
//-------------------------------------------------

bool emu_file::eof()
{
	// load the ZIP file now if we haven't yet
	if (m_zipfile != NULL && load_zipped_file() != FILERR_NONE)
		return 0;

	// return EOF if we can
	if (m_file != NULL)
		return core_feof(m_file);

	return 0;
}


//-------------------------------------------------
//  size - returns the size of a file
//-------------------------------------------------

UINT64 emu_file::size()
{
	// use the ZIP length if present
	if (m_zipfile != NULL)
		return m_ziplength;

	// return length if we can
	if (m_file != NULL)
		return core_fsize(m_file);

	return 0;
}


//-------------------------------------------------
//  read - read from a file
//-------------------------------------------------

UINT32 emu_file::read(void *buffer, UINT32 length)
{
	// load the ZIP file now if we haven't yet
	if (m_zipfile != NULL && load_zipped_file() != FILERR_NONE)
		return 0;

	// read the data if we can
	if (m_file != NULL)
		return core_fread(m_file, buffer, length);

	return 0;
}


//-------------------------------------------------
//  getc - read a character from a file
//-------------------------------------------------

int emu_file::getc()
{
	// load the ZIP file now if we haven't yet
	if (m_zipfile != NULL && load_zipped_file() != FILERR_NONE)
		return EOF;

	// read the data if we can
	if (m_file != NULL)
		return core_fgetc(m_file);

	return EOF;
}


//-------------------------------------------------
//  ungetc - put back a character read from a file
//-------------------------------------------------

int emu_file::ungetc(int c)
{
	// load the ZIP file now if we haven't yet
	if (m_zipfile != NULL && load_zipped_file() != FILERR_NONE)
		return 1;

	// read the data if we can
	if (m_file != NULL)
		return core_ungetc(c, m_file);

	return 1;
}


//-------------------------------------------------
//  gets - read a line from a text file
//-------------------------------------------------

char *emu_file::gets(char *s, int n)
{
	// load the ZIP file now if we haven't yet
	if (m_zipfile != NULL && load_zipped_file() != FILERR_NONE)
		return NULL;

	// read the data if we can
	if (m_file != NULL)
		return core_fgets(s, n, m_file);

	return NULL;
}


//-------------------------------------------------
//  write - write to a file
//-------------------------------------------------

UINT32 emu_file::write(const void *buffer, UINT32 length)
{
	// write the data if we can
	if (m_file != NULL)
		return core_fwrite(m_file, buffer, length);

	return 0;
}


//-------------------------------------------------
//  puts - write a line to a text file
//-------------------------------------------------

int emu_file::puts(const char *s)
{
	// write the data if we can
	if (m_file != NULL)
		return core_fputs(m_file, s);

	return 0;
}


//-------------------------------------------------
//  printf - vfprintf to a text file
//-------------------------------------------------

int CLIB_DECL emu_file::printf(const char *fmt, ...)
{
	int rc;
	va_list va;
	va_start(va, fmt);
	rc = vprintf(fmt, va);
	va_end(va);
	return rc;
}


//-------------------------------------------------
//  mame_vfprintf - vfprintf to a text file
//-------------------------------------------------

int emu_file::vprintf(const char *fmt, va_list va)
{
	// write the data if we can
	return (m_file != NULL) ? core_vfprintf(m_file, fmt, va) : 0;
}



//-------------------------------------------------
//  attempt_zipped - attempt to open a ZIPped file
//-------------------------------------------------

file_error emu_file::attempt_zipped()
{
	astring filename;

	// loop over directory parts up to the start of filename
	while (1)
	{
		// find the final path separator
		int dirsep = m_fullpath.rchr(0, PATH_SEPARATOR[0]);
		if (dirsep == -1)
			return FILERR_NOT_FOUND;

		// insert the part from the right of the separator into the head of the filename
		if (filename.len() > 0)
			filename.ins(0, "/");
		filename.inssubstr(0, m_fullpath, dirsep + 1, -1);

		// remove this part of the filename and append a .zip extension
		m_fullpath.substr(0, dirsep).cat(".zip");

		// attempt to open the ZIP file
		zip_file *zip;
		zip_error ziperr = zip_file_open(m_fullpath, &zip);

		// chop the .zip back off the filename before continuing
		m_fullpath.substr(0, dirsep);

		// if we failed to open this file, continue scanning
		if (ziperr != ZIPERR_NONE)
			continue;

		// see if we can find a file with the right name and (if available) crc
		const zip_file_header *header;
		for (header = zip_file_first_file(zip); header != NULL; header = zip_file_next_file(zip))
			if (zip_filename_match(*header, filename) && (!(m_openflags & OPEN_FLAG_HAS_CRC) || header->crc == m_crc))
				break;

		// if that failed, look for a file with the right crc, but the wrong filename
		if (header == NULL && (m_openflags & OPEN_FLAG_HAS_CRC))
			for (header = zip_file_first_file(zip); header != NULL; header = zip_file_next_file(zip))
				if (header->crc == m_crc && !zip_header_is_path(*header))
					break;

		// if that failed, look for a file with the right name; reporting a bad checksum
		// is more helpful and less confusing than reporting "rom not found"
		if (header == NULL)
			for (header = zip_file_first_file(zip); header != NULL; header = zip_file_next_file(zip))
				if (zip_filename_match(*header, filename))
					break;

		// if we got it, read the data
		if (header != NULL)
		{
			m_zipfile = zip;
			m_ziplength = header->uncompressed_length;

			// build a hash with just the CRC
			m_hashes.reset();
			m_hashes.add_crc(header->crc);
			return (m_openflags & OPEN_FLAG_NO_PRELOAD) ? FILERR_NONE : load_zipped_file();
		}

		// close up the ZIP file and try the next level
		zip_file_close(zip);
	}
}


//-------------------------------------------------
//  load_zipped_file - load a ZIPped file
//-------------------------------------------------

file_error emu_file::load_zipped_file()
{
	assert(m_file == NULL);
	assert(m_zipdata == NULL);
	assert(m_zipfile != NULL);

	// allocate some memory
	m_zipdata = global_alloc_array(UINT8, m_ziplength);

	// read the data into our buffer and return
	zip_error ziperr = zip_file_decompress(m_zipfile, m_zipdata, m_ziplength);
	if (ziperr != ZIPERR_NONE)
	{
		global_free(m_zipdata);
		m_zipdata = NULL;
		return FILERR_FAILURE;
	}

	// convert to RAM file
	file_error filerr = core_fopen_ram(m_zipdata, m_ziplength, m_openflags, &m_file);
	if (filerr != FILERR_NONE)
	{
		global_free(m_zipdata);
		m_zipdata = NULL;
		return FILERR_FAILURE;
	}

	// close out the ZIP file
	zip_file_close(m_zipfile);
	m_zipfile = NULL;
	return FILERR_NONE;
}


//-------------------------------------------------
//  zip_filename_match - compare zip filename
//  to expected filename, ignoring any directory
//-------------------------------------------------

bool emu_file::zip_filename_match(const zip_file_header &header, const astring &filename)
{
	const char *zipfile = header.filename + header.filename_length - filename.len();
	return (zipfile >= header.filename && filename.icmp(zipfile) == 0 && (zipfile == header.filename || zipfile[-1] == '/'));
}


//-------------------------------------------------
//  zip_header_is_path - check whether filename
//  in header is a path
//-------------------------------------------------

bool emu_file::zip_header_is_path(const zip_file_header &header)
{
	const char *zipfile = header.filename + header.filename_length - 1;
	return (zipfile >= header.filename && zipfile[0] == '/');
}
