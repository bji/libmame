/*************************************************************************

    Entertainment Sciences RIP System hardware

*************************************************************************/

/* TODO */
#define ESRIPSYS_PIXEL_CLOCK	(XTAL_25MHz / 2)
#define ESRIPSYS_HTOTAL			(512 + 141 + 2)
#define ESRIPSYS_HBLANK_START	(512)
#define ESRIPSYS_HBLANK_END		(0)
#define ESRIPSYS_VTOTAL			(384 + 20)
#define ESRIPSYS_VBLANK_START	(384)
#define ESRIPSYS_VBLANK_END		(0)

#define CMOS_RAM_SIZE			(2048)
#define FDT_RAM_SIZE			(2048 * sizeof(UINT16))

struct line_buffer_t
{
	UINT8 *colour_buf;
	UINT8 *intensity_buf;
	UINT8 *priority_buf;
};

class esripsys_state : public driver_device
{
public:
	esripsys_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 m_g_iodata;
	UINT8 m_g_ioaddr;
	UINT8 m_coin_latch;
	UINT8 m_keypad_status;
	UINT8 m_g_status;
	UINT8 m_f_status;
	int m_io_firq_status;
	UINT8 m_cmos_ram_a2_0;
	UINT8 m_cmos_ram_a10_3;
	UINT8 *m_cmos_ram;
	UINT8 m_u56a;
	UINT8 m_u56b;
	UINT8 m_g_to_s_latch1;
	UINT8 m_g_to_s_latch2;
	UINT8 m_s_to_g_latch1;
	UINT8 m_s_to_g_latch2;
	UINT8 m_dac_msb;
	UINT8 m_dac_vol;
	UINT8 m_tms_data;
	UINT8 *m_fdt_a;
	UINT8 *m_fdt_b;
	struct line_buffer_t m_line_buffer[2];
	int m_fasel;
	int m_fbsel;
	int m_hblank;
	UINT8 *m_pal_ram;
	int m_frame_vbl;
	int m_12sel;
	int m_video_firq_en;
	emu_timer *m_hblank_end_timer;
	emu_timer *m_hblank_start_timer;
	UINT8 *m_fig_scale_table;
	UINT8 *m_scale_table;
	int m_video_firq;
	UINT8 m_bg_intensity;
};


/*----------- defined in video/esripsys.c -----------*/

VIDEO_START( esripsys );
SCREEN_UPDATE( esripsys );

WRITE8_HANDLER( esripsys_bg_intensity_w );
INTERRUPT_GEN( esripsys_vblank_irq );

int esripsys_draw(running_machine &machine, int l, int r, int fig, int attr, int addr, int col, int x_scale, int bank);
