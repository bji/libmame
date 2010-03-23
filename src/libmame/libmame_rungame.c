/** **************************************************************************
 * libmame_rungame.c
 *
 * LibMame RunGame implementation.
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
 * These are exported by other source files within libmame itself
 **/
extern core_options *get_mame_options(const LibMame_RunGameOptions *options,
                                      const char *gamename);


/** **************************************************************************
 * Enumeration Type Definition
 ************************************************************************** **/

typedef enum
{
    libmame_input_type_invalid,
    libmame_input_type_Normal_button,
    libmame_input_type_Mahjong_button,
    libmame_input_type_Hanafuda_button,
    libmame_input_type_Gambling_button,
    libmame_input_type_Other_button,
    libmame_input_type_left_or_single_joystick_left,
    libmame_input_type_left_or_single_joystick_right,
    libmame_input_type_left_or_single_joystick_up,
    libmame_input_type_left_or_single_joystick_down,
    libmame_input_type_right_joystick_left,
    libmame_input_type_right_joystick_right,
    libmame_input_type_right_joystick_up,
    libmame_input_type_right_joystick_down,
    libmame_input_type_analog_joystick_horizontal,
    libmame_input_type_analog_joystick_vertical,
    libmame_input_type_analog_joystick_altitude,
    libmame_input_type_spinner,
    libmame_input_type_spinner_vertical,
    libmame_input_type_paddle,
    libmame_input_type_paddle_vertical,
    libmame_input_type_trackball_horizontal,
    libmame_input_type_trackball_vertical,
    libmame_input_type_lightgun_horizontal,
    libmame_input_type_lightgun_vertical,
    libmame_input_type_pedal,
    libmame_input_type_pedal2,
    libmame_input_type_pedal3,
    libmame_input_type_positional,
    libmame_input_type_positional_vertical,
    libmame_input_type_mouse_x,
    libmame_input_type_mouse_y,
    libmame_input_type_Ui_button
} libmame_input_type;


/** **************************************************************************
 * Structured Type Definitions
 ************************************************************************** **/

/**
 * Each IPT_ input port type has a corresponding entry in a table here that
 * provides enough details about that IPT_ entry to locate its state value
 * in the controllers state and to make the code that sets up the input
 * devices more concise.
 **/
typedef struct libmame_input_descriptor
{
    libmame_input_type type;
    int number;
    /* This is the input item id will should be used for this input type,
       or ITEM_ID_INVALID to use a generated key */
    input_item_id item_id;
} libmame_input_descriptor;


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
    LibMame_AllControllersDescriptor controllers;

    /**
     * This is the controllers state used to query the controllers state via
     * the callback provided in the callbacks structure.
     **/
    LibMame_AllControllersState controllers_state;

    /**
     * This is the running machine that this state is associated with
     **/
    running_machine *machine;

    /**
     * This is the render target for the game
     **/
    render_target *target;

} LibMame_RunGame_State;


/** **************************************************************************
 * Helper Macros
 ************************************************************************** **/

/**
 * This is a packed representation of everything necessary to identify the
 * controller that MAME is asking about.
 **/
#define CBDATA_PLAYER_MASK              0x000F
#define CBDATA_IPT_MASK                 0xFFF0
#define CBDATA_PLAYER(d)                (((long) d) & CBDATA_PLAYER_MASK)
#define CBDATA_IPT(d)                   ((((long) d) & CBDATA_IPT_MASK) >> 8)
#define CBDATA_MAKE(player, ipt)        (((player) & CBDATA_PLAYER_MASK) | \
                                         ((ipt << 8) & CBDATA_IPT_MASK))

/* These macros make the following table definition more conscise */
#define INVALID_INPUT \
    { libmame_input_type_invalid, 0, ITEM_ID_INVALID }
#define BUTTON_INPUT(button_type, button_name)                  \
    { libmame_input_type_##button_type##_button,                \
      LibMame_##button_type##ButtonType_##button_name,          \
      ITEM_ID_INVALID }
#define JOYSTICK_INPUT(joystick_type, direction) \
    { libmame_input_type_##joystick_type##_joystick_##direction,    \
      0, ITEM_ID_INVALID }
