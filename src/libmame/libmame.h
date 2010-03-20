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


/**
 * These flags define all of the possible controller types
 **/
#define LIBMAME_CONTROLLERFLAGS_JOYSTICKHORIZONTAL              0x0001
#define LIBMAME_CONTROLLERFLAGS_JOYSTICKVERTICAL                0x0002
#define LIBMAME_CONTROLLERFLAGS_JOYSTICK4WAY                    0x0004
#define LIBMAME_CONTROLLERFLAGS_JOYSTICK8WAY                    0x0008
#define LIBMAME_CONTROLLERFLAGS_DOUBLEJOYSTICKHORIZONTAL        0x0010
#define LIBMAME_CONTROLLERFLAGS_DOUBLEJOYSTICKVERTICAL          0x0020
#define LIBMAME_CONTROLLERFLAGS_DOUBLEJOYSTICK4WAY              0x0040
#define LIBMAME_CONTROLLERFLAGS_DOUBLEJOYSTICK8WAY              0x0800
#define LIBMAME_CONTROLLERFLAGS_JOYSTICKANALOG                  0x0100
#define LIBMAME_CONTROLLERFLAGS_SPINNER                         0x0200
#define LIBMAME_CONTROLLERFLAGS_PADDLE                          0x0400
#define LIBMAME_CONTROLLERFLAGS_TRACKBALL                       0x0800
#define LIBMAME_CONTROLLERFLAGS_LIGHTGUN                        0x1000
#define LIBMAME_CONTROLLERFLAGS_PEDAL                           0x2000
#define LIBMAME_CONTROLLERFLAGS_PEDAL2                          0x4000
#define LIBMAME_CONTROLLERFLAGS_PEDAL3                          0x8000

/**
 * These flags define all of the possible general purpose buttons.
 **/
#define LIBMAME_CONTROLLERFLAGS_BUTTON_1                        0x0001
#define LIBMAME_CONTROLLERFLAGS_BUTTON_2                        0x0002
#define LIBMAME_CONTROLLERFLAGS_BUTTON_3                        0x0004
#define LIBMAME_CONTROLLERFLAGS_BUTTON_4                        0x0008
#define LIBMAME_CONTROLLERFLAGS_BUTTON_5                        0x0010
#define LIBMAME_CONTROLLERFLAGS_BUTTON_6                        0x0020
#define LIBMAME_CONTROLLERFLAGS_BUTTON_7                        0x0040
#define LIBMAME_CONTROLLERFLAGS_BUTTON_8                        0x0080
#define LIBMAME_CONTROLLERFLAGS_BUTTON_9                        0x0100
#define LIBMAME_CONTROLLERFLAGS_BUTTON_10                       0x0200
#define LIBMAME_CONTROLLERFLAGS_BUTTON_11                       0x0400
#define LIBMAME_CONTROLLERFLAGS_BUTTON_12                       0x0800
#define LIBMAME_CONTROLLERFLAGS_BUTTON_13                       0x1000
#define LIBMAME_CONTROLLERFLAGS_BUTTON_14                       0x2000
#define LIBMAME_CONTROLLERFLAGS_BUTTON_15                       0x4000
#define LIBMAME_CONTROLLERFLAGS_BUTTON_16                       0x8000

/**
 * These flags define all of the possible special controller buttons used for
 * Mahjong games.
 **/
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_A                       0x00000001
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_B                       0x00000002
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_C                       0x00000004
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_D                       0x00000008
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_E                       0x00000010
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_F                       0x00000020
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_G                       0x00000040
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_H                       0x00000080
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_I                       0x00000100
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_J                       0x00000200
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_K                       0x00000400
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_L                       0x00000800
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_M                       0x00001000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_N                       0x00002000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_O                       0x00004000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_P                       0x00008000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_Q                       0x00010000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_KAN                     0x00020000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_PON                     0x00040000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_CHI                     0x00080000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_REACH                   0x00100000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_RON                     0x00200000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_BET                     0x00400000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_LAST_CHANCE             0x00800000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_SCORE                   0x01000000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_DOUBLE_UP               0x02000000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_FLIP_FLOP               0x04000000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_BIG                     0x08000000
#define LIBMAME_CONTROLLERFLAGS_MAHJONG_SMALL                   0x10000000

