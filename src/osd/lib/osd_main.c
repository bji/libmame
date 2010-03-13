/** **************************************************************************
 * osd_main.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "emu.h"
#include "emuopts.h"
#include "libmame.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    printf("LibMame version %s\n", LibMame_Get_Version_String());

    options_entry options_entries[1] = { { NULL } };

    core_options *core_options = mame_options_init(options_entries);

    int ret = mame_execute(core_options);

    if (core_options != NULL) {
        options_free(core_options);
    }

    LibMame_Deinitialize();

    return ret;
}
