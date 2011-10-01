/*************************************************************************

    Gun Dealer

*************************************************************************/

class gundealr_state : public driver_device
{
public:
	gundealr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_bg_videoram;
	UINT8 *    m_fg_videoram;
	UINT8 *    m_rambase;
	UINT8 *    m_paletteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	int        m_flipscreen;
	UINT8      m_scroll[4];

	/* misc */
	int        m_input_ports_hack;
};



/*----------- defined in video/gundealr.c -----------*/

WRITE8_HANDLER( gundealr_paletteram_w );
WRITE8_HANDLER( gundealr_bg_videoram_w );
WRITE8_HANDLER( gundealr_fg_videoram_w );
WRITE8_HANDLER( gundealr_fg_scroll_w );
WRITE8_HANDLER( yamyam_fg_scroll_w );
WRITE8_HANDLER( gundealr_flipscreen_w );

SCREEN_UPDATE( gundealr );
VIDEO_START( gundealr );
