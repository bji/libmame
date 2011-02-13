/** **************************************************************************
 * osd_main.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include <stdio.h>

int main(int argc, char **argv)
{
    fprintf(stderr, "ERROR: this program was compiled using OSD=posix, "
            "which does not produce\nviable MAME executables.  Please select "
            "a different OSD and recompile MAME.\n");

    return -1;
}
