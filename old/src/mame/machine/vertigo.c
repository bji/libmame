/*************************************************************************

    Exidy Vertigo hardware

*************************************************************************/

#include "emu.h"
#include "machine/74148.h"
#include "machine/pit8253.h"
#include "audio/exidy440.h"
#include "includes/vertigo.h"


/*************************************
 *
 *  Prototypes
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( v_irq4_w );
static WRITE_LINE_DEVICE_HANDLER( v_irq3_w );



/*************************************
 *
 *  Statics
 *
 *************************************/

/* Timestamp of last INTL4 change. The vector CPU runs for
   the delta between this and now.
*/

/* State of the priority encoder output */

/* Result of the last ADC channel sampled */

/* 8254 timer config */
const struct pit8253_config vertigo_pit8254_config =
{
	{
		{
			240000,
			DEVCB_NULL,
			DEVCB_LINE(v_irq4_w)
		}, {
			240000,
			DEVCB_NULL,
			DEVCB_LINE(v_irq3_w)
		}, {
			240000,
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};



/*************************************
 *
 *  IRQ handling. The priority encoder
 *  has to be emulated. Otherwise
 *  interrupts are lost.
 *
 *************************************/

void vertigo_update_irq(device_t *device)
{
	vertigo_state *state = device->machine->driver_data<vertigo_state>();
	if (state->irq_state < 7)
		cputag_set_input_line(device->machine, "maincpu", state->irq_state ^ 7, CLEAR_LINE);

	state->irq_state = ttl74148_output_r(device);

	if (state->irq_state < 7)
		cputag_set_input_line(device->machine, "maincpu", state->irq_state ^ 7, ASSERT_LINE);
}


static void update_irq_encoder(running_machine *machine, int line, int state)
{
	vertigo_state *drvstate = machine->driver_data<vertigo_state>();
	ttl74148_input_line_w(drvstate->ttl74148, line, !state);
	ttl74148_update(drvstate->ttl74148);
}


static WRITE_LINE_DEVICE_HANDLER( v_irq4_w )
{
	vertigo_state *drvstate = device->machine->driver_data<vertigo_state>();
	update_irq_encoder(device->machine, INPUT_LINE_IRQ4, state);
	vertigo_vproc(device->machine, device->machine->device<cpu_device>("maincpu")->attotime_to_cycles(device->machine->time() - drvstate->irq4_time), state);
	drvstate->irq4_time = device->machine->time();
}


static WRITE_LINE_DEVICE_HANDLER( v_irq3_w )
{
	if (state)
		cputag_set_input_line(device->machine, "audiocpu", INPUT_LINE_IRQ0, ASSERT_LINE);

	update_irq_encoder(device->machine, INPUT_LINE_IRQ3, state);
}



/*************************************
 *
 *  ADC and coin handlers
 *
 *************************************/

READ16_HANDLER( vertigo_io_convert )
{
	vertigo_state *state = space->machine->driver_data<vertigo_state>();
	static const char *const adcnames[] = { "P1X", "P1Y", "PADDLE" };

	if (offset > 2)
		state->adc_result = 0;
	else
		state->adc_result = input_port_read(space->machine, adcnames[offset]);

	update_irq_encoder(space->machine, INPUT_LINE_IRQ2, ASSERT_LINE);
	return 0;
}


READ16_HANDLER( vertigo_io_adc )
{
	vertigo_state *state = space->machine->driver_data<vertigo_state>();
	update_irq_encoder(space->machine, INPUT_LINE_IRQ2, CLEAR_LINE);
	return state->adc_result;
}


READ16_HANDLER( vertigo_coin_r )
{
	update_irq_encoder(space->machine, INPUT_LINE_IRQ6, CLEAR_LINE);
	return (input_port_read(space->machine, "COIN"));
}


INTERRUPT_GEN( vertigo_interrupt )
{
	/* Coin inputs cause IRQ6 */
	if ((input_port_read(device->machine, "COIN") & 0x7) < 0x7)
		update_irq_encoder(device->machine, INPUT_LINE_IRQ6, ASSERT_LINE);
}



/*************************************
 *
 *  Sound board interface
 *
 *************************************/

WRITE16_HANDLER( vertigo_wsot_w )
{
	/* Reset sound cpu */
	if ((data & 2) == 0)
		cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);
	else
		cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, CLEAR_LINE);
}


static TIMER_CALLBACK( sound_command_w )
{
	exidy440_sound_command = param;
	exidy440_sound_command_ack = 0;
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_IRQ1, ASSERT_LINE);

	/* It is important that the sound cpu ACKs the sound command
       quickly. Otherwise the main CPU gives up with sound. Boosting
       the interleave for a while helps. */

	machine->scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}


WRITE16_HANDLER( vertigo_audio_w )
{
	if (ACCESSING_BITS_0_7)
		space->machine->scheduler().synchronize(FUNC(sound_command_w), data & 0xff);
}


READ16_HANDLER( vertigo_sio_r )
{
	return exidy440_sound_command_ack ? 0xfc : 0xfd;
}



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

MACHINE_START( vertigo )
{
	vertigo_state *state = machine->driver_data<vertigo_state>();
	state_save_register_global(machine, state->irq_state);
	state_save_register_global(machine, state->adc_result);
	state_save_register_global(machine, state->irq4_time);

	vertigo_vproc_init(machine);
}

MACHINE_RESET( vertigo )
{
	vertigo_state *state = machine->driver_data<vertigo_state>();
	int i;

	state->ttl74148 = machine->device("74148");
	ttl74148_enable_input_w(state->ttl74148, 0);

	for (i = 0; i < 8; i++)
		ttl74148_input_line_w(state->ttl74148, i, 1);

	ttl74148_update(state->ttl74148);
	vertigo_vproc_reset(machine);

	state->irq4_time = machine->time();
	state->irq_state = 7;
}



/*************************************
 *
 *  Motor controller interface
 *
 *************************************/

WRITE16_HANDLER( vertigo_motor_w )
{
	/* Motor controller interface. Not emulated. */
}
