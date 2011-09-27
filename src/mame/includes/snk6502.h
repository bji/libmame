/*************************************************************************

    rokola hardware

*************************************************************************/

#include "devlegcy.h"
#include "sound/discrete.h"
#include "sound/samples.h"
#include "sound/sn76477.h"


class snk6502_state : public driver_device
{
public:
	snk6502_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 m_sasuke_counter;

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_videoram2;
	UINT8 *m_charram;

	int m_charbank;
	int m_backcolor;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	rgb_t m_palette[64];
};


/*----------- defined in audio/snk6502.c -----------*/

extern const samples_interface sasuke_samples_interface;
extern const samples_interface vanguard_samples_interface;
extern const samples_interface fantasy_samples_interface;
extern const sn76477_interface sasuke_sn76477_intf_1;
extern const sn76477_interface sasuke_sn76477_intf_2;
extern const sn76477_interface sasuke_sn76477_intf_3;
extern const sn76477_interface satansat_sn76477_intf;
extern const sn76477_interface vanguard_sn76477_intf_1;
extern const sn76477_interface vanguard_sn76477_intf_2;
extern const sn76477_interface fantasy_sn76477_intf;

extern WRITE8_HANDLER( sasuke_sound_w );
extern WRITE8_HANDLER( satansat_sound_w );
extern WRITE8_HANDLER( vanguard_sound_w );
extern WRITE8_HANDLER( vanguard_speech_w );
extern WRITE8_HANDLER( fantasy_sound_w );
extern WRITE8_HANDLER( fantasy_speech_w );

DECLARE_LEGACY_SOUND_DEVICE(SNK6502, snk6502_sound);

void snk6502_set_music_clock(running_machine &machine, double clock_time);
void snk6502_set_music_freq(running_machine &machine, int freq);
int snk6502_music0_playing(running_machine &machine);

DISCRETE_SOUND_EXTERN( fantasy );


/*----------- defined in video/snk6502.c -----------*/

WRITE8_HANDLER( snk6502_videoram_w );
WRITE8_HANDLER( snk6502_videoram2_w );
WRITE8_HANDLER( snk6502_colorram_w );
WRITE8_HANDLER( snk6502_charram_w );
WRITE8_HANDLER( snk6502_flipscreen_w );
WRITE8_HANDLER( snk6502_scrollx_w );
WRITE8_HANDLER( snk6502_scrolly_w );

PALETTE_INIT( snk6502 );
VIDEO_START( snk6502 );
SCREEN_UPDATE( snk6502 );
VIDEO_START( pballoon );

WRITE8_HANDLER( satansat_b002_w );
WRITE8_HANDLER( satansat_backcolor_w );

PALETTE_INIT( satansat );
VIDEO_START( satansat );

