/** **************************************************************************
 * osd_util.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/


#include <string.h>
#include "osdcore.h"
#include "osd_util.h"

int convert_to_posix_path(const char *path, char *posix_path, int pathsize)
{
    char *posix_path_end = &(posix_path[pathsize - 1]);

    while (*path)
    {
        if (posix_path == posix_path_end)
        {
            return 1;
        }
        if (!strncmp(path, PATH_SEPARATOR, sizeof(PATH_SEPARATOR) - 1))
        {
            *posix_path++ = '/';
            path += sizeof(PATH_SEPARATOR) - 1;
        }
        else
        {
            *posix_path++ = *path++;
        }
    }

    *posix_path = 0;

    return 0;
}