/**
 * These flags define all of the possible controller buttons used for Hanafuda
 * games.
 **/
#define LIBMAME_CONTROLLERFLAGS_HANAFUDA_A                      0x001
#define LIBMAME_CONTROLLERFLAGS_HANAFUDA_B                      0x002
#define LIBMAME_CONTROLLERFLAGS_HANAFUDA_C                      0x004
#define LIBMAME_CONTROLLERFLAGS_HANAFUDA_D                      0x008
#define LIBMAME_CONTROLLERFLAGS_HANAFUDA_E                      0x010
#define LIBMAME_CONTROLLERFLAGS_HANAFUDA_F                      0x020
#define LIBMAME_CONTROLLERFLAGS_HANAFUDA_G                      0x040
#define LIBMAME_CONTROLLERFLAGS_HANAFUDA_H                      0x080
#define LIBMAME_CONTROLLERFLAGS_HANAFUDA_YES                    0x100
#define LIBMAME_CONTROLLERFLAGS_HANAFUDA_NO                     0x200

/**
 * These flags define all of the possible controller buttons used for gambling
 * games.
 **/
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_HIGH                   0x0000001
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_LOW                    0x0000002
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_HALF                   0x0000004
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_DEAL                   0x0000008
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_D_UP                   0x0000010
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_TAKE                   0x0000020
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_STAND                  0x0000040
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_BET                    0x0000080
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_KEYIN                  0x0000100
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_KEYOUT                 0x0000200
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_PAYOUT                 0x0000400
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_DOOR                   0x0000800
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_SERVICE                0x0001000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_BOOK                   0x0002000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_HOLD1                  0x0004000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_HOLD2                  0x0008000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_HOLD3                  0x0010000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_HOLD4                  0x0020000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_HOLD5                  0x0040000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_CANCEL                 0x0080000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_STOP1                  0x0100000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_STOP2                  0x0200000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_STOP3                  0x0400000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_STOP4                  0x0800000
#define LIBMAME_CONTROLLERFLAGS_GAMBLING_STOP_ALL               0x1000000

/**
 * These flags define all of the possible non-gameplay binary inputs
 **/
#define LIBMAME_CONTROLLERFLAGS_OTHER_COIN1                     0x00001
#define LIBMAME_CONTROLLERFLAGS_OTHER_COIN2                     0x00002
#define LIBMAME_CONTROLLERFLAGS_OTHER_COIN3                     0x00004
#define LIBMAME_CONTROLLERFLAGS_OTHER_COIN4                     0x00008
#define LIBMAME_CONTROLLERFLAGS_OTHER_COIN5                     0x00010
#define LIBMAME_CONTROLLERFLAGS_OTHER_COIN6                     0x00020
#define LIBMAME_CONTROLLERFLAGS_OTHER_COIN7                     0x00040
#define LIBMAME_CONTROLLERFLAGS_OTHER_COIN8                     0x00080
#define LIBMAME_CONTROLLERFLAGS_OTHER_BILL1                     0x00010
#define LIBMAME_CONTROLLERFLAGS_OTHER_START1                    0x00020
#define LIBMAME_CONTROLLERFLAGS_OTHER_START2                    0x00040
#define LIBMAME_CONTROLLERFLAGS_OTHER_START3                    0x00080
#define LIBMAME_CONTROLLERFLAGS_OTHER_START4                    0x00100
#define LIBMAME_CONTROLLERFLAGS_OTHER_START5                    0x00200
#define LIBMAME_CONTROLLERFLAGS_OTHER_START6                    0x00400
#define LIBMAME_CONTROLLERFLAGS_OTHER_START7                    0x00800
#define LIBMAME_CONTROLLERFLAGS_OTHER_START8                    0x01000
#define LIBMAME_CONTROLLERFLAGS_OTHER_SERVICE                   0x02000
#define LIBMAME_CONTROLLERFLAGS_OTHER_TILT                      0x04000
#define LIBMAME_CONTROLLERFLAGS_OTHER_INTERLOCK                 0x08000
#define LIBMAME_CONTROLLERFLAGS_OTHER_VOLUME_UP                 0x10000
#define LIBMAME_CONTROLLERFLAGS_OTHER_VOLUME_DOWN               0x20000

/**
 * These values define all of the possible UI inputs.
 **/
