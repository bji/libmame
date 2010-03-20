/** **************************************************************************
 * libmame.h
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#ifndef __LIBMAME_H__
#define __LIBMAME_H__


/** **************************************************************************
 * Preprocessor definitions - constants
 ************************************************************************** **/

/**
 * These flags define the 'working' status of a game.  Any number of these
 * flags may be or'd together to indicate the complete working status of a
 * game.  If none of these flags are set, then the game is known to work
 * perfectly.
 **/

/**
 * Colors are totally wrong.
 **/
#define LIBMAME_WORKINGFLAGS_WRONG_COLORS                       0x001
/**
 * Colors are not 100% accurate, but close.
 **/
#define LIBMAME_WORKINGFLAGS_IMPERFECT_COLORS                   0x002
/**
 * Graphics are wrong/incomplete.
 **/
#define LIBMAME_WORKINGFLAGS_IMPERFECT_GRAPHICS                 0x004
/**
 * Sound is missing, either due to missing sound files, or missing
 * hardware emulation.
 **/
#define LIBMAME_WORKINGFLAGS_NO_SOUND                           0x008
/**
 * Sound is known to be wrong.
 **/
#define LIBMAME_WORKINGFLAGS_IMPERFECT_SOUND                    0x010
/**
 * Screen flip support is missing.
 **/
#define LIBMAME_WORKINGFLAGS_NO_COCKTAIL                        0x020
/**
 * Game's protection not fully emulated.
 **/
#define LIBMAME_WORKINGFLAGS_UNEMULATED_PROTECTION              0x040
/**
 * Game does not support saving state.
 **/
#define LIBMAME_WORKINGFLAGS_NO_SAVE_STATE                      0x080
/**
 * Game does not work at all.
 **/
#define LIBMAME_WORKINGFLAGS_NOTWORKING                         0x100


/**
 * These flags define the screen orientations that could supported by a game.
 * Not all games support all orientations.
 **/

/**
 * Mirror along a vertical line midway aross the screen.
 **/
#define LIBMAME_ORIENTATIONFLAGS_FLIP_X                         0x01

/**
 * Mirror along a horizontal line midway across the screen
 **/
#define LIBMAME_ORIENTATIONFLAGS_FLIP_Y                         0x02

/**
 * Rotate the screen 90 degrees
 **/
#define LIBMAME_ORIENTATIONFLAGS_ROTATE_90                      0x04

/**
 * Rotate the screen 180 degrees
 **/
#define LIBMAME_ORIENTATIONFLAGS_ROTATE_180                     0x08

/**
 * Rotate the screen 270 degrees
 **/
#define LIBMAME_ORIENTATIONFLAGS_ROTATE_270                     0x10


/** **************************************************************************
 * Enumerated types
 ************************************************************************** **/

/**
 * Screen types
 **/
typedef enum
{
    LibMame_ScreenType_Raster,
    LibMame_ScreenType_LCD,
    LibMame_ScreenType_Vector
} LibMame_ScreenType;

/**
 * Setting types
 **/
typedef enum
{
    /**
     * This is a setting that is a MAME-specific, nonstandard configuration
     * option for a game.  Changes to these settings typically only take
     * effect when a game is re-loaded in MAME.
     **/
    LibMame_SettingType_Configuration,
    /**
     * This is a setting that was present in the original game as a
     * dipswitch.  Changes to these settings typically only take effect in
     * between games.
     **/
    LibMame_SettingType_Dipswitch,
    /**
     * This is a setting that may have been present in the original game or
     * may not have been, but that affects a parameter of the game that can be
     * adjusted in real time as a game runs.  Changes to these settings take
     * effect immediately.
    **/
    LibMame_SettingType_Adjuster
} LibMame_SettingType;

/**
 * All of the possible controller types
 **/
