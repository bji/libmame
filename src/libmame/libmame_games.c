/** **************************************************************************
 * libmame_games.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/


/**
 * Support for getting game info
 **/

#include "HashTable.h"
#include "emu.h"
#include "config.h"
#include "libmame.h"
#include "osdcore.h"
#include "sound/samples.h"
#include <pthread.h>
#include <string.h>

/**
 * MAME sources seem to follow 8.3 limits, so 16 should be enough
 **/
#define SOURCE_FILE_NAME_MAX 16
#define INVALID_CONTROLLER_TYPE ((LibMame_ControllerType) -1)

typedef struct GameInfo
{
    bool converted;
    int driver_index;
    int gameinfo_index;
    int year_of_release;
    int working_flags, orientation_flags;
    LibMame_ScreenType screen_type;
    LibMame_ScreenResolution screen_resolution;
    int screen_refresh_rate;
    int sound_channel_count;
    int sound_samples_count;
    int sound_samples_source;
    const char **sound_samples;
    int chip_count;
    LibMame_ChipDescriptor *chips;
    int setting_count;
    LibMame_SettingDescriptor *settings;
    int max_simultaneous_players;
    LibMame_ControllerSetDescriptor controller_set;
    char source_file_name[SOURCE_FILE_NAME_MAX];
} GameInfo;


static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_game_count = 0;
static GameInfo *g_gameinfos = 0;
/* Hash short names to driver indexes */
static Hash::Table<Hash::StringKey, int> g_drivers_hash;
/* Hash short names to gameinfo indexes */
static Hash::Table<Hash::StringKey, int> g_gameinfos_hash;


#define CONVERT_FLAG(mameflag, field, libflag) do {                 \
        if (driver->flags & mameflag) {                             \
            gameinfo-> field |= libflag;                            \
        }                                                           \
    } while (0)

#define CONVERT_FLAG_OPPOSITE(mameflag, field, libflag) do {           \
        if (!(driver->flags & mameflag)) {                             \
            gameinfo-> field |= libflag;                               \
        }                                                              \
    } while (0)

#define SAFE_STRCPY(dest, src) \
    safe_strncpy(dest, src, sizeof(dest))


static GameInfo *get_gameinfo_locked(int gamenum);
static GameInfo *get_gameinfo(int gamenum);

static void *osd_calloc(size_t size)
{
    void *ret = osd_malloc(size);

    if (ret) {
        memset(ret, 0, size);
    }

    return ret;
}


static void safe_strncpy(char *dest, const char *src, size_t n)
{
    if (src == NULL) {
        *dest = 0;
    }
    else {
        (void) strncpy(dest, src, n);
        dest[n - 1] = 0;
    }
}


static void convert_year(const game_driver *driver, GameInfo *gameinfo)
{
    const char *yearstr = driver->year;

    if (yearstr) {
        gameinfo->year_of_release = 0;
        while (*yearstr) {
            if ((*yearstr >= '0') && (*yearstr <= '9')) {
                gameinfo->year_of_release *= 10;
                gameinfo->year_of_release += *yearstr - '0';
            }
            else {
                gameinfo->year_of_release = -1;
                break;
            }
            yearstr++;
        }
    }
    else {
        gameinfo->year_of_release = -1;
    }
}


static void convert_working_flags(const game_driver *driver,
                                  GameInfo *gameinfo)
{
    CONVERT_FLAG(GAME_NOT_WORKING, working_flags, 
                 LIBMAME_WORKINGFLAGS_NOTWORKING);
    CONVERT_FLAG(GAME_UNEMULATED_PROTECTION, working_flags, 
                 LIBMAME_WORKINGFLAGS_UNEMULATED_PROTECTION);
    CONVERT_FLAG(GAME_WRONG_COLORS, working_flags,
                 LIBMAME_WORKINGFLAGS_WRONG_COLORS);
    CONVERT_FLAG(GAME_IMPERFECT_COLORS, working_flags,
                 LIBMAME_WORKINGFLAGS_IMPERFECT_COLORS);
    CONVERT_FLAG(GAME_IMPERFECT_GRAPHICS, working_flags,
                 LIBMAME_WORKINGFLAGS_IMPERFECT_GRAPHICS);
    CONVERT_FLAG(GAME_NO_COCKTAIL, working_flags, 
                 LIBMAME_WORKINGFLAGS_NO_COCKTAIL);
    CONVERT_FLAG(GAME_NO_SOUND, working_flags, 
                 LIBMAME_WORKINGFLAGS_NO_SOUND);
    CONVERT_FLAG(GAME_IMPERFECT_SOUND, working_flags, 
                 LIBMAME_WORKINGFLAGS_IMPERFECT_SOUND);
    CONVERT_FLAG_OPPOSITE(GAME_SUPPORTS_SAVE, working_flags, 
                          LIBMAME_WORKINGFLAGS_NO_SAVE_STATE);
    CONVERT_FLAG(GAME_NO_SOUND_HW, working_flags, 
                 LIBMAME_WORKINGFLAGS_NO_SOUND);
}


