
class bigevglf_state : public driver_device
{
public:
	bigevglf_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  m_paletteram;
	UINT8 *  m_spriteram1;
	UINT8 *  m_spriteram2;

	/* video-related */
	bitmap_t *m_tmp_bitmap[4];
	UINT8    *m_vidram;
	UINT32   m_vidram_bank;
	UINT32   m_plane_selected;
	UINT32   m_plane_visible;

	/* sound-related */
	int      m_sound_nmi_enable;
	int      m_pending_nmi;
	UINT8    m_for_sound;
	UINT8    m_from_sound;
	UINT8    m_sound_state;

	/* MCU related */
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
	int      m_mcu_coin_bit5;

	/* misc */
	UINT32   m_beg_bank;
	UINT8    m_beg13_ls74[2];
	UINT8    m_port_select;     /* for muxed controls */

	/* devices */
	device_t *m_audiocpu;
	device_t *m_mcu;
};


/*----------- defined in machine/bigevglf.c -----------*/

READ8_HANDLER( bigevglf_68705_port_a_r );
WRITE8_HANDLER( bigevglf_68705_port_a_w );
READ8_HANDLER( bigevglf_68705_port_b_r );
WRITE8_HANDLER( bigevglf_68705_port_b_w );
READ8_HANDLER( bigevglf_68705_port_c_r );
WRITE8_HANDLER( bigevglf_68705_port_c_w );
WRITE8_HANDLER( bigevglf_68705_ddr_a_w );
WRITE8_HANDLER( bigevglf_68705_ddr_b_w );
WRITE8_HANDLER( bigevglf_68705_ddr_c_w );

WRITE8_HANDLER( bigevglf_mcu_w );
READ8_HANDLER( bigevglf_mcu_r );
READ8_HANDLER( bigevglf_mcu_status_r );


/*----------- defined in video/bigevglf.c -----------*/

VIDEO_START( bigevglf );
SCREEN_UPDATE( bigevglf );

READ8_HANDLER( bigevglf_vidram_r );
WRITE8_HANDLER( bigevglf_vidram_w );
WRITE8_HANDLER( bigevglf_vidram_addr_w );

WRITE8_HANDLER( bigevglf_gfxcontrol_w );
WRITE8_HANDLER( bigevglf_palette_w );
