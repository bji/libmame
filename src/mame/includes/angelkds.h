/*************************************************************************

    Angel Kids

*************************************************************************/

class angelkds_state : public driver_device
{
public:
	angelkds_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_paletteram;
	UINT8 *    m_spriteram;
	UINT8 *    m_txvideoram;
	UINT8 *    m_bgtopvideoram;
	UINT8 *    m_bgbotvideoram;

	tilemap_t    *m_tx_tilemap;
	tilemap_t    *m_bgbot_tilemap;
	tilemap_t    *m_bgtop_tilemap;
	int        m_txbank;
	int        m_bgbotbank;
	int        m_bgtopbank;

	UINT8      m_sound[4];
	UINT8      m_sound2[4];
	UINT8      m_layer_ctrl;

	/* devices */
	device_t *m_subcpu;
};


/*----------- defined in video/angelkds.c -----------*/

WRITE8_HANDLER( angelkds_bgtopvideoram_w );
WRITE8_HANDLER( angelkds_bgbotvideoram_w );
WRITE8_HANDLER( angelkds_txvideoram_w );

WRITE8_HANDLER( angelkds_bgtopbank_write );
WRITE8_HANDLER( angelkds_bgtopscroll_write );
WRITE8_HANDLER( angelkds_bgbotbank_write );
WRITE8_HANDLER( angelkds_bgbotscroll_write );
WRITE8_HANDLER( angelkds_txbank_write );

WRITE8_HANDLER( angelkds_paletteram_w );
WRITE8_HANDLER( angelkds_layer_ctrl_write );

VIDEO_START( angelkds );
SCREEN_UPDATE( angelkds );