static void convert_orientation_flags(const game_driver *driver,
                                      GameInfo *gameinfo)
{
    CONVERT_FLAG(ORIENTATION_FLIP_X, orientation_flags,
                 LIBMAME_ORIENTATIONFLAGS_FLIP_X);
    CONVERT_FLAG(ORIENTATION_FLIP_Y, orientation_flags,
                 LIBMAME_ORIENTATIONFLAGS_FLIP_Y);
    CONVERT_FLAG(ROT90, orientation_flags,
                 LIBMAME_ORIENTATIONFLAGS_ROTATE_90);
    CONVERT_FLAG(ROT180, orientation_flags,
                 LIBMAME_ORIENTATIONFLAGS_ROTATE_180);
    CONVERT_FLAG(ROT270, orientation_flags,
                 LIBMAME_ORIENTATIONFLAGS_ROTATE_270);
}


static void convert_screen_info(const machine_config *machineconfig,
                                GameInfo *gameinfo)
{
    /* We assume that all screens are the same; and in any case, only report
       on the first screen, which is assumed to be the primary screen */
    const device_config *devconfig = video_screen_first(machineconfig);
    if (devconfig != NULL) {
        const screen_config *screenconfig =
            (const screen_config *) devconfig->inline_config;
        switch (screenconfig->type) {
        case SCREEN_TYPE_RASTER:
            gameinfo->screen_type = LibMame_ScreenType_Raster;
            break;
        case SCREEN_TYPE_LCD:
            gameinfo->screen_type = LibMame_ScreenType_LCD;
            break;
        case SCREEN_TYPE_VECTOR:
            gameinfo->screen_type = LibMame_ScreenType_Vector;
            break;
        }
        if (gameinfo->screen_type != LibMame_ScreenType_Vector) {
            gameinfo->screen_resolution.width = 
                ((screenconfig->visarea.max_x - 
                  screenconfig->visarea.min_x) + 1);
            gameinfo->screen_resolution.height = 
                ((screenconfig->visarea.max_y - 
                  screenconfig->visarea.min_y) + 1);
        }
        gameinfo->screen_refresh_rate = 
            ATTOSECONDS_TO_HZ(screenconfig->refresh);
    }
}


static void convert_sound_channels(const machine_config *machineconfig,
                                   GameInfo *gameinfo)
{
    gameinfo->sound_channel_count = sound_first(machineconfig) ?
        speaker_output_count(machineconfig) : 0;
}


static void convert_sound_samples_helper(const machine_config *machineconfig,
                                         GameInfo *gameinfo, bool just_count)
{
    const char **destsample = gameinfo->sound_samples;

	const device_config *devconfig;

	for (devconfig = sound_first(machineconfig); devconfig; 
         devconfig = sound_next(devconfig)) {
		if (sound_get_type(devconfig) != SOUND_SAMPLES) {
            continue;
        }

        const char * const *samplenames = 
            ((const samples_interface *) devconfig->static_config)->samplenames;

        if (!samplenames) {
            continue;
        }
        
        /* iterate over sample names */
        for (int i = 0; samplenames[i]; i++) {
            const char *samplename = samplenames[i];

            /* "*" sample indicates the sample source */
            if (*samplename == '*') {
                if (!just_count && (gameinfo->sound_samples_source == -1) &&
                    samplename[1]) {
                    int *index = g_gameinfos_hash.Get(&(samplename[1]));
                    if (index) {
                        gameinfo->sound_samples_source = *index;
                    }
                }
                continue;
            }

            /* Find the index of the first sample with the same name as this */
            int j;
            for (j = 0; j < i; j++) {
                if (!strcmp(samplenames[j], samplename)) {
                    break;
                }
            }

            /* If this is the first occurrence of this sample name, then use
               it; otherwise it's a duplicate that we already used so ignore */
            if (j == i) {
                if (just_count) {
                    gameinfo->sound_samples_count++;
                }
                else {
                    *destsample = samplename;
                    destsample++;
                }
            }
        }
    }
}

