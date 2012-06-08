/** **************************************************************************
 * libmame_rungame.c
 *
 * LibMame RunGame implementation.
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include <stddef.h>
#include <string.h>
#include "emu.h"
#include "emuopts.h"
#include "libmame.h"
#include "options.h"
#include "osdcore.h"
#include "osdepend.h"
#include "render.h"
#include "video.h"


/** **************************************************************************
 * External symbol references
 ************************************************************************** **/

/**
 * These are exported by other source files within libmame itself
 **/
extern void get_mame_options(const LibMame_RunGameOptions *options,
                             const char *gamename,
                             emu_options &mame_options);


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
    libmame_input_type_Shared_button,
    libmame_input_type_left_joystick,
    libmame_input_type_right_joystick,
    libmame_input_type_analog_joystick_horizontal,
    libmame_input_type_analog_joystick_vertical,
    libmame_input_type_spinner,
    libmame_input_type_vertical_spinner,
    libmame_input_type_paddle,
    libmame_input_type_vertical_paddle,
    libmame_input_type_trackball_horizontal,
    libmame_input_type_trackball_vertical,
    libmame_input_type_lightgun_horizontal,
    libmame_input_type_lightgun_vertical,
    libmame_input_type_pedal,
    libmame_input_type_pedal2,
    libmame_input_type_pedal3,
    libmame_input_type_Ui_button,
} libmame_input_type;


/** **************************************************************************
 * Static prototypes
 ************************************************************************** **/
/* libmame OSD prototypes */
static void osd_init(running_machine *machine);
static void osd_update(running_machine *machine, int skip_redraw);
static void osd_update_audio_stream(running_machine *machine, 
                                    const INT16 *buffer,
                                    int samples_this_frame);
static void osd_set_mastervolume(int attenuation);
static void osd_customize_input_type_list(simple_list<input_type_entry> &typelist);


/** **************************************************************************
 * Classes
 ************************************************************************** **/

class libmame_rungame_osd_interface : public osd_interface
{
public:

	libmame_rungame_osd_interface()
    {
    }

	virtual ~libmame_rungame_osd_interface()
    {
    }

	virtual void init(running_machine &machine)
    {
        this->osd_interface::init(machine);

        return osd_init(&machine);
    }

	virtual void update(bool skip_redraw)
    {
        return osd_update(&(this->machine()), skip_redraw);
    }

	virtual void update_audio_stream(const INT16 *buffer, 
                                     int samples_this_frame)
    {
        return osd_update_audio_stream(&(this->machine()), buffer, 
                                       samples_this_frame);
    }

	virtual void set_mastervolume(int attenuation)
    {
        return osd_set_mastervolume(attenuation);
    }

	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist)
    {
        return osd_customize_input_type_list(typelist);
    }
};


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
    int ipt_port;
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
    LibMame_AllControllers controllers;

    /**
     * Have the special inputs been configured yet?
     */
    bool special_inputs_configured;

    /**
     * Has a LibMame_RunningGame_SchedulePause() call been made?
     **/
    bool waiting_for_pause;

    /**
     * This is the controllers state used to query the controllers state via
     * the callback provided in the callbacks structure.
     **/
    LibMame_AllControlsState controls_state;

    /**
     * This is the running machine that this state is associated with
     **/
    running_machine *machine;

    /**
     * This is the render target for the game
     **/
    render_target *target;

    /**
     * Most recently requested 'speed text'
     **/
    astring speed_text;

    // More MAME incredible C++ lameness.  Seriously MAME's C++ is the worst
    // C++ I have ever seen.  Totally unmaintainable.
    void output_callback(const char *format, va_list args)
    {
        (*(callbacks->StatusText))(format, args, callback_data);
    }

} LibMame_RunGame_State;


/** **************************************************************************
 * Helper Macros
 ************************************************************************** **/

/**
 * This is a packed representation of everything necessary to identify the
 * controller that MAME is asking about.
 **/
#define CBDATA_PLAYER_MASK           0x000F
#define CBDATA_IPT_MASK              0xFFF0
#define CBDATA_PLAYER(d)             (((uintptr_t) d) & CBDATA_PLAYER_MASK)
#define CBDATA_IPT(d)                ((((uintptr_t) d) & CBDATA_IPT_MASK) >> 8)
#define CBDATA_MAKE(player, ipt)     ((void *) (uintptr_t)                \
                                      (((player) & CBDATA_PLAYER_MASK) |  \
                                       ((ipt << 8) & CBDATA_IPT_MASK)))

/* These macros make the following table definition more conscise */
#define INVALID_INPUT \
    libmame_input_type_invalid, 0, ITEM_ID_INVALID
#define BUTTON_INPUT(button_type, button_name)                  \
      libmame_input_type_##button_type##_button,                \
      LibMame_##button_type##ButtonType_##button_name,          \
      ITEM_ID_INVALID
#define JOYSTICK_INPUT(joystick_type, direction)        \
      libmame_input_type_##joystick_type##_joystick,    \
      direction, ITEM_ID_INVALID
#define ANALOG_INPUT(input_type, input_item_id) \
    libmame_input_type_##input_type, 0, input_item_id


/** **************************************************************************
 * Static global variables
 ************************************************************************** **/

