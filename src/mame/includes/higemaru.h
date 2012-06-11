/*************************************************************************

    Pirate Ship Higemaru

*************************************************************************/

class higemaru_state : public driver_device
{
public:
	higemaru_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(higemaru_videoram_w);
	DECLARE_WRITE8_MEMBER(higemaru_colorram_w);
	DECLARE_WRITE8_MEMBER(higemaru_c800_w);
};


/*----------- defined in video/higemaru.c -----------*/


PALETTE_INIT( higemaru );
VIDEO_START( higemaru );
SCREEN_UPDATE_IND16( higemaru );