#define LIBMAME_CONTROLLERFLAGS_UI_CONFIGURE                    1
#define LIBMAME_CONTROLLERFLAGS_UI_ON_SCREEN_DISPLAY            2
#define LIBMAME_CONTROLLERFLAGS_UI_DEBUG_BREAK                  3
#define LIBMAME_CONTROLLERFLAGS_UI_PAUSE                        4
#define LIBMAME_CONTROLLERFLAGS_UI_RESET_MACHINE                5
#define LIBMAME_CONTROLLERFLAGS_UI_SOFT_RESET                   6
#define LIBMAME_CONTROLLERFLAGS_UI_SHOW_GFX                     7
#define LIBMAME_CONTROLLERFLAGS_UI_FRAMESKIP_DEC                8
#define LIBMAME_CONTROLLERFLAGS_UI_FRAMESKIP_INC                9
#define LIBMAME_CONTROLLERFLAGS_UI_THROTTLE                     10
#define LIBMAME_CONTROLLERFLAGS_UI_FAST_FORWARD                 11
#define LIBMAME_CONTROLLERFLAGS_UI_SHOW_FPS                     12
#define LIBMAME_CONTROLLERFLAGS_UI_SNAPSHOT                     13
#define LIBMAME_CONTROLLERFLAGS_UI_RECORD_MOVIE                 14
#define LIBMAME_CONTROLLERFLAGS_UI_TOGGLE_CHEAT                 15
#define LIBMAME_CONTROLLERFLAGS_UI_UP                           16
#define LIBMAME_CONTROLLERFLAGS_UI_DOWN                         17
#define LIBMAME_CONTROLLERFLAGS_UI_LEFT                         18
#define LIBMAME_CONTROLLERFLAGS_UI_RIGHT                        19
#define LIBMAME_CONTROLLERFLAGS_UI_HOME                         20
#define LIBMAME_CONTROLLERFLAGS_UI_END                          21
#define LIBMAME_CONTROLLERFLAGS_UI_PAGE_UP                      22
#define LIBMAME_CONTROLLERFLAGS_UI_PAGE_DOWN                    23
#define LIBMAME_CONTROLLERFLAGS_UI_SELECT                       24
#define LIBMAME_CONTROLLERFLAGS_UI_CANCEL                       25
#define LIBMAME_CONTROLLERFLAGS_UI_DISPLAY_COMMENT              26
#define LIBMAME_CONTROLLERFLAGS_UI_CLEAR                        27
#define LIBMAME_CONTROLLERFLAGS_UI_ZOOM_IN                      28
#define LIBMAME_CONTROLLERFLAGS_UI_ZOOM_OUT                     29
#define LIBMAME_CONTROLLERFLAGS_UI_PREV_GROUP                   30
#define LIBMAME_CONTROLLERFLAGS_UI_NEXT_GROUP                   31
#define LIBMAME_CONTROLLERFLAGS_UI_ROTATE                       32
#define LIBMAME_CONTROLLERFLAGS_UI_SHOW_PROFILER                33
#define LIBMAME_CONTROLLERFLAGS_UI_TOGGLE_UI                    34
#define LIBMAME_CONTROLLERFLAGS_UI_TOGGLE_DEBUG                 35
#define LIBMAME_CONTROLLERFLAGS_UI_PASTE                        36
#define LIBMAME_CONTROLLERFLAGS_UI_SAVE_STATE                   37
#define LIBMAME_CONTROLLERFLAGS_UI_LOAD_STATE                   38


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
typedef struct LibMame_ControllersDescriptor
{
    /**
     * These are all of the general purpose buttons which are present.
     **/
    int normal_button_flags;

    /**
     * These are the names of the normal buttons, or NULL if the button has
     * no name.
     **/
    const char *normal_button_names[16];

    /**
     * These flags identify which Mahjong buttons are present
     **/
    int mahjong_button_flags;

    /**
     * These flags identify which hanafuda buttons are present
     **/
    int hanafuda_button_flags;

    /**
     * These flags identify which gambling buttons are present
     **/
    int gambling_button_flags;

    /**
     * These flags identify which other binary controls are present
     **/
    int other_flags;

    /**
     * These flags identify which controllers are present
     **/
    int controller_flags;
} LibMame_ControllersDescriptor;


