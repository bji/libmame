/*************************************************************************

    Yun Sung 16 Bit Games

*************************************************************************/

class yunsun16_state : public driver_device
{
public:
	yunsun16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    m_vram_0;
	UINT16 *    m_vram_1;
	UINT16 *    m_scrollram_0;
	UINT16 *    m_scrollram_1;
	UINT16 *    m_priorityram;
//  UINT16 *    m_paletteram; // currently this uses generic palette handling
	UINT16 *    m_spriteram;
	size_t      m_spriteram_size;

	/* other video-related elements */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;
	int         m_sprites_scrolldx;
	int         m_sprites_scrolldy;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/yunsun16.c -----------*/

WRITE16_HANDLER( yunsun16_vram_0_w );
WRITE16_HANDLER( yunsun16_vram_1_w );

VIDEO_START( yunsun16 );
SCREEN_UPDATE( yunsun16 );
