
class paradise_state : public driver_device
{
public:
	paradise_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_vram_0;
	UINT8 *  m_vram_1;
	UINT8 *  m_vram_2;
	UINT8 *  m_videoram;
	UINT8 *  m_paletteram;
	UINT8 *  m_spriteram;
	size_t   m_spriteram_size;

	/* video-related */
	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;
	bitmap_t *m_tmpbitmap;
	UINT8 m_palbank;
	UINT8 m_priority;
	UINT8 m_pixbank;
	int m_sprite_inc;

	int irq_count;
};

/*----------- defined in video/paradise.c -----------*/

WRITE8_HANDLER( paradise_vram_0_w );
WRITE8_HANDLER( paradise_vram_1_w );
WRITE8_HANDLER( paradise_vram_2_w );

WRITE8_HANDLER( paradise_flipscreen_w );
WRITE8_HANDLER( tgtball_flipscreen_w );
WRITE8_HANDLER( paradise_palette_w );
WRITE8_HANDLER( paradise_pixmap_w );

WRITE8_HANDLER( paradise_priority_w );
WRITE8_HANDLER( paradise_palbank_w );

VIDEO_START( paradise );

SCREEN_UPDATE( paradise );
SCREEN_UPDATE( torus );
SCREEN_UPDATE( madball );
