class tetrisp2_state : public driver_device
{
public:
	tetrisp2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram"),
		  m_spriteram2(*this, "spriteram2") ,
		m_vram_fg(*this, "vram_fg"),
		m_vram_bg(*this, "vram_bg"),
		m_vram_rot(*this, "vram_rot"),
		m_nvram(*this, "nvram"),
		m_scroll_fg(*this, "scroll_fg"),
		m_scroll_bg(*this, "scroll_bg"),
		m_rotregs(*this, "rotregs"),
		m_rocknms_sub_priority(*this, "sub_priority"),
		m_rocknms_sub_vram_rot(*this, "sub_vram_rot"),
		m_rocknms_sub_vram_fg(*this, "sub_vram_fg"),
		m_rocknms_sub_vram_bg(*this, "sub_vram_bg"),
		m_rocknms_sub_scroll_fg(*this, "sub_scroll_fg"),
		m_rocknms_sub_scroll_bg(*this, "sub_scroll_bg"),
		m_rocknms_sub_rotregs(*this, "sub_rotregs"){ }

	required_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_spriteram2;

	UINT16 m_systemregs[0x10];
	required_shared_ptr<UINT16> m_vram_fg;
	required_shared_ptr<UINT16> m_vram_bg;
	required_shared_ptr<UINT16> m_vram_rot;
	required_shared_ptr<UINT16> m_nvram;
	required_shared_ptr<UINT16> m_scroll_fg;
	required_shared_ptr<UINT16> m_scroll_bg;
	required_shared_ptr<UINT16> m_rotregs;
	UINT8 *m_priority;
	optional_shared_ptr<UINT16> m_rocknms_sub_priority;
	optional_shared_ptr<UINT16> m_rocknms_sub_vram_rot;
	optional_shared_ptr<UINT16> m_rocknms_sub_vram_fg;
	optional_shared_ptr<UINT16> m_rocknms_sub_vram_bg;
	optional_shared_ptr<UINT16> m_rocknms_sub_scroll_fg;
	optional_shared_ptr<UINT16> m_rocknms_sub_scroll_bg;
	optional_shared_ptr<UINT16> m_rocknms_sub_rotregs;

	UINT16 m_rocknms_sub_systemregs[0x10];
	UINT16 m_rockn_protectdata;
	UINT16 m_rockn_adpcmbank;
	UINT16 m_rockn_soundvolume;
	emu_timer *m_rockn_timer_l4;
	emu_timer *m_rockn_timer_sub_l4;
	int m_bank_lo;
	int m_bank_hi;
	UINT16 m_rocknms_main2sub;
	UINT16 m_rocknms_sub2main;
	int m_flipscreen_old;
	tilemap_t *m_tilemap_bg;
	tilemap_t *m_tilemap_fg;
	tilemap_t *m_tilemap_rot;
	tilemap_t *m_tilemap_sub_bg;
	tilemap_t *m_tilemap_sub_fg;
	tilemap_t *m_tilemap_sub_rot;
	DECLARE_WRITE16_MEMBER(rockn_systemregs_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_systemregs_w);
	DECLARE_READ16_MEMBER(rockn_adpcmbank_r);
	DECLARE_WRITE16_MEMBER(rockn_adpcmbank_w);
	DECLARE_WRITE16_MEMBER(rockn2_adpcmbank_w);
	DECLARE_READ16_MEMBER(rockn_soundvolume_r);
	DECLARE_WRITE16_MEMBER(rockn_soundvolume_w);
	DECLARE_WRITE16_MEMBER(nndmseal_sound_bank_w);
	DECLARE_READ16_MEMBER(tetrisp2_ip_1_word_r);
	DECLARE_READ16_MEMBER(rockn_nvram_r);
	DECLARE_READ16_MEMBER(rocknms_main2sub_r);
	DECLARE_WRITE16_MEMBER(rocknms_main2sub_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub2main_w);
	DECLARE_WRITE16_MEMBER(tetrisp2_coincounter_w);
	DECLARE_WRITE16_MEMBER(nndmseal_coincounter_w);
	DECLARE_WRITE16_MEMBER(nndmseal_b20000_w);
	DECLARE_WRITE16_MEMBER(tetrisp2_systemregs_w);
	DECLARE_READ16_MEMBER(tetrisp2_nvram_r);
	DECLARE_WRITE16_MEMBER(tetrisp2_nvram_w);
	DECLARE_WRITE16_MEMBER(tetrisp2_palette_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_palette_w);
	DECLARE_WRITE8_MEMBER(tetrisp2_priority_w);
	DECLARE_WRITE8_MEMBER(rockn_priority_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_priority_w);
	DECLARE_READ16_MEMBER(nndmseal_priority_r);
	DECLARE_READ8_MEMBER(tetrisp2_priority_r);
	DECLARE_WRITE16_MEMBER(tetrisp2_vram_bg_w);
	DECLARE_WRITE16_MEMBER(tetrisp2_vram_fg_w);
	DECLARE_WRITE16_MEMBER(tetrisp2_vram_rot_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_vram_bg_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_vram_fg_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_vram_rot_w);
	DECLARE_CUSTOM_INPUT_MEMBER(rocknms_main2sub_status_r);
};

class stepstag_state : public tetrisp2_state
{
public:
	stepstag_state(const machine_config &mconfig, device_type type, const char *tag)
		: tetrisp2_state(mconfig, type, tag),
		  m_spriteram3(*this, "spriteram3") { }

	required_shared_ptr<UINT16> m_spriteram3;
	DECLARE_READ16_MEMBER(stepstag_coins_r);
	DECLARE_READ16_MEMBER(unknown_read_0xc00000);
	DECLARE_READ16_MEMBER(unknown_read_0xffff00);
	DECLARE_READ16_MEMBER(unk_a42000_r);
	DECLARE_WRITE16_MEMBER(stepstag_soundlatch_word_w);
	DECLARE_WRITE16_MEMBER(stepstag_leds_w);
	DECLARE_WRITE16_MEMBER( stepstag_palette_w );
};

/*----------- defined in video/tetrisp2.c -----------*/




VIDEO_START( tetrisp2 );
SCREEN_UPDATE_IND16( tetrisp2 );

VIDEO_START( rockntread );
SCREEN_UPDATE_IND16( rockntread );

VIDEO_START( rocknms );
SCREEN_UPDATE_RGB32( rocknms_left );
SCREEN_UPDATE_RGB32( rocknms_right );

VIDEO_START( nndmseal );

VIDEO_START( stepstag );
SCREEN_UPDATE_IND16( stepstag_left );
SCREEN_UPDATE_IND16( stepstag_mid );
SCREEN_UPDATE_IND16( stepstag_right );
