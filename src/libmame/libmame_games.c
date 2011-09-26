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

#include "hashtable.h"
#include "emu.h"
#include "emuopts.h"
#include "config.h"
#include "hash.h"
#include "libmame.h"
#include "osdcore.h"
#include "sound/samples.h"
#include <pthread.h>
#include <string.h>

// Sorry MAME, you don't get to tell me I can't use C++ delete
#undef delete

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
    int working_flags;
    LibMame_OrientationType orientation;
    LibMame_ScreenType screen_type;
    LibMame_ScreenResolution screen_resolution;
    float screen_refresh_rate;
    int sound_channel_count;
    int sound_samples_count;
    int sound_samples_source;
    const char **sound_samples;
    int chip_count;
    LibMame_Chip *chips;
    int dipswitch_count;
    LibMame_Dipswitch *dipswitches;
    int max_simultaneous_players;
    LibMame_AllControllers controllers;
    int biosset_count;
    LibMame_BiosSet *biossets;
    int rom_count;
    LibMame_Image *roms;
    int hdd_count;
    LibMame_Image *hdds;
    char source_file_name[SOURCE_FILE_NAME_MAX];
} GameInfo;

// MAME is getting really ugly in its use of C++.  This is very unfortunate.
class MameDriversWrapper
{
public:

    MameDriversWrapper()
        : driversM(0)
    {
        pthread_mutex_init(&mutexM, 0);
    }

    ~MameDriversWrapper()
    {
        pthread_mutex_destroy(&mutexM);
    }

    driver_enumerator &Get()
    {
        pthread_mutex_lock(&mutexM);
        
        if (!driversM) {
            emu_options options;
            driversM = new driver_enumerator(options);
        }

        pthread_mutex_unlock(&mutexM);

        return *driversM;
    }

private:

    pthread_mutex_t mutexM;
    driver_enumerator *driversM;
};

static MameDriversWrapper g_drivers;

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


