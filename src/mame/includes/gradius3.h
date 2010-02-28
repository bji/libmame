/*************************************************************************

    Gradius 3

*************************************************************************/

typedef struct _gradius3_state gradius3_state;
struct _gradius3_state
{
	/* memory pointers */
	UINT16 *    gfxram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         layer_colorbase[3], sprite_colorbase;

	/* misc */
	int         priority;
	int         irqAen, irqBmask;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *subcpu;
	running_device *k007232;
	running_device *k052109;
	running_device *k051960;
};

/*----------- defined in video/gradius3.c -----------*/

extern void gradius3_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);
extern void gradius3_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);

READ16_HANDLER( gradius3_gfxrom_r );
WRITE16_HANDLER( gradius3_gfxram_w );

VIDEO_START( gradius3 );
VIDEO_UPDATE( gradius3 );
