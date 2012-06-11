/*************************************************************************

    Contra / Gryzor

*************************************************************************/

class contra_state : public driver_device
{
public:
	contra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_paletteram(*this, "paletteram"),
		m_fg_cram(*this, "fg_cram"),
		m_fg_vram(*this, "fg_vram"),
		m_tx_cram(*this, "tx_cram"),
		m_tx_vram(*this, "tx_vram"),
		m_spriteram(*this, "spriteram"),
		m_bg_cram(*this, "bg_cram"),
		m_bg_vram(*this, "bg_vram"){ }

	/* memory pointers */
	UINT8 *        m_buffered_spriteram;
	UINT8 *        m_buffered_spriteram_2;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_fg_cram;
	required_shared_ptr<UINT8> m_fg_vram;
	required_shared_ptr<UINT8> m_tx_cram;
	required_shared_ptr<UINT8> m_tx_vram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_bg_cram;
	required_shared_ptr<UINT8> m_bg_vram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;
	rectangle m_bg_clip;
	rectangle m_fg_clip;
	rectangle m_tx_clip;

	/* devices */
	device_t *m_audiocpu;
	device_t *m_k007121_1;
	device_t *m_k007121_2;
	DECLARE_WRITE8_MEMBER(contra_bankswitch_w);
	DECLARE_WRITE8_MEMBER(contra_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(contra_coin_counter_w);
	DECLARE_WRITE8_MEMBER(cpu_sound_command_w);
	DECLARE_WRITE8_MEMBER(contra_fg_vram_w);
	DECLARE_WRITE8_MEMBER(contra_fg_cram_w);
	DECLARE_WRITE8_MEMBER(contra_bg_vram_w);
	DECLARE_WRITE8_MEMBER(contra_bg_cram_w);
	DECLARE_WRITE8_MEMBER(contra_text_vram_w);
	DECLARE_WRITE8_MEMBER(contra_text_cram_w);
	DECLARE_WRITE8_MEMBER(contra_K007121_ctrl_0_w);
	DECLARE_WRITE8_MEMBER(contra_K007121_ctrl_1_w);
};


/*----------- defined in video/contra.c -----------*/

PALETTE_INIT( contra );



SCREEN_UPDATE_IND16( contra );
VIDEO_START( contra );