static bool sound_samples_identical(const GameInfo *g1, const GameInfo *g2)
{
    if (g1->sound_samples_count != g2->sound_samples_count) {
        return false;
    }

    for (int i = 0; i < g1->sound_samples_count; i++) {
        /* First assume that it's in the sample position because it
           normally is */
        if (!strcmp(g1->sound_samples[i], g2->sound_samples[i])) {
            continue;
        }

        /* Now check the rest */
        int j = 0;
        for (j = 0; j < g2->sound_samples_count; j++) {
            if (j == i) {
                continue;
            }
            if (!strcmp(g1->sound_samples[i], g2->sound_samples[j])) {
                break;
            }
        }
        if (j == g2->sound_samples_count) {
            return false;
        }
    }

    return true;
}


static void convert_sound_samples(const machine_config *machineconfig,
                                  GameInfo *gameinfo)
{
    gameinfo->sound_samples_source = -1;

    convert_sound_samples_helper(machineconfig, gameinfo, true);
    if (gameinfo->sound_samples_count) {
        gameinfo->sound_samples = (const char **) osd_malloc
            (sizeof(const char *) * gameinfo->sound_samples_count);
        convert_sound_samples_helper(machineconfig, gameinfo, false);
        if (gameinfo->sound_samples_source == -1) {
            gameinfo->sound_samples_source = gameinfo->gameinfo_index;
        }
        /* Finally, if the sound samples that were found were identical to
           those of the source game (and the source game was different than
           this game), then just free them as an indication that they are
           identical */
        if (gameinfo->sound_samples_source != gameinfo->gameinfo_index) {
            const GameInfo *source_gameinfo = 
                get_gameinfo_locked(gameinfo->sound_samples_source);
            if (sound_samples_identical(gameinfo, source_gameinfo)) {
                osd_free(gameinfo->sound_samples);
                gameinfo->sound_samples = NULL;
            }
        }
    }
}


static void convert_chips(const machine_config *machineconfig,
                          GameInfo *gameinfo)
{
	const device_config *devconfig;

	for (devconfig = cpu_first(machineconfig); devconfig; 
         devconfig = cpu_next(devconfig)) {
        gameinfo->chip_count++;
    }

	for (devconfig = sound_first(machineconfig); devconfig; 
         devconfig = sound_next(devconfig)) {
        gameinfo->chip_count++;
    }

    if (gameinfo->chip_count == 0) {
        return;
    }

    gameinfo->chips = (LibMame_ChipDescriptor *) osd_calloc
        (sizeof(LibMame_ChipDescriptor) * gameinfo->chip_count);
    
    LibMame_ChipDescriptor *descriptor = gameinfo->chips;

	for (devconfig = cpu_first(machineconfig); devconfig; 
         devconfig = cpu_next(devconfig)) {
        descriptor->is_sound = false;
        descriptor->tag = devconfig->tag();
        descriptor->name = devconfig->name();
        descriptor->clock_hz = devconfig->clock;
        descriptor++;
    }        
    
	for (devconfig = sound_first(machineconfig); devconfig; 
         devconfig = sound_next(devconfig)) {
        descriptor->is_sound = true;
        descriptor->tag = devconfig->tag();
        descriptor->name = devconfig->name();
        descriptor->clock_hz = devconfig->clock;
        descriptor++;
    }        
}


