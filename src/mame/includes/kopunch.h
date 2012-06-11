/*************************************************************************

    KO Punch

*************************************************************************/

class kopunch_state : public driver_device
{
public:
	kopunch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	int        m_gfxbank;

	/* devices */
	device_t *m_maincpu;
	DECLARE_READ8_MEMBER(kopunch_in_r);
	DECLARE_WRITE8_MEMBER(kopunch_lamp_w);
	DECLARE_WRITE8_MEMBER(kopunch_coin_w);
	DECLARE_WRITE8_MEMBER(kopunch_videoram_w);
	DECLARE_WRITE8_MEMBER(kopunch_videoram2_w);
	DECLARE_WRITE8_MEMBER(kopunch_scroll_x_w);
	DECLARE_WRITE8_MEMBER(kopunch_scroll_y_w);
	DECLARE_WRITE8_MEMBER(kopunch_gfxbank_w);
	DECLARE_INPUT_CHANGED_MEMBER(left_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(right_coin_inserted);
};

/*----------- defined in video/kopunch.c -----------*/


PALETTE_INIT( kopunch );
VIDEO_START( kopunch );
SCREEN_UPDATE_IND16( kopunch );
