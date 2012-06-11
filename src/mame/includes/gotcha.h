/*************************************************************************

    Gotcha

*************************************************************************/

class gotcha_state : public driver_device
{
public:
	gotcha_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_fgvideoram;
	required_shared_ptr<UINT16> m_bgvideoram;
	required_shared_ptr<UINT16> m_spriteram;
//  UINT16 *    m_paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	int         m_banksel;
	int         m_gfxbank[4];
	UINT16      m_scroll[4];

	/* devices */
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(gotcha_lamps_w);
	DECLARE_WRITE16_MEMBER(gotcha_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(gotcha_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(gotcha_gfxbank_select_w);
	DECLARE_WRITE16_MEMBER(gotcha_gfxbank_w);
	DECLARE_WRITE16_MEMBER(gotcha_scroll_w);
};


/*----------- defined in video/gotcha.c -----------*/


VIDEO_START( gotcha );
SCREEN_UPDATE_IND16( gotcha );

