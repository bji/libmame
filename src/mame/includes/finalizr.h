/***************************************************************************

    Finalizer

***************************************************************************/

class finalizr_state : public driver_device
{
public:
	finalizr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *       m_videoram;
	UINT8 *       m_colorram;
	UINT8 *       m_videoram2;
	UINT8 *       m_colorram2;
	UINT8 *       m_scroll;
	UINT8 *       m_spriteram;
	UINT8 *       m_spriteram_2;
	size_t        m_videoram_size;
	size_t        m_spriteram_size;

	/* video-related */
	tilemap_t       *m_fg_tilemap;
	tilemap_t       *m_bg_tilemap;
	int           m_spriterambank;
	int           m_charbank;

	/* misc */
	int           m_T1_line;
	UINT8         m_nmi_enable;
	UINT8         m_irq_enable;

	/* devices */
	device_t *m_audio_cpu;
};


/*----------- defined in video/finalizr.c -----------*/

WRITE8_HANDLER( finalizr_videoctrl_w );

PALETTE_INIT( finalizr );
VIDEO_START( finalizr );
SCREEN_UPDATE( finalizr );
