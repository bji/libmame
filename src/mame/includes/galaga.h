#include "sound/discrete.h"

class galaga_state : public driver_device
{
public:
	galaga_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *m_galaga_ram1;
	UINT8 *m_galaga_ram2;
	UINT8 *m_galaga_ram3;
	UINT8 *m_galaga_starcontrol;	// 6 addresses
	emu_timer *m_cpu3_interrupt_timer;
	UINT8 m_custom_mod;

	/* machine state */
	UINT32 m_stars_scrollx;
	UINT32 m_stars_scrolly;

	UINT32 m_galaga_gfxbank; // used by catsbee

	/* devices */

	/* bank support */

	/* shared */
	UINT8 *m_videoram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
};

class xevious_state : public galaga_state
{
public:
	xevious_state(running_machine &machine, const driver_device_config_base &config)
		: galaga_state(machine, config) { }

	UINT8 *m_xevious_fg_videoram;
	UINT8 *m_xevious_fg_colorram;
	UINT8 *m_xevious_bg_videoram;
	UINT8 *m_xevious_bg_colorram;
	UINT8 *m_xevious_sr1;
	UINT8 *m_xevious_sr2;
	UINT8 *m_xevious_sr3;

	INT32 m_xevious_bs[2];
};


class bosco_state : public galaga_state
{
public:
	bosco_state(running_machine &machine, const driver_device_config_base &config)
		: galaga_state(machine, config) { }

	UINT8 *m_bosco_radarattr;

	UINT8 *m_bosco_starcontrol;
	UINT8 *m_bosco_starblink;

	UINT8 *m_bosco_radarx;
	UINT8 *m_bosco_radary;
};

class digdug_state : public galaga_state
{
public:
	digdug_state(running_machine &machine, const driver_device_config_base &config)
		: galaga_state(machine, config) { }

	UINT8 *m_digdug_objram;
	UINT8 *m_digdug_posram;
	UINT8 *m_digdug_flpram;

	UINT8 m_bg_select;
	UINT8 m_tx_color_mode;
	UINT8 m_bg_disable;
	UINT8 m_bg_color_bank;
};



/*----------- defined in video/bosco.c -----------*/

WRITE8_HANDLER( bosco_videoram_w );
WRITE8_HANDLER( bosco_scrollx_w );
WRITE8_HANDLER( bosco_scrolly_w );
WRITE8_HANDLER( bosco_starclr_w );
VIDEO_START( bosco );
SCREEN_UPDATE( bosco );
PALETTE_INIT( bosco );
SCREEN_EOF( bosco );	/* update starfield */

/*----------- defined in audio/galaga.c -----------*/

DISCRETE_SOUND_EXTERN( bosco );
DISCRETE_SOUND_EXTERN( galaga );


/*----------- defined in video/galaga.c -----------*/

struct star
{
	UINT16 x,y;
	UINT8 col,set;
};

extern const struct star star_seed_tab[];

PALETTE_INIT( galaga );
WRITE8_HANDLER( galaga_videoram_w );
WRITE8_HANDLER( gatsbee_bank_w );
VIDEO_START( galaga );
SCREEN_UPDATE( galaga );
SCREEN_EOF( galaga );	/* update starfield */

/*----------- defined in video/xevious.c -----------*/

WRITE8_HANDLER( xevious_fg_videoram_w );
WRITE8_HANDLER( xevious_fg_colorram_w );
WRITE8_HANDLER( xevious_bg_videoram_w );
WRITE8_HANDLER( xevious_bg_colorram_w );
WRITE8_HANDLER( xevious_vh_latch_w );
WRITE8_HANDLER( xevious_bs_w );
READ8_HANDLER( xevious_bb_r );
VIDEO_START( xevious );
PALETTE_INIT( xevious );
SCREEN_UPDATE( xevious );

PALETTE_INIT( battles );

/*----------- defined in machine/xevious.c -----------*/

void battles_customio_init(running_machine &machine);
TIMER_DEVICE_CALLBACK( battles_nmi_generate );

READ8_HANDLER( battles_customio0_r );
READ8_HANDLER( battles_customio_data0_r );
READ8_HANDLER( battles_customio3_r );
READ8_HANDLER( battles_customio_data3_r );
READ8_HANDLER( battles_input_port_r );

WRITE8_HANDLER( battles_customio0_w );
WRITE8_HANDLER( battles_customio_data0_w );
WRITE8_HANDLER( battles_customio3_w );
WRITE8_HANDLER( battles_customio_data3_w );
WRITE8_HANDLER( battles_CPU4_coin_w );
WRITE8_HANDLER( battles_noise_sound_w );

INTERRUPT_GEN( battles_interrupt_4 );

/*----------- defined in video/digdug.c -----------*/

WRITE8_HANDLER( digdug_videoram_w );
WRITE8_HANDLER( digdug_PORT_w );
VIDEO_START( digdug );
SCREEN_UPDATE( digdug );
PALETTE_INIT( digdug );
