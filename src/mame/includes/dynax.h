/***************************************************************************

      Dynax hardware

***************************************************************************/

#include "sound/okim6295.h"

class dynax_state : public driver_device
{
public:
	dynax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_dsw_sel16(*this, "dsw_sel16"),
		  m_protection1(*this, "protection1"),
		  m_protection2(*this, "protection2")
		{ }

	// up to 8 layers, 2 images per layer (interleaved on screen)
	UINT8 *  m_pixmap[8][2];
	UINT8 *  m_ddenlovr_pixmap[8];

	/* irq */
	void (*m_update_irq_func)(running_machine &machine);	// some games trigger IRQ at blitter end, some don't
	UINT8 m_sound_irq;
	UINT8 m_vblank_irq;
	UINT8 m_blitter_irq;
	UINT8 m_blitter2_irq;
	UINT8 m_soundlatch_irq;
	UINT8 m_sound_vblank_irq;

	/* blitters */
	int m_blit_scroll_x;
	int m_blit2_scroll_x;
	int m_blit_scroll_y;
	int m_blit2_scroll_y;
	int m_blit_wrap_enable;
	int m_blit2_wrap_enable;
	int m_blit_x;
	int m_blit_y;
	int m_blit2_x;
	int m_blit2_y;
	int m_blit_src;
	int m_blit2_src;
	int m_blit_romregion;
	int m_blit2_romregion;
	int m_blit_dest;
	int m_blit2_dest;
	int m_blit_pen;
	int m_blit2_pen;
	int m_blit_palbank;
	int m_blit2_palbank;
	int m_blit_palettes;
	int m_blit2_palettes;
	int m_layer_enable;
	int m_blit_backpen;

	int m_hanamai_layer_half;
	int m_hnoridur_layer_half2;

	int m_extra_scroll_x;
	int m_extra_scroll_y;
	int m_flipscreen;

	int m_layer_layout;

	const int *m_priority_table;
	int m_hanamai_priority;

	/* ddenlovr blitter (TODO: merge with the above, where possible) */
	int m_extra_layers;
	int m_ddenlovr_dest_layer;
	int m_ddenlovr_blit_flip;
	int m_ddenlovr_blit_x;
	int m_ddenlovr_blit_y;
	int m_ddenlovr_blit_address;
	int m_ddenlovr_blit_pen;
	int m_ddenlovr_blit_pen_mode;
	int m_ddenlovr_blitter_irq_flag;
	int m_ddenlovr_blitter_irq_enable;
	int m_ddenlovr_rect_width;
	int m_ddenlovr_rect_height;
	int m_ddenlovr_clip_width;
	int m_ddenlovr_clip_height;
	int m_ddenlovr_line_length;
	int m_ddenlovr_clip_ctrl;
	int m_ddenlovr_clip_x;
	int m_ddenlovr_clip_y;
	int m_ddenlovr_scroll[8*2];
	int m_ddenlovr_priority;
	int m_ddenlovr_priority2;
	int m_ddenlovr_bgcolor;
	int m_ddenlovr_bgcolor2;
	int m_ddenlovr_layer_enable;
	int m_ddenlovr_layer_enable2;
	int m_ddenlovr_palette_base[8];
	int m_ddenlovr_palette_mask[8];
	int m_ddenlovr_transparency_pen[8];
	int m_ddenlovr_transparency_mask[8];
	int m_ddenlovr_blit_latch;
	int m_ddenlovr_blit_pen_mask;	// not implemented
	int m_ddenlovr_blit_rom_bits;			// usually 8, 16 in hanakanz
	const int *m_ddenlovr_blit_commands;
	int m_ddenlovr_blit_regs[2];

	/* input */
	UINT8 m_input_sel;
	UINT8 m_dsw_sel;
	UINT8 m_keyb;
	UINT8 m_coins;
	UINT8 m_hopper;

