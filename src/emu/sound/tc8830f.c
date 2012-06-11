/***************************************************************************

    tc8830f.c - Toshiba TC8830F, CMOS voice recording/reproducing LSI
    1-bit ADM (Adaptive Delta Modulation), similar to TC8801 and T6668.

    Very preliminary...

    TODO:
    - ADM decoder
    - remaining commands
    - manual control
    - chip read
    - RAM
    - recording

***************************************************************************/

#include "emu.h"
#include "tc8830f.h"


// device type definition
const device_type TC8830F = &device_creator<tc8830f_device>;

tc8830f_device::tc8830f_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC8830F, "TC8830F", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
}


void tc8830f_device::device_start()
{
	// create the stream
	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock() / 0x10, this);

	m_mem_base = (UINT8 *)device().machine().root_device().memregion(":tc8830f")->base();
	m_mem_mask = device().machine().root_device().memregion(":tc8830f")->bytes() - 1;

	// register for savestates
	save_item(NAME(m_playing));
	save_item(NAME(m_address));
	save_item(NAME(m_stop_address));
	save_item(NAME(m_bitcount));
	save_item(NAME(m_bitrate));
	save_item(NAME(m_command));
	save_item(NAME(m_cmd_rw));
	save_item(NAME(m_phrase));

	reset();
}


void tc8830f_device::device_post_load()
{
	device_clock_changed();
}


void tc8830f_device::device_clock_changed()
{
	int divisor = 0x10 * (4 - (m_bitrate & 3));
	m_stream->set_sample_rate(clock() / divisor);
}



void tc8830f_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	for (int i = 0; i < samples; i++)
	{
		int mix = 0;
		if (m_playing)
		{
			// get bit
			int bit = m_mem_base[m_address] >> m_bitcount & 1;
			m_bitcount = (m_bitcount + 1) & 7;
			if (m_bitcount == 0)
			{
				m_address = (m_address + 1) & m_mem_mask;
				if (m_address == m_stop_address)
					m_playing = false;
			}

			// compute sample
			// this is a placeholder until ADM is implemented
			mix = bit * 0x7fff;
		}
		outputs[0][i] = mix;
	}
}


void tc8830f_device::reset()
{
	m_stream->update();

	m_playing = false;
	m_address = 0x100;
	m_bitcount = 0;
	m_cmd_rw = 0;

	// in cpu control, enter play mode and reset bitrate
	write_p(1);
	write_p(6);
	write_p(0);
}


void tc8830f_device::write_p(UINT8 data)
{
	m_stream->update();
	data &= 0xf;

	if (m_cmd_rw == 0)
	{
		// select command
		m_command = data;
		switch (m_command)
		{
			// immediate commands
			case 0x1:
				break;

			case 0x2:
				m_playing = true;
				break;

			case 0x3:
				m_playing = false;
				break;

			// multi-nibble commands
			case 0x4: case 0x5: case 0x6: case 0x7:
				m_cmd_rw = 1;
				break;

			case 0x8: case 0x9: case 0xa: case 0xb:
				logerror("tc8830f: Unemulated command %X\n", m_command);
				break;

			default:
				logerror("tc8830f: Invalid command %X\n", m_command);
				break;
		}
	}

	else
	{
		// write command
		switch (m_command)
		{
			case 0x4:
				// ADLD1: set address counter
				m_address = (m_address & ~(0xf << (m_cmd_rw*4))) | (data << (m_cmd_rw*4));
				if (m_cmd_rw == 5)
				{
					m_address &= m_mem_mask;
					m_bitcount = 0;
					m_cmd_rw = -1;
				}
				break;

			case 0x5:
				// ADLD2: set address stop
				m_stop_address = (m_stop_address & ~(0xf << (m_cmd_rw*4))) | (data << (m_cmd_rw*4));
				if (m_cmd_rw == 5)
				{
					m_stop_address &= m_mem_mask;
					m_cmd_rw = -1;
				}
				break;

			case 0x6:
				// CNDT: d0-d1: bitrate, d2: enable overflow
				m_bitrate = data & 3;
				device_clock_changed();
				m_cmd_rw = -1;
				break;

			case 0x7:
				// LABEL: set phrase
				if (m_cmd_rw == 1)
				{
					m_phrase = (m_phrase & 0x30) | data;
				}
				else
				{
					m_phrase = (m_phrase & 0x0f) | (data << 4 & 0x30);

					// update addresses and start
					UINT8 offs = m_phrase * 4;
					m_address = (m_mem_base[offs] | m_mem_base[offs|1]<<8 | m_mem_base[offs|2]<<16) & m_mem_mask;
					offs += 4;
					m_stop_address = (m_mem_base[offs] | m_mem_base[offs|1]<<8 | m_mem_base[offs|2]<<16) & m_mem_mask;

					m_bitcount = 0;
					m_playing = true;
					m_cmd_rw = -1;
				}
				break;

			default:
				m_cmd_rw = -1;
				break;
		}
		m_cmd_rw++;
	}
}
