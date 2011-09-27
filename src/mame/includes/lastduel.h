/*************************************************************************

    Last Duel

*************************************************************************/

class lastduel_state : public driver_device
{
public:
	lastduel_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    m_vram;
	UINT16 *    m_scroll1;
	UINT16 *    m_scroll2;
//  UINT16 *    m_spriteram;  // this currently uses generic buffered spriteram
	UINT16 *    m_paletteram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_tx_tilemap;
	UINT16      m_scroll[8];
	int         m_sprite_flipy_mask;
	int         m_sprite_pri_mask;
	int         m_tilemap_priority;

	/* devices */
	device_t *m_audiocpu;
};

/*----------- defined in video/lastduel.c -----------*/

WRITE16_HANDLER( lastduel_vram_w );
WRITE16_HANDLER( lastduel_flip_w );
WRITE16_HANDLER( lastduel_scroll1_w );
WRITE16_HANDLER( lastduel_scroll2_w );
WRITE16_HANDLER( madgear_scroll1_w );
WRITE16_HANDLER( madgear_scroll2_w );
WRITE16_HANDLER( lastduel_scroll_w );
WRITE16_HANDLER( lastduel_palette_word_w );

VIDEO_START( lastduel );
VIDEO_START( madgear );
SCREEN_UPDATE( lastduel );
SCREEN_UPDATE( madgear );
SCREEN_EOF( lastduel );
