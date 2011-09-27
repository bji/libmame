/*************************************************************************

    Atari Cloak & Dagger hardware

*************************************************************************/

class cloak_state : public driver_device
{
public:
	cloak_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_videoram;
	UINT8 *m_spriteram;
	int m_nvram_enabled;
	UINT8 m_bitmap_videoram_selected;
	UINT8 m_bitmap_videoram_address_x;
	UINT8 m_bitmap_videoram_address_y;
	UINT8 *m_bitmap_videoram1;
	UINT8 *m_bitmap_videoram2;
	UINT8 *m_current_bitmap_videoram_accessed;
	UINT8 *m_current_bitmap_videoram_displayed;
	UINT16 *m_palette_ram;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/cloak.c -----------*/

WRITE8_HANDLER( cloak_videoram_w );
WRITE8_HANDLER( cloak_flipscreen_w );

WRITE8_HANDLER( cloak_paletteram_w );
READ8_HANDLER( graph_processor_r );
WRITE8_HANDLER( graph_processor_w );
WRITE8_HANDLER( cloak_clearbmp_w );

VIDEO_START( cloak );
SCREEN_UPDATE( cloak );
