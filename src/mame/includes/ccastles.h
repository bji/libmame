/*************************************************************************

    Atari Crystal Castles hardware

*************************************************************************/

#include "cpu/m6502/m6502.h"
#include "machine/x2212.h"

class ccastles_state : public driver_device
{
public:
	ccastles_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_nvram_4b(*this, "nvram_4b"),
		  m_nvram_4a(*this, "nvram_4a") ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"){ }

	/* devices */
	required_device<m6502_device> m_maincpu;
	required_device<x2212_device> m_nvram_4b;
	required_device<x2212_device> m_nvram_4a;
	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	const UINT8 *m_syncprom;
	const UINT8 *m_wpprom;
	const UINT8 *m_priprom;
	bitmap_ind16 m_spritebitmap;
	double m_rweights[3];
	double m_gweights[3];
	double m_bweights[3];
	UINT8 m_video_control[8];
	UINT8 m_bitmode_addr[2];
	UINT8 m_hscroll;
	UINT8 m_vscroll;

	/* misc */
	int      m_vblank_start;
	int      m_vblank_end;
	emu_timer *m_irq_timer;
	UINT8    m_irq_state;
	UINT8    m_nvram_store[2];

	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_WRITE8_MEMBER(ccounter_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ8_MEMBER(leta_r);
	DECLARE_WRITE8_MEMBER(nvram_recall_w);
	DECLARE_WRITE8_MEMBER(nvram_store_w);
	DECLARE_READ8_MEMBER(nvram_r);
	DECLARE_WRITE8_MEMBER(nvram_w);
	DECLARE_WRITE8_MEMBER(ccastles_hscroll_w);
	DECLARE_WRITE8_MEMBER(ccastles_vscroll_w);
	DECLARE_WRITE8_MEMBER(ccastles_video_control_w);
	DECLARE_WRITE8_MEMBER(ccastles_paletteram_w);
	DECLARE_WRITE8_MEMBER(ccastles_videoram_w);
	DECLARE_READ8_MEMBER(ccastles_bitmode_r);
	DECLARE_WRITE8_MEMBER(ccastles_bitmode_w);
	DECLARE_WRITE8_MEMBER(ccastles_bitmode_addr_w);
	DECLARE_CUSTOM_INPUT_MEMBER(get_vblank);
};


/*----------- defined in video/ccastles.c -----------*/


VIDEO_START( ccastles );
SCREEN_UPDATE_IND16( ccastles );



