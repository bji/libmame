/***************************************************************************

    state.c

    Save state management functions.

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

    Save state file format:

    00..07  'MAMESAVE'
    08      Format version (this is format 2)
    09      Flags
    0A..1B  Game name padded with \0
    1C..1F  Signature
    20..end Save game data (compressed)

    Data is always written as native-endian.
    Data is converted from the endiannness it was written upon load.

***************************************************************************/

#include "emu.h"

#include <zlib.h>


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int SAVE_VERSION		= 2;
const int HEADER_SIZE		= 32;

// Available flags
enum
{
	SS_MSB_FIRST = 0x02
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

#ifdef MESS
const char state_manager::s_magic_num[8] = { 'M', 'E', 'S', 'S', 'S', 'A', 'V', 'E' };
#else
const char state_manager::s_magic_num[8] = { 'M', 'A', 'M', 'E', 'S', 'A', 'V', 'E' };
#endif



//**************************************************************************
//  INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  state_manager - constructor
//-------------------------------------------------

state_manager::state_manager(running_machine &machine)
	: m_machine(machine),
	  m_reg_allowed(true),
	  m_illegal_regs(0),
	  m_entry_list(machine.m_respool),
	  m_presave_list(machine.m_respool),
	  m_postload_list(machine.m_respool)
{
}


//-------------------------------------------------
//  allow_registration - allow/disallow
//  registrations to happen
//-------------------------------------------------

void state_manager::allow_registration(bool allowed)
{
	// allow/deny registration
	m_reg_allowed = allowed;
	if (!allowed)
		dump_registry();
}


//-------------------------------------------------
//  indexed_item - return an item with the given
//  index
//-------------------------------------------------

const char *state_manager::indexed_item(int index, void *&base, UINT32 &valsize, UINT32 &valcount) const
{
	state_entry *entry = m_entry_list.find(index);
	if (entry == NULL)
		return NULL;

	base = entry->m_data;
	valsize = entry->m_typesize;
	valcount = entry->m_typecount;

	return entry->m_name;
}


//-------------------------------------------------
//  register_presave - register a pre-save
//  function callback
//-------------------------------------------------

void state_manager::register_presave(prepost_func func, void *param)
{
	// check for invalid timing
	if (!m_reg_allowed)
		fatalerror("Attempt to register callback function after state registration is closed!");

	// scan for duplicates and push through to the end
	for (state_callback *cb = m_presave_list.first(); cb != NULL; cb = cb->next())
		if (cb->m_func == func && cb->m_param == param)
			fatalerror("Duplicate save state function (%p, %p)", param, func);

	// allocate a new entry
	m_presave_list.append(*auto_alloc(&m_machine, state_callback(func, param)));
}


//-------------------------------------------------
//  state_save_register_postload -
//  register a post-load function callback
//-------------------------------------------------

void state_manager::register_postload(prepost_func func, void *param)
{
	// check for invalid timing
	if (!m_reg_allowed)
		fatalerror("Attempt to register callback function after state registration is closed!");

	// scan for duplicates and push through to the end
	for (state_callback *cb = m_postload_list.first(); cb != NULL; cb = cb->next())
		if (cb->m_func == func && cb->m_param == param)
			fatalerror("Duplicate save state function (%p, %p)", param, func);

	// allocate a new entry
	m_postload_list.append(*auto_alloc(&m_machine, state_callback(func, param)));
}


//-------------------------------------------------
//  save_memory - register an array of data in
//  memory
//-------------------------------------------------

void state_manager::save_memory(const char *module, const char *tag, UINT32 index, const char *name, void *val, UINT32 valsize, UINT32 valcount)
{
	assert(valsize == 1 || valsize == 2 || valsize == 4 || valsize == 8);

	// check for invalid timing
	if (!m_reg_allowed)
	{
		logerror("Attempt to register save state entry after state registration is closed!\nModule %s tag %s name %s\n", module, tag, name);
		if (m_machine.gamedrv->flags & GAME_SUPPORTS_SAVE)
			fatalerror("Attempt to register save state entry after state registration is closed!\nModule %s tag %s name %s\n", module, tag, name);
		m_illegal_regs++;
		return;
	}

	// create the full name
	astring totalname;
	if (tag != NULL)
		totalname.printf("%s/%s/%X/%s", module, tag, index, name);
	else
		totalname.printf("%s/%X/%s", module, index, name);

	// look for duplicates and an entry to insert in front of
	state_entry *insert_after = NULL;
	for (state_entry *entry = m_entry_list.first(); entry != NULL; entry = entry->next())
	{
		// stop when we find an entry whose name is after ours
		if (entry->m_name > totalname)
			break;
		insert_after = entry;

		// error if we are equal
		if (entry->m_name == totalname)
			fatalerror("Duplicate save state registration entry (%s)", totalname.cstr());
	}

	// insert us into the list
	m_entry_list.insert_after(*auto_alloc(&m_machine, state_entry(val, totalname, valsize, valcount)), insert_after);
}


//-------------------------------------------------
//  check_file - check if a file is a valid save
//  state
//-------------------------------------------------

state_save_error state_manager::check_file(running_machine *machine, emu_file &file, const char *gamename, void (CLIB_DECL *errormsg)(const char *fmt, ...))
{
	// if we want to validate the signature, compute it
	UINT32 sig = 0;
	if (machine != NULL)
		sig = machine->state().signature();

	// seek to the beginning and read the header
	file.compress(FCOMPRESS_NONE);
	file.seek(0, SEEK_SET);
	UINT8 header[HEADER_SIZE];
	if (file.read(header, sizeof(header)) != sizeof(header))
	{
		if (errormsg != NULL)
			(*errormsg)("Could not read " APPNAME " save file header");
		return STATERR_READ_ERROR;
	}

	// let the generic header check work out the rest
	return validate_header(header, gamename, sig, errormsg, "");
}


//-------------------------------------------------
//  read_file - read the data from a file
//-------------------------------------------------

state_save_error state_manager::read_file(emu_file &file)
{
	// if we have illegal registrations, return an error
	if (m_illegal_regs > 0)
		return STATERR_ILLEGAL_REGISTRATIONS;

	// read the header and turn on compression for the rest of the file
	file.compress(FCOMPRESS_NONE);
	file.seek(0, SEEK_SET);
	UINT8 header[HEADER_SIZE];
	if (file.read(header, sizeof(header)) != sizeof(header))
		return STATERR_READ_ERROR;
	file.compress(FCOMPRESS_MEDIUM);

	// verify the header and report an error if it doesn't match
	UINT32 sig = signature();
	if (validate_header(header, m_machine.gamedrv->name, sig, popmessage, "Error: ")  != STATERR_NONE)
		return STATERR_INVALID_HEADER;

	// determine whether or not to flip the data when done
	bool flip = NATIVE_ENDIAN_VALUE_LE_BE((header[9] & SS_MSB_FIRST) != 0, (header[9] & SS_MSB_FIRST) == 0);

	// read all the data, flipping if necessary
	for (state_entry *entry = m_entry_list.first(); entry != NULL; entry = entry->next())
	{
		UINT32 totalsize = entry->m_typesize * entry->m_typecount;
		if (file.read(entry->m_data, totalsize) != totalsize)
			return STATERR_READ_ERROR;

		// handle flipping
		if (flip)
			entry->flip_data();
	}

	// call the post-load functions
	for (state_callback *func = m_postload_list.first(); func != NULL; func = func->next())
		(*func->m_func)(&m_machine, func->m_param);

	return STATERR_NONE;
}


//-------------------------------------------------
//  write_file - writes the data to a file
//-------------------------------------------------

state_save_error state_manager::write_file(emu_file &file)
{
	// if we have illegal registrations, return an error
	if (m_illegal_regs > 0)
		return STATERR_ILLEGAL_REGISTRATIONS;

	// generate the header
	UINT8 header[HEADER_SIZE];
	memcpy(&header[0], s_magic_num, 8);
	header[8] = SAVE_VERSION;
	header[9] = NATIVE_ENDIAN_VALUE_LE_BE(0, SS_MSB_FIRST);
	strncpy((char *)&header[0x0a], m_machine.gamedrv->name, 0x1c - 0x0a);
	UINT32 sig = signature();
	*(UINT32 *)&header[0x1c] = LITTLE_ENDIANIZE_INT32(sig);

	// write the header and turn on compression for the rest of the file
	file.compress(FCOMPRESS_NONE);
	file.seek(0, SEEK_SET);
	if (file.write(header, sizeof(header)) != sizeof(header))
		return STATERR_WRITE_ERROR;
	file.compress(FCOMPRESS_MEDIUM);

	// call the pre-save functions
	for (state_callback *func = m_presave_list.first(); func != NULL; func = func->next())
		(*func->m_func)(&m_machine, func->m_param);

	// then write all the data
	for (state_entry *entry = m_entry_list.first(); entry != NULL; entry = entry->next())
	{
		UINT32 totalsize = entry->m_typesize * entry->m_typecount;
		if (file.write(entry->m_data, totalsize) != totalsize)
			return STATERR_WRITE_ERROR;
	}
	return STATERR_NONE;
}


//-------------------------------------------------
//  signature - compute the signature, which
//  is a CRC over the structure of the data
//-------------------------------------------------

UINT32 state_manager::signature() const
{
	// iterate over entries
	UINT32 crc = 0;
	for (state_entry *entry = m_entry_list.first(); entry != NULL; entry = entry->next())
	{
		// add the entry name to the CRC
		crc = crc32(crc, (UINT8 *)entry->m_name.cstr(), entry->m_name.len());

		// add the type and size to the CRC
		UINT32 temp[2];
		temp[0] = LITTLE_ENDIANIZE_INT32(entry->m_typecount);
		temp[1] = LITTLE_ENDIANIZE_INT32(entry->m_typesize);
		crc = crc32(crc, (UINT8 *)&temp[0], sizeof(temp));
	}
	return crc;
}


//-------------------------------------------------
//  dump_registry - dump the registry to the
//  logfile
//-------------------------------------------------

void state_manager::dump_registry() const
{
	for (state_entry *entry = m_entry_list.first(); entry != NULL; entry = entry->next())
		LOG(("%s: %d x %d\n", entry->m_name.cstr(), entry->m_typesize, entry->m_typecount));
}


//-------------------------------------------------
//  validate_header - validate the data in the
//  header
//-------------------------------------------------

state_save_error state_manager::validate_header(const UINT8 *header, const char *gamename, UINT32 signature,
	void (CLIB_DECL *errormsg)(const char *fmt, ...), const char *error_prefix)
{
	// check magic number
	if (memcmp(header, s_magic_num, 8))
	{
		if (errormsg != NULL)
			(*errormsg)("%sThis is not a " APPNAME " save file", error_prefix);
		return STATERR_INVALID_HEADER;
	}

	// check save state version
	if (header[8] != SAVE_VERSION)
	{
		if (errormsg != NULL)
			(*errormsg)("%sWrong version in save file (version %d, expected %d)", error_prefix, header[8], SAVE_VERSION);
		return STATERR_INVALID_HEADER;
	}

	// check gamename, if we were asked to
	if (gamename != NULL && strncmp(gamename, (const char *)&header[0x0a], 0x1c - 0x0a))
	{
		if (errormsg != NULL)
			(*errormsg)("%s'File is not a valid savestate file for game '%s'.", error_prefix, gamename);
		return STATERR_INVALID_HEADER;
	}

	// check signature, if we were asked to
	if (signature != 0)
	{
		UINT32 rawsig = *(UINT32 *)&header[0x1c];
		if (signature != LITTLE_ENDIANIZE_INT32(rawsig))
		{
			if (errormsg != NULL)
				(*errormsg)("%sIncompatible save file (signature %08x, expected %08x)", error_prefix, LITTLE_ENDIANIZE_INT32(rawsig), signature);
			return STATERR_INVALID_HEADER;
		}
	}
	return STATERR_NONE;
}


//-------------------------------------------------
//  state_callback - constructor
//-------------------------------------------------

state_manager::state_callback::state_callback(prepost_func callback, void *param)
	: m_next(NULL),
	  m_param(param),
	  m_func(callback)
{
}


//-------------------------------------------------
//  state_entry - constructor
//-------------------------------------------------

state_manager::state_entry::state_entry(void *data, const char *name, UINT8 size, UINT32 count)
	: m_next(NULL),
	  m_data(data),
	  m_name(name),
	  m_typesize(size),
	  m_typecount(count),
	  m_offset(0)
{
}


//-------------------------------------------------
//  flip_data - reverse the endianness of a
//  block of  data
//-------------------------------------------------

void state_manager::state_entry::flip_data()
{
	UINT16 *data16;
	UINT32 *data32;
	UINT64 *data64;
	int count;

	switch (m_typesize)
	{
		case 2:
			data16 = (UINT16 *)m_data;
			for (count = 0; count < m_typecount; count++)
				data16[count] = FLIPENDIAN_INT16(data16[count]);
			break;

		case 4:
			data32 = (UINT32 *)m_data;
			for (count = 0; count < m_typecount; count++)
				data32[count] = FLIPENDIAN_INT32(data32[count]);
			break;

		case 8:
			data64 = (UINT64 *)m_data;
			for (count = 0; count < m_typecount; count++)
				data64[count] = FLIPENDIAN_INT64(data64[count]);
			break;
	}
}