static void convert_settings(const ioport_list *ioportlist,
                             GameInfo *gameinfo)
{
	const input_port_config *port;
	const input_field_config *field;
    
	for (port = ioportlist->first(); port; port = port->next) {
		for (field = port->fieldlist; field; field = field->next) {
            if ((field->type != IPT_CONFIG) &&
                (field->type != IPT_DIPSWITCH) &&
                (field->type != IPT_ADJUSTER)) {
                continue;
            }
            gameinfo->setting_count++;
        }
    }

    if (gameinfo->setting_count == 0) {
        return;
    }

    gameinfo->settings = (LibMame_SettingDescriptor *)
        osd_calloc(sizeof(LibMame_SettingDescriptor) * 
                   gameinfo->setting_count);

    LibMame_SettingDescriptor *desc = gameinfo->settings;

	for (port = ioportlist->first(); port; port = port->next) {
		for (field = port->fieldlist; field; field = field->next) {
            if (field->type == IPT_CONFIG) {
                desc->type = LibMame_SettingType_Configuration;
            }
            else if (field->type == IPT_DIPSWITCH) {
                desc->type = LibMame_SettingType_Dipswitch;
            }
            else if (field->type == IPT_ADJUSTER) {
                desc->type = LibMame_SettingType_Adjuster;
                desc->name = input_field_name(field);
                desc->default_value = field->defvalue;
                desc++;
                continue;
            }
            else {
                continue;
            }
            
            /* IPT_CONFIG and IPT_DIPSWITCH are handled here */

            desc->name = input_field_name(field);
            const input_setting_config *setting;
            for (setting = field->settinglist; setting; 
                 setting = setting->next) {
                desc->value_count++;
            }

            if (desc->value_count == 0) {
                continue;
            }

            desc->value_names = (const char * const *) osd_malloc
                (sizeof(const char *) * desc->value_count);
            const char **value_names = (const char **) desc->value_names;
            int index = 0;
            for (setting = field->settinglist; setting; 
                 setting = setting->next) {
                value_names[index] = setting->name;
                if (setting->value == field->defvalue) {
                    desc->default_value = index;
                }
                index++;
            }
            
            desc++;
        }
    }
}


