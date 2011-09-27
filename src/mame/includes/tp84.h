class tp84_state : public driver_device
{
public:
	tp84_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	cpu_device *m_audiocpu;
	UINT8 *m_bg_videoram;
	UINT8 *m_bg_colorram;
	UINT8 *m_fg_videoram;
	UINT8 *m_fg_colorram;
	UINT8 *m_spriteram;
	UINT8 *m_scroll_x;
	UINT8 *m_scroll_y;
	UINT8 *m_palette_bank;
	UINT8 *m_flipscreen_x;
	UINT8 *m_flipscreen_y;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
};


/*----------- defined in video/tp84.c -----------*/

WRITE8_HANDLER( tp84_spriteram_w );
READ8_HANDLER( tp84_scanline_r );

PALETTE_INIT( tp84 );
VIDEO_START( tp84 );
SCREEN_UPDATE( tp84 );