	/* misc */
	int m_hnoridur_bank;
	UINT8 m_palette_ram[16*256*2];
	int m_palbank;
	int m_msm5205next;
	int m_resetkludge;
	int m_toggle;
	int m_toggle_cpu1;
	int m_yarunara_clk_toggle;
	UINT8 m_soundlatch_ack;
	UINT8 m_soundlatch_full;
	UINT8 m_latch;
	int m_rombank;
	UINT8 m_tenkai_p5_val;
	int m_tenkai_6c;
	int m_tenkai_70;
	UINT8 m_gekisha_val[2];
	UINT8 m_gekisha_rom_enable;
	UINT8 *m_romptr;

	/* ddenlovr misc (TODO: merge with the above, where possible) */
	UINT8 m_palram[0x200];
	int m_okibank;
	UINT8 m_rongrong_blitter_busy_select;

	optional_shared_ptr<UINT16> m_dsw_sel16;
	optional_shared_ptr<UINT16> m_protection1;
	optional_shared_ptr<UINT16> m_protection2;
	UINT8 m_prot_val;
	UINT16 m_prot_16;
	UINT16 m_quiz365_protection[2];

	UINT16 m_mmpanic_leds;	/* A led for each of the 9 buttons */
	UINT8 m_funkyfig_lockout;
	UINT8 m_romdata[2];
	int m_palette_index;
	UINT8 m_hginga_rombank;
	UINT8 m_mjflove_irq_cause;
	UINT8 m_daimyojn_palette_sel;

	int m_irq_count;


