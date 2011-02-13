/***************************************************************************

    asap.h

    Core implementation for the portable ASAP emulator.
    ASAP = Atari Simplified Architecture Processor

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

#ifndef __ASAP_H__
#define __ASAP_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> asap_device_config

class asap_device_config :	public cpu_device_config
{
	friend class asap_device;

	// construction/destruction
	asap_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

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
	virtual const address_space_config *memory_space_config(int spacenum = 0) const;

	// device_config_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;

	// inline data
	const address_space_config		m_program_config;
};



// ======================> asap_device

class asap_device : public cpu_device
{
	friend class asap_device_config;

	// construction/destruction
	asap_device(running_machine &_machine, const asap_device_config &config);

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

	// helpers
	inline UINT32 readop(offs_t pc);
	inline UINT8 readbyte(offs_t address);
	inline UINT16 readword(offs_t address);
	inline UINT32 readlong(offs_t address);
	inline void writebyte(offs_t address, UINT8 data);
	inline void writeword(offs_t address, UINT16 data);
	inline void writelong(offs_t address, UINT32 data);
	inline void generate_exception(int exception);
	inline void check_irqs();
	inline void fetch_instruction();
	inline void fetch_instruction_debug();
	inline void execute_instruction();

	// condition handlers
	void bsp();
	void bmz();
	void bgt();
	void ble();
	void bge();
	void blt();
	void bhi();
	void bls();
	void bcc();
	void bcs();
	void bpl();
	void bmi();
	void bne();
	void beq();
	void bvc();
	void bvs();

	// opcode handlers
	void noop();
	void trap0();
	void bsr();
	void bsr_0();
	void lea();
	void lea_c();
	void lea_c0();
	void leah();
	void leah_c();
	void leah_c0();
	void subr();
	void subr_c();
	void subr_c0();
	void xor_();
	void xor_c();
	void xor_c0();
	void xorn();
	void xorn_c();
	void xorn_c0();
	void add();
	void add_c();
	void add_c0();
	void sub();
	void sub_c();
	void sub_c0();
	void addc();
	void addc_c();
	void addc_c0();
	void subc();
	void subc_c();
	void subc_c0();
	void and_();
	void and_c();
	void and_c0();
	void andn();
	void andn_c();
	void andn_c0();
	void or_();
	void or_c();
	void or_c0();
	void orn();
	void orn_c();
	void orn_c0();
	void ld();
	void ld_0();
	void ld_c();
	void ld_c0();
	void ldh();
	void ldh_0();
	void ldh_c();
	void ldh_c0();
	void lduh();
	void lduh_0();
	void lduh_c();
	void lduh_c0();
	void sth();
	void sth_0();
	void sth_c();
	void sth_c0();
	void st();
	void st_0();
	void st_c();
	void st_c0();
	void ldb();
	void ldb_0();
	void ldb_c();
	void ldb_c0();
	void ldub();
	void ldub_0();
	void ldub_c();
	void ldub_c0();
	void stb();
	void stb_0();
	void stb_c();
	void stb_c0();
	void ashr();
	void ashr_c();
	void ashr_c0();
	void lshr();
	void lshr_c();
	void lshr_c0();
	void ashl();
	void ashl_c();
	void ashl_c0();
	void rotl();
	void rotl_c();
	void rotl_c0();
	void getps();
	void putps();
	void jsr();
	void jsr_0();
	void jsr_c();
	void jsr_c0();
	void trapf();

	// internal state
	UINT32				m_pc;

	// expanded flags
	UINT32				m_pflag;
	UINT32				m_iflag;
	UINT32				m_cflag;
	UINT32				m_vflag;
	UINT32				m_znflag;
	UINT32				m_flagsio;

	// internal stuff
	UINT32				m_op;
	UINT32				m_ppc;
	UINT32				m_nextpc;
	UINT8				m_irq_state;
	int					m_icount;
	address_space *		m_program;
	direct_read_data *	m_direct;

	// src2val table, registers are at the end
	UINT32				m_src2val[65536];

	// opcode/condition tables
	typedef void (asap_device::*ophandler)();

	ophandler			m_opcode[32 * 32 * 2];

	static const ophandler s_opcodetable[32][4];
	static const ophandler s_conditiontable[16];
};



//**************************************************************************
//  ENUMERATIONS
//**************************************************************************

// registers
enum
{
	ASAP_PC = 1,
	ASAP_PS,
	ASAP_R0,
	ASAP_R1,
	ASAP_R2,
	ASAP_R3,
	ASAP_R4,
	ASAP_R5,
	ASAP_R6,
	ASAP_R7,
	ASAP_R8,
	ASAP_R9,
	ASAP_R10,
	ASAP_R11,
	ASAP_R12,
	ASAP_R13,
	ASAP_R14,
	ASAP_R15,
	ASAP_R16,
	ASAP_R17,
	ASAP_R18,
	ASAP_R19,
	ASAP_R20,
	ASAP_R21,
	ASAP_R22,
	ASAP_R23,
	ASAP_R24,
	ASAP_R25,
	ASAP_R26,
	ASAP_R27,
	ASAP_R28,
	ASAP_R29,
	ASAP_R30,
	ASAP_R31
};

// input lines
enum
{
	ASAP_IRQ0
};



// device type definition
extern const device_type ASAP;


#endif /* __ASAP_H__ */
