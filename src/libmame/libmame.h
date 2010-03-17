/** **************************************************************************
 * libmame.h
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#ifndef __LIBMAME_H__
#define __LIBMAME_H__

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
 * Screen types
 **/
typedef enum
{
    LibMame_ScreenType_Raster,
    LibMame_ScreenType_LCD,
    LibMame_ScreenType_Vector
} LibMame_ScreenType;


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
 * This describes a dipswitch that MAME emulates.
 **/
typedef struct LibMame_DipswitchDescriptor
{
    /**
     * This is the name of the dipswitch, which typically describes what
     * the dipswitch does
     **/
    const char *name;

    /**
     * This is the number of individual settings that the dipswitch may be
     * set to
     **/
    int setting_count;

    /**
     * This is the index into the setting_names array of the default setting
     **/
    int default_setting_number;

    /**
     * This is the names of each of the individual settings that the dipswitch
     * may be set to, which typically describe what the setting does
     **/
    const char **setting_names;
} LibMame_DipswitchDescriptor;


/**
 * This describes an adjuster that MAME can use to adjust a setting, typically
 * a volume.  Adjusters always adjust a setting between a value of 0 and 100.
 **/
typedef struct LibMame_AdjusterDescriptor
{
    /**
     * This is the name of the adjuster, which typically describes its purpose
     **/
    const char *name;

    /**
     * This is the default value of the adjuster.  Adjusters can be set to
     * values between 0 and 100, inclusive.
     **/
    int default_value;
} LibMame_AdjusterDescriptor;


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
int LibMame_Get_Game_SoundChannelCount(int gamenum);


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
 * Returns the number of dipswitches that MAME emulates for a given game.
 *
 * @param gamenum is the game number of the game
 * @return the number of dipswitches that MAME emulates for a given game.
 **/
int LibMame_Get_Game_Dipswitch_Count(int gamenum);


/**
 * Returns a description of a dipswitch that MAME supports for a given game.
 *
 * @param gamenum is the game number of the game
 * @param dipswitchnum is the dipswitch number of the dipswitch
 * @return a description of a dipswitch that MAME supports for a given game.
 **/
LibMame_DipswitchDescriptor LibMame_Get_Game_Dipswitch(int gamenum,
                                                       int dipswitchnum);


/**
 * Returns the number of setting adjusters that MAME supports for a given
 * game.
 *
 * @param gamenum is the game number of the game
 * @return the number of setting adjusters that MAME supports for a given
 *         game.
 **/
int LibMame_Get_Game_Adjusters_Count(int gamenum);


/**
 * Returns a description of a setting adjuster that MAME supports for a
 * given game.
 *
 * @param gamenum is the game number of the game
 * @param adjusternum is the adjuster number of the adjuster
 * @return a description of a setting adjuster that MAME supports for a
 *         given game.
 **/
LibMame_AdjusterDescriptor LibMame_Get_Game_Adjuster(int gamenum,
                                                     int adjusternum);


/**
 * Returns the name of the MAME source file that implements the game.
 *
 * @param gamenum is the game number of the game
 * @return the name of the MAME source file that implements the game.
 **/
const char *LibMame_Get_Game_SourceFileName(int gamenum);


/**
 * info.c to emulate (in order of importance, most important first):

	print_game_input(out, game, portlist);
	print_game_configs(out, game, portlist);
	print_game_adjusters(out, game, portlist);
	print_game_bios(out, game);
	print_game_rom(out, game, config);
**/


#endif /* __LIBMAME_H__ */
