 /**************************************************************************\
 *                 Texas Instruments TMS32010 DSP Emulator                  *
 *                                                                          *
 *                  Copyright Tony La Porta                                 *
 *      You are not allowed to distribute this software commercially.       *
 *                      Written for the MAME project.                       *
 *                                                                          *
 *                                                                          *
 *      Note :  This is a word based microcontroller, with addressing       *
 *              architecture based on the Harvard addressing scheme.        *
 *                                                                          *
 \**************************************************************************/

#pragma once

#ifndef __TMS32010_H__
#define __TMS32010_H__




/****************************************************************************
 * Use this in the I/O port address fields of your driver for the BIO pin
 * i.e,
 *  AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(twincobr_bio_line_r)
 */

#define TMS32010_BIO			0x10		/* BIO input */


#define TMS32010_INT_PENDING	0x80000000
#define TMS32010_INT_NONE		0

#define  TMS32010_ADDR_MASK  0x0fff		/* TMS32010 can only address 0x0fff */
										/* however other TMS3201x devices   */
										/* can address up to 0xffff (incase */
										/* their support is ever added).    */


enum
{
	TMS32010_PC=1, TMS32010_SP,   TMS32010_STR,  TMS32010_ACC,
	TMS32010_PREG, TMS32010_TREG, TMS32010_AR0,  TMS32010_AR1,
	TMS32010_STK0, TMS32010_STK1, TMS32010_STK2, TMS32010_STK3
};


/****************************************************************************
 *  Public Functions
 */

CPU_GET_INFO( tms32010 );
#define CPU_TMS32010 CPU_GET_INFO_NAME( tms32010 )


CPU_DISASSEMBLE( tms32010 );

#endif	/* __TMS32010_H__ */
