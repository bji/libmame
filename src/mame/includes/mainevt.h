/*************************************************************************

    The Main Event / Devastators

*************************************************************************/

class mainevt_state : public driver_device
{
public:
	mainevt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;

	/* misc */
	int        m_nmi_enable;
	UINT8      m_sound_irq_mask;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_upd;
	device_t *m_k007232;
	device_t *m_k052109;
	device_t *m_k051960;
	DECLARE_WRITE8_MEMBER(dv_nmienable_w);
	DECLARE_WRITE8_MEMBER(mainevt_bankswitch_w);
	DECLARE_WRITE8_MEMBER(mainevt_coin_w);
	DECLARE_WRITE8_MEMBER(mainevt_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(mainevt_sh_irqcontrol_w);
	DECLARE_WRITE8_MEMBER(devstor_sh_irqcontrol_w);
	DECLARE_WRITE8_MEMBER(mainevt_sh_bankswitch_w);
	DECLARE_READ8_MEMBER(k052109_051960_r);
	DECLARE_WRITE8_MEMBER(k052109_051960_w);
};

/*----------- defined in video/mainevt.c -----------*/

extern void mainevt_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void dv_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void mainevt_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask,int *shadow);
extern void dv_sprite_callback(running_machine &machine, int *code,int *color,int *priority,int *shadow);

VIDEO_START( mainevt );
VIDEO_START( dv );

SCREEN_UPDATE_IND16( mainevt );
SCREEN_UPDATE_IND16( dv );
