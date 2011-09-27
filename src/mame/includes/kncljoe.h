/*************************************************************************

    Knuckle Joe

*************************************************************************/

class kncljoe_state : public driver_device
{
public:
	kncljoe_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_scrollregs;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_tile_bank;
	int			m_sprite_bank;
	int        m_flipscreen;

	/* misc */
	UINT8      m_port1;
	UINT8      m_port2;

	/* devices */
	device_t *m_soundcpu;
};



/*----------- defined in video/kncljoe.c -----------*/

WRITE8_HANDLER(kncljoe_videoram_w);
WRITE8_HANDLER(kncljoe_control_w);
WRITE8_HANDLER(kncljoe_scroll_w);

PALETTE_INIT( kncljoe );
VIDEO_START( kncljoe );
SCREEN_UPDATE( kncljoe );
