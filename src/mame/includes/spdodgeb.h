class spdodgeb_state : public driver_device
{
public:
	spdodgeb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this,"maincpu"){ }

	int m_toggle;
	int m_adpcm_pos[2];
	int m_adpcm_end[2];
	int m_adpcm_idle[2];
	int m_adpcm_data[2];
	int m_mcu63701_command;
	int m_inputs[4];
	UINT8 m_tapc[4];
	UINT8 m_last_port[2];
	UINT8 m_last_dash[2];
#if 0
	int m_running[2];
	int m_jumped[2];
	int m_prev[2][2];
	int m_countup[2][2];
	int m_countdown[2][2];
	int m_prev[2];
#endif
	required_shared_ptr<UINT8> m_videoram;
	int m_tile_palbank;
	int m_sprite_palbank;
	tilemap_t *m_bg_tilemap;
	int m_lastscroll;
	required_shared_ptr<UINT8> m_spriteram;

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(spd_adpcm_w);
	DECLARE_READ8_MEMBER(mcu63701_r);
	DECLARE_WRITE8_MEMBER(mcu63701_w);
	DECLARE_READ8_MEMBER(port_0_r);
	DECLARE_WRITE8_MEMBER(spdodgeb_scrollx_lo_w);
	DECLARE_WRITE8_MEMBER(spdodgeb_ctrl_w);
	DECLARE_WRITE8_MEMBER(spdodgeb_videoram_w);
};


/*----------- defined in video/spdodgeb.c -----------*/

PALETTE_INIT( spdodgeb );
VIDEO_START( spdodgeb );
SCREEN_UPDATE_IND16( spdodgeb );
TIMER_DEVICE_CALLBACK( spdodgeb_interrupt );
