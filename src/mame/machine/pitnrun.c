/***************************************************************************

    Pit&Run

    Based on TaitsoSJ driver
    68705 has access to Z80 memory space

***************************************************************************/

#include "emu.h"
#include "cpu/m6805/m6805.h"
#include "includes/pitnrun.h"


MACHINE_RESET( pitnrun )
{
	pitnrun_state *state = machine.driver_data<pitnrun_state>();
	state->m_zaccept = 1;
	state->m_zready = 0;
	cputag_set_input_line(machine, "mcu", 0, CLEAR_LINE);
}

static TIMER_CALLBACK( pitnrun_mcu_real_data_r )
{
	pitnrun_state *state = machine.driver_data<pitnrun_state>();
	state->m_zaccept = 1;
}

READ8_HANDLER( pitnrun_mcu_data_r )
{
	pitnrun_state *state = space->machine().driver_data<pitnrun_state>();
	space->machine().scheduler().synchronize(FUNC(pitnrun_mcu_real_data_r));
	return state->m_toz80;
}

static TIMER_CALLBACK( pitnrun_mcu_real_data_w )
{
	pitnrun_state *state = machine.driver_data<pitnrun_state>();
	state->m_zready = 1;
	cputag_set_input_line(machine, "mcu", 0, ASSERT_LINE);
	state->m_fromz80 = param;
}

WRITE8_HANDLER( pitnrun_mcu_data_w )
{
	space->machine().scheduler().synchronize(FUNC(pitnrun_mcu_real_data_w), data);
	space->machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(5));
}

READ8_HANDLER( pitnrun_mcu_status_r )
{
	pitnrun_state *state = space->machine().driver_data<pitnrun_state>();
	/* mcu synchronization */
	/* bit 0 = the 68705 has read data from the Z80 */
	/* bit 1 = the 68705 has written data for the Z80 */
	return ~((state->m_zready << 1) | (state->m_zaccept << 0));
}


READ8_HANDLER( pitnrun_68705_portA_r )
{
	pitnrun_state *state = space->machine().driver_data<pitnrun_state>();
	return state->m_portA_in;
}

WRITE8_HANDLER( pitnrun_68705_portA_w )
{
	pitnrun_state *state = space->machine().driver_data<pitnrun_state>();
	state->m_portA_out = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   W  !68INTRQ
 *  1   W  !68LRD (enables latch which holds command from the Z80)
 *  2   W  !68LWR (loads the latch which holds data for the Z80, and sets a
 *                 status bit so the Z80 knows there's data waiting)
 *  3   W  to Z80 !BUSRQ (aka !WAIT) pin
 *  4   W  !68WRITE (triggers write to main Z80 memory area )
 *  5   W  !68READ (triggers read from main Z80 memory area )
 *  6   W  !LAL (loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access)
 *  7   W  !UAL (loads the latch which holds the high 8 bits of the address of
 *               the main Z80 memory location to access)
 */

READ8_HANDLER( pitnrun_68705_portB_r )
{
	return 0xff;
}


static TIMER_CALLBACK( pitnrun_mcu_data_real_r )
{
	pitnrun_state *state = machine.driver_data<pitnrun_state>();
	state->m_zready = 0;
}

static TIMER_CALLBACK( pitnrun_mcu_status_real_w )
{
	pitnrun_state *state = machine.driver_data<pitnrun_state>();
	state->m_toz80 = param;
	state->m_zaccept = 0;
}

WRITE8_HANDLER( pitnrun_68705_portB_w )
{
	pitnrun_state *state = space->machine().driver_data<pitnrun_state>();
	address_space *cpu0space = space->machine().device("maincpu")->memory().space(AS_PROGRAM);
	if (~data & 0x02)
	{
		/* 68705 is going to read data from the Z80 */
		space->machine().scheduler().synchronize(FUNC(pitnrun_mcu_data_real_r));
		cputag_set_input_line(space->machine(), "mcu",0,CLEAR_LINE);
		state->m_portA_in = state->m_fromz80;
	}
	if (~data & 0x04)
	{
		/* 68705 is writing data for the Z80 */
		space->machine().scheduler().synchronize(FUNC(pitnrun_mcu_status_real_w), state->m_portA_out);
	}
	if (~data & 0x10)
	{
		cpu0space->write_byte(state->m_address, state->m_portA_out);
	}
	if (~data & 0x20)
	{
		state->m_portA_in = cpu0space->read_byte(state->m_address);
	}
	if (~data & 0x40)
	{
		state->m_address = (state->m_address & 0xff00) | state->m_portA_out;
	}
	if (~data & 0x80)
	{
		state->m_address = (state->m_address & 0x00ff) | (state->m_portA_out << 8);
	}
}

/*
 *  Port C connections:
 *
 *  0   R  ZREADY (1 when the Z80 has written a command in the latch)
 *  1   R  ZACCEPT (1 when the Z80 has read data from the latch)
 *  2   R  from Z80 !BUSAK pin
 *  3   R  68INTAK (goes 0 when the interrupt request done with 68INTRQ
 *                  passes through)
 */

READ8_HANDLER( pitnrun_68705_portC_r )
{
	pitnrun_state *state = space->machine().driver_data<pitnrun_state>();
	return (state->m_zready << 0) | (state->m_zaccept << 1);
}
