#define MCU_BUFFER_MAX 6

class renegade_state : public driver_device
{
public:
	renegade_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 m_bank;
	int m_mcu_sim;
	int m_from_main;
	int m_from_mcu;
	int m_main_sent;
	int m_mcu_sent;
	UINT8 m_ddr_a;
	UINT8 m_ddr_b;
	UINT8 m_ddr_c;
	UINT8 m_port_a_out;
	UINT8 m_port_b_out;
	UINT8 m_port_c_out;
	UINT8 m_port_a_in;
	UINT8 m_port_b_in;
	UINT8 m_port_c_in;
	UINT8 m_mcu_buffer[MCU_BUFFER_MAX];
	UINT8 m_mcu_input_size;
	UINT8 m_mcu_output_byte;
	INT8 m_mcu_key;
	int m_mcu_checksum;
	const UINT8 *m_mcu_encrypt_table;
	int m_mcu_encrypt_table_len;
	int m_coin;
	UINT8 *m_videoram;
	UINT8 *m_videoram2;
	INT32 m_scrollx;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 *m_spriteram;
};


/*----------- defined in video/renegade.c -----------*/

SCREEN_UPDATE( renegade );
VIDEO_START( renegade );
WRITE8_HANDLER( renegade_scroll0_w );
WRITE8_HANDLER( renegade_scroll1_w );
WRITE8_HANDLER( renegade_videoram_w );
WRITE8_HANDLER( renegade_videoram2_w );
WRITE8_HANDLER( renegade_flipscreen_w );
