/*************************************************************************

    King of Boxer - Ring King

*************************************************************************/

class kingofb_state : public driver_device
{
public:
	kingofb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_scroll_y(*this, "scroll_y"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_scroll_y;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	int        m_palette_bank;

	/* misc */
	int        m_nmi_enable;

	/* devices */
	device_t *m_video_cpu;
	device_t *m_sprite_cpu;
	device_t *m_audio_cpu;
	DECLARE_WRITE8_MEMBER(video_interrupt_w);
	DECLARE_WRITE8_MEMBER(sprite_interrupt_w);
	DECLARE_WRITE8_MEMBER(scroll_interrupt_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(kingofb_videoram_w);
	DECLARE_WRITE8_MEMBER(kingofb_colorram_w);
	DECLARE_WRITE8_MEMBER(kingofb_videoram2_w);
	DECLARE_WRITE8_MEMBER(kingofb_colorram2_w);
	DECLARE_WRITE8_MEMBER(kingofb_f800_w);
};


/*----------- defined in video/kingobox.c -----------*/


PALETTE_INIT( kingofb );
VIDEO_START( kingofb );
SCREEN_UPDATE_IND16( kingofb );

PALETTE_INIT( ringking );
VIDEO_START( ringking );
SCREEN_UPDATE_IND16( ringking );
