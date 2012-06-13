#define MAX_STARS			250

struct star {
	float x,y;
	int col,set;
};


class gaplus_state : public driver_device
{
public:
	gaplus_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *customio_3;
	UINT8 *videoram;
	UINT8 *spriteram;
	tilemap_t *bg_tilemap;
	UINT8 starfield_control[4];
	int total_stars;
	struct star stars[MAX_STARS];
};


/*----------- defined in machine/gaplus.c -----------*/

WRITE8_HANDLER( gaplus_customio_3_w );
READ8_HANDLER( gaplus_customio_3_r );


/*----------- defined in video/gaplus.c -----------*/

READ8_HANDLER( gaplus_videoram_r );
WRITE8_HANDLER( gaplus_videoram_w );
WRITE8_HANDLER( gaplus_starfield_control_w );
VIDEO_START( gaplus );
PALETTE_INIT( gaplus );
SCREEN_UPDATE( gaplus );
SCREEN_EOF( gaplus );	/* update starfields */
