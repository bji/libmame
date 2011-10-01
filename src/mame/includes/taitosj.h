class taitosj_state : public driver_device
{
public:
	taitosj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_sndnmi_disable;
	UINT8 m_input_port_4_f0;
	UINT8 m_kikstart_gears[2];
	INT8 m_dac_out;
	UINT8 m_dac_vol;
	UINT8 *m_videoram_1;
	UINT8 *m_videoram_2;
	UINT8 *m_videoram_3;
	UINT8 *m_spriteram;
	UINT8 *m_paletteram;
	UINT8 *m_characterram;
	UINT8 *m_scroll;
	UINT8 *m_colscrolly;
	UINT8 *m_gfxpointer;
	UINT8 *m_colorbank;
	UINT8 *m_video_mode;
	UINT8 *m_video_priority;
	UINT8 *m_collision_reg;
	UINT8 *m_kikstart_scrollram;
	UINT8 m_fromz80;
	UINT8 m_toz80;
	UINT8 m_zaccept;
	UINT8 m_zready;
	UINT8 m_busreq;
	UINT8 m_portA_in;
	UINT8 m_portA_out;
	UINT8 m_spacecr_prot_value;
	UINT8 m_protection_value;
	UINT32 m_address;
	bitmap_t *m_layer_bitmap[3];
	bitmap_t *m_sprite_sprite_collbitmap1;
	bitmap_t *m_sprite_sprite_collbitmap2;
	bitmap_t *m_sprite_layer_collbitmap1;
	bitmap_t *m_sprite_layer_collbitmap2[3];
	int m_draw_order[32][4];
};


/*----------- defined in machine/taitosj.c -----------*/

MACHINE_START( taitosj );
MACHINE_RESET( taitosj );
WRITE8_HANDLER( taitosj_bankswitch_w );
READ8_HANDLER( taitosj_fake_data_r );
READ8_HANDLER( taitosj_fake_status_r );
WRITE8_HANDLER( taitosj_fake_data_w );
READ8_HANDLER( taitosj_mcu_data_r );
READ8_HANDLER( taitosj_mcu_status_r );
WRITE8_HANDLER( taitosj_mcu_data_w );
READ8_HANDLER( taitosj_68705_portA_r );
READ8_HANDLER( taitosj_68705_portB_r );
READ8_HANDLER( taitosj_68705_portC_r );
WRITE8_HANDLER( taitosj_68705_portA_w );
WRITE8_HANDLER( taitosj_68705_portB_w );

READ8_HANDLER( spacecr_prot_r );
WRITE8_HANDLER( alpine_protection_w );
WRITE8_HANDLER( alpinea_bankswitch_w );
READ8_HANDLER( alpine_port_2_r );


/*----------- defined in video/taitosj.c -----------*/

READ8_HANDLER( taitosj_gfxrom_r );
WRITE8_HANDLER( taitosj_characterram_w );
WRITE8_HANDLER( junglhbr_characterram_w );
WRITE8_HANDLER( taitosj_collision_reg_clear_w );
VIDEO_START( taitosj );
SCREEN_UPDATE( taitosj );
SCREEN_UPDATE( kikstart );
