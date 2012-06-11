class exzisus_state : public driver_device
{
public:
	exzisus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_objectram1(*this, "objectram1"),
		m_videoram1(*this, "videoram1"),
		m_sharedram_ac(*this, "sharedram_ac"),
		m_sharedram_ab(*this, "sharedram_ab"),
		m_objectram0(*this, "objectram0"),
		m_videoram0(*this, "videoram0"){ }

	required_shared_ptr<UINT8> m_objectram1;
	required_shared_ptr<UINT8> m_videoram1;
	required_shared_ptr<UINT8> m_sharedram_ac;
	required_shared_ptr<UINT8> m_sharedram_ab;
	required_shared_ptr<UINT8> m_objectram0;
	required_shared_ptr<UINT8> m_videoram0;

	int m_cpua_bank;
	int m_cpub_bank;

	DECLARE_WRITE8_MEMBER(exzisus_cpua_bankswitch_w);
	DECLARE_WRITE8_MEMBER(exzisus_cpub_bankswitch_w);
	DECLARE_WRITE8_MEMBER(exzisus_coincounter_w);
	DECLARE_READ8_MEMBER(exzisus_sharedram_ab_r);
	DECLARE_READ8_MEMBER(exzisus_sharedram_ac_r);
	DECLARE_WRITE8_MEMBER(exzisus_sharedram_ab_w);
	DECLARE_WRITE8_MEMBER(exzisus_sharedram_ac_w);
	DECLARE_WRITE8_MEMBER(exzisus_cpub_reset_w);
	DECLARE_READ8_MEMBER(exzisus_videoram_0_r);
	DECLARE_READ8_MEMBER(exzisus_videoram_1_r);
	DECLARE_READ8_MEMBER(exzisus_objectram_0_r);
	DECLARE_READ8_MEMBER(exzisus_objectram_1_r);
	DECLARE_WRITE8_MEMBER(exzisus_videoram_0_w);
	DECLARE_WRITE8_MEMBER(exzisus_videoram_1_w);
	DECLARE_WRITE8_MEMBER(exzisus_objectram_0_w);
	DECLARE_WRITE8_MEMBER(exzisus_objectram_1_w);
};


/*----------- defined in video/exzisus.c -----------*/


SCREEN_UPDATE_IND16( exzisus );


