/***************************************************************************

    gamedrv.h

    Definitions for game drivers.

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

#ifndef __GAMEDRV_H__
#define __GAMEDRV_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// maxima
const int MAX_DRIVER_NAME_CHARS = 8;

// flags for game drivers
const UINT32 ORIENTATION_MASK       		= 0x00000007;
const UINT32 GAME_NOT_WORKING				= 0x00000008;
const UINT32 GAME_UNEMULATED_PROTECTION		= 0x00000010;	// game's protection not fully emulated
const UINT32 GAME_WRONG_COLORS				= 0x00000020;	// colors are totally wrong
const UINT32 GAME_IMPERFECT_COLORS			= 0x00000040;	// colors are not 100% accurate, but close
const UINT32 GAME_IMPERFECT_GRAPHICS		= 0x00000080;	// graphics are wrong/incomplete
const UINT32 GAME_NO_COCKTAIL				= 0x00000100;	// screen flip support is missing
const UINT32 GAME_NO_SOUND					= 0x00000200;	// sound is missing
const UINT32 GAME_IMPERFECT_SOUND			= 0x00000400;	// sound is known to be wrong
const UINT32 GAME_SUPPORTS_SAVE				= 0x00000800;	// game supports save states
const UINT32 GAME_IS_BIOS_ROOT				= 0x00001000;	// this driver entry is a BIOS root
const UINT32 GAME_NO_STANDALONE				= 0x00002000;	// this driver cannot stand alone
const UINT32 GAME_REQUIRES_ARTWORK			= 0x00004000;	// the driver requires external artwork for key elements of the game
const UINT32 GAME_UNOFFICIAL    			= 0x00008000;	// unofficial hardware change
const UINT32 GAME_NO_SOUND_HW				= 0x00010000;	// sound hardware not available
const UINT32 GAME_MECHANICAL				= 0x00020000;	// contains mechanical parts (pinball, redemption games,...)
const UINT32 GAME_TYPE_ARCADE				= 0x00040000;	// arcade machine (coin operated machines)
const UINT32 GAME_TYPE_CONSOLE				= 0x00080000;	// console system
const UINT32 GAME_TYPE_COMPUTER				= 0x00100000;	// any kind of computer including home computers, minis, calcs,...
const UINT32 GAME_TYPE_OTHER				= 0x00200000;	// any other emulated system that doesn't fit above (ex. clock, satelite receiver,...)

// useful combinations of flags
const UINT32 GAME_IS_SKELETON				= GAME_NO_SOUND | GAME_NOT_WORKING; // mask for skelly games
const UINT32 GAME_IS_SKELETON_MECHANICAL	= GAME_IS_SKELETON | GAME_MECHANICAL | GAME_REQUIRES_ARTWORK; // mask for skelly mechanical games



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// static driver initialization callback
typedef void (*driver_init_func)(running_machine &machine);

// static POD structure describing each game driver entry
struct game_driver
{
	const char *		source_file;				// set this to __FILE__
	const char *		parent;						// if this is a clone, the name of the parent
	const char *		name;						// short (8-character) name of the game
	const char *		description;				// full name of the game
	const char *		year;						// year the game was released
	const char *		manufacturer;				// manufacturer of the game
	machine_config_constructor machine_config;		// machine driver tokens
	ioport_constructor	ipt;						// pointer to constructor for input ports
	void				(*driver_init)(running_machine &machine); // DRIVER_INIT callback
	const rom_entry *	rom;						// pointer to list of ROMs for the game
	const char *		compatible_with;
	UINT32				flags;						// orientation and other flags; see defines below
	const char *		default_layout;				// default internally defined layout
};



//**************************************************************************
//  MACROS
//**************************************************************************

// wrappers for the DRIVER_INIT callback
#define DRIVER_INIT_NAME(name)		driver_init_##name
#define DRIVER_INIT(name)			void DRIVER_INIT_NAME(name)(running_machine &machine)
#define DRIVER_INIT_CALL(name)		DRIVER_INIT_NAME(name)(machine)

#define driver_init_0				NULL

// wrappers for declaring and defining game drivers
#define GAME_NAME(name) driver_##name
#define GAME_EXTERN(name) extern const game_driver GAME_NAME(name)

// standard GAME() macro
#define GAME(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS)	\
	GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,((const char *)0))

// standard macro with additional layout
#define GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,LAYOUT)	\
extern const game_driver GAME_NAME(NAME) =	\
{											\
	__FILE__,								\
	#PARENT,								\
	#NAME,									\
	FULLNAME,								\
	#YEAR,									\
	COMPANY,								\
	MACHINE_CONFIG_NAME(MACHINE),			\
	INPUT_PORTS_NAME(INPUT),				\
	DRIVER_INIT_NAME(INIT),					\
	ROM_NAME(NAME),							\
	NULL,									\
	(MONITOR)|(FLAGS)|GAME_TYPE_ARCADE,		\
	&LAYOUT[0]								\
};

// standard console definition macro
#define CONS(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,INIT,COMPANY,FULLNAME,FLAGS)	\
extern const game_driver GAME_NAME(NAME) =	\
{											\
	__FILE__,								\
	#PARENT,								\
	#NAME,									\
	FULLNAME,								\
	#YEAR,									\
	COMPANY,								\
	MACHINE_CONFIG_NAME(MACHINE),			\
	INPUT_PORTS_NAME(INPUT),				\
	DRIVER_INIT_NAME(INIT),					\
	ROM_NAME(NAME),							\
	#COMPAT,								\
	ROT0|(FLAGS)|GAME_TYPE_CONSOLE,			\
	NULL									\
};

// standard computer definition macro
#define COMP(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,INIT,COMPANY,FULLNAME,FLAGS)	\
extern const game_driver GAME_NAME(NAME) =	\
{											\
	__FILE__,								\
	#PARENT,								\
	#NAME,									\
	FULLNAME,								\
	#YEAR,									\
	COMPANY,								\
	MACHINE_CONFIG_NAME(MACHINE),			\
	INPUT_PORTS_NAME(INPUT),				\
	DRIVER_INIT_NAME(INIT),					\
	ROM_NAME(NAME),							\
	#COMPAT,								\
	ROT0|(FLAGS)|GAME_TYPE_COMPUTER,		\
	NULL									\
};

// standard system definition macro
#define SYST(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,INIT,COMPANY,FULLNAME,FLAGS)	\
extern const game_driver GAME_NAME(NAME) =	\
{											\
	__FILE__,								\
	#PARENT,								\
	#NAME,									\
	FULLNAME,								\
	#YEAR,									\
	COMPANY,								\
	MACHINE_CONFIG_NAME(MACHINE),			\
	INPUT_PORTS_NAME(INPUT),				\
	DRIVER_INIT_NAME(INIT),					\
	ROM_NAME(NAME),							\
	#COMPAT,								\
	ROT0|(FLAGS)|GAME_TYPE_OTHER,			\
	NULL									\
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

GAME_EXTERN(___empty);

#endif
