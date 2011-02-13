/*************************************************************************

    Parodius

*************************************************************************/

class parodius_state : public driver_device
{
public:
	parodius_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    ram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;
	int        layerpri[3];

	/* misc */
	int        videobank;
	//int        nmi_enabled;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k053260;
	running_device *k052109;
	running_device *k053245;
	running_device *k053251;
};

/*----------- defined in video/parodius.c -----------*/

extern void parodius_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void parodius_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);

VIDEO_UPDATE( parodius );
