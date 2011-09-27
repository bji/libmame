/*************************************************************************

    Air Buster

*************************************************************************/

class airbustr_state : public driver_device
{
public:
	airbustr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_videoram2;
	UINT8 *    m_colorram;
	UINT8 *    m_colorram2;
	UINT8 *    m_paletteram;
	UINT8 *    m_devram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	bitmap_t   *m_sprites_bitmap;
	int        m_bg_scrollx;
	int        m_bg_scrolly;
	int        m_fg_scrollx;
	int        m_fg_scrolly;
	int        m_highbits;

	/* misc */
	int        m_soundlatch_status;
	int        m_soundlatch2_status;
	int        m_master_addr;
	int        m_slave_addr;

	/* devices */
	device_t *m_master;
	device_t *m_slave;
	device_t *m_audiocpu;
	device_t *m_pandora;
};


/*----------- defined in video/airbustr.c -----------*/

WRITE8_HANDLER( airbustr_videoram_w );
WRITE8_HANDLER( airbustr_colorram_w );
WRITE8_HANDLER( airbustr_videoram2_w );
WRITE8_HANDLER( airbustr_colorram2_w );
WRITE8_HANDLER( airbustr_scrollregs_w );

VIDEO_START( airbustr );
SCREEN_UPDATE( airbustr );
SCREEN_EOF( airbustr );