#define ANALOG_INPUT(input_type, input_item_id) \
    { libmame_input_type_##input_type, 0, input_item_id }


/** **************************************************************************
 * Static global variables
 ************************************************************************** **/

/* This maps each MAME IPT_ type to a libmame_input descriptor. */
static libmame_input_descriptor g_input_descriptors[] =
{
	INVALID_INPUT, /* IPT_INVALID */
	INVALID_INPUT, /* IPT_UNUSED */
	INVALID_INPUT, /* IPT_END */
	INVALID_INPUT, /* IPT_UNKNOWN */
	INVALID_INPUT, /* IPT_PORT */
	INVALID_INPUT, /* IPT_DIPSWITCH */
	INVALID_INPUT, /* IPT_VBLANK */
	INVALID_INPUT, /* IPT_CONFIG */
	INVALID_INPUT, /* IPT_CATEGORY */
    BUTTON_INPUT(Other, Start1), /* IPT_START1 */
	BUTTON_INPUT(Other, Start2), /* IPT_START2 */
	BUTTON_INPUT(Other, Start3), /* IPT_START3 */
	BUTTON_INPUT(Other, Start4), /* IPT_START4 */
	BUTTON_INPUT(Other, Start5), /* IPT_START5 */
	BUTTON_INPUT(Other, Start6), /* IPT_START6 */
	BUTTON_INPUT(Other, Start7), /* IPT_START7 */
	BUTTON_INPUT(Other, Start8), /* IPT_START8 */
	BUTTON_INPUT(Other, Coin1), /* IPT_COIN1 */
	BUTTON_INPUT(Other, Coin2), /* IPT_COIN2 */
	BUTTON_INPUT(Other, Coin3), /* IPT_COIN3 */
	BUTTON_INPUT(Other, Coin4), /* IPT_COIN4 */
	BUTTON_INPUT(Other, Coin5), /* IPT_COIN5 */
	BUTTON_INPUT(Other, Coin6), /* IPT_COIN6 */
	BUTTON_INPUT(Other, Coin7), /* IPT_COIN7 */
	BUTTON_INPUT(Other, Coin8), /* IPT_COIN8 */
	BUTTON_INPUT(Other, Bill1), /* IPT_BILL1 */
	BUTTON_INPUT(Other, Service1), /* IPT_SERVICE1 */
	BUTTON_INPUT(Other, Service2), /* IPT_SERVICE2 */
	BUTTON_INPUT(Other, Service3), /* IPT_SERVICE3 */
	BUTTON_INPUT(Other, Service4), /* IPT_SERVICE4 */
	BUTTON_INPUT(Other, Service), /* IPT_SERVICE */
	BUTTON_INPUT(Other, Tilt), /* IPT_TILT */
	BUTTON_INPUT(Other, Interlock), /* IPT_INTERLOCK */
	BUTTON_INPUT(Other, Volume_Up), /* IPT_VOLUME_UP */
	BUTTON_INPUT(Other, Volume_Down), /* IPT_VOLUME_DOWN */
	INVALID_INPUT, /* IPT_START */
	INVALID_INPUT, /* IPT_SELECT */
	INVALID_INPUT, /* IPT_KEYBOARD */
	JOYSTICK_INPUT(left_or_single, up), /* IPT_JOYSTICK_UP */
	JOYSTICK_INPUT(left_or_single, down), /* IPT_JOYSTICK_DOWN */
	JOYSTICK_INPUT(left_or_single, left), /* IPT_JOYSTICK_LEFT */
	JOYSTICK_INPUT(left_or_single, right), /* IPT_JOYSTICK_RIGHT */
	JOYSTICK_INPUT(right, up), /* IPT_JOYSTICKRIGHT_UP */
	JOYSTICK_INPUT(right, down), /* IPT_JOYSTICKRIGHT_DOWN */
	JOYSTICK_INPUT(right, left), /* IPT_JOYSTICKRIGHT_LEFT */
	JOYSTICK_INPUT(right, right), /* IPT_JOYSTICKRIGHT_RIGHT */
	JOYSTICK_INPUT(left_or_single, up), /* IPT_JOYSTICKLEFT_UP */
	JOYSTICK_INPUT(left_or_single, down), /* IPT_JOYSTICKLEFT_DOWN */
	JOYSTICK_INPUT(left_or_single, left), /* IPT_JOYSTICKLEFT_LEFT */
	JOYSTICK_INPUT(left_or_single, right), /* IPT_JOYSTICKLEFT_RIGHT */
	BUTTON_INPUT(Normal, 1), /* IPT_BUTTON1 */
	BUTTON_INPUT(Normal, 2), /* IPT_BUTTON2 */
	BUTTON_INPUT(Normal, 3), /* IPT_BUTTON3 */
	BUTTON_INPUT(Normal, 4), /* IPT_BUTTON4 */
	BUTTON_INPUT(Normal, 5), /* IPT_BUTTON5 */
	BUTTON_INPUT(Normal, 6), /* IPT_BUTTON6 */
	BUTTON_INPUT(Normal, 7), /* IPT_BUTTON7 */
	BUTTON_INPUT(Normal, 8), /* IPT_BUTTON8 */
	BUTTON_INPUT(Normal, 9), /* IPT_BUTTON9 */
	BUTTON_INPUT(Normal, 10), /* IPT_BUTTON10 */
	BUTTON_INPUT(Normal, 11), /* IPT_BUTTON11 */
	BUTTON_INPUT(Normal, 12), /* IPT_BUTTON12 */
	BUTTON_INPUT(Normal, 13), /* IPT_BUTTON13 */
	BUTTON_INPUT(Normal, 14), /* IPT_BUTTON14 */
	BUTTON_INPUT(Normal, 15), /* IPT_BUTTON15 */
	BUTTON_INPUT(Normal, 16), /* IPT_BUTTON16 */
	BUTTON_INPUT(Mahjong, A), /* IPT_MAHJONG_A */
	BUTTON_INPUT(Mahjong, B), /* IPT_MAHJONG_B */
	BUTTON_INPUT(Mahjong, C), /* IPT_MAHJONG_C */
	BUTTON_INPUT(Mahjong, D), /* IPT_MAHJONG_D */
	BUTTON_INPUT(Mahjong, E), /* IPT_MAHJONG_E */
	BUTTON_INPUT(Mahjong, F), /* IPT_MAHJONG_F */
	BUTTON_INPUT(Mahjong, G), /* IPT_MAHJONG_G */
	BUTTON_INPUT(Mahjong, H), /* IPT_MAHJONG_H */
	BUTTON_INPUT(Mahjong, I), /* IPT_MAHJONG_I */
	BUTTON_INPUT(Mahjong, J), /* IPT_MAHJONG_J */
	BUTTON_INPUT(Mahjong, K), /* IPT_MAHJONG_K */
	BUTTON_INPUT(Mahjong, L), /* IPT_MAHJONG_L */
	BUTTON_INPUT(Mahjong, M), /* IPT_MAHJONG_M */
	BUTTON_INPUT(Mahjong, N), /* IPT_MAHJONG_N */
	BUTTON_INPUT(Mahjong, O), /* IPT_MAHJONG_O */
	BUTTON_INPUT(Mahjong, P), /* IPT_MAHJONG_P */
	BUTTON_INPUT(Mahjong, Q), /* IPT_MAHJONG_Q */
	BUTTON_INPUT(Mahjong, Kan), /* IPT_MAHJONG_KAN */
	BUTTON_INPUT(Mahjong, Pon), /* IPT_MAHJONG_PON */
	BUTTON_INPUT(Mahjong, Chi), /* IPT_MAHJONG_CHI */
	BUTTON_INPUT(Mahjong, Reach), /* IPT_MAHJONG_REACH */
	BUTTON_INPUT(Mahjong, Ron), /* IPT_MAHJONG_RON */
	BUTTON_INPUT(Mahjong, Bet), /* IPT_MAHJONG_BET */
	BUTTON_INPUT(Mahjong, Last_Chance), /* IPT_MAHJONG_LAST_CHANCE */
	BUTTON_INPUT(Mahjong, Score), /* IPT_MAHJONG_SCORE */
	BUTTON_INPUT(Mahjong, Double_Up), /* IPT_MAHJONG_DOUBLE_UP */
	BUTTON_INPUT(Mahjong, Flip_Flop), /* IPT_MAHJONG_FLIP_FLOP */
	BUTTON_INPUT(Mahjong, Big), /* IPT_MAHJONG_BIG */
	BUTTON_INPUT(Mahjong, Small), /* IPT_MAHJONG_SMALL */
	BUTTON_INPUT(Hanafuda, A), /* IPT_HANAFUDA_A */
	BUTTON_INPUT(Hanafuda, B), /* IPT_HANAFUDA_B */
	BUTTON_INPUT(Hanafuda, C), /* IPT_HANAFUDA_C */
	BUTTON_INPUT(Hanafuda, D), /* IPT_HANAFUDA_D */
	BUTTON_INPUT(Hanafuda, E), /* IPT_HANAFUDA_E */
	BUTTON_INPUT(Hanafuda, F), /* IPT_HANAFUDA_F */
	BUTTON_INPUT(Hanafuda, G), /* IPT_HANAFUDA_G */
	BUTTON_INPUT(Hanafuda, H), /* IPT_HANAFUDA_H */
	BUTTON_INPUT(Hanafuda, Yes), /* IPT_HANAFUDA_YES */
	BUTTON_INPUT(Hanafuda, No), /* IPT_HANAFUDA_NO */
	BUTTON_INPUT(Gambling, High), /* IPT_GAMBLE_HIGH */
	BUTTON_INPUT(Gambling, Low), /* IPT_GAMBLE_LOW */
	BUTTON_INPUT(Gambling, Half), /* IPT_GAMBLE_HALF */
	BUTTON_INPUT(Gambling, Deal), /* IPT_GAMBLE_DEAL */
	BUTTON_INPUT(Gambling, D_Up), /* IPT_GAMBLE_D_UP */
	BUTTON_INPUT(Gambling, Take), /* IPT_GAMBLE_TAKE */
	BUTTON_INPUT(Gambling, Stand), /* IPT_GAMBLE_STAND */
	BUTTON_INPUT(Gambling, Bet), /* IPT_GAMBLE_BET */
	BUTTON_INPUT(Gambling, Keyin), /* IPT_GAMBLE_KEYIN */
	BUTTON_INPUT(Gambling, Keyout), /* IPT_GAMBLE_KEYOUT */
	BUTTON_INPUT(Gambling, Payout), /* IPT_GAMBLE_PAYOUT */
	BUTTON_INPUT(Gambling, Door), /* IPT_GAMBLE_DOOR */
	BUTTON_INPUT(Gambling, Service), /* IPT_GAMBLE_SERVICE */
	BUTTON_INPUT(Gambling, Book), /* IPT_GAMBLE_BOOK */
	BUTTON_INPUT(Gambling, Hold1), /* IPT_POKER_HOLD1 */
	BUTTON_INPUT(Gambling, Hold2), /* IPT_POKER_HOLD2 */
	BUTTON_INPUT(Gambling, Hold3), /* IPT_POKER_HOLD3 */
	BUTTON_INPUT(Gambling, Hold4), /* IPT_POKER_HOLD4 */
	BUTTON_INPUT(Gambling, Hold5), /* IPT_POKER_HOLD5 */
	BUTTON_INPUT(Gambling, Cancel), /* IPT_POKER_CANCEL */
	BUTTON_INPUT(Gambling, Bet), /* IPT_POKER_BET */
	BUTTON_INPUT(Gambling, Stop1), /* IPT_SLOT_STOP1 */
	BUTTON_INPUT(Gambling, Stop2), /* IPT_SLOT_STOP2 */
	BUTTON_INPUT(Gambling, Stop3), /* IPT_SLOT_STOP3 */
	BUTTON_INPUT(Gambling, Stop4), /* IPT_SLOT_STOP4 */
	BUTTON_INPUT(Gambling, Stop_All), /* IPT_SLOT_STOP_ALL */
	ANALOG_INPUT(paddle, ITEM_ID_XAXIS), /* IPT_PADDLE */
	ANALOG_INPUT(paddle_vertical, ITEM_ID_YAXIS), /* IPT_PADDLE_V */
  ANALOG_INPUT(analog_joystick_horizontal, ITEM_ID_XAXIS), /* IPT_AD_STICK_X */
	ANALOG_INPUT(analog_joystick_vertical, ITEM_ID_YAXIS), /* IPT_AD_STICK_Y */
	ANALOG_INPUT(analog_joystick_altitude, ITEM_ID_ZAXIS), /* IPT_AD_STICK_Z */
	ANALOG_INPUT(lightgun_horizontal, ITEM_ID_XAXIS), /* IPT_LIGHTGUN_X */
	ANALOG_INPUT(lightgun_vertical, ITEM_ID_YAXIS), /* IPT_LIGHTGUN_Y */
	ANALOG_INPUT(pedal, ITEM_ID_XAXIS), /* IPT_PEDAL */
	ANALOG_INPUT(pedal2, ITEM_ID_YAXIS), /* IPT_PEDAL2 */
	ANALOG_INPUT(pedal3, ITEM_ID_ZAXIS), /* IPT_PEDAL3 */
	ANALOG_INPUT(positional, ITEM_ID_XAXIS), /* IPT_POSITIONAL */
	ANALOG_INPUT(positional_vertical, ITEM_ID_YAXIS), /* IPT_POSITIONAL_V */
	ANALOG_INPUT(spinner, ITEM_ID_RXAXIS), /* IPT_DIAL */
	ANALOG_INPUT(spinner_vertical, ITEM_ID_RYAXIS), /* IPT_DIAL_V */
	ANALOG_INPUT(trackball_horizontal, ITEM_ID_RXAXIS), /* IPT_TRACKBALL_X */
	ANALOG_INPUT(trackball_vertical, ITEM_ID_RYAXIS), /* IPT_TRACKBALL_Y */
	ANALOG_INPUT(mouse_x, ITEM_ID_XAXIS), /* IPT_MOUSE_X */
	ANALOG_INPUT(mouse_y, ITEM_ID_YAXIS), /* IPT_MOUSE_Y */
	INVALID_INPUT, /* IPT_ADJUSTER */
	BUTTON_INPUT(Ui, Configure), /* IPT_UI_CONFIGURE */
	BUTTON_INPUT(Ui, On_Screen_Display), /* IPT_UI_ON_SCREEN_DISPLAY */
	BUTTON_INPUT(Ui, Debug_Break), /* IPT_UI_DEBUG_BREAK */
	BUTTON_INPUT(Ui, Pause), /* IPT_UI_PAUSE */
	BUTTON_INPUT(Ui, Reset_Machine), /* IPT_UI_RESET_MACHINE */
	BUTTON_INPUT(Ui, Soft_Reset), /* IPT_UI_SOFT_RESET */
	BUTTON_INPUT(Ui, Show_Gfx), /* IPT_UI_SHOW_GFX */
	BUTTON_INPUT(Ui, Frameskip_Dec), /* IPT_UI_FRAMESKIP_DEC */
	BUTTON_INPUT(Ui, Frameskip_Inc), /* IPT_UI_FRAMESKIP_INC */
	BUTTON_INPUT(Ui, Throttle), /* IPT_UI_THROTTLE */
	BUTTON_INPUT(Ui, Fast_Forward), /* IPT_UI_FAST_FORWARD */
	BUTTON_INPUT(Ui, Show_Fps), /* IPT_UI_SHOW_FPS */
	BUTTON_INPUT(Ui, Snapshot), /* IPT_UI_SNAPSHOT */
	BUTTON_INPUT(Ui, Record_Movie), /* IPT_UI_RECORD_MOVIE */
	BUTTON_INPUT(Ui, Toggle_Cheat), /* IPT_UI_TOGGLE_CHEAT */
	BUTTON_INPUT(Ui, Up), /* IPT_UI_UP */
	BUTTON_INPUT(Ui, Down), /* IPT_UI_DOWN */
	BUTTON_INPUT(Ui, Left), /* IPT_UI_LEFT */
	BUTTON_INPUT(Ui, Right), /* IPT_UI_RIGHT */
	BUTTON_INPUT(Ui, Home), /* IPT_UI_HOME */
	BUTTON_INPUT(Ui, End), /* IPT_UI_END */
	BUTTON_INPUT(Ui, Page_Up), /* IPT_UI_PAGE_UP */
	BUTTON_INPUT(Ui, Page_Down), /* IPT_UI_PAGE_DOWN */
	BUTTON_INPUT(Ui, Select), /* IPT_UI_SELECT */
	BUTTON_INPUT(Ui, Cancel), /* IPT_UI_CANCEL */
	BUTTON_INPUT(Ui, Display_Comment), /* IPT_UI_DISPLAY_COMMENT */
	BUTTON_INPUT(Ui, Clear), /* IPT_UI_CLEAR */
	BUTTON_INPUT(Ui, Zoom_In), /* IPT_UI_ZOOM_IN */
	BUTTON_INPUT(Ui, Zoom_Out), /* IPT_UI_ZOOM_OUT */
	BUTTON_INPUT(Ui, Prev_Group), /* IPT_UI_PREV_GROUP */
	BUTTON_INPUT(Ui, Next_Group), /* IPT_UI_NEXT_GROUP */
	BUTTON_INPUT(Ui, Rotate), /* IPT_UI_ROTATE */
	BUTTON_INPUT(Ui, Show_Profiler), /* IPT_UI_SHOW_PROFILER */
	BUTTON_INPUT(Ui, Toggle_Ui), /* IPT_UI_TOGGLE_UI */
	BUTTON_INPUT(Ui, Toggle_Debug), /* IPT_UI_TOGGLE_DEBUG */
	BUTTON_INPUT(Ui, Paste), /* IPT_UI_PASTE */
	BUTTON_INPUT(Ui, Save_State), /* IPT_UI_SAVE_STATE */
	BUTTON_INPUT(Ui, Load_State) /* IPT_UI_LOAD_STATE */
};
            
static int g_input_descriptor_count = 
    (sizeof(g_input_descriptors) / sizeof(g_input_descriptors[0]));

/**
 * This global is unfortunate but necessary.  If MAME is enhanced to
 * support callback data in its callbacks, then it will be unnecessary.
 **/
static LibMame_RunGame_State g_state;


/** **************************************************************************
 * Static helper functions
 ************************************************************************** **/

/**
 * This is the callback we hook up to the input device that MAME uses
 * to be called back to get the state of a controller input.  We also
 * arrange that the data passed in includes enough information to identify
 * what bit of state is being asked about; this one function handles all of
 * the input for all controller types.
 **/
static INT32 get_controller_state(void *, void *data)
{
    int player = CBDATA_PLAYER(data);
    int ipt_type = CBDATA_IPT(data);

    if (ipt_type >= g_input_descriptor_count) {
        /* This is weird, we're being asked for something bogus */
        return 0;
    }
    libmame_input_type input_type = g_input_descriptors[ipt_type].type;
    int input_number = g_input_descriptors[ipt_type].number;

    /* Just in case we need these */
    LibMame_PerPlayerControllersState *perplayer_state =
        &(g_state.controllers_state.per_player[player]);
    LibMame_SharedControllersState *shared_state =
        &(g_state.controllers_state.shared);

    switch (input_type) {
    case libmame_input_type_invalid:
        /* This is an input type that we don't handle (yet) */
        return 0;
    case libmame_input_type_Normal_button:
        return (perplayer_state->normal_buttons_state & (1 << input_number));
    case libmame_input_type_Mahjong_button:
        return (perplayer_state->mahjong_buttons_state & (1 << input_number));
    case libmame_input_type_Hanafuda_button:
        return (perplayer_state->hanafuda_buttons_state & (1 << input_number));
    case libmame_input_type_Gambling_button:
        return (perplayer_state->gambling_buttons_state & (1 << input_number));
    case libmame_input_type_Other_button:
        return (shared_state->other_buttons_state & (1 << input_number));
    case libmame_input_type_left_or_single_joystick_left:
        return perplayer_state->left_or_single_joystick_left_state;
    case libmame_input_type_left_or_single_joystick_right:
        return perplayer_state->left_or_single_joystick_right_state;
    case libmame_input_type_left_or_single_joystick_up:
        return perplayer_state->left_or_single_joystick_up_state;
    case libmame_input_type_left_or_single_joystick_down:
        return perplayer_state->left_or_single_joystick_down_state;
    case libmame_input_type_right_joystick_left:
        return perplayer_state->right_joystick_down_state;
    case libmame_input_type_right_joystick_right:
        return perplayer_state->right_joystick_down_state;
    case libmame_input_type_right_joystick_up:
        return perplayer_state->right_joystick_down_state;
    case libmame_input_type_right_joystick_down:
        return perplayer_state->right_joystick_down_state;
    case libmame_input_type_analog_joystick_horizontal:
        return perplayer_state->analog_joystick_horizontal_state;
    case libmame_input_type_analog_joystick_vertical:
        return perplayer_state->analog_joystick_vertical_state;
    case libmame_input_type_analog_joystick_altitude:
        return perplayer_state->analog_joystick_altitude_state;
    case libmame_input_type_spinner:
        return perplayer_state->spinner_delta;
    case libmame_input_type_spinner_vertical:
        return perplayer_state->spinner_vertical_delta;
    case libmame_input_type_paddle:
        return perplayer_state->paddle_state;
    case libmame_input_type_paddle_vertical:
        return perplayer_state->paddle_vertical_state;
    case libmame_input_type_trackball_horizontal:
        return perplayer_state->trackball_horizontal_delta;
    case libmame_input_type_trackball_vertical:
        return perplayer_state->trackball_vertical_delta;
    case libmame_input_type_lightgun_horizontal:
        return perplayer_state->lightgun_horizontal_state;
    case libmame_input_type_lightgun_vertical:
        return perplayer_state->lightgun_vertical_state;
    case libmame_input_type_pedal:
        return perplayer_state->pedal_state;
    case libmame_input_type_pedal2:
        return perplayer_state->pedal2_state;
    case libmame_input_type_pedal3:
        return perplayer_state->pedal3_state;
    case libmame_input_type_positional:
        return perplayer_state->positional_state;
    case libmame_input_type_positional_vertical:
        return perplayer_state->positional_vertical_state;
    case libmame_input_type_mouse_x:
        return perplayer_state->mouse_x_state;
    case libmame_input_type_mouse_y:
        return perplayer_state->mouse_y_state;
    case libmame_input_type_Ui_button:
        return (shared_state->ui_input_state == input_number);
    }

    /* Weird, this is not an input type that we know about */
    return 0;
}


static bool has_left_joystick_except(int controller_flags, int exception)
{
    if (controller_flags & (1 << exception)) {
        return false;
    }

    return (controller_flags &
            ((1 << LibMame_ControllerType_JoystickHorizontal) |
             (1 << LibMame_ControllerType_JoystickVertical) |
             (1 << LibMame_ControllerType_Joystick4Way) |
             (1 << LibMame_ControllerType_Joystick8Way) |
             (1 << LibMame_ControllerType_DoubleJoystickHorizontal) |
             (1 << LibMame_ControllerType_DoubleJoystickVertical) |
             (1 << LibMame_ControllerType_DoubleJoystick4Way) |
             (1 << LibMame_ControllerType_DoubleJoystick8Way)));
}


static bool has_right_joystick_except(int controller_flags, int exception)
{
    if (controller_flags & (1 << exception)) {
        return false;
    }

    return (controller_flags &
            ((1 << LibMame_ControllerType_DoubleJoystickHorizontal) |
             (1 << LibMame_ControllerType_DoubleJoystickVertical) |
             (1 << LibMame_ControllerType_DoubleJoystick4Way) |
             (1 << LibMame_ControllerType_DoubleJoystick8Way)));
}


static bool controllers_have_input
    (int num_players, const LibMame_AllControllersDescriptor *controllers,
     int player, int ipt_type)
{
    /* Game doesn't support this player, no need for this input */
    if (player >= num_players) {
        return false;
    }

    /* If it's not an IPT_TYPE that we have in our table, then no need for
       this input as we can't handle it */
    if (ipt_type >= g_input_descriptor_count) {
        return false;
    }
    
    libmame_input_type input_type = g_input_descriptors[ipt_type].type;
    int input_number = g_input_descriptors[ipt_type].number;

    switch (input_type) {
    case libmame_input_type_invalid:
        return false;
    case libmame_input_type_Normal_button:
        return (controllers->per_player.normal_button_flags & 
                (1 << input_number));
    case libmame_input_type_Mahjong_button:
        return (controllers->per_player.mahjong_button_flags &
                (1 << input_number));
    case libmame_input_type_Hanafuda_button:
        return (controllers->per_player.hanafuda_button_flags &
                (1 << input_number));
    case libmame_input_type_Gambling_button:
        return (controllers->per_player.gambling_button_flags &
                (1 << input_number));
    case libmame_input_type_Other_button:
        return (controllers->shared.other_button_flags & (1 << input_number));
    case libmame_input_type_left_or_single_joystick_left:
    case libmame_input_type_left_or_single_joystick_right:
        return has_left_joystick_except
            (controllers->per_player.controller_flags,
             LibMame_ControllerType_JoystickVertical);
    case libmame_input_type_left_or_single_joystick_up:
    case libmame_input_type_left_or_single_joystick_down:
        return has_left_joystick_except
            (controllers->per_player.controller_flags,
             LibMame_ControllerType_JoystickHorizontal);
    case libmame_input_type_right_joystick_left:
    case libmame_input_type_right_joystick_right:
        return has_right_joystick_except
            (controllers->per_player.controller_flags,
             LibMame_ControllerType_DoubleJoystickVertical);
    case libmame_input_type_right_joystick_up:
    case libmame_input_type_right_joystick_down:
        return has_right_joystick_except
            (controllers->per_player.controller_flags,
             LibMame_ControllerType_DoubleJoystickHorizontal);
    case libmame_input_type_analog_joystick_horizontal:
    case libmame_input_type_analog_joystick_vertical:
    case libmame_input_type_analog_joystick_altitude:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_JoystickAnalog));
    case libmame_input_type_spinner:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Spinner));
    case libmame_input_type_spinner_vertical:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_SpinnerVertical));
    case libmame_input_type_paddle:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Paddle));
    case libmame_input_type_paddle_vertical:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_PaddleVertical));
    case libmame_input_type_trackball_horizontal:
    case libmame_input_type_trackball_vertical:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Trackball));
    case libmame_input_type_lightgun_horizontal:
    case libmame_input_type_lightgun_vertical:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Lightgun));
    case libmame_input_type_pedal:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Pedal));
    case libmame_input_type_pedal2:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Pedal2));
    case libmame_input_type_pedal3:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Pedal3));
    case libmame_input_type_positional:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Positional));
    case libmame_input_type_positional_vertical:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_PositionalVertical));
    case libmame_input_type_mouse_x:
    case libmame_input_type_mouse_y:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Mouse));
    case libmame_input_type_Ui_button:
        return true;
    }

    return false;
}


