/***************************************************************************

    Century CVS System

    (and Quasar)

****************************************************************************/


#define CVS_S2636_Y_OFFSET     (3)
#define CVS_S2636_X_OFFSET     (-26)
#define CVS_MAX_STARS          250

struct cvs_star
{
	int x, y, code;
};

class cvs_state : public driver_device
{
public:
	cvs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_video_ram;
	UINT8 *    m_bullet_ram;
	UINT8 *    m_fo_state;
	UINT8 *    m_cvs_4_bit_dac_data;
	UINT8 *    m_tms5110_ctl_data;
	UINT8 *    m_dac3_state;

	/* video-related */
	struct cvs_star m_stars[CVS_MAX_STARS];
	bitmap_ind16   m_collision_background;
	bitmap_ind16   m_background_bitmap;
	bitmap_ind16   m_scrolled_collision_background;
	int        m_collision_register;
	int        m_total_stars;
	int        m_stars_on;
	UINT8      m_scroll_reg;
	int        m_stars_scroll;

	/* misc */
	emu_timer  *m_cvs_393hz_timer;
	UINT8      m_cvs_393hz_clock;

	UINT8      m_character_banking_mode;
	UINT16     m_character_ram_page_start;
	UINT16     m_speech_rom_bit_address;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_speech;
	device_t *m_dac3;
	device_t *m_tms;
	device_t *m_s2636_0;
	device_t *m_s2636_1;
	device_t *m_s2636_2;

	/* memory */
	UINT8      m_color_ram[0x400];
	UINT8      m_palette_ram[0x10];
	UINT8      m_character_ram[3 * 0x800];	/* only half is used, but
                                               by allocating twice the amount,
                                               we can use the same gfx_layout */
};

class quasar_state : public cvs_state
{
public:
	quasar_state(const machine_config &mconfig, device_type type, const char *tag)
		: cvs_state(mconfig, type, tag) { }

	UINT8 *    m_effectram;
	UINT8      m_effectcontrol;
	UINT8      m_page;
	UINT8      m_io_page;
};


/*----------- defined in drivers/cvs.c -----------*/

MACHINE_START( cvs );
MACHINE_RESET( cvs );

READ8_HANDLER( cvs_video_or_color_ram_r );
WRITE8_HANDLER( cvs_video_or_color_ram_w );

READ8_HANDLER( cvs_bullet_ram_or_palette_r );
WRITE8_HANDLER( cvs_bullet_ram_or_palette_w );

READ8_HANDLER( cvs_s2636_0_or_character_ram_r );
WRITE8_HANDLER( cvs_s2636_0_or_character_ram_w );
READ8_HANDLER( cvs_s2636_1_or_character_ram_r );
WRITE8_HANDLER( cvs_s2636_1_or_character_ram_w );
READ8_HANDLER( cvs_s2636_2_or_character_ram_r );
WRITE8_HANDLER( cvs_s2636_2_or_character_ram_w );

/*----------- defined in video/cvs.c -----------*/

WRITE8_HANDLER( cvs_scroll_w );
WRITE8_HANDLER( cvs_video_fx_w );

READ8_HANDLER( cvs_collision_r );
READ8_HANDLER( cvs_collision_clear );

void cvs_scroll_stars(running_machine &machine);

PALETTE_INIT( cvs );
SCREEN_UPDATE_IND16( cvs );
VIDEO_START( cvs );

/*----------- defined in video/quasar.c -----------*/

PALETTE_INIT( quasar );
SCREEN_UPDATE_IND16( quasar );
VIDEO_START( quasar );
