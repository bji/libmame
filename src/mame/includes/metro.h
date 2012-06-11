/*************************************************************************

    Metro Games

*************************************************************************/

#include "sound/okim6295.h"
#include "sound/2151intf.h"
#include "video/konicdev.h"

class metro_state : public driver_device
{
public:
	metro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_audiocpu(*this, "audiocpu"),
		  m_oki(*this, "oki"),
		  m_ymsnd(*this, "ymsnd"),
		  m_k053936(*this, "k053936") ,
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vram_2(*this, "vram_2"),
		m_spriteram(*this, "spriteram"),
		m_tiletable(*this, "tiletable"),
		m_blitter_regs(*this, "blitter_regs"),
		m_scroll(*this, "scroll"),
		m_window(*this, "window"),
		m_irq_enable(*this, "irq_enable"),
		m_irq_levels(*this, "irq_levels"),
		m_irq_vectors(*this, "irq_vectors"),
		m_rombank(*this, "rombank"),
		m_videoregs(*this, "videoregs"),
		m_screenctrl(*this, "screenctrl"),
		m_input_sel(*this, "input_sel"),
		m_k053936_ram(*this, "k053936_ram"){ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<device_t> m_ymsnd;
	optional_device<k053936_device> m_k053936;
	/* memory pointers */
	optional_shared_ptr<UINT16> m_vram_0;
	optional_shared_ptr<UINT16> m_vram_1;
	optional_shared_ptr<UINT16> m_vram_2;
	required_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_tiletable;
	UINT16 *    m_tiletable_old;
	optional_shared_ptr<UINT16> m_blitter_regs;
	optional_shared_ptr<UINT16> m_scroll;
	optional_shared_ptr<UINT16> m_window;
	optional_shared_ptr<UINT16> m_irq_enable;
	optional_shared_ptr<UINT16> m_irq_levels;
	optional_shared_ptr<UINT16> m_irq_vectors;
	optional_shared_ptr<UINT16> m_rombank;
	required_shared_ptr<UINT16> m_videoregs;
	optional_shared_ptr<UINT16> m_screenctrl;
	optional_shared_ptr<UINT16> m_input_sel;
	optional_shared_ptr<UINT16> m_k053936_ram;


	int         m_flip_screen;

	/* video-related */
	tilemap_t   *m_k053936_tilemap;
	int         m_bg_tilemap_enable[3];
	int         m_bg_tilemap_enable16[3];
	int         m_bg_tilemap_scrolldx[3];

	int         m_support_8bpp;
	int         m_support_16x16;
	int         m_has_zoom;
	int         m_sprite_xoffs;
	int         m_sprite_yoffs;

	UINT8 *		m_expanded_gfx1;

	/* blitter */
	int         m_blitter_bit;

	/* irq_related */
	int         m_irq_line;
	UINT8       m_requested_int[8];
	emu_timer   *m_mouja_irq_timer;

	/* sound related */
	UINT16      m_soundstatus;
	int         m_porta;
	int         m_portb;
	int         m_busy_sndcpu;

	/* misc */
	int         m_gakusai_oki_bank_lo;
	int         m_gakusai_oki_bank_hi;


	DECLARE_READ16_MEMBER(metro_irq_cause_r);
	DECLARE_WRITE16_MEMBER(metro_irq_cause_w);
	DECLARE_WRITE16_MEMBER(mouja_irq_timer_ctrl_w);
	DECLARE_WRITE16_MEMBER(metro_soundlatch_w);
	DECLARE_READ16_MEMBER(metro_soundstatus_r);
	DECLARE_WRITE16_MEMBER(metro_soundstatus_w);
	DECLARE_WRITE8_MEMBER(metro_sound_rombank_w);
	DECLARE_WRITE8_MEMBER(daitorid_sound_rombank_w);
	DECLARE_READ8_MEMBER(metro_porta_r);
	DECLARE_WRITE8_MEMBER(metro_porta_w);
	DECLARE_WRITE8_MEMBER(metro_portb_w);
	DECLARE_WRITE8_MEMBER(daitorid_portb_w);
	DECLARE_WRITE16_MEMBER(metro_coin_lockout_1word_w);
	DECLARE_WRITE16_MEMBER(metro_coin_lockout_4words_w);
	DECLARE_READ16_MEMBER(metro_bankedrom_r);
	DECLARE_WRITE16_MEMBER(metro_blitter_w);
	DECLARE_READ16_MEMBER(balcube_dsw_r);
	DECLARE_READ16_MEMBER(karatour_vram_0_r);
	DECLARE_READ16_MEMBER(karatour_vram_1_r);
	DECLARE_READ16_MEMBER(karatour_vram_2_r);
	DECLARE_WRITE16_MEMBER(karatour_vram_0_w);
	DECLARE_WRITE16_MEMBER(karatour_vram_1_w);
	DECLARE_WRITE16_MEMBER(karatour_vram_2_w);
	DECLARE_READ16_MEMBER(gakusai_input_r);
	DECLARE_WRITE16_MEMBER(blzntrnd_sound_w);
	DECLARE_WRITE8_MEMBER(blzntrnd_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(puzzlet_irq_enable_w);
	DECLARE_WRITE16_MEMBER(vram_0_clr_w);
	DECLARE_WRITE16_MEMBER(vram_1_clr_w);
	DECLARE_WRITE16_MEMBER(vram_2_clr_w);
	DECLARE_WRITE8_MEMBER(puzzlet_portb_w);
	DECLARE_WRITE16_MEMBER(metro_k053936_w);
	DECLARE_WRITE16_MEMBER(metro_vram_0_w);
	DECLARE_WRITE16_MEMBER(metro_vram_1_w);
	DECLARE_WRITE16_MEMBER(metro_vram_2_w);
	DECLARE_WRITE16_MEMBER(metro_window_w);
	void blt_write( address_space *space, const int tmap, const offs_t offs, const UINT16 data, const UINT16 mask );
	DECLARE_CUSTOM_INPUT_MEMBER(custom_soundstatus_r);
};


/*----------- defined in video/metro.c -----------*/


VIDEO_START( metro_i4100 );
VIDEO_START( metro_i4220 );
VIDEO_START( metro_i4300 );
VIDEO_START( blzntrnd );
VIDEO_START( gstrik2 );

SCREEN_UPDATE_IND16( metro );

void metro_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect);
