/* Disabled because the mcu dump is currently unavailable. -AS */
//#define USE_MCU

class msisaac_state : public driver_device
{
public:
	msisaac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *     m_videoram;
	UINT8 *     m_videoram2;
	UINT8 *     m_videoram3;
	UINT8 *     m_spriteram;
	size_t      m_videoram_size;
	size_t      m_spriteram_size;
	size_t      m_spriteram2_size;

	/* video-related */
	bitmap_ind16    *m_tmp_bitmap1;
	bitmap_ind16    *m_tmp_bitmap2;
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_bg2_tilemap;
	int         m_bg2_textbank;

	/* sound-related */
	int         m_sound_nmi_enable;
	int         m_pending_nmi;

	/* fake mcu (in msisaac.c) */
#ifndef USE_MCU
	UINT8       m_mcu_val;
	UINT8       m_direction;
#endif

	int         m_vol_ctrl[16];
	UINT8       m_snd_ctrl0;
	UINT8       m_snd_ctrl1;
	UINT8       m_snd_ctrl2;
	UINT8       m_snd_ctrl3;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/msisaac.c -----------*/

WRITE8_HANDLER( msisaac_fg_scrolly_w );
WRITE8_HANDLER( msisaac_fg_scrollx_w );
WRITE8_HANDLER( msisaac_bg_scrolly_w );
WRITE8_HANDLER( msisaac_bg_scrollx_w );
WRITE8_HANDLER( msisaac_bg2_scrolly_w );
WRITE8_HANDLER( msisaac_bg2_scrollx_w );
WRITE8_HANDLER( msisaac_bg2_textbank_w );

WRITE8_HANDLER( msisaac_bg_videoram_w );
WRITE8_HANDLER( msisaac_bg2_videoram_w );
WRITE8_HANDLER( msisaac_fg_videoram_w );

SCREEN_UPDATE_IND16( msisaac );
VIDEO_START( msisaac );
