/*************************************************************************

    Atari Cloud 9 (prototype) hardware

*************************************************************************/

#include "cpu/m6502/m6502.h"
#include "machine/x2212.h"

class cloud9_state : public driver_device
{
public:
	cloud9_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_maincpu(*this, "maincpu"),
		  m_nvram(*this, "nvram") { }

	/* memory pointers */
	UINT8 *     m_videoram;
	UINT8 *     m_spriteram;
	UINT8 *     m_paletteram;

	/* video-related */
	const UINT8 *m_syncprom;
	const UINT8 *m_wpprom;
	const UINT8 *m_priprom;
	bitmap_t    *m_spritebitmap;
	double      m_rweights[3];
	double		m_gweights[3];
	double		m_bweights[3];
	UINT8       m_video_control[8];
	UINT8       m_bitmode_addr[2];

	/* misc */
	int         m_vblank_start;
	int         m_vblank_end;
	emu_timer   *m_irq_timer;
	UINT8       m_irq_state;

	/* devices */
	required_device<m6502_device> m_maincpu;
	required_device<x2212_device> m_nvram;
};


/*----------- defined in video/cloud9.c -----------*/

VIDEO_START( cloud9 );
SCREEN_UPDATE( cloud9 );

WRITE8_HANDLER( cloud9_video_control_w );

WRITE8_HANDLER( cloud9_paletteram_w );
WRITE8_HANDLER( cloud9_videoram_w );

READ8_HANDLER( cloud9_bitmode_r );
WRITE8_HANDLER( cloud9_bitmode_w );
WRITE8_HANDLER( cloud9_bitmode_addr_w );
