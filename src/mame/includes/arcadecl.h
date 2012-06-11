/*************************************************************************

    Atari Arcade Classics hardware (prototypes)

*************************************************************************/


#include "machine/atarigen.h"

class arcadecl_state : public atarigen_state
{
public:
	arcadecl_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT16 *		m_bitmap;
	UINT8			m_has_mo;
};

/*----------- defined in video/arcadecl.c -----------*/

VIDEO_START( arcadecl );
SCREEN_UPDATE_IND16( arcadecl );
