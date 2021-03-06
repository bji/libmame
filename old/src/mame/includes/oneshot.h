
class oneshot_state : public driver_device
{
public:
	oneshot_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *        sprites;
	UINT16 *        bg_videoram;
	UINT16 *        mid_videoram;
	UINT16 *        fg_videoram;
	UINT16 *        scroll;

	/* video-related */
	tilemap_t  *bg_tilemap, *mid_tilemap, *fg_tilemap;

	/* misc */
	int gun_x_p1, gun_y_p1, gun_x_p2, gun_y_p2;
	int gun_x_shift;
	int p1_wobble, p2_wobble;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
};

/*----------- defined in video/oneshot.c -----------*/

WRITE16_HANDLER( oneshot_bg_videoram_w );
WRITE16_HANDLER( oneshot_mid_videoram_w );
WRITE16_HANDLER( oneshot_fg_videoram_w );

VIDEO_START( oneshot );
SCREEN_UPDATE( oneshot );
SCREEN_UPDATE( maddonna );