typedef enum
{
    LibMame_ControllerType_JoystickHorizontal,
    LibMame_ControllerType_JoystickVertical,
    LibMame_ControllerType_Joystick4Way,
    LibMame_ControllerType_Joystick8Way,
    LibMame_ControllerType_DoubleJoystickHorizontal,
    LibMame_ControllerType_DoubleJoystickVertical,
    LibMame_ControllerType_DoubleJoystick4Way,
    LibMame_ControllerType_DoubleJoystick8Way,
    LibMame_ControllerType_JoystickAnalog,
    LibMame_ControllerType_Spinner,
    LibMame_ControllerType_SpinnerVertical,
    LibMame_ControllerType_Paddle,
    LibMame_ControllerType_PaddleVertical,
    LibMame_ControllerType_Trackball,
    LibMame_ControllerType_Lightgun,
    LibMame_ControllerType_Pedal,
    LibMame_ControllerType_Pedal2,
    LibMame_ControllerType_Pedal3,
    LibMame_ControllerType_Positional,
    LibMame_ControllerType_PositionalVertical,
    LibMame_ControllerType_Mouse,
    /* This is not a type, it's the number of entries in this enum */
    LibMame_ControllerTypeCount
} LibMame_ControllerType;

/**
 * All of the possible general purpose buttons.
 **/
typedef enum
{
    LibMame_NormalButtonType_1,
    LibMame_NormalButtonType_2,
    LibMame_NormalButtonType_3,
    LibMame_NormalButtonType_4,
    LibMame_NormalButtonType_5,
    LibMame_NormalButtonType_6,
    LibMame_NormalButtonType_7,
    LibMame_NormalButtonType_8,
    LibMame_NormalButtonType_9,
    LibMame_NormalButtonType_10,
    LibMame_NormalButtonType_11,
    LibMame_NormalButtonType_12,
    LibMame_NormalButtonType_13,
    LibMame_NormalButtonType_14,
    LibMame_NormalButtonType_15,
    LibMame_NormalButtonType_16,
    /* This is not a type, it's the number of entries in this enum */
    LibMame_NormalButtonTypeCount
} LibMame_NormalButtonType;

/**
 * All of the possible Mahjong buttons.
 **/
typedef enum
{
    LibMame_MahjongButtonType_A,
    LibMame_MahjongButtonType_B,
    LibMame_MahjongButtonType_C,
    LibMame_MahjongButtonType_D,
    LibMame_MahjongButtonType_E,
    LibMame_MahjongButtonType_F,
    LibMame_MahjongButtonType_G,
    LibMame_MahjongButtonType_H,
    LibMame_MahjongButtonType_I,
    LibMame_MahjongButtonType_J,
    LibMame_MahjongButtonType_K,
    LibMame_MahjongButtonType_L,
    LibMame_MahjongButtonType_M,
    LibMame_MahjongButtonType_N,
    LibMame_MahjongButtonType_O,
    LibMame_MahjongButtonType_P,
    LibMame_MahjongButtonType_Q,
    LibMame_MahjongButtonType_Kan,
    LibMame_MahjongButtonType_Pon,
    LibMame_MahjongButtonType_Chi,
    LibMame_MahjongButtonType_Reach,
    LibMame_MahjongButtonType_Ron,
    LibMame_MahjongButtonType_Bet,
    LibMame_MahjongButtonType_Last_Chance,
    LibMame_MahjongButtonType_Score,
    LibMame_MahjongButtonType_Double_Up,
    LibMame_MahjongButtonType_Flip_Flop,
    LibMame_MahjongButtonType_Big,
    LibMame_MahjongButtonType_Small,
    /* This is not a type, it's the number of entries in this enum */
    LibMame_MahjongButtonTypeCount
} LibMame_MahjongButtonType;

/**
 * All of the possible Hanafuda buttons.
 **/
typedef enum
{
    LibMame_HanafudaButtonType_A,
    LibMame_HanafudaButtonType_B,
    LibMame_HanafudaButtonType_C,
    LibMame_HanafudaButtonType_D,
    LibMame_HanafudaButtonType_E,
    LibMame_HanafudaButtonType_F,
    LibMame_HanafudaButtonType_G,
    LibMame_HanafudaButtonType_H,
    LibMame_HanafudaButtonType_Yes,
    LibMame_HanafudaButtonType_No,
    /* This is not a type, it's the number of entries in this enum */
    LibMame_HanafudaButtonTypeCount
} LibMame_HanafudaButtonType;

/**
 * All of the possible gambling buttons.
 **/
