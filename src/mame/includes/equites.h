
#include "sound/msm5232.h"

#define POPDRUMKIT 0


class equites_state : public driver_device
{
public:
	equites_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_bg_videoram;
	UINT8  *  m_fg_videoram;	// 8bits
	UINT16 *  m_spriteram;
	UINT16 *  m_spriteram_2;
	UINT16 *  m_workram;
	UINT8  *  m_mcu_ram;	// 8bits
//  UINT16 *  m_nvram;    // currently this uses generic nvram handling

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int       m_fg_char_bank;
	UINT8     m_bgcolor;
	UINT16    m_splndrbt_bg_scrollx;
	UINT16	  m_splndrbt_bg_scrolly;

	/* misc */
	int       m_sound_prom_address;
	UINT8     m_dac_latch;
	UINT8     m_eq8155_port_b;
	UINT8     m_eq8155_port_a;
	UINT8     m_eq8155_port_c;
	UINT8     m_ay_port_a;
	UINT8     m_ay_port_b;
	UINT8     m_eq_cymbal_ctrl;
	emu_timer *m_nmi_timer;
	emu_timer *m_adjuster_timer;
	float     m_cymvol;
	float     m_hihatvol;
	int       m_timer_count;
	int       m_unknown_bit;	// Gekisou special handling
#if POPDRUMKIT
	int       m_hihat;
	int       m_cymbal;
#endif

	/* devices */
	device_t *m_mcu;
	device_t *m_audio_cpu;
	msm5232_device *m_msm;
	device_t *m_dac_1;
	device_t *m_dac_2;
};


/*----------- defined in video/equites.c -----------*/

extern READ16_HANDLER(equites_fg_videoram_r);
extern WRITE16_HANDLER(equites_fg_videoram_w);
extern WRITE16_HANDLER(equites_bg_videoram_w);
extern WRITE16_HANDLER(equites_scrollreg_w);
extern WRITE16_HANDLER(equites_bgcolor_w);
extern WRITE16_HANDLER(splndrbt_selchar0_w);
extern WRITE16_HANDLER(splndrbt_selchar1_w);
extern WRITE16_HANDLER(equites_flip0_w);
extern WRITE16_HANDLER(equites_flip1_w);
extern WRITE16_HANDLER(splndrbt_flip0_w);
extern WRITE16_HANDLER(splndrbt_flip1_w);
extern WRITE16_HANDLER(splndrbt_bg_scrollx_w);
extern WRITE16_HANDLER(splndrbt_bg_scrolly_w);

extern PALETTE_INIT( equites );
extern VIDEO_START( equites );
extern SCREEN_UPDATE_IND16( equites );
extern PALETTE_INIT( splndrbt );
extern VIDEO_START( splndrbt );
extern SCREEN_UPDATE_IND16( splndrbt );
