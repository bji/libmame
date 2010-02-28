/***************************************************************************

    carjmbre

***************************************************************************/

typedef struct _carjmbre_state carjmbre_state;
struct _carjmbre_state
{
	/* memory pointers */
	UINT8 * videoram;
	UINT8 * spriteram;
	size_t  spriteram_size;

	/* video-related */
	tilemap_t *cj_tilemap;
	UINT8   flipscreen;
	UINT16  bgcolor;
};



/*----------- defined in video/carjmbre.c -----------*/

WRITE8_HANDLER( carjmbre_flipscreen_w );
WRITE8_HANDLER( carjmbre_bgcolor_w );
WRITE8_HANDLER( carjmbre_videoram_w );

PALETTE_INIT( carjmbre );
VIDEO_START( carjmbre );
VIDEO_UPDATE( carjmbre );


