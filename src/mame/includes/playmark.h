#include "sound/okim6295.h"
#include "machine/eeprom.h"

class playmark_state : public driver_device
{
public:
	playmark_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *     m_bgvideoram;
	UINT16 *     m_videoram1;
	UINT16 *     m_videoram2;
	UINT16 *     m_videoram3;
	UINT16 *     m_rowscroll;
	UINT16 *     m_spriteram;
//      UINT16 *     m_paletteram;    // currently this uses generic palette handling
	size_t       m_spriteram_size;

	/* video-related */
	tilemap_t   *m_tx_tilemap;
	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_bg_tilemap;
	int         m_bgscrollx;
	int         m_bgscrolly;
	int         m_bg_enable;
	int         m_bg_full_size;
	int         m_fgscrollx;
	int         m_fg_rowscroll_enable;

	int         m_xoffset;
	int         m_yoffset;
	int         m_txt_tile_offset;
	int         m_pri_masks[3];
	UINT16      m_scroll[7];

	/* powerbal-specific */
	int         m_tilebank;
	int         m_bg_yoffset;

	/* misc */
	UINT16      m_snd_command;
	UINT16      m_snd_flag;
	UINT8       m_oki_control;
	UINT8       m_oki_command;
	int         m_old_oki_bank;

	/* devices */
	okim6295_device *m_oki;
	eeprom_device *m_eeprom;
};

/*----------- defined in video/playmark.c -----------*/

WRITE16_HANDLER( wbeachvl_txvideoram_w );
WRITE16_HANDLER( wbeachvl_fgvideoram_w );
WRITE16_HANDLER( wbeachvl_bgvideoram_w );
WRITE16_HANDLER( hrdtimes_txvideoram_w );
WRITE16_HANDLER( hrdtimes_fgvideoram_w );
WRITE16_HANDLER( hrdtimes_bgvideoram_w );
WRITE16_HANDLER( bigtwin_paletteram_w );
WRITE16_HANDLER( bigtwin_scroll_w );
WRITE16_HANDLER( wbeachvl_scroll_w );
WRITE16_HANDLER( excelsr_scroll_w );
WRITE16_HANDLER( hrdtimes_scroll_w );

VIDEO_START( bigtwin );
VIDEO_START( bigtwinb );
VIDEO_START( wbeachvl );
VIDEO_START( excelsr );
VIDEO_START( hotmind );
VIDEO_START( hrdtimes );

SCREEN_UPDATE( bigtwin );
SCREEN_UPDATE( bigtwinb );
SCREEN_UPDATE( wbeachvl );
SCREEN_UPDATE( excelsr );
SCREEN_UPDATE( hrdtimes );