/**
 * This describes all of the controller values that can be polled by MAME.
 * One structure of this type for each player is passed to the xxx callback
 * function periodically while a game is running to poll the current input.
 * Not all values provided in this structure are used for every game.
 **/
typedef struct LibMame_ControllersState
{
    /**
     * These are the current states of each normal button; the flag for
     * a button being set here means that the button is currently pressed,
     * not being set means that the button is currently not pressed.  See
     * the LIBMAME_CONTROLLERFLAGS_BUTTON_XX constants.
     **/
    int normal_buttons_state;

    /**
     * These are the current states of each Mahjong button; the flag for
     * a button being set here means that the button is currently pressed,
     * not being set means that the button is currently not pressed.  See
     * the LIBMAME_CONTROLLERFLAGS_MAHJONG_XX constants.
     **/
    int mahjong_buttons_state;

    /**
     * These are the current states of each Hanafuda button; the flag for
     * a button being set here means that the button is currently pressed,
     * not being set means that the button is currently not pressed.  See
     * the LIBMAME_CONTROLLERFLAGS_HANAFUDA_XX constants.
     **/
    int hanafuda_buttons_state;
    
    /**
     * These are the current states of each Gambling button; the flag for
     * a button being set here means that the button is currently pressed,
     * not being set means that the button is currently not pressed.  See
     * the LIBMAME_CONTROLLERFLAGS_GAMBLING_XX constants.
     **/
    int gambling_buttons_state;
    
    /**
     * These are the current states of each other binary input; the flag for
     * an input being set here means that the input is currently triggered,
     * not being set means that the input is currently not triggered.  See
     * the LIBMAME_CONTROLLERFLAGS_OTHER_XX constants.
     **/
    int other_buttons_state;

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
     * mapped to a range from -32768 (full left) to 32767 (full right).
     **/
    int analog_joystick_horizontal_position_state;

    /**
     * This value is the current vertical position of the analog joystick,
     * mapped to a range from -32768 (full down) to 32767 (full up).
     **/
    int analog_joystick_vertical_position_state;

    /**
     * This value is the change in position of the spinner since the last
     * time it was polled, mapped to a range from -32768 (furthest possible
     * spin left) to 32767 (furthest possible spin right).
     **/
    int spinner_delta;

    /**
     * This value is the current paddle position, mapped to a range from
     * -32768 (full left) to 32767 (full right).
     **/
    int paddle_state;

    /**
     * This value is the change in position of the trackball along the
     * horizontal axis since the last time it was polled, mapped to a range
     * from -32768 (furthest possible roll left) 32767 (furthest possible roll
     * right).
     **/
    int trackball_horizontal_delta;

    /**
     * This value is the change in position of the trackball along the
     * vertical axis since the last time it was polled, mapped to a range
     * from -32768 (furthest possible roll down) 32767 (furthest possible roll
     * up).
     **/
    int trackball_vertical_delta;

    /**
     * This value is the current horizontal position of the analog joystick,
     * mapped to a range from -32767 (left edge of screen) to 32767
     * (right edge of screen).  The value -32768 indicates off of the screen
     * if present in either lightgun_horizontal_position_state or
     * lightgun_vertical_position_state.
     **/
    int lightgun_horizontal_position_state;

    /**
     * This value is the current vertical position of the analog joystick,
     * mapped to a range from -32767 (left edge of screen) to 32767
     * (right edge of screen).  The value -32768 indicates off of the screen
     * if present in either lightgun_horizontal_position_state or
     * lightgun_vertical_position_state.
     **/
    int lightgun_vertical_position_state;

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
     * This is the current UI input.  libmame allows only one UI input at a
     * time.  See the LIBMAME_CONTROLLERFLAGS_UI_XXX constants.
     **/
    int ui_input_state;
} LibMame_ControllersState;


typedef struct LibMame_RunGameCallbacks
{
    void (*PollControllersStates)(LibMame_ControllersState *controllers_states,
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
 * This returns the controllers describing the controllers for a given game.
 * All players are assumed to have the same controllers.
 *
 * @param gamenum is the game number of the game
 * @return the controllers describing the controllers for a given game.
 **/
LibMame_ControllersDescriptor LibMame_Get_Game_Controllers(int gamenum);


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
