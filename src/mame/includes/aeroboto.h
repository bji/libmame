/***************************************************************************

    Aeroboto

***************************************************************************/

class aeroboto_state : public driver_device
{
public:
	aeroboto_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 * m_mainram;
	UINT8 * m_spriteram;
	UINT8 * m_videoram;
	UINT8 * m_hscroll;
	UINT8 * m_vscroll;
	UINT8 * m_tilecolor;
	UINT8 * m_starx;
	UINT8 * m_stary;
	UINT8 * m_bgcolor;
	size_t  m_spriteram_size;

	/* stars layout */
	UINT8 * m_stars_rom;
	int     m_stars_length;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int     m_charbank;
	int     m_starsoff;
	int     m_sx;
	int     m_sy;
	UINT8   m_ox;
	UINT8   m_oy;

	/* misc */
	int m_count;
	int m_disable_irq;
};


/*----------- defined in video/aeroboto.c -----------*/

VIDEO_START( aeroboto );
SCREEN_UPDATE( aeroboto );

READ8_HANDLER( aeroboto_in0_r );
WRITE8_HANDLER( aeroboto_3000_w );
WRITE8_HANDLER( aeroboto_videoram_w );
WRITE8_HANDLER( aeroboto_tilecolor_w );
