/*************************************************************************

    Tail to Nose / Super Formula

*************************************************************************/

class tail2nos_state : public driver_device
{
public:
	tail2nos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_bgvideoram;
	UINT16 *    m_spriteram;
	UINT16 *    m_zoomdata;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t   *m_bg_tilemap;
	int         m_charbank;
	int         m_charpalette;
	int         m_video_enable;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k051316;
};


/*----------- defined in video/tail2nos.c -----------*/

extern void tail2nos_zoom_callback(running_machine &machine, int *code,int *color,int *flags);

WRITE16_HANDLER( tail2nos_bgvideoram_w );
READ16_HANDLER( tail2nos_zoomdata_r );
WRITE16_HANDLER( tail2nos_zoomdata_w );
WRITE16_HANDLER( tail2nos_gfxbank_w );

VIDEO_START( tail2nos );
SCREEN_UPDATE( tail2nos );
