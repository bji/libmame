

class changela_state : public driver_device
{
public:
	changela_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_spriteram;

	/* video-related */
	bitmap_t *m_obj0_bitmap;
	bitmap_t *m_river_bitmap;
	bitmap_t *m_tree0_bitmap;
	bitmap_t *m_tree1_bitmap;
	UINT8*   m_tree_ram;
	UINT8*   m_memory_devices;
	UINT32   m_mem_dev_selected;	/* an offset within memory_devices area */
	UINT32   m_slopeROM_bank;
	UINT8    m_tree_en;
	UINT8    m_horizon;
	UINT8    m_v_count_river;
	UINT8    m_v_count_tree;
	int      m_tree_on[2];
	emu_timer* m_scanline_timer;

	/* misc */
	UINT8    m_tree0_col;
	UINT8    m_tree1_col;
	UINT8    m_left_bank_col;
	UINT8    m_right_bank_col;
	UINT8    m_boat_shore_col;
	UINT8    m_collision_reset;
	UINT8    m_tree_collision_reset;
	UINT8    m_prev_value_31;
	int      m_dir_31;

	/* mcu-related */
	UINT8    m_port_a_in;
	UINT8    m_port_a_out;
	UINT8    m_ddr_a;
	UINT8    m_port_b_out;
	UINT8    m_ddr_b;
	UINT8    m_port_c_in;
	UINT8    m_port_c_out;
	UINT8    m_ddr_c;

	UINT8    m_mcu_out;
	UINT8    m_mcu_in;
	UINT8    m_mcu_pc_1;
	UINT8    m_mcu_pc_0;

	/* devices */
	device_t *m_mcu;
};

/*----------- defined in video/changela.c -----------*/

VIDEO_START( changela );
SCREEN_UPDATE( changela );

WRITE8_HANDLER( changela_colors_w );
WRITE8_HANDLER( changela_mem_device_select_w );
WRITE8_HANDLER( changela_mem_device_w );
READ8_HANDLER( changela_mem_device_r );
WRITE8_HANDLER( changela_slope_rom_addr_hi_w );
WRITE8_HANDLER( changela_slope_rom_addr_lo_w );
