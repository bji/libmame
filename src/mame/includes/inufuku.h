
class inufuku_state : public driver_device
{
public:
	inufuku_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  m_bg_videoram;
	UINT16 *  m_bg_rasterram;
	UINT16 *  m_tx_videoram;
	UINT16 *  m_spriteram1;
	UINT16 *  m_spriteram2;
//      UINT16 *  m_paletteram;    // currently this uses generic palette handling
	size_t    m_spriteram1_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_tx_tilemap;
	int       m_bg_scrollx;
	int       m_bg_scrolly;
	int       m_tx_scrollx;
	int       m_tx_scrolly;
	int       m_bg_raster;
	int       m_bg_palettebank;
	int       m_tx_palettebank;

	/* misc */
	UINT16    m_pending_command;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/inufuku.c -----------*/

READ16_HANDLER( inufuku_bg_videoram_r );
WRITE16_HANDLER( inufuku_bg_videoram_w );
READ16_HANDLER( inufuku_tx_videoram_r );
WRITE16_HANDLER( inufuku_tx_videoram_w );
WRITE16_HANDLER( inufuku_palettereg_w );
WRITE16_HANDLER( inufuku_scrollreg_w );

SCREEN_UPDATE( inufuku );
VIDEO_START( inufuku );
