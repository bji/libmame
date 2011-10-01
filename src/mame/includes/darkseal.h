#include "video/deco16ic.h"

class darkseal_state : public driver_device
{
public:
	darkseal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	      m_deco_tilegen1(*this, "tilegen1"),
		  m_deco_tilegen2(*this, "tilegen2") { }

	UINT16 *m_ram;
	UINT16 *m_pf1_rowscroll;
	//UINT16 *m_pf2_rowscroll;
	UINT16 *m_pf3_rowscroll;
	//UINT16 *m_pf4_rowscroll;

	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;

	int m_flipscreen;
};


/*----------- defined in video/darkseal.c -----------*/

VIDEO_START( darkseal );
SCREEN_UPDATE( darkseal );

WRITE16_HANDLER( darkseal_palette_24bit_rg_w );
WRITE16_HANDLER( darkseal_palette_24bit_b_w );
