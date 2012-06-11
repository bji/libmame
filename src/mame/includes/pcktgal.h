class pcktgal_state : public driver_device
{
public:
	pcktgal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"){ }

	int m_msm5205next;
	int m_toggle;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(pcktgal_bank_w);
	DECLARE_WRITE8_MEMBER(pcktgal_sound_bank_w);
	DECLARE_WRITE8_MEMBER(pcktgal_sound_w);
	DECLARE_WRITE8_MEMBER(pcktgal_adpcm_data_w);
};


/*----------- defined in video/pcktgal.c -----------*/


PALETTE_INIT( pcktgal );
SCREEN_UPDATE_IND16( pcktgal );
SCREEN_UPDATE_IND16( pcktgalb );
