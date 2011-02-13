/*************************************************************************

    Street Fighter

*************************************************************************/

class sf_state : public driver_device
{
public:
	sf_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    videoram;
	UINT16 *    objectram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling
	size_t      videoram_size;

	/* video-related */
	tilemap_t     *bg_tilemap, *fg_tilemap, *tx_tilemap;
	int         sf_active;
	UINT16      bgscroll, fgscroll;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
};


/*----------- defined in video/sf.c -----------*/

WRITE16_HANDLER( sf_bg_scroll_w );
WRITE16_HANDLER( sf_fg_scroll_w );
WRITE16_HANDLER( sf_videoram_w );
WRITE16_HANDLER( sf_gfxctrl_w );

VIDEO_START( sf );
VIDEO_UPDATE( sf );