static void convert_controllers(const ioport_list *ioportlist,
                                GameInfo *gameinfo)
{
	const input_port_config *port;
	const input_field_config *field;

    /* Count the players and buttons */
	for (port = ioportlist->first(); port; port = port->next) {
		for (field = port->fieldlist; field; field = field->next) {
			if (gameinfo->max_simultaneous_players < field->player) {
				gameinfo->max_simultaneous_players = field->player;
            }
            switch (field->type) {

#define CASE_BUTTON(iptname, n)                                         \
            case iptname:                                               \
                if (gameinfo->controller_set.normal_button_count < n) { \
                    gameinfo->controller_set.normal_button_count = n;   \
                }                                                       \
                break

                CASE_BUTTON(IPT_BUTTON1, 1);
                CASE_BUTTON(IPT_BUTTON2, 2);
                CASE_BUTTON(IPT_BUTTON3, 3);
                CASE_BUTTON(IPT_BUTTON4, 4);
                CASE_BUTTON(IPT_BUTTON5, 5);
                CASE_BUTTON(IPT_BUTTON6, 6);
                CASE_BUTTON(IPT_BUTTON7, 7);
                CASE_BUTTON(IPT_BUTTON8, 8);
                CASE_BUTTON(IPT_BUTTON9, 9);
                CASE_BUTTON(IPT_BUTTON10, 10);
                CASE_BUTTON(IPT_BUTTON11, 11);
                CASE_BUTTON(IPT_BUTTON12, 12);
                CASE_BUTTON(IPT_BUTTON13, 13);
                CASE_BUTTON(IPT_BUTTON14, 14);
                CASE_BUTTON(IPT_BUTTON15, 15);
                CASE_BUTTON(IPT_BUTTON16, 16);
                break;
            }
        }
    }
    gameinfo->max_simultaneous_players++;

    if (gameinfo->controller_set.normal_button_count) {
        gameinfo->controller_set.normal_button_names = (const char * const *) 
            osd_calloc(sizeof(const char *) *
                       gameinfo->controller_set.normal_button_count);
    }

    const char **normal_button_names = (const char **) 
        gameinfo->controller_set.normal_button_names;

	for (port = ioportlist->first(); port; port = port->next) {
		for (field = port->fieldlist; field; field = field->next) {
            switch (field->type) {
            case IPT_JOYSTICK_LEFT:
            case IPT_JOYSTICK_RIGHT:
                switch (field->way) {
                case 2:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_JOYSTICKHORIZONTAL;
                    break;
                case 4:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_JOYSTICK4WAY;
                    break;
                default:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_JOYSTICK8WAY;
                    break;
                }
                break;
            case IPT_JOYSTICK_UP:
            case IPT_JOYSTICK_DOWN:
                switch (field->way) {
                case 2:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_JOYSTICKVERTICAL;
                    break;
                case 4:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_JOYSTICK4WAY;
                    break;
                default:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_JOYSTICK8WAY;
                    break;
                }
                break;
            case IPT_JOYSTICKRIGHT_LEFT:
            case IPT_JOYSTICKRIGHT_RIGHT:
            case IPT_JOYSTICKLEFT_LEFT:
            case IPT_JOYSTICKLEFT_RIGHT:
                switch (field->way) {
                case 2:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_DOUBLEJOYSTICKHORIZONTAL;
                    break;
                case 4:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_DOUBLEJOYSTICK4WAY;
                    break;
                default:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_DOUBLEJOYSTICK8WAY;
                    break;
                }
                break;
            case IPT_JOYSTICKRIGHT_UP:
            case IPT_JOYSTICKRIGHT_DOWN:
            case IPT_JOYSTICKLEFT_UP:
            case IPT_JOYSTICKLEFT_DOWN:
                switch (field->way) {
                case 2:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_DOUBLEJOYSTICKVERTICAL;
                    break;
                case 4:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_DOUBLEJOYSTICK4WAY;
                    break;
                default:
                    gameinfo->controller_set.controller_flags |= 
                        LIBMAME_CONTROLLERFLAGS_DOUBLEJOYSTICK8WAY;
                    break;
                }
                break;
            case IPT_PADDLE:
                gameinfo->controller_set.controller_flags |= 
                    LIBMAME_CONTROLLERFLAGS_PADDLE;
                break;
            case IPT_DIAL:
                gameinfo->controller_set.controller_flags |= 
                    LIBMAME_CONTROLLERFLAGS_SPINNER;
                break;
            case IPT_TRACKBALL_X:
            case IPT_TRACKBALL_Y:
                gameinfo->controller_set.controller_flags |= 
                    LIBMAME_CONTROLLERFLAGS_TRACKBALL;
                break;
            case IPT_AD_STICK_X:
            case IPT_AD_STICK_Y:
                gameinfo->controller_set.controller_flags |= 
                    LIBMAME_CONTROLLERFLAGS_JOYSTICKANALOG;
                break;
            case IPT_LIGHTGUN_X:
            case IPT_LIGHTGUN_Y:
                gameinfo->controller_set.controller_flags |= 
                    LIBMAME_CONTROLLERFLAGS_LIGHTGUN;
                break;
            case IPT_PEDAL:
                gameinfo->controller_set.controller_flags |= 
                    LIBMAME_CONTROLLERFLAGS_PEDAL;
                break;
            case IPT_PEDAL2:
                gameinfo->controller_set.controller_flags |= 
                    LIBMAME_CONTROLLERFLAGS_PEDAL2;
                break;
            case IPT_PEDAL3:
                gameinfo->controller_set.controller_flags |= 
                    LIBMAME_CONTROLLERFLAGS_PEDAL3;
                break;

#undef CASE_BUTTON
#define CASE_BUTTON(iptname, n)                         \
            case iptname:                               \
                normal_button_names[n] = field->name;   \
                break
                
            CASE_BUTTON(IPT_BUTTON1, 0);
            CASE_BUTTON(IPT_BUTTON2, 1);
            CASE_BUTTON(IPT_BUTTON3, 2);
            CASE_BUTTON(IPT_BUTTON4, 3);
            CASE_BUTTON(IPT_BUTTON5, 4);
            CASE_BUTTON(IPT_BUTTON6, 5);
            CASE_BUTTON(IPT_BUTTON7, 6);
            CASE_BUTTON(IPT_BUTTON8, 7);
            CASE_BUTTON(IPT_BUTTON9, 8);
            CASE_BUTTON(IPT_BUTTON10, 9);
            CASE_BUTTON(IPT_BUTTON11, 10);
            CASE_BUTTON(IPT_BUTTON12, 11);
            CASE_BUTTON(IPT_BUTTON13, 12);
            CASE_BUTTON(IPT_BUTTON14, 13);
            CASE_BUTTON(IPT_BUTTON15, 14);
            CASE_BUTTON(IPT_BUTTON16, 15);

#undef CASE_BUTTON
#define CASE_BUTTON(field, iptname, flag)                               \
            case iptname:                                               \
                gameinfo->controller_set. field |= flag;                \
                break

            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_A,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_A);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_B,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_B);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_C,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_C);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_D,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_D);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_E,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_E);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_F,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_F);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_G,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_G);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_H,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_H);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_I,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_I);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_J,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_J);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_K,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_K);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_L,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_L);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_M,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_M);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_N,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_N);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_O,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_O);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_P,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_P);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_Q,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_Q);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_KAN,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_KAN);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_PON,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_PON);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_CHI,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_CHI);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_REACH,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_REACH);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_RON,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_RON);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_BET,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_BET);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_LAST_CHANCE,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_LAST_CHANCE);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_SCORE,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_SCORE);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_DOUBLE_UP,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_DOUBLE_UP);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_FLIP_FLOP,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_FLIP_FLOP);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_BIG,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_BIG);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_SMALL,
                        LIBMAME_CONTROLLERFLAGS_MAHJONG_SMALL);
            
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_A,
                        LIBMAME_CONTROLLERFLAGS_HANAFUDA_A);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_B,
                        LIBMAME_CONTROLLERFLAGS_HANAFUDA_B);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_C,
                        LIBMAME_CONTROLLERFLAGS_HANAFUDA_C);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_D,
                        LIBMAME_CONTROLLERFLAGS_HANAFUDA_D);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_E,
                        LIBMAME_CONTROLLERFLAGS_HANAFUDA_E);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_F,
                        LIBMAME_CONTROLLERFLAGS_HANAFUDA_F);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_G,
                        LIBMAME_CONTROLLERFLAGS_HANAFUDA_G);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_H,
                        LIBMAME_CONTROLLERFLAGS_HANAFUDA_H);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_YES,
                        LIBMAME_CONTROLLERFLAGS_HANAFUDA_YES);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_NO,
                        LIBMAME_CONTROLLERFLAGS_HANAFUDA_NO);

            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_HIGH,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_HIGH);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_LOW,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_LOW);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_HALF,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_HALF);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_DEAL,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_DEAL);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_D_UP,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_D_UP);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_TAKE,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_TAKE);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_STAND,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_STAND);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_BET,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_BET);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_KEYIN,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_KEYIN);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_KEYOUT,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_KEYOUT);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_PAYOUT,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_PAYOUT);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_DOOR,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_DOOR);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_SERVICE,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_SERVICE);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_BOOK,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_BOOK);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_HOLD1,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_HOLD1);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_HOLD2,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_HOLD2);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_HOLD3,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_HOLD3);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_HOLD4,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_HOLD4);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_HOLD5,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_HOLD5);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_CANCEL,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_CANCEL);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_BET,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_BET);
            CASE_BUTTON(gambling_button_flags, IPT_SLOT_STOP1,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_STOP1);
            CASE_BUTTON(gambling_button_flags, IPT_SLOT_STOP2,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_STOP2);
            CASE_BUTTON(gambling_button_flags, IPT_SLOT_STOP3,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_STOP3);
            CASE_BUTTON(gambling_button_flags, IPT_SLOT_STOP4,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_STOP4);
            CASE_BUTTON(gambling_button_flags, IPT_SLOT_STOP_ALL,
                        LIBMAME_CONTROLLERFLAGS_GAMBLING_STOP_ALL);
            }
        }
    }
}