/* This maps each MAME IPT_ type to a libmame_input descriptor. */
static libmame_input_descriptor g_input_descriptors[IPT_COUNT] =
{
	{ IPT_INVALID, INVALID_INPUT },
	{ IPT_UNUSED, INVALID_INPUT },
	{ IPT_END, INVALID_INPUT },
	{ IPT_UNKNOWN, INVALID_INPUT },
	{ IPT_PORT, INVALID_INPUT },
	{ IPT_DIPSWITCH, INVALID_INPUT },
	{ IPT_CONFIG, INVALID_INPUT },
    { IPT_START1, BUTTON_INPUT(Shared, Start1) },
	{ IPT_START2, BUTTON_INPUT(Shared, Start2) },
	{ IPT_START3, BUTTON_INPUT(Shared, Start3) },
	{ IPT_START4, BUTTON_INPUT(Shared, Start4) },
	{ IPT_START5, BUTTON_INPUT(Shared, Start5) },
	{ IPT_START6, BUTTON_INPUT(Shared, Start6) },
	{ IPT_START7, BUTTON_INPUT(Shared, Start7) },
	{ IPT_START8, BUTTON_INPUT(Shared, Start8) },
	{ IPT_COIN1, BUTTON_INPUT(Shared, Coin1) },
	{ IPT_COIN2, BUTTON_INPUT(Shared, Coin2) },
	{ IPT_COIN3, BUTTON_INPUT(Shared, Coin3) },
	{ IPT_COIN4, BUTTON_INPUT(Shared, Coin4) },
	{ IPT_COIN5, BUTTON_INPUT(Shared, Coin5) },
	{ IPT_COIN6, BUTTON_INPUT(Shared, Coin6) },
	{ IPT_COIN7, BUTTON_INPUT(Shared, Coin7) },
	{ IPT_COIN8, BUTTON_INPUT(Shared, Coin8) },
    { IPT_COIN9, INVALID_INPUT },
    { IPT_COIN10, INVALID_INPUT },
    { IPT_COIN11, INVALID_INPUT },
    { IPT_COIN12, INVALID_INPUT },
	{ IPT_BILL1, BUTTON_INPUT(Shared, Bill1) },
	{ IPT_SERVICE1, BUTTON_INPUT(Shared, Service1) },
	{ IPT_SERVICE2, BUTTON_INPUT(Shared, Service2) },
	{ IPT_SERVICE3, BUTTON_INPUT(Shared, Service3) },
	{ IPT_SERVICE4, BUTTON_INPUT(Shared, Service4) },
    { IPT_TILT1, INVALID_INPUT },
    { IPT_TILT2, INVALID_INPUT },
    { IPT_TILT3, INVALID_INPUT },
    { IPT_TILT4, INVALID_INPUT },
	{ IPT_SERVICE, BUTTON_INPUT(Shared, Service) },
	{ IPT_TILT, BUTTON_INPUT(Shared, Tilt) },
	{ IPT_INTERLOCK, BUTTON_INPUT(Shared, Interlock) },
	{ IPT_VOLUME_UP, BUTTON_INPUT(Shared, Volume_Up) },
	{ IPT_VOLUME_DOWN, BUTTON_INPUT(Shared, Volume_Down) },
	{ IPT_START, INVALID_INPUT },
	{ IPT_SELECT, INVALID_INPUT },
    { IPT_KEYPAD, INVALID_INPUT },
	{ IPT_KEYBOARD, INVALID_INPUT },
    { IPT_DIGITAL_JOYSTICK_FIRST, INVALID_INPUT },
    { IPT_JOYSTICK_UP, JOYSTICK_INPUT(left, LibMame_JoystickDirection_Up) },
    { IPT_JOYSTICK_DOWN, JOYSTICK_INPUT(left, LibMame_JoystickDirection_Down) },
    { IPT_JOYSTICK_LEFT, JOYSTICK_INPUT(left, LibMame_JoystickDirection_Left) },
    { IPT_JOYSTICK_RIGHT, 
      JOYSTICK_INPUT(left, LibMame_JoystickDirection_Right) },
    { IPT_JOYSTICKRIGHT_UP,
      JOYSTICK_INPUT(right, LibMame_JoystickDirection_Up) },
    { IPT_JOYSTICKRIGHT_DOWN, 
      JOYSTICK_INPUT(right, LibMame_JoystickDirection_Down) },
    { IPT_JOYSTICKRIGHT_LEFT, 
      JOYSTICK_INPUT(right, LibMame_JoystickDirection_Left) },
    { IPT_JOYSTICKRIGHT_RIGHT, 
      JOYSTICK_INPUT(right, LibMame_JoystickDirection_Right) },
    { IPT_JOYSTICKLEFT_UP, 
      JOYSTICK_INPUT(left, LibMame_JoystickDirection_Up) },
    { IPT_JOYSTICKLEFT_DOWN, 
      JOYSTICK_INPUT(left, LibMame_JoystickDirection_Down) },
    { IPT_JOYSTICKLEFT_LEFT, 
      JOYSTICK_INPUT(left, LibMame_JoystickDirection_Left) },
    { IPT_JOYSTICKLEFT_RIGHT, 
      JOYSTICK_INPUT(left, LibMame_JoystickDirection_Right) },
    { IPT_DIGITAL_JOYSTICK_LAST, INVALID_INPUT },
	{ IPT_BUTTON1, BUTTON_INPUT(Normal, 1) },
	{ IPT_BUTTON2, BUTTON_INPUT(Normal, 2) },
	{ IPT_BUTTON3, BUTTON_INPUT(Normal, 3) },
	{ IPT_BUTTON4, BUTTON_INPUT(Normal, 4) },
	{ IPT_BUTTON5, BUTTON_INPUT(Normal, 5) },
	{ IPT_BUTTON6, BUTTON_INPUT(Normal, 6) },
	{ IPT_BUTTON7, BUTTON_INPUT(Normal, 7) },
	{ IPT_BUTTON8, BUTTON_INPUT(Normal, 8) },
	{ IPT_BUTTON9, BUTTON_INPUT(Normal, 9) },
	{ IPT_BUTTON10, BUTTON_INPUT(Normal, 10) },
	{ IPT_BUTTON11, BUTTON_INPUT(Normal, 11) },
	{ IPT_BUTTON12, BUTTON_INPUT(Normal, 12) },
	{ IPT_BUTTON13, BUTTON_INPUT(Normal, 13) },
	{ IPT_BUTTON14, BUTTON_INPUT(Normal, 14) },
	{ IPT_BUTTON15, BUTTON_INPUT(Normal, 15) },
	{ IPT_BUTTON16, BUTTON_INPUT(Normal, 16) },
    { IPT_MAHJONG_FIRST, INVALID_INPUT },
	{ IPT_MAHJONG_A, BUTTON_INPUT(Mahjong, A) },
	{ IPT_MAHJONG_B, BUTTON_INPUT(Mahjong, B) },
	{ IPT_MAHJONG_C, BUTTON_INPUT(Mahjong, C) },
	{ IPT_MAHJONG_D, BUTTON_INPUT(Mahjong, D) },
	{ IPT_MAHJONG_E, BUTTON_INPUT(Mahjong, E) },
	{ IPT_MAHJONG_F, BUTTON_INPUT(Mahjong, F) },
	{ IPT_MAHJONG_G, BUTTON_INPUT(Mahjong, G) },
	{ IPT_MAHJONG_H, BUTTON_INPUT(Mahjong, H) },
	{ IPT_MAHJONG_I, BUTTON_INPUT(Mahjong, I) },
	{ IPT_MAHJONG_J, BUTTON_INPUT(Mahjong, J) },
	{ IPT_MAHJONG_K, BUTTON_INPUT(Mahjong, K) },
	{ IPT_MAHJONG_L, BUTTON_INPUT(Mahjong, L) },
	{ IPT_MAHJONG_M, BUTTON_INPUT(Mahjong, M) },
	{ IPT_MAHJONG_N, BUTTON_INPUT(Mahjong, N) },
	{ IPT_MAHJONG_O, BUTTON_INPUT(Mahjong, O) },
	{ IPT_MAHJONG_P, BUTTON_INPUT(Mahjong, P) },
	{ IPT_MAHJONG_Q, BUTTON_INPUT(Mahjong, Q) },
	{ IPT_MAHJONG_KAN, BUTTON_INPUT(Mahjong, Kan) },
	{ IPT_MAHJONG_PON, BUTTON_INPUT(Mahjong, Pon) },
	{ IPT_MAHJONG_CHI, BUTTON_INPUT(Mahjong, Chi) },
	{ IPT_MAHJONG_REACH, BUTTON_INPUT(Mahjong, Reach) },
	{ IPT_MAHJONG_RON, BUTTON_INPUT(Mahjong, Ron) },
	{ IPT_MAHJONG_BET, BUTTON_INPUT(Mahjong, Bet) },
	{ IPT_MAHJONG_LAST_CHANCE, BUTTON_INPUT(Mahjong, Last_Chance) },
	{ IPT_MAHJONG_SCORE, BUTTON_INPUT(Mahjong, Score) },
	{ IPT_MAHJONG_DOUBLE_UP, BUTTON_INPUT(Mahjong, Double_Up) },
	{ IPT_MAHJONG_FLIP_FLOP, BUTTON_INPUT(Mahjong, Flip_Flop) },
	{ IPT_MAHJONG_BIG, BUTTON_INPUT(Mahjong, Big) },
	{ IPT_MAHJONG_SMALL, BUTTON_INPUT(Mahjong, Small) },
    { IPT_MAHJONG_LAST, INVALID_INPUT },
    { IPT_HANAFUDA_FIRST, INVALID_INPUT },
	{ IPT_HANAFUDA_A, BUTTON_INPUT(Hanafuda, A) },
	{ IPT_HANAFUDA_B, BUTTON_INPUT(Hanafuda, B) },
	{ IPT_HANAFUDA_C, BUTTON_INPUT(Hanafuda, C) },
	{ IPT_HANAFUDA_D, BUTTON_INPUT(Hanafuda, D) },
	{ IPT_HANAFUDA_E, BUTTON_INPUT(Hanafuda, E) },
	{ IPT_HANAFUDA_F, BUTTON_INPUT(Hanafuda, F) },
	{ IPT_HANAFUDA_G, BUTTON_INPUT(Hanafuda, G) },
	{ IPT_HANAFUDA_H, BUTTON_INPUT(Hanafuda, H) },
	{ IPT_HANAFUDA_YES, BUTTON_INPUT(Hanafuda, Yes) },
	{ IPT_HANAFUDA_NO, BUTTON_INPUT(Hanafuda, No) },
    { IPT_HANAFUDA_LAST, INVALID_INPUT },
    { IPT_GAMBLING_FIRST, INVALID_INPUT },
	{ IPT_GAMBLE_KEYIN, BUTTON_INPUT(Gambling, Keyin) },
	{ IPT_GAMBLE_KEYOUT, BUTTON_INPUT(Gambling, Keyout) },
	{ IPT_GAMBLE_SERVICE, BUTTON_INPUT(Gambling, Service) },
	{ IPT_GAMBLE_BOOK, BUTTON_INPUT(Gambling, Book) },
	{ IPT_GAMBLE_DOOR, BUTTON_INPUT(Gambling, Door) },
	{ IPT_GAMBLE_HIGH, BUTTON_INPUT(Gambling, High) },
	{ IPT_GAMBLE_LOW, BUTTON_INPUT(Gambling, Low) },
	{ IPT_GAMBLE_HALF, BUTTON_INPUT(Gambling, Half) },
	{ IPT_GAMBLE_DEAL, BUTTON_INPUT(Gambling, Deal) },
	{ IPT_GAMBLE_D_UP, BUTTON_INPUT(Gambling, D_Up) },
	{ IPT_GAMBLE_TAKE, BUTTON_INPUT(Gambling, Take) },
	{ IPT_GAMBLE_STAND, BUTTON_INPUT(Gambling, Stand) },
	{ IPT_GAMBLE_BET, BUTTON_INPUT(Gambling, Bet) },
	{ IPT_GAMBLE_PAYOUT, BUTTON_INPUT(Gambling, Payout) },
	{ IPT_POKER_HOLD1, BUTTON_INPUT(Gambling, Hold1) },
	{ IPT_POKER_HOLD2, BUTTON_INPUT(Gambling, Hold2) },
	{ IPT_POKER_HOLD3, BUTTON_INPUT(Gambling, Hold3) },
	{ IPT_POKER_HOLD4, BUTTON_INPUT(Gambling, Hold4) },
	{ IPT_POKER_HOLD5, BUTTON_INPUT(Gambling, Hold5) },
	{ IPT_POKER_CANCEL, BUTTON_INPUT(Gambling, Cancel) },
	{ IPT_POKER_BET, BUTTON_INPUT(Gambling, Bet) },
	{ IPT_SLOT_STOP1, BUTTON_INPUT(Gambling, Stop1) },
	{ IPT_SLOT_STOP2, BUTTON_INPUT(Gambling, Stop2) },
	{ IPT_SLOT_STOP3, BUTTON_INPUT(Gambling, Stop3) },
	{ IPT_SLOT_STOP4, BUTTON_INPUT(Gambling, Stop4) },
	{ IPT_SLOT_STOP_ALL, BUTTON_INPUT(Gambling, Stop_All) },
    { IPT_GAMBLING_LAST, INVALID_INPUT },
    { IPT_ANALOG_FIRST, INVALID_INPUT },
    { IPT_ANALOG_ABSOLUTE_FIRST, INVALID_INPUT },

    { IPT_AD_STICK_X, ANALOG_INPUT(analog_joystick_horizontal, ITEM_ID_XAXIS) },
	{ IPT_AD_STICK_Y, ANALOG_INPUT(analog_joystick_vertical, ITEM_ID_YAXIS) },
    // IPT_AD_STICK_Z is only ever used as a throttle input when there
    // isn't otherwise a pedal
    { IPT_AD_STICK_Z, ANALOG_INPUT(pedal, ITEM_ID_ZAXIS) },
	{ IPT_PADDLE, ANALOG_INPUT(paddle, ITEM_ID_XAXIS) },
	{ IPT_PADDLE_V, ANALOG_INPUT(vertical_paddle, ITEM_ID_YAXIS) },
    { IPT_PEDAL, ANALOG_INPUT(pedal, ITEM_ID_XAXIS) },
    { IPT_PEDAL2, ANALOG_INPUT(pedal2, ITEM_ID_YAXIS) },
    { IPT_PEDAL3, ANALOG_INPUT(pedal3, ITEM_ID_ZAXIS) },
    { IPT_LIGHTGUN_X, ANALOG_INPUT(lightgun_horizontal, ITEM_ID_XAXIS) },
    { IPT_LIGHTGUN_Y, ANALOG_INPUT(lightgun_vertical, ITEM_ID_YAXIS) },
    // Using paddles for positionals.  It is possible that some games have
    // both, which will suck.
    { IPT_POSITIONAL, ANALOG_INPUT(paddle, ITEM_ID_XAXIS) },
    { IPT_POSITIONAL_V, ANALOG_INPUT(vertical_paddle, ITEM_ID_YAXIS) },
    { IPT_ANALOG_ABSOLUTE_LAST, INVALID_INPUT },
    { IPT_DIAL, ANALOG_INPUT(spinner, ITEM_ID_RXAXIS) },
    { IPT_DIAL_V, ANALOG_INPUT(vertical_spinner, ITEM_ID_RYAXIS) },
    { IPT_TRACKBALL_X, ANALOG_INPUT(trackball_horizontal, ITEM_ID_RXAXIS) },
    { IPT_TRACKBALL_Y, ANALOG_INPUT(trackball_vertical, ITEM_ID_RYAXIS) },
    { IPT_MOUSE_X, INVALID_INPUT },
    { IPT_MOUSE_Y, INVALID_INPUT },
    { IPT_ANALOG_LAST, INVALID_INPUT },
    { IPT_ADJUSTER, INVALID_INPUT },
    { IPT_UI_FIRST, INVALID_INPUT },
    { IPT_UI_CONFIGURE, BUTTON_INPUT(Ui, Configure) },
    { IPT_UI_ON_SCREEN_DISPLAY, BUTTON_INPUT(Ui, On_Screen_Display) },
    { IPT_UI_DEBUG_BREAK, BUTTON_INPUT(Ui, Debug_Break) },
    { IPT_UI_PAUSE, BUTTON_INPUT(Ui, Pause) },
    { IPT_UI_RESET_MACHINE, BUTTON_INPUT(Ui, Reset_Machine) },
    { IPT_UI_SOFT_RESET, BUTTON_INPUT(Ui, Soft_Reset) },
	{ IPT_UI_SHOW_GFX, BUTTON_INPUT(Ui, Show_Gfx) },
	{ IPT_UI_FRAMESKIP_DEC, BUTTON_INPUT(Ui, Frameskip_Dec) },
	{ IPT_UI_FRAMESKIP_INC, BUTTON_INPUT(Ui, Frameskip_Inc) },
	{ IPT_UI_THROTTLE, BUTTON_INPUT(Ui, Throttle) },
	{ IPT_UI_FAST_FORWARD, BUTTON_INPUT(Ui, Fast_Forward) },
	{ IPT_UI_SHOW_FPS, BUTTON_INPUT(Ui, Show_Fps) },
	{ IPT_UI_SNAPSHOT, BUTTON_INPUT(Ui, Snapshot) },
	{ IPT_UI_RECORD_MOVIE, BUTTON_INPUT(Ui, Record_Movie) },
	{ IPT_UI_TOGGLE_CHEAT, BUTTON_INPUT(Ui, Toggle_Cheat) },
	{ IPT_UI_UP, BUTTON_INPUT(Ui, Up) },
	{ IPT_UI_DOWN, BUTTON_INPUT(Ui, Down) },
	{ IPT_UI_LEFT, BUTTON_INPUT(Ui, Left) },
	{ IPT_UI_RIGHT, BUTTON_INPUT(Ui, Right) },
	{ IPT_UI_HOME, BUTTON_INPUT(Ui, Home) },
	{ IPT_UI_END, BUTTON_INPUT(Ui, End) },
	{ IPT_UI_PAGE_UP, BUTTON_INPUT(Ui, Page_Up) },
	{ IPT_UI_PAGE_DOWN, BUTTON_INPUT(Ui, Page_Down) },
	{ IPT_UI_SELECT, BUTTON_INPUT(Ui, Select) },
	{ IPT_UI_CANCEL, BUTTON_INPUT(Ui, Cancel) },
	{ IPT_UI_DISPLAY_COMMENT, BUTTON_INPUT(Ui, Display_Comment) },
	{ IPT_UI_CLEAR, BUTTON_INPUT(Ui, Clear) },
	{ IPT_UI_ZOOM_IN, BUTTON_INPUT(Ui, Zoom_In) },
	{ IPT_UI_ZOOM_OUT, BUTTON_INPUT(Ui, Zoom_Out) },
	{ IPT_UI_PREV_GROUP, BUTTON_INPUT(Ui, Prev_Group) },
	{ IPT_UI_NEXT_GROUP, BUTTON_INPUT(Ui, Next_Group) },
	{ IPT_UI_ROTATE, BUTTON_INPUT(Ui, Rotate) },
	{ IPT_UI_SHOW_PROFILER, BUTTON_INPUT(Ui, Show_Profiler) },
	{ IPT_UI_TOGGLE_UI, BUTTON_INPUT(Ui, Toggle_Ui) },
	{ IPT_UI_TOGGLE_DEBUG, BUTTON_INPUT(Ui, Toggle_Debug) },
	{ IPT_UI_PASTE, BUTTON_INPUT(Ui, Paste) },
	{ IPT_UI_SAVE_STATE, BUTTON_INPUT(Ui, Save_State) },
	{ IPT_UI_LOAD_STATE, BUTTON_INPUT(Ui, Load_State) },
    { IPT_OSD_1, INVALID_INPUT },
    { IPT_OSD_2, INVALID_INPUT },
    { IPT_OSD_3, INVALID_INPUT },
    { IPT_OSD_4, INVALID_INPUT },
    { IPT_OSD_5, INVALID_INPUT },
    { IPT_OSD_6, INVALID_INPUT },
    { IPT_OSD_7, INVALID_INPUT },
    { IPT_OSD_8, INVALID_INPUT },
    { IPT_OSD_9, INVALID_INPUT },
    { IPT_OSD_10, INVALID_INPUT },
    { IPT_OSD_11, INVALID_INPUT },
    { IPT_OSD_12, INVALID_INPUT },
    { IPT_OSD_13, INVALID_INPUT },
    { IPT_OSD_14, INVALID_INPUT },
    { IPT_OSD_15, INVALID_INPUT },
    { IPT_OSD_16, INVALID_INPUT },
    { IPT_UI_LAST, INVALID_INPUT },
    { IPT_OTHER, INVALID_INPUT },
    { IPT_SPECIAL, INVALID_INPUT },
    { IPT_CUSTOM, INVALID_INPUT },
    { IPT_OUTPUT, INVALID_INPUT }
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
    int index = CBDATA_IPT(data);

    if (index >= g_input_descriptor_count) {
        /* This is weird, we're being asked for something bogus */
        return 0;
    }

    libmame_input_type input_type = g_input_descriptors[index].type;
    int input_number = g_input_descriptors[index].number;

    /* Just in case we need these */
    LibMame_PerPlayerControlsState *perplayer_state =
        &(g_state.controls_state.per_player[player]);
    LibMame_SharedControlsState *shared_state =
        &(g_state.controls_state.shared);

    /* MAME defines input ranges from -65536 to +65536 inclusive.  This is 17
       bits which is cumbersome.  So libmame allows ranges of -65535 to
       +65535, which is 16 bits in each direction and much less cumbersome.
       Just in case MAME treats the maximum or minimum of a range specially,
       we'll map an input of -65535 to -65536 for MAME and same for +65535 and
       +65536. */
#define RANGED(value) ((value == -65535) ? -65536 : \
                       (value == 65535) ? 65536 : value)

    /* Supply only half-axis for pedal inputs (negative pedal values make
     * no sense) */
