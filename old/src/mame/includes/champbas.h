/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/


#define CPUTAG_MCU "mcu"


class champbas_state : public driver_device
{
public:
	champbas_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        bg_videoram;
	UINT8 *        spriteram;
	UINT8 *        spriteram_2;
	size_t         spriteram_size;

	/* video-related */
	tilemap_t        *bg_tilemap;
	UINT8          gfx_bank;
	UINT8          palette_bank;

	/* misc */
	int            watchdog_count;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *mcu;
};


/*----------- defined in video/champbas.c -----------*/

WRITE8_HANDLER( champbas_bg_videoram_w );
WRITE8_HANDLER( champbas_gfxbank_w );
WRITE8_HANDLER( champbas_palette_bank_w );
WRITE8_HANDLER( champbas_flipscreen_w );

PALETTE_INIT( champbas );
PALETTE_INIT( exctsccr );
VIDEO_START( champbas );
VIDEO_START( exctsccr );
SCREEN_UPDATE( champbas );
SCREEN_UPDATE( exctsccr );


