class xain_state : public driver_device
{
public:
	xain_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_vblank;
	int m_from_main;
	int m_from_mcu;
	UINT8 m_ddr_a;
	UINT8 m_ddr_b;
	UINT8 m_ddr_c;
	UINT8 m_port_a_out;
	UINT8 m_port_b_out;
	UINT8 m_port_c_out;
	UINT8 m_port_a_in;
	UINT8 m_port_b_in;
	UINT8 m_port_c_in;
	int m_mcu_ready;
	int m_mcu_accept;
	UINT8 *m_charram;
	UINT8 *m_bgram0;
	UINT8 *m_bgram1;
	UINT8 m_pri;
	tilemap_t *m_char_tilemap;
	tilemap_t *m_bgram0_tilemap;
	tilemap_t *m_bgram1_tilemap;
	UINT8 m_scrollxP0[2];
	UINT8 m_scrollyP0[2];
	UINT8 m_scrollxP1[2];
	UINT8 m_scrollyP1[2];
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/xain.c -----------*/

SCREEN_UPDATE( xain );
VIDEO_START( xain );
WRITE8_HANDLER( xain_scrollxP0_w );
WRITE8_HANDLER( xain_scrollyP0_w );
WRITE8_HANDLER( xain_scrollxP1_w );
WRITE8_HANDLER( xain_scrollyP1_w );
WRITE8_HANDLER( xain_charram_w );
WRITE8_HANDLER( xain_bgram0_w );
WRITE8_HANDLER( xain_bgram1_w );
WRITE8_HANDLER( xain_flipscreen_w );