#define PEDAL(value) ((value < 0) ? ((value == -65535) ? -65536 : -value) : \
                      (value == 65535) ? 65536 : value)

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
    case libmame_input_type_Shared_button:
        return (shared_state->shared_buttons_state & (1 << input_number));
    case libmame_input_type_left_joystick:
        return (perplayer_state->left_joystick_state & (1 << input_number));
    case libmame_input_type_right_joystick:
        return (perplayer_state->right_joystick_state & (1 << input_number));
    case libmame_input_type_analog_joystick_horizontal:
        return RANGED(perplayer_state->analog_joystick_horizontal_state);
    case libmame_input_type_analog_joystick_vertical:
        return RANGED(perplayer_state->analog_joystick_vertical_state);
    case libmame_input_type_spinner:
        return RANGED(perplayer_state->spinner_delta);
    case libmame_input_type_vertical_spinner:
        return RANGED(perplayer_state->vertical_spinner_delta);
    case libmame_input_type_paddle:
        return RANGED(perplayer_state->paddle_state);
    case libmame_input_type_vertical_paddle:
        return RANGED(perplayer_state->vertical_paddle_state);
    case libmame_input_type_trackball_horizontal:
        return RANGED(perplayer_state->trackball_horizontal_delta);
    case libmame_input_type_trackball_vertical:
        return RANGED(perplayer_state->trackball_vertical_delta);
    case libmame_input_type_lightgun_horizontal:
        return RANGED(perplayer_state->lightgun_horizontal_state);
    case libmame_input_type_lightgun_vertical:
        return RANGED(perplayer_state->lightgun_vertical_state);
    case libmame_input_type_pedal:
        return PEDAL(perplayer_state->pedal_state);
    case libmame_input_type_pedal2:
        return PEDAL(perplayer_state->pedal2_state);
    case libmame_input_type_pedal3:
        return PEDAL(perplayer_state->pedal3_state);
    case libmame_input_type_Ui_button:
        return (shared_state->ui_input_state == input_number);
    }

