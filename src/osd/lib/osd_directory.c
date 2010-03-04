/** **************************************************************************
 * osd_directory.c
 *
 * Copyright Nicola Salmoria and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

/**
 * This file contains some of the glue between the MAME osd layer and libmame.
 * It implements all of the osd_ functions that MAME calls that are related to
 * directory I/O, in such a way that they integrate with the API that libmame
 * provides.
 **/

#include "osdcore.h"
#include "osdepend.h"
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

osd_directory *osd_opendir(const char *dirname);


const osd_directory_entry *osd_readdir(osd_directory *dir);


void osd_closedir(osd_directory *dir);


int osd_is_absolute_path(const char *path);
