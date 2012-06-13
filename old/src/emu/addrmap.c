/***************************************************************************

    addrmap.c

    Macros and helper functions for handling address map definitions.

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


//**************************************************************************
//  ADDRESS MAP ENTRY
//**************************************************************************

//-------------------------------------------------
//  set_tag - set the appropriate tag for a device
//-------------------------------------------------

inline void map_handler_data::set_tag(const device_config &devconfig, const char *tag)
{
	if (tag == NULL)
		m_tag = NULL;
	else if (strcmp(tag, DEVICE_SELF) == 0)
		m_tag = devconfig.tag();
	else if (strcmp(tag, DEVICE_SELF_OWNER) == 0)
	{
		assert(devconfig.owner() != NULL);
		m_tag = devconfig.owner()->tag();
	}
	else
		m_tag = devconfig.siblingtag(m_derived_tag, tag);
}



//**************************************************************************
//  ADDRESS MAP ENTRY
//**************************************************************************

//-------------------------------------------------
//  address_map_entry - constructor
//-------------------------------------------------

address_map_entry::address_map_entry(address_map &map, offs_t start, offs_t end)
	: m_next(NULL),
	  m_map(map),
	  m_addrstart((map.m_globalmask == 0) ? start : start & map.m_globalmask),
	  m_addrend((map.m_globalmask == 0) ? end : end & map.m_globalmask),
	  m_addrmirror(0),
	  m_addrmask(0),
	  m_share(NULL),
	  m_baseptr(NULL),
	  m_sizeptr(NULL),
	  m_baseptroffs_plus1(0),
	  m_sizeptroffs_plus1(0),
	  m_genbaseptroffs_plus1(0),
	  m_gensizeptroffs_plus1(0),
	  m_region(NULL),
	  m_rgnoffs(0),
	  m_rspace8(NULL),
	  m_rspace16(NULL),
	  m_rspace32(NULL),
	  m_rspace64(NULL),
	  m_rdevice8(NULL),
	  m_rdevice16(NULL),
	  m_rdevice32(NULL),
	  m_rdevice64(NULL),
	  m_wspace8(NULL),
	  m_wspace16(NULL),
	  m_wspace32(NULL),
	  m_wspace64(NULL),
	  m_wdevice8(NULL),
	  m_wdevice16(NULL),
	  m_wdevice32(NULL),
	  m_wdevice64(NULL),
	  m_memory(NULL),
	  m_bytestart(0),
	  m_byteend(0),
	  m_bytemirror(0),
	  m_bytemask(0)
{
}


//-------------------------------------------------
//  set_mask - set the mask value
//-------------------------------------------------

void address_map_entry::set_mask(offs_t _mask)
{
	m_addrmask = _mask;
	if (m_map.m_globalmask != 0)
		m_addrmask &= m_map.m_globalmask;
}


//-------------------------------------------------
//  set_read_port - set up a handler for reading
//  an I/O port
//-------------------------------------------------

void address_map_entry::set_read_port(const device_config &devconfig, const char *tag)
{
	m_read.m_type = AMH_PORT;
	m_read.set_tag(devconfig, tag);
}


//-------------------------------------------------
//  set_write_port - set up a handler for writing
//  an I/O port
//-------------------------------------------------

void address_map_entry::set_write_port(const device_config &devconfig, const char *tag)
{
	m_write.m_type = AMH_PORT;
	m_write.set_tag(devconfig, tag);
}


//-------------------------------------------------
//  set_readwrite_port - set up a handler for
//  reading and writing an I/O port
//-------------------------------------------------

void address_map_entry::set_readwrite_port(const device_config &devconfig, const char *tag)
{
	m_read.m_type = AMH_PORT;
	m_read.set_tag(devconfig, tag);
	m_write.m_type = AMH_PORT;
	m_write.set_tag(devconfig, tag);
}


//-------------------------------------------------
//  set_read_bank - set up a handler for reading
//  from a memory bank
//-------------------------------------------------

void address_map_entry::set_read_bank(const device_config &devconfig, const char *tag)
{
	m_read.m_type = AMH_BANK;
	m_read.set_tag(devconfig, tag);
}


//-------------------------------------------------
//  set_write_bank - set up a handler for writing
//  to a memory bank
//-------------------------------------------------

void address_map_entry::set_write_bank(const device_config &devconfig, const char *tag)
{
	m_write.m_type = AMH_BANK;
	m_write.set_tag(devconfig, tag);
}


//-------------------------------------------------
//  set_readwrite_bank - set up a handler for
//  writing to a memory bank
//-------------------------------------------------

void address_map_entry::set_readwrite_bank(const device_config &devconfig, const char *tag)
{
	m_read.m_type = AMH_BANK;
	m_read.set_tag(devconfig, tag);
	m_write.m_type = AMH_BANK;
	m_write.set_tag(devconfig, tag);
}


//-------------------------------------------------
//  internal_set_handler - handler setters for
//  8-bit read/write handlers
//-------------------------------------------------

void address_map_entry::internal_set_handler(read8_space_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(8, unitmask, string));
	m_read.m_type = AMH_LEGACY_SPACE_HANDLER;
	m_read.m_bits = 8;
	m_read.m_mask = unitmask;
	m_read.m_name = string;
	m_rspace8 = func;
}


void address_map_entry::internal_set_handler(write8_space_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(8, unitmask, string));
	m_write.m_type = AMH_LEGACY_SPACE_HANDLER;
	m_write.m_bits = 8;
	m_write.m_mask = unitmask;
	m_write.m_name = string;
	m_wspace8 = func;
}


void address_map_entry::internal_set_handler(read8_space_func rfunc, const char *rstring, write8_space_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(rfunc, rstring, unitmask);
	internal_set_handler(wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read8_device_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(8, unitmask, string));
	m_read.m_type = AMH_LEGACY_DEVICE_HANDLER;
	m_read.m_bits = 8;
	m_read.m_mask = unitmask;
	m_read.m_name = string;
	m_read.set_tag(devconfig, tag);
	m_rdevice8 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write8_device_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(8, unitmask, string));
	m_write.m_type = AMH_LEGACY_DEVICE_HANDLER;
	m_write.m_bits = 8;
	m_write.m_mask = unitmask;
	m_write.m_name = string;
	m_write.set_tag(devconfig, tag);
	m_wdevice8 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read8_device_func rfunc, const char *rstring, write8_device_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, rstring, unitmask);
	internal_set_handler(devconfig, tag, wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read8_proto_delegate func, UINT64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(8, unitmask, func.name()));
	m_read.m_type = (tag == NULL) ? AMH_DRIVER_DELEGATE : AMH_DEVICE_DELEGATE;
	m_read.m_bits = 8;
	m_read.m_mask = unitmask;
	m_read.m_name = func.name();
	m_read.set_tag(devconfig, tag);
	m_rproto8 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write8_proto_delegate func, UINT64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(8, unitmask, func.name()));
	m_write.m_type = (tag == NULL) ? AMH_DRIVER_DELEGATE : AMH_DEVICE_DELEGATE;;
	m_write.m_bits = 8;
	m_write.m_mask = unitmask;
	m_write.m_name = func.name();
	m_write.set_tag(devconfig, tag);
	m_wproto8 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read8_proto_delegate rfunc, write8_proto_delegate wfunc, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, unitmask);
	internal_set_handler(devconfig, tag, wfunc, unitmask);
}


//-------------------------------------------------
//  internal_set_handler - handler setters for
//  16-bit read/write handlers
//-------------------------------------------------

void address_map_entry::internal_set_handler(read16_space_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(16, unitmask, string));
	m_read.m_type = AMH_LEGACY_SPACE_HANDLER;
	m_read.m_bits = 16;
	m_read.m_mask = unitmask;
	m_read.m_name = string;
	m_rspace16 = func;
}


void address_map_entry::internal_set_handler(write16_space_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(16, unitmask, string));
	m_write.m_type = AMH_LEGACY_SPACE_HANDLER;
	m_write.m_bits = 16;
	m_write.m_mask = unitmask;
	m_write.m_name = string;
	m_wspace16 = func;
}


void address_map_entry::internal_set_handler(read16_space_func rfunc, const char *rstring, write16_space_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(rfunc, rstring, unitmask);
	internal_set_handler(wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read16_device_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(16, unitmask, string));
	m_read.m_type = AMH_LEGACY_DEVICE_HANDLER;
	m_read.m_bits = 16;
	m_read.m_mask = unitmask;
	m_read.m_name = string;
	m_read.set_tag(devconfig, tag);
	m_rdevice16 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write16_device_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(16, unitmask, string));
	m_write.m_type = AMH_LEGACY_DEVICE_HANDLER;
	m_write.m_bits = 16;
	m_write.m_mask = unitmask;
	m_write.m_name = string;
	m_write.set_tag(devconfig, tag);
	m_wdevice16 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read16_device_func rfunc, const char *rstring, write16_device_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, rstring, unitmask);
	internal_set_handler(devconfig, tag, wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read16_proto_delegate func, UINT64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(16, unitmask, func.name()));
	m_read.m_type = (tag == NULL) ? AMH_DRIVER_DELEGATE : AMH_DEVICE_DELEGATE;;
	m_read.m_bits = 16;
	m_read.m_mask = unitmask;
	m_read.m_name = func.name();
	m_read.set_tag(devconfig, tag);
	m_rproto16 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write16_proto_delegate func, UINT64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(16, unitmask, func.name()));
	m_write.m_type = (tag == NULL) ? AMH_DRIVER_DELEGATE : AMH_DEVICE_DELEGATE;;
	m_write.m_bits = 16;
	m_write.m_mask = unitmask;
	m_write.m_name = func.name();
	m_write.set_tag(devconfig, tag);
	m_wproto16 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read16_proto_delegate rfunc, write16_proto_delegate wfunc, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, unitmask);
	internal_set_handler(devconfig, tag, wfunc, unitmask);
}


//-------------------------------------------------
//  internal_set_handler - handler setters for
//  32-bit read/write handlers
//-------------------------------------------------

void address_map_entry::internal_set_handler(read32_space_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(32, unitmask, string));
	m_read.m_type = AMH_LEGACY_SPACE_HANDLER;
	m_read.m_bits = 32;
	m_read.m_mask = unitmask;
	m_read.m_name = string;
	m_rspace32 = func;
}


void address_map_entry::internal_set_handler(write32_space_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(32, unitmask, string));
	m_write.m_type = AMH_LEGACY_SPACE_HANDLER;
	m_write.m_bits = 32;
	m_write.m_mask = unitmask;
	m_write.m_name = string;
	m_wspace32 = func;
}


void address_map_entry::internal_set_handler(read32_space_func rfunc, const char *rstring, write32_space_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(rfunc, rstring, unitmask);
	internal_set_handler(wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read32_device_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(32, unitmask, string));
	m_read.m_type = AMH_LEGACY_DEVICE_HANDLER;
	m_read.m_bits = 32;
	m_read.m_mask = unitmask;
	m_read.m_name = string;
	m_read.set_tag(devconfig, tag);
	m_rdevice32 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write32_device_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(32, unitmask, string));
	m_write.m_type = AMH_LEGACY_DEVICE_HANDLER;
	m_write.m_bits = 32;
	m_write.m_mask = unitmask;
	m_write.m_name = string;
	m_write.set_tag(devconfig, tag);
	m_wdevice32 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read32_device_func rfunc, const char *rstring, write32_device_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, rstring, unitmask);
	internal_set_handler(devconfig, tag, wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read32_proto_delegate func, UINT64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(32, unitmask, func.name()));
	m_read.m_type = (tag == NULL) ? AMH_DRIVER_DELEGATE : AMH_DEVICE_DELEGATE;;
	m_read.m_bits = 32;
	m_read.m_mask = unitmask;
	m_read.m_name = func.name();
	m_read.set_tag(devconfig, tag);
	m_rproto32 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write32_proto_delegate func, UINT64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(32, unitmask, func.name()));
	m_write.m_type = (tag == NULL) ? AMH_DRIVER_DELEGATE : AMH_DEVICE_DELEGATE;;
	m_write.m_bits = 32;
	m_write.m_mask = unitmask;
	m_write.m_name = func.name();
	m_write.set_tag(devconfig, tag);
	m_wproto32 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read32_proto_delegate rfunc, write32_proto_delegate wfunc, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, unitmask);
	internal_set_handler(devconfig, tag, wfunc, unitmask);
}


//-------------------------------------------------
//  internal_set_handler - handler setters for
//  64-bit read/write handlers
//-------------------------------------------------

void address_map_entry::internal_set_handler(read64_space_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(64, unitmask, string));
	m_read.m_type = AMH_LEGACY_SPACE_HANDLER;
	m_read.m_bits = 64;
	m_read.m_mask = 0;
	m_read.m_name = string;
	m_rspace64 = func;
}


void address_map_entry::internal_set_handler(write64_space_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(64, unitmask, string));
	m_write.m_type = AMH_LEGACY_SPACE_HANDLER;
	m_write.m_bits = 64;
	m_write.m_mask = 0;
	m_write.m_name = string;
	m_wspace64 = func;
}


void address_map_entry::internal_set_handler(read64_space_func rfunc, const char *rstring, write64_space_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(rfunc, rstring, unitmask);
	internal_set_handler(wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read64_device_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(64, unitmask, string));
	m_read.m_type = AMH_LEGACY_DEVICE_HANDLER;
	m_read.m_bits = 64;
	m_read.m_mask = 0;
	m_read.m_name = string;
	m_read.set_tag(devconfig, tag);
	m_rdevice64 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write64_device_func func, const char *string, UINT64 unitmask)
{
	assert(func != NULL);
	assert(unitmask_is_appropriate(64, unitmask, string));
	m_write.m_type = AMH_LEGACY_DEVICE_HANDLER;
	m_write.m_bits = 64;
	m_write.m_mask = 0;
	m_write.m_name = string;
	m_write.set_tag(devconfig, tag);
	m_wdevice64 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read64_device_func rfunc, const char *rstring, write64_device_func wfunc, const char *wstring, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, rstring, unitmask);
	internal_set_handler(devconfig, tag, wfunc, wstring, unitmask);
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read64_proto_delegate func, UINT64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(64, unitmask, func.name()));
	m_read.m_type = (tag == NULL) ? AMH_DRIVER_DELEGATE : AMH_DEVICE_DELEGATE;;
	m_read.m_bits = 64;
	m_read.m_mask = 0;
	m_read.m_name = func.name();
	m_read.set_tag(devconfig, tag);
	m_rproto64 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, write64_proto_delegate func, UINT64 unitmask)
{
	assert(!func.isnull());
	assert(unitmask_is_appropriate(64, unitmask, func.name()));
	m_write.m_type = (tag == NULL) ? AMH_DRIVER_DELEGATE : AMH_DEVICE_DELEGATE;;
	m_write.m_bits = 64;
	m_write.m_mask = 0;
	m_write.m_name = func.name();
	m_write.set_tag(devconfig, tag);
	m_wproto64 = func;
}


void address_map_entry::internal_set_handler(const device_config &devconfig, const char *tag, read64_proto_delegate rfunc, write64_proto_delegate wfunc, UINT64 unitmask)
{
	internal_set_handler(devconfig, tag, rfunc, unitmask);
	internal_set_handler(devconfig, tag, wfunc, unitmask);
}


//-------------------------------------------------
//  unitmask_is_appropriate - verify that the
//  provided unitmask is valid and expected
//-------------------------------------------------

bool address_map_entry::unitmask_is_appropriate(UINT8 width, UINT64 unitmask, const char *string)
{
	// if no mask, this must match the default width of the map
	if (unitmask == 0)
	{
		if (m_map.m_databits != width)
			throw emu_fatalerror("Handler %s is a %d-bit handler but was specified in a %d-bit address map", string, width, m_map.m_databits);
		return true;
	}

	// if we have a mask, we must be smaller than the default width of the map
	if (m_map.m_databits <= width)
		throw emu_fatalerror("Handler %s is a %d-bit handler and is too wide to be used in a %d-bit address map", string, width, m_map.m_databits);

	// the mask must represent whole units of width
	UINT32 basemask = (width == 8) ? 0xff : (width == 16) ? 0xffff : 0xffffffff;
	UINT64 singlemask = basemask;
	while (singlemask != 0)
	{
		if ((unitmask & singlemask) != 0 && (unitmask & singlemask) != singlemask)
			throw emu_fatalerror("Handler %s specified a mask of %08X%08X; needs to be in even chunks of %X", string, (UINT32)(unitmask >> 32), (UINT32)unitmask, basemask);
		singlemask <<= width;
	}
	return true;
}



//**************************************************************************
//  WIDTH-SPECIFIC ADDRESS MAP ENTRY CONSTRUCTORS
//**************************************************************************

//-------------------------------------------------
//  address_map_entry8 - constructor
//-------------------------------------------------

address_map_entry8::address_map_entry8(address_map &map, offs_t start, offs_t end)
	: address_map_entry(map, start, end)
{
}


//-------------------------------------------------
//  address_map_entry16 - constructor
//-------------------------------------------------

address_map_entry16::address_map_entry16(address_map &map, offs_t start, offs_t end)
	: address_map_entry(map, start, end)
{
}


//-------------------------------------------------
//  address_map_entry32 - constructor
//-------------------------------------------------

address_map_entry32::address_map_entry32(address_map &map, offs_t start, offs_t end)
	: address_map_entry(map, start, end)
{
}


//-------------------------------------------------
//  address_map_entry64 - constructor
//-------------------------------------------------

address_map_entry64::address_map_entry64(address_map &map, offs_t start, offs_t end)
	: address_map_entry(map, start, end)
{
}



//**************************************************************************
//  ADDRESS MAP
//**************************************************************************

//-------------------------------------------------
//  address_map - constructor
//-------------------------------------------------

address_map::address_map(const device_config &devconfig, int spacenum)
	: m_spacenum(spacenum),
	  m_databits(0xff),
	  m_unmapval(0),
	  m_globalmask(0)
{
	// get our memory interface
	const device_config_memory_interface *memintf;
	if (!devconfig.interface(memintf))
		throw emu_fatalerror("No memory interface defined for device '%s'\n", devconfig.tag());

	// and then the configuration for the current address space
	const address_space_config *spaceconfig = memintf->space_config(spacenum);
	if (!devconfig.interface(memintf))
		throw emu_fatalerror("No memory address space configuration found for device '%s', space %d\n", devconfig.tag(), spacenum);

	// append the internal device map (first so it takes priority) */
	if (spaceconfig->m_internal_map != NULL)
		(*spaceconfig->m_internal_map)(*this, devconfig);

	// construct the standard map */
	if (memintf->address_map(spacenum) != NULL)
		(*memintf->address_map(spacenum))(*this, devconfig);

	// append the default device map (last so it can be overridden) */
	if (spaceconfig->m_default_map != NULL)
		(*spaceconfig->m_default_map)(*this, devconfig);
}


