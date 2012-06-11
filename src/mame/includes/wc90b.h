class wc90b_state : public driver_device
{
public:
	wc90b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_scroll1x(*this, "scroll1x"),
		m_scroll2x(*this, "scroll2x"),
		m_scroll1y(*this, "scroll1y"),
		m_scroll2y(*this, "scroll2y"),
		m_scroll_x_lo(*this, "scroll_x_lo"),
		m_spriteram(*this, "spriteram"){ }

	int m_msm5205next;
	int m_toggle;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_txvideoram;
	required_shared_ptr<UINT8> m_scroll1x;
	required_shared_ptr<UINT8> m_scroll2x;
	required_shared_ptr<UINT8> m_scroll1y;
	required_shared_ptr<UINT8> m_scroll2y;
	required_shared_ptr<UINT8> m_scroll_x_lo;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(wc90b_bankswitch_w);
	DECLARE_WRITE8_MEMBER(wc90b_bankswitch1_w);
	DECLARE_WRITE8_MEMBER(wc90b_sound_command_w);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_WRITE8_MEMBER(wc90b_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(wc90b_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(wc90b_txvideoram_w);
};


/*----------- defined in video/wc90b.c -----------*/

VIDEO_START( wc90b );
SCREEN_UPDATE_IND16( wc90b );

