/*************************************************************************

    Wild West C.O.W.boys of Moo Mesa / Bucky O'Hare

*************************************************************************/

typedef struct _moo_state moo_state;
struct _moo_state
{
	/* memory pointers */
	UINT16 *    workram;
	UINT16 *    spriteram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         sprite_colorbase;
	int         layer_colorbase[4], layerpri[3];
	int         alpha_enabled;

	/* misc */
	int         game_type;
	UINT16      protram[16];
	UINT16      cur_control2;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k054539;
	running_device *k053246;
	running_device *k053251;
	running_device *k056832;
	running_device *k054338;
};



/*----------- defined in video/moo.c -----------*/

extern void moo_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);
extern void moo_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);

VIDEO_START(moo);
VIDEO_UPDATE(moo);
