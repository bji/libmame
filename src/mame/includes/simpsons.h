
class simpsons_state : public driver_device
{
public:
	simpsons_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_ram;
	UINT8 *    m_xtraram;
	UINT16 *   m_spriteram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_sprite_colorbase;
	int        m_layer_colorbase[3];
	int        m_layerpri[3];

	/* misc */
	int        m_firq_enabled;
	int        m_video_bank;
	//int        m_nmi_enabled;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k053260;
	device_t *m_k052109;
	device_t *m_k053246;
	device_t *m_k053251;
};

/*----------- defined in machine/simpsons.c -----------*/

WRITE8_HANDLER( simpsons_eeprom_w );
WRITE8_HANDLER( simpsons_coin_counter_w );
READ8_HANDLER( simpsons_sound_interrupt_r );
READ8_DEVICE_HANDLER( simpsons_sound_r );
MACHINE_RESET( simpsons );
MACHINE_START( simpsons );

/*----------- defined in video/simpsons.c -----------*/

void simpsons_video_banking( running_machine &machine, int select );
SCREEN_UPDATE( simpsons );

extern void simpsons_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void simpsons_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
