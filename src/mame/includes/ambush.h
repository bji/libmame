/*************************************************************************

    Ambush

*************************************************************************/

typedef struct _ambush_state ambush_state;
struct _ambush_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	UINT8 *    colorram;
	UINT8 *    scrollram;
	UINT8 *    colorbank;

	size_t     videoram_size;
	size_t     spriteram_size;
};


/*----------- defined in video/ambush.c -----------*/

PALETTE_INIT( ambush );
VIDEO_UPDATE( ambush );