#undef RANGED

    /* Weird, this is not an input type that we know about */
    return 0;
}


/**
 * This is the callback we hook up to the input device that MAME uses
 * to be called back to get the state of a special input.  We also
 * arrange that the data passed in includes enough information to identify
 * what bit of state is being asked about; this one function handles all of
 * the input for all special inputs.
 **/
static INT32 get_special_state(void *, void *data)
{
    int special_button_index = (long) (uintptr_t) data;

    LibMame_SharedControlsState *shared_state =
        &(g_state.controls_state.shared);

    return shared_state->special_buttons_state & (1 << special_button_index);
}


static bool has_left_joystick_except(int controller_flags, int exception)
{
    if (controller_flags & (1 << exception)) {
        return false;
    }

    return (controller_flags &
            ((1 << LibMame_ControllerType_LeftHorizontalJoystick) |
             (1 << LibMame_ControllerType_LeftVerticalJoystick) |
             (1 << LibMame_ControllerType_Left4WayJoystick) |
             (1 << LibMame_ControllerType_Left8WayJoystick)));
}


static bool has_right_joystick_except(int controller_flags, int exception)
{
    if (controller_flags & (1 << exception)) {
        return false;
    }

    return (controller_flags &
            ((1 << LibMame_ControllerType_RightHorizontalJoystick) |
             (1 << LibMame_ControllerType_RightVerticalJoystick) |
             (1 << LibMame_ControllerType_Right4WayJoystick) |
             (1 << LibMame_ControllerType_Right8WayJoystick)));
}


