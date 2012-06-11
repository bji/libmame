/*************************************************************************

    Atari G42 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarig42_state : public atarigen_state
{
public:
	atarig42_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT16			m_playfield_base;

	UINT16			m_current_control;
	UINT8			m_playfield_tile_bank;
	UINT8			m_playfield_color_bank;
	UINT16			m_playfield_xscroll;
	UINT16			m_playfield_yscroll;

	UINT8			m_analog_data;
	UINT16 *		m_mo_command;

	int 			m_sloop_bank;
	int 			m_sloop_next_bank;
	int 			m_sloop_offset;
	int 			m_sloop_state;
	UINT16 *		m_sloop_base;

	device_t *		m_rle;
	UINT32			m_last_accesses[8];
};


/*----------- defined in video/atarig42.c -----------*/

VIDEO_START( atarig42 );
SCREEN_VBLANK( atarig42 );
SCREEN_UPDATE_IND16( atarig42 );

WRITE16_HANDLER( atarig42_mo_control_w );

void atarig42_scanline_update(screen_device &screen, int scanline);

