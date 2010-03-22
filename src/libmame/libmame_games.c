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
    LibMame_PerPlayerControllersDescriptor perplayer_controllers;
    LibMame_SharedControllersDescriptor shared_controllers;
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

	for (port = ioportlist->first(); port; port = port->next) {
		for (field = port->fieldlist; field; field = field->next) {
			if (gameinfo->max_simultaneous_players < field->player) {
				gameinfo->max_simultaneous_players = field->player;
            }
            switch (field->type) {
            case IPT_JOYSTICK_LEFT:
            case IPT_JOYSTICK_RIGHT:
                switch (field->way) {
                case 2:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_JoystickHorizontal);
                    break;
                case 4:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_Joystick4Way);
                    break;
                default:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_Joystick8Way);
                    break;
                }
                break;
            case IPT_JOYSTICK_UP:
            case IPT_JOYSTICK_DOWN:
                switch (field->way) {
                case 2:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_JoystickVertical);
                    break;
                case 4:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_Joystick4Way);
                    break;
                default:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_Joystick8Way);
                    break;
                }
                break;
            case IPT_JOYSTICKRIGHT_LEFT:
            case IPT_JOYSTICKRIGHT_RIGHT:
            case IPT_JOYSTICKLEFT_LEFT:
            case IPT_JOYSTICKLEFT_RIGHT:
                switch (field->way) {
                case 2:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_DoubleJoystickHorizontal);
                    break;
                case 4:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_DoubleJoystick4Way);
                    break;
                default:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_DoubleJoystick8Way);
                    break;
                }
                break;
            case IPT_JOYSTICKRIGHT_UP:
            case IPT_JOYSTICKRIGHT_DOWN:
            case IPT_JOYSTICKLEFT_UP:
            case IPT_JOYSTICKLEFT_DOWN:
                switch (field->way) {
                case 2:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_DoubleJoystickVertical);
                    break;
                case 4:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_DoubleJoystick4Way);
                    break;
                default:
                    gameinfo->perplayer_controllers.controller_flags |= 
                        (1 << LibMame_ControllerType_DoubleJoystick8Way);
                    break;
                }
                break;
            case IPT_PADDLE:
                gameinfo->perplayer_controllers.controller_flags |= 
                    (1 << LibMame_ControllerType_Paddle);
                break;
            case IPT_DIAL:
                gameinfo->perplayer_controllers.controller_flags |= 
                    (1 << LibMame_ControllerType_Spinner);
                break;
            case IPT_TRACKBALL_X:
            case IPT_TRACKBALL_Y:
                gameinfo->perplayer_controllers.controller_flags |= 
                    (1 << LibMame_ControllerType_Trackball);
                break;
            case IPT_AD_STICK_X:
            case IPT_AD_STICK_Y:
                gameinfo->perplayer_controllers.controller_flags |= 
                    (1 << LibMame_ControllerType_JoystickAnalog);
                break;
            case IPT_LIGHTGUN_X:
            case IPT_LIGHTGUN_Y:
                gameinfo->perplayer_controllers.controller_flags |= 
                    (1 << LibMame_ControllerType_Lightgun);
                break;
            case IPT_PEDAL:
                gameinfo->perplayer_controllers.controller_flags |= 
                    (1 << LibMame_ControllerType_Pedal);
                break;
            case IPT_PEDAL2:
                gameinfo->perplayer_controllers.controller_flags |= 
                    (1 << LibMame_ControllerType_Pedal2);
                break;
            case IPT_PEDAL3:
                gameinfo->perplayer_controllers.controller_flags |= 
                    (1 << LibMame_ControllerType_Pedal3);
                break;

#define CASE_BUTTON(cs_field, iptname, enumvalue)                       \
            case iptname:                                               \
                gameinfo->perplayer_controllers. cs_field |=            \
                    (1 << enumvalue);                                   \
                break
                
#define CASE_BUTTON_AND_NAME(cs_field, iptname, n, enumvalue)           \
            case iptname:                                               \
                gameinfo->perplayer_controllers. cs_field |=            \
                    (1 << enumvalue);                                   \
                gameinfo->                                              \
                    perplayer_controllers.normal_button_names[n] =      \
                    field->name;                                        \
                break
                
