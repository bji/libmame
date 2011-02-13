/***************************************************************************

    drcbeut.h

    Utility functions for dynamic recompiling backends.

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

#ifndef __DRCBEUT_H__
#define __DRCBEUT_H__

#include "drcuml.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> drc_hash_table

// common hash table management
class drc_hash_table
{
public:
	// construction/destruction
	drc_hash_table(drc_cache &cache, UINT32 modes, UINT8 addrbits, UINT8 ignorebits);

	// getters
	drccodeptr ***base() const { return m_base; }
	UINT8 l1bits() const { return m_l1bits; }
	UINT8 l2bits() const { return m_l2bits; }
	UINT8 l1shift() const { return m_l1shift; }
	UINT8 l2shift() const { return m_l2shift; }
	offs_t l1mask() const { return m_l1mask; }
	offs_t l2mask() const { return m_l2mask; }
	bool is_mode_populated(UINT32 mode) const { return m_base[mode] != m_emptyl1; }

	// set up and configuration
	bool reset();
	void set_default_codeptr(drccodeptr code);

	// block begin/end
	void block_begin(drcuml_block &block, const uml::instruction *instlist, UINT32 numinst);
	void block_end(drcuml_block &block);

	// code pointer access
	bool set_codeptr(UINT32 mode, UINT32 pc, drccodeptr code);
	drccodeptr get_codeptr(UINT32 mode, UINT32 pc) { assert(mode < m_modes); return m_base[mode][(pc >> m_l1shift) & m_l1mask][(pc >> m_l2shift) & m_l2mask]; }
	bool code_exists(UINT32 mode, UINT32 pc) { return get_codeptr(mode, pc) != m_nocodeptr; }

private:
	// internal state
	drc_cache &		m_cache;				// cache where allocations come from
	UINT32			m_modes;				// number of modes supported

	drccodeptr		m_nocodeptr;			// pointer to code which will handle missing entries

	UINT8			m_l1bits;				// bits worth of entries in l1 hash tables
	UINT8			m_l2bits;				// bits worth of entries in l2 hash tables
	UINT8			m_l1shift;				// shift to apply to the PC to get the l1 hash entry
	UINT8			m_l2shift;				// shift to apply to the PC to get the l2 hash entry
	offs_t			m_l1mask;				// mask to apply after shifting
	offs_t			m_l2mask;				// mask to apply after shifting

	drccodeptr ***	m_base;					// pointer to the l1 table for each mode
	drccodeptr **	m_emptyl1;				// pointer to empty l1 hash table
	drccodeptr *	m_emptyl2;				// pointer to empty l2 hash table
};


// ======================> drc_map_variables

// common map variable management
class drc_map_variables
{
public:
	// construction/destruction
	drc_map_variables(drc_cache &cache, UINT64 uniquevalue);
	~drc_map_variables();

	// block begin/end
	void block_begin(drcuml_block &block);
	void block_end(drcuml_block &block);

	// get/set values
	void set_value(drccodeptr codebase, UINT32 mapvar, UINT32 newvalue);
	UINT32 get_value(drccodeptr codebase, UINT32 mapvar) const;
	UINT32 get_last_value(UINT32 mapvar);

	// static accessors to be called directly by generated code
	static UINT32 static_get_value(drc_map_variables &map, drccodeptr codebase, UINT32 mapvar);

private:
	// internal state
	drc_cache &			m_cache;			// pointer to the cache
	UINT64				m_uniquevalue;		// unique value used to find the table
	UINT32				m_mapvalue[uml::MAPVAR_END - uml::MAPVAR_M0]; // array of current values

	// list of entries
	struct map_entry
	{
		map_entry *next() const { return m_next; }
		map_entry *		m_next;				// pointer to next map entry
		drccodeptr		m_codeptr;			// pointer to the relevant code
		UINT32			m_mapvar;			// map variable id
		UINT32			m_newval;			// value of the variable starting at codeptr
	};
	simple_list<map_entry> m_entry_list;	// list of entries
};


// ======================> drc_label_list

// structure holding a live list of labels
class drc_label_list
{
	// callback function for forward-referenced labels
	typedef void (*fixup_func)(void *parameter, drccodeptr labelcodeptr);

public:
	// construction/destruction
	drc_label_list(drc_cache &cache);
	~drc_label_list();

	// block begin/end
	void block_begin(drcuml_block &block);
	void block_end(drcuml_block &block);

	// get/set values
	drccodeptr get_codeptr(uml::code_label label, fixup_func fixup, void *param);
	void set_codeptr(uml::code_label label, drccodeptr codeptr);

private:
	struct label_entry
	{
		label_entry *next() const { return m_next; }
		label_entry *		m_next;			// pointer to next label
		uml::code_label		m_label;		// the label specified
		drccodeptr			m_codeptr;		// pointer to the relevant code
	};

	// internal helpers
	void reset(bool fatal_on_leftovers);
	label_entry *find_or_allocate(uml::code_label label);
	static void oob_callback(drccodeptr *codeptr, void *param1, void *param2, void *param3);

	// internal state
	drc_cache &			m_cache;			// pointer to the cache
	simple_list<label_entry> m_list;		// head of the live list
};


#endif /* __DRCBEUT_H__ */
