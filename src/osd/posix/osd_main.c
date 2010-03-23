/** **************************************************************************
 * osd_main.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "clifront.h"
#include "osdepend.h"

/**
 * Provide dummy implementations of OSD functions not otherwise provided by
 * the POSIX OSD layer.
 **/

void osd_init(running_machine *machine)
{
    (void) machine;
}

void osd_update(running_machine *machine, int skip_redraw)
{
    (void) machine;
    (void) skip_redraw;
}

void osd_update_audio_stream(running_machine *machine, INT16 *buffer,
                             int samples_this_frame)
{
    (void) machine;
    (void) buffer;
    (void) samples_this_frame;
}

void osd_set_mastervolume(int attenuation)
{
    (void) attenuation;
}

int main(int argc, char **argv)
{
    /**
     * This version of OSD is not really meant to be used in a standalone
     * version of a MAME executable.  As a result, this will do pretty
     * much nothing of value.
     **/
	return cli_execute(argc, argv, 0);
}
