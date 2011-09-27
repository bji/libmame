class tecmo16_state : public driver_device
{
public:
	tecmo16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_videoram;
	UINT16 *m_colorram;
	UINT16 *m_videoram2;
	UINT16 *m_colorram2;
	UINT16 *m_charram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	bitmap_t *m_sprite_bitmap;
	bitmap_t *m_tile_bitmap_bg;
	bitmap_t *m_tile_bitmap_fg;
	int m_flipscreen;
	int m_game_is_riot;
	UINT16 m_scroll_x_w;
	UINT16 m_scroll_y_w;
	UINT16 m_scroll2_x_w;
	UINT16 m_scroll2_y_w;
	UINT16 m_scroll_char_x_w;
	UINT16 m_scroll_char_y_w;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/tecmo16.c -----------*/

WRITE16_HANDLER( tecmo16_videoram_w );
WRITE16_HANDLER( tecmo16_colorram_w );
WRITE16_HANDLER( tecmo16_videoram2_w );
WRITE16_HANDLER( tecmo16_colorram2_w );
WRITE16_HANDLER( tecmo16_charram_w );
WRITE16_HANDLER( tecmo16_flipscreen_w );

WRITE16_HANDLER( tecmo16_scroll_x_w );
WRITE16_HANDLER( tecmo16_scroll_y_w );
WRITE16_HANDLER( tecmo16_scroll2_x_w );
WRITE16_HANDLER( tecmo16_scroll2_y_w );
WRITE16_HANDLER( tecmo16_scroll_char_x_w );
WRITE16_HANDLER( tecmo16_scroll_char_y_w );

VIDEO_START( fstarfrc );
VIDEO_START( ginkun );
VIDEO_START( riot );
SCREEN_UPDATE( tecmo16 );
