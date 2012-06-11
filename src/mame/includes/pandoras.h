/*************************************************************************

    Pandora's Palace

*************************************************************************/

class pandoras_state : public driver_device
{
public:
	pandoras_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;

	/* video-related */
	tilemap_t     *m_layer0;
	int         m_flipscreen;

	int m_irq_enable_a;
	int m_irq_enable_b;
	int m_firq_old_data_a;
	int m_firq_old_data_b;
	int m_i8039_status;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_subcpu;
	cpu_device *m_audiocpu;
	cpu_device *m_mcu;
};


/*----------- defined in video/pandoras.c -----------*/

PALETTE_INIT( pandoras );

WRITE8_HANDLER( pandoras_vram_w );
WRITE8_HANDLER( pandoras_cram_w );
WRITE8_HANDLER( pandoras_flipscreen_w );
WRITE8_HANDLER( pandoras_scrolly_w );

VIDEO_START( pandoras );
SCREEN_UPDATE_IND16( pandoras );

