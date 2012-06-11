class xxmissio_state : public driver_device
{
public:
	xxmissio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram"){ }

	UINT8 m_status;
	required_shared_ptr<UINT8> m_bgram;
	required_shared_ptr<UINT8> m_fgram;
	required_shared_ptr<UINT8> m_spriteram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 m_xscroll;
	UINT8 m_yscroll;
	UINT8 m_flipscreen;
	DECLARE_WRITE8_MEMBER(xxmissio_bank_sel_w);
	DECLARE_WRITE8_MEMBER(xxmissio_status_m_w);
	DECLARE_WRITE8_MEMBER(xxmissio_status_s_w);
	DECLARE_WRITE8_MEMBER(xxmissio_flipscreen_w);
	DECLARE_WRITE8_MEMBER(xxmissio_bgram_w);
	DECLARE_READ8_MEMBER(xxmissio_bgram_r);
	DECLARE_WRITE8_MEMBER(xxmissio_paletteram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(xxmissio_status_r);
};


/*----------- defined in video/xxmissio.c -----------*/

VIDEO_START( xxmissio );
SCREEN_UPDATE_IND16( xxmissio );

WRITE8_DEVICE_HANDLER( xxmissio_scroll_x_w );
WRITE8_DEVICE_HANDLER( xxmissio_scroll_y_w );


