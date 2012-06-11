/*************************************************************************

    Commando

*************************************************************************/

class commando_state : public driver_device
{
public:
	commando_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_videoram2;
	UINT8 *  m_colorram2;
//  UINT8 *  m_spriteram; // currently this uses generic buffered_spriteram

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	UINT8 m_scroll_x[2];
	UINT8 m_scroll_y[2];

	/* devices */
	device_t *m_audiocpu;
};



/*----------- defined in video/commando.c -----------*/

WRITE8_HANDLER( commando_videoram_w );
WRITE8_HANDLER( commando_colorram_w );
WRITE8_HANDLER( commando_videoram2_w );
WRITE8_HANDLER( commando_colorram2_w );
WRITE8_HANDLER( commando_scrollx_w );
WRITE8_HANDLER( commando_scrolly_w );
WRITE8_HANDLER( commando_c804_w );

VIDEO_START( commando );
SCREEN_UPDATE_IND16( commando );
SCREEN_VBLANK( commando );
