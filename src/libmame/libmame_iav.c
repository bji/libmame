/** **************************************************************************
 * libmame_iav.c
 *
 * LibMame Input, Audio, and Video function implementations.
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "osdcore.h"
#include "osdepend.h"


/**
 * These are defined by the POSIX OSD implementation, and are meant to be
 * replaced when a game is run.
 **/
extern void (*mame_osd_init_function)(running_machine *machine);
extern void (*mame_osd_update_function)(running_machine *machine,
                                        int skip_redraw);
extern void (*mame_osd_update_audio_stream_function)(running_machine *machine,
                                                     INT16 *buffer,
                                                     int samples_this_frame);
extern void (*mame_osd_set_mastervolume_function)(int attenuation);
extern void (*mame_osd_customize_input_type_list_function)
    (input_type_desc *typelist);


/**
 * This global is unfortunate but necessary.  If MAME is enhanced to support
 * callback data in its callbacks, then it will be unnecessary.
 **/
#if 0
static xxx yyy;
#endif


static void libmame_osd_init(running_machine *machine)
{
    /**
     * Do LibMame init stuff
     **/

    /**
     * Call back the callback provided in the RunGame() structure to get
     * something that a MAME display thing can be created from.
     **/
}


static void libmame_osd_update(running_machine *machine, int skip_redraw)
{
    /**
     * Call back the callback provided in the RunGame() function.  Hopefully
     * it will not require converting too much to new structures.
     **/
}


static void libmame_osd_update_audio_stream(running_machine *machine,
                                            INT16 *buffer,
                                            int samples_this_frame)
{
    /**
     * Call back the callback provided in the RunGame() function.
     **/
}


static void libmame_osd_set_mastervolume(int attenuation)
{
    /**
     * Call back the callback provided in the RunGame() function.
     **/
}


static void libmame_osd_customize_input_type_list(input_type_desc *typelist)
{
    /**
     * Update the input list based on the values passed into RunGame()
     **/
}


void LibMame_RunGame()
{
    /* Ensure that the OSD callbacks are set up.  This may be redundant if
       LibMame_RunGame() was called previously, but will be a harmless no-op
       in that case. */
    mame_osd_init_function = &libmame_osd_init;
    mame_osd_update_function = &libmame_osd_update;
    mame_osd_update_audio_stream_function = &libmame_osd_update_audio_stream;
    mame_osd_set_mastervolume_function = &libmame_osd_set_mastervolume;
    mame_osd_customize_input_type_list_function =
        &libmame_osd_customize_input_type_list;

    /* Call MAME */
}