static void convert_source_file_name(const game_driver *driver,
                                     GameInfo *gameinfo)
{
    const char *s = driver->source_file;
    
    if ((s == NULL) || !*s) {
        gameinfo->source_file_name[0] = 0;
        return;
    }

    while (*s) {
        s++;
    }

    while ((*(s - 1) != '/') && (*(s - 1) != '\\')) {
        s--;
    }

    SAFE_STRCPY(gameinfo->source_file_name, s);
}


static void convert_game_info(GameInfo *gameinfo)
{
    const game_driver *driver = drivers[gameinfo->driver_index];

	machine_config *machineconfig = 
        machine_config_alloc(driver->machine_config);
    ioport_list ioportlist;
    input_port_list_init(ioportlist, driver->ipt, 0, 0, FALSE);
    /* Mame's code assumes the above succeeds, we will too */

    convert_year(driver, gameinfo);
    convert_working_flags(driver, gameinfo);
    convert_orientation_flags(driver, gameinfo);
    convert_screen_info(machineconfig, gameinfo);
    convert_sound_channels(machineconfig, gameinfo);
    convert_sound_samples(machineconfig, gameinfo);
    convert_chips(machineconfig, gameinfo);
    convert_settings(&ioportlist, gameinfo);
    convert_controllers(&ioportlist, gameinfo);
    convert_source_file_name(driver, gameinfo);

    machine_config_free(machineconfig);
}


