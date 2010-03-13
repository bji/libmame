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


#define FULL_NAME_MAX 80
#define SHORT_NAME_MAX 9
#define MANUFACTURER_MAX 64


typedef struct GameInfo
{
    bool converted;
    int driver_index;
    char short_name[SHORT_NAME_MAX], full_name[FULL_NAME_MAX];
    int year_of_release;
    char clone_of[SHORT_NAME_MAX];
    char manufacturer[MANUFACTURER_MAX];
    int working_flags, orientation_flags, screen_flags;
    int screen_width, screen_height, refresh_rate_hz;
} GameInfo;

typedef char short_name[SHORT_NAME_MAX];

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

#define SAFE_STRCPY(dest, src) \
    safe_strncpy(dest, src, sizeof(dest))


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


static void convert_strings(const game_driver *driver,
                            GameInfo *gameinfo)
{
    SAFE_STRCPY(gameinfo->full_name, driver->description);
    SAFE_STRCPY(gameinfo->clone_of, driver->parent);
    SAFE_STRCPY(gameinfo->manufacturer, driver->manufacturer);
}


static int convert_year(const char *yearstr)
{
    int year = 0;
    while (*yearstr) {
        if ((*yearstr >= '0') && (*yearstr <= '9')) {
            year *= 10;
            year += *yearstr - '0';
        }
        else {
            return -1;
        }
        yearstr++;
    }

    return year;
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


static void convert_screen_info(const game_driver *driver,
                                GameInfo *gameinfo)
{
    gameinfo->screen_flags = 0;
    gameinfo->screen_width = 0;
    gameinfo->screen_height = 0;
#if 0
    const device_config *devconfig;
    for (devconfig = video_screen_first(driver->machine_config);
         devconfig != NULL; devconfig = video_screen_next(devconfig)) {
        const screen_config screenconfig =
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
            gameinfo->screen_width = 
                ((screenconfig->visarea.max_x - 
                  screenconfig->visarea.min_x) + 1);
            gameinfo->screen_height = 
                ((screenconfig->visarea.max_y - 
                  screenconfig->visarea.min_y) + 1);
        }
        gameinfo->refresh_rate_hz = 
            ATTOSECONDS_TO_HZ(screenconfig->refresh);
    }
#endif
}


static void convert_game_info(GameInfo *gameinfo)
{
    const game_driver *driver = drivers[gameinfo->driver_index];

    convert_strings(driver, gameinfo);
    gameinfo->year_of_release = convert_year(driver->year);
    convert_working_flags(driver, gameinfo);
    convert_orientation_flags(driver, gameinfo);
    convert_screen_info(driver, gameinfo);
}


static GameInfo *get_gameinfo_helper(int gamenum, bool complete)
{
    pthread_mutex_lock(&g_mutex);

    if (g_game_count == 0) {
        const game_driver * const *pdriver = drivers;
        while (*pdriver) {
            const game_driver *driver = *pdriver;
            if (!(driver->flags & GAME_IS_BIOS_ROOT)) {
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
            if (!(driver->flags & GAME_IS_BIOS_ROOT)) {
                gameinfo->converted = false;
                gameinfo->driver_index = driver_index;
                SAFE_STRCPY(gameinfo->short_name, driver->name);
            }
            driver_index++;
        }
    }

    GameInfo *ret = &(g_gameinfos[gamenum]);

    if (complete && !ret->converted) {
        convert_game_info(ret);
    }

    pthread_mutex_unlock(&g_mutex);

    return ret;
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
    return get_gameinfo_helper(gamenum, false)->short_name;
}


const char *LibMame_Get_Game_Full_Name(int gamenum)
{
    return get_gameinfo(gamenum)->full_name;
}


int LibMame_Get_Game_Year_Of_Release(int gamenum)
{
    return get_gameinfo(gamenum)->year_of_release;
}


const char *LibMame_Get_Game_CloneOf_Short_Name(int gamenum)
{
    const char *clone_of = get_gameinfo(gamenum)->clone_of;
    return *clone_of ? clone_of : NULL;
}


const char *LibMame_Get_Game_Manufacturer(int gamenum)
{
    return get_gameinfo(gamenum)->manufacturer;
}


int LibMame_Get_Game_WorkingFlags(int gamenum)
{
    return get_gameinfo(gamenum)->working_flags;
}


int LibMame_Get_Game_OrientationFlags(int gamenum)
{
    return get_gameinfo(gamenum)->orientation_flags;
}
