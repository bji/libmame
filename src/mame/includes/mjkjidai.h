class mjkjidai_state : public driver_device
{
public:
	mjkjidai_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_videoram;
	UINT8 *m_spriteram1;
	UINT8 *m_spriteram2;
	UINT8 *m_spriteram3;

	int m_keyb;
	int m_nvram_init_count;
	UINT8 *m_nvram;
	size_t m_nvram_size;
	int m_display_enable;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/mjkjidai.c -----------*/

VIDEO_START( mjkjidai );
SCREEN_UPDATE( mjkjidai );
WRITE8_HANDLER( mjkjidai_videoram_w );
WRITE8_HANDLER( mjkjidai_ctrl_w );


