/*****************************************************************************
 *
 *   m6502.h
 *   Portable 6502/65c02/65sc02/6510/n2a03 emulator interface
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   65sc02 core Copyright Peter Trauner.
 *   Deco16 portions Copyright Bryan McPhail.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/
/* 2.February 2000 PeT added 65sc02 subtype */

#pragma once

#ifndef __M6502_H__
#define __M6502_H__


/* set to 1 to test cur_mrhard/cur_wmhard to avoid calls */
#define FAST_MEMORY 0

#define SUBTYPE_6502	0
#define SUBTYPE_65C02	1
#define SUBTYPE_6510	2
#define SUBTYPE_2A03	3
#define SUBTYPE_65SC02	4
#define SUBTYPE_DECO16	5

enum
{
	M6502_PC=1, M6502_S, M6502_P, M6502_A, M6502_X, M6502_Y,
	M6502_EA, M6502_ZP,
	M6502_SUBTYPE
};

#define M6502_IRQ_LINE		0
/* use cpu_set_input_line(cpudevice, M6502_SET_OVERFLOW, level)
   to change level of the so input line
   positiv edge sets overflow flag */
#define M6502_SET_OVERFLOW	1

typedef UINT8 (*m6510_port_read_func)(running_device *device, UINT8 direction);
typedef void (*m6510_port_write_func)(running_device *device, UINT8 direction, UINT8 data);


/* Optional interface to set callbacks */
typedef struct _m6502_interface m6502_interface;
struct _m6502_interface
{
	read8_space_func		read_indexed_func;
	write8_space_func		write_indexed_func;
	m6510_port_read_func	port_read_func;
	m6510_port_write_func	port_write_func;
};


enum
{
	CPUINFO_INT_M6510_PORT = CPUINFO_INT_CPU_SPECIFIC
};

extern CPU_GET_INFO( m6502 );
#define CPU_M6502 CPU_GET_INFO_NAME( m6502 )

extern CPU_DISASSEMBLE( m6502 );

/****************************************************************************
 * The 6510
 ****************************************************************************/
#define M6510_A 						M6502_A
#define M6510_X 						M6502_X
#define M6510_Y 						M6502_Y
#define M6510_S 						M6502_S
#define M6510_PC						M6502_PC
#define M6510_P 						M6502_P
#define M6510_EA						M6502_EA
#define M6510_ZP						M6502_ZP
#define M6510_NMI_STATE 				M6502_NMI_STATE
#define M6510_IRQ_STATE 				M6502_IRQ_STATE

#define M6510_IRQ_LINE					M6502_IRQ_LINE

extern CPU_GET_INFO( m6510 );
#define CPU_M6510 CPU_GET_INFO_NAME( m6510 )

extern CPU_DISASSEMBLE( m6510 );


#define M6510T_A						M6502_A
#define M6510T_X						M6502_X
#define M6510T_Y						M6502_Y
#define M6510T_S						M6502_S
#define M6510T_PC						M6502_PC
#define M6510T_P						M6502_P
#define M6510T_EA						M6502_EA
#define M6510T_ZP						M6502_ZP
#define M6510T_NMI_STATE				M6502_NMI_STATE
#define M6510T_IRQ_STATE				M6502_IRQ_STATE

#define M6510T_IRQ_LINE					M6502_IRQ_LINE

extern CPU_GET_INFO( m6510t );
#define CPU_M6510T CPU_GET_INFO_NAME( m6510t )


#define M7501_A 						M6502_A
#define M7501_X 						M6502_X
#define M7501_Y 						M6502_Y
#define M7501_S 						M6502_S
#define M7501_PC						M6502_PC
#define M7501_P 						M6502_P
#define M7501_EA						M6502_EA
#define M7501_ZP						M6502_ZP
#define M7501_NMI_STATE 				M6502_NMI_STATE
#define M7501_IRQ_STATE 				M6502_IRQ_STATE

#define M7501_IRQ_LINE					M6502_IRQ_LINE

extern CPU_GET_INFO( m7501 );
#define CPU_M7501 CPU_GET_INFO_NAME( m7501 )