static char *copy_string(const char *string)
{
    int len = strlen(string);

    char *ret = (char *) osd_malloc(len + 1);

    strcpy(ret, string);

    return ret;
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


static void convert_orientation(const game_driver *driver,
                                GameInfo *gameinfo)
{
    switch (driver->flags & ORIENTATION_MASK) {
    case ROT270:
        gameinfo->orientation = LibMame_OrientationType_270;
        break;
    case ROT180:
        gameinfo->orientation = LibMame_OrientationType_180;
        break;
    case ROT90:
        gameinfo->orientation = LibMame_OrientationType_90;
        break;
    default:
        gameinfo->orientation = LibMame_OrientationType_Normal;
        break;
    }
}


static void convert_screen_info(const machine_config *machineconfig,
                                GameInfo *gameinfo)
{
    /* We assume that all screens are the same; and in any case, only report
       on the first screen, which is assumed to be the primary screen */
    const screen_device *screenconfig = machineconfig->first_screen();
    if (screenconfig != NULL) {
        switch (screenconfig->screen_type()) {
        case SCREEN_TYPE_RASTER:
            gameinfo->screen_type = LibMame_ScreenType_Raster;
            break;
        case SCREEN_TYPE_LCD:
            gameinfo->screen_type = LibMame_ScreenType_LCD;
            break;
        case SCREEN_TYPE_VECTOR:
            gameinfo->screen_type = LibMame_ScreenType_Vector;
            break;
        case SCREEN_TYPE_INVALID:
            // MAME doesn't actually use this; not sure why they bother to
            // define it, it only creates confusion and difficulty
            gameinfo->screen_type = LibMame_ScreenType_Raster;
            break;
        }
        const rectangle &visarea = screenconfig->visible_area();
        gameinfo->screen_resolution.width = 
            ((visarea.max_x - visarea.min_x) + 1);
        gameinfo->screen_resolution.height = 
            ((visarea.max_y - visarea.min_y) + 1);
        gameinfo->screen_refresh_rate = 
            ATTOSECONDS_TO_HZ(screenconfig->refresh_attoseconds());
    }
}


static void convert_sound_channels(const machine_config *machineconfig,
                                   GameInfo *gameinfo)
{
    gameinfo->sound_channel_count = 
        machineconfig->devicelist().count(SPEAKER);
}


static void convert_sound_samples_helper(const machine_config *machineconfig,
                                         GameInfo *gameinfo, bool just_count)
{
    const char **destsample = gameinfo->sound_samples;

    for (const device_t *device = machineconfig->devicelist().first(SAMPLES);
         device; device = device->typenext()) {

        const char * const *samplenames = 
            ((const samples_interface *) device->static_config())->samplenames;

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
	const device_execute_interface *execi;
	for (bool is_valid = machineconfig->devicelist().first(execi); is_valid;
         is_valid = execi->next(execi)) {
        gameinfo->chip_count++;
    }

	const device_sound_interface *soundi;
	for (bool is_valid = machineconfig->devicelist().first(soundi); is_valid;
         is_valid = soundi->next(soundi)) {
        gameinfo->chip_count++;
    }

    if (gameinfo->chip_count == 0) {
        return;
    }

    gameinfo->chips = (LibMame_Chip *) osd_calloc
        (sizeof(LibMame_Chip) * gameinfo->chip_count);
    
    LibMame_Chip *descriptor = gameinfo->chips;

	for (bool is_valid = machineconfig->devicelist().first(execi); is_valid;
         is_valid = execi->next(execi)) {
        descriptor->is_sound = false;
        descriptor->tag = copy_string(execi->device().tag());
        descriptor->name = copy_string(execi->device().name());
        descriptor->clock_hz = execi->device().clock();
        descriptor++;
    }        
    
	for (bool is_valid = machineconfig->devicelist().first(soundi); is_valid;
         is_valid = soundi->next(soundi)) {
        descriptor->is_sound = true;
        descriptor->tag = copy_string(soundi->device().tag());
        descriptor->name = copy_string(soundi->device().name());
        descriptor->clock_hz = soundi->device().clock();
        descriptor++;
    }        
}


static void convert_settings(const ioport_list *ioportlist,
                             GameInfo *gameinfo)
{
	input_port_config *port;
	const input_field_config *field;
    
	for (port = ioportlist->first(); port; port = port->next()) {
		for (field = port->fieldlist().first(); field; field = field->next()) {
            if (field->type == IPT_DIPSWITCH) {
                gameinfo->dipswitch_count++;
            }
        }
    }

    if (gameinfo->dipswitch_count == 0) {
        return;
    }

    gameinfo->dipswitches = (LibMame_Dipswitch *)
        osd_calloc(sizeof(LibMame_Dipswitch) * gameinfo->dipswitch_count);

    LibMame_Dipswitch *desc = gameinfo->dipswitches;

	for (port = ioportlist->first(); port; port = port->next()) {
		for (field = port->fieldlist().first(); field; field = field->next()) {
            if (field->type != IPT_DIPSWITCH) {
                continue;
            }

            desc->name = input_field_name(field);
            desc->tag = field->port().tag();
            desc->mask = field->mask;
            const input_setting_config *setting;
            for (setting = field->settinglist().first(); setting; 
                 setting = setting->next()) {
                desc->value_count++;
            }

            if (desc->value_count == 0) {
                desc++;
                continue;
            }

            desc->value_names = (const char * const *) osd_malloc
                (sizeof(const char *) * desc->value_count);
            const char **value_names = (const char **) desc->value_names;
            int index = 0;
            for (setting = field->settinglist().first(); setting; 
                 setting = setting->next()) {
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
	input_port_config *port;
	const input_field_config *field;

    int special_count = 0;

	for (port = ioportlist->first(); port; port = port->next()) {
		for (field = port->fieldlist().first(); field; field = field->next()) {
            if (field->flags & FIELD_FLAG_UNUSED) {
                continue;
            }
            // Skip cocktail ports, for the time being; at some point in the
            // future this code should store the cocktail port specifications
            // in a different set of perplayer configs.
            if (field->flags & FIELD_FLAG_COCKTAIL) {
                continue;
            }
			if (gameinfo->max_simultaneous_players < field->player) {
				gameinfo->max_simultaneous_players = field->player;
            }
            // Only process player 1 controls; it is assumed that all controls
            // are identical between all players
            if (field->player != 0) {
                continue;
            }
            switch (field->type) {
            case IPT_OTHER:
                if (field->name && 
                    (special_count < LibMame_SpecialButtonTypeCount)) {
                    gameinfo->controllers.shared.special_button_flags |=
                        (1 << special_count);
                    gameinfo->controllers.shared.
                        special_button_names[special_count++] = field->name;
                }
                break;
            case IPT_JOYSTICK_LEFT:
            case IPT_JOYSTICK_RIGHT:
            case IPT_JOYSTICKLEFT_LEFT:
            case IPT_JOYSTICKLEFT_RIGHT:
                switch (field->way) {
                case 2:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_LeftHorizontalJoystick);
                    break;
                case 4:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_Left4WayJoystick);
                    break;
                default:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_Left8WayJoystick);
                    break;
                }
                break;
            case IPT_JOYSTICK_UP:
            case IPT_JOYSTICK_DOWN:
            case IPT_JOYSTICKLEFT_UP:
            case IPT_JOYSTICKLEFT_DOWN:
                switch (field->way) {
                case 2:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_LeftVerticalJoystick);
                    break;
                case 4:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_Left4WayJoystick);
                    break;
                default:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_Left8WayJoystick);
                    break;
                }
                break;
            case IPT_JOYSTICKRIGHT_LEFT:
            case IPT_JOYSTICKRIGHT_RIGHT:
                switch (field->way) {
                case 2:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_RightHorizontalJoystick);
                    break;
                case 4:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_Right4WayJoystick);
                    break;
                default:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_Right8WayJoystick);
                    break;
                }
                break;
            case IPT_JOYSTICKRIGHT_UP:
            case IPT_JOYSTICKRIGHT_DOWN:
                switch (field->way) {
                case 2:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_RightVerticalJoystick);
                    break;
                case 4:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_Right4WayJoystick);
                    break;
                default:
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_Right8WayJoystick);
                    break;
                }
                break;
            case IPT_PADDLE:
                gameinfo->controllers.per_player.controller_flags |= 
                    (1 << LibMame_ControllerType_Paddle);
                break;
            case IPT_DIAL:
                gameinfo->controllers.per_player.controller_flags |= 
                    (1 << LibMame_ControllerType_Spinner);
                break;
            case IPT_TRACKBALL_X:
            case IPT_TRACKBALL_Y:
                gameinfo->controllers.per_player.controller_flags |= 
                    (1 << LibMame_ControllerType_Trackball);
                break;
            case IPT_AD_STICK_X:
                if (gameinfo->controllers.per_player.controller_flags &
                    (1 << LibMame_ControllerType_AnalogVerticalJoystick)) {
                    gameinfo->controllers.per_player.controller_flags &=
                        ~(1 << LibMame_ControllerType_AnalogVerticalJoystick);
                    gameinfo->controllers.per_player.controller_flags |=
                        (1 << LibMame_ControllerType_Analog8WayJoystick);
                }
                else {
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_AnalogHorizontalJoystick);
                }
                break;
            case IPT_AD_STICK_Y:
                if (gameinfo->controllers.per_player.controller_flags &
                    (1 << LibMame_ControllerType_AnalogHorizontalJoystick)) {
                    gameinfo->controllers.per_player.controller_flags &=
                        ~(1 << LibMame_ControllerType_AnalogHorizontalJoystick);
                    gameinfo->controllers.per_player.controller_flags |=
                        (1 << LibMame_ControllerType_Analog8WayJoystick);
                }
                else {
                    gameinfo->controllers.per_player.controller_flags |= 
                        (1 << LibMame_ControllerType_AnalogVerticalJoystick);
                }
                break;
            case IPT_LIGHTGUN_X:
            case IPT_LIGHTGUN_Y:
                gameinfo->controllers.per_player.controller_flags |= 
                    (1 << LibMame_ControllerType_Lightgun);
                break;
            case IPT_PEDAL:
                gameinfo->controllers.per_player.controller_flags |= 
                    (1 << LibMame_ControllerType_Pedal);
                break;
            case IPT_PEDAL2:
                gameinfo->controllers.per_player.controller_flags |= 
                    (1 << LibMame_ControllerType_Pedal2);
                break;
            case IPT_PEDAL3:
                gameinfo->controllers.per_player.controller_flags |= 
                    (1 << LibMame_ControllerType_Pedal3);
                break;