static GameInfo *get_gameinfo_helper_locked(int gamenum, bool converted)
{
    if (g_game_count == 0) {
        const game_driver * const *pdriver = drivers;
        while (*pdriver) {
            const game_driver *driver = *pdriver;
            if (!(driver->flags & (GAME_IS_BIOS_ROOT | GAME_NO_STANDALONE))) {
                g_game_count++;
            }
            pdriver++;
        }
        g_gameinfos = (GameInfo *) osd_calloc
            (sizeof(GameInfo) * g_game_count);
        int driver_index = 0;
        int gameinfo_index = 0;
        while (true) {
            const game_driver *driver = drivers[driver_index];
            GameInfo *gameinfo = &(g_gameinfos[gameinfo_index]);
            if (driver == NULL) {
                break;
            }
            int *pHashValue;
            g_drivers_hash.Put(driver->name, /* returns */ pHashValue);
            *pHashValue = driver_index;
            if (!(driver->flags & (GAME_IS_BIOS_ROOT | GAME_NO_STANDALONE))) {
                gameinfo->converted = false;
                gameinfo->driver_index = driver_index;
                gameinfo->gameinfo_index = gameinfo_index;
                g_gameinfos_hash.Put(driver->name, /* returns */ pHashValue);
                *pHashValue = gameinfo_index;
                gameinfo_index++;
            }
            driver_index++;
        }
    }

    GameInfo *ret = &(g_gameinfos[gamenum]);

    if (converted && !ret->converted) {
        convert_game_info(ret);
        ret->converted = true;
    }

    return ret;
}


static GameInfo *get_gameinfo_helper(int gamenum, bool converted)
{
    pthread_mutex_lock(&g_mutex);

    GameInfo *ret = get_gameinfo_helper_locked(gamenum, converted);

    pthread_mutex_unlock(&g_mutex);

    return ret;
}


static const game_driver *get_game_driver(int gamenum)
{
    return drivers[get_gameinfo_helper(gamenum, false)->driver_index];
}


static const game_driver *get_game_driver_by_name(const char *name)
{
    int *pIndex = g_drivers_hash.Get(name);
    
    if (pIndex == NULL) {
        return NULL;
    }

    return drivers[*pIndex];
}


static GameInfo *get_gameinfo_locked(int gamenum)
{
    return get_gameinfo_helper_locked(gamenum, true);
}


static GameInfo *get_gameinfo(int gamenum)
{
    return get_gameinfo_helper(gamenum, true);
}


void LibMame_Games_Deinitialize()
{
    pthread_mutex_lock(&g_mutex);

    if (g_gameinfos) {
        for (int i = 0; i < g_game_count; i++) {
            GameInfo *gameinfo = &(g_gameinfos[i]);
            if (gameinfo->sound_samples) {
                osd_free(gameinfo->sound_samples);
            }
            if (gameinfo->chips) {
                osd_free(gameinfo->chips);
            }
            if (gameinfo->settings) {
                for (int j = 0; j < gameinfo->setting_count; j++) {
                    if (gameinfo->settings[j].value_names) {
                        osd_free((const char **) 
                                 gameinfo->settings[j].value_names);
                    }
                }
                osd_free(gameinfo->settings);
            }
            if (gameinfo->controller_set.normal_button_names) {
                osd_free((const char **) 
                         gameinfo->controller_set.normal_button_names);
            }
        }
        osd_free(g_gameinfos);
        g_gameinfos = 0;
    }

    g_game_count = 0;

    g_drivers_hash.Clear();
    g_gameinfos_hash.Clear();

    pthread_mutex_unlock(&g_mutex);
}


int LibMame_Get_Game_Count()
{
    /* Force get_gameinfo_helper() to ensure that all game infos are
       created */
    (void) get_gameinfo_helper(0, false);

    return g_game_count;
}


int LibMame_Get_Game_Number(const char *short_name)
{
    /* Force get_gameinfo_helper() to ensure that all game infos are
       created and hashed */
    (void) get_gameinfo_helper(0, false);
 
    int *pIndex = g_gameinfos_hash.Get(short_name);
    
    return pIndex ? *pIndex : -1;
}


