/** **************************************************************************
 * osd_iav.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "osdcore.h"
#include "osdepend.h"

/**
 * These are global pointers to functions which any application can set and
 * which will then be used to implement the MAME OSD functions that cannot
 * be implemented purely by POSIX.  Because there is no header file defining
 * these, external callers need to declare these symbols extern within their
 * own code.
 **/
void (*mame_osd_init_function)(running_machine *machine) = 0;
void (*mame_osd_update_function)(running_machine *machine,
                                 int skip_redraw) = 0;
void (*mame_osd_update_audio_stream_function)(running_machine *machine,
                                              INT16 *buffer,
                                              int samples_this_frame) = 0;
void (*mame_osd_set_mastervolume_function)(int attenuation) = 0;
void (*mame_osd_customize_input_type_list_function)
      (input_type_desc *typelist) = 0;


void osd_init(running_machine *machine)
{
    if (mame_osd_init_function) {
        (*mame_osd_init_function)(machine);
    }
}


void osd_update(running_machine *machine, int skip_redraw)
{
    if (mame_osd_update_function) {
        (*mame_osd_update_function)(machine, skip_redraw);
    }
}


void osd_update_audio_stream(running_machine *machine, INT16 *buffer,
                             int samples_this_frame)
{
    if (mame_osd_update_audio_stream_function) {
        (*mame_osd_update_audio_stream_function)(machine, buffer, 
                                                 samples_this_frame);
    }
}


void osd_set_mastervolume(int attenuation)
{
    if (mame_osd_set_mastervolume_function) {
        (*mame_osd_set_mastervolume_function)(attenuation);
    }
}


void osd_customize_input_type_list(input_type_desc *typelist)
{
    if (mame_osd_customize_input_type_list_function) {
        (*mame_osd_customize_input_type_list_function)(typelist);
    }
}
