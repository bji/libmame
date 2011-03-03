class gatron_state : public driver_device
{
public:
	gatron_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/gatron.c -----------*/

WRITE8_HANDLER( gat_videoram_w );
PALETTE_INIT( gat );
VIDEO_START( gat );
SCREEN_UPDATE( gat );

