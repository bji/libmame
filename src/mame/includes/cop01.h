/*************************************************************************

    Cops 01

*************************************************************************/

class cop01_state : public driver_device
{
public:
	cop01_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_bgvideoram;
	UINT8 *        m_fgvideoram;
	UINT8 *        m_spriteram;
	size_t         m_spriteram_size;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	UINT8          m_vreg[4];

	/* sound-related */
	int            m_pulse;
	int            m_timer;	// kludge for ym3526 in mightguy

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
};



/*----------- defined in video/cop01.c -----------*/


PALETTE_INIT( cop01 );
VIDEO_START( cop01 );
SCREEN_UPDATE( cop01 );

WRITE8_HANDLER( cop01_background_w );
WRITE8_HANDLER( cop01_foreground_w );
WRITE8_HANDLER( cop01_vreg_w );