typedef enum
{
    LibMame_GamblingButtonType_High,
    LibMame_GamblingButtonType_Low,
    LibMame_GamblingButtonType_Half,
    LibMame_GamblingButtonType_Deal,
    LibMame_GamblingButtonType_D_Up,
    LibMame_GamblingButtonType_Take,
    LibMame_GamblingButtonType_Stand,
    LibMame_GamblingButtonType_Bet,
    LibMame_GamblingButtonType_Keyin,
    LibMame_GamblingButtonType_Keyout,
    LibMame_GamblingButtonType_Payout,
    LibMame_GamblingButtonType_Door,
    LibMame_GamblingButtonType_Service,
    LibMame_GamblingButtonType_Book,
    LibMame_GamblingButtonType_Hold1,
    LibMame_GamblingButtonType_Hold2,
    LibMame_GamblingButtonType_Hold3,
    LibMame_GamblingButtonType_Hold4,
    LibMame_GamblingButtonType_Hold5,
    LibMame_GamblingButtonType_Cancel,
    LibMame_GamblingButtonType_Stop1,
    LibMame_GamblingButtonType_Stop2,
    LibMame_GamblingButtonType_Stop3,
    LibMame_GamblingButtonType_Stop4,
    LibMame_GamblingButtonType_Stop_All,
    /* This is not a type, it's the number of entries in this enum */
    LibMame_GamblingButtonTypeCount
} LibMame_GamblingButtonType;

/**
 * All of the possible 'other' buttons.
 **/
typedef enum
{
    LibMame_OtherButtonType_Coin1,
    LibMame_OtherButtonType_Coin2,
    LibMame_OtherButtonType_Coin3,
    LibMame_OtherButtonType_Coin4,
    LibMame_OtherButtonType_Coin5,
    LibMame_OtherButtonType_Coin6,
    LibMame_OtherButtonType_Coin7,
    LibMame_OtherButtonType_Coin8,
    LibMame_OtherButtonType_Bill1,
    LibMame_OtherButtonType_Start1,
    LibMame_OtherButtonType_Start2,
    LibMame_OtherButtonType_Start3,
    LibMame_OtherButtonType_Start4,
    LibMame_OtherButtonType_Start5,
    LibMame_OtherButtonType_Start6,
    LibMame_OtherButtonType_Start7,
    LibMame_OtherButtonType_Start8,
    LibMame_OtherButtonType_Service,
    LibMame_OtherButtonType_Service1,
    LibMame_OtherButtonType_Service2,
    LibMame_OtherButtonType_Service3,
    LibMame_OtherButtonType_Service4,
    LibMame_OtherButtonType_Tilt,
    LibMame_OtherButtonType_Interlock,
    LibMame_OtherButtonType_Volume_Up,
    LibMame_OtherButtonType_Volume_Down,
    /* This is not a type, it's the number of entries in this enum */
    LibMame_OtherButtonTypeCount
} LibMame_OtherButtonType;

/**
 * All of the possible UI buttons.
 **/
typedef enum
{
    LibMame_UiButtonType_Configure,
    LibMame_UiButtonType_On_Screen_Display,
    LibMame_UiButtonType_Debug_Break,
    LibMame_UiButtonType_Pause,
    LibMame_UiButtonType_Reset_Machine,
    LibMame_UiButtonType_Soft_Reset,
    LibMame_UiButtonType_Show_Gfx,
    LibMame_UiButtonType_Frameskip_Dec,
    LibMame_UiButtonType_Frameskip_Inc,
    LibMame_UiButtonType_Throttle,
    LibMame_UiButtonType_Fast_Forward,
    LibMame_UiButtonType_Show_Fps,
    LibMame_UiButtonType_Snapshot,
    LibMame_UiButtonType_Record_Movie,
    LibMame_UiButtonType_Toggle_Cheat,
    LibMame_UiButtonType_Up,
    LibMame_UiButtonType_Down,
    LibMame_UiButtonType_Left,
    LibMame_UiButtonType_Right,
    LibMame_UiButtonType_Home,
    LibMame_UiButtonType_End,
    LibMame_UiButtonType_Page_Up,
    LibMame_UiButtonType_Page_Down,
    LibMame_UiButtonType_Select,
    LibMame_UiButtonType_Cancel,
    LibMame_UiButtonType_Display_Comment,
    LibMame_UiButtonType_Clear,
    LibMame_UiButtonType_Zoom_In,
    LibMame_UiButtonType_Zoom_Out,
    LibMame_UiButtonType_Prev_Group,
    LibMame_UiButtonType_Next_Group,
    LibMame_UiButtonType_Rotate,
    LibMame_UiButtonType_Show_Profiler,
    LibMame_UiButtonType_Toggle_Ui,
    LibMame_UiButtonType_Toggle_Debug,
    LibMame_UiButtonType_Paste,
    LibMame_UiButtonType_Save_State,
    LibMame_UiButtonType_Load_State,
    /* This is not a type, it's the number of entries in this enum */
    LibMame_UiButtonTypeCount
} LibMame_UiButtonType;

