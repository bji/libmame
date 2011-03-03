/*************************************************************************

    Mouser

*************************************************************************/

class mouser_state : public driver_device
{
public:
	mouser_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* misc */
	UINT8      sound_byte;
	UINT8      nmi_enable;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
};

/*----------- defined in video/mouser.c -----------*/

WRITE8_HANDLER( mouser_flip_screen_x_w );
WRITE8_HANDLER( mouser_flip_screen_y_w );

PALETTE_INIT( mouser );
SCREEN_UPDATE( mouser );
