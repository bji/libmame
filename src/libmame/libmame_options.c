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
#include "libmame.h"


/** **************************************************************************
 * Enumerations
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
} option_map_entry;


/** **************************************************************************
 * Helper Macros
 ************************************************************************** **/

#define OPTION_MAP_ENTRY(type, name, field)             \
    { option_map_entry_type_##type, name,               \
            offsetof(LibMame_RunGameOptions, field) }


/** **************************************************************************
 * Static global variables
 ************************************************************************** **/

static const option_map_entry g_option_map[] =
{
    OPTION_MAP_ENTRY(boolean, OPTIONS_READCONFIG, read_config),
    OPTION_MAP_ENTRY(string, OPTIONS_ROMPATH, rom_path),
    OPTION_MAP_ENTRY(string, OPTIONS_SAMPLEPATH, sample_path),
    OPTION_MAP_ENTRY(string, OPTIONS_ARTPATH, art_path),
    OPTION_MAP_ENTRY(string, OPTIONS_CTRLRPATH, ctrl_path),
    OPTION_MAP_ENTRY(string, OPTIONS_INIPATH, ini_path),
    OPTION_MAP_ENTRY(string, OPTIONS_FONTPATH, font_path),
    OPTION_MAP_ENTRY(string, OPTIONS_CHEATPATH, cheat_path),
    OPTION_MAP_ENTRY(string, OPTIONS_CROSSHAIRPATH, crosshair_path),
    OPTION_MAP_ENTRY(string, OPTIONS_CFG_DIRECTORY, config_directory),
    OPTION_MAP_ENTRY(string, OPTIONS_NVRAM_DIRECTORY, nvram_directory),
    OPTION_MAP_ENTRY(string, OPTIONS_MEMCARD_DIRECTORY, memcard_directory),
    OPTION_MAP_ENTRY(string, OPTIONS_INPUT_DIRECTORY, input_directory),
    OPTION_MAP_ENTRY(string, OPTIONS_STATE_DIRECTORY, state_directory),
    OPTION_MAP_ENTRY(string, OPTIONS_SNAPSHOT_DIRECTORY, snapshot_directory),
    OPTION_MAP_ENTRY(string, OPTIONS_DIFF_DIRECTORY, diff_directory),
    OPTION_MAP_ENTRY(string, OPTIONS_COMMENT_DIRECTORY, comment_directory),
    OPTION_MAP_ENTRY(string, OPTIONS_STATE, state),
    OPTION_MAP_ENTRY(boolean, OPTIONS_AUTOSAVE, autosave),
    OPTION_MAP_ENTRY(string, OPTIONS_PLAYBACK, playback_file),
    OPTION_MAP_ENTRY(string, OPTIONS_RECORD, record_file),
    OPTION_MAP_ENTRY(string, OPTIONS_MNGWRITE, mngwrite_file),
    OPTION_MAP_ENTRY(string, OPTIONS_AVIWRITE, aviwrite_file),
    OPTION_MAP_ENTRY(string, OPTIONS_WAVWRITE, wavwrite_file),
    OPTION_MAP_ENTRY(string, OPTIONS_SNAPSIZE, snapsize),
    OPTION_MAP_ENTRY(string, OPTIONS_SNAPVIEW, snapview),
    OPTION_MAP_ENTRY(integer, OPTIONS_BURNIN, burnin),
    OPTION_MAP_ENTRY(boolean, OPTIONS_AUTOFRAMESKIP, auto_frame_skip),
    OPTION_MAP_ENTRY(integer, OPTIONS_FRAMESKIP, frame_skip_level),
    OPTION_MAP_ENTRY(boolean, OPTIONS_THROTTLE, throttle),
    OPTION_MAP_ENTRY(boolean, OPTIONS_SLEEP, sleep),
    OPTION_MAP_ENTRY(float, OPTIONS_SPEED, speed_multiplier),
    OPTION_MAP_ENTRY(boolean, OPTIONS_REFRESHSPEED, auto_refresh_speed),
    OPTION_MAP_ENTRY(boolean, OPTIONS_ROTATE, rotate),
    OPTION_MAP_ENTRY(boolean, OPTIONS_ROR, rotate_right),
    OPTION_MAP_ENTRY(boolean, OPTIONS_ROL, rotate_left),
    OPTION_MAP_ENTRY(boolean, OPTIONS_AUTOROR, auto_rotate_right),
    OPTION_MAP_ENTRY(boolean, OPTIONS_AUTOROL, auto_rotate_left),
    OPTION_MAP_ENTRY(boolean, OPTIONS_FLIPX, flip_x),
    OPTION_MAP_ENTRY(boolean, OPTIONS_FLIPY, flip_y),
    OPTION_MAP_ENTRY(boolean, OPTIONS_ARTWORK_CROP, crop_artwork),
    OPTION_MAP_ENTRY(boolean, OPTIONS_USE_BACKDROPS, use_backdrops),
    OPTION_MAP_ENTRY(boolean, OPTIONS_USE_OVERLAYS, use_overlays),
    OPTION_MAP_ENTRY(boolean, OPTIONS_USE_BEZELS, use_bezels),
    OPTION_MAP_ENTRY(float, OPTIONS_BRIGHTNESS, brightness),
    OPTION_MAP_ENTRY(float, OPTIONS_PAUSE_BRIGHTNESS, pause_brightness),
    OPTION_MAP_ENTRY(float, OPTIONS_CONTRAST, contrast),
    OPTION_MAP_ENTRY(float, OPTIONS_GAMMA, gamma),
    OPTION_MAP_ENTRY(boolean, OPTIONS_ANTIALIAS, vector_antialias),
    OPTION_MAP_ENTRY(float, OPTIONS_BEAM, vector_beam),
    OPTION_MAP_ENTRY(float, OPTIONS_FLICKER, vector_flicker),
    OPTION_MAP_ENTRY(boolean, OPTIONS_SOUND, sound),
    OPTION_MAP_ENTRY(integer, OPTIONS_SAMPLERATE, sample_rate),
    OPTION_MAP_ENTRY(boolean, OPTIONS_SAMPLES, use_samples),
    OPTION_MAP_ENTRY(integer, OPTIONS_VOLUME, volume_attenuation),
    OPTION_MAP_ENTRY(boolean, OPTIONS_COIN_LOCKOUT, coin_lockout),
    OPTION_MAP_ENTRY(string, OPTIONS_CTRLR, controller_config_file_name),
    OPTION_MAP_ENTRY(string, OPTIONS_JOYSTICK_MAP, joystick_map),
    OPTION_MAP_ENTRY(float, OPTIONS_JOYSTICK_DEADZONE, joystick_deadzone),
    OPTION_MAP_ENTRY(float, OPTIONS_JOYSTICK_SATURATION, joystick_saturation),
    OPTION_MAP_ENTRY(boolean, OPTIONS_STEADYKEY, enable_steadykey),
    OPTION_MAP_ENTRY(boolean, OPTIONS_NATURAL_KEYBOARD,
                     enable_natural_keyboard),
    OPTION_MAP_ENTRY(boolean, OPTIONS_OFFSCREEN_RELOAD,
                     enable_lightgun_offscreen_reload),
    OPTION_MAP_ENTRY(boolean, OPTIONS_VERBOSE, verbose_output),
    OPTION_MAP_ENTRY(boolean, OPTIONS_LOG, emit_log),
    OPTION_MAP_ENTRY(boolean, OPTIONS_DEBUG, debug),
    OPTION_MAP_ENTRY(boolean, OPTIONS_DEBUG_INTERNAL, debug_internal),
    OPTION_MAP_ENTRY(boolean, OPTIONS_DEBUGSCRIPT, debugscript),
    OPTION_MAP_ENTRY(boolean, OPTIONS_UPDATEINPAUSE, update_in_pause),
    OPTION_MAP_ENTRY(string, OPTIONS_BIOS, special_bios),
    OPTION_MAP_ENTRY(boolean, OPTIONS_CHEAT, enable_cheats),
    OPTION_MAP_ENTRY(boolean, OPTIONS_SKIP_GAMEINFO, skip_gameinfo_screens)
};

static int g_option_map_count[] =
    (sizeof(g_option_map) / sizeof(g_option_map[0]));


/** **************************************************************************
 * Exported functions
 ************************************************************************** **/

mame_options *get_mame_options(const LibMame_RunGameOptions *options)
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
                             *((char *) (((char *) options) + entry->offset)),
                             OPTION_PRIORITY_MAXIMUM);
            break;
        }
    }

    return mame_options;
}
