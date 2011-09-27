
class lsasquad_state : public driver_device
{
public:
	lsasquad_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *      m_scrollram;
	UINT8 *      m_videoram;
	UINT8 *      m_spriteram;
	size_t       m_spriteram_size;
	size_t       m_videoram_size;

	/* misc */
	int m_sound_pending;
	int m_sound_nmi_enable;
	int m_pending_nmi;
	int m_sound_cmd;
	int m_sound_result;

	/* mcu */
	UINT8 m_from_main;
	UINT8 m_from_mcu;
	int m_mcu_sent;
	int m_main_sent;
	UINT8 m_port_a_in;
	UINT8 m_port_a_out;
	UINT8 m_ddr_a;
	UINT8 m_port_b_in;
	UINT8 m_port_b_out;
	UINT8 m_ddr_b;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_mcu;
};

/*----------- defined in machine/lsasquad.c -----------*/

WRITE8_HANDLER( lsasquad_sh_nmi_disable_w );
WRITE8_HANDLER( lsasquad_sh_nmi_enable_w );
WRITE8_HANDLER( lsasquad_sound_command_w );
READ8_HANDLER( lsasquad_sh_sound_command_r );
WRITE8_HANDLER( lsasquad_sh_result_w );
READ8_HANDLER( lsasquad_sound_result_r );
READ8_HANDLER( lsasquad_sound_status_r );

READ8_HANDLER( lsasquad_68705_port_a_r );
WRITE8_HANDLER( lsasquad_68705_port_a_w );
WRITE8_HANDLER( lsasquad_68705_ddr_a_w );
READ8_HANDLER( lsasquad_68705_port_b_r );
WRITE8_HANDLER( lsasquad_68705_port_b_w );
WRITE8_HANDLER( lsasquad_68705_ddr_b_w );
WRITE8_HANDLER( lsasquad_mcu_w );
READ8_HANDLER( lsasquad_mcu_r );
READ8_HANDLER( lsasquad_mcu_status_r );

READ8_HANDLER( daikaiju_sound_status_r );
READ8_HANDLER( daikaiju_sh_sound_command_r );
READ8_HANDLER( daikaiju_mcu_status_r );


/*----------- defined in video/lsasquad.c -----------*/

SCREEN_UPDATE( lsasquad );
SCREEN_UPDATE( daikaiju );
