
class flstory_state : public driver_device
{
public:
	flstory_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_workram;
	UINT8 *  m_scrlram;
	UINT8 *  m_spriteram;
//  UINT8 *  m_paletteram;    // currently this uses generic palette handling
//  UINT8 *  m_paletteram_2;  // currently this uses generic palette handling
	size_t   m_videoram_size;
	size_t   m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int      m_char_bank;
	int      m_palette_bank;
	int      m_flipscreen;
	int      m_gfxctrl;

	/* sound-related */
	UINT8    m_snd_data;
	UINT8    m_snd_flag;
	int      m_sound_nmi_enable;
	int      m_pending_nmi;
	int      m_vol_ctrl[16];
	UINT8    m_snd_ctrl0;
	UINT8    m_snd_ctrl1;
	UINT8    m_snd_ctrl2;
	UINT8    m_snd_ctrl3;

	/* protection */
	UINT8    m_from_main;
	UINT8    m_from_mcu;
	int      m_mcu_sent;
	int      m_main_sent;
	UINT8    m_port_a_in;
	UINT8    m_port_a_out;
	UINT8    m_ddr_a;
	UINT8    m_port_b_in;
	UINT8    m_port_b_out;
	UINT8    m_ddr_b;
	UINT8    m_port_c_in;
	UINT8    m_port_c_out;
	UINT8    m_ddr_c;
	int      m_mcu_select;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_mcu;

	/* mcu */
	UINT8 m_mcu_cmd;
	UINT8 m_mcu_counter;
	UINT8 m_mcu_b4_cmd;
	UINT8 m_mcu_param;
	UINT8 m_mcu_b2_res;
	UINT8 m_mcu_b1_res;
	UINT8 m_mcu_bb_res;
	UINT8 m_mcu_b5_res;
	UINT8 m_mcu_b6_res;
};


/*----------- defined in machine/flstory.c -----------*/

READ8_HANDLER( flstory_68705_port_a_r );
WRITE8_HANDLER( flstory_68705_port_a_w );
READ8_HANDLER( flstory_68705_port_b_r );
WRITE8_HANDLER( flstory_68705_port_b_w );
READ8_HANDLER( flstory_68705_port_c_r );
WRITE8_HANDLER( flstory_68705_port_c_w );
WRITE8_HANDLER( flstory_68705_ddr_a_w );
WRITE8_HANDLER( flstory_68705_ddr_b_w );
WRITE8_HANDLER( flstory_68705_ddr_c_w );
WRITE8_HANDLER( flstory_mcu_w );
READ8_HANDLER( flstory_mcu_r );
READ8_HANDLER( flstory_mcu_status_r );
WRITE8_HANDLER( onna34ro_mcu_w );
READ8_HANDLER( onna34ro_mcu_r );
READ8_HANDLER( onna34ro_mcu_status_r );
WRITE8_HANDLER( victnine_mcu_w );
READ8_HANDLER( victnine_mcu_r );
READ8_HANDLER( victnine_mcu_status_r );


/*----------- defined in video/flstory.c -----------*/

VIDEO_START( flstory );
SCREEN_UPDATE( flstory );
VIDEO_START( victnine );
SCREEN_UPDATE( victnine );
VIDEO_START( rumba );
SCREEN_UPDATE( rumba );

WRITE8_HANDLER( flstory_videoram_w );
READ8_HANDLER( flstory_palette_r );
WRITE8_HANDLER( flstory_palette_w );
WRITE8_HANDLER( flstory_gfxctrl_w );
WRITE8_HANDLER( flstory_scrlram_w );
READ8_HANDLER( victnine_gfxctrl_r );
WRITE8_HANDLER( victnine_gfxctrl_w );
