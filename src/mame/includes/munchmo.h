/*************************************************************************

    Munch Mobile

*************************************************************************/

class munchmo_state : public driver_device
{
public:
	munchmo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *      m_vreg;
	UINT8 *      m_status_vram;
	UINT8 *      m_sprite_xpos;
	UINT8 *      m_sprite_attr;
	UINT8 *      m_sprite_tile;
	UINT8 *      m_videoram;

	/* video-related */
	bitmap_t     *m_tmpbitmap;
	int          m_palette_bank;
	int          m_flipscreen;

	/* misc */
	int          m_nmi_enable;
	int          m_which;
	UINT8        m_sound_nmi_enable;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};


/*----------- defined in video/munchmo.c -----------*/

WRITE8_HANDLER( mnchmobl_palette_bank_w );
WRITE8_HANDLER( mnchmobl_flipscreen_w );

PALETTE_INIT( mnchmobl );
VIDEO_START( mnchmobl );
SCREEN_UPDATE( mnchmobl );
