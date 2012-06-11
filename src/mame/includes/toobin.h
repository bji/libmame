/*************************************************************************

    Atari Toobin' hardware

*************************************************************************/

#include "machine/atarigen.h"

class toobin_state : public atarigen_state
{
public:
	toobin_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
		  m_interrupt_scan(*this, "interrupt_scan") { }

	required_shared_ptr<UINT16> m_interrupt_scan;

	double			m_brightness;
	bitmap_ind16 m_pfbitmap;
	DECLARE_WRITE16_MEMBER(interrupt_scan_w);
	DECLARE_READ16_MEMBER(special_port1_r);
};


/*----------- defined in video/toobin.c -----------*/

WRITE16_HANDLER( toobin_paletteram_w );
WRITE16_HANDLER( toobin_intensity_w );
WRITE16_HANDLER( toobin_xscroll_w );
WRITE16_HANDLER( toobin_yscroll_w );
WRITE16_HANDLER( toobin_slip_w );

VIDEO_START( toobin );
SCREEN_UPDATE_RGB32( toobin );
