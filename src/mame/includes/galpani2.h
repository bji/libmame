#include "includes/kaneko16.h"

class galpani2_state : public kaneko16_state
{
public:
	galpani2_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag),
		m_bg8(*this, "bg8"),
		m_palette(*this, "palette"),
		m_bg8_scrollx(*this, "bg8_scrollx"),
		m_bg8_scrolly(*this, "bg8_scrolly"),
		m_bg15(*this, "bg15"),
		m_ram(*this, "ram"),
		m_ram2(*this, "ram2"),
		m_rombank(*this, "rombank"),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this,"sub")
		{ }

	required_shared_ptr_array<UINT16, 2> m_bg8;
	optional_shared_ptr_array<UINT16, 2> m_palette;
	required_shared_ptr_array<UINT16, 2> m_bg8_scrollx;
	required_shared_ptr_array<UINT16, 2> m_bg8_scrolly;
	required_shared_ptr<UINT16> m_bg15;
	UINT16 m_eeprom_word;
	required_shared_ptr<UINT16> m_ram;
	required_shared_ptr<UINT16> m_ram2;
	UINT16 m_old_mcu_nmi1;
	UINT16 m_old_mcu_nmi2;
	required_shared_ptr<UINT16> m_rombank;
	bitmap_ind16 *m_bg8_bitmap[2];
	bitmap_ind16 *m_bg15_bitmap;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	DECLARE_WRITE8_MEMBER(galpani2_mcu_init_w);
	DECLARE_WRITE8_MEMBER(galpani2_mcu_nmi1_w);
	DECLARE_WRITE8_MEMBER(galpani2_mcu_nmi2_w);
	DECLARE_WRITE8_MEMBER(galpani2_coin_lockout_w);
	DECLARE_READ16_MEMBER(galpani2_bankedrom_r);
};


/*----------- defined in video/galpani2.c -----------*/

PALETTE_INIT( galpani2 );
VIDEO_START( galpani2 );
SCREEN_UPDATE_IND16( galpani2 );

WRITE16_HANDLER( galpani2_palette_0_w );
WRITE16_HANDLER( galpani2_palette_1_w );

WRITE16_HANDLER( galpani2_bg8_0_w );
WRITE16_HANDLER( galpani2_bg8_1_w );

WRITE16_HANDLER( galpani2_bg15_w );