/** **************************************************************************
 * Static MAME OSD function implementations
 ************************************************************************** **/

void osd_init(running_machine *machine)
{
    /**
     *  Save away the machine, we'll need it in osd_customize_input_type_list
     **/
    g_state.machine = machine;

    /**
     * Create the render_target that tells MAME the rendering parameters it
     * will use.
     **/
    g_state.target = render_target_alloc(g_state.machine, NULL, 0);

    /* Set it up to be the same size as the game's original display, if it's
       a raster display; then any stretching to the actual display hardware
       will be done by the update callback. */
    if (LibMame_Get_Game_ScreenType(g_state.gamenum) != 
        LibMame_ScreenType_Vector) {
        LibMame_ScreenResolution res = LibMame_Get_Game_ScreenResolution
            (g_state.gamenum);
        render_target_set_bounds(g_state.target, res.width, res.height, 0.0);
    }
}


void osd_update(running_machine *machine, int skip_redraw)
{
    /**
     * Poll input
     **/
    memset(g_state.controllers_state.per_player, 0, 
           sizeof(LibMame_PerPlayerControllersState) * 
           g_state.maximum_player_count);
    g_state.controllers_state.shared.other_buttons_state = 0;
    g_state.controllers_state.shared.ui_input_state = 0;

    (*(g_state.callbacks->PollAllControllersState))
        (&(g_state.controllers_state), g_state.callback_data);

    /**
     * Ask the callbacks to update the video.  For now, assume that there
     * is only one display.  Might want to support multiple displays in the
     * future.
     **/
    const render_primitive_list *list = 
        render_target_get_primitives(g_state.target);
    osd_lock_acquire(list->lock);
    (*(g_state.callbacks->UpdateVideo))((LibMame_RenderPrimitive *) list->head,
                                        g_state.callback_data);
    osd_lock_release(list->lock);

    /**
     * Give the callbacks a chance to make running game calls
     **/
    (*(g_state.callbacks->MakeRunningGameCalls))(g_state.callback_data);
}


