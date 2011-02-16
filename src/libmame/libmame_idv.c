/** **************************************************************************
 * libmame_idv.c
 *
 * LibMame Initialize, Deinitialize and Version implementation.
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


LibMame_InitializeStatus LibMame_Initialize()
{
    /**
     * Currently does nothing, it's in the API so that the API doesn't have to
     * change if/when it is needed.
     **/
    return LibMame_InitializeStatus_Success;
}


void LibMame_Deinitialize()
{
    LibMame_Games_Deinitialize();
}
