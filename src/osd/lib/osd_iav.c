/** **************************************************************************
 * osd_iav.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "osdcore.h"
#include "osdepend.h"


void osd_init(running_machine *machine)
{
    /* Create a render target */

    /* Create the keyboard device; the callback used to get its state will
       result in a call through the library callbacks */

    (void) machine;
}


void osd_update(running_machine *machine, int skip_redraw)
{
    /* Translate the render stuff into calls into the library callbacks */

    (void) machine;
    (void) skip_redraw;
}


void osd_update_audio_stream(running_machine *machine, INT16 *buffer,
                             int samples_this_frame)
{
    /* Call into library callbacks with audio data */

    (void) machine;
    (void) buffer;
    (void) samples_this_frame;
}


void osd_set_mastervolume(int attenuation)
{
    /* Call into library callback */

    (void) attenuation;
}


void osd_customize_input_type_list(input_type_desc *typelist)
{
    /* Customize according to library settings */

    (void) typelist;
}