void osd_update_audio_stream(running_machine *machine, INT16 *buffer,
                             int samples_this_frame)
{
    /**
     * Ask the callbacks to update the audio
     **/
    (*(g_state.callbacks->UpdateAudio))(buffer, samples_this_frame,
                                        g_state.callback_data);
}


void osd_set_mastervolume(int attenuation)
{
    (*(g_state.callbacks->SetMasterVolume))(attenuation, g_state.callback_data);
}


/**
 * This function is called back by MAME when it is starting up.  It passes the
 * descriptor that it uses internally for every possible input port, and gives
 * us the opportunity to define what callback we will provide for fetching the
 * state of that input port.  We ignore those input ports that are not used in
 * the current game, and for the others, make our own custom controllers as
 * neceessary to provide the inputs, and hook the input callbacks up to fetch
 * the input state from the one single LibMame_AllControllersState structure
 * that we store in the g_state object.  In this way, we completely hardware
 * the way that MAME handles input so that the users of libmame can do
 * whatever they want to satisfy getting inputs for the various controllers
 * types.
 **/
void osd_customize_input_type_list(input_type_desc *typelist)
{
    /**
     * For each input descriptor, create a keyboard key, or mouse axis, or
     * lightgun control, or joystick axis for it, and change its input
     * sequence to match the new item.  Also, set up the polling callback for
     * it to have the data it needs to get the right controller state.
     **/
	input_type_desc *typedesc;
    input_device *item_device;
    input_item_id item_id = ITEM_ID_INVALID;
    int input_code = 0;

    /**
     * New keyboards are created as we run out of keys; the only keys we
     * use are between ITEM_ID_A and ITEM_ID_CANCEL (inclusive).  It is
     * important that ITEM_IDs that identify keys from a keyboard are used as
     * MAME internally detects the ITEM_ID type and treats it specially in
     * different circumstances, and so the ITEM_ID must "match" the
     * input_device type that it is attached to.  For example, if we used
     * something outside of the key range for a keyboard
     * (e.g. ITEM_ID_RELATIVE1), then MAME would interpret the INT32 that it
     * gets when it calls back to get the state of the input incorrectly (it
     * would interpret it as some kind of delta value, not as a "pressed or
     * not pressed" boolean).  The same concept holds true for joysticks,
     * mice, etc, so those only use ITEM_ID values that make sense for their
     * types.
     **/
    int keyboard_index = -1;
    input_item_id keyboard_item = ITEM_ID_A;
    int keyboard_count = 0;
    /* As to all of the rest, they are created as needed, one for each player */
    int analog_joystick_index[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    int spinner_index[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    int paddle_index[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }; 
    int trackball_index[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    int lightgun_index[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    int pedal_index[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    int positional_index[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    int mouse_index[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    char namebuf[256];


#define GET_ITEM_DEVICE_AND_ID(device_type, device_class, item_class)   \
    do {                                                                \
        if (device_type##_index [typedesc->player] == -1) {             \
            snprintf(namebuf, sizeof(namebuf),                          \
                     "libmame_virtual_" #device_type "_%d",             \
                     typedesc->player);                                 \
            item_device = input_device_add                              \
                (g_state.machine, device_class, namebuf, NULL);         \
            device_type##_index [typedesc->player] =                    \
                input_device_get_index(g_state.machine, item_device);   \
        }                                                               \
        item_device = input_device_get_by_index                         \
            (g_state.machine,                                           \
             device_type##_index [typedesc->player],                    \
             device_class);                                             \
        item_id = g_input_descriptors[typedesc->type].item_id;          \
        input_code =                                                    \
            INPUT_CODE(device_class,                                    \
                       device_type##_index [typedesc->player],          \
                       item_class, ITEM_MODIFIER_NONE, item_id);        \
    } while (0);


	for (typedesc = typelist; typedesc; typedesc = typedesc->next) {
        item_device = NULL;
        if (controllers_have_input(g_state.maximum_player_count,
                                   &(g_state.controllers),
                                   typedesc->player, typedesc->type)) {
            switch (g_input_descriptors[typedesc->type].type) {
            case libmame_input_type_invalid:
                break;

            case libmame_input_type_Normal_button:
            case libmame_input_type_Mahjong_button:
            case libmame_input_type_Hanafuda_button:
            case libmame_input_type_Gambling_button:
            case libmame_input_type_Other_button:
            case libmame_input_type_left_or_single_joystick_left:
            case libmame_input_type_left_or_single_joystick_right:
            case libmame_input_type_left_or_single_joystick_up:
            case libmame_input_type_left_or_single_joystick_down:
            case libmame_input_type_right_joystick_left:
            case libmame_input_type_right_joystick_right:
            case libmame_input_type_right_joystick_up:
            case libmame_input_type_right_joystick_down:
            case libmame_input_type_Ui_button:
                if ((keyboard_index == -1) || 
                    (keyboard_item > ITEM_ID_CANCEL)) {
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_keyboard_%d", keyboard_count++);
                    item_device = input_device_add
                        (g_state.machine, DEVICE_CLASS_KEYBOARD, 
                         namebuf, NULL);
                    keyboard_index = input_device_get_index
                        (g_state.machine, item_device);
                    keyboard_item = ITEM_ID_A;
                }
                item_device = input_device_get_by_index
                    (g_state.machine, keyboard_index, DEVICE_CLASS_KEYBOARD);
                item_id = keyboard_item;
                keyboard_item++;
                input_code = INPUT_CODE(DEVICE_CLASS_KEYBOARD, keyboard_index,
                                        ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE,
                                        item_id);
                break;

            case libmame_input_type_analog_joystick_horizontal:
            case libmame_input_type_analog_joystick_vertical:
            case libmame_input_type_analog_joystick_altitude:
                GET_ITEM_DEVICE_AND_ID(analog_joystick, DEVICE_CLASS_JOYSTICK,
                                       ITEM_CLASS_ABSOLUTE);
                break;

            case libmame_input_type_spinner:
            case libmame_input_type_spinner_vertical:
                GET_ITEM_DEVICE_AND_ID(spinner, DEVICE_CLASS_MOUSE, 
                                       ITEM_CLASS_RELATIVE);
                break;

            case libmame_input_type_paddle:
            case libmame_input_type_paddle_vertical:
                GET_ITEM_DEVICE_AND_ID(paddle, DEVICE_CLASS_MOUSE,
                                       ITEM_CLASS_ABSOLUTE);
                break;

            case libmame_input_type_trackball_horizontal:
            case libmame_input_type_trackball_vertical:
                GET_ITEM_DEVICE_AND_ID(trackball, DEVICE_CLASS_MOUSE,
                                       ITEM_CLASS_RELATIVE);
                break;

            case libmame_input_type_lightgun_horizontal:
            case libmame_input_type_lightgun_vertical:
                GET_ITEM_DEVICE_AND_ID(lightgun, DEVICE_CLASS_LIGHTGUN,
                                       ITEM_CLASS_ABSOLUTE);
                break;

            case libmame_input_type_pedal:
            case libmame_input_type_pedal2:
            case libmame_input_type_pedal3:
                GET_ITEM_DEVICE_AND_ID(pedal, DEVICE_CLASS_MOUSE,
                                       ITEM_CLASS_ABSOLUTE);
                break;

            case libmame_input_type_positional:
            case libmame_input_type_positional_vertical:
                GET_ITEM_DEVICE_AND_ID(positional, DEVICE_CLASS_MOUSE,
                                       ITEM_CLASS_ABSOLUTE);
                break;

            case libmame_input_type_mouse_x:
            case libmame_input_type_mouse_y:
                GET_ITEM_DEVICE_AND_ID(mouse, DEVICE_CLASS_MOUSE,
                                       ITEM_CLASS_RELATIVE);
                break;
            }
        }

        if (item_device) {
            input_device_item_add
                (item_device, "", 
                 (void *) CBDATA_MAKE(typedesc->player, typedesc->type), 
                 item_id, &get_controller_state);
        }
        else {
            /* For some reason or another, we can't handle this input, so turn
               it off by setting it to an item_id that we never use */
            item_id = ITEM_ID_MAXIMUM;
        }

        input_seq_set_1(&(typedesc->seq[SEQ_TYPE_STANDARD]), input_code);
    }
}


/** **************************************************************************
 * LibMame exported function implementations
 ************************************************************************** **/

LibMame_RunGameStatus LibMame_RunGame(int gamenum,
                                      const LibMame_RunGameOptions *options,
                                      const LibMame_RunGameCallbacks *cbs,
                                      void *callback_data)
{
    if (gamenum >= LibMame_Get_Game_Count()) {
        return LibMame_RunGameStatus_InvalidGameNum;
    }

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
    g_state.controllers = LibMame_Get_Game_AllControllers(gamenum);

    /* Set up options stuff for MAME.  If none were supplied, use the
       defaults. */
    core_options *mame_options;
    if (options == NULL) {
        LibMame_RunGameOptions default_options;
        LibMame_Set_Default_RunGameOptions(&default_options);
        mame_options = get_mame_options
            (&default_options, LibMame_Get_Game_Short_Name(gamenum));
    }
    else {
        mame_options = get_mame_options
            (options, LibMame_Get_Game_Short_Name(gamenum));
    }

    /* Run the game */
    int result = mame_execute(mame_options);

    /* Convert the resulting MAME code to a libmame code and return */
    switch (result) {
    case MAMERR_NONE:
        return LibMame_RunGameStatus_Success;
    case MAMERR_FAILED_VALIDITY:
        return LibMame_RunGameStatus_FailedValidtyCheck;
    case MAMERR_MISSING_FILES:
        return LibMame_RunGameStatus_MissingFiles;
    case MAMERR_FATALERROR:
    case MAMERR_DEVICE:
    case MAMERR_IDENT_NONROMS:
    case MAMERR_IDENT_PARTIAL:
    case MAMERR_IDENT_NONE:
        return LibMame_RunGameStatus_GeneralError;
    case MAMERR_NO_SUCH_GAME:
        return LibMame_RunGameStatus_NoSuchGame;
    case MAMERR_INVALID_CONFIG:
        return LibMame_RunGameStatus_InvalidConfig;
    }

    return LibMame_RunGameStatus_GeneralError;
}
