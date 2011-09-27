class terracre_state : public driver_device
{
public:
	terracre_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_videoram;
	const UINT16 *m_mpProtData;
	UINT8 m_mAmazonProtCmd;
	UINT8 m_mAmazonProtReg[6];
	UINT16 *m_amazon_videoram;
	UINT16 m_xscroll;
	UINT16 m_yscroll;
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	UINT16 *m_spriteram;
};


/*----------- defined in video/terracre.c -----------*/

PALETTE_INIT( amazon );
WRITE16_HANDLER( amazon_background_w );
WRITE16_HANDLER( amazon_foreground_w );
WRITE16_HANDLER( amazon_scrolly_w );
WRITE16_HANDLER( amazon_scrollx_w );
WRITE16_HANDLER( amazon_flipscreen_w );
VIDEO_START( amazon );
SCREEN_UPDATE( amazon );
