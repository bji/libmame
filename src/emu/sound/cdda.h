#pragma once

#ifndef __CDDA_H__
#define __CDDA_H__

#include "devlegcy.h"

void cdda_set_cdrom(device_t *device, void *file);
device_t *cdda_from_cdrom(running_machine *machine, void *file);

void cdda_start_audio(device_t *device, UINT32 startlba, UINT32 numblocks);
void cdda_stop_audio(device_t *device);
void cdda_pause_audio(device_t *device, int pause);
void cdda_set_volume(device_t *device, int volume);
void cdda_set_channel_volume(device_t *device, int channel, int volume);

UINT32 cdda_get_audio_lba(device_t *device);
int cdda_audio_active(device_t *device);
int cdda_audio_paused(device_t *device);
int cdda_audio_ended(device_t *device);

DECLARE_LEGACY_SOUND_DEVICE(CDDA, cdda);

#endif /* __CDDA_H__ */
