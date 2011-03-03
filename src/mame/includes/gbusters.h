/*************************************************************************

    Gangbusters

*************************************************************************/

class gbusters_state : public driver_device
{
public:
	gbusters_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    ram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* misc */
	int        palette_selected;
	int        priority;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k007232;
	device_t *k052109;
	device_t *k051960;
};

/*----------- defined in video/gbusters.c -----------*/

extern void gbusters_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags, int *priority);
extern void gbusters_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);

VIDEO_START( gbusters );
SCREEN_UPDATE( gbusters );