#define CASE_BUTTON(cs_field, iptname, enumvalue)                       \
            case iptname:                                               \
                gameinfo->controllers.per_player. cs_field |=           \
                    (1 << enumvalue);                                   \
                break
                
#define CASE_BUTTON_AND_NAME(cs_field, iptname, n, enumvalue)           \
            case iptname:                                               \
                gameinfo->controllers.per_player. cs_field |=           \
                    (1 << enumvalue);                                   \
                gameinfo->                                              \
                    controllers.per_player.normal_button_names[n] =     \
                    field->name;                                        \
                break
                
#define CASE_SHARED_BUTTON(iptname, enumvalue)                          \
            case iptname:                                               \
                gameinfo->controllers.shared.shared_button_flags |=     \
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

            CASE_SHARED_BUTTON(IPT_COIN1, LibMame_SharedButtonType_Coin1);
            CASE_SHARED_BUTTON(IPT_COIN2, LibMame_SharedButtonType_Coin2);
            CASE_SHARED_BUTTON(IPT_COIN3, LibMame_SharedButtonType_Coin3);
            CASE_SHARED_BUTTON(IPT_COIN4, LibMame_SharedButtonType_Coin4);
            CASE_SHARED_BUTTON(IPT_COIN5, LibMame_SharedButtonType_Coin5);
            CASE_SHARED_BUTTON(IPT_COIN6, LibMame_SharedButtonType_Coin6);
            CASE_SHARED_BUTTON(IPT_COIN7, LibMame_SharedButtonType_Coin7);
            CASE_SHARED_BUTTON(IPT_COIN8, LibMame_SharedButtonType_Coin8);
            CASE_SHARED_BUTTON(IPT_BILL1, LibMame_SharedButtonType_Bill1);
            CASE_SHARED_BUTTON(IPT_START1, LibMame_SharedButtonType_Start1);
            CASE_SHARED_BUTTON(IPT_START2, LibMame_SharedButtonType_Start2);
            CASE_SHARED_BUTTON(IPT_START3, LibMame_SharedButtonType_Start3);
            CASE_SHARED_BUTTON(IPT_START4, LibMame_SharedButtonType_Start4);
            CASE_SHARED_BUTTON(IPT_START5, LibMame_SharedButtonType_Start5);
            CASE_SHARED_BUTTON(IPT_START6, LibMame_SharedButtonType_Start6);
            CASE_SHARED_BUTTON(IPT_START7, LibMame_SharedButtonType_Start7);
            CASE_SHARED_BUTTON(IPT_START8, LibMame_SharedButtonType_Start8);
            CASE_SHARED_BUTTON(IPT_SERVICE1, LibMame_SharedButtonType_Service1);
            CASE_SHARED_BUTTON(IPT_SERVICE2, LibMame_SharedButtonType_Service2);
            CASE_SHARED_BUTTON(IPT_SERVICE3, LibMame_SharedButtonType_Service3);
            CASE_SHARED_BUTTON(IPT_SERVICE4, LibMame_SharedButtonType_Service4);
            CASE_SHARED_BUTTON(IPT_SERVICE, LibMame_SharedButtonType_Service);
            CASE_SHARED_BUTTON(IPT_TILT, LibMame_SharedButtonType_Tilt);
            CASE_SHARED_BUTTON(IPT_INTERLOCK,
                               LibMame_SharedButtonType_Interlock);
            CASE_SHARED_BUTTON(IPT_VOLUME_UP,
                               LibMame_SharedButtonType_Volume_Up);
            CASE_SHARED_BUTTON(IPT_VOLUME_DOWN,
                              LibMame_SharedButtonType_Volume_Down);
            }
        }
    }
    gameinfo->max_simultaneous_players++;
}


