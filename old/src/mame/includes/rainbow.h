/*************************************************************************

    Rainbow Islands

*************************************************************************/

class rainbow_state : public driver_device
{
public:
	rainbow_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    spriteram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	UINT16      sprite_ctrl;
	UINT16      sprites_flipscreen;

	/* misc */
	UINT8       jumping_latch;

	/* c-chip */
	UINT8       *CRAM[8];
	int         extra_version;
	UINT8       current_bank;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *pc080sn;
	device_t *pc090oj;
};


/*----------- defined in machine/rainbow.c -----------*/

void rainbow_cchip_init(running_machine *machine, int version);
READ16_HANDLER( rainbow_cchip_ctrl_r );
READ16_HANDLER( rainbow_cchip_ram_r );
WRITE16_HANDLER( rainbow_cchip_ctrl_w );
WRITE16_HANDLER( rainbow_cchip_bank_w );
WRITE16_HANDLER( rainbow_cchip_ram_w );


/*----------- defined in video/rainbow.c -----------*/

SCREEN_UPDATE( rainbow );
VIDEO_START( jumping );
SCREEN_UPDATE( jumping );

WRITE16_HANDLER( jumping_spritectrl_w );
WRITE16_HANDLER( rainbow_spritectrl_w );
