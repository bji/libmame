/*************************************************************************

    Son Son

*************************************************************************/

class sonson_state : public driver_device
{
public:
	sonson_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	size_t     videoram_size;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;

	/* misc */
	int        last_irq;

	/* devices */
	device_t *audiocpu;
};


/*----------- defined in video/sonson.c -----------*/

WRITE8_HANDLER( sonson_videoram_w );
WRITE8_HANDLER( sonson_colorram_w );
WRITE8_HANDLER( sonson_scrollx_w );
WRITE8_HANDLER( sonson_flipscreen_w );

PALETTE_INIT( sonson );
VIDEO_START( sonson );
SCREEN_UPDATE( sonson );
