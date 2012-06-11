class ninjakd2_state : public driver_device
{
public:
	ninjakd2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	const INT16* m_sampledata;
	UINT8 m_omegaf_io_protection[3];
	UINT8 m_omegaf_io_protection_input;
	int m_omegaf_io_protection_tic;
	UINT8* m_bg_videoram;
	UINT8* m_fg_videoram;
	int m_next_sprite_overdraw_enabled;
	int (*m_stencil_compare_function) (UINT16 pal);
	int m_sprites_updated;
	bitmap_ind16 m_sp_bitmap;
	int m_robokid_sprites;
	tilemap_t* m_fg_tilemap;
	tilemap_t* m_bg_tilemap;
	tilemap_t* m_bg0_tilemap;
	tilemap_t* m_bg1_tilemap;
	tilemap_t* m_bg2_tilemap;
	int m_bank_mask;
	int m_robokid_bg0_bank;
	int m_robokid_bg1_bank;
	int m_robokid_bg2_bank;
	UINT8* m_robokid_bg0_videoram;
	UINT8* m_robokid_bg1_videoram;
	UINT8* m_robokid_bg2_videoram;
	UINT8 *m_spriteram;
};


/*----------- defined in video/ninjakd2.c -----------*/

extern WRITE8_HANDLER( ninjakd2_bgvideoram_w );
extern WRITE8_HANDLER( ninjakd2_fgvideoram_w );
extern WRITE8_HANDLER( ninjakd2_bg_ctrl_w );
extern WRITE8_HANDLER( ninjakd2_sprite_overdraw_w );

extern READ8_HANDLER( robokid_bg0_videoram_r );
extern READ8_HANDLER( robokid_bg1_videoram_r );
extern READ8_HANDLER( robokid_bg2_videoram_r );
extern WRITE8_HANDLER( robokid_bg0_videoram_w );
extern WRITE8_HANDLER( robokid_bg1_videoram_w );
extern WRITE8_HANDLER( robokid_bg2_videoram_w );
extern WRITE8_HANDLER( robokid_bg0_ctrl_w );
extern WRITE8_HANDLER( robokid_bg1_ctrl_w );
extern WRITE8_HANDLER( robokid_bg2_ctrl_w );
extern WRITE8_HANDLER( robokid_bg0_bank_w );
extern WRITE8_HANDLER( robokid_bg1_bank_w );
extern WRITE8_HANDLER( robokid_bg2_bank_w );

extern VIDEO_START( ninjakd2 );
extern VIDEO_START( mnight );
extern VIDEO_START( arkarea );
extern VIDEO_START( robokid );
extern VIDEO_START( omegaf );
extern SCREEN_UPDATE_IND16( ninjakd2 );
extern SCREEN_UPDATE_IND16( robokid );
extern SCREEN_UPDATE_IND16( omegaf );
extern SCREEN_VBLANK( ninjakd2 );

