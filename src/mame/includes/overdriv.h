/*************************************************************************

    Over Drive

*************************************************************************/

class overdriv_state : public driver_device
{
public:
	overdriv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT16 *   m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int       m_zoom_colorbase[2];
	int       m_road_colorbase[2];
	int       m_sprite_colorbase;

	/* misc */
	UINT16     m_cpuB_ctrl;

	/* devices */
	device_t *m_maincpu;
	device_t *m_subcpu;
	device_t *m_audiocpu;
	device_t *m_k053260_1;
	device_t *m_k053260_2;
	device_t *m_k051316_1;
	device_t *m_k051316_2;
	device_t *m_k053246;
	device_t *m_k053251;
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_WRITE16_MEMBER(cpuA_ctrl_w);
	DECLARE_READ16_MEMBER(cpuB_ctrl_r);
	DECLARE_WRITE16_MEMBER(cpuB_ctrl_w);
	DECLARE_WRITE16_MEMBER(overdriv_soundirq_w);
	DECLARE_WRITE16_MEMBER(overdriv_cpuB_irq5_w);
	DECLARE_WRITE16_MEMBER(overdriv_cpuB_irq6_w);
};

/*----------- defined in video/overdriv.c -----------*/

extern void overdriv_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
extern void overdriv_zoom_callback_0(running_machine &machine, int *code,int *color,int *flags);
extern void overdriv_zoom_callback_1(running_machine &machine, int *code,int *color,int *flags);

SCREEN_UPDATE_IND16( overdriv );