static bool controllers_have_input
    (int num_players, const LibMame_AllControllers *controllers,
     int player, int index)
{
    /* Game doesn't support this player, no need for this input */
    if (player >= num_players) {
        return false;
    }

    /* If it's not an IPT_TYPE that we have in our table, then no need for
       this input as we can't handle it */
    if (index >= g_input_descriptor_count) {
        return false;
    }
    
    libmame_input_type input_type = g_input_descriptors[index].type;
    int input_number = g_input_descriptors[index].number;

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
    case libmame_input_type_Shared_button:
        return (controllers->shared.shared_button_flags & (1 << input_number));
    case libmame_input_type_left_joystick:
        switch (input_number) {
        case LibMame_JoystickDirection_Up:
        case LibMame_JoystickDirection_Down:
            return has_left_joystick_except
                (controllers->per_player.controller_flags,
                 LibMame_ControllerType_LeftHorizontalJoystick);
        case LibMame_JoystickDirection_Left:
        case LibMame_JoystickDirection_Right:
            return has_left_joystick_except
                (controllers->per_player.controller_flags,
                 LibMame_ControllerType_LeftVerticalJoystick);
        }
    case libmame_input_type_right_joystick:
        switch (input_number) {
        case LibMame_JoystickDirection_Up:
        case LibMame_JoystickDirection_Down:
            return has_right_joystick_except
                (controllers->per_player.controller_flags,
                 LibMame_ControllerType_RightHorizontalJoystick);
        case LibMame_JoystickDirection_Left:
        case LibMame_JoystickDirection_Right:
            return has_right_joystick_except
                (controllers->per_player.controller_flags,
                 LibMame_ControllerType_RightVerticalJoystick);
        }
    case libmame_input_type_analog_joystick_horizontal:
    case libmame_input_type_analog_joystick_vertical:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Analog8WayJoystick));
    case libmame_input_type_spinner:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Spinner));
    case libmame_input_type_vertical_spinner:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_VerticalSpinner));
    case libmame_input_type_paddle:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_Paddle));
    case libmame_input_type_vertical_paddle:
        return (controllers->per_player.controller_flags &
                (1 << LibMame_ControllerType_VerticalPaddle));
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
    case libmame_input_type_Ui_button:
        return true;
    }

    return false;
}


