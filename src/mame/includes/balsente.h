/***************************************************************************

    Bally/Sente SAC-1 system

    driver by Aaron Giles

***************************************************************************/

#include "sound/cem3394.h"

#define BALSENTE_MASTER_CLOCK	(20000000)
#define BALSENTE_CPU_CLOCK		(BALSENTE_MASTER_CLOCK / 16)
#define BALSENTE_PIXEL_CLOCK	(BALSENTE_MASTER_CLOCK / 4)
#define BALSENTE_HTOTAL			(0x140)
#define BALSENTE_HBEND			(0x000)
#define BALSENTE_HBSTART		(0x100)
#define BALSENTE_VTOTAL			(0x108)
#define BALSENTE_VBEND			(0x010)
#define BALSENTE_VBSTART		(0x100)


#define POLY17_BITS 17
#define POLY17_SIZE ((1 << POLY17_BITS) - 1)
#define POLY17_SHL	7
#define POLY17_SHR	10
#define POLY17_ADD	0x18000


class balsente_state : public driver_device
{
public:
	balsente_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_scanline_timer(*this, "scan_timer"),
		  m_counter_0_timer(*this, "8253_0_timer"),
		  m_cem1(*this, "cem1"),
		  m_cem2(*this, "cem2"),
		  m_cem3(*this, "cem3"),
		  m_cem4(*this, "cem4"),
		  m_cem5(*this, "cem5"),
		  m_cem6(*this, "cem6") { }

	/* global data */
	UINT8 m_shooter;
	UINT8 m_shooter_x;
	UINT8 m_shooter_y;
	UINT8 m_adc_shift;
	UINT16 *m_shrike_shared;
	UINT16 *m_shrike_io;

	/* 8253 counter state */
	struct
	{
		timer_device *timer;
		UINT8 timer_active;
		INT32 initial;
		INT32 count;
		UINT8 gate;
		UINT8 out;
		UINT8 mode;
		UINT8 readbyte;
		UINT8 writebyte;
	} m_counter[3];

	required_device<timer_device> m_scanline_timer;

	/* manually clocked counter 0 states */
	UINT8 m_counter_control;
	UINT8 m_counter_0_ff;
	required_device<timer_device> m_counter_0_timer;
	UINT8 m_counter_0_timer_active;

	/* random number generator states */
	UINT8 m_poly17[POLY17_SIZE + 1];
	UINT8 m_rand17[POLY17_SIZE + 1];

	/* ADC I/O states */
	INT8 m_analog_input_data[4];
	UINT8 m_adc_value;

	/* CEM3394 DAC control states */
	UINT16 m_dac_value;
	UINT8 m_dac_register;
	UINT8 m_chip_select;

	/* main CPU 6850 states */
	UINT8 m_m6850_status;
	UINT8 m_m6850_control;
	UINT8 m_m6850_input;
	UINT8 m_m6850_output;
	UINT8 m_m6850_data_ready;

	/* sound CPU 6850 states */
	UINT8 m_m6850_sound_status;
	UINT8 m_m6850_sound_control;
	UINT8 m_m6850_sound_input;
	UINT8 m_m6850_sound_output;

	/* noise generator states */
	UINT32 m_noise_position[6];
	required_device<cem3394_device> m_cem1;
	required_device<cem3394_device> m_cem2;
	required_device<cem3394_device> m_cem3;
	required_device<cem3394_device> m_cem4;
	required_device<cem3394_device> m_cem5;
	required_device<cem3394_device> m_cem6;
	cem3394_device *m_cem_device[6];

	/* game-specific states */
	UINT8 m_nstocker_bits;
	UINT8 m_spiker_expand_color;
	UINT8 m_spiker_expand_bgcolor;
	UINT8 m_spiker_expand_bits;
	UINT8 m_grudge_steering_result;
	UINT8 m_grudge_last_steering[3];

	/* video data */
	UINT8 *m_videoram;
	UINT8 m_expanded_videoram[256*256];
	UINT8 *m_sprite_data;
	UINT32 m_sprite_mask;
	UINT8 *m_sprite_bank[2];

	UINT8 m_palettebank_vis;
	UINT8 *m_spriteram;
};


/*----------- defined in machine/balsente.c -----------*/

TIMER_DEVICE_CALLBACK( balsente_interrupt_timer );

MACHINE_START( balsente );
MACHINE_RESET( balsente );

void balsente_noise_gen(device_t *device, int count, short *buffer);

WRITE8_HANDLER( balsente_random_reset_w );
READ8_HANDLER( balsente_random_num_r );

WRITE8_HANDLER( balsente_rombank_select_w );
WRITE8_HANDLER( balsente_rombank2_select_w );

WRITE8_HANDLER( balsente_misc_output_w );

READ8_HANDLER( balsente_m6850_r );
WRITE8_HANDLER( balsente_m6850_w );

READ8_HANDLER( balsente_m6850_sound_r );
WRITE8_HANDLER( balsente_m6850_sound_w );

INTERRUPT_GEN( balsente_update_analog_inputs );
READ8_HANDLER( balsente_adc_data_r );
WRITE8_HANDLER( balsente_adc_select_w );

TIMER_DEVICE_CALLBACK( balsente_counter_callback );

READ8_HANDLER( balsente_counter_8253_r );
WRITE8_HANDLER( balsente_counter_8253_w );

TIMER_DEVICE_CALLBACK( balsente_clock_counter_0_ff );

READ8_HANDLER( balsente_counter_state_r );
WRITE8_HANDLER( balsente_counter_control_w );

WRITE8_HANDLER( balsente_chip_select_w );
WRITE8_HANDLER( balsente_dac_data_w );
WRITE8_HANDLER( balsente_register_addr_w );

CUSTOM_INPUT( nstocker_bits_r );
WRITE8_HANDLER( spiker_expand_w );
READ8_HANDLER( spiker_expand_r );
READ8_HANDLER( grudge_steering_r );

READ8_HANDLER( shrike_shared_6809_r );
WRITE8_HANDLER( shrike_shared_6809_w );

READ16_HANDLER( shrike_io_68k_r );
WRITE16_HANDLER( shrike_io_68k_w );


/*----------- defined in video/balsente.c -----------*/

VIDEO_START( balsente );
SCREEN_UPDATE( balsente );

WRITE8_HANDLER( balsente_videoram_w );
WRITE8_HANDLER( balsente_paletteram_w );
WRITE8_HANDLER( balsente_palette_select_w );
WRITE8_HANDLER( shrike_sprite_select_w );
