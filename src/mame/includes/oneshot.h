
class oneshot_state : public driver_device
{
public:
	oneshot_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *        m_sprites;
	UINT16 *        m_bg_videoram;
	UINT16 *        m_mid_videoram;
	UINT16 *        m_fg_videoram;
	UINT16 *        m_scroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_mid_tilemap;
	tilemap_t  *m_fg_tilemap;

	/* misc */
	int m_gun_x_p1;
	int m_gun_y_p1;
	int m_gun_x_p2;
	int m_gun_y_p2;
	int m_gun_x_shift;
	int m_p1_wobble;
	int m_p2_wobble;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};

/*----------- defined in video/oneshot.c -----------*/

WRITE16_HANDLER( oneshot_bg_videoram_w );
WRITE16_HANDLER( oneshot_mid_videoram_w );
WRITE16_HANDLER( oneshot_fg_videoram_w );

VIDEO_START( oneshot );
SCREEN_UPDATE( oneshot );
SCREEN_UPDATE( maddonna );
