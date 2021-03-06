/*************************************************************************

    Cosmic Guerilla & other Universal boards (in cosmic.c)

*************************************************************************/

#define COSMICG_MASTER_CLOCK     XTAL_9_828MHz
#define Z80_MASTER_CLOCK         XTAL_10_816MHz


class cosmic_state : public driver_device
{
public:
	cosmic_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        videoram;
	UINT8 *        spriteram;
	size_t         videoram_size;
	size_t         spriteram_size;

	/* video-related */
	pen_t          (*map_color)(running_machine *machine, UINT8 x, UINT8 y);
	int            color_registers[3];
	int            background_enable;
	int            magspot_pen_mask;

	/* sound-related */
	int            sound_enabled;
	int            march_select, gun_die_select, dive_bomb_b_select;

	/* misc */
	UINT32         pixel_clock;

	/* devices */
	device_t *samples;
	device_t *dac;
};


/*----------- defined in video/cosmic.c -----------*/

WRITE8_HANDLER( cosmic_color_register_w );
WRITE8_HANDLER( cosmic_background_enable_w );

PALETTE_INIT( panic );
PALETTE_INIT( cosmica );
PALETTE_INIT( cosmicg );
PALETTE_INIT( magspot );
PALETTE_INIT( nomnlnd );

SCREEN_UPDATE( panic );
SCREEN_UPDATE( magspot );
SCREEN_UPDATE( devzone );
SCREEN_UPDATE( cosmica );
SCREEN_UPDATE( cosmicg );
SCREEN_UPDATE( nomnlnd );
