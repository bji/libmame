class taotaido_state : public driver_device
{
public:
	taotaido_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_pending_command;
	UINT16 *m_spriteram;
	UINT16 *m_spriteram2;
	UINT16 *m_scrollram;
	UINT16 *m_bgram;
	UINT16 m_sprite_character_bank_select[8];
	UINT16 m_video_bank_select[8];
	tilemap_t *m_bg_tilemap;
	UINT16 *m_spriteram_old;
	UINT16 *m_spriteram_older;
	UINT16 *m_spriteram2_old;
	UINT16 *m_spriteram2_older;
};


/*----------- defined in video/taotaido.c -----------*/

WRITE16_HANDLER( taotaido_sprite_character_bank_select_w );
WRITE16_HANDLER( taotaido_tileregs_w );
WRITE16_HANDLER( taotaido_bgvideoram_w );
VIDEO_START( taotaido );
SCREEN_UPDATE_IND16( taotaido );
SCREEN_VBLANK( taotaido );
