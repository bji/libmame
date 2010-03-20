/** **************************************************************************
 * libmame_iav.c
 *
 * LibMame Input, Audio, and Video function implementations.
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "HashTable.h"
#include "emu.h"
#include "osdcore.h"
#include "osdepend.h"
#include "libmame.h"
#include <string.h>


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
#define DIGCTL_IPT_MASK                          0xFFF0

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
#define DIGCTL_MAKE_JOYSTICK_DIRECTION(player_number, type, direction) \
    ((player_number) | (type) | (direction))
#define DIGCTL_PLAYER_NUMBER(d)       (((long) d) & DIGCTL_PLAYER_MASK)
#define DIGCTL_TYPE(d)                (((long) d) & DIGCTL_TYPE_MASK)
#define DIGCTL_ITEM_NUMBER(d)         ((((long) d) & DIGCTL_ITEM_MASK) >> 8)
#define DIGCTL_JOYSTICK(d)            (((long) d) & DIGCTL_ITEM_MASK)


typedef enum
{
    libmame_input_type_invalid,
    libmame_input_type_normal_button,
    libmame_input_type_mahjong_button,
    libmame_input_type_hanafuda_button,
    libmame_input_type_gambling_button,
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
    libmame_input_type_normal_button,
    libmame_input_type_mahjong_button,
    libmame_input_type_hanafuda_button,
    libmame_input_type_gambling_button,
    libmame_input_type_other_button,
    libmame_input_type_ui_button
} libmame_input_type;

/**
 * Each IPT_ input port type has a corresponding entry in a table here that
 * provides enough details about that IPT_ entry to locate its state value
 * in the controllers state.
 **/
typedef struct libmame_input_descriptor
{
    libmame_input_type type;
    int input_number;
} libmame_input_descriptor;

