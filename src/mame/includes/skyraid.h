#include "sound/discrete.h"

class skyraid_state : public driver_device
{
public:
	skyraid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_analog_range;
	int m_analog_offset;

	int m_scroll;

	UINT8* m_alpha_num_ram;
	UINT8* m_pos_ram;
	UINT8* m_obj_ram;

	bitmap_ind16 m_helper;
};


/*----------- defined in audio/skyraid.c -----------*/

DISCRETE_SOUND_EXTERN( skyraid );

WRITE8_DEVICE_HANDLER( skyraid_sound_w );


/*----------- defined in video/skyraid.c -----------*/

VIDEO_START(skyraid);
SCREEN_UPDATE_IND16(skyraid);
