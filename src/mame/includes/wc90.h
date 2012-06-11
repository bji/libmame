class wc90_state : public driver_device
{
public:
	wc90_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_scroll0xlo(*this, "scroll0xlo"),
		m_scroll0xhi(*this, "scroll0xhi"),
		m_scroll1xlo(*this, "scroll1xlo"),
		m_scroll1xhi(*this, "scroll1xhi"),
		m_scroll2xlo(*this, "scroll2xlo"),
		m_scroll2xhi(*this, "scroll2xhi"),
		m_scroll0ylo(*this, "scroll0ylo"),
		m_scroll0yhi(*this, "scroll0yhi"),
		m_scroll1ylo(*this, "scroll1ylo"),
		m_scroll1yhi(*this, "scroll1yhi"),
		m_scroll2ylo(*this, "scroll2ylo"),
		m_scroll2yhi(*this, "scroll2yhi"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_txvideoram;
	required_shared_ptr<UINT8> m_scroll0xlo;
	required_shared_ptr<UINT8> m_scroll0xhi;
	required_shared_ptr<UINT8> m_scroll1xlo;
	required_shared_ptr<UINT8> m_scroll1xhi;
	required_shared_ptr<UINT8> m_scroll2xlo;
	required_shared_ptr<UINT8> m_scroll2xhi;
	required_shared_ptr<UINT8> m_scroll0ylo;
	required_shared_ptr<UINT8> m_scroll0yhi;
	required_shared_ptr<UINT8> m_scroll1ylo;
	required_shared_ptr<UINT8> m_scroll1yhi;
	required_shared_ptr<UINT8> m_scroll2ylo;
	required_shared_ptr<UINT8> m_scroll2yhi;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(wc90_bankswitch_w);
	DECLARE_WRITE8_MEMBER(wc90_bankswitch1_w);
	DECLARE_WRITE8_MEMBER(wc90_sound_command_w);
	DECLARE_WRITE8_MEMBER(wc90_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(wc90_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(wc90_txvideoram_w);
};


/*----------- defined in video/wc90.c -----------*/

VIDEO_START( wc90 );
VIDEO_START( wc90t );
SCREEN_UPDATE_IND16( wc90 );
