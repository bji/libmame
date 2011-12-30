/***************************************************************************

    upd7725.h

    Core implementation for the portable NEC uPD7725/uPD96050 emulator

****************************************************************************/

#pragma once

#ifndef __UPD7725_H__
#define __UPD7725_H__

//**************************************************************************
//  ENUMERATIONS
//**************************************************************************

// input lines
enum
{
	NECDSP_INPUT_LINE_INT = 0
	// add more here as needed
};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class necdsp_device;
class upd7725_device;
class upd96050_device;

// ======================> necdsp_interface

struct necdsp_interface
{
	devcb_read_line		m_in_int_cb;
	//devcb_read8       m_in_si_cb;
	//devcb_read_line   m_in_sck_cb;
	//devcb_read_line   m_in_sien_cb;
	//devcb_read_line   m_in_soen_cb;
	//devcb_read_line   m_in_dack_cb;
	devcb_write_line	m_out_p0_cb;
	devcb_write_line	m_out_p1_cb;
	//devcb_write8      m_out_so_cb;
	//devcb_write_line  m_out_sorq_cb;
	//devcb_write_line  m_out_drq_cb;
};

#define NECDSP_INTERFACE(name) \
	const necdsp_interface (name) =

// ======================> necdsp_device

class necdsp_device : public cpu_device, public necdsp_interface
{
protected:
	// construction/destruction
	necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, UINT32 abits, UINT32 dbits, const char *name);

public:
	UINT8 snesdsp_read(bool mode);
	void snesdsp_write(bool mode, UINT8 data);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;
	virtual UINT32 execute_input_lines() const;
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_export(const device_state_entry &entry, astring &string);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	// inline data
	const address_space_config m_program_config, m_data_config;

	UINT16 dataRAM[2048];

private:
	struct Flag
	{
		bool s1, s0, c, z, ov1, ov0, ov0p, ov0pp;

		inline operator unsigned() const
		{
			return (s1 << 7) + (s0 << 6) + (c << 5) + (z << 4) + (ov1 << 3) + (ov0 << 2) + (ov0p << 1) + (ov0pp << 0);
		}

		inline unsigned operator=(unsigned d)
		{
			s1 = d & 0x80; s0 = d & 0x40; c = d & 0x20; z = d & 0x10; ov1 = d & 0x08; ov0 = d & 0x04; ov0p = d & 0x02; ov0pp = d & 0x01;
			return d;
		}
	};

	struct Status
	{
		bool rqm, usf1, usf0, drs, dma, drc, soc, sic, ei, p1, p0;

		inline operator unsigned() const
		{
			return (rqm << 15) + (usf1 << 14) + (usf0 << 13) + (drs << 12)
			   + (dma << 11) + (drc  << 10) + (soc  <<  9) + (sic <<  8)
			   + (ei  <<  7) + (p1   <<  1) + (p0   <<  0);
		}

		inline unsigned operator=(unsigned d)
		{
			rqm = d & 0x8000; usf1 = d & 0x4000; usf0 = d & 0x2000; drs = d & 0x1000;
			dma = d & 0x0800; drc  = d & 0x0400; soc  = d & 0x0200; sic = d & 0x0100;
			ei  = d & 0x0080; p1   = d & 0x0002; p0   = d & 0x0001;
			return d;
		}
	};

	struct Regs
	{
		UINT16 pc;			//program counter
		UINT16 stack[16];	//LIFO
		UINT16 rp;			//ROM pointer
		UINT16 dp;			//data pointer
		UINT8  sp;			//stack pointer
		INT16  k;
		INT16  l;
		INT16  m;
		INT16  n;
		INT16  a;         //accumulator
		INT16  b;         //accumulator
		Flag  flaga;
		Flag  flagb;
		UINT16 tr;        //temporary register
		UINT16 trb;       //temporary register
		Status sr;        //status register
		UINT16 dr;        //data register
		UINT16 si;
		UINT16 so;
		UINT16 idb;
	} regs;

  void exec_op(UINT32 opcode);
  void exec_rt(UINT32 opcode);
  void exec_jp(UINT32 opcode);
  void exec_ld(UINT32 opcode);

  void stack_push();
  void stack_pull();

  int m_icount;
  int m_irq; // old irq line state, for detecting rising edges.

  address_space *m_program, *m_data;
  direct_read_data *m_direct;

protected:
// device callbacks
	devcb_resolved_read_line	m_in_int_func;
	//devcb_resolved_read8      m_in_si_func;
	//devcb_resolved_read_line  m_in_sck_func;
	//devcb_resolved_read_line  m_in_sien_func;
	//devcb_resolved_read_line  m_in_soen_func;
	//devcb_resolved_read_line  m_in_dack_func;
	devcb_resolved_write_line	m_out_p0_func;
	devcb_resolved_write_line	m_out_p1_func;
	//devcb_resolved_write8     m_out_so_func;
	//devcb_resolved_write_line m_out_sorq_func;
	//devcb_resolved_write_line m_out_drq_func;
};

class upd7725_device : public necdsp_device
{
public:
	// construction/destruction
	upd7725_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class upd96050_device : public necdsp_device
{
public:
	// construction/destruction
	upd96050_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	UINT16 dataram_r(UINT16 addr) { return dataRAM[addr]; }
	void dataram_w(UINT16 addr, UINT16 data) { dataRAM[addr] = data; }
};

// device type definition
extern const device_type UPD7725;
extern const device_type UPD96050;

//**************************************************************************
//  ENUMERATIONS
//**************************************************************************

// registers
enum
{
	UPD7725_PC = 1,
	UPD7725_RP,
	UPD7725_DP,
	UPD7725_K,
	UPD7725_L,
	UPD7725_M,
	UPD7725_N,
	UPD7725_A,
	UPD7725_B,
	UPD7725_FLAGA,
	UPD7725_FLAGB,
	UPD7725_SR,
	UPD7725_DR,
	UPD7725_SP,
	UPD7725_TR,
	UPD7725_TRB,
	UPD7725_SI,
	UPD7725_SO,
	UPD7725_IDB
};

#endif /* __UPD7725_H__ */
