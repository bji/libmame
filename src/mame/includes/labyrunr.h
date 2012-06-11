/*************************************************************************

    Labyrinth Runner

*************************************************************************/

class labyrunr_state : public driver_device
{
public:
	labyrunr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_scrollram(*this, "scrollram"),
		m_paletteram(*this, "paletteram"),
		m_spriteram(*this, "spriteram"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"){ }

	/* devices */
	device_t *m_k007121;

	required_device<cpu_device> m_maincpu;
	/* memory pointers */
	required_shared_ptr<UINT8> m_scrollram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram1;
	required_shared_ptr<UINT8> m_videoram2;

	/* video-related */
	tilemap_t    *m_layer0;
	tilemap_t    *m_layer1;
	rectangle  m_clip0;
	rectangle  m_clip1;

	DECLARE_WRITE8_MEMBER(labyrunr_bankswitch_w);
	DECLARE_WRITE8_MEMBER(labyrunr_vram1_w);
	DECLARE_WRITE8_MEMBER(labyrunr_vram2_w);
};


/*----------- defined in video/labyrunr.c -----------*/



PALETTE_INIT( labyrunr );
VIDEO_START( labyrunr );
SCREEN_UPDATE_IND16( labyrunr );
