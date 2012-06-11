/*************************************************************************

    GI Joe

*************************************************************************/

class gijoe_state : public driver_device
{
public:
	gijoe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_workram;
	UINT16 *    m_spriteram;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         m_avac_bits[4];
	int         m_avac_occupancy[4];
	int         m_layer_colorbase[4];
	int         m_layer_pri[4];
	int         m_avac_vrc;
	int         m_sprite_colorbase;

	/* misc */
	UINT16  	m_cur_control2;
	emu_timer	*m_dmadelay_timer;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k054539;
	device_t *m_k056832;
	device_t *m_k053246;
	device_t *m_k053251;
};

/*----------- defined in video/gijoe.c -----------*/

extern void gijoe_sprite_callback(running_machine &machine, int *code, int *color, int *priority_mask);
extern void gijoe_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);

VIDEO_START( gijoe );
SCREEN_UPDATE_IND16( gijoe );