/**
 * Status codes that can be returned by LibMame_RunGame()
 **/
typedef enum
{
    LibMame_RunGameStatus_Success,
    LibMame_RunGameStatus_InvalidGameNum
} LibMame_RunGameStatus;


/** **************************************************************************
 * Structured type definitions
 ************************************************************************** **/

/**
 * This is a screen size in pixels
 **/
typedef struct LibMame_ScreenResolution
{
    int width, height;
} LibMame_ScreenResolution;


/**
 * This describes the name and location of a sound sample.
 **/
typedef struct LibMame_SoundSampleDescriptor
{
    /**
     * This is the name of the sample, which is a file name of a sound file as
     * stored in a sound sample ZIP file.
     **/
    const char *name;

    /**
     * This is the game number of the game that has the actual sample.  Some
     * games reference sound sample files from other games, and thus use a
     * different source game number than their own game number.
     **/
    int source_gamenum;
} LibMame_SoundSampleDescriptor;


/**
 * This describes a chip that MAME emulates.
 **/
typedef struct LibMame_ChipDescriptor
{
    /**
     * This is true if the chip is a sound chip, false if it is a CPU chip
     **/
    bool is_sound;

    /**
     * This is a tag for the chip
     **/
    const char *tag;

    /**
     * This is the name of the chip
     **/
    const char *name;

    /**
     * This is the clock rate of the chip, or 0 if the clock rate is unknown
     **/
    int clock_hz;
} LibMame_ChipDescriptor;


/**
 * This describes a setting that MAME allows to be adjusted for a game.
 **/
typedef struct LibMame_SettingDescriptor
{
    /**
     * This is the type of this setting
     **/
    LibMame_SettingType type;

    /**
     * This is the name of the setting, which typically describes what the 
     * setting does.
     **/
    const char *name;

    /**
     * This is the number of individual values that the setting may be set to,
     * if the setting is a Configuration or Dipswitch setting.  This value is
     * meaningless for Adjuster settings, which always range from 0 to 100
     * inclusive.
     **/
    int value_count;

    /**
     * If this is a Configuration or Dipswitch setting, then this gives the
     * index into the value_names array of the default value.  If this is an
     * Adjuster setting, then this gives the default value of the setting.
     **/
    int default_value;

    /**
     * This is the names of each of the individual values that the setting
     * may be set to, which typically describe what the setting does.  These
     * are only relevent for Configuration and Dipswitch settings.
     **/
    const char * const *value_names;
} LibMame_SettingDescriptor;


/**
 * This describes a player's control inputs for a game.  This is a set of
 * individual controllers comprising the entire set of control inputs that a
 * player would use to play a game.  All players are assumed to use the same
 * type and number of controls during a multiplayer game.
 **/
typedef struct LibMame_PerPlayerControllersDescriptor
{
    /**
     * These are all of the general purpose buttons which are present,
     * each is indicated in this as (1 << LibMame_NormalButtonType_XXX).
     **/
    int normal_button_flags;

    /**
     * These are the names of the normal buttons, or NULL if the button has
     * no name, one per button type, or NULL if the button as no name.
     **/
    const char *normal_button_names[LibMame_NormalButtonTypeCount];

    /**
     * These flags identify which Mahjong buttons are present,
     * each is indicated in this as (1 << LibMame_MahjongButtonType_XXX).
     **/
    int mahjong_button_flags;

    /**
     * These flags identify which hanafuda buttons are present,
     * each is indicated in this as (1 << LibMame_HanafudaButtonType_XXX).
     **/
    int hanafuda_button_flags;

    /**
     * These flags identify which gambling buttons are present,
     * each is indicated in this as (1 << LibMame_GamblingButtonType_XXX).
     **/
    int gambling_button_flags;

    /**
     * These flags identify which controllers are present,
     * each is indicated in this as (1 << LibMame_ControllerType_XXX).
     **/
    int controller_flags;
} LibMame_PerPlayerPerPlayerControllersDescriptor;


/**
 * This describes the shared controller inputs for a game.  This is a set of
 * individual controllers comprising the entire set of control inputs that
 * a game has that are not active play controllers.  All players share these
 * controllers.
 **/
