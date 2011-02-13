/*************************************************************************

    Dragonball Z

*************************************************************************/

class dbz_state : public driver_device
{
public:
	dbz_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *      bg1_videoram;
	UINT16 *      bg2_videoram;
//  UINT16 *      paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *bg1_tilemap, *bg2_tilemap;
	int          layer_colorbase[6], layerpri[5], sprite_colorbase;

	/* misc */
	int           control;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k053246;
	device_t *k053251;
	device_t *k056832;
	device_t *k053936_1;
	device_t *k053936_2;
};


/*----------- defined in video/dbz.c -----------*/

extern void dbz_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
extern void dbz_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

WRITE16_HANDLER(dbz_bg1_videoram_w);
WRITE16_HANDLER(dbz_bg2_videoram_w);

VIDEO_START(dbz);
VIDEO_UPDATE(dbz);