#define CASE_OTHER_BUTTON(iptname, enumvalue)                           \
            case iptname:                                               \
                gameinfo->shared_controllers.other_button_flags |=      \
                    (1 << enumvalue);                                   \
                break

            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON1, 0,
                                 LibMame_NormalButtonType_1);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON2, 1,
                                 LibMame_NormalButtonType_2);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON3, 2,
                                 LibMame_NormalButtonType_3);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON4, 3,
                                 LibMame_NormalButtonType_4);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON5, 4,
                                 LibMame_NormalButtonType_5);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON6, 5,
                                 LibMame_NormalButtonType_6);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON7, 6,
                                 LibMame_NormalButtonType_7);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON8, 7,
                                 LibMame_NormalButtonType_8);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON9, 8,
                                 LibMame_NormalButtonType_9);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON10, 9,
                                 LibMame_NormalButtonType_10);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON11, 10,
                                 LibMame_NormalButtonType_11);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON12, 11,
                                 LibMame_NormalButtonType_12);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON13, 12,
                                 LibMame_NormalButtonType_13);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON14, 13,
                                 LibMame_NormalButtonType_14);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON15, 14,
                                 LibMame_NormalButtonType_15);
            CASE_BUTTON_AND_NAME(normal_button_flags, IPT_BUTTON16, 15,
                                 LibMame_NormalButtonType_16);

            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_A,
                        LibMame_MahjongButtonType_A);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_B,
                        LibMame_MahjongButtonType_B);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_C,
                        LibMame_MahjongButtonType_C);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_D,
                        LibMame_MahjongButtonType_D);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_E,
                        LibMame_MahjongButtonType_E);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_F,
                        LibMame_MahjongButtonType_F);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_G,
                        LibMame_MahjongButtonType_G);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_H,
                        LibMame_MahjongButtonType_H);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_I,
                        LibMame_MahjongButtonType_I);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_J,
                        LibMame_MahjongButtonType_J);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_K,
                        LibMame_MahjongButtonType_K);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_L,
                        LibMame_MahjongButtonType_L);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_M,
                        LibMame_MahjongButtonType_M);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_N,
                        LibMame_MahjongButtonType_N);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_O,
                        LibMame_MahjongButtonType_O);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_P,
                        LibMame_MahjongButtonType_P);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_Q,
                        LibMame_MahjongButtonType_Q);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_KAN,
                        LibMame_MahjongButtonType_Kan);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_PON,
                        LibMame_MahjongButtonType_Pon);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_CHI,
                        LibMame_MahjongButtonType_Chi);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_REACH,
                        LibMame_MahjongButtonType_Reach);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_RON,
                        LibMame_MahjongButtonType_Ron);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_BET,
                        LibMame_MahjongButtonType_Bet);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_LAST_CHANCE,
                        LibMame_MahjongButtonType_Last_Chance);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_SCORE,
                        LibMame_MahjongButtonType_Score);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_DOUBLE_UP,
                        LibMame_MahjongButtonType_Double_Up);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_FLIP_FLOP,
                        LibMame_MahjongButtonType_Flip_Flop);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_BIG,
                        LibMame_MahjongButtonType_Big);
            CASE_BUTTON(mahjong_button_flags, IPT_MAHJONG_SMALL,
                        LibMame_MahjongButtonType_Small);
            
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_A,
                        LibMame_HanafudaButtonType_A);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_B,
                        LibMame_HanafudaButtonType_B);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_C,
                        LibMame_HanafudaButtonType_C);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_D,
                        LibMame_HanafudaButtonType_D);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_E,
                        LibMame_HanafudaButtonType_E);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_F,
                        LibMame_HanafudaButtonType_F);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_G,
                        LibMame_HanafudaButtonType_G);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_H,
                        LibMame_HanafudaButtonType_H);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_YES,
                        LibMame_HanafudaButtonType_Yes);
            CASE_BUTTON(hanafuda_button_flags, IPT_HANAFUDA_NO,
                        LibMame_HanafudaButtonType_No);

            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_HIGH,
                        LibMame_GamblingButtonType_High);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_LOW,
                        LibMame_GamblingButtonType_Low);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_HALF,
                        LibMame_GamblingButtonType_Half);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_DEAL,
                        LibMame_GamblingButtonType_Deal);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_D_UP,
                        LibMame_GamblingButtonType_D_Up);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_TAKE,
                        LibMame_GamblingButtonType_Take);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_STAND,
                        LibMame_GamblingButtonType_Stand);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_BET,
                        LibMame_GamblingButtonType_Bet);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_KEYIN,
                        LibMame_GamblingButtonType_Keyin);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_KEYOUT,
                        LibMame_GamblingButtonType_Keyout);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_PAYOUT,
                        LibMame_GamblingButtonType_Payout);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_DOOR,
                        LibMame_GamblingButtonType_Door);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_SERVICE,
                        LibMame_GamblingButtonType_Service);
            CASE_BUTTON(gambling_button_flags, IPT_GAMBLE_BOOK,
                        LibMame_GamblingButtonType_Book);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_HOLD1,
                        LibMame_GamblingButtonType_Hold1);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_HOLD2,
                        LibMame_GamblingButtonType_Hold2);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_HOLD3,
                        LibMame_GamblingButtonType_Hold3);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_HOLD4,
                        LibMame_GamblingButtonType_Hold4);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_HOLD5,
                        LibMame_GamblingButtonType_Hold5);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_CANCEL,
                        LibMame_GamblingButtonType_Cancel);
            CASE_BUTTON(gambling_button_flags, IPT_POKER_BET,
                        LibMame_GamblingButtonType_Bet);
            CASE_BUTTON(gambling_button_flags, IPT_SLOT_STOP1,
                        LibMame_GamblingButtonType_Stop1);
            CASE_BUTTON(gambling_button_flags, IPT_SLOT_STOP2,
                        LibMame_GamblingButtonType_Stop2);
            CASE_BUTTON(gambling_button_flags, IPT_SLOT_STOP3,
                        LibMame_GamblingButtonType_Stop3);
            CASE_BUTTON(gambling_button_flags, IPT_SLOT_STOP4,
                        LibMame_GamblingButtonType_Stop4);
            CASE_BUTTON(gambling_button_flags, IPT_SLOT_STOP_ALL,
                        LibMame_GamblingButtonType_Stop_All);

            CASE_OTHER_BUTTON(IPT_COIN1, LibMame_OtherButtonType_Coin1);
            CASE_OTHER_BUTTON(IPT_COIN2, LibMame_OtherButtonType_Coin2);
            CASE_OTHER_BUTTON(IPT_COIN3, LibMame_OtherButtonType_Coin3);
            CASE_OTHER_BUTTON(IPT_COIN4, LibMame_OtherButtonType_Coin4);
            CASE_OTHER_BUTTON(IPT_COIN5, LibMame_OtherButtonType_Coin5);
            CASE_OTHER_BUTTON(IPT_COIN6, LibMame_OtherButtonType_Coin6);
            CASE_OTHER_BUTTON(IPT_COIN7, LibMame_OtherButtonType_Coin7);
            CASE_OTHER_BUTTON(IPT_COIN8, LibMame_OtherButtonType_Coin8);
            CASE_OTHER_BUTTON(IPT_BILL1, LibMame_OtherButtonType_Bill1);
            CASE_OTHER_BUTTON(IPT_START1, LibMame_OtherButtonType_Start1);
            CASE_OTHER_BUTTON(IPT_START2, LibMame_OtherButtonType_Start2);
            CASE_OTHER_BUTTON(IPT_START3, LibMame_OtherButtonType_Start3);
            CASE_OTHER_BUTTON(IPT_START4, LibMame_OtherButtonType_Start4);
            CASE_OTHER_BUTTON(IPT_START5, LibMame_OtherButtonType_Start5);
            CASE_OTHER_BUTTON(IPT_START6, LibMame_OtherButtonType_Start6);
            CASE_OTHER_BUTTON(IPT_START7, LibMame_OtherButtonType_Start7);
            CASE_OTHER_BUTTON(IPT_START8, LibMame_OtherButtonType_Start8);
            CASE_OTHER_BUTTON(IPT_SERVICE1, LibMame_OtherButtonType_Service1);
            CASE_OTHER_BUTTON(IPT_SERVICE2, LibMame_OtherButtonType_Service2);
            CASE_OTHER_BUTTON(IPT_SERVICE3, LibMame_OtherButtonType_Service3);
            CASE_OTHER_BUTTON(IPT_SERVICE4, LibMame_OtherButtonType_Service4);
            CASE_OTHER_BUTTON(IPT_SERVICE, LibMame_OtherButtonType_Service);
            CASE_OTHER_BUTTON(IPT_TILT, LibMame_OtherButtonType_Tilt);
            CASE_OTHER_BUTTON(IPT_INTERLOCK, LibMame_OtherButtonType_Interlock);
            CASE_OTHER_BUTTON(IPT_VOLUME_UP, LibMame_OtherButtonType_Volume_Up);
            CASE_OTHER_BUTTON(IPT_VOLUME_DOWN,
                              LibMame_OtherButtonType_Volume_Down);
            }
        }
    }
    gameinfo->max_simultaneous_players++;
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


int32_t LibMame_Get_Game_Year_Of_Release(int gamenum)
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


LibMame_PerPlayerControllersDescriptor LibMame_Get_Game_PerPlayerControllers
    (int gamenum)
{
    return get_gameinfo(gamenum)->perplayer_controllers;
}


LibMame_SharedControllersDescriptor LibMame_Get_Game_SharedControllers
    (int gamenum)
{
    return get_gameinfo(gamenum)->shared_controllers;
}


const char *LibMame_Get_Game_SourceFileName(int gamenum)
{
    return get_gameinfo(gamenum)->source_file_name;
}
