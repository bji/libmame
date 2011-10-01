/*************************************************************************

    KO Punch

*************************************************************************/

class kopunch_state : public driver_device
{
public:
	kopunch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_videoram2;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	int        m_gfxbank;

	/* devices */
	device_t *m_maincpu;
};

/*----------- defined in video/kopunch.c -----------*/

WRITE8_HANDLER( kopunch_videoram_w );
WRITE8_HANDLER( kopunch_videoram2_w );
WRITE8_HANDLER( kopunch_scroll_x_w );
WRITE8_HANDLER( kopunch_scroll_y_w );
WRITE8_HANDLER( kopunch_gfxbank_w );

PALETTE_INIT( kopunch );
VIDEO_START( kopunch );
SCREEN_UPDATE( kopunch );
