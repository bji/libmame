class stlforce_state : public driver_device
{
public:
	stlforce_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	tilemap_t *bg_tilemap;
	tilemap_t *mlow_tilemap;
	tilemap_t *mhigh_tilemap;
	tilemap_t *tx_tilemap;

	UINT16 *bg_videoram;
	UINT16 *mlow_videoram;
	UINT16 *mhigh_videoram;
	UINT16 *tx_videoram;
	UINT16 *bg_scrollram;
	UINT16 *mlow_scrollram;
	UINT16 *mhigh_scrollram;
	UINT16 *vidattrram;

	UINT16 *spriteram;

	int sprxoffs;
};


/*----------- defined in video/stlforce.c -----------*/

VIDEO_START( stlforce );
SCREEN_UPDATE( stlforce );
WRITE16_HANDLER( stlforce_tx_videoram_w );
WRITE16_HANDLER( stlforce_mhigh_videoram_w );
WRITE16_HANDLER( stlforce_mlow_videoram_w );
WRITE16_HANDLER( stlforce_bg_videoram_w );
