/****************************************************************************

    Sega "Space Tactics" Driver

    Frank Palazzolo (palazzol@home.com)

****************************************************************************/


class stactics_state : public driver_device
{
public:
	stactics_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* machine state */
	int    m_vert_pos;
	int    m_horiz_pos;
	UINT8 *m_motor_on;

	/* video state */
	UINT8 *m_videoram_b;
	UINT8 *m_videoram_d;
	UINT8 *m_videoram_e;
	UINT8 *m_videoram_f;
	UINT8 *m_palette;
	UINT8 *m_display_buffer;
	UINT8 *m_lamps;

	UINT8  m_y_scroll_d;
	UINT8  m_y_scroll_e;
	UINT8  m_y_scroll_f;
	UINT8  m_frame_count;
	UINT8  m_shot_standby;
	UINT8  m_shot_arrive;
	UINT16 m_beam_state;
	UINT16 m_old_beam_state;
	UINT16 m_beam_states_per_frame;
};


/*----------- defined in video/stactics.c -----------*/

MACHINE_CONFIG_EXTERN( stactics_video );

WRITE8_HANDLER( stactics_scroll_ram_w );
WRITE8_HANDLER( stactics_speed_latch_w );
WRITE8_HANDLER( stactics_shot_trigger_w );
WRITE8_HANDLER( stactics_shot_flag_clear_w );
CUSTOM_INPUT( stactics_get_frame_count_d3 );
CUSTOM_INPUT( stactics_get_shot_standby );
CUSTOM_INPUT( stactics_get_not_shot_arrive );

