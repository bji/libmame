class timelimt_state : public driver_device
{
public:
	timelimt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	int m_nmi_enabled;
	UINT8 *m_bg_videoram;
	size_t m_bg_videoram_size;
	int m_scrollx;
	int m_scrolly;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/timelimt.c -----------*/

VIDEO_START( timelimt );
PALETTE_INIT( timelimt );
SCREEN_UPDATE( timelimt );

WRITE8_HANDLER( timelimt_videoram_w );
WRITE8_HANDLER( timelimt_bg_videoram_w );
WRITE8_HANDLER( timelimt_scroll_y_w );
WRITE8_HANDLER( timelimt_scroll_x_msb_w );
WRITE8_HANDLER( timelimt_scroll_x_lsb_w );
