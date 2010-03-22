/** **************************************************************************
 * libmame_iav.c
 *
 * LibMame Input, Audio, and Video function implementations.
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "emu.h"
#include "osdcore.h"
#include "osdepend.h"
#include "render.h"
#include "libmame.h"
#include <stddef.h>
#include <string.h>


/** **************************************************************************
 * External symbol references
 ************************************************************************** **/

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
 * These are defined by other libmame source files.
 **/
extern void libmame_osd_customize_input_type_list(input_type_desc *typelist);
extern mame_options *get_mame_options(const LibMame_RunGameOptions *options);


/** **************************************************************************
 * Structured Type Definitions
 ************************************************************************** **/

/**
 * This encapsulates all of the state that LibMame keeps track of during
 * LibMame_RunGame().
 **/
typedef struct LibMame_RunGame_State
{
    /**
     * These are the callbacks that were provided to LibMame_RunGame.
     **/
    const LibMame_RunGameCallbacks *callbacks;

    /**
     * This is the callback data that was provided to LibMame_RunGame.
     **/
    void *callback_data;

    /**
     * This is the number of the game being played
     **/
    int gamenum;

    /**
     * These describe the maximum number of players, and the controllers
     * that they use.
     **/
    int maximum_player_count;
    LibMame_PerPlayerControllersDescriptor perplayer_controllers;
    LibMame_SharedControllersDescriptor shared_controllers;

    /**
     * This is the controllers state used to query the controllers state via
     * the callback provided in the callbacks structure.
     **/
    LibMame_AllControllersState controllers_state;

    /**
     * This is the running machine that this state is associated with
     **/
    running_machine *machine;

} LibMame_RunGame_State;


/** **************************************************************************
 * Static global variables
 ************************************************************************** **/

/**
 * This global is unfortunate but necessary.  If MAME is enhanced to
 * support callback data in its callbacks, then it will be unnecessary.
 **/
static LibMame_RunGame_State g_state;


/** **************************************************************************
 * Static OSD function implementations
 ************************************************************************** **/

static void libmame_osd_init(running_machine *machine)
{
    /**
     *  Save away the machine, we'll need it in 
     * libmame_osd_customize_input_type_list
     **/
    g_state.machine = machine;

    /**
     * Create the render_target that tells MAME the rendering parameters it
     * will use.
     **/
    render_target *target = render_target_alloc(g_state.machine, NULL, 0);
    /* Set it up to be the same size as the game's original display, if it's
       a raster display; then any stretching to the actual display hardware
       will be done by the update callback. */
    if (LibMame_Get_Game_ScreenType(g_state.gamenum) != 
        LibMame_ScreenType_Vector) {
        LibMame_ScreenResolution res = LibMame_Get_Game_ScreenResolution
            (g_state.gamenum);
        render_target_set_bounds(target, res.width, res.height, 0.0);
    }
}


static void libmame_osd_update(running_machine *machine, int skip_redraw)
{
    /**
     * Poll input
     **/
    memset(g_state.controllers_state.per_player, 0, 
           sizeof(LibMame_PerPlayerControllersState) * 
           g_state.maximum_player_count);
    g_state.controllers_state.shared.other_buttons_state = 0;

    (*(g_state.callbacks->PollAllControllersState))
        (&(g_state.controllers_state), g_state.callback_data);

    /**
     * Give the callbacks a chance to make running game calls
     **/
    (*(g_state.callbacks->MakeRunningGameCalls))(g_state.callback_data);

    /**
     * And ask the callbacks to update the video
     **/
    (*(g_state.callbacks->UpdateVideo))(g_state.callback_data);

    /* TEMPORARY FOR TESTING */
    static int exit_count = 0;
    if (++exit_count == 1000) {
		mame_schedule_exit(machine);
    }
    else if ((exit_count % 50) == 0) {
        printf("exit_count is now %d\n", exit_count);
    }
}


static void libmame_osd_update_audio_stream(running_machine *machine,
                                            INT16 *buffer,
                                            int samples_this_frame)
{
    /**
     * Ask the callbacks to update the audio
     **/
    (*(g_state.callbacks->UpdateAudio))(g_state.callback_data);
}


static void libmame_osd_set_mastervolume(int attenuation)
{
    (*(g_state.callbacks->SetMasterVolume))(attenuation, g_state.callback_data);
}


/** **************************************************************************
 * LibMame exported function implementations
 ************************************************************************** **/

LibMame_RunGameStatus LibMame_RunGame(int gamenum,
                                      const LibMame_RunGameOptions *options,
                                      const LibMame_RunGameCallbacks *cbs,
                                      void *callback_data)
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

    /* Set the unfortunate globals.  Would greatly prefer to allocate a
       new one of these and pass it to MAME, having it pass it back in the
       osd_ callbacks. */
    g_state.callbacks = cbs;
    g_state.callback_data = callback_data;

    /* Save the game number */
    g_state.gamenum = gamenum;

    /* Look up the game number of players and controllers */
    g_state.maximum_player_count =
        LibMame_Get_Game_MaxSimultaneousPlayers(gamenum);
    g_state.perplayer_controllers = 
        LibMame_Get_Game_PerPlayerControllers(gamenum);
    g_state.shared_controllers = 
        LibMame_Get_Game_SharedControllers(gamenum);

    /* Set up options stuff for MAME */
    core_options *mame_options = get_mame_options(options);

    /* TEMPORARY FOR TESTING */
    const char *short_name = LibMame_Get_Game_Short_Name(gamenum);
    const char *argv[] = { "libmame_test", "-rompath", "test_roms",
                           "-aviwrite", "libmame.avi", short_name };

    /* Run the game */
    /* TEMPORARY FOR TESTING */
    int result = mame_execute(mame_options);

    switch (result) {
    default:
        break;
    }

    return LibMame_RunGameStatus_Success;
}
