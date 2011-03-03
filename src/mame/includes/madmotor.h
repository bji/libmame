/*************************************************************************

    Mad Motor

*************************************************************************/

class madmotor_state : public driver_device
{
public:
	madmotor_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *        pf1_rowscroll;
	UINT16 *        pf1_data;
	UINT16 *        pf2_data;
	UINT16 *        pf3_data;
	UINT16 *        pf1_control;
	UINT16 *        pf2_control;
	UINT16 *        pf3_control;
	UINT16 *        spriteram;
//  UINT16 *        paletteram;     // this currently uses generic palette handlers
	size_t          spriteram_size;

	/* video-related */
	tilemap_t       *pf1_tilemap, *pf2_tilemap, *pf3_tilemap, *pf3a_tilemap;
	int             flipscreen;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
};


/*----------- defined in video/madmotor.c -----------*/

WRITE16_HANDLER( madmotor_pf1_data_w );
WRITE16_HANDLER( madmotor_pf2_data_w );
WRITE16_HANDLER( madmotor_pf3_data_w );

VIDEO_START( madmotor );
SCREEN_UPDATE( madmotor );
