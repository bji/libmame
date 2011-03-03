class liberate_state : public driver_device
{
public:
	liberate_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *paletteram;
	UINT8 *spriteram;
	UINT8 *scratchram;
	UINT8 *charram;	/* prosoccr */
	UINT8 *bg_vram; /* prosport */

	UINT8 io_ram[16];

	int bank;
	int latch;
	UINT8 gfx_rom_readback;
	int background_color;
	int background_disable;

	tilemap_t *back_tilemap;
	tilemap_t *fix_tilemap;

	device_t *maincpu;
	device_t *audiocpu;
};


/*----------- defined in video/liberate.c -----------*/

PALETTE_INIT( liberate );
SCREEN_UPDATE( prosoccr );
SCREEN_UPDATE( prosport );
SCREEN_UPDATE( liberate );
SCREEN_UPDATE( boomrang );
VIDEO_START( prosoccr );
VIDEO_START( prosport );
VIDEO_START( boomrang );
VIDEO_START( liberate );

WRITE8_HANDLER( deco16_io_w );
WRITE8_HANDLER( prosoccr_io_w );
WRITE8_HANDLER( prosport_io_w );
WRITE8_HANDLER( prosport_paletteram_w );
WRITE8_HANDLER( prosport_bg_vram_w );
WRITE8_HANDLER( liberate_videoram_w );
WRITE8_HANDLER( liberate_colorram_w );

