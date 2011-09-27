/***************************************************************************

    dsp16.h

    WE|AT&T DSP16 series emulator.

***************************************************************************/

#pragma once

#ifndef __DSP16_H__
#define __DSP16_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

//#define MCFG_DSP16_CONFIG(_config)
//  dsp16_device_config::static_set_config(device, _config);



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type DSP16;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dsp16_device;


// ======================> dsp16_device_config

class dsp16_device_config :  public cpu_device_config
{
    friend class dsp16_device;

    // construction/destruction
    dsp16_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

protected:
	// device_config_execute_interface overrides
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;
	virtual UINT32 execute_input_lines() const;

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_config_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;

	// address spaces
	const address_space_config m_program_config;
};



// ======================> dsp16_device

class dsp16_device : public cpu_device
{
	friend class dsp16_device_config;

	// construction/destruction
	dsp16_device(running_machine &_machine, const dsp16_device_config &config);

public:
	// public interfaces

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_export(const device_state_entry &entry, astring &string);

	// device_disasm_interface overrides
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);



	// CPU registers
	UINT16 m_pc;

    // internal stuff
	UINT16 m_ppc;

	// memory access
	inline UINT32 program_read(UINT32 addr);
	inline void program_write(UINT32 addr, UINT32 data);
	inline UINT32 opcode_read();

	// address spaces
    address_space* m_program;
    direct_read_data* m_direct;

	// other internal states
    int m_icount;
};


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	DSP16_PC
};


#endif /* __DSP16_H__ */
