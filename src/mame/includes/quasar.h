/***************************************************************************

    Zaccaria Quasar

****************************************************************************/

#include "includes/cvs.h"

class quasar_state : public cvs_state
{
public:
	quasar_state(const machine_config &mconfig, device_type type, const char *tag)
		: cvs_state(mconfig, type, tag) { }

	UINT8 *    m_effectram;
	UINT8      m_effectcontrol;
	UINT8      m_page;
	UINT8      m_io_page;
	DECLARE_WRITE8_MEMBER(video_page_select_w);
	DECLARE_WRITE8_MEMBER(io_page_select_w);
	DECLARE_WRITE8_MEMBER(quasar_video_w);
	DECLARE_READ8_MEMBER(quasar_IO_r);
	DECLARE_WRITE8_MEMBER(quasar_bullet_w);
	DECLARE_WRITE8_MEMBER(quasar_sh_command_w);
	DECLARE_READ8_MEMBER(quasar_sh_command_r);
	DECLARE_READ8_MEMBER(audio_t1_r);
};


/*----------- defined in video/quasar.c -----------*/

PALETTE_INIT( quasar );
SCREEN_UPDATE_IND16( quasar );
VIDEO_START( quasar );
