/*************************************************************************

    Taito Dual Screen Games

*************************************************************************/

class warriorb_state : public driver_device
{
public:
	warriorb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *   m_spriteram;
	size_t     m_spriteram_size;

	/* misc */
	INT32      m_banknum;
	int        m_pandata[4];

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_tc0140syt;
	device_t *m_tc0100scn_1;
	device_t *m_tc0100scn_2;
	device_t *m_lscreen;
	device_t *m_rscreen;
	device_t *m_2610_1l;
	device_t *m_2610_1r;
	device_t *m_2610_2l;
	device_t *m_2610_2r;
};


/*----------- defined in video/warriorb.c -----------*/

VIDEO_START( warriorb );
SCREEN_UPDATE( warriorb );
