/*************************************************************************

    Taito O system

*************************************************************************/

typedef struct _taitoo_state taitoo_state;
struct _taitoo_state
{
	/* memory pointers */
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* devices */
	running_device *maincpu;
	running_device *tc0080vco;
};

/*----------- defined in video/taito_o.c -----------*/

VIDEO_UPDATE( parentj );
