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
#include "libmame.h"
#include <string.h>


/**
 * This is a packed representation of everything necessary to identify a
 * unique digital control.
 **/
#define DIGCTL_PLAYER_MASK              0x000F
#define DIGCTL_IPT_MASK                 0xFFF0
#define DIGCTL_PLAYER(d)                (((long) d) & DIGCTL_PLAYER_MASK)
#define DIGCTL_IPT(d)                   ((((long) d) & DIGCTL_IPT_MASK) >> 8)
#define DIGCTL_MAKE(player, ipt)        (((player) & DIGCTL_PLAYER_MASK) | \
                                         ((ipt << 8) & DIGCTL_IPT_MASK))

typedef enum
{
    libmame_input_type_invalid,
    libmame_input_type_normal_button,
    libmame_input_type_mahjong_button,
    libmame_input_type_hanafuda_button,
    libmame_input_type_gambling_button,
    libmame_input_type_other_button,
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
    int number;
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
	{ libmame_input_type_left_or_single_joystick_down, 0 }, /* IPT_JOYSTICKLEFT_DOWN */
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
	{ libmame_input_type_gambling_button, LibMame_GamblingButtonType_Bet }, /* IPT_POKER_BET */
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
	{ libmame_input_type_spinner, 0 }, /* IPT_DIAL */
	{ libmame_input_type_spinner_vertical, 0 }, /* IPT_DIAL_V */
	{ libmame_input_type_trackball_horizontal, 0 }, /* IPT_TRACKBALL_X */
	{ libmame_input_type_trackball_vertical, 0}, /* IPT_TRACKBALL_Y */
	{ libmame_input_type_mouse_x, 0 }, /* IPT_MOUSE_X */
	{ libmame_input_type_mouse_y, 0 }, /* IPT_MOUSE_Y */
	{ libmame_input_type_invalid, 0 }, /* IPT_ADJUSTER */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Configure }, /* IPT_UI_CONFIGURE */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_On_Screen_Display }, /* IPT_UI_ON_SCREEN_DISPLAY */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Debug_Break }, /* IPT_UI_DEBUG_BREAK */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Pause }, /* IPT_UI_PAUSE */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Reset_Machine }, /* IPT_UI_RESET_MACHINE */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Soft_Reset }, /* IPT_UI_SOFT_RESET */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Show_Gfx }, /* IPT_UI_SHOW_GFX */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Frameskip_Dec }, /* IPT_UI_FRAMESKIP_DEC */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Frameskip_Inc }, /* IPT_UI_FRAMESKIP_INC */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Throttle }, /* IPT_UI_THROTTLE */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Fast_Forward }, /* IPT_UI_FAST_FORWARD */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Show_Fps }, /* IPT_UI_SHOW_FPS */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Snapshot }, /* IPT_UI_SNAPSHOT */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Record_Movie }, /* IPT_UI_RECORD_MOVIE */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Toggle_Cheat }, /* IPT_UI_TOGGLE_CHEAT */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Up }, /* IPT_UI_UP */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Down }, /* IPT_UI_DOWN */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Left }, /* IPT_UI_LEFT */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Right }, /* IPT_UI_RIGHT */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Home }, /* IPT_UI_HOME */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_End }, /* IPT_UI_END */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Page_Up }, /* IPT_UI_PAGE_UP */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Page_Down }, /* IPT_UI_PAGE_DOWN */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Select }, /* IPT_UI_SELECT */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Cancel }, /* IPT_UI_CANCEL */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Display_Comment }, /* IPT_UI_DISPLAY_COMMENT */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Clear }, /* IPT_UI_CLEAR */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Zoom_In }, /* IPT_UI_ZOOM_IN */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Zoom_Out }, /* IPT_UI_ZOOM_OUT */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Prev_Group }, /* IPT_UI_PREV_GROUP */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Next_Group }, /* IPT_UI_NEXT_GROUP */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Rotate }, /* IPT_UI_ROTATE */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Show_Profiler }, /* IPT_UI_SHOW_PROFILER */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Toggle_Ui }, /* IPT_UI_TOGGLE_UI */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Toggle_Debug }, /* IPT_UI_TOGGLE_DEBUG */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Paste }, /* IPT_UI_PASTE */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Save_State }, /* IPT_UI_SAVE_STATE */
	{ libmame_input_type_ui_button, LibMame_UiButtonType_Load_State } /* IPT_UI_LOAD_STATE */
};
            
