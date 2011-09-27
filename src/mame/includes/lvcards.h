class lvcards_state : public driver_device
{
public:
	lvcards_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 m_payout;
	UINT8 m_pulse;
	UINT8 m_result;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/lvcards.c -----------*/

WRITE8_HANDLER( lvcards_videoram_w );
WRITE8_HANDLER( lvcards_colorram_w );

PALETTE_INIT( lvcards );
PALETTE_INIT( ponttehk );
VIDEO_START( lvcards );
SCREEN_UPDATE( lvcards );
