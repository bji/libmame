/** **************************************************************************
 * libmame_test_main.c
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
    printf("LibMame version %s\n", LibMame_Get_Version_String());

    if (argc > 1) {
        int gamenum = LibMame_Get_Game_Number(argv[1]);
        if (gamenum == -1) {
            int gamenums[15];
            int count = LibMame_Get_Game_Matches(argv[1], 15, gamenums);
            printf("Closest matches:\n");
            for (int i = 0; i < count; i++) {
                printf("%s (%s)\n", LibMame_Get_Game_Short_Name(gamenums[i]),
                       LibMame_Get_Game_Full_Name(gamenums[i]));
            }
            return -1;
        }
        else {
            printf("%s\n", LibMame_Get_Game_Full_Name(gamenum));
            return 0;
        }
    }

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
               LibMame_Get_Game_ScreenRefreshRateHz(i));
        printf("\tSound Channels: %d\n", LibMame_Get_Game_SoundChannelCount(i));
        int samplecount = LibMame_Get_Game_SoundSamples_Count(i);
        if (samplecount) {
            printf("\tSound Samples: %d", samplecount);
            int source =  LibMame_Get_Game_SoundSamplesSource(i);
            if (source != i) {
                printf(" (from %s)", LibMame_Get_Game_Short_Name(source));
            }
            if (!LibMame_Get_Game_SoundSamplesIdenticalToSource(i)) {
                printf(" -- ");
                for (int j = 0; j < samplecount; j++) {
                    if (j > 0) {
                        printf(", ");
                    }
                    printf("%s", LibMame_Get_Game_SoundSampleFileName(i, j));
                }
            }
            printf("\n");
        }
        int chipcount = LibMame_Get_Game_Chip_Count(i);
        if (chipcount) {
            printf("\tChips: ");
            for (int j = 0; j < chipcount; j++) {
                LibMame_ChipDescriptor desc = LibMame_Get_Game_Chip(i, j);
                if (j > 0) {
                    printf(", ");
                }
                printf("[ %s: %s, %s, %d Hz ]", 
                       desc.is_sound ? "sound" : "cpu", desc.tag,
                       desc.name, desc.clock_hz);
            }
            printf("\n");
        }
        int dipswitchcount = LibMame_Get_Game_Dipswitch_Count(i);
        if (dipswitchcount) {
            printf("\tDipswitches: ");
            for (int j = 0; j < dipswitchcount; j++) {
                LibMame_DipswitchDescriptor desc = 
                    LibMame_Get_Game_Dipswitch(i, j);
                if (j > 0) {
                    printf(", ");
                }
                printf("[ %s = ", desc.name);
                for (int k = 0; k < desc.setting_count; k++) {
                    if (k > 0) {
                        printf (", ");
                    }
                    if (k == desc.default_setting_number) {
                        printf("*");
                    }
                    printf("%s", desc.setting_names[k]);
                }
                printf("]");
            }
            printf("\n");
        }
        int adjusterscount = LibMame_Get_Game_Adjusters_Count(i);
        if (adjusterscount) {
            printf("\tAdjusters: ");
            for (int j = 0; j < adjusterscount; j++) {
                LibMame_AdjusterDescriptor desc = 
                    LibMame_Get_Game_Adjuster(i, j);
                if (j > 0) {
                    printf(", ");
                }
                printf("%s (%d)", desc.name, desc.default_value);
            }
            printf("\n");
        }
        const char *srcname = LibMame_Get_Game_SourceFileName(i);
        if (srcname) {
            printf("\tSource File Name: %s\n", srcname);
        }
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
