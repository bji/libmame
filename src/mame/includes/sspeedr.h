class sspeedr_state : public driver_device
{
public:
	sspeedr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_led_TIME[2];
	UINT8 m_led_SCORE[24];
	int m_toggle;
	unsigned m_driver_horz;
	unsigned m_driver_vert;
	unsigned m_driver_pic;
	unsigned m_drones_horz;
	unsigned m_drones_vert[3];
	unsigned m_drones_mask;
	unsigned m_track_horz;
	unsigned m_track_vert[2];
	unsigned m_track_ice;
};


/*----------- defined in video/sspeedr.c -----------*/

WRITE8_HANDLER( sspeedr_driver_horz_w );
WRITE8_HANDLER( sspeedr_driver_horz_2_w );
WRITE8_HANDLER( sspeedr_driver_vert_w );
WRITE8_HANDLER( sspeedr_driver_pic_w );

WRITE8_HANDLER( sspeedr_drones_horz_w );
WRITE8_HANDLER( sspeedr_drones_horz_2_w );
WRITE8_HANDLER( sspeedr_drones_vert_w );
WRITE8_HANDLER( sspeedr_drones_mask_w );

WRITE8_HANDLER( sspeedr_track_horz_w );
WRITE8_HANDLER( sspeedr_track_horz_2_w );
WRITE8_HANDLER( sspeedr_track_vert_w );
WRITE8_HANDLER( sspeedr_track_ice_w );

VIDEO_START( sspeedr );
SCREEN_UPDATE( sspeedr );
SCREEN_EOF( sspeedr );