/* This maps each input IPT_ type to an input descriptor */
static libmame_input_descriptor g_input_descriptors[] =
{
	{ libmame_input_type_invalid, 0 }, /* IPT_INVALID */
	{ libmame_input_type_invalid, 0 }, /* IPT_UNUSED */
	{ libmame_input_type_invalid, 0 }, /* IPT_END */
	{ libmame_input_type_invalid, 0 }, /* IPT_UNKNOWN */
	{ libmame_input_type_invalid, 0 }, /* IPT_PORT */
	{ libmame_input_type_invalid, 0 }, /* IPT_DIPSWITCH */
	{ libmame_input_type_invalid, 0 }, /* IPT_VBLANK */
	{ libmame_input_type_invalid, 0 }, /* IPT_CONFIG */
	{ libmame_input_type_invalid, 0 }, /* IPT_CATEGORY */
    { libmame_input_type_other_button, LibMame_OtherButtonType_Start1 }, /* IPT_START1 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Start2 }, /* IPT_START2 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Start3 }, /* IPT_START3 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Start4 }, /* IPT_START4 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Start5 }, /* IPT_START5 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Start6 }, /* IPT_START6 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Start7 }, /* IPT_START7 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Start8 }, /* IPT_START8 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Coin1 }, /* IPT_COIN1 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Coin2 }, /* IPT_COIN2 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Coin3 }, /* IPT_COIN3 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Coin4 }, /* IPT_COIN4 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Coin5 }, /* IPT_COIN5 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Coin6 }, /* IPT_COIN6 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Coin7 }, /* IPT_COIN7 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Coin8 }, /* IPT_COIN8 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Bill1 }, /* IPT_BILL1 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Service1 }, /* IPT_SERVICE1 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Service2 }, /* IPT_SERVICE2 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Service3 }, /* IPT_SERVICE3 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Service4 }, /* IPT_SERVICE4 */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Service }, /* IPT_SERVICE */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Tilt }, /* IPT_TILT */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Interlock }, /* IPT_INTERLOCK */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Volume_Up }, /* IPT_VOLUME_UP */
	{ libmame_input_type_other_button, LibMame_OtherButtonType_Volume_Down }, /* IPT_VOLUME_DOWN */
	{ libmame_input_type_invalid, 0 }, /* IPT_START */
	{ libmame_input_type_invalid, 0 }, /* IPT_SELECT */
	{ libmame_input_type_invalid, 0 }, /* IPT_KEYBOARD */
	{ libmame_input_type_left_or_single_joystick_up, 0 }, /* IPT_JOYSTICK_UP */
	{ libmame_input_type_left_or_single_joystick_down, 0 }, /* IPT_JOYSTICK_DOWN */
	{ libmame_input_type_left_or_single_joystick_left, 0 }, /* IPT_JOYSTICK_LEFT */
	{ libmame_input_type_left_or_single_joystick_right, 0 }, /* IPT_JOYSTICK_RIGHT */
	{ libmame_input_type_right_joystick_up, 0 }, /* IPT_JOYSTICKRIGHT_UP */
	{ libmame_input_type_right_joystick_down, 0 }, /* IPT_JOYSTICKRIGHT_DOWN */
	{ libmame_input_type_right_joystick_left, 0 }, /* IPT_JOYSTICKRIGHT_LEFT */
	{ libmame_input_type_right_joystick_right, 0 }, /* IPT_JOYSTICKRIGHT_RIGHT */
	{ libmame_input_type_left_or_single_joystick_up, 0 }, /* IPT_JOYSTICKLEFT_UP */
	{ libmame_input_type_left_or_single_joystick_downt, 0 }, /* IPT_JOYSTICKLEFT_DOWN */
	{ libmame_input_type_left_or_single_joystick_left, 0 }, /* IPT_JOYSTICKLEFT_LEFT */
	{ libmame_input_type_left_or_single_joystick_right, 0 }, /* IPT_JOYSTICKLEFT_RIGHT */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_1 }, /* IPT_BUTTON1 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_2 }, /* IPT_BUTTON2 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_3 }, /* IPT_BUTTON3 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_4 }, /* IPT_BUTTON4 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_5 }, /* IPT_BUTTON5 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_6 }, /* IPT_BUTTON6 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_7 }, /* IPT_BUTTON7 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_8 }, /* IPT_BUTTON8 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_9 }, /* IPT_BUTTON9 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_10 }, /* IPT_BUTTON10 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_11 }, /* IPT_BUTTON11 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_12 }, /* IPT_BUTTON12 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_13 }, /* IPT_BUTTON13 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_14 }, /* IPT_BUTTON14 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_15 }, /* IPT_BUTTON15 */
	{ libmame_input_type_normal_button, LibMame_NormalButtonType_16 }, /* IPT_BUTTON16 */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_A }, /* IPT_MAHJONG_A */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_B }, /* IPT_MAHJONG_B */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_C }, /* IPT_MAHJONG_C */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_D }, /* IPT_MAHJONG_D */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_E }, /* IPT_MAHJONG_E */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_F }, /* IPT_MAHJONG_F */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_G }, /* IPT_MAHJONG_G */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_H }, /* IPT_MAHJONG_H */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_I }, /* IPT_MAHJONG_I */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_J }, /* IPT_MAHJONG_J */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_K }, /* IPT_MAHJONG_K */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_L }, /* IPT_MAHJONG_L */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_M }, /* IPT_MAHJONG_M */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_N }, /* IPT_MAHJONG_N */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_O }, /* IPT_MAHJONG_O */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_P }, /* IPT_MAHJONG_P */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Q }, /* IPT_MAHJONG_Q */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Kan }, /* IPT_MAHJONG_KAN */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Pon }, /* IPT_MAHJONG_PON */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Chi }, /* IPT_MAHJONG_CHI */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Reach }, /* IPT_MAHJONG_REACH */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Ron }, /* IPT_MAHJONG_RON */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Bet }, /* IPT_MAHJONG_BET */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Last_Chance }, /* IPT_MAHJONG_LAST_CHANCE */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Score }, /* IPT_MAHJONG_SCORE */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Double_Up }, /* IPT_MAHJONG_DOUBLE_UP */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Flip_Flop }, /* IPT_MAHJONG_FLIP_FLOP */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Big }, /* IPT_MAHJONG_BIG */
	{ libmame_input_type_mahjong_button, LibMame_MahjongButtonType_Small }, /* IPT_MAHJONG_SMALL */
	{ libmame_input_type_hanafuda_button, LibMame_HanafudaButtonType_A }, /* IPT_HANAFUDA_A */
	{ libmame_input_type_hanafuda_button, LibMame_HanafudaButtonType_B }, /* IPT_HANAFUDA_B */
	{ libmame_input_type_hanafuda_button, LibMame_HanafudaButtonType_C }, /* IPT_HANAFUDA_C */
	{ libmame_input_type_hanafuda_button, LibMame_HanafudaButtonType_D }, /* IPT_HANAFUDA_D */
	{ libmame_input_type_hanafuda_button, LibMame_HanafudaButtonType_E }, /* IPT_HANAFUDA_E */
	{ libmame_input_type_hanafuda_button, LibMame_HanafudaButtonType_F }, /* IPT_HANAFUDA_F */
	{ libmame_input_type_hanafuda_button, LibMame_HanafudaButtonType_G }, /* IPT_HANAFUDA_G */
	{ libmame_input_type_hanafuda_button, LibMame_HanafudaButtonType_H }, /* IPT_HANAFUDA_H */
	{ libmame_input_type_hanafuda_button, LibMame_HanafudaButtonType_Yes }, /* IPT_HANAFUDA_YES */
	{ libmame_input_type_hanafuda_button, LibMame_HanafudaButtonType_No }, /* IPT_HANAFUDA_NO */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_High }, /* IPT_GAMBLE_HIGH */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Low }, /* IPT_GAMBLE_LOW */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Half }, /* IPT_GAMBLE_HALF */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Deal }, /* IPT_GAMBLE_DEAL */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_D_Up }, /* IPT_GAMBLE_D_UP */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Take }, /* IPT_GAMBLE_TAKE */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Stand }, /* IPT_GAMBLE_STAND */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Bet }, /* IPT_GAMBLE_BET */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Keyin }, /* IPT_GAMBLE_KEYIN */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Keyout }, /* IPT_GAMBLE_KEYOUT */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Payout }, /* IPT_GAMBLE_PAYOUT */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Door }, /* IPT_GAMBLE_DOOR */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Service }, /* IPT_GAMBLE_SERVICE */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Book }, /* IPT_GAMBLE_BOOK */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Hold1 }, /* IPT_POKER_HOLD1 */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Hold2 }, /* IPT_POKER_HOLD2 */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Hold3 }, /* IPT_POKER_HOLD3 */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Hold4 }, /* IPT_POKER_HOLD4 */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Hold5 }, /* IPT_POKER_HOLD5 */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Cancel }, /* IPT_POKER_CANCEL */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Poker_Bet }, /* IPT_POKER_BET */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Stop1 }, /* IPT_SLOT_STOP1 */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Stop2 }, /* IPT_SLOT_STOP2 */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Stop3 }, /* IPT_SLOT_STOP3 */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Stop4 }, /* IPT_SLOT_STOP4 */
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Stop_All }, /* IPT_SLOT_STOP_ALL */
	{ libmame_input_type_paddle, 0 }, /* IPT_PADDLE */
	{ libmame_input_type_paddle_vertical, 0 }, /* IPT_PADDLE_V */
	{ libmame_input_type_analog_joystick_horizontal, 0 }, /* IPT_AD_STICK_X */
	{ libmame_input_type_analog_joystick_vertical, 0 }, /* IPT_AD_STICK_Y */
	{ libmame_input_type_analog_joystick_altitude, 0 }, /* IPT_AD_STICK_Z */
	{ libmame_input_type_lightgun_horizontal, 0 }, /* IPT_LIGHTGUN_X */
	{ libmame_input_type_lightgun_vertical, 0 }, /* IPT_LIGHTGUN_Y */
	{ libmame_input_type_pedal, 0 }, /* IPT_PEDAL */
	{ libmame_input_type_pedal2, 0 }, /* IPT_PEDAL2 */
	{ libmame_input_type_pedal3, 0 }, /* IPT_PEDAL3 */
	{ libmame_input_type_positional, 0 }, /* IPT_POSITIONAL */
	{ libmame_input_type_positional_vertical, 0}, /* IPT_POSITIONAL_V */
	{ libmame_input_spinner, 0 }, /* IPT_DIAL */
	{ libmame_input_spinner_vertical, 0 }, /* IPT_DIAL_V */
	{ libmame_input_spinner_trackball_horizontal, 0 }, /* IPT_TRACKBALL_X */
	{ libmame_input_spinner_trackball_vertical, 0}, /* IPT_TRACKBALL_Y */
	{ libmame_input_type_invalid, 0 }, /* IPT_MOUSE_X */
	{ libmame_input_type_invalid, 0 }, /* IPT_MOUSE_Y */
	{ }, /* IPT_ADJUSTER */
	{ }, /* IPT_UI_CONFIGURE */
	{ }, /* IPT_UI_ON_SCREEN_DISPLAY */
	{ }, /* IPT_UI_DEBUG_BREAK */
	{ }, /* IPT_UI_PAUSE */
	{ }, /* IPT_UI_RESET_MACHINE */
	{ }, /* IPT_UI_SOFT_RESET */
	{ }, /* IPT_UI_SHOW_GFX */
	{ }, /* IPT_UI_FRAMESKIP_DEC */
	{ }, /* IPT_UI_FRAMESKIP_INC */
	{ }, /* IPT_UI_THROTTLE */
	{ }, /* IPT_UI_FAST_FORWARD */
	{ }, /* IPT_UI_SHOW_FPS */
	{ }, /* IPT_UI_SNAPSHOT */
	{ }, /* IPT_UI_RECORD_MOVIE */
	{ }, /* IPT_UI_TOGGLE_CHEAT */
	{ }, /* IPT_UI_UP */
	{ }, /* IPT_UI_DOWN */
	{ }, /* IPT_UI_LEFT */
	{ }, /* IPT_UI_RIGHT */
	{ }, /* IPT_UI_HOME */
	{ }, /* IPT_UI_END */
	{ }, /* IPT_UI_PAGE_UP */
	{ }, /* IPT_UI_PAGE_DOWN */
	{ }, /* IPT_UI_SELECT */
	{ }, /* IPT_UI_CANCEL */
	{ }, /* IPT_UI_DISPLAY_COMMENT */
	{ }, /* IPT_UI_CLEAR */
	{ }, /* IPT_UI_ZOOM_IN */
	{ }, /* IPT_UI_ZOOM_OUT */
	{ }, /* IPT_UI_PREV_GROUP */
	{ }, /* IPT_UI_NEXT_GROUP */
	{ }, /* IPT_UI_ROTATE */
	{ }, /* IPT_UI_SHOW_PROFILER */
	{ }, /* IPT_UI_TOGGLE_UI */
	{ }, /* IPT_UI_TOGGLE_DEBUG */
	{ }, /* IPT_UI_PASTE */
	{ }, /* IPT_UI_SAVE_STATE */
	{ } /* IPT_UI_LOAD_STATE */
};
            

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
    LibMame_AllControllersState controllers_state;

    /* Hash MAME input_item_id to a DIGCTL value for each keyboard key */
    static Hash::Table<int, input_item_id> digctl_to_key_id_hash;

} LibMame_RunGame_State;


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
    if (DIGCTL_TYPE(data) == DIGCTL_TYPE_OTHER_BUTTON) {
        LibMame_SharedControllersState *state = 
            &(g_state.controllers_state.shared);
        return (state->other_buttons_state &
                (1 << (DIGCTL_ITEM_NUMBER(data))));
    }
    /* Else it's a per-player state */

    /* Get the controller state associated with the given digital control
       descriptor */
    LibMame_PerPlayerControllersState *state = 
        &(g_state.controllers_state.per_player[DIGCTL_PLAYER_NUMBER(data)]);

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
        return (state->normal_buttons_state & 
                (1 << (DIGCTL_ITEM_NUMBER(data))));
    case DIGCTL_TYPE_LEFT_OR_SINGLE_JOYSTICK:
        switch (DIGCTL_JOYSTICK(data)) {
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
        switch (DIGCTL_JOYSTICK(data)) {
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
    case DIGCTL_TYPE_UI_BUTTON:
        /* UI input is not a flag, it is an actual value to check against */
        return (state->ui_input_state == DIGCTL_ITEM_NUMBER(data));
    case DIGCTL_TYPE_MAHJONG_BUTTON:
        return (state->mahjong_buttons_state &
                (1 << (DIGCTL_ITEM_NUMBER(data))));
    case DIGCTL_TYPE_HANAFUDA_BUTTON:
        return (state->hanafuda_buttons_state &
                (1 << (DIGCTL_ITEM_NUMBER(data))));
    case DIGCTL_TYPE_GAMBLING_BUTTON:
        return (state->gambling_buttons_state &
                (1 << (DIGCTL_ITEM_NUMBER(data))));
    }

    /* Weird, this is not a 'key' that we know about */
    return 0;
}


