/*************************************************************************

    Cheeky Mouse

*************************************************************************/


class cheekyms_state : public driver_device
{
public:
	cheekyms_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        videoram;
	UINT8 *        spriteram;
	UINT8 *        port_80;

	/* video-related */
	tilemap_t        *cm_tilemap;
	bitmap_t       *bitmap_buffer;

	/* devices */
	device_t *maincpu;
	device_t *dac;
};


/*----------- defined in video/cheekyms.c -----------*/

PALETTE_INIT( cheekyms );
VIDEO_START( cheekyms );
SCREEN_UPDATE( cheekyms );
WRITE8_HANDLER( cheekyms_port_40_w );
WRITE8_HANDLER( cheekyms_port_80_w );
