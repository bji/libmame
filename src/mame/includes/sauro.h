class sauro_state : public driver_device
{
public:
	sauro_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	size_t m_spriteram_size;
	UINT8 *m_spriteram;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_videoram2;
	UINT8 *m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 m_palette_bank;
};


/*----------- defined in video/sauro.c -----------*/

WRITE8_HANDLER( tecfri_videoram_w );
WRITE8_HANDLER( tecfri_colorram_w );
WRITE8_HANDLER( tecfri_videoram2_w );
WRITE8_HANDLER( tecfri_colorram2_w );
WRITE8_HANDLER( tecfri_scroll_bg_w );
WRITE8_HANDLER( sauro_scroll_fg_w );
WRITE8_HANDLER( sauro_palette_bank_w );

VIDEO_START( sauro );
VIDEO_START( trckydoc );

SCREEN_UPDATE( sauro );
SCREEN_UPDATE( trckydoc );
