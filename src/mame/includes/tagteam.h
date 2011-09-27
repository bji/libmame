class tagteam_state : public driver_device
{
public:
	tagteam_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_coin;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	int m_palettebank;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/tagteam.c -----------*/

WRITE8_HANDLER( tagteam_videoram_w );
WRITE8_HANDLER( tagteam_colorram_w );
READ8_HANDLER( tagteam_mirrorvideoram_r );
WRITE8_HANDLER( tagteam_mirrorvideoram_w );
READ8_HANDLER( tagteam_mirrorcolorram_r );
WRITE8_HANDLER( tagteam_mirrorcolorram_w );
WRITE8_HANDLER( tagteam_control_w );
WRITE8_HANDLER( tagteam_flipscreen_w );

PALETTE_INIT( tagteam );
VIDEO_START( tagteam );
SCREEN_UPDATE( tagteam );
