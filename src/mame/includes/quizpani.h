class quizpani_state : public driver_device
{
public:
	quizpani_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_scrollreg(*this, "scrollreg"),
		m_bg_videoram(*this, "bg_videoram"),
		m_txt_videoram(*this, "txt_videoram"){ }

	required_shared_ptr<UINT16> m_scrollreg;
	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_txt_videoram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_txt_tilemap;
	int m_bgbank;
	int m_txtbank;
	DECLARE_WRITE16_MEMBER(quizpani_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(quizpani_txt_videoram_w);
	DECLARE_WRITE16_MEMBER(quizpani_tilesbank_w);
};


/*----------- defined in video/quizpani.c -----------*/


VIDEO_START( quizpani );
SCREEN_UPDATE_IND16( quizpani );
