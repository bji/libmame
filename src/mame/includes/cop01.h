/*************************************************************************

    Cops 01

*************************************************************************/

class cop01_state : public driver_device
{
public:
	cop01_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_fgvideoram(*this, "fgvideoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_fgvideoram;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	UINT8          m_vreg[4];

	/* sound-related */
	int            m_pulse;
	int            m_timer;	// kludge for ym3526 in mightguy

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	DECLARE_WRITE8_MEMBER(cop01_sound_command_w);
	DECLARE_READ8_MEMBER(cop01_sound_command_r);
	DECLARE_WRITE8_MEMBER(cop01_irq_ack_w);
	DECLARE_READ8_MEMBER(cop01_sound_irq_ack_w);
	DECLARE_READ8_MEMBER(kludge);
	DECLARE_WRITE8_MEMBER(cop01_background_w);
	DECLARE_WRITE8_MEMBER(cop01_foreground_w);
	DECLARE_WRITE8_MEMBER(cop01_vreg_w);
	DECLARE_CUSTOM_INPUT_MEMBER(mightguy_area_r);
};



/*----------- defined in video/cop01.c -----------*/


PALETTE_INIT( cop01 );
VIDEO_START( cop01 );
SCREEN_UPDATE_IND16( cop01 );

