/*************************************************************************

    S.P.Y.

*************************************************************************/

typedef struct _spy_state spy_state;
struct _spy_state
{
	/* memory pointers */
	UINT8 *    ram;
	UINT8 *    pmcram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* misc */
	int        rambank, pmcbank;
	int        video_enable;
	int        old_3f90;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k007232_1;
	running_device *k007232_2;
	running_device *k052109;
	running_device *k051960;
};


/*----------- defined in video/spy.c -----------*/

extern void spy_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void spy_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);

VIDEO_START( spy );
VIDEO_UPDATE( spy );
