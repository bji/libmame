#define NAMCOS1_MAX_BANK 0x400

/* Bank handler definitions */
typedef struct
{
	read8_space_func bank_handler_r;
	write8_space_func bank_handler_w;
	int           bank_offset;
	UINT8 *bank_pointer;
} bankhandler;

class namcos1_state : public driver_device
{
public:
	namcos1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_dac0_value;
	int m_dac1_value;
	int m_dac0_gain;
	int m_dac1_gain;
	UINT8 *m_paletteram;
	UINT8 *m_triram;
	UINT8 *m_s1ram;
	bankhandler m_bank_element[NAMCOS1_MAX_BANK];
	bankhandler m_active_bank[16];
	int m_key_id;
	int m_key_reg;
	int m_key_rng;
	int m_key_swap4_arg;
	int m_key_swap4;
	int m_key_bottom4;
	int m_key_top4;
	unsigned int m_key_quotient;
	unsigned int m_key_reminder;
	unsigned int m_key_numerator_high_word;
	UINT8 m_key[8];
	int m_mcu_patch_data;
	int m_reset;
	int m_wdog;
	int m_chip[16];
	UINT8 *m_videoram;
	UINT8 m_cus116[0x10];
	UINT8 *m_spriteram;
	UINT8 m_playfield_control[0x20];
	tilemap_t *m_bg_tilemap[6];
	UINT8 *m_tilemap_maskdata;
	int m_copy_sprites;
	UINT8 m_drawmode_table[16];
};


/*----------- defined in drivers/namcos1.c -----------*/

void namcos1_init_DACs(running_machine &machine);


/*----------- defined in machine/namcos1.c -----------*/

WRITE8_HANDLER( namcos1_bankswitch_w );
WRITE8_HANDLER( namcos1_subcpu_bank_w );

WRITE8_HANDLER( namcos1_cpu_control_w );
WRITE8_HANDLER( namcos1_watchdog_w );
WRITE8_HANDLER( namcos1_sound_bankswitch_w );

WRITE8_HANDLER( namcos1_mcu_bankswitch_w );
WRITE8_HANDLER( namcos1_mcu_patch_w );

MACHINE_RESET( namcos1 );

DRIVER_INIT( shadowld );
DRIVER_INIT( dspirit );
DRIVER_INIT( quester );
DRIVER_INIT( blazer );
DRIVER_INIT( pacmania );
DRIVER_INIT( galaga88 );
DRIVER_INIT( ws );
DRIVER_INIT( berabohm );
DRIVER_INIT( alice );
DRIVER_INIT( bakutotu );
DRIVER_INIT( wldcourt );
DRIVER_INIT( splatter );
DRIVER_INIT( faceoff );
DRIVER_INIT( rompers );
DRIVER_INIT( blastoff );
DRIVER_INIT( ws89 );
DRIVER_INIT( dangseed );
DRIVER_INIT( ws90 );
DRIVER_INIT( pistoldm );
DRIVER_INIT( soukobdx );
DRIVER_INIT( puzlclub );
DRIVER_INIT( tankfrce );
DRIVER_INIT( tankfrc4 );

/*----------- defined in video/namcos1.c -----------*/

READ8_HANDLER( namcos1_videoram_r );
WRITE8_HANDLER( namcos1_videoram_w );
WRITE8_HANDLER( namcos1_paletteram_w );
READ8_HANDLER( namcos1_spriteram_r );
WRITE8_HANDLER( namcos1_spriteram_w );

VIDEO_START( namcos1 );
SCREEN_UPDATE( namcos1 );
SCREEN_EOF( namcos1 );
