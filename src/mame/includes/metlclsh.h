/*************************************************************************

    Metal Clash

*************************************************************************/

class metlclsh_state : public driver_device
{
public:
	metlclsh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram"),
		m_bgram(*this, "bgram"),
		m_scrollx(*this, "scrollx"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_fgram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_bgram;
	required_shared_ptr<UINT8> m_scrollx;
	UINT8 *        m_otherram;
//      UINT8 *        m_paletteram;    // currently this uses generic palette handling
//      UINT8 *        m_paletteram2;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t      *m_bg_tilemap;
	tilemap_t      *m_fg_tilemap;
	UINT8          m_write_mask;
	UINT8          m_gfxbank;

	/* devices */
	device_t *m_maincpu;
	device_t *m_subcpu;
	DECLARE_WRITE8_MEMBER(metlclsh_cause_irq);
	DECLARE_WRITE8_MEMBER(metlclsh_ack_nmi);
	DECLARE_WRITE8_MEMBER(metlclsh_cause_nmi2);
	DECLARE_WRITE8_MEMBER(metlclsh_ack_irq2);
	DECLARE_WRITE8_MEMBER(metlclsh_ack_nmi2);
	DECLARE_WRITE8_MEMBER(metlclsh_flipscreen_w);
	DECLARE_WRITE8_MEMBER(metlclsh_rambank_w);
	DECLARE_WRITE8_MEMBER(metlclsh_gfxbank_w);
	DECLARE_WRITE8_MEMBER(metlclsh_bgram_w);
	DECLARE_WRITE8_MEMBER(metlclsh_fgram_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
};


/*----------- defined in video/metlclsh.c -----------*/


VIDEO_START( metlclsh );
SCREEN_UPDATE_IND16( metlclsh );
