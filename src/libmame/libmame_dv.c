/** **************************************************************************
 * libmame_dv.c
 *
 * LibMame Deinitialize and Version implementation.
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "libmame.h"

/**
 * Support for setting up libmame, and getting libmame version
 **/

extern const char build_version[];
extern bool LibMame_Games_Deinitialize();


const char *LibMame_Get_Version_String()
{
    return build_version;
}


void LibMame_Deinitialize()
{
    LibMame_Games_Deinitialize();
}
