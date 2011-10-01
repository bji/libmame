class m62_state : public driver_device
{
public:
	m62_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *              m_spriteram;
	size_t               m_spriteram_size;

	UINT8 *              m_m62_tileram;
	UINT8 *              m_m62_textram;
	UINT8 *              m_scrollram;

	/* video-related */
	tilemap_t*             m_bg_tilemap;
	tilemap_t*             m_fg_tilemap;
	int                  m_flipscreen;

	const UINT8          *m_sprite_height_prom;
	INT32                m_m62_background_hscroll;
	INT32                m_m62_background_vscroll;
	UINT8                m_kidniki_background_bank;
	INT32                m_kidniki_text_vscroll;
	int                  m_ldrun3_topbottom_mask;
	INT32                m_spelunkr_palbank;

	/* misc */
	int                 m_ldrun2_bankswap;	//ldrun2
	int                 m_bankcontrol[2];	//ldrun2
};


/*----------- defined in video/m62.c -----------*/

WRITE8_HANDLER( m62_tileram_w );
WRITE8_HANDLER( m62_textram_w );
WRITE8_HANDLER( m62_flipscreen_w );
WRITE8_HANDLER( m62_hscroll_low_w );
WRITE8_HANDLER( m62_hscroll_high_w );
WRITE8_HANDLER( m62_vscroll_low_w );
WRITE8_HANDLER( m62_vscroll_high_w );

WRITE8_HANDLER( horizon_scrollram_w );
WRITE8_HANDLER( kidniki_text_vscroll_low_w );
WRITE8_HANDLER( kidniki_text_vscroll_high_w );
WRITE8_HANDLER( kidniki_background_bank_w );
WRITE8_HANDLER( kungfum_tileram_w );
WRITE8_HANDLER( ldrun3_topbottom_mask_w );
WRITE8_HANDLER( spelunkr_palbank_w );
WRITE8_HANDLER( spelunk2_gfxport_w );

PALETTE_INIT( m62 );
PALETTE_INIT( lotlot );
PALETTE_INIT( battroad );
PALETTE_INIT( spelunk2 );

VIDEO_START( battroad );
VIDEO_START( horizon );
VIDEO_START( kidniki );
VIDEO_START( kungfum );
VIDEO_START( ldrun );
VIDEO_START( ldrun2 );
VIDEO_START( ldrun4 );
VIDEO_START( lotlot );
VIDEO_START( spelunkr );
VIDEO_START( spelunk2 );
VIDEO_START( youjyudn );

SCREEN_UPDATE( battroad );
SCREEN_UPDATE( horizon );
SCREEN_UPDATE( kidniki );
SCREEN_UPDATE( kungfum );
SCREEN_UPDATE( ldrun );
SCREEN_UPDATE( ldrun3 );
SCREEN_UPDATE( ldrun4 );
SCREEN_UPDATE( lotlot );
SCREEN_UPDATE( spelunkr );
SCREEN_UPDATE( spelunk2 );
SCREEN_UPDATE( youjyudn );
