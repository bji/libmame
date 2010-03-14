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

#include "emu.h"
#include "config.h"
#include "libmame.h"
#include "osdcore.h"
#include <pthread.h>
#include <string.h>


typedef struct GameInfo
{
    bool converted;
    int driver_index;
    int year_of_release;
    int working_flags, orientation_flags, screen_flags;
    LibMame_ScreenResolution screen_resolution;
    int screen_refresh_rate;
} GameInfo;


static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_game_count = 0;
static GameInfo *g_gameinfos = 0;
/* Need Hashtable hashing game short names to indexes into g_gameinfos
   to support fast lookup of game info */


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


static void convert_year(const game_driver *driver, GameInfo *gameinfo)
{
    gameinfo->year_of_release = 0;
    const char *yearstr = driver->year;
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


static void convert_working_flags(const game_driver *driver,
                                  GameInfo *gameinfo)
{
    gameinfo->working_flags = 0;
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
    gameinfo->orientation_flags = 0;
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
    gameinfo->screen_flags = 0;
    gameinfo->screen_resolution.width = 0;
    gameinfo->screen_resolution.height = 0;
    gameinfo->screen_refresh_rate = 0;

    const device_config *devconfig;
    for (devconfig = video_screen_first(machineconfig);
         devconfig != NULL; devconfig = video_screen_next(devconfig)) {
        const screen_config *screenconfig =
            (const screen_config *) devconfig->inline_config;
        switch (screenconfig->type) {
        case SCREEN_TYPE_RASTER:
            gameinfo->screen_flags |= LIBMAME_SCREENFLAGS_RASTER;
            break;
        case SCREEN_TYPE_LCD:
            gameinfo->screen_flags |= LIBMAME_SCREENFLAGS_LCD;
            break;
        case SCREEN_TYPE_VECTOR:
            gameinfo->screen_flags |= LIBMAME_SCREENFLAGS_VECTOR;
            break;
        }
        if (screenconfig->type != SCREEN_TYPE_VECTOR) {
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


static void convert_game_info(GameInfo *gameinfo)
{
    const game_driver *driver = drivers[gameinfo->driver_index];

	machine_config *machineconfig = 
        machine_config_alloc(driver->machine_config);
    /* Mame's code assumes the above succeeds, we will too */

    convert_year(driver, gameinfo);
    convert_working_flags(driver, gameinfo);
    convert_orientation_flags(driver, gameinfo);
    convert_screen_info(machineconfig, gameinfo);

    machine_config_free(machineconfig);
}


static GameInfo *get_gameinfo_helper(int gamenum, bool converted)
{
    pthread_mutex_lock(&g_mutex);

    if (g_game_count == 0) {
        const game_driver * const *pdriver = drivers;
        while (*pdriver) {
            const game_driver *driver = *pdriver;
            if (!(driver->flags & (GAME_IS_BIOS_ROOT | GAME_NO_STANDALONE))) {
                g_game_count++;
            }
            pdriver++;
        }
        g_gameinfos = (GameInfo *) osd_malloc
            (sizeof(GameInfo) * g_game_count);
        GameInfo *gameinfo = g_gameinfos;
        int driver_index = 0;
        while (true) {
            const game_driver *driver = drivers[driver_index];
            if (driver == NULL) {
                break;
            }
            if (!(driver->flags & (GAME_IS_BIOS_ROOT | GAME_NO_STANDALONE))) {
                gameinfo->converted = false;
                gameinfo->driver_index = driver_index;
            }
            driver_index++;
        }
    }

    GameInfo *ret = &(g_gameinfos[gamenum]);

    if (converted && !ret->converted) {
        convert_game_info(ret);
    }

    pthread_mutex_unlock(&g_mutex);

    return ret;
}


static const game_driver *get_game_driver(int gamenum)
{
    return drivers[get_gameinfo_helper(gamenum, false)->driver_index];
}


static GameInfo *get_gameinfo(int gamenum)
{
    return get_gameinfo_helper(gamenum, true);
}


void LibMame_Games_Deinitialize()
{
    pthread_mutex_lock(&g_mutex);

    g_game_count = 0;
    if (g_gameinfos) {
        osd_free(g_gameinfos);
        g_gameinfos = 0;
    }

    pthread_mutex_unlock(&g_mutex);
}


int LibMame_Get_Game_Count()
{
    /* Force get_gameinfo_helper() to ensure that all game infos are
       created */
    (void) get_gameinfo_helper(0, false);

    return g_game_count;
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
    return get_game_driver(gamenum)->parent;
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


int LibMame_Get_Game_ScreenFlags(int gamenum)
{
    return get_gameinfo(gamenum)->screen_flags;
}


LibMame_ScreenResolution LibMame_Get_Game_ScreenResolution(int gamenum)
{
    return get_gameinfo(gamenum)->screen_resolution;
}


int LibMame_Get_Game_ScreenRefreshRate(int gamenum)
{
    return get_gameinfo(gamenum)->screen_refresh_rate;
}
