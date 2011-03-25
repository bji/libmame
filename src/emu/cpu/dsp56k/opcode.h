#ifndef __DSP56K_OPCODE_H__
#define __DSP56K_OPCODE_H__

#include "emu.h"
#include "inst.h"
#include "pmove.h"

#include "dsp56k.h"

//
// An Opcode contains an instruction and a parallel move operation.
//
namespace DSP56K
{

class Instruction;
class ParallelMove;

class Opcode
{
public:
	Opcode(UINT16 w0, UINT16 w1);
	virtual ~Opcode();

	astring disassemble() const;
	void evaluate(dsp56k_core* cpustate) const;
	size_t size() const;
	size_t evalSize() const;

	// Peek through to the instruction
	const reg_id& instSource() const;
	const reg_id& instDestination() const;
	const size_t instAccumulatorBitsModified() const;

private:
	Instruction* m_instruction;
	ParallelMove* m_parallelMove;

	UINT16 m_word0;
	UINT16 m_word1;

	astring dcString() const;
};

}
#endif
