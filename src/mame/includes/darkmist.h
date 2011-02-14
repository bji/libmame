class darkmist_state : public driver_device
{
public:
	darkmist_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *workram;
	int hw;
	UINT8 *scroll;
	UINT8 *spritebank;
	tilemap_t *bgtilemap;
	tilemap_t *fgtilemap;
	tilemap_t *txtilemap;
};


/*----------- defined in video/darkmist.c -----------*/

VIDEO_START( darkmist );
VIDEO_UPDATE( darkmist );
PALETTE_INIT( darkmist );

