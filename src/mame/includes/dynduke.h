class dynduke_state : public driver_device
{
public:
	dynduke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram;
	UINT16 *m_back_data;
	UINT16 *m_fore_data;
	UINT16 *m_scroll_ram;
	tilemap_t *m_bg_layer;
	tilemap_t *m_fg_layer;
	tilemap_t *m_tx_layer;
	int m_back_bankbase;
	int m_fore_bankbase;
	int m_back_enable;
	int m_fore_enable;
	int m_sprite_enable;
	int m_txt_enable;
	int m_old_back;
	int m_old_fore;
};


/*----------- defined in video/dynduke.c -----------*/

WRITE16_HANDLER( dynduke_background_w );
WRITE16_HANDLER( dynduke_foreground_w );
WRITE16_HANDLER( dynduke_text_w );
WRITE16_HANDLER( dynduke_gfxbank_w );
WRITE16_HANDLER( dynduke_control_w );
WRITE16_HANDLER( dynduke_paletteram_w );
VIDEO_START( dynduke );
SCREEN_UPDATE_IND16( dynduke );
SCREEN_VBLANK( dynduke );
