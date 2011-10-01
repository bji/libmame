class tryout_state : public driver_device
{
public:
	tryout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_gfx_control;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 m_vram_bank;
	UINT8 *m_vram;
	UINT8 *m_vram_gfx;
	UINT8 *m_spriteram;
	UINT8 *m_spriteram2;
};


/*----------- defined in video/tryout.c -----------*/

READ8_HANDLER( tryout_vram_r );
WRITE8_HANDLER( tryout_videoram_w );
WRITE8_HANDLER( tryout_vram_w );
WRITE8_HANDLER( tryout_vram_bankswitch_w );
WRITE8_HANDLER( tryout_flipscreen_w );

PALETTE_INIT( tryout );
VIDEO_START( tryout );
SCREEN_UPDATE( tryout );
