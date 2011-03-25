/***************************************************************************

    Kyugo hardware games

***************************************************************************/

class kyugo_state : public driver_device
{
public:
	kyugo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *     fgvideoram;
	UINT8 *     bgvideoram;
	UINT8 *     bgattribram;
	UINT8 *     spriteram_1;
	UINT8 *     spriteram_2;
	UINT8 *     shared_ram;

	/* video-related */
	tilemap_t     *bg_tilemap, *fg_tilemap;
	UINT8       scroll_x_lo, scroll_x_hi, scroll_y;
	int         bgpalbank, fgcolor;
	int         flipscreen;
	const UINT8 *color_codes;

	/* devices */
	device_t *maincpu;
	device_t *subcpu;
};


/*----------- defined in video/kyugo.c -----------*/

READ8_HANDLER( kyugo_spriteram_2_r );

WRITE8_HANDLER( kyugo_fgvideoram_w );
WRITE8_HANDLER( kyugo_bgvideoram_w );
WRITE8_HANDLER( kyugo_bgattribram_w );
WRITE8_HANDLER( kyugo_scroll_x_lo_w );
WRITE8_HANDLER( kyugo_gfxctrl_w );
WRITE8_HANDLER( kyugo_scroll_y_w );
WRITE8_HANDLER( kyugo_flipscreen_w );

VIDEO_START( kyugo );
SCREEN_UPDATE( kyugo );
