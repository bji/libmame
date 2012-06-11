
class mugsmash_state : public driver_device
{
public:
	mugsmash_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram1;
	UINT16 *m_videoram2;
	UINT16 *m_spriteram;
	UINT16 *m_regs1;
	UINT16 *m_regs2;

	tilemap_t *m_tilemap1;
	tilemap_t *m_tilemap2;

	device_t *m_maincpu;
	device_t *m_audiocpu;
};


/*----------- defined in video/mugsmash.c -----------*/

VIDEO_START( mugsmash );
SCREEN_UPDATE_IND16( mugsmash );

WRITE16_HANDLER( mugsmash_reg_w );
WRITE16_HANDLER( mugsmash_videoram2_w );
WRITE16_HANDLER( mugsmash_videoram1_w );