static int g_input_descriptor_count = 
    (sizeof(g_input_descriptors) / sizeof(g_input_descriptors[0]));

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

    /**
     * This is the running machine that this state is associated with
     **/
    running_machine *machine;

    /**
     * This is the virtual keyboard device
     **/
    input_device *kbd;

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


static INT32 get_controller_state(void *, void *data)
{
    int player = DIGCTL_PLAYER(data);
    int ipt_type = DIGCTL_IPT(data);
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
    case libmame_input_type_normal_button:
        return (perplayer_state->normal_buttons_state & (1 << input_number));
    case libmame_input_type_mahjong_button:
        return (perplayer_state->mahjong_buttons_state & (1 << input_number));
    case libmame_input_type_hanafuda_button:
        return (perplayer_state->hanafuda_buttons_state & (1 << input_number));
    case libmame_input_type_gambling_button:
        return (perplayer_state->gambling_buttons_state & (1 << input_number));
    case libmame_input_type_other_button:
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
        return perplayer_state->pedal_state;
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
    case libmame_input_type_ui_button:
        return perplayer_state->ui_input_state;
    }

    /* Weird, this is not an input type that we know about */
    return 0;
}


static void libmame_osd_init(running_machine *machine)
{
    /**
     *  Save away the machine, we'll need it in 
     * libmame_osd_customize_input_type_list
     **/
    g_state.machine = machine;

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
     * For each input descriptor, create a keyboard key, or mouse axis, or
     * lightgun???, or joystick axis for it, and change its input sequence to
     * match the new item.  Also, set up the polling callback for it to have
     * the data it needs to get the right controller state.
     **/
	input_type_desc *typedesc;
    input_device *item_device;
    input_item_id item_id;

    /**
     * New keyboards are created as we run out of keys; the only keys we
     * use are between ITEM_ID_A and ITEM_ID_XAXIS - 1.
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
    input_device *positional[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    input_device *mouse[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    char namebuf[256];

	for (typedesc = typelist; typedesc; typedesc = typedesc->next) {
        item_device = NULL;
        if (typedesc->type < g_input_descriptor_count) {
            switch (g_input_descriptors[typedesc->type].type) {
            case libmame_input_type_invalid:
                break;
            case libmame_input_type_normal_button:
            case libmame_input_type_mahjong_button:
            case libmame_input_type_hanafuda_button:
            case libmame_input_type_gambling_button:
            case libmame_input_type_other_button:
            case libmame_input_type_left_or_single_joystick_left:
            case libmame_input_type_left_or_single_joystick_right:
            case libmame_input_type_left_or_single_joystick_up:
            case libmame_input_type_left_or_single_joystick_down:
            case libmame_input_type_right_joystick_left:
            case libmame_input_type_right_joystick_right:
            case libmame_input_type_right_joystick_up:
            case libmame_input_type_right_joystick_down:
            case libmame_input_type_ui_button:
                if ((keyboard == NULL) || (keyboard_item > ITEM_ID_CANCEL)) {
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_keyboard_%d", keyboard_count++);
                    keyboard = input_device_add
                        (g_state.machine, DEVICE_CLASS_KEYBOARD, 
                         namebuf, NULL);
                    keyboard_item = ITEM_ID_A;
                }
                item_device = keyboard;
                item_id = keyboard_item;
                keyboard_item++;
                break;

            case libmame_input_type_analog_joystick_horizontal:
            case libmame_input_type_analog_joystick_vertical:
            case libmame_input_type_analog_joystick_altitude:
                if (analog_joystick[typedesc->player] == NULL) {
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_analog_joystick_player_%d", 
                             typedesc->player);
                    analog_joystick[typedesc->player] = input_device_add
                        (g_state.machine, DEVICE_CLASS_JOYSTICK, 
                         namebuf, NULL);
                }
                item_device = analog_joystick[typedesc->player];
                item_id = ((typedesc->type == 
                            libmame_input_type_analog_joystick_horizontal) ?
                           ITEM_ID_XAXIS :
                           (typedesc->type == 
                            libmame_input_type_analog_joystick_vertical) ?
                           ITEM_ID_YAXIS : ITEM_ID_ZAXIS);
                break;

            case libmame_input_type_spinner:
            case libmame_input_type_spinner_vertical:
                if (spinner[typedesc->player] == NULL) {
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_spinner_player_%d", 
                             typedesc->player);
                    spinner[typedesc->player] = input_device_add
                        (g_state.machine, DEVICE_CLASS_MOUSE, namebuf, NULL);
                }
                item_device = spinner[typedesc->player];
                item_id = ((typedesc->type == libmame_input_type_spinner) ?
                           ITEM_ID_RXAXIS : ITEM_ID_RYAXIS);
                break;

            case libmame_input_type_paddle:
            case libmame_input_type_paddle_vertical:
                if (paddle[typedesc->player] == NULL) {
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_paddle_player_%d", 
                             typedesc->player);
                    paddle[typedesc->player] = input_device_add
                        (g_state.machine, DEVICE_CLASS_MOUSE, namebuf, NULL);
                }
                item_device = paddle[typedesc->player];
                item_id = ((typedesc->type == libmame_input_type_paddle) ?
                           ITEM_ID_XAXIS : ITEM_ID_YAXIS);
                break;

            case libmame_input_type_trackball_horizontal:
            case libmame_input_type_trackball_vertical:
                if (trackball[typedesc->player] == NULL) {
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_trackball_player_%d", 
                             typedesc->player);
                    trackball[typedesc->player] = input_device_add
                        (g_state.machine, DEVICE_CLASS_MOUSE, namebuf, NULL);
                }
                item_device = trackball[typedesc->player];
                item_id = ((typedesc->type == 
                            libmame_input_type_trackball_horizontal) ? 
                           ITEM_ID_RXAXIS : ITEM_ID_RYAXIS);
                break;

            case libmame_input_type_lightgun_horizontal:
            case libmame_input_type_lightgun_vertical:
                if (lightgun[typedesc->player] == NULL) {
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_lightgun_player_%d", 
                             typedesc->player);
                    lightgun[typedesc->player] = input_device_add
                        (g_state.machine, DEVICE_CLASS_LIGHTGUN,
                         namebuf, NULL);
                }
                item_device = lightgun[typedesc->player];
                item_id = ((typedesc->type ==
                            libmame_input_type_lightgun_horizontal) ?
                           ITEM_ID_XAXIS : ITEM_ID_YAXIS);
                break;

            case libmame_input_type_pedal:
            case libmame_input_type_pedal2:
            case libmame_input_type_pedal3:
                if (pedal[typedesc->player] == NULL) {
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_pedal_player_%d", 
                             typedesc->player);
                    pedal[typedesc->player] = input_device_add
                        (g_state.machine, DEVICE_CLASS_MOUSE, namebuf, NULL);
                }
                item_device = pedal[typedesc->player];
                item_id = ((typedesc->type == libmame_input_type_pedal) ?
                           ITEM_ID_XAXIS :
                           ((typedesc->type == libmame_input_type_pedal2) ?
                            ITEM_ID_YAXIS : ITEM_ID_ZAXIS));
                break;

            case libmame_input_type_positional:
            case libmame_input_type_positional_vertical:
                if (positional[typedesc->player] == NULL) {
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_positional_player_%d", 
                             typedesc->player);
                    positional[typedesc->player] = input_device_add
                        (g_state.machine, DEVICE_CLASS_MOUSE, namebuf, NULL);
                }
                item_device = positional[typedesc->player];
                item_id = ((typedesc->type == libmame_input_type_positional) ?
                           ITEM_ID_XAXIS : ITEM_ID_YAXIS);
                break;

            case libmame_input_type_mouse_x:
            case libmame_input_type_mouse_y:
                if (mouse[typedesc->player] == NULL) {
                    snprintf(namebuf, sizeof(namebuf), 
                             "libmame_virtual_mouse_player_%d", 
                             typedesc->player);
                    mouse[typedesc->player] = input_device_add
                        (g_state.machine, DEVICE_CLASS_MOUSE, namebuf, NULL);
                }
                item_device = mouse[typedesc->player];
                item_id = ((typedesc->type == libmame_input_type_positional) ?
                           ITEM_ID_XAXIS : ITEM_ID_YAXIS);
                break;
            }
        }

        if (item_device) {
            input_device_item_add
                (item_device, "", 
                 (void *) DIGCTL_MAKE(typedesc->player, typedesc->type), 
                 item_id, &get_controller_state);
        }
        else {
            /* For some reason or another, we can't handle this input, so turn
               it off by setting it to an item_id that we never use */
            item_id = ITEM_ID_MAXIMUM;
        }

        input_seq_set_1(&(typedesc->seq[SEQ_TYPE_STANDARD]), item_id);
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
