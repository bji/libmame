/*************************************************************************

    Kitco Crowns Golf hardware

**************************************************************************/

#define MASTER_CLOCK		18432000


class crgolf_state : public driver_device
{
public:
	crgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram_a;
	UINT8 *  m_videoram_b;
	UINT8 *  m_color_select;
	UINT8 *  m_screen_flip;
	UINT8 *  m_screen_select;
	UINT8 *  m_screena_enable;
	UINT8 *  m_screenb_enable;

	/* misc */
	UINT8    m_port_select;
	UINT8    m_main_to_sound_data;
	UINT8    m_sound_to_main_data;
	UINT16   m_sample_offset;
	UINT8    m_sample_count;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};

/*----------- defined in video/crgolf.c -----------*/

WRITE8_HANDLER( crgolf_videoram_w );
READ8_HANDLER( crgolf_videoram_r );

MACHINE_CONFIG_EXTERN( crgolf_video );
