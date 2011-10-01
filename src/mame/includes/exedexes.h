/*************************************************************************

    Exed Exes

*************************************************************************/


class exedexes_state : public driver_device
{
public:
	exedexes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_videoram;
	UINT8 *        m_colorram;
	UINT8 *        m_bg_scroll;
	UINT8 *        m_nbg_yscroll;
	UINT8 *        m_nbg_xscroll;
//  UINT8 *        m_spriteram;   // currently this uses generic buffered_spriteram

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	tilemap_t        *m_tx_tilemap;
	int            m_chon;
	int            m_objon;
	int            m_sc1on;
	int            m_sc2on;
};



/*----------- defined in video/exedexes.c -----------*/

extern WRITE8_HANDLER( exedexes_videoram_w );
extern WRITE8_HANDLER( exedexes_colorram_w );
extern WRITE8_HANDLER( exedexes_c804_w );
extern WRITE8_HANDLER( exedexes_gfxctrl_w );

extern PALETTE_INIT( exedexes );
extern VIDEO_START( exedexes );
extern SCREEN_UPDATE( exedexes );
extern SCREEN_EOF( exedexes );
