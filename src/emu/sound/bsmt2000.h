/***************************************************************************

    bsmt2000.h

    BSMT2000 device emulator.

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

#ifndef __BSMT2000_H__
#define __BSMT2000_H__

#include "cpu/tms32010/tms32010.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BSMT2000_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, BSMT2000, _clock) \

#define MCFG_BSMT2000_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, BSMT2000, _clock) \

#define MCFG_BSMT2000_READY_CALLBACK(_callback) \
	bsmt2000_device_config::static_set_ready_callback(device, _callback); \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bsmt2000_device;


// ======================> bsmt2000_device_config

class bsmt2000_device_config :	public device_config,
								public device_config_sound_interface,
								public device_config_memory_interface
{
	friend class bsmt2000_device;

	typedef void (*ready_callback)(bsmt2000_device &device);

	// construction/destruction
	bsmt2000_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

	// inline configuration helpers
	static void static_set_ready_callback(device_config *device, ready_callback callback);

protected:
	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	// device_config overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// internal state
	const address_space_config  m_space_config;

	// inline data
	ready_callback				m_ready_callback;
};



// ======================> bsmt2000_device

class bsmt2000_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
	friend class bsmt2000_device_config;

	// construction/destruction
	bsmt2000_device(running_machine &_machine, const bsmt2000_device_config &config);

public:
	UINT16 read_status();
	void write_reg(UINT16 data);
	void write_data(UINT16 data);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// sound interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	// internal TMS I/O callbacks
	DECLARE_READ16_MEMBER( tms_register_r );
	DECLARE_READ16_MEMBER( tms_data_r );
	DECLARE_READ16_MEMBER( tms_rom_r );
	DECLARE_WRITE16_MEMBER( tms_rom_addr_w );
	DECLARE_WRITE16_MEMBER( tms_rom_bank_w );
	DECLARE_WRITE16_MEMBER( tms_left_w );
	DECLARE_WRITE16_MEMBER( tms_right_w );
	DECLARE_READ16_MEMBER( tms_write_pending_r );

private:
	// timers
	enum
	{
		TIMER_ID_RESET,
		TIMER_ID_REG_WRITE,
		TIMER_ID_DATA_WRITE
	};

	// internal state
	const bsmt2000_device_config &m_config;
	sound_stream *				m_stream;
	direct_read_data *			m_direct;
	tms32015_device *			m_cpu;
	UINT16						m_register_select;
	UINT16						m_write_data;
	UINT16						m_rom_address;
	UINT16						m_rom_bank;
	INT16						m_left_data;
	INT16						m_right_data;
	bool						m_write_pending;
};


// device type definition
extern const device_type BSMT2000;


#endif /* __BSMT2000_H__ */