//-------------------------------------------------
//  ~address_map - destructor
//-------------------------------------------------

address_map::~address_map()
{
}


//-------------------------------------------------
//  configure - either configure the space and
//  databits, or verify they match previously-set
//  values
//-------------------------------------------------

void address_map::configure(UINT8 spacenum, UINT8 databits)
{
	assert(m_spacenum == spacenum);
	if (m_databits == 0xff)
		m_databits = databits;
	else
		assert(m_databits == databits);
}


//-------------------------------------------------
//  append - append an entry to the end of the
//  list
//-------------------------------------------------

void address_map::set_global_mask(offs_t mask)
{
//  if (m_entrylist != NULL)
//      throw emu_fatalerror("AM_GLOBALMASK must be specified before any entries");
	m_globalmask = mask;
}



//-------------------------------------------------
//  add - add a new entry of the appropriate type
//-------------------------------------------------

address_map_entry8 *address_map::add(offs_t start, offs_t end, address_map_entry8 *ptr)
{
	ptr = global_alloc(address_map_entry8(*this, start, end));
	m_entrylist.append(*ptr);
	return ptr;
}


address_map_entry16 *address_map::add(offs_t start, offs_t end, address_map_entry16 *ptr)
{
	ptr = global_alloc(address_map_entry16(*this, start, end));
	m_entrylist.append(*ptr);
	return ptr;
}


address_map_entry32 *address_map::add(offs_t start, offs_t end, address_map_entry32 *ptr)
{
	ptr = global_alloc(address_map_entry32(*this, start, end));
	m_entrylist.append(*ptr);
	return ptr;
}


address_map_entry64 *address_map::add(offs_t start, offs_t end, address_map_entry64 *ptr)
{
	ptr = global_alloc(address_map_entry64(*this, start, end));
	m_entrylist.append(*ptr);
	return ptr;
}
