class wwfsstar_state : public driver_device
{
public:
	wwfsstar_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int vblank;
	int scrollx;
	int scrolly;
	UINT16 *spriteram;
	UINT16 *fg0_videoram;
	UINT16 *bg0_videoram;
	tilemap_t *fg0_tilemap;
	tilemap_t *bg0_tilemap;
};


/*----------- defined in video/wwfsstar.c -----------*/

VIDEO_START( wwfsstar );
SCREEN_UPDATE( wwfsstar );
WRITE16_HANDLER( wwfsstar_fg0_videoram_w );
WRITE16_HANDLER( wwfsstar_bg0_videoram_w );
