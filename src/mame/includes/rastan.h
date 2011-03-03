/*************************************************************************

    Rastan

*************************************************************************/

class rastan_state : public driver_device
{
public:
	rastan_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
//  UINT16 *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	UINT16      sprite_ctrl;
	UINT16      sprites_flipscreen;

	/* misc */
	int         adpcm_pos;
	int         adpcm_data;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *pc090oj;
	device_t *pc080sn;
};


/*----------- defined in video/rastan.c -----------*/

WRITE16_HANDLER( rastan_spritectrl_w );

SCREEN_UPDATE( rastan );
