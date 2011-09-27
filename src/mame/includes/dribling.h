/*************************************************************************

    Model Racing Dribbling hardware

*************************************************************************/



class dribling_state : public driver_device
{
public:
	dribling_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;

	/* misc */
	UINT8    m_abca;
	UINT8    m_dr;
	UINT8    m_ds;
	UINT8    m_sh;
	UINT8    m_input_mux;
	UINT8    m_di;

	/* devices */
	device_t *m_maincpu;
	device_t *m_ppi_0;
	device_t *m_ppi_1;
};


/*----------- defined in video/dribling.c -----------*/

PALETTE_INIT( dribling );
WRITE8_HANDLER( dribling_colorram_w );
SCREEN_UPDATE( dribling );