	/* devices */
	device_t *m_maincpu;
	device_t *m_soundcpu;
	device_t *m_rtc;
	device_t *m_ymsnd;
	okim6295_device *m_oki;
	device_t *m_top_scr;
	device_t *m_bot_scr;
	DECLARE_WRITE8_MEMBER(dynax_vblank_ack_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_ack_w);
	DECLARE_WRITE8_MEMBER(jantouki_vblank_ack_w);
	DECLARE_WRITE8_MEMBER(jantouki_blitter_ack_w);
	DECLARE_WRITE8_MEMBER(jantouki_blitter2_ack_w);
	DECLARE_WRITE8_MEMBER(jantouki_sound_vblank_ack_w);
	DECLARE_WRITE8_MEMBER(dynax_coincounter_0_w);
	DECLARE_WRITE8_MEMBER(dynax_coincounter_1_w);
	DECLARE_READ8_MEMBER(ret_ff);
	DECLARE_READ8_MEMBER(hanamai_keyboard_0_r);
	DECLARE_READ8_MEMBER(hanamai_keyboard_1_r);
	DECLARE_WRITE8_MEMBER(hanamai_keyboard_w);
	DECLARE_WRITE8_MEMBER(dynax_rombank_w);
	DECLARE_WRITE8_MEMBER(jantouki_sound_rombank_w);
	DECLARE_WRITE8_MEMBER(hnoridur_rombank_w);
	DECLARE_WRITE8_MEMBER(hnoridur_palbank_w);
	DECLARE_WRITE8_MEMBER(hnoridur_palette_w);
	DECLARE_WRITE8_MEMBER(yarunara_palette_w);
	DECLARE_WRITE8_MEMBER(nanajign_palette_w);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_WRITE8_MEMBER(yarunara_layer_half_w);
	DECLARE_WRITE8_MEMBER(yarunara_layer_half2_w);
	DECLARE_WRITE8_MEMBER(hjingi_bank_w);
	DECLARE_WRITE8_MEMBER(hjingi_lockout_w);
	DECLARE_WRITE8_MEMBER(hjingi_hopper_w);
	DECLARE_READ8_MEMBER(hjingi_keyboard_0_r);
	DECLARE_READ8_MEMBER(hjingi_keyboard_1_r);
	DECLARE_WRITE8_MEMBER(yarunara_input_w);
	DECLARE_READ8_MEMBER(yarunara_input_r);
	DECLARE_WRITE8_MEMBER(yarunara_rombank_w);
	DECLARE_WRITE8_MEMBER(yarunara_flipscreen_w);
	DECLARE_WRITE8_MEMBER(yarunara_flipscreen_inv_w);
	DECLARE_WRITE8_MEMBER(yarunara_blit_romregion_w);
	DECLARE_READ8_MEMBER(jantouki_soundlatch_ack_r);
	DECLARE_WRITE8_MEMBER(jantouki_soundlatch_w);
	DECLARE_READ8_MEMBER(jantouki_blitter_busy_r);
	DECLARE_WRITE8_MEMBER(jantouki_rombank_w);
	DECLARE_WRITE8_MEMBER(jantouki_soundlatch_ack_w);
	DECLARE_READ8_MEMBER(jantouki_soundlatch_r);
	DECLARE_READ8_MEMBER(jantouki_soundlatch_status_r);
	DECLARE_READ8_MEMBER(mjelctrn_keyboard_1_r);
	DECLARE_READ8_MEMBER(mjelctrn_dsw_r);
	DECLARE_WRITE8_MEMBER(mjelctrn_blitter_ack_w);
	DECLARE_WRITE8_MEMBER(htengoku_select_w);
	DECLARE_WRITE8_MEMBER(htengoku_coin_w);
	DECLARE_READ8_MEMBER(htengoku_input_r);
	DECLARE_READ8_MEMBER(htengoku_coin_r);
	DECLARE_WRITE8_MEMBER(htengoku_rombank_w);
	DECLARE_WRITE8_MEMBER(htengoku_blit_romregion_w);
	DECLARE_WRITE8_MEMBER(tenkai_ipsel_w);
	DECLARE_WRITE8_MEMBER(tenkai_ip_w);
	DECLARE_READ8_MEMBER(tenkai_ip_r);
	DECLARE_READ8_MEMBER(tenkai_palette_r);
	DECLARE_WRITE8_MEMBER(tenkai_palette_w);
	DECLARE_READ8_MEMBER(tenkai_p3_r);
	DECLARE_WRITE8_MEMBER(tenkai_p3_w);
	DECLARE_WRITE8_MEMBER(tenkai_p4_w);
	DECLARE_READ8_MEMBER(tenkai_p5_r);
	DECLARE_WRITE8_MEMBER(tenkai_p6_w);
	DECLARE_WRITE8_MEMBER(tenkai_p7_w);
	DECLARE_WRITE8_MEMBER(tenkai_p8_w);
	DECLARE_READ8_MEMBER(tenkai_p8_r);
	DECLARE_READ8_MEMBER(tenkai_8000_r);
	DECLARE_WRITE8_MEMBER(tenkai_8000_w);
	DECLARE_WRITE8_MEMBER(tenkai_6c_w);
	DECLARE_WRITE8_MEMBER(tenkai_70_w);
	DECLARE_WRITE8_MEMBER(tenkai_blit_romregion_w);
	DECLARE_READ8_MEMBER(gekisha_keyboard_0_r);
	DECLARE_READ8_MEMBER(gekisha_keyboard_1_r);
	DECLARE_WRITE8_MEMBER(gekisha_hopper_w);
	DECLARE_WRITE8_MEMBER(gekisha_p4_w);
	DECLARE_READ8_MEMBER(gekisha_8000_r);
	DECLARE_WRITE8_MEMBER(gekisha_8000_w);
	DECLARE_WRITE8_MEMBER(dynax_extra_scrollx_w);
	DECLARE_WRITE8_MEMBER(dynax_extra_scrolly_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_pen_w);
	DECLARE_WRITE8_MEMBER(dynax_blit2_pen_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_dest_w);
	DECLARE_WRITE8_MEMBER(dynax_blit2_dest_w);
	DECLARE_WRITE8_MEMBER(tenkai_blit_dest_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_backpen_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_palette01_w);
	DECLARE_WRITE8_MEMBER(tenkai_blit_palette01_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_palette45_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_palette23_w);
	DECLARE_WRITE8_MEMBER(tenkai_blit_palette23_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_palette67_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_palbank_w);
	DECLARE_WRITE8_MEMBER(dynax_blit2_palbank_w);
	DECLARE_WRITE8_MEMBER(hanamai_layer_half_w);
	DECLARE_WRITE8_MEMBER(hnoridur_layer_half2_w);
	DECLARE_WRITE8_MEMBER(mjdialq2_blit_dest_w);
	DECLARE_WRITE8_MEMBER(dynax_layer_enable_w);
	DECLARE_WRITE8_MEMBER(jantouki_layer_enable_w);
	DECLARE_WRITE8_MEMBER(mjdialq2_layer_enable_w);
	DECLARE_WRITE8_MEMBER(dynax_flipscreen_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_romregion_w);
	DECLARE_WRITE8_MEMBER(dynax_blit2_romregion_w);
	DECLARE_WRITE8_MEMBER(dynax_blit_scroll_w);
	DECLARE_WRITE8_MEMBER(tenkai_blit_scroll_w);
	DECLARE_WRITE8_MEMBER(dynax_blit2_scroll_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_rev2_w);
	DECLARE_WRITE8_MEMBER(tenkai_blitter_rev2_w);
	DECLARE_WRITE8_MEMBER(jantouki_blitter_rev2_w);
	DECLARE_WRITE8_MEMBER(jantouki_blitter2_rev2_w);
	DECLARE_WRITE8_MEMBER(hanamai_priority_w);
	DECLARE_WRITE8_MEMBER(tenkai_priority_w);
	DECLARE_CUSTOM_INPUT_MEMBER(ddenlovr_special_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ddenlovj_blitter_r);
	DECLARE_CUSTOM_INPUT_MEMBER(nettoqc_special_r);
	DECLARE_CUSTOM_INPUT_MEMBER(mjflove_blitter_r);
	DECLARE_WRITE8_MEMBER(ddenlovr_bgcolor_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_bgcolor2_w);
	DECLARE_WRITE16_MEMBER(ddenlovr16_bgcolor_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_priority_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_priority2_w);
	DECLARE_WRITE16_MEMBER(ddenlovr16_priority_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_layer_enable_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_layer_enable2_w);
	DECLARE_WRITE16_MEMBER(ddenlovr16_layer_enable_w);
	DECLARE_WRITE8_MEMBER(hanakanz_blitter_reg_w);
	DECLARE_WRITE8_MEMBER(hanakanz_blitter_data_w);
	DECLARE_WRITE8_MEMBER(rongrong_blitter_w);
	DECLARE_WRITE16_MEMBER(ddenlovr_blitter_w);
	DECLARE_WRITE16_MEMBER(ddenlovr_blitter_irq_ack_w);
	DECLARE_READ8_MEMBER(rongrong_gfxrom_r);
	DECLARE_READ16_MEMBER(ddenlovr_gfxrom_r);
	DECLARE_WRITE16_MEMBER(ddenlovr_coincounter_0_w);
	DECLARE_WRITE16_MEMBER(ddenlovr_coincounter_1_w);
	DECLARE_WRITE8_MEMBER(rongrong_palette_w);
	DECLARE_WRITE16_MEMBER(ddenlovr_palette_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_palette_base_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_palette_base2_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_palette_mask_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_palette_mask2_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_transparency_pen_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_transparency_pen2_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_transparency_mask_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_transparency_mask2_w);
	DECLARE_WRITE16_MEMBER(ddenlovr16_palette_base_w);
	DECLARE_WRITE16_MEMBER(ddenlovr16_palette_mask_w);
	DECLARE_WRITE16_MEMBER(ddenlovr16_transparency_pen_w);
	DECLARE_WRITE16_MEMBER(ddenlovr16_transparency_mask_w);
	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_READ16_MEMBER(unk16_r);
	DECLARE_WRITE16_MEMBER(ddenlovr_select_16_w);
	DECLARE_WRITE8_MEMBER(ddenlovr_select2_w);
	DECLARE_WRITE16_MEMBER(ddenlovr_select2_16_w);
	DECLARE_READ8_MEMBER(rongrong_input2_r);
	DECLARE_READ16_MEMBER(quiz365_input2_r);
	DECLARE_WRITE8_MEMBER(rongrong_blitter_busy_w);
	DECLARE_READ8_MEMBER(rongrong_blitter_busy_r);
	DECLARE_WRITE16_MEMBER(quiz365_coincounter_w);
	DECLARE_READ16_MEMBER(quiz365_protection_r);
	DECLARE_WRITE16_MEMBER(quiz365_protection_w);
	DECLARE_READ16_MEMBER(ddenlovj_dsw_r);
	DECLARE_WRITE16_MEMBER(ddenlovj_coincounter_w);
	DECLARE_READ16_MEMBER(ddenlovrk_protection1_r);
	DECLARE_READ16_MEMBER(ddenlovrk_protection2_r);
	DECLARE_WRITE16_MEMBER(ddenlovrk_protection2_w);
	DECLARE_READ16_MEMBER(nettoqc_input_r);
	DECLARE_READ16_MEMBER(nettoqc_protection_r);
	DECLARE_WRITE16_MEMBER(nettoqc_coincounter_w);
	DECLARE_READ8_MEMBER(rongrong_input_r);
	DECLARE_WRITE8_MEMBER(rongrong_select_w);
	DECLARE_READ8_MEMBER(magic_r);
	DECLARE_WRITE8_MEMBER(mmpanic_rombank_w);
	DECLARE_WRITE8_MEMBER(mmpanic_soundlatch_w);
	DECLARE_WRITE8_MEMBER(mmpanic_blitter_w);
	DECLARE_WRITE8_MEMBER(mmpanic_blitter2_w);
	DECLARE_WRITE8_MEMBER(mmpanic_leds_w);
	DECLARE_WRITE8_MEMBER(mmpanic_leds2_w);
	DECLARE_WRITE8_MEMBER(mmpanic_lockout_w);
	DECLARE_READ8_MEMBER(mmpanic_link_r);
	DECLARE_READ8_MEMBER(funkyfig_busy_r);
	DECLARE_WRITE8_MEMBER(funkyfig_blitter_w);
	DECLARE_WRITE8_MEMBER(funkyfig_rombank_w);
	DECLARE_READ8_MEMBER(funkyfig_dsw_r);
	DECLARE_READ8_MEMBER(funkyfig_coin_r);
	DECLARE_READ8_MEMBER(funkyfig_key_r);
	DECLARE_WRITE8_MEMBER(funkyfig_lockout_w);
	DECLARE_WRITE8_MEMBER(hanakanz_rombank_w);
	DECLARE_WRITE8_MEMBER(hanakanz_keyb_w);
	DECLARE_WRITE8_MEMBER(hanakanz_dsw_w);
	DECLARE_READ8_MEMBER(hanakanz_keyb_r);
	DECLARE_READ8_MEMBER(hanakanz_dsw_r);
	DECLARE_READ8_MEMBER(hanakanz_busy_r);
	DECLARE_READ8_MEMBER(hanakanz_gfxrom_r);
	DECLARE_WRITE8_MEMBER(hanakanz_coincounter_w);
	DECLARE_WRITE8_MEMBER(hanakanz_palette_w);
	DECLARE_READ8_MEMBER(hanakanz_rand_r);
	DECLARE_WRITE8_MEMBER(mjreach1_protection_w);
	DECLARE_READ8_MEMBER(mjreach1_protection_r);
	DECLARE_READ8_MEMBER(mjchuuka_keyb_r);
	DECLARE_WRITE8_MEMBER(mjchuuka_blitter_w);
	DECLARE_READ8_MEMBER(mjchuuka_gfxrom_0_r);
	DECLARE_READ8_MEMBER(mjchuuka_gfxrom_1_r);
	DECLARE_WRITE8_MEMBER(mjchuuka_palette_w);
	DECLARE_WRITE8_MEMBER(mjchuuka_coincounter_w);
	DECLARE_WRITE8_MEMBER(mjmyster_rambank_w);
	DECLARE_WRITE8_MEMBER(mjmyster_select2_w);
	DECLARE_READ8_MEMBER(mjmyster_coins_r);
	DECLARE_READ8_MEMBER(mjmyster_keyb_r);
	DECLARE_READ8_MEMBER(mjmyster_dsw_r);
	DECLARE_WRITE8_MEMBER(mjmyster_coincounter_w);
	DECLARE_WRITE8_MEMBER(mjmyster_blitter_w);
	DECLARE_WRITE8_MEMBER(hginga_rombank_w);
	DECLARE_READ8_MEMBER(hginga_protection_r);
	DECLARE_WRITE8_MEMBER(hginga_input_w);
	DECLARE_READ8_MEMBER(hginga_coins_r);
	DECLARE_WRITE8_MEMBER(hginga_80_w);
	DECLARE_WRITE8_MEMBER(hginga_coins_w);
	DECLARE_READ8_MEMBER(hginga_input_r);
	DECLARE_WRITE8_MEMBER(hginga_blitter_w);
	DECLARE_WRITE8_MEMBER(hgokou_dsw_sel_w);
	DECLARE_READ8_MEMBER(hgokou_input_r);
	DECLARE_WRITE8_MEMBER(hgokou_input_w);
	DECLARE_READ8_MEMBER(hgokou_protection_r);
	DECLARE_READ8_MEMBER(hgokbang_input_r);
	DECLARE_WRITE8_MEMBER(hparadis_select_w);
	DECLARE_READ8_MEMBER(hparadis_input_r);
	DECLARE_READ8_MEMBER(hparadis_dsw_r);
	DECLARE_WRITE8_MEMBER(hparadis_coin_w);
	DECLARE_READ8_MEMBER(mjmywrld_coins_r);
	DECLARE_READ16_MEMBER(akamaru_protection1_r);
	DECLARE_WRITE16_MEMBER(akamaru_protection1_w);
	DECLARE_READ16_MEMBER(akamaru_protection2_r);
	DECLARE_READ16_MEMBER(akamaru_dsw_r);
	DECLARE_READ16_MEMBER(akamaru_blitter_r);
	DECLARE_READ16_MEMBER(akamaru_e0010d_r);
	DECLARE_WRITE8_MEMBER(mjflove_rombank_w);
	DECLARE_READ8_MEMBER(mjflove_protection_r);
	DECLARE_READ8_MEMBER(mjflove_keyb_r);
	DECLARE_WRITE8_MEMBER(mjflove_blitter_w);
	DECLARE_WRITE8_MEMBER(mjflove_coincounter_w);
	DECLARE_WRITE8_MEMBER(jongtei_dsw_keyb_w);
	DECLARE_READ8_MEMBER(jongtei_busy_r);
	DECLARE_READ8_MEMBER(sryudens_keyb_r);
	DECLARE_WRITE8_MEMBER(sryudens_coincounter_w);
	DECLARE_WRITE8_MEMBER(sryudens_rambank_w);
	DECLARE_READ8_MEMBER(daimyojn_keyb1_r);
	DECLARE_READ8_MEMBER(daimyojn_keyb2_r);
	DECLARE_WRITE8_MEMBER(daimyojn_protection_w);
	DECLARE_READ8_MEMBER(daimyojn_protection_r);
	DECLARE_READ8_MEMBER(momotaro_protection_r);
	DECLARE_WRITE8_MEMBER(daimyojn_palette_sel_w);
	DECLARE_WRITE8_MEMBER(daimyojn_blitter_data_palette_w);
	DECLARE_READ8_MEMBER(daimyojn_year_hack_r);
};

//----------- defined in drivers/dynax.c -----------

void sprtmtch_update_irq(running_machine &machine);
void jantouki_update_irq(running_machine &machine);
void mjelctrn_update_irq(running_machine &machine);
void neruton_update_irq(running_machine &machine);

//----------- defined in video/dynax.c -----------
VIDEO_START( hanamai );
VIDEO_START( hnoridur );
VIDEO_START( mcnpshnt );
VIDEO_START( sprtmtch );
VIDEO_START( mjdialq2 );
VIDEO_START( jantouki );
VIDEO_START( mjelctrn );
VIDEO_START( neruton );
VIDEO_START( htengoku );

SCREEN_UPDATE_IND16( hanamai );
SCREEN_UPDATE_IND16( hnoridur );
SCREEN_UPDATE_IND16( sprtmtch );
SCREEN_UPDATE_IND16( mjdialq2 );
SCREEN_UPDATE_IND16( jantouki_top );
SCREEN_UPDATE_IND16( jantouki_bottom );
SCREEN_UPDATE_IND16( htengoku );

PALETTE_INIT( sprtmtch );


//----------- defined in drivers/ddenlovr.c -----------

VIDEO_START(ddenlovr);
SCREEN_UPDATE_IND16(ddenlovr);
