/** **************************************************************************
 * libmame.h
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 * 
 ************************************************************************** **/


#ifndef __LIBMAME_H__
#define __LIBMAME_H__

#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#if 0 // fix emacs indentation
}
#endif
#endif

/*! \file libmame.h
 *
 * LibMame provides a single API for (almost) all functionality of the MAME
 * engine.  It provides the following types of functions:
 *
 * 1. Functions for querying the set of games supported by this version of
 *    LibMame, and many descriptive details about each game:
 *    - LibMame_Get_Version_String
 *    - LibMame_Get_Game_Count
 *    - LibMame_Get_Game_Number
 *    - LibMame_Get_Game_Matches
 *    - LibMame_Get_Game_Short_Name
 *    - LibMame_Get_Game_Full_Name
 *    - LibMame_Get_Game_Year_Of_Release
 *    - LibMame_Get_Game_CloneOf
 *    - LibMame_Get_Game_Manufacturer
 *    - LibMame_Get_Game_WorkingFlags
 *    - LibMame_Get_Game_OrientationFlags
 *    - LibMame_Get_Game_ScreenType
 *    - LibMame_Get_Game_ScreenResolution
 *    - LibMame_Get_Game_ScreenRefreshRateHz
 *    - LibMame_Get_Game_SoundChannels
 *    - LibMame_Get_Game_SoundSamples_Count
 *    - LibMame_Get_Game_SoundSamplesSource
 *    - LibMame_Get_Game_SoundSampleFileName
 *    - LibMame_Get_Game_Chip_Count
 *    - LibMame_Get_Game_Chip
 *    - LibMame_Get_Game_Setting_Count
 *    - LibMame_Get_Game_Setting
 *    - LibMame_Get_Game_MaxSimultaneousPlayers
 *    - LibMame_Get_Game_AllControllers
 *    - LibMame_Get_Game_BiosSet_Count
 *    - LibMame_Get_Game_BiosSet
 *    - LibMame_Get_Game_Rom_Count
 *    - LibMame_Get_Game_Rom
 *    - LibMame_Get_Game_Hdd_Count
 *    - LibMame_Get_Game_Hdd
 *    - LibMame_Get_Game_SourceFileName
 *
 * 2. A function for running a MAME game (only one MAME game may be run
 *    concurrently within the same process due to limitations in the MAME
 *    engine).  MAME will emulate the game and will interact with the rest
 *    of the system (for displaying frames of the game, playing sound, and
 *    getting controller input) via callbacks.
 *    - LibMame_RunGame
 *
 * 3. Functions for manipulating a running MAME game, including pausing,
 *    resetting, and manipulating configuration values of various kinds.
 *    - LibMame_RunningGame_Pause
 *    - LibMame_RunningGame_Schedule_Exit
 *    - LibMame_RunningGame_Schedule_Hard_Reset
 *    - LibMame_RunningGame_Schedule_Soft_Reset
 *    - LibMame_RunningGame_SaveState
 *    - LibMame_RunningGame_LoadState
 *    - LibMame_RunningGame_ChangeConfigurationValue
 *    - LibMame_RunningGame_ChangeDipswitchValue
 *    - LibMame_RunningGame_ChangeAdjusterValue
 *
 * 4. Miscellaneous functions necessary for supporting the other libmame
 *    functionality:
 *    - LibMame_Initialize
 *    - LibMame_Deinitialize
 *    - LibMame_Set_Default_RunGameOptions
 *
 *
 * In general, applications using libmame will follow this pattern:
 *
 * 1. Call LibMame_Initialize.
 *
 * 2. Collect game information for display using the LibMame_Get_Game_XXX
 *    functions.
 *
 * 3. Look up the number of the game that the user wants to play using
 *    LibMame_Get_Game_Number.
 *
 * 4. Set options for running the game by first calling 
 *    LibMame_Set_Default_RunGameOptions, and then customizing the resulting
 *    options according to user preference.
 *
 * 5. Set up internal state for managing the display, sound, and controller
 *    input for the game about to be run.
 *    
 *    The target display frame rate can be found by calling 
 *    LibMame_Get_Game_ScreenRefreshRateHz, if needed, and the size at which
 *    the game will be rendered is known by calling
 *    LibMame_Get_Game_ScreenResolution (the application may scale the
 *    resulting graphics and apply whatever effects it wants to to the
 *    results, but the display callback itself will be made with rendering
 *    primitives set to display at the game's native resolution).
 *
 *    The set of controllers that the game will require inputs on can be
 *    found by calling LibMame_Get_Game_AllControllers (and the maximum number
 *    of players for whom controller input is needed is available from
 *    LibMame_Get_Game_MaxSimultaneousPlayers).  The application will
 *    typically use this information to decide how to map whatever controllers
 *    it knows about to the inputs that the game is expecting, usually via
 *    user preference (managed by the application).
 *
 * 6. Call LibMame_RunGame to run the game, passing in pointers to callback
 *    functions that will handle the rendering of audio and video and
 *    collecting controller input.
 *
 * 7. As the game is running, according to user input, call the
 *    LibMame_RunningGame_XXX functions (but only from the 
 *    MakeRunningGameCallbacks (or in certain cases, Paused) callbacks, which
 *    means that user commands must typically be noted and then applied at the
 *    next call to MakeRunningGameCallbacks).
 *
 * 8. When the game completes (i.e. the LibMame_RunningGame_Schedule_Exit
 *    callback is made by the application), LibMame_RunGame will return, and
 *    the application may then repeat steps 5, 6, and 7 if it wishes.
 *
 * 9. When the application is completely finished calling any LibMame
 *    functions, before it exits, it must then call LibMame_Deinitialize.
 *
 * Please keep in mind that although the API is structured as if it were
 * possible to run multiple MAME games simultaneously from the same
 * application, there is a limit in the MAME engine that prevents more than
 * one game from running at a time.  This limit may someday be lifted, in
 * which case this API will already be ready to take advantage of that, but
 * until the limit is lifted, remember that LibMame_RunGame may only be called
 * from one thread at a time within a single application.
 */


