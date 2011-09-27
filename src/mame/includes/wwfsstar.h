class wwfsstar_state : public driver_device
{
public:
	wwfsstar_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_vblank;
	int m_scrollx;
	int m_scrolly;
	UINT16 *m_spriteram;
	UINT16 *m_fg0_videoram;
	UINT16 *m_bg0_videoram;
	tilemap_t *m_fg0_tilemap;
	tilemap_t *m_bg0_tilemap;
};


/*----------- defined in video/wwfsstar.c -----------*/

VIDEO_START( wwfsstar );
SCREEN_UPDATE( wwfsstar );
WRITE16_HANDLER( wwfsstar_fg0_videoram_w );
WRITE16_HANDLER( wwfsstar_bg0_videoram_w );