typedef struct LibMame_SharedControllersDescriptor
{
    /**
     * These flags identify which other binary controls are present,
     * each is indicated in this as (1 << LibMame_OtherButtonType_XXX).
     **/
    int other_button_flags;
} LibMame_SharedControllersDescriptor;

/**
 * This describes all of the controller values that can be polled by MAME for
 * individual players.  One structure of this type for each player is passed
 * to the xxx callback function periodically while a game is running to poll
 * the current input.  Not all values provided in this structure are used for
 * every game.
 **/
typedef struct LibMame_PerPlayerControllersState
{
    /**
     * These are the current states of each normal button; the flag for
     * a button being set here means that the button is currently pressed,
     * not being set means that the button is currently not pressed.  Each
     * is represented as a flag within this value, by the bit numbered
     * (1 << LibMame_NormalButtonType_XXX).
     **/
    int normal_buttons_state;

    /**
     * These are the current states of each Mahjong button; the flag for
     * a button being set here means that the button is currently pressed,
     * not being set means that the button is currently not pressed.  Each
     * is represented as a flag within this value, by the bit numbered
     * (1 << LibMame_MahjongButtonType_XXX).
     **/
    int mahjong_buttons_state;

    /**
     * These are the current states of each Hanafuda button; the flag for
     * a button being set here means that the button is currently pressed,
     * not being set means that the button is currently not pressed.  Each
     * is represented as a flag within this value, by the bit numbered
     * (1 << LibMame_HanafudaButtonType_XXX).
     **/
    int hanafuda_buttons_state;
    
    /**
     * These are the current states of each Gambling button; the flag for
     * a button being set here means that the button is currently pressed,
     * not being set means that the button is currently not pressed.  Each
     * is represented as a flag within this value, by the bit numbered
     * (1 << LibMame_GamblingButtonType_XXX).
     **/
    int gambling_buttons_state;
    
    /**
     * This value is set if the left joystick (or single joystick if the
     * controller has only a single joystick) is in the left, left-up, or
     * left-down position.
     **/
    int left_or_single_joystick_left_state : 1;

    /**
     * This value is set if the left joystick (or single joystick if the
     * controller has only a single joystick) is in the right, right-up, or
     * right-down position.
     **/
    int left_or_single_joystick_right_state : 1;

    /**
     * This value is set if the left joystick (or single joystick if the
     * controller has only a single joystick) is in the up, right-up, or
     * left-up position.
     **/
    int left_or_single_joystick_up_state : 1;

    /**
     * This value is set if the left joystick (or single joystick if the
     * controller has only a single joystick) is in the down, right-down, or
     * left-down position.
     **/
    int left_or_single_joystick_down_state : 1;

    /**
     * This value is set if the right joystick is in the left, left-up, or
     * left-down position.
     **/
    int right_joystick_left_state : 1;

    /**
     * This value is set if the right joystick is in the right, right-up, or
     * right-down position.
     **/
    int right_joystick_right_state : 1;

    /**
     * This value is set if the right joystick is in the up, right-up, or
     * left-up position.
     **/
    int right_joystick_up_state : 1;

    /**
     * This value is set if the right joystick is in the down, right-down, or
     * left-down position.
     **/
    int right_joystick_down_state : 1;

    /**
     * This value is the current horizontal position of the analog joystick,
     * mapped to a range from -65536 (full left) to 65536 (full right).
     **/
    int analog_joystick_horizontal_state;

    /**
     * This value is the current vertical position of the analog joystick,
     * mapped to a range from -65536 (full down) to 65536 (full up).
     **/
    int analog_joystick_vertical_state;

    /**
     * This value is the current position of the analog joystick when measured
     * in the Z axis, i.e. an altitude, mapped to a range from -65536 (full
     * down) to 65536 (full up).
     **/
    int analog_joystick_altitude_state;

    /**
     * This value is the change in position of the spinner since the last
     * time it was polled, mapped to a range from -65536 (furthest possible
     * spin left) to 65536 (furthest possible spin right).
     **/
    int spinner_delta;

    /**
     * This value is the change in position of the vertical spinner since the
     * last time it was polled, mapped to a range from -65536 (furthest
     * possible spin down) to 65536 (furthest possible spin up).
     **/
    int spinner_vertical_delta;

    /**
     * This value is the current paddle position, mapped to a range from
     * -65536 (full left) to 65536 (full right).
     **/
    int paddle_state;

    /**
     * This value is the current vertical paddle position, mapped to a range
     * from -65536 (full left) to 65536 (full right).
     **/
    int paddle_vertical_state;

    /**
     * This value is the change in position of the trackball along the
     * horizontal axis since the last time it was polled, mapped to a range
     * from -65536 (furthest possible roll left) 65536 (furthest possible roll
     * right).
     **/
    int trackball_horizontal_delta;

    /**
     * This value is the change in position of the trackball along the
     * vertical axis since the last time it was polled, mapped to a range
     * from -65536 (furthest possible roll down) 65536 (furthest possible roll
     * up).
     **/
    int trackball_vertical_delta;

    /**
     * This value is the current horizontal position of the analog joystick,
     * mapped to a range from -65536 (left edge of screen) to 65536 (right
     * edge of screen).  Any values outside this range indicates off of the
     * screen if present in either lightgun_horizontal_position_state or
     * lightgun_vertical_position_state.
     **/
    int lightgun_horizontal_state;

    /**
     * This value is the current vertical position of the analog joystick,
     * mapped to a range from -65536 (left edge of screen) to 65536 (right
     * edge of screen).  Any values outside this range indicates indicates off
     * of the screen if present in either lightgun_horizontal_position_state
     * or lightgun_vertical_position_state.
     **/
    int lightgun_vertical_state;

    /**
     * This value is the current paddle position, mapped to a range from
     * 0 (not pressed) to 65535 (fully pressed).
     **/
    int pedal_state;

    /**
     * This value is the current second pedal position, mapped to a range from
     * 0 (not pressed) to 65535 (fully pressed).
     **/
    int pedal2_state;

    /**
     * This value is the current third pedal position, mapped to a range from
     * 0 (not pressed) to 65535 (fully pressed).
     **/
    int pedal3_state;

    /**
     * This is some kind of positional input that I don't know what it is.
     * The range is from -65536 to 65536.
     **/
    int positional_state;

    /**
     * This is some kind of vertical positional input that I don't know what
     * it is. The range is from -65536 to 65536.
     **/
    int positional_vertical_state;

    /**
     * Mouse X screen coordinate.
     **/
    int mouse_x_state;

    /**
     * Mouse Y screen coordinate.
     **/
    int mouse_y_state;

    /**
     * This is the current UI input.  libmame allows only one UI input at a
     * time.  Its value is one of the LibMame_UiButtonType_XXX values.
     **/
    int ui_input_state;
} LibMame_PerPlayerControllersState;


