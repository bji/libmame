#include "devlegcy.h"

/*----------- defined in audio/flower.c -----------*/

WRITE8_DEVICE_HANDLER( flower_sound1_w );
WRITE8_DEVICE_HANDLER( flower_sound2_w );

DECLARE_LEGACY_SOUND_DEVICE(FLOWER, flower_sound);


/*----------- defined in video/flower.c -----------*/

extern UINT8 *flower_textram, *flower_bg0ram, *flower_bg1ram, *flower_bg0_scroll, *flower_bg1_scroll;

WRITE8_HANDLER( flower_textram_w );
WRITE8_HANDLER( flower_bg0ram_w );
WRITE8_HANDLER( flower_bg1ram_w );
WRITE8_HANDLER( flower_flipscreen_w );

SCREEN_UPDATE( flower );
VIDEO_START( flower );
PALETTE_INIT( flower );