static void startup_callback(running_machine &machine)
{
    /**
     * If the special input ports have not been configured yet, do so now.
     * This is the earliest opportunity we have to do this, which must be done
     * after osd_customize_input_type_list.
     **/
    if (!g_state.special_inputs_configured) {
        g_state.special_inputs_configured = true;
        input_device *keyboard = 0;
        input_item_id keyboard_item = ITEM_ID_A;
        int keyboard_count = 0;
        int special_button_index = 0;

        ioport_manager &man = g_state.machine->ioport();

        for (ioport_port *port = man.first_port(); port != NULL;
             port = port->next()) {
            for (ioport_field *field = port->first_field(); field != NULL;
                 field = field->next()) {
                // This matches the detection done in 
                // libmame_games::convert_controllers, to ensure that
                // the descriptions we inform the user about match the
                // special input ports we set up here
                if (field->unused()) {
                    continue;
                }
                if (field->cocktail()) {
                    continue;
                }
                if (field->player() != 0) {
                    continue;
                }
                if ((field->type() != IPT_OTHER) || !field->name()) {
                    continue;
                }
                if (!keyboard || (keyboard_item > ITEM_ID_Z)) {
                    char namebuf[256];
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_special_keyboard_%d", 
                             keyboard_count++);
                    keyboard = machine.input().device_class
                        (DEVICE_CLASS_KEYBOARD).add_device(namebuf);
                    keyboard_item = ITEM_ID_A;
                }
                input_code item_input_code = 
                    input_code(DEVICE_CLASS_KEYBOARD,
                               keyboard->devindex(),
                               ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE,
                               keyboard_item);
                keyboard->add_item("", keyboard_item, &get_special_state,
                                   (void *) (uintptr_t) special_button_index++);
                keyboard_item++;

                // XXX bji - this no longer works.  MAME cannot be forced to
                // use an item input code via a method call, it has to extract
                // it from some settings values somewhere.

                // The dipswitch and settings stuff has changed significantly
                // over time and I am not sure that libmame's use of it works
                // anymore anyway.
                
                // It might be better to more significantly hack MAME to
                // have much, much saner API for querying about controls,
                // dipswitches, adjusters, and configuration stuff, and
                // setting those at runtime, than to try to continue to
                // try to force MAME into doing what I want in this minimal
                // fashion.

                // Basically, I would leave the macros that are used by all of
                // the drivers to define their ports and stuff, but
                // reimplement them into something usable.  Also I would
                // figure out what minimal API is necessary for the drivers to
                // acquire input values and re-implement that too.
#if 0
                port->first_field()->seq(SEQ_TYPE_STANDARD).
                    set(item_input_code);
#else
                (void) item_input_code;
#endif
            }
        }
    }

    LibMame_StartupPhase phase;

    switch (machine.init_phase()) {
    case STARTUP_PHASE_PREPARING:
        phase = LibMame_StartupPhase_Preparing;
        break;
    case STARTUP_PHASE_LOADING_ROMS:
        phase = LibMame_StartupPhase_LoadingRoms;
        break;
    case STARTUP_PHASE_INITIALIZING_STATE:
        phase = LibMame_StartupPhase_InitializingMachine;
        break;
    default:
        /* Else ignore, unknown phase? */
        return;
    }

    /* Currently, only one running game at a time is supported, so just pass
       in a bogus value */
    (*(g_state.callbacks->StartingUp))
        (phase, machine.init_phase_percent_complete(), 
         (LibMame_RunningGame *) 0x1, g_state.callback_data);
}


static void pause_callback(running_machine &machine)
{
    if (g_state.waiting_for_pause) {
        g_state.waiting_for_pause = false;
        (*(g_state.callbacks->Paused))(g_state.callback_data);
        /* Unpause */
        machine.resume();
    }
    /* Else, spontaneous pause by MAME internally, ignore it */
}


static void set_configuration_value(LibMame_RunningGame *game,
                                    const char *tag, uint32_t mask,
                                    int value)
{
    (void) game;

    // This no longer works; the whole dipswitch/coniguration thing needs
    // to be redone as per other comments

#if 0
    const input_field_config *config = input_field_by_tag_and_mask
        (*(g_state.machine), tag, mask);

    if (config != NULL) {
        input_field_user_settings settings;
        settings.value = value;
        input_field_set_user_settings(config, &settings);
    }
#endif
}


static void look_up_and_set_configuration_value(LibMame_RunningGame *game,
                                                int gamenum,
                                                const char *tag,
                                                uint32_t mask,
                                                const char *value)
{
    int count = LibMame_Get_Game_Dipswitch_Count(gamenum);
    /* Find the descriptor */
    for (int i = 0; i < count; i++) {
        LibMame_Dipswitch desc = LibMame_Get_Game_Dipswitch(gamenum, i);
        if ((desc.mask == mask) && !strcmp(desc.tag, tag)) {
            /* Found the descriptor, now find the value */
            for (int j = 0; j < desc.value_count; j++) {
                if (!strcmp(desc.value_names[j], value)) {
                    set_configuration_value(game, tag, mask, j);
                    break;
                }
            }
            break;
        }
    }
}


/** **************************************************************************
 * libmame OSD function implementations
 ************************************************************************** **/

static void osd_init(running_machine *machine)
{
    /**
     *  Save away the machine, we'll need it in osd_customize_input_type_list
     **/
    g_state.machine = machine;
    
    /**
     * Create the render_target that tells MAME the rendering parameters it
     * will use.
     **/
    g_state.target = g_state.machine->render().target_alloc();

    /**
     * Have this target hold every view since we only support one target
     **/
    g_state.target->set_view(g_state.target->configured_view("auto", 0, 1));

    /**
     * Set render target bounds to 10000 x 10000 and allow the callback to
     * scale that to whatever they want.
     **/
    g_state.target->set_bounds(10000, 10000, 1.0);

    /* Add a startup callback so that we can forward this info to users */
    machine->add_notifier(MACHINE_NOTIFY_STARTUP, 
                          machine_notify_delegate(FUNC(startup_callback), 
                                                  machine));

    /* Add a callback so that we can know when the running game has paused */
    machine->add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate
                          (FUNC(pause_callback), machine));
}