static void convert_image_info(const game_driver *driver, 
                               const machine_config *machineconfig,
                               GameInfo *gameinfo)
{
    /* Convert the roms and hdd images */

    /* Iterate through the sources ... */
    for (const rom_source *source = rom_first_source(*machineconfig); 
         source; source = rom_next_source(*source)) {
        /* Iterate through the regions */
        for (const rom_entry *region = rom_first_region(*source); region;
             region = rom_next_region(region)) {
            /* iterate through ROM entries */
            for (const rom_entry *rom = rom_first_file(region); rom;
                 rom = rom_next_file(rom)) {
                if (ROMREGION_ISDISKDATA(region)) {
                    gameinfo->hdd_count++;
                }
                else {
                    gameinfo->rom_count++;
                }
            }
        }
    }

    /* Allocate them */
    if (gameinfo->rom_count) {
        gameinfo->roms = (LibMame_Image *) osd_malloc
            (sizeof(LibMame_Image) * gameinfo->rom_count);
    }
    if (gameinfo->hdd_count) {
        gameinfo->hdds = (LibMame_Image *) osd_malloc
            (sizeof(LibMame_Image) * gameinfo->hdd_count);
    }

    /* Fill them in, and at the same time, build up the BIOS sets */
    Hash::Table<Hash::StringKey, LibMame_BiosSet> htBiosSets;
    LibMame_Image *rom_image = gameinfo->roms, *hdd_image = gameinfo->hdds;
    int current_rom_index = 0;
    for (const rom_source *source = rom_first_source(*machineconfig);
         source; source = rom_next_source(*source)) {
        /* Iterate through the regions */
        for (const rom_entry *region = rom_first_region(*source); 
             region; region = rom_next_region(region)) {
            /* iterate through ROM entries */
            for (const rom_entry *rom = rom_first_file(region); rom;
                 rom = rom_next_file(rom)) {
                LibMame_Image *image;
                bool is_disk = ROMREGION_ISDISKDATA(region);
                if (is_disk) {
                    image = hdd_image;
                    hdd_image++;
                }
                else {
                    image = rom_image;
                    rom_image++;
                    current_rom_index++;
                }
                image->name = ROM_GETNAME(rom);
                hash_collection hashes(ROM_GETHASHDATA(rom));
                image->status = (hashes.flag(hash_collection::FLAG_BAD_DUMP) ?
                                 LibMame_ImageStatus_BadDump :
                                 hashes.flag(hash_collection::FLAG_NO_DUMP) ?
                                 LibMame_ImageStatus_NoDump :
                                 LibMame_ImageStatus_GoodDump);
                image->is_optional = ((is_disk && DISK_ISOPTIONAL(rom)) ||
                                      (!is_disk && ROM_ISOPTIONAL(rom)));
                image->size_if_known = is_disk ? 0 : rom_file_size(rom);
                image->clone_of_game = 0;
                image->clone_of_rom = 0;

                int clone_of = g_drivers.Get().find(driver->parent);
                if (clone_of != -1) {
                    for (const rom_source *psource = rom_first_source
                             (g_drivers.Get().config(clone_of)); psource;
                         psource = rom_next_source(*psource)) {
                        for (const rom_entry *pregion = 
                                 rom_first_region(*psource); pregion;
                             pregion = rom_next_region(pregion)) {
                            for (const rom_entry *prom = 
                                     rom_first_file(pregion); prom; 
                                 prom = rom_next_file(prom)) {
                                if (hashes == 
                                    hash_collection(ROM_GETHASHDATA(prom))) {
                                    image->clone_of_game = driver->parent;
                                    image->clone_of_rom = ROM_GETNAME(prom);
                                    break;
                                }
                            }
                        }
                    }
                }
                image->crc = image->sha1 = image->md5 = 0;
                if (!hashes.flag(hash_collection::FLAG_NO_DUMP)) {
                    for (hash_base *hash = hashes.first(); hash != NULL; 
                         hash = hash->next()) {
                        char id = hash->id();
                        astring tempstr;
                        const char *value = hash->string(tempstr);
                        if (id == hash_collection::HASH_CRC) {
                            if (!image->crc) {
                                image->crc = copy_string(value);
                            }
                            // Else weirdness???
                        }
                        else if (id == hash_collection::HASH_SHA1) {
                            if (!image->sha1) {
                                image->sha1 = copy_string(value);
                            }
                            // Else weirdness???
                        }
                        else if (id == hash_collection::HASH_MD5) {
                            if (!image->md5) {
                                image->md5 = copy_string(value);
                            }
                            // Else weirdness???
                        }
                        // Else weirdness???
                    }
                }
                /* This is really weird but it's what MAME does. */
                if (!is_disk && ROM_GETBIOSFLAGS(rom)) {
                    /* scan backwards through the ROM entries */
                    for (const rom_entry *brom = rom - 1; brom != driver->rom; 
                         brom--) {
                        if (ROMENTRY_ISSYSTEM_BIOS(brom)) {
                            /* This is a ROM for this BIOS set.  Look the BIOS
                               up in the hashtable, and create if if
                               necessary; then add this ROM to its list of
                               ROMs */
                            const char *bios_set_name = ROM_GETNAME(brom);
                            LibMame_BiosSet *bios_set;
                            if (!htBiosSets.Put(bios_set_name,
                                                /* returns */ bios_set)) {
                                /* New BIOS set, so set it up */
                                bios_set->name = bios_set_name;
                                bios_set->description = ROM_GETHASHDATA(brom);
                                bios_set->is_default = 
                                    (ROM_GETBIOSFLAGS(brom) == 1);
                                bios_set->rom_count = 0;
                                /* Allocate the maximum possible, which is
                                   almost certainly more than needed, but is
                                   easy */ 
                                bios_set->rom_numbers = (const int *) 
                                    osd_malloc(sizeof(int) * 
                                               gameinfo->rom_count);
                            }
                            ((int *) (bios_set->rom_numbers))
                                [bios_set->rom_count++] = current_rom_index - 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    /* Allocate the BIOS sets */
    if ((gameinfo->biosset_count = htBiosSets.Count())) {
        gameinfo->biossets = (LibMame_BiosSet *) osd_malloc
            (sizeof(LibMame_BiosSet) * gameinfo->biosset_count);
        LibMame_BiosSet *pset = gameinfo->biossets;
        /* Fill them in */
        Hash::Table<Hash::StringKey, LibMame_BiosSet>::Iterator 
            iter(htBiosSets);
        while (iter.HasCurrent()) {
            *pset = iter.GetCurrentValue();
            pset++;
            iter.Advance();
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
    const game_driver &driver = g_drivers.Get().driver(gameinfo->driver_index);

	machine_config &machineconfig = g_drivers.Get().config(gameinfo->driver_index);
    ioport_list ioportlist;
    astring errors;
	for (device_t *device = machineconfig.devicelist().first(); device;
         device = device->next()) {
		input_port_list_init(*device, ioportlist, errors);
    }
    /* Mame's code assumes the above succeeds, we will too */

    convert_year(&driver, gameinfo);
    convert_working_flags(&driver, gameinfo);
    convert_orientation(&driver, gameinfo);
    convert_screen_info(&machineconfig, gameinfo);
    convert_sound_channels(&machineconfig, gameinfo);
    convert_sound_samples(&machineconfig, gameinfo);
    convert_chips(&machineconfig, gameinfo);
    convert_settings(&ioportlist, gameinfo);
    convert_controllers(&ioportlist, gameinfo);
    convert_image_info(&driver, &machineconfig, gameinfo);
    convert_source_file_name(&driver, gameinfo);
}


static GameInfo *get_gameinfo_helper_locked(int gamenum, bool converted)
{
    if (g_game_count == 0) {
        g_drivers.Get().reset();
        while (g_drivers.Get().next()) {
            const game_driver &driver = g_drivers.Get().driver();
            if (!(driver.flags & (GAME_IS_BIOS_ROOT | GAME_NO_STANDALONE |
                                  GAME_MECHANICAL))) {
                g_game_count++;
            }
        }
        g_gameinfos = (GameInfo *) osd_calloc
            (sizeof(GameInfo) * g_game_count);
        int driver_count = g_drivers.Get().total();
        int gameinfo_index = 0;
        for (int driver_index = 0; driver_index < driver_count; 
             driver_index++) {
            const game_driver &driver = g_drivers.Get().driver(driver_index);
            GameInfo *gameinfo = &(g_gameinfos[gameinfo_index]);
            int *pHashValue;
            g_drivers_hash.Put(driver.name, /* returns */ pHashValue);
            *pHashValue = driver_index;
            if (!(driver.flags & (GAME_IS_BIOS_ROOT | GAME_NO_STANDALONE |
                                  GAME_MECHANICAL))) {
                gameinfo->converted = false;
                gameinfo->driver_index = driver_index;
                gameinfo->gameinfo_index = gameinfo_index;
                g_gameinfos_hash.Put(driver.name, /* returns */ pHashValue);
                *pHashValue = gameinfo_index;
                gameinfo_index++;
            }
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


static const game_driver &get_game_driver(int gamenum)
{
    return g_drivers.Get().driver(get_gameinfo_helper(gamenum, false)->driver_index);
}


static const game_driver *get_game_driver_by_name(const char *name)
{
    int *pIndex = g_drivers_hash.Get(name);
    
    if (pIndex == NULL) {
        return NULL;
    }
    
    return &(g_drivers.Get().driver(*pIndex));
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
                for (int j = 0; j < gameinfo->chip_count; j++) {
                    osd_free((char *) (gameinfo->chips[j].tag));
                    osd_free((char *) (gameinfo->chips[j].name));
                }
                osd_free(gameinfo->chips);
            }
            if (gameinfo->dipswitches) {
                for (int j = 0; j < gameinfo->dipswitch_count; j++) {
                    if (gameinfo->dipswitches[j].value_names) {
                        osd_free((const char **) 
                                 gameinfo->dipswitches[j].value_names);
                    }
                }
                osd_free(gameinfo->dipswitches);
            }
            if (gameinfo->biossets) {
                for (int j = 0; j < gameinfo->biosset_count; j++) {
                    osd_free((int *) (gameinfo->biossets[j].rom_numbers));
                }
                osd_free(gameinfo->biossets);
            }
            if (gameinfo->roms) {
                for (int j = 0; j < gameinfo->rom_count; j++) {
                    LibMame_Image *img = &(gameinfo->roms[j]);
                    if (img->crc) {
                        osd_free((char *) (img->crc));
                    }
                    if (img->sha1) {
                        osd_free((char *) (img->sha1));
                    }
                    if (img->md5) {
                        osd_free((char *) (img->md5));
                    }
                }
                osd_free(gameinfo->roms);
            }
            if (gameinfo->hdds) {
                for (int j = 0; j < gameinfo->hdd_count; j++) {
                    LibMame_Image *img = &(gameinfo->hdds[j]);
                    if (img->crc) {
                        osd_free((char *) (img->crc));
                    }
                    if (img->sha1) {
                        osd_free((char *) (img->sha1));
                    }
                    if (img->md5) {
                        osd_free((char *) (img->md5));
                    }
                }
                osd_free(gameinfo->hdds);
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

    if (num_matches > 65536) {
        num_matches = 65536;
    }

    int results[num_matches], ret = 0;

    g_drivers.Get().find_approximate_matches(short_name, num_matches, results);

    for (int i = 0; i < num_matches; i++) {
        if (results[i] == -1) {
            break;
        }
        const game_driver &driver = g_drivers.Get().driver(results[i]);
        int *index = g_gameinfos_hash.Get(driver.name);
        if (index) {
            gamenums[ret++] = *index;
        }
    }

    return ret;
}


const char *LibMame_Get_Game_Short_Name(int gamenum)
{
    return get_game_driver(gamenum).name;
}


const char *LibMame_Get_Game_Full_Name(int gamenum)
{
    return get_game_driver(gamenum).description;
}


int32_t LibMame_Get_Game_Year_Of_Release(int gamenum)
{
    return get_gameinfo(gamenum)->year_of_release;
}


int LibMame_Get_Game_CloneOf(int gamenum)
{
    const char *parent = get_game_driver(gamenum).parent;
    /* Not sure why MAME uses "0" to mean "no parent" ... */
    if (parent && *parent && strcmp(parent, "0")) {
        /* If the parent is a BIOS, then it's not a clone */
        const game_driver *driver = get_game_driver_by_name(parent);
        if (driver && (driver->flags & GAME_IS_BIOS_ROOT)) {
            return -1;
        }
        else {
            int *index = g_gameinfos_hash.Get(parent);
            return (index == NULL) ? -1 : *index;
        }
    }
    else {
        return -1;
    }
}


const char *LibMame_Get_Game_Manufacturer(int gamenum)
{
    return get_game_driver(gamenum).manufacturer;
}


int LibMame_Get_Game_WorkingFlags(int gamenum)
{
    return get_gameinfo(gamenum)->working_flags;
}


LibMame_OrientationType LibMame_Get_Game_Orientation(int gamenum)
{
    return get_gameinfo(gamenum)->orientation;
}


LibMame_ScreenType LibMame_Get_Game_ScreenType(int gamenum)
{
    return get_gameinfo(gamenum)->screen_type;
}


LibMame_ScreenResolution LibMame_Get_Game_ScreenResolution(int gamenum)
{
    return get_gameinfo(gamenum)->screen_resolution;
}


float LibMame_Get_Game_ScreenRefreshRateHz(int gamenum)
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


LibMame_Chip LibMame_Get_Game_Chip(int gamenum, int chipnum)
{
    return get_gameinfo(gamenum)->chips[chipnum];
}


int LibMame_Get_Game_Dipswitch_Count(int gamenum)
{
    return get_gameinfo(gamenum)->dipswitch_count;
}


LibMame_Dipswitch LibMame_Get_Game_Dipswitch(int gamenum, int num)
{
    return get_gameinfo(gamenum)->dipswitches[num];
}


int LibMame_Get_Game_MaxSimultaneousPlayers(int gamenum)
{
    return get_gameinfo(gamenum)->max_simultaneous_players;
}


LibMame_AllControllers LibMame_Get_Game_AllControllers(int gamenum)
{
    return get_gameinfo(gamenum)->controllers;
}


int LibMame_Get_Game_BiosSet_Count(int gamenum)
{
    return get_gameinfo(gamenum)->biosset_count;
}


LibMame_BiosSet LibMame_Get_Game_BiosSet(int gamenum, int biossetnum)
{
    return get_gameinfo(gamenum)->biossets[biossetnum];
}


int LibMame_Get_Game_Rom_Count(int gamenum)
{
    return get_gameinfo(gamenum)->rom_count;
}


LibMame_Image LibMame_Get_Game_Rom(int gamenum, int romnum)
{
    return get_gameinfo(gamenum)->roms[romnum];
}


int LibMame_Get_Game_Hdd_Count(int gamenum)
{
    return get_gameinfo(gamenum)->hdd_count;
}


LibMame_Image LibMame_Get_Game_Hdd(int gamenum, int hddnum)
{
    return get_gameinfo(gamenum)->hdds[hddnum];
}


const char *LibMame_Get_Game_SourceFileName(int gamenum)
{
    return get_gameinfo(gamenum)->source_file_name;
}
