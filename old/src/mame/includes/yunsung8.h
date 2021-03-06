/*************************************************************************

    Yun Sung 8 Bit Games

*************************************************************************/

class yunsung8_state : public driver_device
{
public:
	yunsung8_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* video-related */
	tilemap_t     *tilemap_0, *tilemap_1;
	UINT8       *videoram_0, *videoram_1;
	int         layers_ctrl;
	int         videobank;

	/* misc */
	int         adpcm;
	int         toggle;

	/* devices */
	device_t *audiocpu;

	/* memory */
	UINT8      videoram[0x4000];
};


/*----------- defined in video/yunsung8.c -----------*/

WRITE8_HANDLER( yunsung8_videobank_w );
READ8_HANDLER ( yunsung8_videoram_r );
WRITE8_HANDLER( yunsung8_videoram_w );
WRITE8_HANDLER( yunsung8_flipscreen_w );

VIDEO_START( yunsung8 );
SCREEN_UPDATE( yunsung8 );
