
class xmen_state : public driver_device
{
public:
	xmen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT16 *   m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];

	/* for xmen6p */
	bitmap_t   *m_screen_right;
	bitmap_t   *m_screen_left;
	UINT16 *   m_xmen6p_spriteramleft;
	UINT16 *   m_xmen6p_spriteramright;
	UINT16 *   m_xmen6p_tilemapleft;
	UINT16 *   m_xmen6p_tilemapright;
	UINT16 *   m_k053247_ram;

	/* misc */
	UINT8       m_sound_curbank;
	UINT8       m_vblank_irq_mask;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k054539;
	device_t *m_k052109;
	device_t *m_k053246;
	device_t *m_k053251;
	device_t *m_lscreen;
	device_t *m_rscreen;
};

/*----------- defined in video/xmen.c -----------*/

void xmen_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
void xmen_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);

VIDEO_START( xmen6p );
SCREEN_UPDATE( xmen );
SCREEN_UPDATE( xmen6p );
SCREEN_EOF( xmen6p );
