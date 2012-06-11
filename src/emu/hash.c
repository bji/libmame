/***************************************************************************

    hash.c

    Function to handle hash functions (checksums)

    Based on original idea by Farfetch'd

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
#include "hashing.h"
#include <ctype.h>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const char *hash_collection::HASH_TYPES_CRC = "R";
const char *hash_collection::HASH_TYPES_CRC_SHA1 = "RS";
const char *hash_collection::HASH_TYPES_ALL = "RS";



//**************************************************************************
//  HASH COLLECTION
//**************************************************************************

//-------------------------------------------------
//  hash_collection - constructor
//-------------------------------------------------

hash_collection::hash_collection()
	: m_has_crc32(false),
	  m_has_sha1(false),
	  m_creator(NULL)
{
}


hash_collection::hash_collection(const char *string)
	: m_has_crc32(false),
	  m_has_sha1(false),
	  m_creator(NULL)
{
	from_internal_string(string);
}


hash_collection::hash_collection(const hash_collection &src)
	: m_has_crc32(false),
	  m_has_sha1(false),
	  m_creator(NULL)
{
	copyfrom(src);
}


//-------------------------------------------------
//  ~hash_collection - destructor
//-------------------------------------------------

hash_collection::~hash_collection()
{
	delete m_creator;
}


//-------------------------------------------------
//  operator= - assignment operator
//-------------------------------------------------

hash_collection &hash_collection::operator=(const hash_collection &src)
{
	// ignore self-assignment
	if (this != &src)
		copyfrom(src);
	return *this;
}


//-------------------------------------------------
//  operator== - test for equality
//-------------------------------------------------

bool hash_collection::operator==(const hash_collection &rhs) const
{
	// match CRCs
	int matches = 0;
	if (m_has_crc32 && rhs.m_has_crc32)
	{
		if (m_crc32 != rhs.m_crc32)
			return false;
		matches++;
	}

	// match SHA1s
	if (m_has_sha1 && rhs.m_has_sha1)
	{
		if (m_sha1 != rhs.m_sha1)
			return false;
		matches++;
	}

	// if all shared hashes match, return true
	return (matches > 0);
}


//-------------------------------------------------
//  hash_types - return a list of hash types as
//  a string
//-------------------------------------------------

const char *hash_collection::hash_types(astring &buffer) const
{
	buffer.reset();
	if (m_has_crc32)
		buffer.cat(HASH_CRC);
	if (m_has_sha1)
		buffer.cat(HASH_SHA1);
	return buffer;
}


//-------------------------------------------------
//  reset - reset the hash collection to an empty
//  set of hashes and flags
//-------------------------------------------------

void hash_collection::reset()
{
	m_flags.reset();
	m_has_crc32 = m_has_sha1 = false;
	delete m_creator;
	m_creator = NULL;
}


//-------------------------------------------------
//  add_from_string - add a new hash, importing
//  from a string
//-------------------------------------------------

bool hash_collection::add_from_string(char type, const char *buffer, int length)
{
	// handle CRCs
	if (type == HASH_CRC)
		return m_has_crc32 = m_crc32.from_string(buffer, length);

	// handle SHA1s
	else if (type == HASH_SHA1)
		return m_has_sha1 = m_sha1.from_string(buffer, length);

	return false;
}


//-------------------------------------------------
//  remove - remove a hash of the given type
//-------------------------------------------------

bool hash_collection::remove(char type)
{
	bool result = false;

	// handle CRCs
	if (type == HASH_CRC)
	{
		result = m_has_crc32;
		m_has_crc32 = false;
	}

	// handle SHA1s
	else if (type == HASH_SHA1)
	{
		result = m_has_sha1;
		m_has_sha1 = false;
	}
	return result;
}


//-------------------------------------------------
//  internal_string - convert set of hashes and
//  flags to a string in our internal compact
//  format
//-------------------------------------------------

const char *hash_collection::internal_string(astring &buffer) const
{
	buffer.reset();

	// handle CRCs
	astring temp;
	if (m_has_crc32)
		buffer.cat(HASH_CRC).cat(m_crc32.as_string(temp));

	// handle SHA1s
	if (m_has_sha1)
		buffer.cat(HASH_SHA1).cat(m_sha1.as_string(temp));

	// append flags
	return buffer.cat(m_flags);
}


//-------------------------------------------------
//  macro_string - convert set of hashes and
//  flags to a string in the macroized format
//-------------------------------------------------

const char *hash_collection::macro_string(astring &buffer) const
{
	buffer.reset();

	// handle CRCs
	astring temp;
	if (m_has_crc32)
		buffer.cat("CRC(").cat(m_crc32.as_string(temp)).cat(") ");

	// handle SHA1s
	if (m_has_sha1)
		buffer.cat("SHA1(").cat(m_sha1.as_string(temp)).cat(") ");

	// append flags
	if (flag(FLAG_NO_DUMP))
		buffer.cat("NO_DUMP ");
	if (flag(FLAG_BAD_DUMP))
		buffer.cat("BAD_DUMP ");
	return buffer.trimspace();
}


//-------------------------------------------------
//  attribute_string - convert set of hashes and
//  flags to a string in XML attribute format
//-------------------------------------------------

const char *hash_collection::attribute_string(astring &buffer) const
{
	buffer.reset();

	// handle CRCs
	astring temp;
	if (m_has_crc32)
		buffer.cat("crc=\"").cat(m_crc32.as_string(temp)).cat("\" ");

	// handle SHA1s
	if (m_has_sha1)
		buffer.cat("sha1=\"").cat(m_sha1.as_string(temp)).cat("\" ");

	// append flags
	if (flag(FLAG_NO_DUMP))
		buffer.cat("status=\"nodump\"" );
	if (flag(FLAG_BAD_DUMP))
		buffer.cat("status=\"baddump\"" );
	return buffer.trimspace();
}


//-------------------------------------------------
//  from_internal_string - convert an internal
//  compact string to set of hashes and flags
//-------------------------------------------------

bool hash_collection::from_internal_string(const char *string)
{
	// start fresh
	reset();

	// determine the end of the string
	const char *stringend = string + strlen(string);
	const char *ptr = string;

	// loop until we hit it
	bool errors = false;
	int skip_digits = 0;
	while (ptr < stringend)
	{
		char c = *ptr++;
		char uc = toupper(c);

		// non-hex alpha values specify a hash type
		if (uc >= 'G' && uc <= 'Z')
		{
			if (skip_digits != 0)
				errors = true;
			skip_digits = 0;
			if (uc == HASH_CRC)
			{
				m_has_crc32 = true;
				errors = !m_crc32.from_string(ptr, stringend - ptr);
				skip_digits = 2 * sizeof(crc32_t);
			}
			else if (uc == HASH_SHA1)
			{
				m_has_sha1 = true;
				errors = !m_sha1.from_string(ptr, stringend - ptr);
				skip_digits = 2 * sizeof(sha1_t);
			}
			else
				errors = true;
		}

		// hex values are ignored, though unexpected
		else if ((uc >= '0' && uc <= '9') || (uc >= 'A' && uc <= 'F'))
		{
			if (skip_digits != 0)
				skip_digits--;
			else
				errors = true;
		}

		// anything else is a flag
		else if (skip_digits != 0)
			errors = true;
		else
			m_flags.cat(c);
	}
	return !errors;
}


//-------------------------------------------------
//  begin - begin hashing
//-------------------------------------------------

void hash_collection::begin(const char *types)
{
	// nuke previous creator and make a new one
	delete m_creator;
	m_creator = new hash_creator;

	// by default use all types
	if (types == NULL)
		m_creator->m_doing_crc32 = m_creator->m_doing_sha1 = true;

	// otherwise, just allocate the ones that are specified
	else
	{
		m_creator->m_doing_crc32 = (strchr(types, HASH_CRC) != NULL);
		m_creator->m_doing_sha1 = (strchr(types, HASH_SHA1) != NULL);
	}
}


//-------------------------------------------------
//  buffer - add the given buffer to the hash
//-------------------------------------------------

void hash_collection::buffer(const UINT8 *data, UINT32 length)
{
	assert(m_creator != NULL);

	// append to each active hash
	if (m_creator->m_doing_crc32)
		m_creator->m_crc32_creator.append(data, length);
	if (m_creator->m_doing_sha1)
		m_creator->m_sha1_creator.append(data, length);
}


//-------------------------------------------------
//  end - stop hashing
//-------------------------------------------------

void hash_collection::end()
{
	assert(m_creator != NULL);

	// finish up the CRC32
	if (m_creator->m_doing_crc32)
	{
		m_has_crc32 = true;
		m_crc32 = m_creator->m_crc32_creator.finish();
	}

	// finish up the SHA1
	if (m_creator->m_doing_sha1)
	{
		m_has_sha1 = true;
		m_sha1 = m_creator->m_sha1_creator.finish();
	}

	// nuke the creator
	delete m_creator;
	m_creator = NULL;
}


//-------------------------------------------------
//  copyfrom - copy everything from another
//  collection
//-------------------------------------------------

void hash_collection::copyfrom(const hash_collection &src)
{
	// copy flags directly
	m_flags = src.m_flags;

	// copy hashes
	m_has_crc32 = src.m_has_crc32;
	m_crc32 = src.m_crc32;
	m_has_sha1 = src.m_has_sha1;
	m_sha1 = src.m_sha1;

	// don't copy creators
	m_creator = NULL;
}
