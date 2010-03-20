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
#include "HashTable.h"
#include "osdcore.h"
#include "osdepend.h"
#include "libmame.h"
#include <string.h>

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
     * This is the maximum number of players that could be playing the
     * currently running game.  It is stored here as an optimization so that
     * we can know how many of the controller states need to be zeroed out
     * before each controller state poll
     **/
    int running_game_maximum_player_count;
    LibMame_ControllersState controllers_states[8];

    /* Hash MAME input_item_id to a DIGCTL value for each keyboard key */
    static Hash::Table<int, input_item_id> digctl_to_key_id_hash;

} LibMame_RunGame_State;


/**
 * This is a packed representation of everything necessary to identify a
 * unique digital control.  We map each "key" from the huge virtual keyboard
 * device that we register with MAME to one of these, and store this in
 * a hashtable so that we can look up this information given a MAME key.
 * Then we register the keyboard device so that it has a single "key" for
 * each of these digital controls.  Finally, when devices are enumerated,
 * we hook each device up so that MAME looks for input for it from the
 * correct "key".
 **/
#define DIGCTL_PLAYER_MASK                       0x000F
#define DIGCTL_TYPE_MASK                         0x00F0
#define DIGCTL_ITEM_MASK                         0xFF00
#define DIGCTL_TYPE_NORMAL_BUTTON                0x0010
#define DIGCTL_TYPE_LEFT_OR_SINGLE_JOYSTICK      0x0020
#define DIGCTL_TYPE_RIGHT_JOYSTICK               0x0030
#define DIGCTL_TYPE_OTHER_BUTTON                 0x0040
#define DIGCTL_TYPE_UI_BUTTON                    0x0050
#define DIGCTL_TYPE_MAHJONG_BUTTON               0x0060
#define DIGCTL_TYPE_HANAFUDA_BUTTON              0x0070
#define DIGCTL_TYPE_GAMBLING_BUTTON              0x0080
#define DIGCTL_JOYSTICK_LEFT                     0x0100
#define DIGCTL_JOYSTICK_RIGHT                    0x0200
#define DIGCTL_JOYSTICK_UP                       0x0400
#define DIGCTL_JOYSTICK_DOWN                     0x0800
#define DIGCTL_MAKE_BUTTON(player_number, type, item_number) \
    ((player_number) | (type) | ((item_number) << 8))
#define DIGCTL_MAKE_JOYSTICK(player_number, type, joystick) \
    ((player_number) | (type) | (joystick))
#define DIGCTL_PLAYER_NUMBER(d)       (((int) d) & DIGCTL_PLAYER_MASK)
#define DIGCTL_TYPE(d)                (((int) d) & DIGCTL_TYPE_MASK)
#define DIGCTL_ITEM_NUMBER(d)         ((((int) d) & DIGCTL_ITEM_MASK) >> 8)
#define DIGCTL_JOYSTICK(d)            (((int) d) & DIGCTL_ITEM_MASK)

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
 * This global is unfortunate but necessary.  If MAME is enhanced to
 * support callback data in its callbacks, then it will be unnecessary.
 **/
static LibMame_RunGame_State g_state;


static INT32 get_digital_controller_state(void *, void *data)
{
    /* Get the controller state associated with the given digital control
       descriptor */
    LibMame_ControllerState *state = 
        &(g_state.controllers_states[DIGCTL_PLAYER_NUMBER(data)]);

    /* Get the particular state according to the type of controller state
       being asked about */
    switch (DIGCTL_TYPE(data)) {
    case DIGCTL_TYPE_NORMAL_BUTTON:
        /* This logic is used here and for the other button types.  Basically
           it is doing:
           - Get the device item number out of the packed data
           - Left shift this value to get the LIBMAME_CONTROLLERFLAGS_XXX
             flag
           - Return the state of that flag
        **/
        return (state->normal_buttons_state & (1 << (DIGCTL_ITEM_NUMBER(d))));
    case DIGCTL_TYPE_LEFT_OR_SINGLE_JOYSTICK:
        switch (DIGCTL_JOYSTICK(d)) {
        case DIGCTL_JOYSTICK_LEFT:
            return state->left_or_single_joystick_left_state;
        case DIGCTL_JOYSTICK_RIGHT:
            return state->left_or_single_joystick_right_state;
        case DIGCTL_JOYSTICK_UP:
            return state->left_or_single_joystick_up_state;
        case DIGCTL_JOYSTICK_DOWN:
            return state->left_or_single_joystick_down_state;
        default:
            break;
        }
    case DIGCTL_TYPE_RIGHT_JOYSTICK:
        switch (DIGCTL_JOYSTICK(d)) {
        case DIGCTL_JOYSTICK_LEFT:
            return state->right_joystick_left_state;
        case DIGCTL_JOYSTICK_RIGHT:
            return state->right_joystick_right_state;
        case DIGCTL_JOYSTICK_UP:
            return state->right_joystick_up_state;
        case DIGCTL_JOYSTICK_DOWN:
            return state->right_joystick_down_state;
        default:
            break;
        }
    case DIGCTL_TYPE_OTHER_BUTTON:
        return (state->other_buttons_state & (1 << (DIGCTL_ITEM_NUMBER(d))));
    case DIGCTL_TYPE_UI_BUTTON:
        /* UI input is not a flag, it is an actual value to check against */
        return (state->ui_input_state == DIGCTL_ITEM_NUMBER(d));
    case DIGCTL_TYPE_MAHJONG_BUTTON:
        return (state->mahjong_buttons_state & (1 << (DIGCTL_ITEM_NUMBER(d))));
    case DIGCTL_TYPE_HANAFUDA_BUTTON:
        return (state->hanafuda_buttons_state & (1 << (DIGCTL_ITEM_NUMBER(d))));
    case DIGCTL_TYPE_GAMBLING_BUTTON:
        return (state->gambling_buttons_state & (1 << (DIGCTL_ITEM_NUMBER(d))));
    }

    /* Weird, this is not a 'key' that we know about */
    return 0;
}


