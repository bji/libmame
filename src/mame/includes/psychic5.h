class psychic5_state : public driver_device
{
public:
	psychic5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_bank_latch;
	UINT8 m_ps5_vram_page;
	UINT8 m_bg_clip_mode;
	UINT8 m_title_screen;
	UINT8 m_bg_status;
	UINT8 *m_ps5_pagedram[2];
	UINT8 *m_bg_videoram;
	UINT8 *m_ps5_dummy_bg_ram;
	UINT8 *m_ps5_io_ram;
	UINT8 *m_ps5_palette_ram;
	UINT8 *m_fg_videoram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int m_bg_palette_ram_base;
	int m_bg_palette_base;
	UINT16 m_palette_intensity;
	UINT8 m_bombsa_unknown;
	int m_sx1;
	int m_sy1;
	int m_sy2;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/psychic5.c -----------*/

WRITE8_HANDLER( psychic5_paged_ram_w );
WRITE8_HANDLER( psychic5_vram_page_select_w );
WRITE8_HANDLER( psychic5_title_screen_w );

READ8_HANDLER( psychic5_paged_ram_r );
READ8_HANDLER( psychic5_vram_page_select_r );

VIDEO_START( psychic5 );
VIDEO_RESET( psychic5 );
SCREEN_UPDATE_RGB32( psychic5 );

WRITE8_HANDLER( bombsa_paged_ram_w );
WRITE8_HANDLER( bombsa_unknown_w );

VIDEO_START( bombsa );
VIDEO_RESET( bombsa );
SCREEN_UPDATE_RGB32( bombsa );