static void osd_update(running_machine *machine, int skip_redraw)
{
    /**
     * Poll input
     **/
    memset(g_state.controls_state.per_player, 0, 
           sizeof(LibMame_PerPlayerControlsState) * 
           g_state.maximum_player_count);
    g_state.controls_state.shared.shared_buttons_state = 0;
    g_state.controls_state.shared.ui_input_state = 0;

    (*(g_state.callbacks->PollAllControlsState))
        (&(g_state.controls_state), g_state.callback_data);

    /**
     * Ask the callbacks to update the video.  For now, assume that there
     * is only one display.  Might want to support multiple displays in the
     * future.
     **/
    if (!skip_redraw) {
        render_primitive_list &list = g_state.target->get_primitives();
        list.acquire_lock();
        (*(g_state.callbacks->UpdateVideo))
            ((LibMame_RenderPrimitive *) list.first(), g_state.callback_data);
        list.release_lock();
    }

    /**
     * Give the callbacks a chance to make running game calls
     **/
    (*(g_state.callbacks->MakeRunningGameCalls))(g_state.callback_data);
}


static void osd_update_audio_stream(running_machine *machine, 
                                    const INT16 *buffer,
                                    int samples_this_frame)
{
    /**
     * Ask the callbacks to update the audio
     **/
    (*(g_state.callbacks->UpdateAudio))(machine->sample_rate(), 
                                        samples_this_frame,
                                        buffer, g_state.callback_data);
}


static void osd_set_mastervolume(int attenuation)
{
    /**
     * Ask the callbacks to set the master volume
     **/
    (*(g_state.callbacks->SetMasterVolume))(attenuation, g_state.callback_data);
}


/**
 * This function is called back by MAME when it is starting up.  It passes the
 * descriptor that it uses internally for every possible input port, and gives
 * us the opportunity to define what callback we will provide for fetching the
 * state of that input port.  We ignore those input ports that are not used in
 * the current game, and for the others, make our own custom controllers as
 * neceessary to provide the inputs, and hook the input callbacks up to fetch
 * the input state from the one single LibMame_AllControlsState structure
 * that we store in the g_state object.  In this way, we completely hardwire
 * the way that MAME handles input so that the users of libmame can do
 * whatever they want to satisfy getting inputs for the various controllers
 * types.
 **/
