/*************************************************************************

    Bishi Bashi Champ Mini Game Senshuken

*************************************************************************/

#define CPU_CLOCK       (XTAL_24MHz / 2)		/* 68000 clock */
#define SOUND_CLOCK     XTAL_16_9344MHz		/* YMZ280 clock */

class bishi_state : public driver_device
{
public:
	bishi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_ram;
//  UINT8 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[4];

	/* misc */
	UINT16     m_cur_control;
	UINT16     m_cur_control2;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k007232;
	device_t *m_k056832;
	device_t *m_k054338;
	device_t *m_k055555;
	DECLARE_READ16_MEMBER(control_r);
	DECLARE_WRITE16_MEMBER(control_w);
	DECLARE_WRITE16_MEMBER(control2_w);
	DECLARE_READ16_MEMBER(bishi_mirror_r);
	DECLARE_READ16_MEMBER(bishi_K056832_rom_r);
};

/*----------- defined in video/bishi.c -----------*/

extern void bishi_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags);

VIDEO_START(bishi);
SCREEN_UPDATE_RGB32(bishi);