/**
 * This describes all of the controller values that can be polled by MAME for
 * controllers that are shared by all players.  Not all values provided in
 * this structure are used for every game.
 **/
typedef struct LibMame_SharedControllersState
{
    /**
     * These are the current states of each other binary input; the flag for
     * an input being set here means that the input is currently triggered,
     * not being set means that the input is currently not triggered.  Each
     * is represented as a flag within this value, by the bit numbered
     * (1 << LibMame_OtherButtonType_XXX).  These are not duplicated per
     * player but instead have only one set of buttons shared across all
     * players.
     **/
    int other_buttons_state;
} LibMame_SharedControllersState;


/**
 * This combines the per-player and the shared controllers state into one
 * structure representing all states of all controllers.
 **/
typedef struct LibMame_AllControllersState
{
    /**
     * This is the per-player controllers states, one per player.  Not all
     * games use all players.
     **/
    LibMame_PerPlayerControllersState per_player[8];

    /**
     * This is the shared controllers state.
     **/
    LibMame_SharedControllersState shared;
} LibMame_AllControllersState;


typedef struct LibMame_RunGameCallbacks
{
    void (*PollAllControllersState)(LibMame_AllControllersState *all_states,
                                    void *callback_data);
    void (*MakeRunningGameCalls)(void *callback_data);
    void (*UpdateVideo)(void *callback_data);
    void (*UpdateAudio)(void *callback_data);
    void (*SetMasterVolume)(int attenuation, void *callback_data);
} LibMame_RunGameCallbacks;


/** **************************************************************************
 * Functions comprising the libmame API
 ************************************************************************** **/

/**
 * Functions for managing the library.
 **/

/**
 * Deinitializes libmame, releasing any resources that the library has
 * allocated.  It is not necessary to call this method before a process exits,
 * as the operating system will reclaim the memory that libmame has allocated;
 * however, when debugging, it may be useful to call this so that memory
 * allocated by libmame does not appear to have been leaked when the program
 * exits.
 **/
