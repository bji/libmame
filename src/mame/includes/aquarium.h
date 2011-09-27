


class aquarium_state : public driver_device
{
public:
	aquarium_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  m_scroll;
	UINT16 *  m_txt_videoram;
	UINT16 *  m_mid_videoram;
	UINT16 *  m_bak_videoram;
	UINT16 *  m_spriteram;
//  UINT16 *  m_paletteram;   // currently this uses generic palette handling
	size_t    m_spriteram_size;

	/* video-related */
	tilemap_t  *m_txt_tilemap;
	tilemap_t  *m_mid_tilemap;
	tilemap_t  *m_bak_tilemap;

	/* misc */
	int m_aquarium_snd_ack;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/aquarium.c -----------*/

WRITE16_HANDLER( aquarium_txt_videoram_w );
WRITE16_HANDLER( aquarium_mid_videoram_w );
WRITE16_HANDLER( aquarium_bak_videoram_w );

VIDEO_START(aquarium);
SCREEN_UPDATE(aquarium);
