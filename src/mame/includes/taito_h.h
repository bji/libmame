/*************************************************************************

    Taito H system

*************************************************************************/

class taitoh_state : public driver_device
{
public:
	taitoh_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    m_m68000_mainram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* misc */
	INT32       m_banknum;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_tc0080vco;
	device_t *m_tc0220ioc;
};

/*----------- defined in video/taito_h.c -----------*/

SCREEN_UPDATE( syvalion );
SCREEN_UPDATE( recordbr );
SCREEN_UPDATE( dleague );
