/***************************************************************************

    audit.h

    ROM, disk, and sample auditing functions.

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

#ifndef __AUDIT_H__
#define __AUDIT_H__

#include "hash.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// hashes to use for validation
#define AUDIT_VALIDATE_FAST				"R"		/* CRC only */
#define AUDIT_VALIDATE_FULL				"RS"	/* CRC + SHA1 */



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> audit_record

// holds the result of auditing a single item
class audit_record
{
	friend class simple_list<audit_record>;

public:
	// media types
	enum media_type
	{
		MEDIA_ROM = 0,
		MEDIA_DISK,
		MEDIA_SAMPLE
	};

	// status values
	enum audit_status
	{
		STATUS_GOOD = 0,
		STATUS_FOUND_INVALID,
		STATUS_NOT_FOUND,
		STATUS_ERROR
	};

	// substatus values
	enum audit_substatus
	{
		SUBSTATUS_GOOD = 0,
		SUBSTATUS_GOOD_NEEDS_REDUMP,
		SUBSTATUS_FOUND_NODUMP,
		SUBSTATUS_FOUND_BAD_CHECKSUM,
		SUBSTATUS_FOUND_WRONG_LENGTH,
		SUBSTATUS_NOT_FOUND,
		SUBSTATUS_NOT_FOUND_NODUMP,
		SUBSTATUS_NOT_FOUND_OPTIONAL,
		SUBSTATUS_NOT_FOUND_PARENT,
		SUBSTATUS_NOT_FOUND_BIOS,
		SUBSTATUS_ERROR = 100
	};

	// construction/destruction
	audit_record(const rom_entry &media, media_type type);
	audit_record(const char *name, media_type type);

	// getters
	audit_record *next() const { return m_next; }
	media_type type() const { return m_type; }
	audit_status status() const { return m_status; }
	audit_substatus substatus() const { return m_substatus; }
	const char *name() const { return m_name; }
	UINT64 expected_length() const { return m_explength; }
	UINT64 actual_length() const { return m_length; }
	const hash_collection &expected_hashes() const { return m_exphashes; }
	const hash_collection &actual_hashes() const { return m_hashes; }

	// setters
	void set_status(audit_status status, audit_substatus substatus)
	{
		m_status = status;
		m_substatus = substatus;
	}

	void set_actual(const hash_collection &hashes, UINT64 length = 0)
	{
		m_hashes = hashes;
		m_length = length;
	}

private:
	// internal state
	audit_record *		m_next;
	media_type			m_type;					/* type of item that was audited */
	audit_status		m_status;				/* status of audit on this item */
	audit_substatus		m_substatus;			/* finer-detail status */
	const char *		m_name;					/* name of item */
	UINT64				m_explength;			/* expected length of item */
	UINT64				m_length;				/* actual length of item */
	hash_collection		m_exphashes;    		/* expected hash data */
	hash_collection		m_hashes;				/* actual hash information */
};


// ======================> media_auditor

// class which manages auditing of items
class media_auditor
{
public:
	// summary values
	enum summary
	{
		CORRECT = 0,
		NONE_NEEDED,
		BEST_AVAILABLE,
		INCORRECT,
		NOTFOUND
	};

	// construction/destruction
	media_auditor(const driver_enumerator &enumerator);

	// getters
	audit_record *first() const { return m_record_list.first(); }
	int count() const { return m_record_list.count(); }

	// audit operations
	summary audit_media(const char *validation = AUDIT_VALIDATE_FULL);
	summary audit_samples();
	summary summarize(astring *output = NULL);

private:
	// internal helpers
	audit_record *audit_one_rom(const rom_entry *rom);
	audit_record *audit_one_disk(const rom_entry *rom);
	void compute_status(audit_record &record, const rom_entry *rom, bool found);
	int also_used_by_parent(const hash_collection &romhashes);

	// internal state
	simple_list<audit_record>	m_record_list;
	const driver_enumerator &	m_enumerator;
	const char *				m_validation;
	const char *				m_searchpath;
};


#endif	/* __AUDIT_H__ */
