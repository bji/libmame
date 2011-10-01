/*************************************************************************

    Metal Clash

*************************************************************************/

class metlclsh_state : public driver_device
{
public:
	metlclsh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_bgram;
	UINT8 *        m_fgram;
	UINT8 *        m_scrollx;
	UINT8 *        m_otherram;
//      UINT8 *        m_paletteram;    // currently this uses generic palette handling
//      UINT8 *        m_paletteram2;    // currently this uses generic palette handling
	UINT8 *        m_spriteram;
	size_t         m_spriteram_size;

	/* video-related */
	tilemap_t      *m_bg_tilemap;
	tilemap_t      *m_fg_tilemap;
	UINT8          m_write_mask;
	UINT8          m_gfxbank;

	/* devices */
	device_t *m_maincpu;
	device_t *m_subcpu;
};


/*----------- defined in video/metlclsh.c -----------*/

WRITE8_HANDLER( metlclsh_bgram_w );
WRITE8_HANDLER( metlclsh_fgram_w );
WRITE8_HANDLER( metlclsh_gfxbank_w );
WRITE8_HANDLER( metlclsh_rambank_w );

VIDEO_START( metlclsh );
SCREEN_UPDATE( metlclsh );
