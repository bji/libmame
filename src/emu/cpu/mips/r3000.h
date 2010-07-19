/***************************************************************************

    r3000.h
    Interface file for the portable MIPS R3000 emulator.
    Written by Aaron Giles

***************************************************************************/

#ifndef __R3000_H__
#define __R3000_H__



/***************************************************************************
    COMPILE-TIME DEFINITIONS
***************************************************************************/


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	R3000_PC=1,R3000_SR,
	R3000_R0,R3000_R1,R3000_R2,R3000_R3,R3000_R4,R3000_R5,R3000_R6,R3000_R7,
	R3000_R8,R3000_R9,R3000_R10,R3000_R11,R3000_R12,R3000_R13,R3000_R14,R3000_R15,
	R3000_R16,R3000_R17,R3000_R18,R3000_R19,R3000_R20,R3000_R21,R3000_R22,R3000_R23,
	R3000_R24,R3000_R25,R3000_R26,R3000_R27,R3000_R28,R3000_R29,R3000_R30,R3000_R31
};


/***************************************************************************
    INTERRUPT CONSTANTS
***************************************************************************/

#define R3000_IRQ0		0		/* IRQ0 */
#define R3000_IRQ1		1		/* IRQ1 */
#define R3000_IRQ2		2		/* IRQ2 */
#define R3000_IRQ3		3		/* IRQ3 */
#define R3000_IRQ4		4		/* IRQ4 */
#define R3000_IRQ5		5		/* IRQ5 */


/***************************************************************************
    STRUCTURES
***************************************************************************/

typedef struct _r3000_cpu_core r3000_cpu_core;
struct _r3000_cpu_core
{
	UINT8		hasfpu;			/* 1 if we have an FPU, 0 otherwise */
	size_t		icache;			/* code cache size */
	size_t		dcache;			/* data cache size */
};


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

DECLARE_LEGACY_CPU_DEVICE(R3000BE, r3000be);
DECLARE_LEGACY_CPU_DEVICE(R3000LE, r3000le);

DECLARE_LEGACY_CPU_DEVICE(R3041BE, r3041be);
DECLARE_LEGACY_CPU_DEVICE(R3041LE, r3041le);

#endif /* __R3000_H__ */
