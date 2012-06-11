/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/


#define CPUTAG_MCU "mcu"


class champbas_state : public driver_device
{
public:
	champbas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram_2(*this, "spriteram_2"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_spriteram_2;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	UINT8          m_gfx_bank;
	UINT8          m_palette_bank;

	/* misc */
	int            m_watchdog_count;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_mcu;

	UINT8          m_irq_mask;
	DECLARE_WRITE8_MEMBER(champbas_watchdog_reset_w);
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(champbas_mcu_switch_w);
	DECLARE_WRITE8_MEMBER(champbas_mcu_halt_w);
	DECLARE_READ8_MEMBER(champbja_alt_protection_r);
	DECLARE_WRITE8_MEMBER(champbas_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(champbas_gfxbank_w);
	DECLARE_WRITE8_MEMBER(champbas_palette_bank_w);
	DECLARE_WRITE8_MEMBER(champbas_flipscreen_w);
	DECLARE_CUSTOM_INPUT_MEMBER(champbas_watchdog_bit2);
};


/*----------- defined in video/champbas.c -----------*/


PALETTE_INIT( champbas );
PALETTE_INIT( exctsccr );
VIDEO_START( champbas );
VIDEO_START( exctsccr );
SCREEN_UPDATE_IND16( champbas );
SCREEN_UPDATE_IND16( exctsccr );


