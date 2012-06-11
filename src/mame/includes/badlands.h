/*************************************************************************

    Atari Bad Lands hardware

*************************************************************************/

#include "machine/atarigen.h"

class badlands_state : public atarigen_state
{
public:
	badlands_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT8			m_pedal_value[2];

	UINT8 *			m_bank_base;
	UINT8 *			m_bank_source_data;

	UINT8			m_playfield_tile_bank;
	DECLARE_READ16_MEMBER(sound_busy_r);
	DECLARE_READ16_MEMBER(pedal_0_r);
	DECLARE_READ16_MEMBER(pedal_1_r);
	DECLARE_READ8_MEMBER(audio_io_r);
	DECLARE_WRITE8_MEMBER(audio_io_w);
	DECLARE_READ16_MEMBER(badlandsb_unk_r);
};


/*----------- defined in video/badlands.c -----------*/

WRITE16_HANDLER( badlands_pf_bank_w );

VIDEO_START( badlands );
SCREEN_UPDATE_IND16( badlands );
