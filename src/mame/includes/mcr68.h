#include "machine/6821pia.h"

struct counter_state
{
	UINT8			control;
	UINT16			latch;
	UINT16			count;
	emu_timer *		timer;
	UINT8			timer_active;
	attotime		period;
};

class mcr68_state : public driver_device
{
public:
	mcr68_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_videoram;
	UINT16 m_control_word;
	UINT8 m_protection_data[5];
	attotime m_timing_factor;
	UINT8 m_sprite_clip;
	INT8 m_sprite_xoffset;
	UINT8 m_m6840_status;
	UINT8 m_m6840_status_read_since_int;
	UINT8 m_m6840_msb_buffer;
	UINT8 m_m6840_lsb_buffer;
	UINT8 m_m6840_irq_state;
	UINT8 m_m6840_irq_vector;
	struct counter_state m_m6840_state[3];
	UINT8 m_v493_irq_state;
	UINT8 m_v493_irq_vector;
	timer_expired_func m_v493_callback;
	UINT8 m_zwackery_sound_data;
	attotime m_m6840_counter_periods[3];
	attotime m_m6840_internal_counter_period;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in drivers/mcr68.c -----------*/

READ8_DEVICE_HANDLER( zwackery_port_2_r );


/*----------- defined in machine/mcr68.c -----------*/

extern const pia6821_interface zwackery_pia0_intf;
extern const pia6821_interface zwackery_pia1_intf;
extern const pia6821_interface zwackery_pia2_intf;


MACHINE_START( mcr68 );
MACHINE_RESET( mcr68 );
MACHINE_START( zwackery );
MACHINE_RESET( zwackery );

WRITE16_HANDLER( mcr68_6840_upper_w );
WRITE16_HANDLER( mcr68_6840_lower_w );
READ16_HANDLER( mcr68_6840_upper_r );
READ16_HANDLER( mcr68_6840_lower_r );

INTERRUPT_GEN( mcr68_interrupt );


/*----------- defined in video/mcr68.c -----------*/

WRITE16_HANDLER( mcr68_paletteram_w );
WRITE16_HANDLER( mcr68_videoram_w );

VIDEO_START( mcr68 );
SCREEN_UPDATE( mcr68 );

WRITE16_HANDLER( zwackery_paletteram_w );
WRITE16_HANDLER( zwackery_videoram_w );
WRITE16_HANDLER( zwackery_spriteram_w );

VIDEO_START( zwackery );
SCREEN_UPDATE( zwackery );
