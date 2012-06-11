/*************************************************************************

    Pinball Action

*************************************************************************/

class pbaction_state : public driver_device
{
public:
	pbaction_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_work_ram(*this, "work_ram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_work_ram;
	required_shared_ptr<UINT8> m_spriteram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	int        m_scroll;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;

	UINT8      m_nmi_mask;
	DECLARE_WRITE8_MEMBER(pbaction_sh_command_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_READ8_MEMBER(pbactio3_prot_kludge_r);
	DECLARE_WRITE8_MEMBER(pbaction_videoram_w);
	DECLARE_WRITE8_MEMBER(pbaction_colorram_w);
	DECLARE_WRITE8_MEMBER(pbaction_videoram2_w);
	DECLARE_WRITE8_MEMBER(pbaction_colorram2_w);
	DECLARE_WRITE8_MEMBER(pbaction_scroll_w);
	DECLARE_WRITE8_MEMBER(pbaction_flipscreen_w);
};


/*----------- defined in video/pbaction.c -----------*/


extern VIDEO_START( pbaction );
extern SCREEN_UPDATE_IND16( pbaction );
