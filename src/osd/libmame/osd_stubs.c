/** **************************************************************************
 * osd_stubs.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "osdepend.h"


/**
 * This is necessary because of the way that MAME is built.  We have to
 * provide implementations of these functions in the "libosdcore" library,
 * because of dependency order issues when linking MAME programs.  However, we
 * don't want to provide real libmame implementations here because then that
 * would introduce further dependency issues.  So instead we provide an empty
 * implementation that is "replaced" at runtime by libmame.  Probably weak
 * symbols or somesuch could be used to better effect, but that would be
 * nonportable I believe ...
 **/

/**
 * These function pointers are replaced by libmame at runtime
 **/

void (*osd_init_function)(running_machine *machine) = NULL;
void (*osd_update_function)(running_machine *machine, int skip_redraw) = NULL;
void (*osd_update_audio_stream_function)(running_machine *machine,
                                         INT16 *buffer,
                                         int samples_this_frame) = NULL;
void (*osd_set_mastervolume_function)(int attenuation) = NULL;
void (*osd_customize_input_type_list_function)
    (input_type_desc *typelist) = NULL;


void osd_init(running_machine *machine)
{
    if (osd_init_function != NULL) {
        (*osd_init_function)(machine);
    }
}


void osd_update(running_machine *machine, int skip_redraw)
{
    if (osd_update_function != NULL) {
        (*osd_update_function)(machine, skip_redraw);
    }
}


void osd_update_audio_stream(running_machine *machine, INT16 *buffer,
                             int samples_this_frame)
{
    if (osd_update_audio_stream_function != NULL) {
        (*osd_update_audio_stream_function)(machine, buffer, 
                                            samples_this_frame);
    }
}


void osd_set_mastervolume(int attenuation)
{
    if (osd_set_mastervolume_function) {
        (*osd_set_mastervolume_function)(attenuation);
    }
}


void osd_customize_input_type_list(input_type_desc *typelist)
{
    if (osd_customize_input_type_list_function) {
        (*osd_customize_input_type_list_function)(typelist);
    }
}
