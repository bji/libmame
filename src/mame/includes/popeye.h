class popeye_state : public driver_device
{
public:
	popeye_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 m_prot0;
	UINT8 m_prot1;
	UINT8 m_prot_shift;
	int m_dswbit;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_background_pos;
	UINT8 *m_palettebank;
	UINT8 *m_bitmapram;
	bitmap_t *m_tmpbitmap2;
	UINT8 m_invertmask;
	UINT8 m_bitmap_type;
	tilemap_t *m_fg_tilemap;
	UINT8 m_lastflip;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/popeye.c -----------*/

WRITE8_HANDLER( popeye_videoram_w );
WRITE8_HANDLER( popeye_colorram_w );
WRITE8_HANDLER( popeye_bitmap_w );
WRITE8_HANDLER( skyskipr_bitmap_w );

PALETTE_INIT( popeye );
PALETTE_INIT( popeyebl );
VIDEO_START( skyskipr );
VIDEO_START( popeye );
SCREEN_UPDATE( popeye );
