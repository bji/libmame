
class lkage_state : public driver_device
{
public:
	lkage_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *      m_scroll;
	UINT8 *      m_vreg;
	UINT8 *      m_videoram;
	UINT8 *      m_spriteram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;
	UINT8 m_bg_tile_bank;
	UINT8 m_fg_tile_bank;
	UINT8 m_tx_tile_bank;

	int m_sprite_dx;

	/* misc */
	int m_sound_nmi_enable;
	int m_pending_nmi;

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
	UINT8 m_port_c_in;
	UINT8 m_port_c_out;
	UINT8 m_ddr_c;

	/* lkageb fake mcu */
	UINT8 m_mcu_val;
	int m_mcu_ready;	/* cpu data/mcu ready status */

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_mcu;
};

/*----------- defined in machine/lkage.c -----------*/

READ8_HANDLER( lkage_68705_port_a_r );
WRITE8_HANDLER( lkage_68705_port_a_w );
READ8_HANDLER( lkage_68705_port_b_r );
WRITE8_HANDLER( lkage_68705_port_b_w );
READ8_HANDLER( lkage_68705_port_c_r );
WRITE8_HANDLER( lkage_68705_port_c_w );
WRITE8_HANDLER( lkage_68705_ddr_a_w );
WRITE8_HANDLER( lkage_68705_ddr_b_w );
WRITE8_HANDLER( lkage_68705_ddr_c_w );
WRITE8_HANDLER( lkage_mcu_w );
READ8_HANDLER( lkage_mcu_r );
READ8_HANDLER( lkage_mcu_status_r );


/*----------- defined in video/lkage.c -----------*/

WRITE8_HANDLER( lkage_videoram_w );
VIDEO_START( lkage );
SCREEN_UPDATE( lkage );

