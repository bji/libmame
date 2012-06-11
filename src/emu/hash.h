/***************************************************************************

    hash.h

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

#pragma once

#ifndef __HASH_H__
#define __HASH_H__

#include "hashing.h"


//**************************************************************************
//  MACROS
//**************************************************************************

// use these to define compile-time internal-format hash strings
#define CRC(x)				"R" #x
#define SHA1(x)				"S" #x
#define NO_DUMP				"!"
#define BAD_DUMP			"^"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> hash_collection

// a collection of the various supported hashes and flags
class hash_collection
{
public:
	// hash types are identified by non-hex alpha values (G-Z)
	static const char HASH_CRC = 'R';
	static const char HASH_SHA1 = 'S';

	// common combinations for requests
	static const char *HASH_TYPES_CRC;
	static const char *HASH_TYPES_CRC_SHA1;
	static const char *HASH_TYPES_ALL;

	// flags are identified by punctuation marks
	static const char FLAG_NO_DUMP = '!';
	static const char FLAG_BAD_DUMP = '^';

	// construction/destruction
    hash_collection();
    hash_collection(const char *string);
    hash_collection(const hash_collection &src);
    ~hash_collection();

    // operators
    hash_collection &operator=(const hash_collection &src);
    bool operator==(const hash_collection &rhs) const;
    bool operator!=(const hash_collection &rhs) const { return !(*this == rhs); }

	// getters
    bool flag(char flag) const { return (m_flags.chr(0, flag) != -1); }
    const char *hash_types(astring &buffer) const;

	// hash manipulators
	void reset();
	bool add_from_string(char type, const char *buffer, int length = -1);
	bool remove(char type);

	// CRC-specific helpers
	bool crc(UINT32 &result) const { result = m_crc32; return m_has_crc32; }
	void add_crc(UINT32 crc) { m_crc32 = crc; m_has_crc32 = true; }

	// SHA1-specific helpers
	bool sha1(sha1_t &result) const { result = m_sha1; return m_has_sha1; }
	void add_sha1(sha1_t sha1) { m_has_sha1 = true; m_sha1 = sha1; }

	// string conversion
    const char *internal_string(astring &buffer) const;
    const char *macro_string(astring &buffer) const;
    const char *attribute_string(astring &buffer) const;
    bool from_internal_string(const char *string);

	// creation
    void begin(const char *types = NULL);
    void buffer(const UINT8 *data, UINT32 length);
    void end();
    void compute(const UINT8 *data, UINT32 length, const char *types = NULL) { begin(types); buffer(data, length); end(); }

private:
	// internal helpers
    void copyfrom(const hash_collection &src);

	// internal state
	astring					m_flags;
	bool					m_has_crc32;
	crc32_t					m_crc32;
	bool					m_has_sha1;
	sha1_t					m_sha1;

	// creators
	struct hash_creator
	{
		bool					m_doing_crc32;
		crc32_creator			m_crc32_creator;
		bool					m_doing_sha1;
		sha1_creator			m_sha1_creator;
	};
	hash_creator *			m_creator;
};


#endif	/* __HASH_H__ */
