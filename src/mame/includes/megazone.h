/*************************************************************************

    Megazone

*************************************************************************/

class megazone_state : public driver_device
{
public:
	megazone_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *       m_scrollx;
	UINT8 *       m_scrolly;
	UINT8 *       m_videoram;
	UINT8 *       m_colorram;
	UINT8 *       m_videoram2;
	UINT8 *       m_colorram2;
	UINT8 *       m_spriteram;
	size_t        m_spriteram_size;
	size_t        m_videoram_size;
	size_t        m_videoram2_size;

	/* video-related */
	bitmap_t      *m_tmpbitmap;
	int           m_flipscreen;

	/* misc */
	int           m_i8039_status;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	cpu_device *m_daccpu;
};



/*----------- defined in video/megazone.c -----------*/

WRITE8_HANDLER( megazone_flipscreen_w );

PALETTE_INIT( megazone );
VIDEO_START( megazone );
SCREEN_UPDATE( megazone );
