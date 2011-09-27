/*************************************************************************

    Son Son

*************************************************************************/

class sonson_state : public driver_device
{
public:
	sonson_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	size_t     m_videoram_size;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int        m_last_irq;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/sonson.c -----------*/

WRITE8_HANDLER( sonson_videoram_w );
WRITE8_HANDLER( sonson_colorram_w );
WRITE8_HANDLER( sonson_scrollx_w );
WRITE8_HANDLER( sonson_flipscreen_w );

PALETTE_INIT( sonson );
VIDEO_START( sonson );
SCREEN_UPDATE( sonson );
