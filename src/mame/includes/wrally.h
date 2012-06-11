class wrally_state : public driver_device
{
public:
	wrally_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram"){ }

	tilemap_t *m_pant[2];
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_vregs;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_shareram;

	DECLARE_READ8_MEMBER(dallas_share_r);
	DECLARE_WRITE8_MEMBER(dallas_share_w);
	DECLARE_WRITE16_MEMBER(wrally_vram_w);
	DECLARE_WRITE16_MEMBER(wrally_flipscreen_w);
	DECLARE_WRITE16_MEMBER(OKIM6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(wrally_coin_counter_w);
	DECLARE_WRITE16_MEMBER(wrally_coin_lockout_w);
};


/*----------- defined in machine/wrally.c -----------*/


/*----------- defined in video/wrally.c -----------*/

VIDEO_START( wrally );
SCREEN_UPDATE_IND16( wrally );

