class seta2_state : public driver_device
{
public:
	seta2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this,"maincpu"),
		  m_nvram(*this, "nvram") ,
		m_spriteram(*this, "spriteram", 0),
		m_vregs(*this, "vregs", 0),
		m_coldfire_regs(*this, "coldfire_regs"),
		m_funcube_outputs(*this, "funcube_outputs"),
		m_funcube_leds(*this, "funcube_leds"){ }

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<UINT16> m_nvram;

	optional_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_vregs;
	optional_shared_ptr<UINT32> m_coldfire_regs;

	optional_shared_ptr<UINT8> m_funcube_outputs;
	optional_shared_ptr<UINT8> m_funcube_leds;
	int m_xoffset;
	int m_yoffset;
	int m_keyboard_row;

	UINT16 *m_buffered_spriteram;

	UINT64 m_funcube_coin_start_cycles;
	UINT8 m_funcube_hopper_motor;
	UINT8 m_funcube_press;

	UINT8 m_funcube_serial_fifo[4];
	UINT8 m_funcube_serial_count;

	DECLARE_WRITE16_MEMBER(seta2_vregs_w);
	DECLARE_WRITE16_MEMBER(seta2_sound_bank_w);
	DECLARE_WRITE16_MEMBER(grdians_lockout_w);
	DECLARE_READ16_MEMBER(mj4simai_p1_r);
	DECLARE_READ16_MEMBER(mj4simai_p2_r);
	DECLARE_WRITE16_MEMBER(mj4simai_keyboard_w);
	DECLARE_READ16_MEMBER(pzlbowl_protection_r);
	DECLARE_READ16_MEMBER(pzlbowl_coins_r);
	DECLARE_WRITE16_MEMBER(pzlbowl_coin_counter_w);
	DECLARE_WRITE16_MEMBER(reelquak_leds_w);
	DECLARE_WRITE16_MEMBER(reelquak_coin_w);
	DECLARE_WRITE16_MEMBER(samshoot_coin_w);
	DECLARE_READ32_MEMBER(funcube_nvram_dword_r);
	DECLARE_WRITE32_MEMBER(funcube_nvram_dword_w);
	DECLARE_WRITE16_MEMBER(spriteram16_word_w);
	DECLARE_READ16_MEMBER(spriteram16_word_r);
	DECLARE_WRITE32_MEMBER(coldfire_regs_w);
	DECLARE_READ32_MEMBER(coldfire_regs_r);
	DECLARE_READ32_MEMBER(funcube_debug_r);
	DECLARE_READ8_MEMBER(funcube_coins_r);
	DECLARE_READ8_MEMBER(funcube_serial_r);
	DECLARE_WRITE8_MEMBER(funcube_leds_w);
	DECLARE_READ8_MEMBER(funcube_outputs_r);
	DECLARE_WRITE8_MEMBER(funcube_outputs_w);
};

/*----------- defined in video/seta2.c -----------*/


VIDEO_START( seta2 );
VIDEO_START( seta2_xoffset );
VIDEO_START( seta2_yoffset );
SCREEN_UPDATE_IND16( seta2 );
SCREEN_VBLANK( seta2 );