/** **************************************************************************
 * Opaque type definitions
 ************************************************************************** **/

typedef struct LibMame_RunningGame LibMame_RunningGame;


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
 * These flags define the screen orientation of the game as it was originally
 * displayed.
 **/

/**
 * These macros can be used to obtain values from the
 * LibMame_RenderPrimitive structure's flags field.
 **/

/**
 * Calculates the texture orientation of a LibMame_RenderPrimitive's flags
 **/
#define LIBMAME_RENDERFLAGS_TEXTURE_ORIENTATION(flags) (((flags) >>  0) & 0xF))

/**
 * Calculates the texture format of a LibMame_RenderPrimitive's flags
 **/
#define LIBMAME_RENDERFLAGS_TEXTURE_FORMAT(flags)      (((flags) >>  8) & 0xF))

/**
 * Calculates the blend mode of a LibMame_RenderPrimitive's flags
 **/
#define LIBMAME_RENDERFLAGS_BLEND_MODE(flags)          (((flags) >> 12) & 0xF))

/**
 * Calculates the antialias value of a LibMame_RenderPrimitive's flags; zero
 * means no antialiasing, nonzero means antialiasing
 **/
#define LIBMAME_RENDERFLAGS_ANTIALIAS(flags)                   ((flags) & 0x10)

/**
 * Calculates the screen texture value of a LibMame_RenderPrimitive's flags;
 * zero means ???, nonzero means ???
 **/
#define LIBMAME_RENDERFLAGS_SCREEN_TEXTURE(flags)              ((flags) & 0x20)

/**
 * Calculates the texture wrap of a LibMame_RenderPrimitive's flags;
 * zero means ???, nonzero means ???
 **/
#define LIBMAME_RENDERFLAGS_TEXTURE_WRAP(flags)                ((flags) & 0x40)


/** **************************************************************************
 * Enumerated types
 ************************************************************************** **/

/**
 * Status codes that can be returned by LibMame_Initialize()
 **/
typedef enum
{
    LibMame_InitializeStatus_Success,
    LibMame_InitializeStatus_OutOfMemory,
    LibMame_InitializeStatus_GeneralFailure
} LibMame_InitializeStatus;


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
 * Types of orientation for games; this is the orientation of the screen
 * relative to the displayed image.
 **/
typedef enum
{
    /**
     * Normal orientation
     **/
    LibMame_OrientationType_Normal,
    /**
     * 90 degrees rotation
     **/
    LibMame_OrientationType_90,
    /**
     * 180 degrees rotation
     **/
    LibMame_OrientationType_180,
    /**
     * 270 degrees rotation
     **/
    LibMame_OrientationType_270
} LibMame_OrientationType;


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
 * These are the possible ROM/HDD image status values.
 **/
typedef enum LibMame_ImageStatus_Status
{
    /**
     * The image is good (true to the original game)
     **/
    LibMame_ImageStatus_GoodDump,
    /**
     * The image is bad, but is the best available image
     **/
    LibMame_ImageStatus_BadDump,
    /**
     * There is no image available for this ROM/HDD image
     **/
    LibMame_ImageStatus_NoDump
} LibMame_ImageStatus;


/**
 * The possible types of render primitives passed into the update display
 * callback
 **/
typedef enum
{
    LibMame_RenderPrimitiveType_Line,
    LibMame_RenderPrimitiveType_Quad
} LibMame_RenderPrimitiveType;


/**
 * The possible startup stats that libmame can report about as it is starting
 * up a game.
 **/
typedef enum
{
    /**
     * MAME is loading ROMs.  This happens before machine initialization.
     **/
    LibMame_StartupPhase_LoadingRoms,
    /**
     * MAME is intializing internal machine state in preparation for running
     * a game.
     **/
    LibMame_StartupPhase_InitializingMachine
} LibMame_StartupPhase;


/**
 * Status codes that can be returned by LibMame_RunGame()
 **/
typedef enum
{
    LibMame_RunGameStatus_Success,
    LibMame_RunGameStatus_InvalidGameNum,
    LibMame_RunGameStatus_FailedValidityCheck,
    LibMame_RunGameStatus_MissingFiles,
    LibMame_RunGameStatus_NoSuchGame,
    LibMame_RunGameStatus_InvalidConfig,
    LibMame_RunGameStatus_GeneralError
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
typedef struct LibMame_SoundSample
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
} LibMame_SoundSample;


