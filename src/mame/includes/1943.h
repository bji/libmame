/***************************************************************************

    1943

***************************************************************************/

class _1943_state : public driver_device
{
public:
	_1943_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_scrollx(*this, "scrollx"),
		m_scrolly(*this, "scrolly"),
		m_bgscrollx(*this, "bgscrollx"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_scrollx;
	required_shared_ptr<UINT8> m_scrolly;
	required_shared_ptr<UINT8> m_bgscrollx;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg2_tilemap;
	int     m_char_on;
	int     m_obj_on;
	int     m_bg1_on;
	int     m_bg2_on;
	DECLARE_READ8_MEMBER(c1943_protection_r);
	DECLARE_READ8_MEMBER(_1943b_c007_r);
	DECLARE_WRITE8_MEMBER(c1943_videoram_w);
	DECLARE_WRITE8_MEMBER(c1943_colorram_w);
	DECLARE_WRITE8_MEMBER(c1943_c804_w);
	DECLARE_WRITE8_MEMBER(c1943_d806_w);
};



/*----------- defined in video/1943.c -----------*/


extern PALETTE_INIT( 1943 );
extern VIDEO_START( 1943 );
extern SCREEN_UPDATE_IND16( 1943 );
