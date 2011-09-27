class seta2_state : public driver_device
{
public:
	seta2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	optional_shared_ptr<UINT16> m_nvram;

	UINT16 *m_vregs;
	int m_xoffset;
	int m_yoffset;
	int m_keyboard_row;

	UINT16 *m_spriteram;
	size_t m_spriteram_size;
	UINT16 *m_buffered_spriteram;
	UINT32 *m_coldfire_regs;

	UINT8 *m_funcube_outputs;
	UINT8 *m_funcube_leds;

	UINT64 m_funcube_coin_start_cycles;
	UINT8 m_funcube_hopper_motor;
	UINT8 m_funcube_press;

	UINT8 m_funcube_serial_fifo[4];
	UINT8 m_funcube_serial_count;
};

/*----------- defined in video/seta2.c -----------*/

WRITE16_HANDLER( seta2_vregs_w );

VIDEO_START( seta2 );
VIDEO_START( seta2_xoffset );
VIDEO_START( seta2_yoffset );
SCREEN_UPDATE( seta2 );
SCREEN_EOF( seta2 );
