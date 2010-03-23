/** **************************************************************************
 * libmame_options.c
 *
 * LibMame option support.
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "emu.h"
#include "emuopts.h"
#include "libmame.h"


/** **************************************************************************
 * Enumeration Type Definition
 ************************************************************************** **/

typedef enum
{
    option_map_entry_type_boolean,
    option_map_entry_type_integer,
    option_map_entry_type_float,
    option_map_entry_type_string
} option_map_entry_type;


/** **************************************************************************
 * Structured Type Definitions
 ************************************************************************** **/

typedef struct option_map_entry
{
    option_map_entry_type type;
    const char *name;
    int offset;
    int width;
} option_map_entry;


/** **************************************************************************
 * Helper Macros
 ************************************************************************** **/

#define OPTION_MAP_ENTRY(type, name, field)                             \
    { option_map_entry_type_##type, OPTION_##name,                      \
      offsetof(LibMame_RunGameOptions, field),                          \
      sizeof(((LibMame_RunGameOptions *) g_option_map)-> field) }


/** **************************************************************************
 * Static global variables
 ************************************************************************** **/

/**
 * This is just needed as a convenient place to do offsetof
 **/

static const option_map_entry g_option_map[] =
{
    OPTION_MAP_ENTRY(string, ROMPATH, rom_path),
    OPTION_MAP_ENTRY(string, SAMPLEPATH, sample_path),
    OPTION_MAP_ENTRY(string, ARTPATH, art_path),
    OPTION_MAP_ENTRY(string, CTRLRPATH, ctrl_path),
    OPTION_MAP_ENTRY(string, INIPATH, ini_path),
    OPTION_MAP_ENTRY(string, FONTPATH, font_path),
    OPTION_MAP_ENTRY(string, CHEATPATH, cheat_path),
    OPTION_MAP_ENTRY(string, CROSSHAIRPATH, crosshair_path),
    OPTION_MAP_ENTRY(string, NVRAM_DIRECTORY, nvram_directory),
    OPTION_MAP_ENTRY(string, MEMCARD_DIRECTORY, memcard_directory),
    OPTION_MAP_ENTRY(string, INPUT_DIRECTORY, input_directory),
    OPTION_MAP_ENTRY(string, STATE_DIRECTORY, state_directory),
    OPTION_MAP_ENTRY(string, SNAPSHOT_DIRECTORY, snapshot_directory),
    OPTION_MAP_ENTRY(string, DIFF_DIRECTORY, diff_directory),
    OPTION_MAP_ENTRY(string, COMMENT_DIRECTORY, comment_directory),
    OPTION_MAP_ENTRY(string, STATE, state),
    OPTION_MAP_ENTRY(boolean, AUTOSAVE, autosave),
    OPTION_MAP_ENTRY(string, PLAYBACK, playback_file),
    OPTION_MAP_ENTRY(string, RECORD, record_file),
    OPTION_MAP_ENTRY(string, MNGWRITE, mngwrite_file),
    OPTION_MAP_ENTRY(string, AVIWRITE, aviwrite_file),
    OPTION_MAP_ENTRY(string, WAVWRITE, wavwrite_file),
    OPTION_MAP_ENTRY(string, SNAPSIZE, snapsize),
    OPTION_MAP_ENTRY(string, SNAPVIEW, snapview),
    OPTION_MAP_ENTRY(integer, BURNIN, burnin),
    OPTION_MAP_ENTRY(boolean, AUTOFRAMESKIP, auto_frame_skip),
    OPTION_MAP_ENTRY(integer, FRAMESKIP, frame_skip_level),
    OPTION_MAP_ENTRY(boolean, THROTTLE, throttle),
    OPTION_MAP_ENTRY(boolean, SLEEP, sleep),
    OPTION_MAP_ENTRY(float, SPEED, speed_multiplier),
    OPTION_MAP_ENTRY(boolean, REFRESHSPEED, auto_refresh_speed),
    OPTION_MAP_ENTRY(boolean, ROTATE, rotate),
    OPTION_MAP_ENTRY(boolean, ROR, rotate_right),
    OPTION_MAP_ENTRY(boolean, ROL, rotate_left),
    OPTION_MAP_ENTRY(boolean, AUTOROR, auto_rotate_right),
    OPTION_MAP_ENTRY(boolean, AUTOROL, auto_rotate_left),
    OPTION_MAP_ENTRY(boolean, FLIPX, flip_x),
    OPTION_MAP_ENTRY(boolean, FLIPY, flip_y),
    OPTION_MAP_ENTRY(boolean, ARTWORK_CROP, crop_artwork),
    OPTION_MAP_ENTRY(boolean, USE_BACKDROPS, use_backdrops),
    OPTION_MAP_ENTRY(boolean, USE_OVERLAYS, use_overlays),
    OPTION_MAP_ENTRY(boolean, USE_BEZELS, use_bezels),
    OPTION_MAP_ENTRY(float, BRIGHTNESS, brightness),
    OPTION_MAP_ENTRY(float, CONTRAST, contrast),
    OPTION_MAP_ENTRY(float, GAMMA, gamma),
    OPTION_MAP_ENTRY(float, PAUSE_BRIGHTNESS, pause_brightness),
    OPTION_MAP_ENTRY(boolean, ANTIALIAS, vector_antialias),
    OPTION_MAP_ENTRY(float, BEAM, vector_beam),
    OPTION_MAP_ENTRY(float, FLICKER, vector_flicker),
    OPTION_MAP_ENTRY(boolean, SOUND, sound),
    OPTION_MAP_ENTRY(integer, SAMPLERATE, sample_rate),
    OPTION_MAP_ENTRY(boolean, SAMPLES, use_samples),
    OPTION_MAP_ENTRY(integer, VOLUME, volume_attenuation),
    OPTION_MAP_ENTRY(boolean, COIN_LOCKOUT, coin_lockout),
    OPTION_MAP_ENTRY(string, JOYSTICK_MAP, joystick_map),
    OPTION_MAP_ENTRY(float, JOYSTICK_DEADZONE, joystick_deadzone),
    OPTION_MAP_ENTRY(float, JOYSTICK_SATURATION, joystick_saturation),
    OPTION_MAP_ENTRY(boolean, STEADYKEY, enable_steadykey),
    OPTION_MAP_ENTRY(boolean, NATURAL_KEYBOARD, enable_natural_keyboard),
    OPTION_MAP_ENTRY(boolean, OFFSCREEN_RELOAD, 
                     enable_lightgun_offscreen_reload),
    OPTION_MAP_ENTRY(boolean, VERBOSE, verbose_output),
    OPTION_MAP_ENTRY(boolean, LOG, emit_log),
    OPTION_MAP_ENTRY(boolean, DEBUG, debug),
    OPTION_MAP_ENTRY(boolean, DEBUG_INTERNAL, debug_internal),
    OPTION_MAP_ENTRY(boolean, DEBUGSCRIPT, debugscript),
    OPTION_MAP_ENTRY(boolean, UPDATEINPAUSE, update_in_pause),
    OPTION_MAP_ENTRY(string, BIOS, special_bios),
    OPTION_MAP_ENTRY(boolean, CHEAT, enable_cheats),
    OPTION_MAP_ENTRY(boolean, SKIP_GAMEINFO, skip_gameinfo_screens)
};

