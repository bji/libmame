/*********************************************************************

    bufsprite.h

    Buffered Sprite RAM device.

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

*********************************************************************/

#pragma once

#ifndef __BUFSPRITE_H__
#define __BUFSPRITE_H__



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type BUFFERED_SPRITERAM8;
extern const device_type BUFFERED_SPRITERAM16;
extern const device_type BUFFERED_SPRITERAM32;
extern const device_type BUFFERED_SPRITERAM64;



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BUFFERED_SPRITERAM8_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BUFFERED_SPRITERAM8, 0) \

#define MCFG_BUFFERED_SPRITERAM16_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BUFFERED_SPRITERAM16, 0) \

#define MCFG_BUFFERED_SPRITERAM32_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BUFFERED_SPRITERAM32, 0) \

#define MCFG_BUFFERED_SPRITERAM64_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BUFFERED_SPRITERAM64, 0) \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> buffered_spriteram_device

// base class to manage buffered spriteram
template<typename _Type>
class buffered_spriteram_device : public device_t
{
public:
	// construction
	buffered_spriteram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, type, "Buffered Sprite RAM", tag, owner, clock),
		  m_spriteram(*owner, tag) { }

	// getters
	_Type *live() const { return m_spriteram; }
	_Type *buffer() { return m_buffered; }
	UINT32 bytes() const { return m_spriteram.bytes(); }

	// operations
	_Type *copy(UINT32 srcoffset = 0, UINT32 srclength = 0x7fffffff)
	{
		assert(m_spriteram != NULL);
		if (m_spriteram != NULL)
			memcpy(m_buffered, m_spriteram + srcoffset, MIN(srclength, m_spriteram.bytes() / sizeof(_Type) - srcoffset) * sizeof(_Type));
		return m_buffered;
	}

	// read/write handlers
	void write(address_space &space, offs_t offset, _Type data, _Type mem_mask = ~_Type(0)) { copy(); }

	// VBLANK handlers
	void vblank_copy_rising(screen_device &screen, bool state) { if (state) copy(); }
	void vblank_copy_falling(screen_device &screen, bool state) { if (!state) copy(); }

protected:
	// first-time setup
	virtual void device_start()
	{
		if (m_spriteram != NULL)
		{
			m_buffered.resize(m_spriteram.bytes() / sizeof(_Type));
			save_item(NAME(m_buffered));
		}
	}

private:
	// internal state
	required_shared_ptr<_Type>	m_spriteram;
	dynamic_array<_Type>		m_buffered;
};


// ======================> buffered_spriteram8_device

class buffered_spriteram8_device : public buffered_spriteram_device<UINT8>
{
public:
	// construction
	buffered_spriteram8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: buffered_spriteram_device<UINT8>(mconfig, BUFFERED_SPRITERAM8, tag, owner, clock) { }
};


// ======================> buffered_spriteram16_device

class buffered_spriteram16_device : public buffered_spriteram_device<UINT16>
{
public:
	// construction
	buffered_spriteram16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: buffered_spriteram_device<UINT16>(mconfig, BUFFERED_SPRITERAM16, tag, owner, clock) { }
};


// ======================> buffered_spriteram32_device

class buffered_spriteram32_device : public buffered_spriteram_device<UINT32>
{
public:
	// construction
	buffered_spriteram32_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: buffered_spriteram_device<UINT32>(mconfig, BUFFERED_SPRITERAM32, tag, owner, clock) { }
};


// ======================> buffered_spriteram64_device

class buffered_spriteram64_device : public buffered_spriteram_device<UINT64>
{
public:
	// construction
	buffered_spriteram64_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: buffered_spriteram_device<UINT64>(mconfig, BUFFERED_SPRITERAM64, tag, owner, clock) { }
};


#endif	/* __BUFSPRITE_H__ */
