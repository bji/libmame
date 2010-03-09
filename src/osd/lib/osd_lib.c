/** **************************************************************************
 * osd_lib.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "osdcore.h"
#include "osdepend.h"


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


void osd_customize_input_type_list(input_type_desc *typelist)
{
    (void) typelist;
}
