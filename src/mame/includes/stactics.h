/****************************************************************************

    Sega "Space Tactics" Driver

    Frank Palazzolo (palazzol@home.com)

****************************************************************************/


class stactics_state : public driver_device
{
public:
	stactics_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_palette(*this, "palette"),
		m_motor_on(*this, "motor_on"),
		m_lamps(*this, "lamps"),
		m_display_buffer(*this, "display_buffer"),
		m_videoram_b(*this, "videoram_b"),
		m_videoram_d(*this, "videoram_d"),
		m_videoram_e(*this, "videoram_e"),
		m_videoram_f(*this, "videoram_f"){ }

	/* machine state */
	int    m_vert_pos;
	int    m_horiz_pos;
	/* video state */

	required_shared_ptr<UINT8> m_palette;
	required_shared_ptr<UINT8> m_motor_on;
	required_shared_ptr<UINT8> m_lamps;
	required_shared_ptr<UINT8> m_display_buffer;
	required_shared_ptr<UINT8> m_videoram_b;
	required_shared_ptr<UINT8> m_videoram_d;
	required_shared_ptr<UINT8> m_videoram_e;
	required_shared_ptr<UINT8> m_videoram_f;

	UINT8  m_y_scroll_d;
	UINT8  m_y_scroll_e;
	UINT8  m_y_scroll_f;
	UINT8  m_frame_count;
	UINT8  m_shot_standby;
	UINT8  m_shot_arrive;
	UINT16 m_beam_state;
	UINT16 m_old_beam_state;
	UINT16 m_beam_states_per_frame;
	DECLARE_READ8_MEMBER(vert_pos_r);
	DECLARE_READ8_MEMBER(horiz_pos_r);
	DECLARE_WRITE8_MEMBER(stactics_coin_lockout_w);
	DECLARE_WRITE8_MEMBER(stactics_scroll_ram_w);
	DECLARE_WRITE8_MEMBER(stactics_speed_latch_w);
	DECLARE_WRITE8_MEMBER(stactics_shot_trigger_w);
	DECLARE_WRITE8_MEMBER(stactics_shot_flag_clear_w);
	DECLARE_CUSTOM_INPUT_MEMBER(stactics_get_frame_count_d3);
	DECLARE_CUSTOM_INPUT_MEMBER(stactics_get_shot_standby);
	DECLARE_CUSTOM_INPUT_MEMBER(stactics_get_not_shot_arrive);
	DECLARE_CUSTOM_INPUT_MEMBER(get_motor_not_ready);
	DECLARE_CUSTOM_INPUT_MEMBER(get_rng);
};


/*----------- defined in video/stactics.c -----------*/

MACHINE_CONFIG_EXTERN( stactics_video );



