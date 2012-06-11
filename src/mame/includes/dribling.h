/*************************************************************************

    Model Racing Dribbling hardware

*************************************************************************/

#include "machine/i8255.h"


class dribling_state : public driver_device
{
public:
	dribling_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_ppi8255_0(*this, "ppi8255_0"),
		m_ppi8255_1(*this, "ppi8255_1"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<i8255_device>  m_ppi8255_0;
	optional_device<i8255_device>  m_ppi8255_1;
	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* misc */
	UINT8    m_abca;
	UINT8    m_dr;
	UINT8    m_ds;
	UINT8    m_sh;
	UINT8    m_input_mux;
	UINT8    m_di;

	DECLARE_READ8_MEMBER(ioread);
	DECLARE_WRITE8_MEMBER(iowrite);
	DECLARE_WRITE8_MEMBER(dribling_colorram_w);
};


/*----------- defined in video/dribling.c -----------*/

PALETTE_INIT( dribling );
SCREEN_UPDATE_IND16( dribling );
