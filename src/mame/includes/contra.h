/*************************************************************************

    Contra / Gryzor

*************************************************************************/

class contra_state : public driver_device
{
public:
	contra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_spriteram;
	UINT8 *        m_buffered_spriteram;
	UINT8 *        m_buffered_spriteram_2;
	UINT8 *        m_paletteram;
	UINT8 *        m_bg_vram;
	UINT8 *        m_bg_cram;
	UINT8 *        m_fg_vram;
	UINT8 *        m_fg_cram;
	UINT8 *        m_tx_vram;
	UINT8 *        m_tx_cram;
	// this driver also uses a large generic spriteram region...

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;
	rectangle m_bg_clip;
	rectangle m_fg_clip;
	rectangle m_tx_clip;

	/* devices */
	device_t *m_audiocpu;
	device_t *m_k007121_1;
	device_t *m_k007121_2;
};


/*----------- defined in video/contra.c -----------*/

PALETTE_INIT( contra );

WRITE8_HANDLER( contra_fg_vram_w );
WRITE8_HANDLER( contra_fg_cram_w );
WRITE8_HANDLER( contra_bg_vram_w );
WRITE8_HANDLER( contra_bg_cram_w );
WRITE8_HANDLER( contra_text_vram_w );
WRITE8_HANDLER( contra_text_cram_w );

WRITE8_HANDLER( contra_K007121_ctrl_0_w );
WRITE8_HANDLER( contra_K007121_ctrl_1_w );

SCREEN_UPDATE_IND16( contra );
VIDEO_START( contra );
