/*************************************************************************

Crazy Ballooon

*************************************************************************/


#define CRBALOON_MASTER_XTAL	(XTAL_9_987MHz)


class crbaloon_state : public driver_device
{
public:
	crbaloon_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *pc3092_data;
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *spriteram;
	UINT16 collision_address;
	UINT8 collision_address_clear;
	tilemap_t *bg_tilemap;
};


/*----------- defined in audio/crbaloon.c -----------*/

WRITE8_DEVICE_HANDLER( crbaloon_audio_set_music_freq );
WRITE8_DEVICE_HANDLER( crbaloon_audio_set_music_enable );
void crbaloon_audio_set_explosion_enable(device_t *sn, int enabled);
void crbaloon_audio_set_breath_enable(device_t *sn, int enabled);
void crbaloon_audio_set_appear_enable(device_t *sn, int enabled);
WRITE8_DEVICE_HANDLER( crbaloon_audio_set_laugh_enable );

MACHINE_CONFIG_EXTERN( crbaloon_audio );


/*----------- defined in video/crbaloon.c -----------*/

PALETTE_INIT( crbaloon );
VIDEO_START( crbaloon );
VIDEO_UPDATE( crbaloon );

WRITE8_HANDLER( crbaloon_videoram_w );
WRITE8_HANDLER( crbaloon_colorram_w );

UINT16 crbaloon_get_collision_address(running_machine *machine);
void crbaloon_set_clear_collision_address(running_machine *machine, int _crbaloon_collision_address_clear);
