/***************************************************************************

    tms32031.c

    TMS32031/2 emulator

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
#include "debugger.h"
#include "tms32031.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// indexes into the register file
enum
{
	TMR_R0 = 0,
	TMR_R1,
	TMR_R2,
	TMR_R3,
	TMR_R4,
	TMR_R5,
	TMR_R6,
	TMR_R7,
	TMR_AR0,
	TMR_AR1,
	TMR_AR2,
	TMR_AR3,
	TMR_AR4,
	TMR_AR5,
	TMR_AR6,
	TMR_AR7,
	TMR_DP,
	TMR_IR0,
	TMR_IR1,
	TMR_BK,
	TMR_SP,
	TMR_ST,
	TMR_IE,
	TMR_IF,
	TMR_IOF,
	TMR_RS,
	TMR_RE,
	TMR_RC,
	TMR_R8,		// 3204x only
	TMR_R9,		// 3204x only
	TMR_R10,	// 3204x only
	TMR_R11,	// 3204x only
	TMR_TEMP1,	// used by the interpreter
	TMR_TEMP2,	// used by the interpreter
	TMR_TEMP3	// used by the interpreter
};

// flags
const int CFLAG		= 0x0001;
const int VFLAG		= 0x0002;
const int ZFLAG		= 0x0004;
const int NFLAG		= 0x0008;
const int UFFLAG	= 0x0010;
const int LVFLAG	= 0x0020;
const int LUFFLAG	= 0x0040;
const int OVMFLAG	= 0x0080;
const int RMFLAG	= 0x0100;
const int CFFLAG	= 0x0400;
const int CEFLAG	= 0x0800;
const int CCFLAG	= 0x1000;
const int GIEFLAG	= 0x2000;



//**************************************************************************
//  MACROS
//**************************************************************************

#define IREG(rnum)	(m_r[rnum].i32[0])



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device definitions
const device_type TMS32031 = tms32031_device_config::static_alloc_device_config;
const device_type TMS32032 = tms32032_device_config::static_alloc_device_config;

// internal memory maps
static ADDRESS_MAP_START( internal_32031, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x809800, 0x809fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( internal_32032, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x87fe00, 0x87ffff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  TMS3203X DEVICE CONFIG
//**************************************************************************

//-------------------------------------------------
//  tms3203x_device_config - constructor
//-------------------------------------------------

tms3203x_device_config::tms3203x_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 chiptype, address_map_constructor internal_map)
	: cpu_device_config(mconfig, type, name, tag, owner, clock),
	  m_program_config("program", ENDIANNESS_LITTLE, 32, 24, -2, internal_map),
	  m_chip_type(chiptype)
{
	m_bootoffset = 0;
	m_xf0_w = NULL;
	m_xf1_w = NULL;
	m_iack_w = NULL;
}

tms32031_device_config::tms32031_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: tms3203x_device_config(mconfig, static_alloc_device_config, "TMS32031", tag, owner, clock, CHIP_TYPE_TMS32031, ADDRESS_MAP_NAME(internal_32031)) { }

tms32032_device_config::tms32032_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: tms3203x_device_config(mconfig, static_alloc_device_config, "TMS32032", tag, owner, clock, CHIP_TYPE_TMS32032, ADDRESS_MAP_NAME(internal_32032)) { }


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *tms32031_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(tms32031_device_config(mconfig, tag, owner, clock));
}

device_config *tms32032_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(tms32032_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *tms32031_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, tms32031_device(machine, *this));
}

device_t *tms32032_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, tms32032_device(machine, *this));
}


//-------------------------------------------------
//  static_set_config - set the configuration
//  structure
//-------------------------------------------------

void tms3203x_device_config::static_set_config(device_config *device, const tms3203x_config &config)
{
	tms3203x_device_config *tms = downcast<tms3203x_device_config *>(device);
	*static_cast<tms3203x_config *>(tms) = config;
}


//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 tms3203x_device_config::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 tms3203x_device_config::execute_max_cycles() const
{
	return 4;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 tms3203x_device_config::execute_input_lines() const
{
	return 11;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *tms3203x_device_config::memory_space_config(int spacenum) const
{
	return (spacenum == AS_PROGRAM) ? &m_program_config : NULL;
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 tms3203x_device_config::disasm_min_opcode_bytes() const
{
	return 4;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 tms3203x_device_config::disasm_max_opcode_bytes() const
{
	return 4;
}



//**************************************************************************
//  TMSREG REGISTER
//**************************************************************************

//-------------------------------------------------
//  as_float - interpret the contents of a tmsreg
//  as a DSP-encoded floating-point value, and
//  extract a 32-bit IEEE float from it
//-------------------------------------------------

float tms3203x_device::tmsreg::as_float() const
{
	int_double id;

	// map 0 to 0
	if (mantissa() == 0 && exponent() == -128)
		return 0;

	// handle positive numbers
	else if (mantissa() >= 0)
	{
		int exp = (exponent() + 127) << 23;
		id.i[0] = exp + (mantissa() >> 8);
	}

	// handle negative numbers
	else
	{
		int exp = (exponent() + 127) << 23;
		INT32 man = -mantissa();
		id.i[0] = 0x80000000 + exp + ((man >> 8) & 0x00ffffff);
	}

	// return the converted float
	return id.f[0];
}


//-------------------------------------------------
//  as_double - interpret the contents of a tmsreg
//  as a DSP-encoded floating-point value, and
//  extract a 64-bit IEEE double from it
//-------------------------------------------------

double tms3203x_device::tmsreg::as_double() const
{
	int_double id;

	// map 0 to 0
	if (mantissa() == 0 && exponent() == -128)
		return 0;

	// handle positive numbers
	else if (mantissa() >= 0)
	{
		int exp = (exponent() + 1023) << 20;
		id.i[BYTE_XOR_BE(0)] = exp + (mantissa() >> 11);
		id.i[BYTE_XOR_BE(1)] = (mantissa() << 21) & 0xffe00000;
	}

	// handle negative numbers
	else
	{
		int exp = (exponent() + 1023) << 20;
		INT32 man = -mantissa();
		id.i[BYTE_XOR_BE(0)] = 0x80000000 + exp + ((man >> 11) & 0x001fffff);
		id.i[BYTE_XOR_BE(1)] = (man << 21) & 0xffe00000;
	}

	// return the converted double
	return id.d;
}


//-------------------------------------------------
//  from_double - import a 64-bit IEEE double into
//  the DSP's internal floating point format
//-------------------------------------------------

void tms3203x_device::tmsreg::from_double(double val)
{
	// extract mantissa and exponent from the IEEE input
	int_double id;
	id.d = val;
	INT32 mantissa = ((id.i[BYTE_XOR_BE(0)] & 0x000fffff) << 11) | ((id.i[BYTE_XOR_BE(1)] & 0xffe00000) >> 21);
	INT32 exponent = ((id.i[BYTE_XOR_BE(0)] & 0x7ff00000) >> 20) - 1023;

	// if we're too small, map to 0
	if (exponent < -128)
	{
		set_mantissa(0);
		set_exponent(-128);
	}

	// if we're too large, map to the maximum value
	else if (exponent > 127)
	{
		if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
			set_mantissa(0x7fffffff);
		else
			set_mantissa(0x80000001);
		set_exponent(127);
	}

	// if we're positive, map directly
	else if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
	{
		set_mantissa(mantissa);
		set_exponent(exponent);
	}

	// if we're negative with a non-zero mantissa, remove the leading sign bit
	else if (mantissa != 0)
	{
		set_mantissa(0x80000000 | -mantissa);
		set_exponent(exponent);
	}

	// if we're negative with a zero mantissa, normalize
	else
	{
		set_mantissa(0x80000000);
		set_exponent(exponent - 1);
	}
}



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  tms3203x_device - constructor
//-------------------------------------------------

tms3203x_device::tms3203x_device(running_machine &_machine, const tms3203x_device_config &config)
	: cpu_device(_machine, config),
	  m_config(config),
	  m_pc(0),
	  m_bkmask(0),
	  m_irq_state(0),
	  m_delayed(false),
	  m_irq_pending(false),
	  m_mcu_mode(false),
	  m_is_idling(false),
	  m_icount(0),
	  m_irq_callback(0),
	  m_program(0),
	  m_direct(0)
{
	// initialize remaining state
	memset(&m_r, 0, sizeof(m_r));

	// set our instruction counter
	m_icountptr = &m_icount;

#if (TMS_3203X_LOG_OPCODE_USAGE)
	memset(m_hits, 0, sizeof(m_hits));
#endif
}

tms32031_device::tms32031_device(running_machine &_machine, const tms32031_device_config &config)
	: tms3203x_device(_machine, config) { }

tms32032_device::tms32032_device(running_machine &_machine, const tms32032_device_config &config)
	: tms3203x_device(_machine, config) { }


//-------------------------------------------------
//  ~tms3203x_device - destructor
//-------------------------------------------------

tms3203x_device::~tms3203x_device()
{
#if (TMS_3203X_LOG_OPCODE_USAGE)
	for (int i = 0; i < ARRAY_LENGTH(m_hits); i++)
		if (m_hits[i] != 0)
			printf("%10d - %03X.%X\n", m_hits[i], i / 4, i % 4);
#endif
}


//-------------------------------------------------
//  ROPCODE - fetch an opcode
//-------------------------------------------------

inline UINT32 tms3203x_device::ROPCODE(offs_t pc)
{
	return m_direct->read_decrypted_dword(pc << 2);
}


//-------------------------------------------------
//  RMEM - read memory
//-------------------------------------------------

inline UINT32 tms3203x_device::RMEM(offs_t addr)
{
	return m_program->read_dword(addr << 2);
}


//-------------------------------------------------
//  WMEM - write memory
//-------------------------------------------------

inline void tms3203x_device::WMEM(offs_t addr, UINT32 data)
{
	m_program->write_dword(addr << 2, data);
}


//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void tms3203x_device::device_start()
{
	// find address spaces
	m_program = space(AS_PROGRAM);
	m_direct = &m_program->direct();

	// save state
	save_item(NAME(m_pc));
	for (int regnum = 0; regnum < 36; regnum++)
		save_item(NAME(m_r[regnum].i32), regnum);
	save_item(NAME(m_bkmask));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_delayed));
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_mcu_mode));
	save_item(NAME(m_is_idling));

	// register our state for the debugger
	state_add(TMS3203X_PC,      "PC",        m_pc);
	state_add(STATE_GENPC,      "GENPC",     m_pc).noshow();
	state_add(STATE_GENFLAGS,   "GENFLAGS",  m_r[TMR_ST].i32[0]).mask(0xff).noshow().formatstr("%8s");
	state_add(TMS3203X_R0,      "R0",        m_r[TMR_R0].i32[0]);
	state_add(TMS3203X_R1,      "R1",        m_r[TMR_R1].i32[0]);
	state_add(TMS3203X_R2,      "R2",        m_r[TMR_R2].i32[0]);
	state_add(TMS3203X_R3,      "R3",        m_r[TMR_R3].i32[0]);
	state_add(TMS3203X_R4,      "R4",        m_r[TMR_R4].i32[0]);
	state_add(TMS3203X_R5,      "R5",        m_r[TMR_R5].i32[0]);
	state_add(TMS3203X_R6,      "R6",        m_r[TMR_R6].i32[0]);
	state_add(TMS3203X_R7,      "R7",        m_r[TMR_R7].i32[0]);
	state_add(TMS3203X_R0F,     "R0F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R1F,     "R1F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R2F,     "R2F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R3F,     "R3F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R4F,     "R4F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R5F,     "R5F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R6F,     "R6F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_R7F,     "R7F",       m_iotemp).callimport().callexport().formatstr("%12s");
	state_add(TMS3203X_AR0,     "AR0",       m_r[TMR_AR0].i32[0]);
	state_add(TMS3203X_AR1,     "AR1",       m_r[TMR_AR1].i32[0]);
	state_add(TMS3203X_AR2,     "AR2",       m_r[TMR_AR2].i32[0]);
	state_add(TMS3203X_AR3,     "AR3",       m_r[TMR_AR3].i32[0]);
	state_add(TMS3203X_AR4,     "AR4",       m_r[TMR_AR4].i32[0]);
	state_add(TMS3203X_AR5,     "AR5",       m_r[TMR_AR5].i32[0]);
	state_add(TMS3203X_AR6,     "AR6",       m_r[TMR_AR6].i32[0]);
	state_add(TMS3203X_AR7,     "AR7",       m_r[TMR_AR7].i32[0]);
	state_add(TMS3203X_DP,      "DP",        m_r[TMR_DP].i32[0]).mask(0xff);
	state_add(TMS3203X_IR0,     "IR0",       m_r[TMR_IR0].i32[0]);
	state_add(TMS3203X_IR1,     "IR1",       m_r[TMR_IR1].i32[0]);
	state_add(TMS3203X_BK,      "BK",        m_r[TMR_BK].i32[0]);
	state_add(TMS3203X_SP,      "SP",        m_r[TMR_SP].i32[0]);
	state_add(TMS3203X_ST,      "ST",        m_r[TMR_ST].i32[0]);
	state_add(TMS3203X_IE,      "IE",        m_r[TMR_IE].i32[0]);
	state_add(TMS3203X_IF,      "IF",        m_r[TMR_IF].i32[0]);
	state_add(TMS3203X_IOF,     "IOF",       m_r[TMR_IOF].i32[0]);
	state_add(TMS3203X_RS,      "RS",        m_r[TMR_RS].i32[0]);
	state_add(TMS3203X_RE,      "RE",        m_r[TMR_RE].i32[0]);
	state_add(TMS3203X_RC,      "RC",        m_r[TMR_RC].i32[0]);
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void tms3203x_device::device_reset()
{
	// if we have a config struct, get the boot ROM address
	if (m_config.m_bootoffset != 0)
	{
		m_mcu_mode = true;
		m_pc = boot_loader(m_config.m_bootoffset);
	}
	else
	{
		m_mcu_mode = false;
		m_pc = RMEM(0);
	}

	// reset some registers
	IREG(TMR_IE) = 0;
	IREG(TMR_IF) = 0;
	IREG(TMR_ST) = 0;
	IREG(TMR_IOF) = 0;

	// reset internal stuff
	m_delayed = m_irq_pending = m_is_idling = false;
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void tms3203x_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case TMS3203X_R0F:
		case TMS3203X_R1F:
		case TMS3203X_R2F:
		case TMS3203X_R3F:
		case TMS3203X_R4F:
		case TMS3203X_R5F:
		case TMS3203X_R6F:
		case TMS3203X_R7F:
			m_r[TMR_R0 + (entry.index() - TMS3203X_R0F)].from_double(*(float *)&m_iotemp);
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(tms3203x) called for unexpected value\n");
			break;
	}
}


//-------------------------------------------------
//  state_export - export state into the device,
//  before returning it to the caller
//-------------------------------------------------

void tms3203x_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case TMS3203X_R0F:
		case TMS3203X_R1F:
		case TMS3203X_R2F:
		case TMS3203X_R3F:
		case TMS3203X_R4F:
		case TMS3203X_R5F:
		case TMS3203X_R6F:
		case TMS3203X_R7F:
			*(float *)&m_iotemp = m_r[TMR_R0 + (entry.index() - TMS3203X_R0F)].as_float();
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(tms3203x) called for unexpected value\n");
			break;
	}
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void tms3203x_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case TMS3203X_R0F:
		case TMS3203X_R1F:
		case TMS3203X_R2F:
		case TMS3203X_R3F:
		case TMS3203X_R4F:
		case TMS3203X_R5F:
		case TMS3203X_R6F:
		case TMS3203X_R7F:
			string.printf("%12g", m_r[TMR_R0 + (entry.index() - TMS3203X_R0F)].as_double());
			break;

		case STATE_GENFLAGS:
			UINT32 temp = m_r[TMR_ST].i32[0];
			string.printf("%c%c%c%c%c%c%c%c",
				(temp & 0x80) ? 'O':'.',
				(temp & 0x40) ? 'U':'.',
                (temp & 0x20) ? 'V':'.',
                (temp & 0x10) ? 'u':'.',
                (temp & 0x08) ? 'n':'.',
                (temp & 0x04) ? 'z':'.',
                (temp & 0x02) ? 'v':'.',
                (temp & 0x01) ? 'c':'.');
			break;
	}
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t tms3203x_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms3203x );
	return CPU_DISASSEMBLE_NAME(tms3203x)(NULL, buffer, pc, oprom, opram, 0);
}



//**************************************************************************
//  PUBLIC INTERFACES
//**************************************************************************

//-------------------------------------------------
//  fp_to_float - convert a 32-bit value from DSP
//  floating-point format a 32-bit IEEE float
//-------------------------------------------------

float tms3203x_device::fp_to_float(UINT32 floatdata)
{
	tmsreg gen(floatdata << 8, (INT32)floatdata >> 24);
	return gen.as_float();
}


//-------------------------------------------------
//  fp_to_double - convert a 32-bit value from DSP
//  floating-point format a 64-bit IEEE double
//-------------------------------------------------

double tms3203x_device::fp_to_double(UINT32 floatdata)
{
	tmsreg gen(floatdata << 8, (INT32)floatdata >> 24);
	return gen.as_double();
}


//-------------------------------------------------
//  float_to_fp - convert a 32-bit IEEE float to
//  a 32-bit DSP floating-point value
//-------------------------------------------------

UINT32 tms3203x_device::float_to_fp(float fval)
{
	tmsreg gen(fval);
	return (gen.exponent() << 24) | ((UINT32)gen.mantissa() >> 8);
}


//-------------------------------------------------
//  double_to_fp - convert a 64-bit IEEE double to
//  a 32-bit DSP floating-point value
//-------------------------------------------------

UINT32 tms3203x_device::double_to_fp(double dval)
{
	tmsreg gen(dval);
	return (gen.exponent() << 24) | ((UINT32)gen.mantissa() >> 8);
}



//**************************************************************************
//  EXECUTION
//**************************************************************************

//-------------------------------------------------
//  check_irqs - check for pending IRQs and take
//  them if enabled
//-------------------------------------------------

void tms3203x_device::check_irqs()
{
	// determine if we have any live interrupts
	UINT16 validints = IREG(TMR_IF) & IREG(TMR_IE) & 0x0fff;
	if (validints == 0 || (IREG(TMR_ST) & GIEFLAG) == 0)
		return;

	// find the lowest signalled value
	int whichtrap = 0;
	for (int i = 0; i < 12; i++)
		if (validints & (1 << i))
		{
			whichtrap = i + 1;
			break;
		}

	// no longer idling if we get here
	m_is_idling = false;
	if (!m_delayed)
	{
		UINT16 intmask = 1 << (whichtrap - 1);

		// bit in IF is cleared when interrupt is taken
		IREG(TMR_IF) &= ~intmask;
		trap(whichtrap);

		// after auto-clearing the interrupt bit, we need to re-trigger
        // level-sensitive interrupts
		if (m_config.m_chip_type == tms3203x_device_config::CHIP_TYPE_TMS32031 || (IREG(TMR_ST) & 0x4000) == 0)
			IREG(TMR_IF) |= m_irq_state & 0x0f;
	}
	else
		m_irq_pending = true;
}


//-------------------------------------------------
//  execute_set_input - set input and IRQ lines
//-------------------------------------------------

void tms3203x_device::execute_set_input(int inputnum, int state)
{
	// ignore anything out of range
	if (inputnum >= 12)
		return;

	// update the external state
	UINT16 intmask = 1 << inputnum;
    if (state == ASSERT_LINE)
    {
		m_irq_state |= intmask;
	    IREG(TMR_IF) |= intmask;
	}
	else
		m_irq_state &= ~intmask;

	// external interrupts are level-sensitive on the '31 and can be
    // configured as such on the '32; in that case, if the external
    // signal is high, we need to update the value in IF accordingly
	if (m_config.m_chip_type == tms3203x_device_config::CHIP_TYPE_TMS32031 || (IREG(TMR_ST) & 0x4000) == 0)
		IREG(TMR_IF) |= m_irq_state & 0x0f;
}


//-------------------------------------------------
//  execute_run - execute until our icount expires
//-------------------------------------------------

void tms3203x_device::execute_run()
{
	// check IRQs up front
	check_irqs();

	// if we're idling, just eat the cycles
	if (m_is_idling)
	{
		m_icount = 0;
		return;
	}

	// non-debug case
	if ((m_machine.debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		while (m_icount > 0)
		{
			if ((IREG(TMR_ST) & RMFLAG) && m_pc == IREG(TMR_RE) + 1)
			{
				if ((INT32)--IREG(TMR_RC) >= 0)
					m_pc = IREG(TMR_RS);
				else
				{
					IREG(TMR_ST) &= ~RMFLAG;
					if (m_delayed)
					{
						m_delayed = false;
						if (m_irq_pending)
						{
							m_irq_pending = false;
							check_irqs();
						}
					}
				}
				continue;
			}

			execute_one();
		}
	}

	// debugging case
	else
	{
		while (m_icount > 0)
		{
			// watch for out-of-range stack pointers
			if (IREG(TMR_SP) & 0xff000000)
				debugger_break(&m_machine);
			if ((IREG(TMR_ST) & RMFLAG) && m_pc == IREG(TMR_RE) + 1)
			{
				if ((INT32)--IREG(TMR_RC) >= 0)
					m_pc = IREG(TMR_RS);
				else
				{
					IREG(TMR_ST) &= ~RMFLAG;
					if (m_delayed)
					{
						m_delayed = false;
						if (m_irq_pending)
						{
							m_irq_pending = false;
							check_irqs();
						}
					}
				}
				continue;
			}

			debugger_instruction_hook(this, m_pc);
			execute_one();
		}
	}
}


//-------------------------------------------------
//  boot_loader - reset the state of the system
//  by simulating the internal boot loader
//-------------------------------------------------

UINT32 tms3203x_device::boot_loader(UINT32 boot_rom_addr)
{
	// read the size of the data
	UINT32 bits = RMEM(boot_rom_addr);
	if (bits != 8 && bits != 16 && bits != 32)
		return 0;
	UINT32 datamask = 0xffffffffUL >> (32 - bits);
	UINT32 advance = 32 / bits;
	boot_rom_addr += advance;

	// read the control register
	UINT32 control = RMEM(boot_rom_addr++) & datamask;
	for (int i = 1; i < advance; i++)
		control |= (RMEM(boot_rom_addr++) & datamask) << (bits * i);

	// now parse the data
	UINT32 start_offset = 0;
	bool first = true;
	while (1)
	{
		// read the length of this section
		UINT32 len = RMEM(boot_rom_addr++) & datamask;
		for (int i = 1; i < advance; i++)
			len |= (RMEM(boot_rom_addr++) & datamask) << (bits * i);

		// stop at 0
		if (len == 0)
			return start_offset;

		// read the destination offset of this section
		UINT32 offs = RMEM(boot_rom_addr++) & datamask;
		for (int i = 1; i < advance; i++)
			offs |= (RMEM(boot_rom_addr++) & datamask) << (bits * i);

		// if this is the first block, that's where we boot to
		if (first)
		{
			start_offset = offs;
			first = false;
		}

		// now copy the data
		while (len--)
		{
			// extract the 32-bit word
			UINT32 data = RMEM(boot_rom_addr++) & datamask;
			for (int i = 1; i < advance; i++)
				data |= (RMEM(boot_rom_addr++) & datamask) << (bits * i);

			// write it out
			WMEM(offs++, data);
		}
	}
}


//**************************************************************************
//  CORE OPCODES
//**************************************************************************

#include "32031ops.c"
