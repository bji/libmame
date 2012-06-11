/***************************************************************************

    Kyugo hardware games

***************************************************************************/

class kyugo_state : public driver_device
{
public:
	kyugo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_bgattribram(*this, "bgattribram"),
		m_spriteram_1(*this, "spriteram_1"),
		m_spriteram_2(*this, "spriteram_2"),
		m_shared_ram(*this, "shared_ram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_bgattribram;
	required_shared_ptr<UINT8> m_spriteram_1;
	required_shared_ptr<UINT8> m_spriteram_2;
	required_shared_ptr<UINT8> m_shared_ram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	UINT8       m_scroll_x_lo;
	UINT8       m_scroll_x_hi;
	UINT8       m_scroll_y;
	int         m_bgpalbank;
	int         m_fgcolor;
	int         m_flipscreen;
	const UINT8 *m_color_codes;

	/* devices */
	device_t *m_maincpu;
	device_t *m_subcpu;

	UINT8       m_nmi_mask;
	DECLARE_WRITE8_MEMBER(kyugo_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(kyugo_sub_cpu_control_w);
	DECLARE_WRITE8_MEMBER(kyugo_coin_counter_w);
	DECLARE_WRITE8_MEMBER(kyugo_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(kyugo_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(kyugo_bgattribram_w);
	DECLARE_READ8_MEMBER(kyugo_spriteram_2_r);
	DECLARE_WRITE8_MEMBER(kyugo_scroll_x_lo_w);
	DECLARE_WRITE8_MEMBER(kyugo_gfxctrl_w);
	DECLARE_WRITE8_MEMBER(kyugo_scroll_y_w);
	DECLARE_WRITE8_MEMBER(kyugo_flipscreen_w);
};


/*----------- defined in video/kyugo.c -----------*/



VIDEO_START( kyugo );
SCREEN_UPDATE_IND16( kyugo );
