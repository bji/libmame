class shangkid_state : public driver_device
{
public:
	shangkid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_spriteram;
	UINT8 m_bbx_sound_enable;
	UINT8 m_sound_latch;
	UINT8 *m_videoreg;
	int m_gfx_type;
	tilemap_t *m_background;
};


/*----------- defined in video/shangkid.c -----------*/

VIDEO_START( shangkid );
SCREEN_UPDATE( shangkid );
WRITE8_HANDLER( shangkid_videoram_w );

PALETTE_INIT( dynamski );
SCREEN_UPDATE( dynamski );

