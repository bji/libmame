/*************************************************************************

    4enraya

*************************************************************************/

class _4enraya_state : public driver_device
{
public:
	_4enraya_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8	   m_videoram[0x1000];
	UINT8      m_workram[0x1000];

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* sound-related */
	int        m_soundlatch;
	int        m_last_snd_ctrl;
};


/*----------- defined in video/4enraya.c -----------*/

WRITE8_HANDLER( fenraya_videoram_w );

VIDEO_START( 4enraya );
SCREEN_UPDATE( 4enraya );
