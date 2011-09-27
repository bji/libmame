
class ajax_state : public driver_device
{
public:
	ajax_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_zoom_colorbase;
	UINT8      m_priority;

	/* misc */
	int        m_firq_enable;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_subcpu;
	device_t *m_k007232_1;
	device_t *m_k007232_2;
	device_t *m_k052109;
	device_t *m_k051960;
	device_t *m_k051316;
};


/*----------- defined in machine/ajax.c -----------*/

WRITE8_HANDLER( ajax_bankswitch_2_w );
READ8_HANDLER( ajax_ls138_f10_r );
WRITE8_HANDLER( ajax_ls138_f10_w );
MACHINE_START( ajax );
MACHINE_RESET( ajax );
INTERRUPT_GEN( ajax_interrupt );

/*----------- defined in video/ajax.c -----------*/

VIDEO_START( ajax );
SCREEN_UPDATE( ajax );

extern void ajax_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void ajax_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);
extern void ajax_zoom_callback(running_machine &machine, int *code,int *color,int *flags);