static void
osd_customize_input_type_list(simple_list<input_type_entry> &typelist)
{
    /**
     * For each input descriptor, create a keyboard key, or mouse axis, or
     * lightgun control, or joystick axis for it, and change its input
     * sequence to match the new item.  Also, set up the polling callback for
     * it to have the data it needs to get the right controller state.
     **/
	input_type_entry *entry;
    input_device *item_device;
    input_item_id item_id = ITEM_ID_INVALID;
    input_code item_input_code;

    /**
     * New keyboards are created as we run out of keys; the only keys we
     * use are between ITEM_ID_A and ITEM_ID_Z (inclusive).  It is
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
     * types.  And why only use keyboard keys up to Z and not the others?
     * Well, apparently MAME also does something very weird and interprets
     * devices which have been mapped to key '0' (and probably other MAME
     * default UI keys) as having input that triggers that UI event.
     **/
    input_device *keyboard = 0;
    input_item_id keyboard_item = ITEM_ID_A;
    int keyboard_count = 0;
    /* As to all of the rest, they are created as needed, one for each player */
    input_device *analog_joystick[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    input_device *spinner[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    input_device *paddle[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; 
    input_device *trackball[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    input_device *lightgun[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    input_device *pedal[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    char namebuf[256];


#define GET_ITEM_DEVICE_AND_ID(devices, deviceclass, item_class)        \
    do {                                                                \
        if (devices [entry->player] == 0) {                             \
            snprintf(namebuf, sizeof(namebuf),                          \
                     "libmame_virtual_" #devices "_%d",                 \
                     entry->player);                                    \
            devices [entry->player] = g_state.machine->input().         \
                device_class(deviceclass).add_device(namebuf);          \
        }                                                               \
        item_device = devices [entry->player];                          \
        item_id = g_input_descriptors[entry->type].item_id;             \
        item_input_code = input_code(deviceclass,                       \
                                     devices [entry->player]->          \
                                     devindex(),                        \
                                     item_class, ITEM_MODIFIER_NONE,    \
                                     item_id);                          \
    } while (0);


	for (entry = typelist.first(); entry; entry = entry->next()) {
        item_device = NULL;
        int index = 0;
        for (index = 0; index < g_input_descriptor_count; index++) {
            if (g_input_descriptors[index].ipt_port == entry->type()) {
                break;
            }
        }
        if (index == g_input_descriptor_count) {
            continue;
        }
        if (controllers_have_input(g_state.maximum_player_count,
                                   &(g_state.controllers),
                                   entry->player(), index)) {
            switch (g_input_descriptors[entry->type()].type) {
            case libmame_input_type_invalid:
                break;

            case libmame_input_type_Normal_button:
            case libmame_input_type_Mahjong_button:
            case libmame_input_type_Hanafuda_button:
            case libmame_input_type_Gambling_button:
            case libmame_input_type_Shared_button:
            case libmame_input_type_left_joystick:
            case libmame_input_type_right_joystick:
            case libmame_input_type_Ui_button:
                if (!keyboard || (keyboard_item > ITEM_ID_Z)) {
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_keyboard_%d", keyboard_count++);
                    keyboard = g_state.machine->input().device_class
                        (DEVICE_CLASS_KEYBOARD).add_device(namebuf);
                    keyboard_item = ITEM_ID_A;
                }
                item_device = keyboard;
                item_id = keyboard_item;
                keyboard_item = (input_item_id) (keyboard_item + 1);
                item_input_code = 
                    input_code(DEVICE_CLASS_KEYBOARD, keyboard->devindex(),
                               ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE,
                               item_id);
                break;

                /*
                 * For all of the following, we use DEVICE_CLASS_LIGHTGUN,
                 * ITEM_CLASS_ABSOLUTE for any device for which we provide
                 * absolute values, and DEVICE_CLASS_MOUSE,
                 * ITEM_CLASS_RELATIVE for any device for which we provide
                 * relative values.  The { MOUSE, RELATIVE } choice makes
                 * sense, but the { LIGHTGUN, ABSOLUTE } seems weird for
                 * things like analog joysticks and paddles.  However,
                 * LIGHTGUN is the device class that MAME does the least
                 * amount of internal fiddling with when interpreting input
                 * values, and that's what we want.  If we choose
                 * DEVICE_CLASS_JOYSTICK then it will do deadzone stuff, and
                 * if we choose DEVICE_CLASS_MOUSE, it will force the input to
                 * be relative which will cause input handling to always use
                 * 0.  It's weird, but DEVICE_CLASS_LIGHTGUN is the best
                 * choice for any absolute control type.
                 */

            case libmame_input_type_analog_joystick_horizontal:
            case libmame_input_type_analog_joystick_vertical:
                /* Yes, it's weird that LIGHTGUN is used here - see above
                   comment */
                GET_ITEM_DEVICE_AND_ID(analog_joystick, DEVICE_CLASS_LIGHTGUN,
                                       ITEM_CLASS_ABSOLUTE);
                break;

            case libmame_input_type_spinner:
            case libmame_input_type_vertical_spinner:
                GET_ITEM_DEVICE_AND_ID(spinner, DEVICE_CLASS_MOUSE, 
                                       ITEM_CLASS_RELATIVE);
                break;

            case libmame_input_type_paddle:
            case libmame_input_type_vertical_paddle:
                /* Yes, it's weird that LIGHTGUN is used here - see above
                   comment */
                GET_ITEM_DEVICE_AND_ID(paddle, DEVICE_CLASS_LIGHTGUN,
                                       ITEM_CLASS_ABSOLUTE);
                break;

            case libmame_input_type_trackball_horizontal:
            case libmame_input_type_trackball_vertical:
                GET_ITEM_DEVICE_AND_ID(trackball, DEVICE_CLASS_MOUSE,
                                       ITEM_CLASS_RELATIVE);
                break;

            case libmame_input_type_lightgun_horizontal:
            case libmame_input_type_lightgun_vertical:
                /* Hey, LIGHTGUN actually makes sense here! */
                GET_ITEM_DEVICE_AND_ID(lightgun, DEVICE_CLASS_LIGHTGUN,
                                       ITEM_CLASS_ABSOLUTE);
                break;

            case libmame_input_type_pedal:
            case libmame_input_type_pedal2:
            case libmame_input_type_pedal3:
                /* Yes, it's weird that LIGHTGUN is used here - see above
                   comment */
                GET_ITEM_DEVICE_AND_ID(pedal, DEVICE_CLASS_LIGHTGUN,
                                       ITEM_CLASS_ABSOLUTE);
                break;
            }
        }

        if (item_device) {
            item_device->add_item("", item_id, &get_controller_state,
                                  CBDATA_MAKE(entry->player, index));
        }
        else {
            /* For some reason or another, we can't handle this input, so turn
               it off by setting it to an item_id that we never use */
            item_id = ITEM_ID_MAXIMUM;
        }

        entry->defseq[SEQ_TYPE_STANDARD].set(item_input_code);
    }
}


/** **************************************************************************
 * LibMame exported function implementations
 ************************************************************************** **/

LibMame_RunGameStatus LibMame_RunGame(int gamenum, int benchmarking,
                                      const LibMame_RunGameOptions *options,
                                      const LibMame_RunGameCallbacks *cbs,
                                      void *callback_data)
{
    if (gamenum >= LibMame_Get_Game_Count()) {
        return LibMame_RunGameStatus_InvalidGameNum;
    }

#define MAME_LAMENESS \
    output_delegate(FUNC(LibMame_RunGame_State::output_callback), &g_state)

    /* Set up MAME's "output channels" so that we accumulate it all in a
       buffer rather than dumping it to stdout/stderr/wherever */
    mame_set_output_channel(OUTPUT_CHANNEL_ERROR, MAME_LAMENESS);
    mame_set_output_channel(OUTPUT_CHANNEL_WARNING, MAME_LAMENESS);
    mame_set_output_channel(OUTPUT_CHANNEL_INFO, MAME_LAMENESS);
    mame_set_output_channel(OUTPUT_CHANNEL_DEBUG, MAME_LAMENESS);
    mame_set_output_channel(OUTPUT_CHANNEL_VERBOSE, MAME_LAMENESS);
    mame_set_output_channel(OUTPUT_CHANNEL_LOG, MAME_LAMENESS);

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
    emu_options mame_options;
    if (options == NULL) {
        LibMame_RunGameOptions default_options;
        LibMame_Get_Default_RunGameOptions(&default_options);
        get_mame_options(&default_options,
                         LibMame_Get_Game_Short_Name(gamenum), mame_options);
    }
    else {
        get_mame_options(options, LibMame_Get_Game_Short_Name(gamenum),
                         mame_options);
    }

    /* Haven't configured special inputs yet */
    g_state.special_inputs_configured = false;

    /* Not waiting for a pause */
    g_state.waiting_for_pause = false;

    /* Run the game */
    int result;
    {
        libmame_rungame_osd_interface osd;
        result = mame_execute(mame_options, osd, benchmarking ? true : false);
    }

    /* Convert the resulting MAME code to a libmame code and return */
    switch (result) {
    case MAMERR_NONE:
        return LibMame_RunGameStatus_Success;
    case MAMERR_FAILED_VALIDITY:
        return LibMame_RunGameStatus_FailedValidityCheck;
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


const char *LibMame_RunningGame_GetSpeedText(LibMame_RunningGame *game)
{
    (void) game;

    g_state.machine->video().speed_text(g_state.speed_text);

    return g_state.speed_text.cstr();
}


void LibMame_RunningGame_Schedule_Pause(LibMame_RunningGame *game)
{
    (void) game;

    g_state.waiting_for_pause = true;

    g_state.machine->pause();
}


void LibMame_RunningGame_Schedule_Exit(LibMame_RunningGame *game)
{
    (void) game;

    g_state.machine->schedule_exit();
}


void LibMame_RunningGame_Schedule_Hard_Reset(LibMame_RunningGame *game)
{
    (void) game;

    g_state.machine->schedule_hard_reset();
}


void LibMame_RunningGame_Schedule_Soft_Reset(LibMame_RunningGame *game)
{
    (void) game;

    g_state.machine->schedule_soft_reset();
}


void LibMame_RunningGame_SaveState(LibMame_RunningGame *game, 
                                   const char *filename)
{
    (void) game;

    g_state.machine->schedule_save(filename);
}


void LibMame_RunningGame_LoadState(LibMame_RunningGame *game,
                                   const char *filename)
{
    (void) game;

    g_state.machine->schedule_load(filename);
}


void LibMame_RunningGame_ChangeDipswitchValue(LibMame_RunningGame *game,
                                              const char *tag, uint32_t mask,
                                              const char *value)
{
    look_up_and_set_configuration_value
        (game, g_state.gamenum, tag, mask, value);
}
