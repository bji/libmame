/***************************************************************************

    ppcfe.h

    Front-end for PowerPC recompiler

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

#ifndef __PPCFE_H__
#define __PPCFE_H__

#include "ppccom.h"
#include "cpu/drcfe.h"


//**************************************************************************
//  MACROS
//**************************************************************************

// register flags 0
#define REGFLAG_R(n)					(1 << (n))
#define REGFLAG_RZ(n)					(((n) == 0) ? 0 : REGFLAG_R(n))

// register flags 1
#define REGFLAG_FR(n)					(1 << (n))

// register flags 2
#define REGFLAG_CR(n)					(0xf0000000 >> (4 * (n)))
#define REGFLAG_CR_BIT(n)				(0x80000000 >> (n))

// register flags 3
#define REGFLAG_XER_CA					(1 << 0)
#define REGFLAG_XER_OV					(1 << 1)
#define REGFLAG_XER_SO					(1 << 2)
#define REGFLAG_XER_COUNT				(1 << 3)
#define REGFLAG_CTR						(1 << 4)
#define REGFLAG_LR						(1 << 5)
#define REGFLAG_FPSCR(n)				(1 << (6 + (n)))



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ppc_frontend : public drc_frontend
{
public:
	// construction/destruction
	ppc_frontend(powerpc_state &state, UINT32 window_start, UINT32 window_end, UINT32 max_sequence);

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev);

private:
	// inlines
	UINT32 compute_spr(UINT32 spr) const { return ((spr >> 5) | (spr << 5)) & 0x3ff; }
	bool is_403_class() const { return (m_context.flavor == PPC_MODEL_403GA || m_context.flavor == PPC_MODEL_403GB || m_context.flavor == PPC_MODEL_403GC || m_context.flavor == PPC_MODEL_403GCX || m_context.flavor == PPC_MODEL_405GP); }
	bool is_601_class() const { return (m_context.flavor == PPC_MODEL_601); }
	bool is_602_class() const { return (m_context.flavor == PPC_MODEL_602); }
	bool is_603_class() const { return (m_context.flavor == PPC_MODEL_603 || m_context.flavor == PPC_MODEL_603E || m_context.flavor == PPC_MODEL_603EV || m_context.flavor == PPC_MODEL_603R); }

	// internal helpers
	bool describe_13(UINT32 op, opcode_desc &desc, const opcode_desc *prev);
	bool describe_1f(UINT32 op, opcode_desc &desc, const opcode_desc *prev);
	bool describe_3b(UINT32 op, opcode_desc &desc, const opcode_desc *prev);
	bool describe_3f(UINT32 op, opcode_desc &desc, const opcode_desc *prev);

	// internal state
	powerpc_state &m_context;
};


#endif /* __PPCFE_H__ */
