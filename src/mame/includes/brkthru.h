/***************************************************************************

    Break Thru

***************************************************************************/

class brkthru_state : public driver_device
{
public:
	brkthru_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 * m_videoram;
	UINT8 * m_spriteram;
	UINT8 * m_fg_videoram;
	size_t  m_videoram_size;
	size_t  m_spriteram_size;
	size_t  m_fg_videoram_size;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int     m_bgscroll;
	int     m_bgbasecolor;
	int     m_flipscreen;
	//UINT8 *m_brkthru_nmi_enable; /* needs to be tracked down */

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};


/*----------- defined in video/brkthru.c -----------*/

WRITE8_HANDLER( brkthru_1800_w );
WRITE8_HANDLER( brkthru_bgram_w );
WRITE8_HANDLER( brkthru_fgram_w );
VIDEO_START( brkthru );
PALETTE_INIT( brkthru );
SCREEN_UPDATE( brkthru );
