/** **************************************************************************
 * osd_main.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "emu.h"
#include "emuopts.h"
#include "libmame.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    printf("LibMame version %s\n", LibMame_Get_Version_String());

    int count = LibMame_Get_Game_Count();

    for (int i = 0; i < count; i++) {
        printf("Game %d:\n", i);
        printf("\tShort Name: %s\n", LibMame_Get_Game_Short_Name(i));
        printf("\tFull Name: %s\n", LibMame_Get_Game_Full_Name(i));
        printf("\tYear of Release: ");
        int year = LibMame_Get_Game_Year_Of_Release(i);
        if (year == -1) {
            printf("Unknown\n");
        }
        else {
            printf("%d\n", year);
        }
        const char *clone_of = LibMame_Get_Game_CloneOf_Short_Name(i);
        if (clone_of != NULL) {
            printf("\tClone Of: %s\n", clone_of);
        }
        printf("\tManufacturer: %s\n", LibMame_Get_Game_Manufacturer(i));
        bool needcomma;
        int flags = LibMame_Get_Game_WorkingFlags(i);
        if (flags) {
            needcomma = false;
            printf("\tWorking Status: ");
            if (flags & LIBMAME_WORKINGFLAGS_WRONG_COLORS) {
                printf("Wrong Colors");
                needcomma = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_IMPERFECT_COLORS) {
                printf("%sImperfect Colors", needcomma ? ", " : "");
                needcomma = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_NO_SOUND) {
                printf("%sNo Sound", needcomma ? ", " : "");
                needcomma = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_IMPERFECT_SOUND) {
                printf("%sImperfect Sound", needcomma ? ", " : "");
                needcomma = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_NO_COCKTAIL) {
                printf("%sNo Cocktail", needcomma ? ", " : "");
                needcomma = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_UNEMULATED_PROTECTION) {
                printf("%sUnemulated Protection", needcomma ? ", " : "");
                needcomma = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_NO_SAVE_STATE) {
                printf("%sNo Save State", needcomma ? ", " : "");
                needcomma = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_NOTWORKING) {
                printf("%sNot Working", needcomma ? ", " : "");
            }
            printf("\n");
        }
        flags = LibMame_Get_Game_OrientationFlags(i);
        if (flags) {
            printf("\tOrientations: ");
            needcomma = false;
            if (flags & LIBMAME_ORIENTATIONFLAGS_FLIP_X) {
                printf("Flip X");
                needcomma = true;
            }
            if (flags & LIBMAME_ORIENTATIONFLAGS_FLIP_Y) {
                printf("%sFlip Y", needcomma ? ", " : "");
                needcomma = true;
            }
            if (flags & LIBMAME_ORIENTATIONFLAGS_ROTATE_90) {
                printf("%sRotate 90", needcomma ? ", " : "");
                needcomma = true;
            }
            if (flags & LIBMAME_ORIENTATIONFLAGS_ROTATE_180) {
                printf("%sRotate 180", needcomma ? ", " : "");
                needcomma = true;
            }
            if (flags & LIBMAME_ORIENTATIONFLAGS_ROTATE_270) {
                printf("%sRotate 270", needcomma ? ", " : "");
            }
            printf("\n");
        }
        printf("\tScreen Type: ");
        LibMame_ScreenResolution screen_resolution =
            LibMame_Get_Game_ScreenResolution(i);
        switch (LibMame_Get_Game_ScreenType(i)) {
        case LibMame_ScreenType_Raster:
            printf("Raster %d x %d\n", screen_resolution.width,
                   screen_resolution.height);
            break;
        case LibMame_ScreenType_LCD:
            printf("LCD %d x %d\n", screen_resolution.width,
                   screen_resolution.height);
            break;
        case LibMame_ScreenType_Vector:
            printf("Vector\n");
            break;
        }
        printf("\tScreen Refresh Rate: %d Hz\n", 
               LibMame_Get_Game_ScreenRefreshRate(i));
    }

#if 0
    options_entry options_entries[1] = { { NULL } };

    core_options *core_options = mame_options_init(options_entries);

    int ret = mame_execute(core_options);

    if (core_options != NULL) {
        options_free(core_options);
    }
#else
    int ret = 0;
#endif

    LibMame_Deinitialize();

    return ret;
}
