/** **************************************************************************
 * osd_main.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "emu.h"
#include "options.h"

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    return mame_execute(0);
}
