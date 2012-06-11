/*************************************************************************

    King of Boxer - Ring King

*************************************************************************/

class kingofb_state : public driver_device
{
public:
	kingofb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_videoram2;
	UINT8 *    m_colorram;
	UINT8 *    m_colorram2;
	UINT8 *    m_spriteram;
	UINT8 *    m_scroll_y;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	int        m_palette_bank;

	/* misc */
	int        m_nmi_enable;

	/* devices */
	device_t *m_video_cpu;
	device_t *m_sprite_cpu;
	device_t *m_audio_cpu;
};


/*----------- defined in video/kingobox.c -----------*/

WRITE8_HANDLER( kingofb_videoram_w );
WRITE8_HANDLER( kingofb_colorram_w );
WRITE8_HANDLER( kingofb_videoram2_w );
WRITE8_HANDLER( kingofb_colorram2_w );
WRITE8_HANDLER( kingofb_f800_w );

PALETTE_INIT( kingofb );
VIDEO_START( kingofb );
SCREEN_UPDATE_IND16( kingofb );

PALETTE_INIT( ringking );
VIDEO_START( ringking );
SCREEN_UPDATE_IND16( ringking );