void LibMame_Deinitialize();


/**
 * Returns the string describing the version of mame that this instance of
 * libmame corresponds to.
 *
 * @return the string describing the version of mame that this instance of
 *         libmame corresponds to.
 **/
const char *LibMame_Get_Version_String();


/**
 * Functions for querying the supported games and their properties
 **/

/**
 * Returns the total number of games supported by this instance of libmame.
 *
 * @return the total number of games supported by this instance of libmame.
 **/
int LibMame_Get_Game_Count(); 


/**
 * Returns the game number of a game given the short name of the game, or -1
 * if there is no such game.
 *
 * @return the game number of a game given the short name of the game, or -1
 *         if there is no such game.
 **/
int LibMame_Get_Game_Number(const char *short_name);


/**
 * Returns the best matches for a given game short name.
 *
 * @param short_name is the short name to find matches for
 * @param num_matches gives the maxium number of matches to find; [gamenums]
 *                    must have at least this number of entries
 * @param gamenums will have its first N entries set to the game numbers of
 *        matching games (best match first), where N is the return value of
 *        this function
 * @return the number of matches returned; this many of the first entries of
 *         [gamenums] will include game numbers of games which matched.
 **/
int LibMame_Get_Game_Matches(const char *short_name, int num_matches,
                             int *gamenums);

/**
 * Returns the short name of a game.
 *
 * @param gamenum is the game number of the game
 * @return the short name of a game.
 **/
const char *LibMame_Get_Game_Short_Name(int gamenum);


/**
 * Returns the full name of a game.
 *
 * @param gamenum is the game number of the game
 * @return the full name of a game.
 **/
const char *LibMame_Get_Game_Full_Name(int gamenum);


/**
 * Returns the year that a game was released, or -1 if the year is unknown.
 *
 * @param gamenum is the game number of the game
 * @return the year that a game was released, or -1 if the year is unknown.
 **/
int LibMame_Get_Game_Year_Of_Release(int gamenum);


/**
 * Returns the short name of the game that this game is a clone of, if the
 * game is a clone.  Returns NULL if the game is not a clone.
 *
 * @param gamenum is the game number of the game
 * @return the short name of the game that this game is a clone of, if the
 *         game is a clone, or NULL if the game is not a clone.
 **/
const char *LibMame_Get_Game_CloneOf_Short_Name(int gamenum);


/**
 * Returns the name of the manufacturer of the given game.
 *
 * @param gamenum is the game number of the game
 * @return the name of the manufacturer of the given game.
 **/
const char *LibMame_Get_Game_Manufacturer(int gamenum);


/**
 * Returns the set of flags describing the game's 'working' status.  This is
 * an or'd together set of flags from the LIBMAME_WORKINGFLAGS_XXX symbols.
 * If 0 is returned, then the game is working perfectly, including being able
 * to save game state.
 *
 * @param gamenum is the game number of the game
 * @return the set of flags describing the game's 'working' status, or 0 if
 *         the game is working perfectly, including being able to save game
 *         state.
 **/
int LibMame_Get_Game_WorkingFlags(int gamenum);


/**
 * Returns the set of flags describing the game's supported orientations.
 * This is an or'd together set of flags from the LIBMAME_ORIENTATIONFLAGS_XXX
 * symbols.
 *
 * @param gamenum is the game number of the game
 * @return the set of flags describing the game's supported orientations.
 **/
int LibMame_Get_Game_OrientationFlags(int gamenum);


/**
 * Returns the type of the game's original screen.
 *
 * @param gamenum is the game number of the game
 * @return the type of the game's original screen.
 **/
LibMame_ScreenType LibMame_Get_Game_ScreenType(int gamenum);


/**
 * Returns the original resolution of the game.  If the game is a vector
 * game, then this is an undefined value.
 *
 * @param gamenum is the game number of the game
 * @return the original resolution of the game.
 **/
LibMame_ScreenResolution LibMame_Get_Game_ScreenResolution(int gamenum);


/**
 * Returns the original screen refresh rate, in Hz, of the game.
 *
 * @param gamenum is the game number of the game
 * @return the original screen refresh rate, in Hz, of the game.
 **/
int LibMame_Get_Game_ScreenRefreshRateHz(int gamenum);


