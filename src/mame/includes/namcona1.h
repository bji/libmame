enum
{
	NAMCO_CGANGPZL,
	NAMCO_EMERALDA,
	NAMCO_KNCKHEAD,
	NAMCO_BKRTMAQ,
	NAMCO_EXBANIA,
	NAMCO_QUIZTOU,
	NAMCO_SWCOURT,
	NAMCO_TINKLPIT,
	NAMCO_NUMANATH,
	NAMCO_FA,
	NAMCO_XDAY2
};

#define NA1_NVRAM_SIZE (0x800)
#define NAMCONA1_NUM_TILEMAPS 4

class namcona1_state : public driver_device
{
public:
	namcona1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_mcu(*this,"mcu"),
		m_videoram(*this,"videoram"),
		m_spriteram(*this,"spriteram"),
		m_workram(*this,"workram"),
		m_vreg(*this,"vreg"),
		m_scroll(*this,"scroll")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_workram;
	required_shared_ptr<UINT16> m_vreg;
	required_shared_ptr<UINT16> m_scroll;
	UINT16 *m_mpBank0;
	UINT16 *m_mpBank1;
	int m_mEnableInterrupts;
	int m_gametype;
	UINT8 m_nvmem[NA1_NVRAM_SIZE];
	UINT16 m_count;
	UINT32 m_keyval;
	UINT16 m_mcu_mailbox[8];
	UINT8 m_mcu_port4;
	UINT8 m_mcu_port5;
	UINT8 m_mcu_port6;
	UINT8 m_mcu_port8;
	UINT16 *m_shaperam;
	UINT16 *m_cgram;
	tilemap_t *m_roz_tilemap;
	int m_roz_palette;
	tilemap_t *m_bg_tilemap[NAMCONA1_NUM_TILEMAPS];
	int m_tilemap_palette_bank[NAMCONA1_NUM_TILEMAPS];
	int m_palette_is_dirty;
	UINT8 m_mask_data[8];
	UINT8 m_conv_data[9];

	DECLARE_READ16_MEMBER(namcona1_nvram_r);
	DECLARE_WRITE16_MEMBER(namcona1_nvram_w);
	DECLARE_READ16_MEMBER(custom_key_r);
	DECLARE_WRITE16_MEMBER(custom_key_w);
	DECLARE_READ16_MEMBER(namcona1_vreg_r);
	DECLARE_WRITE16_MEMBER(namcona1_vreg_w);
	DECLARE_READ16_MEMBER(mcu_mailbox_r);
	DECLARE_WRITE16_MEMBER(mcu_mailbox_w_68k);
	DECLARE_WRITE16_MEMBER(mcu_mailbox_w_mcu);
	DECLARE_READ16_MEMBER(na1mcu_shared_r);
	DECLARE_WRITE16_MEMBER(na1mcu_shared_w);
	DECLARE_READ8_MEMBER(port4_r);
	DECLARE_WRITE8_MEMBER(port4_w);
	DECLARE_READ8_MEMBER(port5_r);
	DECLARE_WRITE8_MEMBER(port5_w);
	DECLARE_READ8_MEMBER(port6_r);
	DECLARE_WRITE8_MEMBER(port6_w);
	DECLARE_READ8_MEMBER(port7_r);
	DECLARE_WRITE8_MEMBER(port7_w);
	DECLARE_READ8_MEMBER(port8_r);
	DECLARE_WRITE8_MEMBER(port8_w);
	DECLARE_READ8_MEMBER(portana_r);
	void write_version_info();
	DECLARE_WRITE16_MEMBER(namcona1_videoram_w);
	DECLARE_READ16_MEMBER(namcona1_videoram_r);
	DECLARE_READ16_MEMBER(namcona1_paletteram_r);
	DECLARE_WRITE16_MEMBER(namcona1_paletteram_w);
	DECLARE_READ16_MEMBER(namcona1_gfxram_r);
	DECLARE_WRITE16_MEMBER(namcona1_gfxram_w);
};


/*----------- defined in video/namcona1.c -----------*/





extern SCREEN_UPDATE_IND16( namcona1 );
extern VIDEO_START( namcona1 );
