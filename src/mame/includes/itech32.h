/*************************************************************************

    Incredible Technologies/Strata system
    (8-bit blitter variant)

**************************************************************************/

#include "machine/nvram.h"

#define VIDEO_CLOCK		XTAL_8MHz			/* video (pixel) clock */
#define CPU_CLOCK		XTAL_12MHz			/* clock for 68000-based systems */
#define CPU020_CLOCK	XTAL_25MHz			/* clock for 68EC020-based systems */
#define SOUND_CLOCK		XTAL_16MHz			/* clock for sound board */
#define TMS_CLOCK		XTAL_40MHz			/* TMS320C31 clocks on drivedge */


class itech32_state : public driver_device
{
public:
	itech32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	void nvram_init(nvram_device &nvram, void *base, size_t length);

	UINT16 *m_videoram;
	UINT8 m_vint_state;
	UINT8 m_xint_state;
	UINT8 m_qint_state;
	UINT8 m_sound_data;
	UINT8 m_sound_return;
	UINT8 m_sound_int_state;
	UINT16 *m_main_rom;
	UINT16 *m_main_ram;
	offs_t m_itech020_prot_address;
	UINT32 *m_tms1_ram;
	UINT32 *m_tms2_ram;
	UINT32 *m_tms1_boot;
	UINT8 m_tms_spinning[2];
	int m_special_result;
	int m_p1_effx;
	int m_p1_effy;
	int m_p1_lastresult;
	attotime m_p1_lasttime;
	int m_p2_effx;
	int m_p2_effy;
	int m_p2_lastresult;
	attotime m_p2_lasttime;
	UINT8 m_written[0x8000];
	int m_is_drivedge;
	UINT16 *m_video;
	UINT32 *m_drivedge_zbuf_control;
	UINT8 m_planes;
	UINT16 m_vram_height;
	UINT16 m_xfer_xcount;
	UINT16 m_xfer_ycount;
	UINT16 m_xfer_xcur;
	UINT16 m_xfer_ycur;
	rectangle m_clip_rect;
	rectangle m_scaled_clip_rect;
	rectangle m_clip_save;
	emu_timer *m_scanline_timer;
	UINT8 *m_grom_base;
	UINT32 m_grom_size;
	UINT32 m_grom_bank;
	UINT32 m_grom_bank_mask;
	UINT16 m_color_latch[2];
	UINT8 m_enable_latch[2];
	UINT16 *m_videoplane[2];
	UINT32 m_vram_mask;
	UINT32 m_vram_xmask;
	UINT32 m_vram_ymask;
};


/*----------- defined in drivers/itech32.c -----------*/

void itech32_update_interrupts(running_machine &machine, int vint, int xint, int qint);


/*----------- defined in video/itech32.c -----------*/

VIDEO_START( itech32 );

WRITE16_HANDLER( timekill_colora_w );
WRITE16_HANDLER( timekill_colorbc_w );
WRITE16_HANDLER( timekill_intensity_w );

WRITE16_HANDLER( bloodstm_color1_w );
WRITE16_HANDLER( bloodstm_color2_w );
WRITE16_HANDLER( bloodstm_plane_w );

WRITE32_HANDLER( drivedge_color0_w );

WRITE32_HANDLER( itech020_color1_w );
WRITE32_HANDLER( itech020_color2_w );
WRITE32_HANDLER( itech020_plane_w );

WRITE16_HANDLER( timekill_paletteram_w );
WRITE16_HANDLER( bloodstm_paletteram_w );
WRITE32_HANDLER( drivedge_paletteram_w );
WRITE32_HANDLER( itech020_paletteram_w );

WRITE16_HANDLER( itech32_video_w );
READ16_HANDLER( itech32_video_r );

WRITE16_HANDLER( bloodstm_video_w );
READ16_HANDLER( bloodstm_video_r );
WRITE32_HANDLER( itech020_video_w );
READ32_HANDLER( itech020_video_r );
WRITE32_HANDLER( drivedge_zbuf_control_w );

SCREEN_UPDATE( itech32 );