/**
 * Returns the number of channels of sound that the game supports:
 * 0 means no sound, 1 means mono sound, 2 means stereo sound, anything higher
 * than 2 means surround sound.
 *
 * @param gamenum is the game number of the game
 * @return the number of channels of sound that the game supports
 **/
int LibMame_Get_Game_SoundChannels(int gamenum);


/**
 * Returns the number of sound samples this game uses.
 * 
 * @param gamenum is the game number of the game
 * @return the number of sound samples this game uses.
 **/
int LibMame_Get_Game_SoundSamples_Count(int gamenum);


/**
 * Returns the game number of the game that has the actual sample files for
 * this game.  Some games reference sound sample files from other games, and
 * thus use a different source game number than their own game number.
 *
 * @param gamenum is the game number of the game
 * @return the game number of the game that has the actual sample files for
 *         this game
 **/
int LibMame_Get_Game_SoundSamplesSource(int gamenum);


/**
 * Returns true if the sound samples are identical to those of the the
 * source, false if not
 *
 * @param gamenum is the game number of the game
 * @return true if the sound samples are identical to those of the the
 *         source, false if not
 **/
bool LibMame_Get_Game_SoundSamplesIdenticalToSource(int gamenum);


/**
 * Returns the file name of a sound sample for a game.
 *
 * @param gamenum is the game number of the game
 * @param samplenum is the sample number of the sample
 * @return the file name of a sound sample for a game.
 **/
const char *LibMame_Get_Game_SoundSampleFileName(int gamenum, int samplenum);


/**
 * Returns the number of chips that MAME emulates for a given game.
 *
 * @param gamenum is the game number of the game
 * @return the number of chips that MAME emulates for a given game.
 **/
int LibMame_Get_Game_Chip_Count(int gamenum);


/**
 * Returns a description of a chip that MAME emulates for a given game.
 *
 * @param gamenum is the game number of the game
 * @param chipnum is the chip number of the chip
 * @return a description of a chip that MAME emulates for a given game.
 **/
LibMame_ChipDescriptor LibMame_Get_Game_Chip(int gamenum, int chipnum);


/**
 * Returns the number of settings that MAME supports for a given game.
 *
 * @param gamenum is the game number of the game
 * @return the number of settings that MAME supports for a given game.
 **/
int LibMame_Get_Game_Setting_Count(int gamenum);


/**
 * Returns a description of a setting that MAME supports for a given game.
 *
 * @param gamenum is the game number of the game
 * @param settingnum is the setting number of the setting
 * @return a description of a setting that MAME supports for a given game.
 **/
LibMame_SettingDescriptor LibMame_Get_Game_Setting(int gamenum, int settingnum);


/**
 * Returns the maximum number of simultaneous players of a game.  Each player
 * is assumed to have an identical controllers as the others.  MAME supports a
 * maximum of 8 simultaneous players so this value will always range from 1 to
 * 8.
 *
 * @param gamenum is the game number of the game
 * @return the maximum number of simultaneous players of a game
 **/
int LibMame_Get_Game_MaxSimultaneousPlayers(int gamenum);


/**
 * This returns a structure describing the controllers for a given game that
 * are unique to each player.  All players are assumed to have the same
 * controllers.
 *
 * @param gamenum is the game number of the game
 * @return a structure describing player controllers for a given game.
 **/
LibMame_PerPlayerControllersDescriptor LibMame_Get_Game_PerPlayerControllers
    (int gamenum);


/**
 * This returns a structure describing the controllers for a given game that
 * are shared across all players.
 *
 * @param gamenum is the game number of the game
 * @return a structure describing the shared controllers for a given game.
 **/
LibMame_SharedControllersDescriptor LibMame_Get_Game_SharedControllers
    (int gamenum);


/**
 * Returns the name of the MAME source file that implements the game.
 *
 * @param gamenum is the game number of the game
 * @return the name of the MAME source file that implements the game.
 **/
const char *LibMame_Get_Game_SourceFileName(int gamenum);


/**
 * TODO: Functions for getting descriptions of all of the ROMs that a
 * game needs (possibly also for specially identifying BIOS ROMs)
 **/

LibMame_RunGameStatus LibMame_RunGame(int gamenum,
                                      /* Some options thing xxx, */
                                      const LibMame_RunGameCallbacks *cbs,
                                      void *callback_data);


#endif /* __LIBMAME_H__ */