int LibMame_Get_Game_Matches(const char *short_name, int num_matches,
                             int *gamenums)
{
    /* Force get_gameinfo_helper() to ensure that all game infos are
       created and hashed */
    (void) get_gameinfo_helper(0, false);

    const game_driver *results[num_matches];

    driver_list_get_approx_matches(drivers, short_name, num_matches, results);

    const game_driver **pdriver, **pend = &(results[num_matches]);
    
    int ret = 0;

    for (pdriver = results; pdriver < pend; pdriver++) {
        if (*pdriver == NULL) {
            break;
        }
        int *index = g_gameinfos_hash.Get((*pdriver)->name);
        if (index) {
            gamenums[ret++] = *index;
        }
    }

    return ret;
}


const char *LibMame_Get_Game_Short_Name(int gamenum)
{
    return get_game_driver(gamenum)->name;
}


const char *LibMame_Get_Game_Full_Name(int gamenum)
{
    return get_game_driver(gamenum)->description;
}


int LibMame_Get_Game_Year_Of_Release(int gamenum)
{
    return get_gameinfo(gamenum)->year_of_release;
}


const char *LibMame_Get_Game_CloneOf_Short_Name(int gamenum)
{
    const char *parent = get_game_driver(gamenum)->parent;
    /* Not sure why MAME uses "0" to mean "no parent" ... */
    if (parent && *parent && strcmp(parent, "0")) {
        /* If the parent is a BIOS, then it's not a clone */
        const game_driver *driver = get_game_driver_by_name(parent);
        if (driver && (driver->flags & GAME_IS_BIOS_ROOT)) {
            return NULL;
        }
        else {
            return parent;
        }
    }
    else {
        return NULL;
    }
}


const char *LibMame_Get_Game_Manufacturer(int gamenum)
{
    return get_game_driver(gamenum)->manufacturer;
}


int LibMame_Get_Game_WorkingFlags(int gamenum)
{
    return get_gameinfo(gamenum)->working_flags;
}


int LibMame_Get_Game_OrientationFlags(int gamenum)
{
    return get_gameinfo(gamenum)->orientation_flags;
}


LibMame_ScreenType LibMame_Get_Game_ScreenType(int gamenum)
{
    return get_gameinfo(gamenum)->screen_type;
}


LibMame_ScreenResolution LibMame_Get_Game_ScreenResolution(int gamenum)
{
    return get_gameinfo(gamenum)->screen_resolution;
}


int LibMame_Get_Game_ScreenRefreshRateHz(int gamenum)
{
    return get_gameinfo(gamenum)->screen_refresh_rate;
}


int LibMame_Get_Game_SoundChannels(int gamenum)
{
    return get_gameinfo(gamenum)->sound_channel_count;
}


int LibMame_Get_Game_SoundSamples_Count(int gamenum)
{
    return get_gameinfo(gamenum)->sound_samples_count;
}


int LibMame_Get_Game_SoundSamplesSource(int gamenum)

{
    return get_gameinfo(gamenum)->sound_samples_source;
}


bool LibMame_Get_Game_SoundSamplesIdenticalToSource(int gamenum)
{
    return (get_gameinfo(gamenum)->sound_samples == NULL);
}


const char *LibMame_Get_Game_SoundSampleFileName(int gamenum, int samplenum)
{
    const GameInfo *gameinfo = get_gameinfo(gamenum);
    if (gameinfo->sound_samples == NULL) {
        return LibMame_Get_Game_SoundSampleFileName
            (gameinfo->sound_samples_source, samplenum);
    }
    else {
        return gameinfo->sound_samples[samplenum];
    }
}


int LibMame_Get_Game_Chip_Count(int gamenum)
{
    return get_gameinfo(gamenum)->chip_count;
}


LibMame_ChipDescriptor LibMame_Get_Game_Chip(int gamenum, int chipnum)
{
    return get_gameinfo(gamenum)->chips[chipnum];
}


int LibMame_Get_Game_Setting_Count(int gamenum)
{
    return get_gameinfo(gamenum)->setting_count;
}


LibMame_SettingDescriptor LibMame_Get_Game_Setting(int gamenum, int settingnum)
{
    return get_gameinfo(gamenum)->settings[settingnum];
}


int LibMame_Get_Game_MaxSimultaneousPlayers(int gamenum)
{
    return get_gameinfo(gamenum)->max_simultaneous_players;
}


LibMame_ControllerSetDescriptor LibMame_Get_Game_ControllerSet(int gamenum)
{
    return get_gameinfo(gamenum)->controller_set;
}


const char *LibMame_Get_Game_SourceFileName(int gamenum)
{
    return get_gameinfo(gamenum)->source_file_name;
}
