/*************************************************************************

    Taito O system

*************************************************************************/

class taitoo_state : public driver_device
{
public:
	taitoo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* devices */
	device_t *m_maincpu;
	device_t *m_tc0080vco;
};

/*----------- defined in video/taito_o.c -----------*/

SCREEN_UPDATE( parentj );
