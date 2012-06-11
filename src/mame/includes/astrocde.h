/***************************************************************************

    Bally Astrocade-based hardware

***************************************************************************/

#define ASTROCADE_CLOCK		(XTAL_14_31818MHz/2)

#define AC_SOUND_PRESENT	(0x01)
#define AC_LIGHTPEN_INTS	(0x02)
#define AC_STARS			(0x04)
#define AC_MONITOR_BW		(0x08)


class astrocde_state : public driver_device
{
public:
	astrocde_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 m_video_config;
	UINT8 m_sparkle[4];
	char m_totalword[256];
	char *m_totalword_ptr;
	char m_oldword[256];
	int m_plural;
	UINT8 *m_protected_ram;
	UINT8 m_port_1_last;
	UINT8 m_port_2_last;
	UINT8 m_ram_write_enable;
	UINT8 m_input_select;
	UINT8 m_profpac_bank;
	UINT8 *m_sparklestar;
	UINT8 m_interrupt_enabl;
	UINT8 m_interrupt_vector;
	UINT8 m_interrupt_scanline;
	UINT8 m_vertical_feedback;
	UINT8 m_horizontal_feedback;
	emu_timer *m_scanline_timer;
	UINT8 m_colors[8];
	UINT8 m_colorsplit;
	UINT8 m_bgdata;
	UINT8 m_vblank;
	UINT8 m_video_mode;
	UINT8 m_funcgen_expand_color[2];
	UINT8 m_funcgen_control;
	UINT8 m_funcgen_expand_count;
	UINT8 m_funcgen_rotate_count;
	UINT8 m_funcgen_rotate_data[4];
	UINT8 m_funcgen_shift_prev_data;
	UINT8 m_funcgen_intercept;
	UINT16 m_pattern_source;
	UINT8 m_pattern_mode;
	UINT16 m_pattern_dest;
	UINT8 m_pattern_skip;
	UINT8 m_pattern_width;
	UINT8 m_pattern_height;
	UINT16 *m_profpac_videoram;
	UINT16 m_profpac_palette[16];
	UINT8 m_profpac_colormap[4];
	UINT8 m_profpac_intercept;
	UINT8 m_profpac_vispage;
	UINT8 m_profpac_readpage;
	UINT8 m_profpac_readshift;
	UINT8 m_profpac_writepage;
	UINT8 m_profpac_writemode;
	UINT16 m_profpac_writemask;
	UINT8 m_profpac_vw;
};


/*----------- defined in video/astrocde.c -----------*/

PALETTE_INIT( astrocde );
PALETTE_INIT( profpac );

VIDEO_START( astrocde );
VIDEO_START( profpac );

SCREEN_UPDATE_IND16( astrocde );
SCREEN_UPDATE_IND16( profpac );

WRITE8_HANDLER( astrocade_pattern_board_w );
READ8_HANDLER( astrocade_data_chip_register_r );
WRITE8_HANDLER( astrocade_data_chip_register_w );
WRITE8_HANDLER( astrocade_funcgen_w );

READ8_HANDLER( profpac_videoram_r );
WRITE8_HANDLER( profpac_videoram_w );
READ8_HANDLER( profpac_intercept_r );
WRITE8_HANDLER( profpac_page_select_w );
WRITE8_HANDLER( profpac_screenram_ctrl_w );


/*----------- defined in audio/wow.c -----------*/

extern const char *const wow_sample_names[];

READ8_HANDLER( wow_speech_r );
CUSTOM_INPUT( wow_speech_status_r );


/*----------- defined in audio/gorf.c -----------*/

extern const char *const gorf_sample_names[];

READ8_HANDLER( gorf_speech_r );
CUSTOM_INPUT( gorf_speech_status_r );
