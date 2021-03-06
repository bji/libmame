/*************************************************************************

    The Main Event / Devastators

*************************************************************************/

class mainevt_state : public driver_device
{
public:
	mainevt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* misc */
	int        nmi_enable;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *upd;
	device_t *k007232;
	device_t *k052109;
	device_t *k051960;
};

/*----------- defined in video/mainevt.c -----------*/

extern void mainevt_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void dv_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void mainevt_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);
extern void dv_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow);

VIDEO_START( mainevt );
VIDEO_START( dv );

SCREEN_UPDATE( mainevt );
SCREEN_UPDATE( dv );
