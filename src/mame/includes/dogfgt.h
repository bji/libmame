
#define PIXMAP_COLOR_BASE  (16 + 32)
#define BITMAPRAM_SIZE      0x6000


class dogfgt_state : public driver_device
{
public:
	dogfgt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_bgvideoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_sharedram;
//  UINT8 *    m_paletteram;  // currently this uses generic palette handling
	size_t     m_spriteram_size;

	/* video-related */
	bitmap_t  *m_pixbitmap;
	tilemap_t   *m_bg_tilemap;
	UINT8     *m_bitmapram;
	int       m_bm_plane;
	int       m_pixcolor;
	int       m_scroll[4];
	int       m_lastflip;
	int       m_lastpixcolor;

	/* sound-related */
	int       m_soundlatch;
	int       m_last_snd_ctrl;

	/* devices */
	device_t *m_subcpu;
};


/*----------- defined in video/dogfgt.c -----------*/

WRITE8_HANDLER( dogfgt_plane_select_w );
READ8_HANDLER( dogfgt_bitmapram_r );
WRITE8_HANDLER( dogfgt_bitmapram_w );
WRITE8_HANDLER( dogfgt_bgvideoram_w );
WRITE8_HANDLER( dogfgt_scroll_w );
WRITE8_HANDLER( dogfgt_1800_w );

PALETTE_INIT( dogfgt );
VIDEO_START( dogfgt );
SCREEN_UPDATE( dogfgt );
