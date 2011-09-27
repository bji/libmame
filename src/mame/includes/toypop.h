class toypop_state : public driver_device
{
public:
	toypop_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_videoram;
	UINT8 *m_spriteram;
	UINT16 *m_bg_image;
	UINT8 *m_m68000_sharedram;
	tilemap_t *m_bg_tilemap;

	int m_bitmapflip;
	int m_palettebank;
	int m_interrupt_enable_68k;
};


/*----------- defined in video/toypop.c -----------*/

WRITE8_HANDLER( toypop_videoram_w );
READ16_HANDLER( toypop_merged_background_r );
WRITE16_HANDLER( toypop_merged_background_w );
WRITE8_HANDLER( toypop_palettebank_w );
WRITE16_HANDLER( toypop_flipscreen_w );
VIDEO_START( toypop );
SCREEN_UPDATE( toypop );
PALETTE_INIT( toypop );
