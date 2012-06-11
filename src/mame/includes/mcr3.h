class mcr3_state : public mcr_state
{
public:
	mcr3_state(const machine_config &mconfig, device_type type, const char *tag)
		: mcr_state(mconfig, type, tag),
		  m_spyhunt_alpharam(*this, "spyhunt_alpha") { }

	UINT8 m_input_mux;
	UINT8 m_latched_input;
	UINT8 m_last_op4;
	UINT8 m_maxrpm_adc_control;
	UINT8 m_maxrpm_adc_select;
	UINT8 m_maxrpm_last_shift;
	INT8 m_maxrpm_p1_shift;
	INT8 m_maxrpm_p2_shift;
	UINT8 m_spyhunt_sprite_color_mask;
	INT16 m_spyhunt_scroll_offset;
	optional_shared_ptr<UINT8> m_spyhunt_alpharam;
	INT16 m_spyhunt_scrollx;
	INT16 m_spyhunt_scrolly;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_alpha_tilemap;
	DECLARE_WRITE8_MEMBER(mcr3_paletteram_w);
	DECLARE_WRITE8_MEMBER(mcr3_videoram_w);
	DECLARE_WRITE8_MEMBER(spyhunt_videoram_w);
	DECLARE_WRITE8_MEMBER(spyhunt_alpharam_w);
	DECLARE_WRITE8_MEMBER(spyhunt_scroll_value_w);
	DECLARE_WRITE8_MEMBER(mcrmono_control_port_w);
	DECLARE_READ8_MEMBER(demoderm_ip1_r);
	DECLARE_READ8_MEMBER(demoderm_ip2_r);
	DECLARE_WRITE8_MEMBER(demoderm_op6_w);
	DECLARE_READ8_MEMBER(maxrpm_ip1_r);
	DECLARE_READ8_MEMBER(maxrpm_ip2_r);
	DECLARE_WRITE8_MEMBER(maxrpm_op5_w);
	DECLARE_WRITE8_MEMBER(maxrpm_op6_w);
	DECLARE_READ8_MEMBER(rampage_ip4_r);
	DECLARE_WRITE8_MEMBER(rampage_op6_w);
	DECLARE_READ8_MEMBER(powerdrv_ip2_r);
	DECLARE_WRITE8_MEMBER(powerdrv_op5_w);
	DECLARE_WRITE8_MEMBER(powerdrv_op6_w);
	DECLARE_READ8_MEMBER(stargrds_ip0_r);
	DECLARE_WRITE8_MEMBER(stargrds_op5_w);
	DECLARE_WRITE8_MEMBER(stargrds_op6_w);
	DECLARE_READ8_MEMBER(spyhunt_ip1_r);
	DECLARE_READ8_MEMBER(spyhunt_ip2_r);
	DECLARE_WRITE8_MEMBER(spyhunt_op4_w);
	DECLARE_READ8_MEMBER(turbotag_ip2_r);
	DECLARE_READ8_MEMBER(turbotag_kludge_r);
};


/*----------- defined in video/mcr3.c -----------*/


VIDEO_START( mcrmono );
VIDEO_START( spyhunt );

PALETTE_INIT( spyhunt );

SCREEN_UPDATE_IND16( mcr3 );
SCREEN_UPDATE_IND16( spyhunt );
