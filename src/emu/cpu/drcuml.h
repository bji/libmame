/***************************************************************************

    drcuml.h

    Universal machine language for dynamic recompiling CPU cores.

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

#ifndef __DRCUML_H__
#define __DRCUML_H__

#include "drccache.h"
#include "uml.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// these options are passed into drcuml_alloc() and control global behaviors
const UINT32 DRCUML_OPTION_USE_C		= 0x0001;		// always use the C back-end
const UINT32 DRCUML_OPTION_LOG_UML		= 0x0002;		// generate a UML disassembly of each block
const UINT32 DRCUML_OPTION_LOG_NATIVE	= 0x0004;		// tell the back-end to generate a native disassembly of each block



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// determine the type of the native DRC, falling back to C
#ifndef NATIVE_DRC
typedef drcbe_c drcbe_native;
#else
class NATIVE_DRC;
typedef NATIVE_DRC drcbe_native;
#endif


// opaque structure describing UML generation state
class drcuml_state;


// an integer register, with low/high parts
union drcuml_ireg
{
#ifdef LSB_FIRST
	struct { UINT32	l,h; } w;				// 32-bit low, high parts of the register
#else
	struct { UINT32 h,l; } w;				// 32-bit low, high parts of the register
#endif
	UINT64				d;					// 64-bit full register
};


// a floating-point register, with low/high parts
union drcuml_freg
{
#ifdef LSB_FIRST
	struct { float l,h; } s;				// 32-bit low, high parts of the register
#else
	struct { float h,l;	} s;				// 32-bit low, high parts of the register
#endif
	double				d;					// 64-bit full register
};


// the collected machine state of a system
struct drcuml_machine_state
{
	drcuml_ireg			r[uml::REG_I_COUNT];	// integer registers
	drcuml_freg			f[uml::REG_F_COUNT];	// floating-point registers
	UINT32				exp;				// exception parameter register
	UINT8				fmod;				// fmod (floating-point mode) register
	UINT8				flags;				// flags state
};


// hints and information about the back-end
struct drcbe_info
{
	UINT8				direct_iregs;		// number of direct-mapped integer registers
	UINT8				direct_fregs;		// number of direct-mapped floating point registers
};


// a drcuml_block describes a basic block of instructions
class drcuml_block
{
	template<class T> friend class simple_list;

public:
	// construction/destruction
	drcuml_block(drcuml_state &drcuml, UINT32 maxinst);
	~drcuml_block();

	// getters
	drcuml_block *next() const { return m_next; }
	bool inuse() const { return m_inuse; }
	UINT32 maxinst() const { return m_maxinst; }

	// code generation
	void begin();
	void end();
	void abort();

	// instruction appending
	uml::instruction &append();
	void append_comment(const char *format, ...);

	// this class is thrown if abort() is called
	class abort_compilation : public emu_exception
	{
		friend class drcuml_block;
		abort_compilation() { }
	};

private:
	// internal helpers
	void optimize();
	void disassemble();
	const char *get_comment_text(const uml::instruction &inst, astring &comment);

	// internal state
	drcuml_state &			m_drcuml;			// pointer back to the owning UML
	drcuml_block *			m_next;				// pointer to next block
	UINT32					m_nextinst;			// next instruction to fill in the cache
	UINT32					m_maxinst;			// maximum number of instructions
	uml::instruction *		m_inst;				// pointer to the instruction list
	bool					m_inuse;			// this block is in use
};


// interface structure for a back-end
class drcbe_interface
{
public:
	// construction/destruction
	drcbe_interface(drcuml_state &drcuml, drc_cache &cache, device_t &device);
	virtual ~drcbe_interface();

	// required overrides
	virtual void reset() = 0;
	virtual int execute(uml::code_handle &entry) = 0;
	virtual void generate(drcuml_block &block, const uml::instruction *instlist, UINT32 numinst) = 0;
	virtual bool hash_exists(UINT32 mode, UINT32 pc) = 0;
	virtual void get_info(drcbe_info &info) = 0;

protected:
	// internal state
	drcuml_state &			m_drcuml;			// pointer back to our owner
	drc_cache &				m_cache;			// pointer to the cache
	device_t &				m_device;			// CPU device we are associated with
	address_space *			m_space[ADDRESS_SPACES];// pointers to CPU's address space
	drcuml_machine_state &	m_state;			// state of the machine (in near cache)
	data_accessors *		m_accessors;		// memory accessors (in near cache)
};


// structure describing UML generation state
class drcuml_state
{
public:
	// construction/destruction
	drcuml_state(device_t &device, drc_cache &cache, UINT32 flags, int modes, int addrbits, int ignorebits);
	~drcuml_state();

	// getters
	device_t &device() const { return m_device; }
	drc_cache &cache() const { return m_cache; }

	// reset the state
	void reset();
	int execute(uml::code_handle &entry) { return m_beintf.execute(entry); }

	// code generation
	drcuml_block *begin_block(UINT32 maxinst);

	// back-end interface
	void get_backend_info(drcbe_info &info) { m_beintf.get_info(info); }
	bool hash_exists(UINT32 mode, UINT32 pc) { return m_beintf.hash_exists(mode, pc); }
	void generate(drcuml_block &block, uml::instruction *instructions, UINT32 count) { m_beintf.generate(block, instructions, count); }

	// handle management
	uml::code_handle *handle_alloc(const char *name);

	// symbol management
	void symbol_add(void *base, UINT32 length, const char *name);
	const char *symbol_find(void *base, UINT32 *offset = NULL);

	// logging
	bool logging() const { return (m_umllog != NULL); }
	void log_printf(const char *format, ...);
	void log_flush() { if (logging()) fflush(m_umllog); }

private:
	// symbol class
	class symbol
	{
		friend class drcuml_state;
		template<class T> friend class simple_list;

		// construction/destruction
		symbol(void *base, UINT32 length, const char *name)
			: m_next(NULL),
			  m_base(drccodeptr(base)),
			  m_length(length),
			  m_name(name) { }

	public:
		// getters
		symbol *next() const { return m_next; }

	private:
		// internal state
		symbol *				m_next;				// link to the next symbol
		drccodeptr				m_base;				// base of the symbol
		UINT32					m_length;			// length of the region covered
		astring					m_name;				// name of the symbol
	};

	// internal state
	device_t &					m_device;			// CPU device we are associated with
	drc_cache &					m_cache;			// pointer to the codegen cache
	drcbe_interface &			m_beintf;			// backend interface pointer
	FILE *						m_umllog;			// handle to the UML logfile
	simple_list<drcuml_block>	m_blocklist;		// list of active blocks
	simple_list<uml::code_handle> m_handlelist;		// list of active handles
	simple_list<symbol>			m_symlist;			// list of symbols
};



#endif /* __DRCUML_H__ */
