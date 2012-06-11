class lucky74_state : public driver_device
{
public:
	lucky74_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fg_videoram(*this, "fg_videoram"),
		m_fg_colorram(*this, "fg_colorram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_colorram(*this, "bg_colorram"){ }

	UINT8 m_ym2149_portb;
	UINT8 m_usart_8251;
	UINT8 m_copro_sm7831;
	int m_adpcm_pos;
	int m_adpcm_end;
	int m_adpcm_data;
	UINT8 m_adpcm_reg[6];
	UINT8 m_adpcm_busy_line;
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_fg_colorram;
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_bg_colorram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(custom_09R81P_port_r);
	DECLARE_WRITE8_MEMBER(custom_09R81P_port_w);
	DECLARE_READ8_MEMBER(usart_8251_r);
	DECLARE_WRITE8_MEMBER(usart_8251_w);
	DECLARE_READ8_MEMBER(copro_sm7831_r);
	DECLARE_WRITE8_MEMBER(copro_sm7831_w);
	DECLARE_WRITE8_MEMBER(lucky74_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(lucky74_fg_colorram_w);
	DECLARE_WRITE8_MEMBER(lucky74_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(lucky74_bg_colorram_w);
};


/*----------- defined in video/lucky74.c -----------*/

PALETTE_INIT( lucky74 );
VIDEO_START( lucky74 );
SCREEN_UPDATE_IND16( lucky74 );
