class speedbal_state : public driver_device
{
public:
	speedbal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_background_videoram;
	UINT8 *m_foreground_videoram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/speedbal.c -----------*/

VIDEO_START( speedbal );
SCREEN_UPDATE( speedbal );
WRITE8_HANDLER( speedbal_foreground_videoram_w );
WRITE8_HANDLER( speedbal_background_videoram_w );