static void libmame_add_keyboard_button(input_device *kbd, int key_id,
                                        int digctl)
{
    /* Create an input device for the keyboard that will have the callback
       called back with the digital control descriptor and that will be
       associated with this key for this keyboard */
    input_device_item_add(kbd, "", (void *) digctl, key_id,
                          &get_digital_controller_state);
    /* And hash so that we can look up the key by the digital control when
       customizing the inputs in libmame_osd_customize_input_type_list */
    input_item_id *value = g_state.digctl_to_key_id_hash.Put(digctl);
    *value = key_id;
}


static void libmame_osd_init(running_machine *machine)
{
    /**
     * Create a huge virtual keyboards to cover all possible buttons.
     **/
    input_device *kbd = input_device_add(machine, DEVICE_CLASS_KEYBOARD, 
                                         "libmame_virtual_keyboard", NULL);

    input_item_id key_id = ITEM_ID_FIRST_VALID;

    /* For each player ... */
    for (int player = 0; i < 8; i++) {
        /* Do the normal buttons */
        for (int button = 0; i < 16; i++) {
            libmame_add_keyboard_button
                (kbd, key_id, DIGCTL_MAKE_BUTTON
                 (player, DIGCTL_TYPE_NORMAL_BUTTON, button));
            /* Next key */
            key_id++;
        }
        /* Do the mahjong buttons */
        for (int button = 0; i < 16; i++) {
            libmame_add_keyboard_button
                (kbd, key_id, DIGCTL_MAKE_BUTTON
                 (player, DIGCTL_TYPE_NORMAL_BUTTON, button));
            /* Next key */
            key_id++;
        }
        /* Do the hanafuda buttons */
        /* Do the gambling buttons */
        /* Do the other buttons */
        /* Do the left or single handed joystick */
        /* Do the right joystick */
    }

    /**
     * Create all of the possible analog controls as mice?
     **/

    /**
     * Create the display
     **/
}


static void libmame_osd_update(running_machine *machine, int skip_redraw)
{
    /**
     * Poll input
     **/
    memset(g_state.controllers_states, 0, sizeof(LibMame_ControllersState) * 
           g_state.running_game_maximum_player_count);
    (*(g_state.callbacks->PollControllersStates))(g_state.controllers_states, 
                                                  g_state.callback_data);

    /**
     * Give the callbacks a chance to make running game calls
     **/
    (*(g_state.callbacks->MakeRunningGameCalls))(g_state.callback_data);

    /**
     * And ask the callbacks to update the video
     **/
    (*(g_state.callbacks->UpdateVideo))(g_state.callback_data);
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


static void libmame_osd_customize_input_type_list(input_type_desc *typelist)
{
    /**
     * Examine every input.  For each one, if it doesn't have an input_code
     * assigned to it, assign a new one.  Then, figure out which player
     * and controller it maps to, and update the state map so that when
     * that state is polled, it emits that input_code.
     **/

    /**
     * Set up MAME to use a fixed key set that matches the tables that we use.
     **/
	input_type_desc *typedesc;
	for (typedesc = typelist; typedesc; typedesc = typedesc->next) {
        switch (typedesc->type) {
            
        }
    }
}


LibMame_RunGameStatus LibMame_RunGame(int gamenum,
                                      /* Some options thing xxx, */
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

    /* Set up options stuff for MAME */

    /* Look up the game */

    /* Set the unfortunate globals */
    g_state.callbacks = cbs;
    g_state.callback_data = callback_data;
    g_state.running_game_maximum_player_count = 
        LibMame_Get_Game_MaxSimultaneousPlayers(gamenum);

    /* Run the game */

    return LibMame_RunGameStatus_Success;
}
