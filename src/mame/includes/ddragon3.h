/*************************************************************************

    Double Dragon 3 & The Combatribes

*************************************************************************/


class ddragon3_state : public driver_device
{
public:
	ddragon3_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *        m_bg_videoram;
	UINT16 *        m_fg_videoram;
	UINT16 *        m_spriteram;
//  UINT16 *        m_paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t         *m_fg_tilemap;
	tilemap_t         *m_bg_tilemap;
	UINT16          m_vreg;
	UINT16          m_bg_scrollx;
	UINT16          m_bg_scrolly;
	UINT16          m_fg_scrollx;
	UINT16          m_fg_scrolly;
	UINT16          m_bg_tilebase;

	/* misc */
	UINT16          m_io_reg[8];

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};


/*----------- defined in video/ddragon3.c -----------*/

extern WRITE16_HANDLER( ddragon3_bg_videoram_w );
extern WRITE16_HANDLER( ddragon3_fg_videoram_w );
extern WRITE16_HANDLER( ddragon3_scroll_w );
extern READ16_HANDLER( ddragon3_scroll_r );

extern VIDEO_START( ddragon3 );
extern SCREEN_UPDATE( ddragon3 );
extern SCREEN_UPDATE( ctribe );
