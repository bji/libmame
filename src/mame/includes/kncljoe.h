/*************************************************************************

    Knuckle Joe

*************************************************************************/

class kncljoe_state : public driver_device
{
public:
	kncljoe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_scrollregs(*this, "scrollregs"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_scrollregs;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_tile_bank;
	int			m_sprite_bank;
	int        m_flipscreen;

	/* misc */
	UINT8      m_port1;
	UINT8      m_port2;

	/* devices */
	device_t *m_soundcpu;
	DECLARE_WRITE8_MEMBER(sound_cmd_w);
	DECLARE_WRITE8_MEMBER(sound_irq_ack_w);
	DECLARE_WRITE8_MEMBER(kncljoe_videoram_w);
	DECLARE_WRITE8_MEMBER(kncljoe_control_w);
	DECLARE_WRITE8_MEMBER(kncljoe_scroll_w);
};



/*----------- defined in video/kncljoe.c -----------*/


PALETTE_INIT( kncljoe );
VIDEO_START( kncljoe );
SCREEN_UPDATE_IND16( kncljoe );
