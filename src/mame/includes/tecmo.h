class tecmo_state : public driver_device
{
public:
	tecmo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_txvideoram(*this, "txvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"){ }

	int m_adpcm_pos;
	int m_adpcm_end;
	int m_adpcm_data;
	int m_video_type;
	required_shared_ptr<UINT8> m_txvideoram;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 m_fgscroll[3];
	UINT8 m_bgscroll[3];
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(tecmo_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tecmo_sound_command_w);
	DECLARE_WRITE8_MEMBER(tecmo_nmi_ack_w);
	DECLARE_WRITE8_MEMBER(tecmo_adpcm_end_w);
	DECLARE_READ8_MEMBER(tecmo_dswa_l_r);
	DECLARE_READ8_MEMBER(tecmo_dswa_h_r);
	DECLARE_READ8_MEMBER(tecmo_dswb_l_r);
	DECLARE_READ8_MEMBER(tecmo_dswb_h_r);
	DECLARE_WRITE8_MEMBER(tecmo_txvideoram_w);
	DECLARE_WRITE8_MEMBER(tecmo_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(tecmo_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(tecmo_fgscroll_w);
	DECLARE_WRITE8_MEMBER(tecmo_bgscroll_w);
	DECLARE_WRITE8_MEMBER(tecmo_flipscreen_w);
};


/*----------- defined in video/tecmo.c -----------*/


VIDEO_START( tecmo );
SCREEN_UPDATE_IND16( tecmo );
