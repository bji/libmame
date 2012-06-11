/***************************************************************************

    Time Pilot

***************************************************************************/

class timeplt_state : public driver_device
{
public:
	timeplt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_spriteram;
	UINT8 *  m_spriteram2;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	UINT8    m_nmi_enable;

	/* devices */
	cpu_device *m_maincpu;
};


/*----------- defined in video/timeplt.c -----------*/

READ8_HANDLER( timeplt_scanline_r );
WRITE8_HANDLER( timeplt_videoram_w );
WRITE8_HANDLER( timeplt_colorram_w );
WRITE8_HANDLER( timeplt_flipscreen_w );

PALETTE_INIT( timeplt );
VIDEO_START( timeplt );
VIDEO_START( chkun );
SCREEN_UPDATE_IND16( timeplt );
