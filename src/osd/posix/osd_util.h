/** **************************************************************************
 * osd_util.h
 *
 * Copyright Nicola Salmoria and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#ifndef __OSD_UTIL_H__
#define __OSD_UTIL_H__

/**
 * Utility functions for implementing OSD API
 **/

/**
 * Converts [path] to a POSIX path, replacing PATH_SEPARATOR with '/'
 * wherever it appears.  Stores the result in [posix_path].  [pathsize]
 * gives the number of bytes available in [posix_path]; it is recommended
 * that this be at least the length of [path].  Returns 0 on success, nonzero
 * on failure due to [posix_path] not containing enough characters to hold the
 * converted path.
 **/
int convert_to_posix_path(const char *path, char *posix_path, int pathsize);


#endif /* __OSD_UTIL_H__ */