/**
 * This describes a chip that MAME emulates.
 **/
typedef struct LibMame_Chip
{
    /**
     * This is nonzero if the chip is a sound chip, zero if it is a CPU chip
     **/
    int is_sound;

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
} LibMame_Chip;


/**
 * This describes a setting that MAME allows to be adjusted for a game.
 **/
typedef struct LibMame_Setting
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
     * This is the mask of the setting, which is only relevent for identifying
     * the setting in calls to LibMame_RunningGame_ChangeConfigurationValue,
     * LibMame_RunningGame_ChangeDipswitchValue, or
     * LibMame_RunningGame_ChangeAdjusterValue
     **/
    uint32_t mask;

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
} LibMame_Setting;


/**
 * This describes a player's control inputs for a game.  This is a set of
 * individual controllers comprising the entire set of control inputs that a
 * player would use to play a game.  All players are assumed to use the same
 * type and number of controls during a multiplayer game.
 **/
typedef struct LibMame_PerPlayerControllers
{
    /**
     * These are all of the general purpose buttons which are present,
     * each is indicated in this as (1 << LibMame_NormalButtonType_XXX).
     **/
    int normal_button_flags;

    /**
     * These are the names of the normal buttons, or NULL if the button has
     * no name, one per button type.
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
} LibMame_PerPlayerControllers;


/**
 * This describes the shared controller inputs for a game.  This is a set of
 * individual controllers comprising the entire set of control inputs that
 * a game has that are not active play controllers.  All players share these
 * controllers.
 **/
typedef struct LibMame_SharedControllers
{
    /**
     * These flags identify which other binary controls are present,
     * each is indicated in this as (1 << LibMame_OtherButtonType_XXX).
     **/
    int other_button_flags;
} LibMame_SharedControllers;


/**
 * This describes all of the controller inputs for a game.
 **/
typedef struct LibMame_AllControllers
{
    /**
     * This is the per-player controllers descriptors; each player is assumed
     * to have the same controllers, so this describes the controller set that
     * every player uses for this game
     **/
    LibMame_PerPlayerControllers per_player;

    /**
     * This is the shared controllers descriptor
     **/
    LibMame_SharedControllers shared;
} LibMame_AllControllers;


/**
 * This structure defines a BIOS set, which is the set of ROMS defining the
 * BIOS of a game that may be loaded instead of other BIOS sets for the game.
 * If a game lists multiple BIOS sets, then any one of them may be loaded for
 * the game to produce different behaviors depending on the BIOS set.
 **/
typedef struct LibMame_BiosSet
{
    /**
     * This is the short name of the BIOS set
     **/
    const char *name;

    /**
     * This is a description of the BIOS set, which should given an indication
     * of what the BIOS does
     **/
    const char *description;
    
    /**
     * If this is nonzero then this BIOS set is the default for the game; if
     * this is zero then this BIOS set is not the default for the game
     **/
    int is_default;

    /**
     * This is the number of ROM images included in this BIOS set
     **/
    int rom_count;

    /**
     * These are the numbers of the game's ROMs that make up this BIOS set.
     * To get the details of rom N, call LibMame_Get_Game_Image(gamenum, N).
     **/
    const int *rom_numbers;
} LibMame_BiosSet;


/**
 * This structure defines a single image (ROM or HDD) that a game uses.
 **/
typedef struct LibMame_Image
{
    /**
     * This is the name of the image.
     **/
    const char *name;

    /**
     * This is the status of this ROM/HDD image, which indicates whether or
     * not the image is "good" (true to the original), or "bad" (but the best
     * available), or "no dump", which means that this structure identifies an
     * image which is known to be needed by the game but is there is no good
     * image available.
     **/
    LibMame_ImageStatus status;

    /**
     * If this is nonzero, this image is optional for functioning of the
     * game.  If this is zero, this game is required for the game to run.
     **/
    int is_optional;

    /**
     * This is the exact size, in bytes, of the image, if known, or zero if
     * not
     **/
    int size_if_known;

    /**
     * If the game that this image is for is a clone of another game, and the
     * image itself is shared with that game, rather than duplicated for this
     * game, then this is the name of the cloned game and image as it exists
     * in the cloned game.  Note that the name of the game may be one that is
     * not reported directly by libmame, as MAME calls some rom sets that hold
     * only BIOS files "games" and libmame filters those out of the list of
     * games that it presents.
     *
     * If this value is zero, then this image is specific to this game
     * and not shared with a parent game.
     **/
    const char *clone_of_game;
    const char *clone_of_rom;

    /**
     * If this is non-NULL, it provides a CRC value to be used to validate the
     * image.
     **/
    const char *crc;

    /**
     * If this is non-NULL, it provides a SHA-1 hash value to be used to
     * validate the image.
     **/
    const char *sha1;

    /**
     * If this is non-NULL, it provides a MD5 hash value to be used to
     * validate the image.
     **/
    const char *md5;
} LibMame_Image;


/**
 * This describes all of the controller values that can be polled by MAME for
 * individual players.  One structure of this type for each player is included
 * in the LibMame_AllControllersState structure.  Not all values provided in
 * this structure are used for every game; see the
 * LibMame_Get_Game_AllControllers() function for a way to get a description
 * of the set of controllers actually used by the currently running game.
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
     * -65536 (not pressed) to 65536 (fully pressed).
     **/
    int pedal_state;

    /**
     * This value is the current second pedal position, mapped to a range from
     * -65536 (not pressed) to 65536 (fully pressed).
     **/
    int pedal2_state;

    /**
     * This value is the current third pedal position, mapped to a range from
     * -65536 (not pressed) to 65536 (fully pressed).
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

    /**
     * This is the current UI input.  libmame allows only one UI input at a
     * time.  Its value is one of the LibMame_UiButtonType_XXX values.
     **/
    int ui_input_state;
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


/**
 * These are the configuration options for LibMame_RunGame.  Defaults can
 * be loaded by calling LibMame_Set_Default_RunGameOptions, and then
 * customizations done before calling LibMame_RunGame with the resulting
 * options.
 **/
typedef struct LibMame_RunGameOptions
{
    /* search paths for files of specific types; of the form DIR1;DIR2;...
       These files are only read (not written) by MAME--------------------- */

    /** path to ROMsets and hard disk images **/
    char rom_path[1024];
    /** path to samplesets **/
    char sample_path[1024];
    /** path to artwork files **/
    char art_path[1024];
    /** path to controller definitions **/
    char ctrl_path[1024];
    /** path to ini files **/
    char ini_path[1024];
    /** path to font files **/
    char font_path[1024];
    /** path to cheat files **/
    char cheat_path[1024];
    /** path to crosshair files **/
    char crosshair_path[1024];

    /* single directories for files of specific types.  These files are read
       and written by MAME ------------------------------------------------ */

    /** directory to save nvram contents **/
    char nvram_directory[256];
    /** directory to save memory card contents **/
    char memcard_directory[256];
    /** directory to save input device logs **/
    char input_directory[256];
    /** directory to save states **/
    char state_directory[256];
    /** directory to save screenshots **/
    char snapshot_directory[256];
    /** directory to save hard drive image difference files **/
    char diff_directory[256];
    /** directory to save debugger comments **/
    char comment_directory[256];

    /* state/playback options --------------------------------------------- */

    /** saved state to load **/
    char state[1024]; 
    /** enable automatic save/restore at exit/startup **/
    int autosave; 
    /** playback input file name **/
    char playback_file[256];
    /** record output file name **/
    char record_file[256];
    /** filename to write MNG movie of current game **/
    char mngwrite_file[256];
    /** filename to write AVI movie of current game **/
    char aviwrite_file[256];
    /** filename to write WAV file of current game **/
    char wavwrite_file[256];
    /** specify snapshot/movie resolution (<width>x<height>) or 'auto' to
        use minimal size **/
    char snapsize[16];
    /** specify snapshot/movie view or 'internal' to use internal pixel-aspect
        views **/
    char snapview[64];
    /** create burn-in snapshots for each screen **/
    int burnin;
    
    /* core performance options ------------------------------------------- */

    /** enable automatic frameskip selection **/
    int auto_frame_skip;
    /** set frameskip to fixed value, 0-10 (autoframeskip must be disabled) **/
    int frame_skip_level;
    /** enable throttling to keep game running in sync with real time **/
    int throttle;
    /** enable sleeping, which gives time back to other applications when
        idle **/
    int sleep;
    /** controls the speed of gameplay, relative to realtime; smaller numbers
        are slower **/
    float speed_multiplier;
    /** automatically adjusts the speed of gameplay to keep the refresh rate
        lower than the screen **/
    int auto_refresh_speed;

    /* core rotation/flip options ----------------------------------------- */

    /** rotate the game screen according to the game's orientation needs it **/
    int rotate;
    /** rotate screen clockwise 90 degrees **/
    int rotate_right;
    /** rotate screen counterclockwise 90 degrees **/
    int rotate_left;
    /** automatically rotate screen clockwise 90 degrees if vertical **/
    int auto_rotate_right;
    /** automatically rotate screen counterclockwise 90 degrees if vertical **/
    int auto_rotate_left;
    /** flip screen left-right **/
    int flip_x;
    /** flip screen upside-down **/
    int flip_y;

    /* core artwork options ----------------------------------------------- */

    /** crop artwork to game screen size **/
    int crop_artwork;
    /** enable backdrops if artwork is enabled and available **/
    int use_backdrops;
    /** enable overlays if artwork is enabled and available **/
    int use_overlays;
    /** enable bezels if artwork is enabled and available **/
    int use_bezels;

    /* core screen options ------------------------------------------------ */

    /** default game screen brightness correction (0.1 - 2.0) **/
    float brightness;
    /** default game screen contrast correction (0.1 - 2.0) **/
    float contrast;
    /** default game screen gamma correction (0.1 - 3.0) **/
    float gamma;
    /** amount to scale the screen brightness when paused (0.0 - 1.0) **/
    float pause_brightness;

    /* core vector options ------------------------------------------------ */

    /** use antialiasing when drawing vectors **/
    int vector_antialias;
    /** set vector beam width **/
    float vector_beam;
    /** set vector flicker effect **/
    float vector_flicker;

    /* core sound options ------------------------------------------------- */

    /** enable sound output **/
    int sound;
    /** set sound output sample rate **/
    int sample_rate;
    /** enable the use of external samples if available **/
    int use_samples;
    /** sound volume reduction in decibels (-32 min, 0 max) **/
    int volume_attenuation;

    /* core input options ------------------------------------------------- */

    /** enable coin lockouts to actually lock out coins **/
    int coin_lockout;
    /** explicit joystick map, or auto to auto-select **/
    char joystick_map[32];
    /** center deadzone range for joystick where change is ignored (0.0 center,
        1.0 end) **/
    float joystick_deadzone;
    /** end of axis saturation range for joystick where change is ignored (0.0
        center, 1.0 end) **/
    float joystick_saturation;
    /** enable steadykey support **/
    int enable_steadykey;
    /** specifies whether to use a natural keyboard or not **/
    int enable_natural_keyboard;
    /** convert lightgun button 2 into offscreen reload **/
    int enable_lightgun_offscreen_reload;

    /* core debugging options --------------------------------------------- */

    /** display additional diagnostic information **/
    int verbose_output;
    /** generate an error.log file **/
    int emit_log;
    /** enable/disable debugger **/
    int debug;
    /** use the internal debugger for debugging **/
    int debug_internal;
    /** script for debugger **/
    int debugscript;
    /** keep calling video updates while in pause **/
    int update_in_pause;

    /* core misc options -------------------------------------------------- */

    /** select a special system BIOS to use **/
    char special_bios[64];
    /** enable cheat subsystem **/
    int enable_cheats;
    /** skip displaying the information screen at startup **/
    int skip_gameinfo_screens;

} LibMame_RunGameOptions;


/**
 * This is the type of a render primitive passed to the update video callback
 * by libmame.  It is one element of a list of render primitives, all of which
 * must be rendered in order to produce a complete frame of video for the
 * running game.
 **/
typedef struct LibMame_RenderPrimitive
{
    /**
     * This is the next render primitive in this list, or NULL if there are no
     * more render primitives after this one
     **/
    struct LibMame_RenderPrimitive *next;

    /**
     * This is the type of this render primitive (vector or raster)
     **/
    LibMame_RenderPrimitiveType type;

    /**
     * This is the bounds within the display to draw this render primitive at; 
     * if this is a vector primitive, this gives the endpoints of the vector.
     * If this is a raster primitive, this gives the bounds of the rectangle
     * into which the texture should be rendered
     **/
    struct {
        float x0;   /* leftmost X coordinate */
        float y0;   /* topmost Y coordinate */
        float x1;   /* rightmost X coordinate */
        float y1;   /* bottommost Y coordinate */
    } bounds;

    /**
     * This is the color for the primitive (relevent only to vector, or also
     * to raster?)
     **/
    struct {
        float a;    /* alpha component (0.0 = transparent, 1.0 = opaque) */
        float r;    /* red component (0.0 = none, 1.0 = max) */
        float g;    /* green component (0.0 = none, 1.0 = max) */
        float b;    /* blue component (0.0 = none, 1.0 = max) */
    } color;

    /**
     * These are flags describing render parameters; see the 
     * LIBMAME_RENDERFLAGS_XXX macros
     **/
    uint32_t flags;

    /**
     * This is the width of the line to render, only relevent for vector
     * primitives
     **/
    float width;

    /**
     * This is the texture to render, only relevent for raster primitives
     **/
    struct {
        /**
         * This is the pointer to the beginning of the block of memory holding
         * the texture data for this primitive.  This will likely stay
         * consistent across multiple renderings of the same texture and
         * caching in the display is advised.
         **/
        void *base;           /* base of the data */

        /**
         * What is this?
         **/
        uint32_t rowpixels;   /* pixels per row */

        /**
         * Width of the image
         **/
        uint32_t width;

        /**
         * Height of the image
         **/
        uint32_t height;      /* height of the image */

        /**
         * Palette for PALETTE16 textures.  LUTs for RGB15/RGB32.  Not sure
         * what this means.
         **/
        const uint32_t *palette;

        /**
         * Sequence number.  Not sure what this is for.  Is it an animation
         * sequence number for animations composed of a sequence of tiles?
         **/
        uint32_t seqid;
    } texture;

    /**
     * Not sure what the following is
     **/
    struct 
    {
        /**
         * Top left UV coordinate
         **/
        struct {
            float u, v;
        } top_left;

        /**
         * Top right UV coordinate
         **/
        struct {
            float u, v;
        } top_right;

        /**
         * Bottom left UV coordinate
         **/
        struct {
            float u, v;
        } bottom_left;

        /**
         * Bottom right UV coordinate
         **/
        struct {
            float u, v;
        } bottom_right;
    } quad_texuv;
} LibMame_RenderPrimitive;


/**
 * This is the set of callbacks that the caller of LibMame_RunGame passes in.
 * These provide the interface to allow MAME to supply the frames of video and
 * audio that constitute a running game, as well as allowing MAME to gather
 * input from players of the game, and to provide a mechanism for altering the
 * game state as it is running
 **/
typedef struct LibMame_RunGameCallbacks
{
    /**
     * Called by libmame whenever there is textual status reported by a
     * running game.  In general, this text may be ignored, but some
     * applications may prefer to show this text to the user.
     *
     * @param format is the format text, as in vsprintf
     * @param args are the args, as in vsprintf
     * @param callback_data the data pointer that was passed to
     *        LibMame_RunGame
     **/
    void (*StatusText)(const char *format, va_list args, void *callback_data);

    /**
     * Called by libmame as LibMame_RunGame initializes the game and prepares
     * it for being run.  This includes the loading of ROMs and initializing of
     * internal machine state.  Additionally lets the callbacks know the
     * handle of the running game, so that it may be used for the 
     * LibMame_RunningGame_XXX calls for this game.
     *
     * @param phase is either LibMame_StartupPhase_LoadingRoms, or
     *        LibMame_StartupPhase_InitializingMachine
     * @param pct_complete is the percentage completion of this phase
     * @param running_game is an opaque pointer which identifies the game
     *        being run, and that is used in subsequent calls to
     *        LibMame_RunningGame_XXX functions.  The application should, if
     *        it intends to run more than one game simultaneously (currently
     *        not possible due to internal MAME limitations), save this value
     *        in a way that associates it with the callback data.
     * @param callback_data the data pointer that was passed to
     *        LibMame_RunGame
     **/
    void (*StartingUp)(LibMame_StartupPhase phase, int pct_complete,
                       LibMame_RunningGame *running_game,
                       void *callback_data);

    /**
     * Called by libmame periodically to poll the current state of all
     * controllers.  The supplied callback should expect the all_states
     * structure to be completely zeroed out when this function is called, and
     * should set all relevent states within the structure.  Relevent states
     * are those used by the game being played (which can be determined by
     * calling LibMame_Get_Game_AllControllers()), for each player which
     * could be playing the game (which can be determined by calling
     * LibMame_Get_Game_MaxSimultaneousPlayers()), plus all states from the
     * shared states structure.
     *
     * @param all_states is all possible controller states; the callback
     *        should set the current state of relevent controllers
     * @param callback_data the data pointer that was passed to
     *        LibMame_RunGame
     **/
    void (*PollAllControllersState)(LibMame_AllControllersState *all_states,
                                    void *callback_data);

    /**
     * Called by libmame to periodically (and regularly, at the original
     * frame rate of the game) provide the primitives that need to be rendered
     * to display the current frame of the game.
     *
     * @param render_primitive_list is a list of primitives that are to be
     *        rendered
     * @param callback_data the data pointer that was passed to
     *        LibMame_RunGame
     **/
    void (*UpdateVideo)(const LibMame_RenderPrimitive *render_primitive_list,
                        void *callback_data);

    /**
     * Called by libmame to periodically (and regularly, at the original
     * frame rate of the game) provide the audio that need to be output for
     * the current frame of the game.
     *
     * @param buffer is a pointer to the raw sound data (more documentation
     *        needed about the format of this)
     * @param samples_this_frame is the number of samples contained in
     *        the buffer (?? more docs needed)
     * @param callback_data the data pointer that was passed to
     *        LibMame_RunGame
     **/
    void (*UpdateAudio)(int16_t *buffer, int samples_this_frame,
                        void *callback_data);

    /**
     * Called by libmame to alter the current volume of audio output.
     *
     * Attenuation is the attenuation in dB (a negative number). To convert
     * from dB to a linear volume scale do the following:
     *
     * volume = MAX_VOLUME;
     * while (attenuation++ < 0)
     *     volume /= 1.122018454;      //  = (10 ^ (1/20)) = 1dB
     * 
     * @param attenuation is the attenuation in dB (a negative number). 
     * @param callback_data the data pointer that was passed to
     *        LibMame_RunGame
     **/
    void (*SetMasterVolume)(int attenuation, void *callback_data);

    /**
     * This callback is made by libmame periodically to give the thread
     * which called LibMame_RunGame an opportunity to call any one or more
     * of the LibMame_RunningGame_XXX function calls, which are only permitted
     * to be called from this callback.
     *
     * @param callback_data the data pointer that was passed to
     *        LibMame_RunGame
     **/
    void (*MakeRunningGameCalls)(void *callback_data);

    /**
     * This callback is made when MAME has completed a pause operation in
     * response to a LibMame_RunningGame_Pause() call.  The caller may take an
     * indefinite period of time to return from this callback without
     * affecting the paused game.  When the caller returns from this function,
     * the game will resume.  This is the only callback that is not
     * performance sensitive; taking too long in any of the other callbacks
     * could degrade the performance of a running game.
     *
     * @param callback_data the data pointer that was passed to
     *        LibMame_RunGame
     **/
    void (*Paused)(void *callback_data);
     
} LibMame_RunGameCallbacks;


/** **************************************************************************
 * Functions comprising the libmame API
 ************************************************************************** **/

/**
 * Functions for managing the library.
 **/

/**
 * Initializes libmame, preparing any resources that the library needs for
 * operation.  This function must be called before any other LibMame function
 * is called, and also must be called after any call to LibMame_Deinitialize
 * before any other LibMame function can be called.
 **/
LibMame_InitializeStatus LibMame_Initialize();


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
 * Returns the game number of the game that this game is a clone of, if the
 * game is a clone.  Returns -1 if the game is not a clone.
 *
 * @param gamenum is the game number of the game
 * @return the game number of the game that this game is a clone of, if the
 *         game is a clone, or -1 if the game is not a clone.
 **/
int LibMame_Get_Game_CloneOf(int gamenum);


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
 * Returns a value describing the game's original orientation as it was
 * originally played.
 *
 * @param gamenum is the game number of the game
 * @return a value describing the game's original orientation
 **/
LibMame_OrientationType LibMame_Get_Game_Orientation(int gamenum);


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
float LibMame_Get_Game_ScreenRefreshRateHz(int gamenum);


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
LibMame_Chip LibMame_Get_Game_Chip(int gamenum, int chipnum);


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
LibMame_Setting LibMame_Get_Game_Setting(int gamenum, int settingnum);


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
 * This returns a structure describing the controllers for a given game.
 *
 * @param gamenum is the game number of the game
 * @return a structure describing controllers for a given game.
 **/
LibMame_AllControllers LibMame_Get_Game_AllControllers(int gamenum);


/**
 * This returns the number of BIOS sets that are known for a given game.  A
 * BIOS set describes a set of BIOS roms that may be selected to enable
 * specific functionality in a game.
 *
 * @param gamenum is the game number of the game
 * @return number of BIOS sets that are known for a given game.
 **/
int LibMame_Get_Game_BiosSet_Count(int gamenum);


/**
 * This returns a BIOS set for a given game.
 *
 * @param gamenum is the game number of the game
 * @param biossetnum is the number of the BIOS set
 * @return a BIOS set for a given game.
 **/
LibMame_BiosSet LibMame_Get_Game_BiosSet(int gamenum, int biossetnum);


/**
 * This returns the number of ROM images (BIOS or game ROMs) that are known
 * for a given game.  In general, a game needs all of the ROMs from one BIOS
 * set, plus all ROMs not in any BIOS set, to run.
 *
 * @param gamenum is the game number of the game
 * @return the number of ROM images (BIOS or game ROMs) that are known for a
 *         given game
 **/
int LibMame_Get_Game_Rom_Count(int gamenum);


/**
 * This returns information about a ROM that is known to be used by a given
 * game.
 *
 * @param gamenum is the game number of the game
 * @param romnum is the number of the ROM to return
 * @return information about a ROM that is known to be used by a given game
 **/
LibMame_Image LibMame_Get_Game_Rom(int gamenum, int romnum);


/**
 * This returns the number of Hard Disk Drive images (also called CHDs) for a
 * given game.  In general, a game needs all of its CHDs to run.
 *
 * @param gamenum is the game number of the game
 * @return the number of Hard Disk Drive images for a given game
 **/
int LibMame_Get_Game_Hdd_Count(int gamenum);


/**
 * This returns information about an HDD image that is known to be used by a
 * given game.
 *
 * @param gamenum is the game number of the game
 * @param romnum is the number of the HDD image to return
 * @return information about an HDD image that is known to be used by a given
 *         game
 **/
LibMame_Image LibMame_Get_Game_Hdd(int gamenum, int hddnum);


/**
 * Returns the name of the MAME source file that implements the game.
 *
 * @param gamenum is the game number of the game
 * @return the name of the MAME source file that implements the game.
 **/
const char *LibMame_Get_Game_SourceFileName(int gamenum);


/**
 * Functions for managing options
 **/

/**
 * Sets the LibMame_RunGameOptions structure to contain all default values.
 *
 * @param options is the options structure to set defaults in
 **/
void LibMame_Set_Default_RunGameOptions(LibMame_RunGameOptions *options);


/**
 * Functions for running a game
 **/

/**
 * Runs a game.  Currently, this is a non-thread-safe call, so only one thread
 * can be running this at a time.  The game will be run until it is instructed
 * to exit by a call to LibMame_RunningGame_ScheduleExit().
 *
 * @param gamenum is the game number of the game to run
 * @param options if non-NULL, provides the options that the game will be run
 *        with.  If NULL, defaults will be used.
 * @param cbs is the set of callback functions that will be made as the game
 *        runs to gather input and to display output
 * @param callback_data is a pointer that is passed into all of the callback
 *        functions
 * @return the status that resulted from running (or attempting to run) the
 *         game
 **/
LibMame_RunGameStatus LibMame_RunGame(int gamenum,
                                      const LibMame_RunGameOptions *options,
                                      const LibMame_RunGameCallbacks *cbs,
                                      void *callback_data);


/**
 * Functions for altering the state of a running game
 **/

/**
 * Requests that the currently running game be paused.  This will result in a
 * call to the Paused callback of the callbacks structure that was passed into
 * LibMame_RunGame() function.  The game will unpause when that Paused
 * callback returns.  This function may only be called from within the
 * MakeRunningGameCalls callback, and not from any other context of execution.
 *
 * @param game is the game that is to be paused; this game is known because it
 *        was passed into the StartingUp() callback function.
 **/
void LibMame_RunningGame_Pause(LibMame_RunningGame *game);


/**
 * Requests that the currently running game exit as soon as possible.  This
 * will cause the LibMame_RunGame call that ran the game to return after the
 * game has exited.  This function may only be called from within the
 * MakeRunningGameCalls or Paused callback, and not from any other context of
 * execution.
 *
 * @param game is the game that is to be paused; this game is known because it
 *        was passed into the StartingUp() callback function.
 **/
void LibMame_RunningGame_Schedule_Exit(LibMame_RunningGame *game);


/**
 * Requests that the currently running game execute a hard reset as soon as
 * possible, which will completely reset the state of the game and return it
 * to its "just powered on" state.  This function may only be called from
 * within the MakeRunningGameCalls or Paused callback, and not from any other
 * context of execution.
 *
 * @param game is the game that is to be paused; this game is known because it
 *        was passed into the StartingUp() callback function.
 **/
void LibMame_RunningGame_Schedule_Hard_Reset(LibMame_RunningGame *game);


/**
 * Requests that the currently running game execute a soft reset as soon as
 * possible, which will reset the state of the game without simulating a power
 * cycle as is done with the hard reset.  This function may only be called
 * from within the MakeRunningGameCalls or Paused callback, and not from any
 * other context of execution.
 *
 * @param game is the game that is to be paused; this game is known because it
 *        was passed into the StartingUp() callback function.
 **/
void LibMame_RunningGame_Schedule_Soft_Reset(LibMame_RunningGame *game);


/**
 * Requests that the currently running game save a snapshot of its state to
 * the given file as soon as possible.  This function may only be called from
 * within the MakeRunningGameCalls or Paused callback, and not from any other
 * context of execution.
 *
 * @param game is the game that is to be paused; this game is known because it
 *        was passed into the StartingUp() callback function.
 * @param filename is the filename to write the save state to
 **/
void LibMame_RunningGame_SaveState(LibMame_RunningGame *game,
                                   const char *filename);


/**
 * Requests that the currently running game load a previously saved snapshot
 * of its state from the given file as soon as possible, which will replace
 * the current state.  This function may only be called from within the
 * MakeRunningGameCalls or Paused callback, and not from any other context of
 * execution.
 *
 * @param game is the game that is to be paused; this game is known because it
 *        was passed into the StartingUp() callback function.
 * @param filename is the filename to read the save state from
 **/
void LibMame_RunningGame_LoadState(LibMame_RunningGame *game,
                                   const char *filename);


/**
 * Sets a new value for the setting corresponding to a
 * LibMame_Setting of type LibMame_SettingType_Configuration.  The
 * setting is identified by the name and mask of the
 * LibMame_Setting.  This function may only be called from within
 * the MakeRunningGameCalls or Paused callback, and not from any other context
 * of execution.
 *
 * @param game is the game that is to be paused; this game is known because it
 *        was passed into the StartingUp() callback function.
 * @param name is the name of the configuration setting
 * @param mask is the mask of the configuration setting
 * @param value is the value, which must be one of the value_names of the
 *        LibMame_Setting for this configuration setting
 **/
void LibMame_RunningGame_ChangeConfigurationValue(LibMame_RunningGame *game,
                                                  const char *name, 
                                                  uint32_t mask,
                                                  const char *value);


/**
 * Sets a new value for the setting corresponding to a LibMame_Setting of type
 * LibMame_SettingType_Dipswitch.  The setting is identified by the name and
 * mask of the LibMame_Setting.  This function may only be called from within
 * the MakeRunningGameCalls or Paused callback, and not from any other context
 * of execution.
 *
 * @param game is the game that is to be paused; this game is known because it
 *        was passed into the StartingUp() callback function.
 * @param name is the name of the dipswitch setting
 * @param mask is the mask of the dipswitch setting
 * @param value is the value, which must be one of the value_names of the
 *        LibMame_Setting for this dipswitch setting
 **/
void LibMame_RunningGame_ChangeDipswitchValue(LibMame_RunningGame *game,
                                              const char *name, uint32_t mask,
                                              const char *value);


/**
 * Sets a new value for the setting corresponding to a LibMame_Setting of type
 * LibMame_SettingType_Adjuster.  The setting is identified by the name and
 * mask of the LibMame_Setting.  This function may only be called from within
 * the MakeRunningGameCalls or Paused callback, and not from any other context
 * of execution.
 *
 * @param game is the game that is to be paused; this game is known because it
 *        was passed into the StartingUp() callback function.
 * @param name is the name of the adjuster setting
 * @param mask is the mask of the adjuster setting
 * @param value is the value
 **/
void LibMame_RunningGame_ChangeAdjusterValue(LibMame_RunningGame *game,
                                             const char *name, uint32_t mask,
                                             int value);


#ifdef __cplusplus
}
#endif

#endif /* __LIBMAME_H__ */
