/*************************************************************************

    Taito Triple Screen Games

*************************************************************************/

class ninjaw_state : public driver_device
{
public:
	ninjaw_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *   m_spriteram;
	size_t     m_spriteram_size;

	/* misc */
	UINT16     m_cpua_ctrl;
	INT32      m_banknum;
	int        m_pandata[4];

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_subcpu;
	device_t *m_tc0140syt;
	device_t *m_tc0100scn_1;
	device_t *m_tc0100scn_2;
	device_t *m_tc0100scn_3;
	device_t *m_lscreen;
	device_t *m_mscreen;
	device_t *m_rscreen;
	device_t *m_2610_1l;
	device_t *m_2610_1r;
	device_t *m_2610_2l;
	device_t *m_2610_2r;
};


/*----------- defined in video/ninjaw.c -----------*/

VIDEO_START( ninjaw );
SCREEN_UPDATE( ninjaw );
