class ssrj_state : public driver_device
{
public:
	ssrj_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int m_oldport;
	tilemap_t *m_tilemap1;
	tilemap_t *m_tilemap2;
	tilemap_t *m_tilemap4;
	UINT8 *m_vram1;
	UINT8 *m_vram2;
	UINT8 *m_vram3;
	UINT8 *m_vram4;
	UINT8 *m_scrollram;
};


/*----------- defined in video/ssrj.c -----------*/

WRITE8_HANDLER(ssrj_vram1_w);
WRITE8_HANDLER(ssrj_vram2_w);
WRITE8_HANDLER(ssrj_vram4_w);

VIDEO_START( ssrj );
SCREEN_UPDATE( ssrj );
PALETTE_INIT( ssrj );
