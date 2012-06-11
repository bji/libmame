/***************************************************************************

    mame.c

    Specific (per target) constants

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************/
#include "emu.h"

#define APPNAME					"MAME"
#define APPNAME_LOWER			"mame"
#define CONFIGNAME				"mame"
#define APPLONGNAME				"M.A.M.E."
#define FULLLONGNAME			"Multiple Arcade Machine Emulator"
#define CAPGAMENOUN				"GAME"
#define CAPSTARTGAMENOUN		"Game"
#define GAMENOUN				"game"
#define GAMESNOUN				"games"
#define COPYRIGHT				"Copyright Nicola Salmoria\nand the MAME team\nhttp://mamedev.org"
#define COPYRIGHT_INFO			"Copyright Nicola Salmoria and the MAME team"
#define DISCLAIMER				"MAME is an emulator: it reproduces, more or less faithfully, the behaviour of\n" \
								"several arcade machines. But hardware is useless without software, so an image\n" \
								"of the ROMs which run on that hardware is required. Such ROMs, like any other\n" \
								"commercial software, are copyrighted material and it is therefore illegal to\n" \
								"use them if you don't own the original arcade machine. Needless to say, ROMs\n" \
								"are not distributed together with MAME. Distribution of MAME together with ROM\n" \
								"images is a violation of copyright law and should be promptly reported to the\n" \
								"authors so that appropriate legal action can be taken.\n"
#define USAGE					"Usage:  %s [%s] [options]"
#define XML_ROOT			    "mame"
#define XML_TOP 				"game"
#define STATE_MAGIC_NUM			"MAMESAVE"

const char * emulator_info::get_appname() { return APPNAME;}
const char * emulator_info::get_appname_lower() { return APPNAME_LOWER;}
const char * emulator_info::get_configname() { return CONFIGNAME;}
const char * emulator_info::get_applongname() { return APPLONGNAME;}
const char * emulator_info::get_fulllongname() { return FULLLONGNAME;}
const char * emulator_info::get_capgamenoun() { return CAPGAMENOUN;}
const char * emulator_info::get_capstartgamenoun() { return CAPSTARTGAMENOUN;}
const char * emulator_info::get_gamenoun() { return GAMENOUN;}
const char * emulator_info::get_gamesnoun() { return GAMESNOUN;}
const char * emulator_info::get_copyright() { return COPYRIGHT;}
const char * emulator_info::get_copyright_info() { return COPYRIGHT_INFO;}
const char * emulator_info::get_disclaimer() { return DISCLAIMER;}
const char * emulator_info::get_usage() { return USAGE;}
const char * emulator_info::get_xml_root() { return XML_ROOT;}
const char * emulator_info::get_xml_top() { return XML_TOP;}
const char * emulator_info::get_state_magic_num() { return STATE_MAGIC_NUM;}
void emulator_info::printf_usage(const char *par1, const char *par2) { mame_printf_info(USAGE, par1, par2); }
