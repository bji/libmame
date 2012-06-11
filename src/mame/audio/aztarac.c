/***************************************************************************

    Centuri Aztarac hardware

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/aztarac.h"


READ16_MEMBER(aztarac_state::aztarac_sound_r)
{
    return m_sound_status & 0x01;
}

WRITE16_MEMBER(aztarac_state::aztarac_sound_w)
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xff;
		soundlatch_byte_w(space, offset, data);
		m_sound_status ^= 0x21;
		if (m_sound_status & 0x20)
			cputag_set_input_line(machine(), "audiocpu", 0, HOLD_LINE);
	}
}

READ8_MEMBER(aztarac_state::aztarac_snd_command_r)
{
    m_sound_status |= 0x01;
    m_sound_status &= ~0x20;
    return soundlatch_byte_r(space,offset);
}

READ8_MEMBER(aztarac_state::aztarac_snd_status_r)
{
    return m_sound_status & ~0x01;
}

WRITE8_MEMBER(aztarac_state::aztarac_snd_status_w)
{
    m_sound_status &= ~0x10;
}

INTERRUPT_GEN( aztarac_snd_timed_irq )
{
	aztarac_state *state = device->machine().driver_data<aztarac_state>();
    state->m_sound_status ^= 0x10;

    if (state->m_sound_status & 0x10)
        device_set_input_line(device,0,HOLD_LINE);
}
