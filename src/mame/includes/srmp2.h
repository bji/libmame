struct iox_t
{
	int reset,ff_event,ff_1,protcheck[4],protlatch[4];
	UINT8 data;
	UINT8 mux;
	UINT8 ff;
};

class srmp2_state : public driver_device
{
public:
	srmp2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_color_bank;
	int m_gfx_bank;

	int m_adpcm_bank;
	int m_adpcm_data;
	UINT32 m_adpcm_sptr;
	UINT32 m_adpcm_eptr;

	int m_port_select;

	iox_t m_iox;
	DECLARE_WRITE16_MEMBER(srmp2_flags_w);
	DECLARE_WRITE16_MEMBER(mjyuugi_flags_w);
	DECLARE_WRITE16_MEMBER(mjyuugi_adpcm_bank_w);
	DECLARE_READ8_MEMBER(vox_status_r);
	DECLARE_READ8_MEMBER(iox_mux_r);
	DECLARE_READ8_MEMBER(iox_status_r);
	DECLARE_WRITE8_MEMBER(iox_command_w);
	DECLARE_WRITE8_MEMBER(iox_data_w);
	DECLARE_WRITE8_MEMBER(srmp3_rombank_w);
	DECLARE_WRITE8_MEMBER(srmp2_irq2_ack_w);
	DECLARE_WRITE8_MEMBER(srmp2_irq4_ack_w);
	DECLARE_READ8_MEMBER(mjyuugi_irq2_ack_r);
	DECLARE_READ8_MEMBER(mjyuugi_irq4_ack_r);
	DECLARE_WRITE8_MEMBER(srmp3_flags_w);
	DECLARE_WRITE8_MEMBER(srmp3_irq_ack_w);
	DECLARE_WRITE8_MEMBER(rmgoldyh_rombank_w);
};


/*----------- defined in video/srmp2.c -----------*/

PALETTE_INIT( srmp2 );
SCREEN_UPDATE_IND16( srmp2 );
PALETTE_INIT( srmp3 );
SCREEN_UPDATE_IND16( srmp3 );
SCREEN_UPDATE_IND16( mjyuugi );
