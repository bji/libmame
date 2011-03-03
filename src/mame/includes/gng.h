/*************************************************************************

    Ghosts'n Goblins

*************************************************************************/

class gng_state : public driver_device
{
public:
	gng_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    bgvideoram;
	UINT8 *    fgvideoram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling
//  UINT8 *    paletteram2; // currently this uses generic palette handling
//  UINT8 *    spriteram;   // currently this uses generic buffered spriteram

	/* video-related */
	tilemap_t    *bg_tilemap, *fg_tilemap;
	UINT8      scrollx[2];
	UINT8      scrolly[2];
};


/*----------- defined in video/gng.c -----------*/

WRITE8_HANDLER( gng_fgvideoram_w );
WRITE8_HANDLER( gng_bgvideoram_w );
WRITE8_HANDLER( gng_bgscrollx_w );
WRITE8_HANDLER( gng_bgscrolly_w );
WRITE8_HANDLER( gng_flipscreen_w );

VIDEO_START( gng );
SCREEN_UPDATE( gng );
SCREEN_EOF( gng );
