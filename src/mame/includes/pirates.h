class pirates_state : public driver_device
{
public:
	pirates_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_tx_tileram;
	UINT16 *m_spriteram;
	UINT16 *m_fg_tileram;
	UINT16 *m_bg_tileram;
	UINT16 *m_scroll;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/pirates.c -----------*/

WRITE16_HANDLER( pirates_tx_tileram_w );
WRITE16_HANDLER( pirates_fg_tileram_w );
WRITE16_HANDLER( pirates_bg_tileram_w );

VIDEO_START( pirates );
SCREEN_UPDATE( pirates );
