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
#include <string.h>

static void PollAllControllersState(LibMame_AllControllersState *all_states,
                                    void *callback_data)
{
    static int frame = 0;
    int which = ++frame % 60;
    if (which < 4) {
        all_states->shared.other_buttons_state |=
            (1 << LibMame_OtherButtonType_Coin1);
    }
    else if (which < 8) {
        all_states->shared.other_buttons_state |=
            (1 << LibMame_OtherButtonType_Start1);
    }
    (void) callback_data;
}

void MakeRunningGameCalls(void *callback_data)
{
    (void) callback_data;
}

void UpdateVideo(const LibMame_RenderPrimitive *prims, void *callback_data)
{
    (void) prims;
    (void) callback_data;

#if 0
    /* TESTING */
    static int update_count = 0;
    if (++update_count == 10000) {
        mame_schedule_exit(machine);
    }
    else if ((update_count % 50) == 0) {
        printf("update_count %d\n", update_count);
    }
#endif
}

void UpdateAudio(int16_t *buffer, int samples_this_frame, void *callback_data)
{
    (void) callback_data;
}

void SetMasterVolume(int attenuation, void *callback_data)
{
    (void) attenuation;
    (void) callback_data;
}


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
            LibMame_RunGameOptions options;
            LibMame_Set_Default_RunGameOptions(&options);
            snprintf(options.rom_path, sizeof(options.rom_path), "test_roms");
            snprintf(options.aviwrite_file, sizeof(options.aviwrite_file),
                     "libmame-test.avi");
            LibMame_RunGameCallbacks cbs = {
                &PollAllControllersState,
                &MakeRunningGameCalls,
                &UpdateVideo,
                &UpdateAudio,
                &SetMasterVolume
            };
            (void) LibMame_RunGame(gamenum, &options, &cbs, NULL);
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
        bool needindent;
        int flags = LibMame_Get_Game_WorkingFlags(i);
        if (flags) {
            needindent = false;
            printf("\tWorking Status: ");
            if (flags & LIBMAME_WORKINGFLAGS_WRONG_COLORS) {
                printf("Wrong Colors");
                needindent = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_IMPERFECT_COLORS) {
                printf("%sImperfect Colors", 
                       needindent ? "\n\t                " : "");
                needindent = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_NO_SOUND) {
                printf("%sNo Sound",
                       needindent ? "\n\t                " : "");
                needindent = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_IMPERFECT_SOUND) {
                printf("%sImperfect Sound",
                       needindent ? "\n\t                " : "");
                needindent = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_NO_COCKTAIL) {
                printf("%sNo Cocktail",
                       needindent ? "\n\t                " : "");
                needindent = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_UNEMULATED_PROTECTION) {
                printf("%sUnemulated Protection",
                       needindent ? "\n\t                " : "");
                needindent = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_NO_SAVE_STATE) {
                printf("%sNo Save State",
                       needindent ? "\n\t                " : "");
                needindent = true;
            }
            if (flags & LIBMAME_WORKINGFLAGS_NOTWORKING) {
                printf("%sNot Working",
                       needindent ? "\n\t                " : "");
            }
            printf("\n");
        }
        flags = LibMame_Get_Game_OrientationFlags(i);
        if (flags) {
            printf("\tOrientations: ");
            needindent = false;
            if (flags & LIBMAME_ORIENTATIONFLAGS_FLIP_X) {
                printf("Flip X");
                needindent = true;
            }
            if (flags & LIBMAME_ORIENTATIONFLAGS_FLIP_Y) {
                printf("%sFlip Y",
                       needindent ? "\n\t             " : "");
                needindent = true;
            }
            if (flags & LIBMAME_ORIENTATIONFLAGS_ROTATE_90) {
                printf("%sRotate 90",
                       needindent ? "\n\t             " : "");
                needindent = true;
            }
            if (flags & LIBMAME_ORIENTATIONFLAGS_ROTATE_180) {
                printf("%sRotate 180",
                       needindent ? "\n\t             " : "");
                needindent = true;
            }
            if (flags & LIBMAME_ORIENTATIONFLAGS_ROTATE_270) {
                printf("%sRotate 270",
                       needindent ? "\n\t             " : "");
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
        printf("\tSound Channels: %d\n", LibMame_Get_Game_SoundChannels(i));
        int samplecount = LibMame_Get_Game_SoundSamples_Count(i);
        if (samplecount) {
            printf("\tSound Samples: ");
            int source =  LibMame_Get_Game_SoundSamplesSource(i);
            bool needindent = false;
            if (source != i) {
                if (LibMame_Get_Game_SoundSamplesIdenticalToSource(i)) {
                    printf("(identical to %s)", 
                           LibMame_Get_Game_Short_Name(source));
                }
                else {
                    printf("(from %s)", LibMame_Get_Game_Short_Name(source));
                }
                needindent = true;
            }
            if (!LibMame_Get_Game_SoundSamplesIdenticalToSource(i)) {
                for (int j = 0; j < samplecount; j++) {
                    if (needindent) {
                        printf("\n\t               ");
                    }
                    printf("%s", LibMame_Get_Game_SoundSampleFileName(i, j));
                    needindent = true;
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
                    printf("\n\t       ");
                }
                printf("%s: %s, %s, %d Hz", 
                       desc.is_sound ? "sound" : "cpu", desc.tag,
                       desc.name, desc.clock_hz);
            }
            printf("\n");
        }
        int settingcount = LibMame_Get_Game_Setting_Count(i);
        if (settingcount) {
            printf("\tSettings: ");
            for (int j = 0; j < settingcount; j++) {
                LibMame_SettingDescriptor desc = LibMame_Get_Game_Setting(i, j);
                if (j > 0) {
                    printf("\n\t          ");
                }
                switch (desc.type) {
                case LibMame_SettingType_Configuration:
                    printf("Configuration: %s = ", desc.name);
                    break;
                case LibMame_SettingType_Dipswitch:
                    printf("Dipswitch: %s = ", desc.name);
                    break;
                case LibMame_SettingType_Adjuster:
                    printf("Adjuster: %s = %d", desc.name, desc.default_value);
                    break;
                }

                if (desc.type != LibMame_SettingType_Adjuster) {
                    for (int k = 0; k < desc.value_count; k++) {
                        if (k > 0) {
                            printf (", ");
                        }
                        if (k == desc.default_value) {
                            printf("*");
                        }
                        printf("%s", desc.value_names[k]);
                    }
                }
            }
            printf("\n");
        }
        printf("\tMax Simultaneous Players: %d\n", 
               LibMame_Get_Game_MaxSimultaneousPlayers(i));
        LibMame_PerPlayerControllersDescriptor perplayer_controllers =
            LibMame_Get_Game_PerPlayerControllers(i);
        printf("\tControllers: ");
        needindent = false;
        if (perplayer_controllers.controller_flags & 
            (1 << LibMame_ControllerType_JoystickHorizontal)) {
            printf("Horizontal Joystick");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_JoystickVertical)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Vertical Joystick");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_Joystick4Way)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("4-Way Joystick");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_Joystick8Way)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("8-Way Joystick");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_JoystickAnalog)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Analog Joystick");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_DoubleJoystickHorizontal)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Double Horizontal Joystick");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_DoubleJoystickVertical)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Double Vertical Joystick");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_DoubleJoystick4Way)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Double 4-Way Joystick");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_DoubleJoystick8Way)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Double 8-Way Joystick");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_Spinner)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Spinner");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_Paddle)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Paddle");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_Trackball)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Trackball");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_Lightgun)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Light Gun");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_Pedal)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Pedal");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_Pedal2)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Pedal 2");
            needindent = true;
        }
        if (perplayer_controllers.controller_flags &
            (1 << LibMame_ControllerType_Pedal3)) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Pedal 3");
            needindent = true;
        }
        if (perplayer_controllers.mahjong_button_flags) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Has Mahjong Buttons");
            needindent = true;
        }
        if (perplayer_controllers.hanafuda_button_flags) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Has Hanafuda Buttons");
            needindent = true;
        }
        if (perplayer_controllers.gambling_button_flags) {
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Has Gambling Buttons");
            needindent = true;
        }
        for (int j = 0; j < 16; j++) {
            if (!(perplayer_controllers.normal_button_flags & (1 << j))) {
                continue;
            }
            if (needindent) {
                printf("\n\t             ");
            }
            printf("Button %d - %s", j + 1, 
                   perplayer_controllers.normal_button_names[j] ?
                   perplayer_controllers.normal_button_names[j] : "unknown");
            needindent = true;
        }
        printf("\n");
        const char *srcname = LibMame_Get_Game_SourceFileName(i);
        if (srcname) {
            printf("\tSource File Name: %s\n", srcname);
        }
    }

    LibMame_Deinitialize();

    return 0;
}
