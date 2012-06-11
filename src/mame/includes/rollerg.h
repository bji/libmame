/*************************************************************************

    Rollergames

*************************************************************************/

class rollerg_state : public driver_device
{
public:
	rollerg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_sprite_colorbase;
	int        m_zoom_colorbase;

	/* misc */
	int        m_readzoomroms;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k053260;
	device_t *m_k053244;
	device_t *m_k051316;
	DECLARE_WRITE8_MEMBER(rollerg_0010_w);
	DECLARE_READ8_MEMBER(rollerg_k051316_r);
	DECLARE_WRITE8_MEMBER(soundirq_w);
	DECLARE_WRITE8_MEMBER(sound_arm_nmi_w);
	DECLARE_READ8_MEMBER(pip_r);
};

/*----------- defined in video/rollerg.c -----------*/

extern void rollerg_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
extern void rollerg_zoom_callback(running_machine &machine, int *code,int *color,int *flags);

VIDEO_START( rollerg );
SCREEN_UPDATE_IND16( rollerg );
