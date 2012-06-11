class vball_state : public driver_device
{
public:
	vball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vb_attribram(*this, "vb_attribram"),
		m_vb_videoram(*this, "vb_videoram"),
		m_vb_scrolly_lo(*this, "vb_scrolly_lo"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT8> m_vb_attribram;
	required_shared_ptr<UINT8> m_vb_videoram;
	required_shared_ptr<UINT8> m_vb_scrolly_lo;
	int m_vb_scrollx_hi;
	int m_vb_scrolly_hi;
	int m_vb_scrollx_lo;
	int m_gfxset;
	int m_vb_scrollx[256];
	int m_vb_bgprombank;
	int m_vb_spprombank;
	tilemap_t *m_bg_tilemap;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(vball_irq_ack_w);
	DECLARE_WRITE8_MEMBER(vb_bankswitch_w);
	DECLARE_WRITE8_MEMBER(cpu_sound_command_w);
	DECLARE_WRITE8_MEMBER(vb_scrollx_hi_w);
	DECLARE_WRITE8_MEMBER(vb_scrollx_lo_w);
	DECLARE_WRITE8_MEMBER(vb_videoram_w);
	DECLARE_READ8_MEMBER(vb_attrib_r);
	DECLARE_WRITE8_MEMBER(vb_attrib_w);
};


/*----------- defined in video/vball.c -----------*/

VIDEO_START( vb );
SCREEN_UPDATE_IND16( vb );
void vb_bgprombank_w(running_machine &machine, int bank);
void vb_spprombank_w(running_machine &machine, int bank);
void vb_mark_all_dirty(running_machine &machine);