static int g_option_map_count =
    (sizeof(g_option_map) / sizeof(g_option_map[0]));


/** **************************************************************************
 * Exported functions
 ************************************************************************** **/

core_options *get_mame_options(const LibMame_RunGameOptions *options,
                               const char *gamename)
{
    core_options *mame_options = mame_options_init(NULL);

    for (int i = 0; i < g_option_map_count; i++) {
        const option_map_entry *entry = &(g_option_map[i]);
        switch (entry->type) {
        case option_map_entry_type_boolean:
            options_set_bool(mame_options, entry->name,
                             *((bool *) (((char *) options) + entry->offset)),
                             OPTION_PRIORITY_MAXIMUM);
            break;
        case option_map_entry_type_integer:
            options_set_int(mame_options, entry->name,
                            *((int *) (((char *) options) + entry->offset)),
                            OPTION_PRIORITY_MAXIMUM);
            break;
        case option_map_entry_type_float:
            options_set_float(mame_options, entry->name,
                            *((float *) (((char *) options) + entry->offset)),
                            OPTION_PRIORITY_MAXIMUM);
            break;
        case option_map_entry_type_string:
            options_set_string(mame_options, entry->name,
                               (((char *) options) + entry->offset),
                               OPTION_PRIORITY_MAXIMUM);
            break;
        }
    }

    /* libmame *always* disables MAME's configuration file reading option.
       MAME configuration files can affect the behavior of games in ways that
       don't work with libmame, so they can't be used.  And having config file
       functionality in MAME core seems kind of wrong anyway ... */
    options_set_bool(mame_options, "readconfig", false, 
                     OPTION_PRIORITY_MAXIMUM);

    /* Finally, set the game name in there */
    options_set_string(mame_options, OPTION_GAMENAME, gamename, 
                       OPTION_PRIORITY_MAXIMUM);

    return mame_options;
}


void LibMame_Set_Default_RunGameOptions(LibMame_RunGameOptions *options)
{
    /* Create an options structure just to get the defaults out of it */
	core_options *mame_defaults = options_create(NULL);
    /* Populate it with MAME options */
	options_add_entries(mame_defaults, mame_core_options);

    for (int i = 0; i < g_option_map_count; i++) {
        const option_map_entry *entry = &(g_option_map[i]);
        switch (entry->type) {
        case option_map_entry_type_boolean:
            *((bool *) (((char *) options) + entry->offset)) =
                options_get_bool(mame_defaults, entry->name);
            break;
        case option_map_entry_type_integer:
            *((int *) (((char *) options) + entry->offset)) =
                options_get_int(mame_defaults, entry->name);
            break;
        case option_map_entry_type_float:
            *((float *) (((char *) options) + entry->offset)) =
                options_get_float(mame_defaults, entry->name);
            break;
        case option_map_entry_type_string:
            const char *mame_default = options_get_string
                (mame_defaults, entry->name);
            char *option_value = ((char *) options) + entry->offset;
            if (mame_default) {
                strncpy(option_value, mame_default, entry->width);
                option_value[entry->width - 1] = 0;
            }
            else {
                option_value[0] = 0;
            }
            break;
        }
    }

    /* Don't forget to free the defaults options that are no longer needed! */
    options_free(mame_defaults);
}
