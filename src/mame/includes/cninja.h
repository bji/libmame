/*************************************************************************

    Caveman Ninja (and other DECO 16bit titles)

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"

class cninja_state : public driver_device
{
public:
	cninja_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_audiocpu(*this, "audiocpu"),
		  m_decocomn(*this, "deco_common"),
		  m_deco_tilegen1(*this, "tilegen1"),
		  m_deco_tilegen2(*this, "tilegen2"),
		  m_raster_irq_timer(*this, "raster_timer"),
		  m_oki2(*this, "oki2") { }

	/* memory pointers */
	UINT16 *   m_ram;
	UINT16 *   m_pf1_rowscroll;
	UINT16 *   m_pf2_rowscroll;
	UINT16 *   m_pf3_rowscroll;
	UINT16 *   m_pf4_rowscroll;

	/* misc */
	int        m_scanline;
	int        m_irq_mask;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<decocomn_device> m_decocomn;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;
	optional_device<timer_device> m_raster_irq_timer;
	optional_device<okim6295_device> m_oki2;
};

/*----------- defined in video/cninja.c -----------*/

VIDEO_START( stoneage );
VIDEO_START( mutantf );

SCREEN_UPDATE( cninja );
SCREEN_UPDATE( cninjabl );
SCREEN_UPDATE( edrandy );
SCREEN_UPDATE( robocop2 );
SCREEN_UPDATE( mutantf );
