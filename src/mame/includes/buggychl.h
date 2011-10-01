/*
    buggychl
*/

class buggychl_state : public driver_device
{
public:
	buggychl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *     m_videoram;
	UINT8 *     m_spriteram;
	UINT8 *     m_scrollv;
	UINT8 *     m_scrollh;
	UINT8 *     m_charram;
	size_t      m_videoram_size;
	size_t      m_spriteram_size;

	/* video-related */
	bitmap_t    *m_tmp_bitmap1;
	bitmap_t    *m_tmp_bitmap2;
	tilemap_t     *m_bg_tilemap;
	int         m_sl_bank;
	int         m_bg_on;
	int         m_sky_on;
	int         m_sprite_color_base;
	int         m_bg_scrollx;
	UINT8       m_sprite_lookup[0x2000];

	/* sound-related */
	int         m_sound_nmi_enable;
	int         m_pending_nmi;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/buggychl.c -----------*/

WRITE8_HANDLER( buggychl_chargen_w );
WRITE8_HANDLER( buggychl_sprite_lookup_bank_w );
WRITE8_HANDLER( buggychl_sprite_lookup_w );
WRITE8_HANDLER( buggychl_ctrl_w );
WRITE8_HANDLER( buggychl_bg_scrollx_w );

PALETTE_INIT( buggychl );
VIDEO_START( buggychl );
SCREEN_UPDATE( buggychl );
