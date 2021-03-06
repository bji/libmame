/*************************************************************************

    Pandora's Palace

*************************************************************************/

class pandoras_state : public driver_device
{
public:
	pandoras_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;

	/* video-related */
	tilemap_t     *layer0;
	int         flipscreen;

	int irq_enable_a, irq_enable_b;
	int firq_old_data_a, firq_old_data_b;
	int i8039_status;

	/* devices */
	cpu_device *maincpu;
	cpu_device *subcpu;
	cpu_device *audiocpu;
	cpu_device *mcu;
};


/*----------- defined in video/pandoras.c -----------*/

PALETTE_INIT( pandoras );

WRITE8_HANDLER( pandoras_vram_w );
WRITE8_HANDLER( pandoras_cram_w );
WRITE8_HANDLER( pandoras_flipscreen_w );
WRITE8_HANDLER( pandoras_scrolly_w );

VIDEO_START( pandoras );
SCREEN_UPDATE( pandoras );

