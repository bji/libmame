
class f1gp_state : public driver_device
{
public:
	f1gp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_sharedram;
	UINT16 *  m_spr1vram;
	UINT16 *  m_spr2vram;
	UINT16 *  m_spr1cgram;
	UINT16 *  m_spr2cgram;
	UINT16 *  m_fgvideoram;
	UINT16 *  m_rozvideoram;
	UINT16 *  m_sprcgram;
	UINT16 *  m_spritelist;
	UINT16 *  m_spriteram;
	UINT16 *  m_fgregs;
	UINT16 *  m_rozregs;
	UINT16 *  m_zoomdata;
//      UINT16 *  m_paletteram;    // currently this uses generic palette handling
	size_t    m_spr1cgram_size;
	size_t    m_spr2cgram_size;
	size_t    m_spriteram_size;

	/* video-related */
	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_roz_tilemap;
	int       m_roz_bank;
	int       m_flipscreen;
	int       m_gfxctrl;
	int       m_scroll[2];

	/* misc */
	int       m_pending_command;

	/* devices */
	device_t *m_audiocpu;
	device_t *m_k053936;
};

/*----------- defined in video/f1gp.c -----------*/

READ16_HANDLER( f1gp_zoomdata_r );
WRITE16_HANDLER( f1gp_zoomdata_w );
READ16_HANDLER( f1gp_rozvideoram_r );
WRITE16_HANDLER( f1gp_rozvideoram_w );
WRITE16_HANDLER( f1gp_fgvideoram_w );
WRITE16_HANDLER( f1gp_fgscroll_w );
WRITE16_HANDLER( f1gp_gfxctrl_w );
WRITE16_HANDLER( f1gp2_gfxctrl_w );

VIDEO_START( f1gp );
VIDEO_START( f1gpb );
VIDEO_START( f1gp2 );
SCREEN_UPDATE_IND16( f1gp );
SCREEN_UPDATE_IND16( f1gpb );
SCREEN_UPDATE_IND16( f1gp2 );