static void libmame_add_keyboard_button(input_device *kbd, input_item_id key_id,
                                        int digctl)
{
    /* Create an input device for the keyboard that will have the callback
       called back with the digital control descriptor and that will be
       associated with this key for this keyboard */
    input_device_item_add(kbd, "", (void *) digctl, key_id,
                          &get_digital_controller_state);
    /* And hash so that we can look up the key by the digital control when
       customizing the inputs in libmame_osd_customize_input_type_list */
    input_item_id *value;
    g_state.digctl_to_key_id_hash.Put(digctl, /* returns */ value);
    *value = key_id;
}


static void libmame_osd_init(running_machine *machine)
{
    /**
     * Create a huge virtual keyboard to cover all possible buttons.
     **/
    input_device *kbd = input_device_add(machine, DEVICE_CLASS_KEYBOARD, 
                                         "libmame_virtual_keyboard", NULL);

    /**
     * The buttons that the keyboard has will be created in
     * libmame_osd_customize_input_type_list
     **/


    /* For each player ... */
    for (int player = 0; player < 8; player++) {

        /* Add buttons to the keyboard for this player.  This loops over every
           button for the given type, and calls the add helper function, and
           then increments key_id since that key_id was just used. */
#define ADD_BUTTONS(count, type)                                        \
        do {                                                            \
            for (int button = 0; button < count; button++) {            \
                libmame_add_keyboard_button                             \
                    (kbd, key_id, DIGCTL_MAKE_BUTTON(player, type,      \
                                                     button));          \
                key_id++;                                               \
            }                                                           \
        } while (0)

        ADD_BUTTONS(LibMame_NormalButtonTypeCount, 
                    DIGCTL_TYPE_NORMAL_BUTTON);
        ADD_BUTTONS(LibMame_MahjongButtonTypeCount, 
                    DIGCTL_TYPE_MAHJONG_BUTTON);
        ADD_BUTTONS(LibMame_HanafudaButtonTypeCount, 
                    DIGCTL_TYPE_HANAFUDA_BUTTON);
        ADD_BUTTONS(LibMame_GamblingButtonTypeCount, 
                    DIGCTL_TYPE_GAMBLING_BUTTON);
        ADD_BUTTONS(LibMame_OtherButtonTypeCount, 
                    DIGCTL_TYPE_OTHER_BUTTON);

        for (int joystick = 0; joystick < 2; joystick++) {
            for (int direction = 0; direction < 4; direction++) {
                libmame_add_keyboard_button
                    (kbd, key_id, DIGCTL_MAKE_JOYSTICK_DIRECTION
                     (player, DIGCTL_TYPE_LEFT_OR_SINGLE_JOYSTICK + joystick, 
                      DIGCTL_JOYSTICK_LEFT + direction));
                key_id++;
            }
        }
    }

    /* Other buttons */
    for (int button = 0; button < LibMame_OtherButtonTypeCount; button++) {
        libmame_add_keyboard_button
            (kbd, key_id, DIGCTL_MAKE_BUTTON(0, DIGCTL_TYPE_OTHER_BUTTON,
                                             button));
        key_id++;
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
    memset(g_state.controllers_state.per_player, 0, 
           sizeof(LibMame_PerPlayerControllersState) * 
           g_state.running_game_maximum_player_count);
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
     * Examine every input.  For each one, create a new keyboard key in
     * the keyboard device and assign the input to that.
     **/

    input_item_id key_id = ITEM_ID_FIRST_VALID;

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
