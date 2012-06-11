/*************************************************************************

    Mikie

*************************************************************************/

class mikie_state : public driver_device
{
public:
	mikie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int        m_palettebank;

	/* misc */
	int        m_last_irq;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;

	UINT8      m_irq_mask;
	DECLARE_READ8_MEMBER(mikie_sh_timer_r);
	DECLARE_WRITE8_MEMBER(mikie_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(mikie_coin_counter_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(mikie_videoram_w);
	DECLARE_WRITE8_MEMBER(mikie_colorram_w);
	DECLARE_WRITE8_MEMBER(mikie_palettebank_w);
	DECLARE_WRITE8_MEMBER(mikie_flipscreen_w);
};


/*----------- defined in video/mikie.c -----------*/


PALETTE_INIT( mikie );
VIDEO_START( mikie );
SCREEN_UPDATE_IND16( mikie );