#define M8502_A 						M6502_A
#define M8502_X 						M6502_X
#define M8502_Y 						M6502_Y
#define M8502_S 						M6502_S
#define M8502_PC						M6502_PC
#define M8502_P 						M6502_P
#define M8502_EA						M6502_EA
#define M8502_ZP						M6502_ZP
#define M8502_NMI_STATE 				M6502_NMI_STATE
#define M8502_IRQ_STATE 				M6502_IRQ_STATE

#define M8502_IRQ_LINE					M6502_IRQ_LINE

extern CPU_GET_INFO( m8502 );
#define CPU_M8502 CPU_GET_INFO_NAME( m8502 )


/****************************************************************************
 * The 2A03 (NES 6502 without decimal mode ADC/SBC)
 ****************************************************************************/
#define N2A03_A 						M6502_A
#define N2A03_X 						M6502_X
#define N2A03_Y 						M6502_Y
#define N2A03_S 						M6502_S
#define N2A03_PC						M6502_PC
#define N2A03_P 						M6502_P
#define N2A03_EA						M6502_EA
#define N2A03_ZP						M6502_ZP
#define N2A03_NMI_STATE 				M6502_NMI_STATE
#define N2A03_IRQ_STATE 				M6502_IRQ_STATE

#define N2A03_IRQ_LINE					M6502_IRQ_LINE

extern CPU_GET_INFO( n2a03 );
#define CPU_N2A03 CPU_GET_INFO_NAME( n2a03 )

#define N2A03_DEFAULTCLOCK (21477272.724 / 12)

/* The N2A03 is integrally tied to its PSG (they're on the same die).
   Bit 7 of address $4011 (the PSG's DPCM control register), when set,
   causes an IRQ to be generated.  This function allows the IRQ to be called
   from the PSG core when such an occasion arises. */
extern void n2a03_irq(running_device *device);


/****************************************************************************
 * The 65C02
 ****************************************************************************/
#define M65C02_A						M6502_A
#define M65C02_X						M6502_X
#define M65C02_Y						M6502_Y
#define M65C02_S						M6502_S
#define M65C02_PC						M6502_PC
#define M65C02_P						M6502_P
#define M65C02_EA						M6502_EA
#define M65C02_ZP						M6502_ZP
#define M65C02_NMI_STATE				M6502_NMI_STATE
#define M65C02_IRQ_STATE				M6502_IRQ_STATE

#define M65C02_IRQ_LINE					M6502_IRQ_LINE

extern CPU_GET_INFO( m65c02 );
#define CPU_M65C02 CPU_GET_INFO_NAME( m65c02 )

extern CPU_DISASSEMBLE( m65c02 );


/****************************************************************************
 * The 65SC02
 ****************************************************************************/
#define M65SC02_A						M6502_A
#define M65SC02_X						M6502_X
#define M65SC02_Y						M6502_Y
#define M65SC02_S						M6502_S
#define M65SC02_PC						M6502_PC
#define M65SC02_P						M6502_P
#define M65SC02_EA						M6502_EA
#define M65SC02_ZP						M6502_ZP
#define M65SC02_NMI_STATE				M6502_NMI_STATE
#define M65SC02_IRQ_STATE				M6502_IRQ_STATE

#define M65SC02_IRQ_LINE				M6502_IRQ_LINE

extern CPU_GET_INFO( m65sc02 );
#define CPU_M65SC02 CPU_GET_INFO_NAME( m65sc02 )

extern CPU_DISASSEMBLE( m65sc02 );

/****************************************************************************
 * The DECO CPU16
 ****************************************************************************/
#define DECO16_A						M6502_A
#define DECO16_X						M6502_X
#define DECO16_Y						M6502_Y
#define DECO16_S						M6502_S
#define DECO16_PC						M6502_PC
#define DECO16_P						M6502_P
#define DECO16_EA						M6502_EA
#define DECO16_ZP						M6502_ZP
#define DECO16_NMI_STATE				M6502_NMI_STATE
#define DECO16_IRQ_STATE				M6502_IRQ_STATE

#define DECO16_IRQ_LINE					M6502_IRQ_LINE

extern CPU_GET_INFO( deco16 );
#define CPU_DECO16 CPU_GET_INFO_NAME( deco16 )

extern CPU_DISASSEMBLE( deco16 );

#endif /* __M6502_H__ */
