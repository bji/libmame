class speedbal_state : public driver_device
{
public:
	speedbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_background_videoram(*this, "bg_videoram"),
		m_foreground_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT8> m_background_videoram;
	required_shared_ptr<UINT8> m_foreground_videoram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(speedbal_coincounter_w);
	DECLARE_WRITE8_MEMBER(speedbal_foreground_videoram_w);
	DECLARE_WRITE8_MEMBER(speedbal_background_videoram_w);
};


/*----------- defined in video/speedbal.c -----------*/

VIDEO_START( speedbal );
SCREEN_UPDATE_IND16( speedbal );
