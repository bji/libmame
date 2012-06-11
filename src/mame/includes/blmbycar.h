/***************************************************************************

    Blomby Car

***************************************************************************/

class blmbycar_state : public driver_device
{
public:
	blmbycar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_vram_0;
	UINT16 *    m_scroll_0;
	UINT16 *    m_vram_1;
	UINT16 *    m_scroll_1;
	UINT16 *    m_spriteram;
	UINT16 *    m_paletteram;
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;

	/* input-related */
	UINT8       m_pot_wheel;	// blmbycar
	int         m_old_val;	// blmbycar
	int         m_retvalue;	// waterball
};


/*----------- defined in video/blmbycar.c -----------*/

WRITE16_HANDLER( blmbycar_palette_w );

WRITE16_HANDLER( blmbycar_vram_0_w );
WRITE16_HANDLER( blmbycar_vram_1_w );

VIDEO_START( blmbycar );
SCREEN_UPDATE_IND16( blmbycar );
