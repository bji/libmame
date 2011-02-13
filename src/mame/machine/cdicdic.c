/******************************************************************************


    CD-i Mono-I CDIC MCU simulation
    -------------------

    MESS implementation by Harmony


*******************************************************************************

STATUS:

- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:

- Decapping and proper emulation.

*******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cdrom.h"
#include "machine/cdicdic.h"
#include "includes/cdi.h"
#include "sound/cdda.h"

#if ENABLE_VERBOSE_LOG
INLINE void verboselog(running_machine *machine, int n_level, const char *s_fmt, ...)
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%08x: %s", cpu_get_pc(machine->device("maincpu")), buf );
	}
}
#else
#define verboselog(x,y,z,...)
#endif

#define CDIC_SECTOR_SYNC        0

#define CDIC_SECTOR_HEADER      12

#define CDIC_SECTOR_MODE        15

#define CDIC_SECTOR_FILE1       16
#define CDIC_SECTOR_CHAN1       17
#define CDIC_SECTOR_SUBMODE1    18
#define CDIC_SECTOR_CODING1     19

#define CDIC_SECTOR_FILE2       20
#define CDIC_SECTOR_CHAN2       21
#define CDIC_SECTOR_SUBMODE2    22
#define CDIC_SECTOR_CODING2     23

#define CDIC_SECTOR_DATA        24

#define CDIC_SECTOR_SIZE        2352

#define CDIC_SECTOR_DATASIZE    2048
#define CDIC_SECTOR_AUDIOSIZE   2304
#define CDIC_SECTOR_VIDEOSIZE   2324

#define CDIC_SUBMODE_EOF        0x80
#define CDIC_SUBMODE_RT         0x40
#define CDIC_SUBMODE_FORM       0x20
#define CDIC_SUBMODE_TRIG       0x10
#define CDIC_SUBMODE_DATA       0x08
#define CDIC_SUBMODE_AUDIO      0x04
#define CDIC_SUBMODE_VIDEO      0x02
#define CDIC_SUBMODE_EOR        0x01

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const INT32 cdicdic_device::s_cdic_adpcm_filter_coef[5][2] =
{
	{ 0,0 },
	{ 60,0 },
	{ 115,-52 },
	{ 98,-55 },
	{ 122,-60 },
};

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  cdicdic_device_config - constructor
//-------------------------------------------------

cdicdic_device_config::cdicdic_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
    : device_config(mconfig, static_alloc_device_config, "CDICDIC", tag, owner, clock)
{

}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *cdicdic_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
    return global_alloc(cdicdic_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *cdicdic_device_config::alloc_device(running_machine &machine) const
{
    return auto_alloc(&machine, cdicdic_device(machine, *this));
}


//**************************************************************************
//  INLINES
//**************************************************************************

INLINE int CDIC_IS_VALID_SAMPLE_BUF(UINT16 *cdram, UINT16 addr)
{
	UINT8 *cdram8 = ((UINT8*)cdram) + addr + 8;
	if(cdram8[2] != 0xff)
	{
		return 1;
	}
	return 0;
}

INLINE double CDIC_SAMPLE_BUF_FREQ(UINT16 *cdram, UINT16 addr)
{
	UINT8 *cdram8 = ((UINT8*)cdram) + addr + 8;
	switch(cdram8[2] & 0x3f)
	{
		case 0:
		case 1:
		case 16:
		case 17:
			return 37800.0f;

		case 4:
		case 5:
			return 18900.0f;

		default:
			return 18900.0f;
	}
}

INLINE int CDIC_SAMPLE_BUF_SIZE(UINT16 *cdram, UINT16 addr)
{
	UINT8 *cdram8 = ((UINT8*)cdram) + addr + 8;
	switch(cdram8[2] & 0x3f)
	{
		case 0:
		case 4:
			return 4;

		case 1:
		case 5:
		case 16:
			return 2;

		case 17:
			return 1;

		default:
			return 2;
	}
}

INLINE INT16 clamp(INT16 in)
{
	return in;
}

//**************************************************************************
//  MEMBER FUNCTIONS
//**************************************************************************

UINT32 cdicdic_device::increment_cdda_frame_bcd(UINT32 bcd)
{
    UINT8 nybbles[6] =
    {
         bcd & 0x0000000f,
        (bcd & 0x000000f0) >> 4,
        (bcd & 0x00000f00) >> 8,
        (bcd & 0x0000f000) >> 12,
        (bcd & 0x000f0000) >> 16,
        (bcd & 0x00f00000) >> 20
    };
    nybbles[0]++;
    if(nybbles[0] == 5 && nybbles[1] == 7)
    {
        nybbles[0] = 0;
        nybbles[1] = 0;
        nybbles[2]++;
    }
    else if(nybbles[0] == 10)
    {
        nybbles[1]++;
    }
    if(nybbles[2] == 10)
    {
        nybbles[3]++;
        nybbles[2] = 0;
    }
    if(nybbles[3] == 6)
    {
        nybbles[4]++;
        nybbles[3] = 0;
    }
    if(nybbles[4] == 10)
    {
        nybbles[5]++;
        nybbles[4] = 0;
    }
    return (nybbles[5] << 20) | (nybbles[4] << 16) | (nybbles[3] << 12) | (nybbles[2] << 8) | (nybbles[1] << 4) | nybbles[0];
}

UINT32 cdicdic_device::increment_cdda_sector_bcd(UINT32 bcd)
{
    UINT8 nybbles[6] =
    {
         bcd & 0x0000000f,
        (bcd & 0x000000f0) >> 4,
        (bcd & 0x00000f00) >> 8,
        (bcd & 0x0000f000) >> 12,
        (bcd & 0x000f0000) >> 16,
        (bcd & 0x00f00000) >> 20
    };
    nybbles[2]++;
    if(nybbles[2] == 10)
    {
        nybbles[3]++;
        nybbles[2] = 0;
    }
    if(nybbles[3] == 6)
    {
        nybbles[4]++;
        nybbles[3] = 0;
    }
    if(nybbles[4] == 10)
    {
        nybbles[5]++;
        nybbles[4] = 0;
    }
    return (nybbles[5] << 20) | (nybbles[4] << 16) | (nybbles[3] << 12) | (nybbles[2] << 8) | (nybbles[1] << 4) | nybbles[0];
}

void cdicdic_device::decode_xa_mono(INT32 *cdic_xa_last, const UINT8 *xa, INT16 *dp)
{
	INT32 l0 = cdic_xa_last[0];
    INT32 l1 = cdic_xa_last[1];

	for(INT32 b = 0; b < 18; b++)
	{
		for(INT32 s = 0; s < 4; s++)
		{
            UINT8 flags = xa[(4 + (s << 1)) ^ 1];
            UINT8 shift = flags & 0xf;
            UINT8 filter = flags >> 4;
            INT32 f0 = s_cdic_adpcm_filter_coef[filter][0];
            INT32 f1 = s_cdic_adpcm_filter_coef[filter][1];

			for(INT32 i = 0; i < 28; i++)
			{
				INT16 d = (xa[(16 + (i << 2) + s) ^ 1] & 0xf) << 12;
				d = clamp((d >> shift) + (((l0 * f0) + (l1 * f1) + 32) >> 6));
				*dp = d;
                dp++;
				l1 = l0;
				l0 = d;
			}

			flags = xa[(5 + (s << 1)) ^ 1];
			shift = flags & 0xf;
			filter = flags >> 4;
            f0 = s_cdic_adpcm_filter_coef[filter][0];
            f1 = s_cdic_adpcm_filter_coef[filter][1];

			for(INT32 i = 0; i < 28; i++)
			{
				INT16 d = (xa[(16 + (i << 2) + s) ^ 1] >> 4) << 12;
				d = clamp((d >> shift) + (((l0 * f0) + (l1 * f1) + 32) >> 6));
				*dp = d;
                dp++;
				l1 = l0;
				l0 = d;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
}

void cdicdic_device::decode_xa_mono8(int *cdic_xa_last, const unsigned char *xa, signed short *dp)
{
	INT32 l0 = cdic_xa_last[0];
    INT32 l1 = cdic_xa_last[1];

	for(INT32 b = 0; b < 18; b++)
	{
		for(INT32 s = 0; s < 4; s++)
		{
			UINT8 flags = xa[(4 + s) ^ 1];
            UINT8 shift = flags & 0xf;
            UINT8 filter = flags >> 4;
            INT32 f0 = s_cdic_adpcm_filter_coef[filter][0];
            INT32 f1 = s_cdic_adpcm_filter_coef[filter][1];

			for(INT32 i = 0; i < 28; i++)
			{
				INT16 d = (xa[(16 + (i << 2) + s) ^ 1] << 8);
				d = clamp((d >> shift) + (((l0 * f0) + (l1 * f1) + 32) >> 6));
				*dp = d;
                dp++;
				l1 = l0;
				l0 = d;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
}

void cdicdic_device::decode_xa_stereo(INT32 *cdic_xa_last, const UINT8 *xa, INT16 *dp)
{
	INT32 l0=cdic_xa_last[0];
    INT32 l1=cdic_xa_last[1];
    INT32 l2=cdic_xa_last[2];
    INT32 l3=cdic_xa_last[3];

	for(INT32 b = 0; b < 18; b++)
	{
		for(INT32 s = 0; s < 4; s++)
		{
			UINT8 flags0 = xa[(4 + (s << 1)) ^ 1];
            UINT8 shift0 = flags0 & 0xf;
            UINT8 filter0 = flags0 >> 4;
            UINT8 flags1 = xa[(5 + (s << 1)) ^ 1];
            UINT8 shift1 = flags1 & 0xf;
            UINT8 filter1 = flags1 >> 4;

            INT32 f0 = s_cdic_adpcm_filter_coef[filter0][0];
            INT32 f1 = s_cdic_adpcm_filter_coef[filter0][1];
            INT32 f2 = s_cdic_adpcm_filter_coef[filter1][0];
            INT32 f3 = s_cdic_adpcm_filter_coef[filter1][1];

			for(INT32 i = 0; i < 28; i++)
			{
				INT16 d=xa[(16 + (i << 2) + s) ^ 1];
                INT16 d0 = (d & 0xf) << 12;
                INT16 d1 = (d >> 4) << 12;
				d0 = clamp((d0 >> shift0) + (((l0 * f0) + (l1 * f1) + 32) >> 6));
				*dp = d0;
                dp++;
				l1 = l0;
				l0 = d0;

				d1 = clamp((d1 >> shift1) + (((l2 * f2) + (l3 * f3) + 32) >> 6));
				*dp = d1;
                dp++;
				l3 = l2;
				l2 = d1;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
	cdic_xa_last[2] = l2;
	cdic_xa_last[3] = l3;
}

void cdicdic_device::decode_xa_stereo8(INT32 *cdic_xa_last, const UINT8 *xa, INT16 *dp)
{
    INT32 l0 = cdic_xa_last[0];
    INT32 l1 = cdic_xa_last[1];
    INT32 l2 = cdic_xa_last[2];
    INT32 l3 = cdic_xa_last[3];

    for(INT32 b = 0; b < 18; b++)
	{
        for(INT32 s = 0; s < 4; s += 2)
		{
			UINT8 flags0 = xa[(4 + s) ^ 1];
            UINT8 shift0 = flags0 & 0xf;
            UINT8 filter0 = flags0 >> 4;
            UINT8 flags1 = xa[(5 + s) ^ 1];
            UINT8 shift1 = flags1 & 0xf;
            UINT8 filter1 = flags1 >> 4;
            INT32 f0 = s_cdic_adpcm_filter_coef[filter0][0];
            INT32 f1 = s_cdic_adpcm_filter_coef[filter0][1];
            INT32 f2 = s_cdic_adpcm_filter_coef[filter1][0];
            INT32 f3 = s_cdic_adpcm_filter_coef[filter1][1];

			for(INT32 i = 0; i < 28; i++)
			{
                INT16 d0 = (xa[(16 + (i << 2) + s + 0) ^ 1] << 8);
                INT16 d1 = (xa[(16 + (i << 2) + s + 1) ^ 1] << 8);

				d0 = clamp((d0 >> shift0) + (((l0 * f0) + (l1 * f1) + 32) >> 6));
				*dp = d0;
                dp++;
				l1 = l0;
				l0 = d0;

				d1 = clamp((d1 >> shift1) + (((l2 * f2) + (l3 * f3) + 32) >> 6));
				*dp = d1;
                dp++;
				l3 = l2;
				l2 = d1;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
	cdic_xa_last[2] = l2;
	cdic_xa_last[3] = l3;
}

void cdicdic_device::decode_audio_sector(const UINT8 *xa, INT32 triggered)
{
	// Get XA format from sector header

    cdi_state *state = m_machine.driver_data<cdi_state>();
	const UINT8 *hdr = xa + 4;
    INT32 channels;
    INT32 bits = 4;
    INT32 index = 0;
	INT16 samples[18*28*16+16];

	if(hdr[2] == 0xff && triggered == 1)
	{
		return;
	}

    verboselog(&m_machine, 0, "decode_audio_sector, got header type %02x\n", hdr[2] );

	switch(hdr[2] & 0x3f)	// ignore emphasis and reserved bits
	{
		case 0:
			channels = 1;
            m_audio_sample_freq = 37800.0f; //18900.0f;
			bits = 4;
            m_audio_sample_size = 4;
			break;

		case 1:
			channels=2;
            m_audio_sample_freq=37800.0f;
			bits=4;
            m_audio_sample_size=2;
			break;

		case 4:
			channels=1;
            m_audio_sample_freq=18900.0f;   ///2.0f;
			bits=4;
            m_audio_sample_size=4;
			break;

		case 5:
			channels=2;
            m_audio_sample_freq=18900.0f;   //37800.0f/2.0f;
			bits=4;
            m_audio_sample_size=2;
			break;

		case 16:
			channels=1;
            m_audio_sample_freq=37800.0f;
			bits=8;
            m_audio_sample_size=2;
			break;

		case 17:
			channels=2;
            m_audio_sample_freq=37800.0f;
			bits=8;
            m_audio_sample_size=1;
			break;

		default:
			fatalerror("play_xa: unhandled xa mode %08x",hdr[2]);
			return;
	}

    dmadac_set_frequency(&state->dmadac[0], 2, m_audio_sample_freq);
	dmadac_enable(&state->dmadac[0], 2, 1);

	switch(channels)
	{
		case 1:
			switch(bits)
			{
				case 4:
                    decode_xa_mono(m_xa_last, hdr + 4, samples);
					for(index = 18*28*8 - 1; index >= 0; index--)
					{
						samples[index*2 + 1] = samples[index];
						samples[index*2 + 0] = samples[index];
					}
					samples[18*28*16 + 0] = samples[18*28*16 + 2] = samples[18*28*16 + 4] = samples[18*28*16 + 6] = samples[18*28*16 + 8] = samples[18*28*16 + 10] = samples[18*28*16 + 12] = samples[18*28*16 + 14] = samples[18*28*16 - 2];
					samples[18*28*16 + 1] = samples[18*28*16 + 3] = samples[18*28*16 + 5] = samples[18*28*16 + 7] = samples[18*28*16 + 9] = samples[18*28*16 + 11] = samples[18*28*16 + 13] = samples[18*28*16 + 15] = samples[18*28*16 - 1];
					break;
				case 8:
                    decode_xa_mono8(m_xa_last, hdr + 4, samples);
					for(index = 18*28*8 - 1; index >= 0; index--)
					{
						samples[index*2 + 1] = samples[index];
						samples[index*2 + 0] = samples[index];
					}
					samples[18*28*8 + 0] = samples[18*28*8 + 2] = samples[18*28*8 + 4] = samples[18*28*8 + 6] = samples[18*28*8 + 8] = samples[18*28*8 + 10] = samples[18*28*8 + 12] = samples[18*28*8 + 14] = samples[18*28*8 - 2];
					samples[18*28*8 + 1] = samples[18*28*8 + 3] = samples[18*28*8 + 5] = samples[18*28*8 + 7] = samples[18*28*8 + 9] = samples[18*28*8 + 11] = samples[18*28*8 + 13] = samples[18*28*8 + 15] = samples[18*28*8 - 1];
					break;
			}
			break;
		case 2:
			switch(bits)
			{
				case 4:
                    decode_xa_stereo(m_xa_last, hdr + 4, samples);
					samples[18*28*8 + 0] = samples[18*28*8 + 2] = samples[18*28*8 + 4] = samples[18*28*8 + 6] = samples[18*28*8 + 8] = samples[18*28*8 + 10] = samples[18*28*8 + 12] = samples[18*28*8 + 14] = samples[18*28*8 - 2];
					samples[18*28*8 + 1] = samples[18*28*8 + 3] = samples[18*28*8 + 5] = samples[18*28*8 + 7] = samples[18*28*8 + 9] = samples[18*28*8 + 11] = samples[18*28*8 + 13] = samples[18*28*8 + 15] = samples[18*28*8 - 1];
                    //fwrite(samples, 1, 18*28*4*m_audio_sample_size, temp_adpcm);
					break;
				case 8:
                    decode_xa_stereo8(m_xa_last, hdr + 4, samples);
					samples[18*28*4 + 0] = samples[18*28*4 + 2] = samples[18*28*4 + 4] = samples[18*28*4 + 6] = samples[18*28*4 + 8] = samples[18*28*4 + 10] = samples[18*28*4 + 12] = samples[18*28*4 + 14] = samples[18*28*4 - 2];
					samples[18*28*4 + 1] = samples[18*28*4 + 3] = samples[18*28*4 + 5] = samples[18*28*4 + 7] = samples[18*28*4 + 9] = samples[18*28*4 + 11] = samples[18*28*4 + 13] = samples[18*28*4 + 15] = samples[18*28*4 - 1];
					break;
			}
			break;
	}

    dmadac_transfer(&state->dmadac[0], 2, 1, 2, 18*28*2*m_audio_sample_size, samples);
}

// After an appropriate delay for decoding to take place...
TIMER_CALLBACK( cdicdic_device::audio_sample_trigger )
{
    cdicdic_device *cdic = static_cast<cdicdic_device *>(machine->device("cdic"));
    cdic->sample_trigger();
}

void cdicdic_device::sample_trigger()
{
    if(m_decode_addr == 0xffff)
    {
        verboselog(&m_machine, 0, "Decode stop requested, stopping playback\n" );
        timer_adjust_oneshot(m_audio_sample_timer, attotime_never, 0);
        return;
    }

    if(!m_decode_delay)
    {
        // Indicate that data has been decoded
        verboselog(&m_machine, 0, "Flagging that audio data has been decoded\n" );
        m_audio_buffer |= 0x8000;

        // Set the CDIC interrupt line
        verboselog(&m_machine, 0, "Setting CDIC interrupt line for soundmap decode\n" );
        cpu_set_input_line_vector(m_machine.device("maincpu"), M68K_IRQ_4, 128);
        cputag_set_input_line(&m_machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
    }
    else
    {
        m_decode_delay = 0;
    }

    if(CDIC_IS_VALID_SAMPLE_BUF(m_ram, m_decode_addr & 0x3ffe))
    {
        verboselog(&m_machine, 0, "Hit audio_sample_trigger, with m_decode_addr == %04x, calling decode_audio_sector\n", m_decode_addr );

        // Decode the data at Z+4, the same offset as a normal CD sector.
        decode_audio_sector(((UINT8*)m_ram) + (m_decode_addr & 0x3ffe) + 4, 1);

        // Swap buffer positions to indicate our new buffer position at the next read
        m_decode_addr ^= 0x1a00;

        verboselog(&m_machine, 0, "Updated m_decode_addr, new value is %04x\n", m_decode_addr );

        //// Delay for Frequency * (18*28*2*size in bytes) before requesting more data
        verboselog(&m_machine, 0, "Data is valid, setting up a new callback\n" );
        m_decode_period = attotime_mul(ATTOTIME_IN_HZ(CDIC_SAMPLE_BUF_FREQ(m_ram, m_decode_addr & 0x3ffe)), 18*28*2*CDIC_SAMPLE_BUF_SIZE(m_ram, m_decode_addr & 0x3ffe));
        timer_adjust_oneshot(m_audio_sample_timer, m_decode_period, 0);
        //dmadac_enable(&dmadac[0], 2, 0);
    }
    else
    {
        // Swap buffer positions to indicate our new buffer position at the next read
        m_decode_addr ^= 0x1a00;

        verboselog(&m_machine, 0, "Data is not valid, indicating to shut down on the next audio sample\n" );
        m_decode_addr = 0xffff;
        timer_adjust_oneshot(m_audio_sample_timer, m_decode_period, 0);
    }
}

TIMER_CALLBACK( cdicdic_device::trigger_readback_int )
{
    cdicdic_device *cdic = static_cast<cdicdic_device *>(machine->device("cdic"));
    cdic->process_delayed_command();
}

void cdicdic_device::process_delayed_command()
{
    switch(m_command)
	{
		case 0x23: // Reset Mode 1
		case 0x24: // Reset Mode 2
		case 0x29: // Read Mode 1
		case 0x2a: // Read Mode 2
		//case 0x2c: // Seek
		{
			UINT8 buffer[2560] = { 0 };
            UINT32 msf = m_time >> 8;
			UINT32 lba = 0;
			int index = 0;
			UINT8 nybbles[6] =
			{
				 msf & 0x0000000f,
				(msf & 0x000000f0) >> 4,
				(msf & 0x00000f00) >> 8,
				(msf & 0x0000f000) >> 12,
				(msf & 0x000f0000) >> 16,
				(msf & 0x00f00000) >> 20
			};
			if(msf & 0x000080)
			{
				msf &= 0xffff00;
				nybbles[0] = 0;
				nybbles[1] = 0;
			}
			if(nybbles[2] >= 2)
			{
				nybbles[2] -= 2;
			}
			else
			{
				nybbles[2] = 8 + nybbles[2];
				if(nybbles[3] > 0)
				{
					nybbles[3]--;
				}
				else
				{
					nybbles[3] = 5;
					if(nybbles[4] > 0)
					{
						nybbles[4]--;
					}
					else
					{
						nybbles[4] = 9;
						nybbles[5]--;
					}
				}
			}
			lba = nybbles[0] + nybbles[1]*10 + ((nybbles[2] + nybbles[3]*10)*75) + ((nybbles[4] + nybbles[5]*10)*75*60);

            //printf( "Reading Mode %d sector from MSF location %06x\n", m_command - 0x28, m_time | 2 );
            verboselog(&m_machine, 0, "Reading Mode %d sector from MSF location %06x\n", m_command - 0x28, m_time | 2 );

            cdrom_read_data(m_cd, lba, buffer, CD_TRACK_RAW_DONTCARE);

            m_time += 0x100;
            if((m_time & 0x00000f00) == 0x00000a00)
			{
                m_time &= 0xfffff0ff;
                m_time += 0x00001000;
			}
            if((m_time & 0x0000ff00) == 0x00007500)
			{
                m_time &= 0xffff00ff;
                m_time += 0x00010000;
                if((m_time & 0x000f0000) == 0x000a0000)
				{
                    m_time &= 0xfff0ffff;
                    m_time += 0x00100000;
				}
			}
            if((m_time & 0x00ff0000) == 0x00600000)
			{
                m_time &= 0xff00ffff;
                m_time += 0x01000000;
                if((m_time & 0x0f000000) == 0x0a000000)
				{
                    m_time &= 0xf0ffffff;
                    m_time += 0x10000000;
				}
			}

            m_data_buffer &= ~0x0004;
            m_data_buffer ^= 0x0001;

            if((buffer[CDIC_SECTOR_FILE2] << 8) == m_file)
			{
                if(((buffer[CDIC_SECTOR_SUBMODE2] & (CDIC_SUBMODE_FORM | CDIC_SUBMODE_DATA | CDIC_SUBMODE_AUDIO | CDIC_SUBMODE_VIDEO)) == (CDIC_SUBMODE_FORM | CDIC_SUBMODE_AUDIO)) &&
                   (m_channel & m_audio_channel & (1 << buffer[CDIC_SECTOR_CHAN2])))
                {
                    verboselog(&m_machine, 0, "Audio sector\n" );

                    m_x_buffer |= 0x8000;
                    //m_data_buffer |= 0x4000;
                    m_data_buffer |= 0x0004;

                    for(index = 6; index < 2352/2; index++)
                    {
                        m_ram[(m_data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
                    }

                    decode_audio_sector(((UINT8*)m_ram) + ((m_data_buffer & 5) * 0xa00 + 4), 0);

                    //printf( "Setting CDIC interrupt line\n" );
                    verboselog(&m_machine, 0, "Setting CDIC interrupt line for audio sector\n" );
                    cpu_set_input_line_vector(m_machine.device("maincpu"), M68K_IRQ_4, 128);
                    cputag_set_input_line(&m_machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
                }
                else if((buffer[CDIC_SECTOR_SUBMODE2] & (CDIC_SUBMODE_DATA | CDIC_SUBMODE_AUDIO | CDIC_SUBMODE_VIDEO)) == 0x00)
                {
                    m_x_buffer |= 0x8000;
                    //m_data_buffer |= 0x4000;

                    for(index = 6; index < 2352/2; index++)
                    {
                        m_ram[(m_data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
                    }

                    if((buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_TRIG) == CDIC_SUBMODE_TRIG ||
                       (buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_EOR) == CDIC_SUBMODE_EOR ||
                       (buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_EOF) == CDIC_SUBMODE_EOF)
                    {
                        //printf( "Setting CDIC interrupt line\n" );
                        verboselog(&m_machine, 0, "Setting CDIC interrupt line for message sector\n" );
                        cpu_set_input_line_vector(m_machine.device("maincpu"), M68K_IRQ_4, 128);
                        cputag_set_input_line(&m_machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
                    }
                    else
                    {
                        verboselog(&m_machine, 0, "Message sector, ignored\n" );
                    }
                }
                else
                {
                    m_x_buffer |= 0x8000;
                    //m_data_buffer |= 0x4000;

                    for(index = 6; index < 2352/2; index++)
                    {
                        m_ram[(m_data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
                    }

                    //printf( "Setting CDIC interrupt line\n" );
                    verboselog(&m_machine, 0, "Setting CDIC interrupt line for data sector\n" );
                    cpu_set_input_line_vector(m_machine.device("maincpu"), M68K_IRQ_4, 128);
                    cputag_set_input_line(&m_machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
                }

                if((buffer[CDIC_SECTOR_SUBMODE2] & CDIC_SUBMODE_EOF) == 0 && m_command != 0x23)
				{
                    timer_adjust_oneshot(m_interrupt_timer, ATTOTIME_IN_HZ(75), 0); // 75Hz = 1x CD-ROM speed
				}
				else
				{
                    if(m_command == 0x23) // Mode 1 Reset
					{
                        timer_adjust_oneshot(m_interrupt_timer, attotime_never, 0);
					}
				}
			}

			break;
		}

		case 0x2e: // Abort
            timer_adjust_oneshot(m_interrupt_timer, attotime_never, 0);
            //m_data_buffer &= ~4;
			break;

		case 0x28: // Play CDDA audio
		{
			UINT8 buffer[2560] = { 0 };
			int index = 0;
            UINT32 msf = (m_time & 0xffff7f00) >> 8;
            UINT32 next_msf = increment_cdda_frame_bcd((m_time & 0xffff7f00) >> 8);
            UINT32 rounded_next_msf = increment_cdda_sector_bcd((m_time & 0xffff0000) >> 8);
			UINT32 lba = 0;
			UINT32 next_lba = 0;
			UINT8 nybbles[6] =
			{
				 msf & 0x0000000f,
				(msf & 0x000000f0) >> 4,
				(msf & 0x00000f00) >> 8,
				(msf & 0x0000f000) >> 12,
				(msf & 0x000f0000) >> 16,
				(msf & 0x00f00000) >> 20
			};
			UINT8 next_nybbles[6] =
			{
				 rounded_next_msf & 0x0000000f,
				(rounded_next_msf & 0x000000f0) >> 4,
				(rounded_next_msf & 0x00000f00) >> 8,
				(rounded_next_msf & 0x0000f000) >> 12,
				(rounded_next_msf & 0x000f0000) >> 16,
				(rounded_next_msf & 0x00f00000) >> 20
			};

			lba = nybbles[0] + nybbles[1]*10 + ((nybbles[2] + nybbles[3]*10)*75) + ((nybbles[4] + nybbles[5]*10)*75*60);

            if(!cdrom_read_data(m_cd, lba, buffer, CD_TRACK_RAW_DONTCARE))
            {
                printf( "liajfoaijsdofiaodf\n");
            }

			if(!(msf & 0x0000ff))
			{
				next_lba = next_nybbles[0] + next_nybbles[1]*10 + ((next_nybbles[2] + next_nybbles[3]*10)*75) + ((next_nybbles[4] + next_nybbles[5]*10)*75*60);
                verboselog(&m_machine, 0, "Playing CDDA sector from MSF location %06x\n", m_time | 2 );

                cdda_start_audio(m_machine.device("cdda"), lba, rounded_next_msf);
			}

            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x924/2] = 0x0001;                      //  CTRL
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x926/2] = 0x0001;                      //  TRACK
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x928/2] = 0x0000;                      //  INDEX
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92a/2] = (m_time >> 24) & 0x000000ff; //  MIN
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92c/2] = (m_time >> 16) & 0x000000ff; //  SEC
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92e/2] = (m_time >>  8) & 0x0000007f; //  FRAC
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x930/2] = 0x0000;                      //  ZERO
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x932/2] = (m_time >> 24) & 0x000000ff; //  AMIN
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x934/2] = (m_time >> 16) & 0x000000ff; //  ASEC
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x936/2] = (m_time >>  8) & 0x0000007f; //  AFRAC
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x938/2] = 0x0000;                      //  CRC1
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x93a/2] = 0x0000;                      //  CRC2

            m_time = next_msf << 8;

            timer_adjust_oneshot(m_interrupt_timer, ATTOTIME_IN_HZ(75), 0);

            m_x_buffer |= 0x8000;
            //m_data_buffer |= 0x4000;

			for(index = 6; index < 2352/2; index++)
			{
                m_ram[(m_data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
			}

            verboselog(&m_machine, 0, "Setting CDIC interrupt line for CDDA sector\n" );
            cpu_set_input_line_vector(m_machine.device("maincpu"), M68K_IRQ_4, 128);
            cputag_set_input_line(&m_machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
			break;
		}
		case 0x2c: // Seek
		{
			UINT8 buffer[2560] = { 0 };
			int index = 0;
            UINT32 msf = (m_time & 0xffff7f00) >> 8;
            UINT32 next_msf = increment_cdda_frame_bcd((m_time & 0xffff7f00) >> 8);
			UINT32 lba = 0;
			UINT8 nybbles[6] =
			{
				 msf & 0x0000000f,
				(msf & 0x000000f0) >> 4,
				(msf & 0x00000f00) >> 8,
				(msf & 0x0000f000) >> 12,
				(msf & 0x000f0000) >> 16,
				(msf & 0x00f00000) >> 20
			};
			lba = nybbles[0] + nybbles[1]*10 + ((nybbles[2] + nybbles[3]*10)*75) + ((nybbles[4] + nybbles[5]*10)*75*60);

            timer_adjust_oneshot(m_interrupt_timer, ATTOTIME_IN_HZ(75), 0);

            cdrom_read_data(m_cd, lba, buffer, CD_TRACK_RAW_DONTCARE);

            m_data_buffer ^= 0x0001;
            m_x_buffer |= 0x8000;
            m_data_buffer |= 0x4000;

			for(index = 6; index < 2352/2; index++)
			{
                m_ram[(m_data_buffer & 5) * (0xa00/2) + (index - 6)] = (buffer[index*2] << 8) | buffer[index*2 + 1];
			}

            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x924/2] = 0x0041;                      //  CTRL
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x926/2] = 0x0001;                      //  TRACK
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x928/2] = 0x0000;                      //  INDEX
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92a/2] = (m_time >> 24) & 0x000000ff; //  MIN
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92c/2] = (m_time >> 16) & 0x000000ff; //  SEC
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x92e/2] = (m_time >>  8) & 0x0000007f; //  FRAC
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x930/2] = 0x0000;                      //  ZERO
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x932/2] = (m_time >> 24) & 0x000000ff; //  AMIN
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x934/2] = (m_time >> 16) & 0x000000ff; //  ASEC
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x936/2] = (m_time >>  8) & 0x0000007f; //  AFRAC
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x938/2] = 0x0000;                      //  CRC1
            m_ram[(m_data_buffer & 5) * (0xa00/2) + 0x93a/2] = 0x0000;                      //  CRC2

            m_time = next_msf << 8;

            verboselog(&m_machine, 0, "Setting CDIC interrupt line for Seek sector\n" );
            cpu_set_input_line_vector(m_machine.device("maincpu"), M68K_IRQ_4, 128);
            cputag_set_input_line(&m_machine, "maincpu", M68K_IRQ_4, ASSERT_LINE);
			break;
		}
	}
}

READ16_DEVICE_HANDLER( cdic_r )
{
    return downcast<cdicdic_device *>(device)->register_read(offset, mem_mask);
}

UINT16 cdicdic_device::register_read(const UINT32 offset, const UINT16 mem_mask)
{
    UINT32 addr = offset + 0x3c00/2;

    switch(addr)
    {
        case 0x3c00/2: // Command register
            verboselog(&m_machine, 0, "cdic_r: Command Register = %04x & %04x\n", m_command, mem_mask);
            return m_command;

        case 0x3c02/2: // Time register (MSW)
            verboselog(&m_machine, 0, "cdic_r: Time Register (MSW) = %04x & %04x\n", m_time >> 16, mem_mask);
            return m_time >> 16;

        case 0x3c04/2: // Time register (LSW)
            verboselog(&m_machine, 0, "cdic_r: Time Register (LSW) = %04x & %04x\n", (UINT16)(m_time & 0x0000ffff), mem_mask);
            return m_time & 0x0000ffff;

        case 0x3c06/2: // File register
            verboselog(&m_machine, 0, "cdic_r: File Register = %04x & %04x\n", m_file, mem_mask);
            return m_file;

        case 0x3c08/2: // Channel register (MSW)
            verboselog(&m_machine, 0, "cdic_r: Channel Register (MSW) = %04x & %04x\n", m_channel >> 16, mem_mask);
            return m_channel >> 16;

        case 0x3c0a/2: // Channel register (LSW)
            verboselog(&m_machine, 0, "cdic_r: Channel Register (LSW) = %04x & %04x\n", m_channel & 0x0000ffff, mem_mask);
            return m_channel & 0x0000ffff;

        case 0x3c0c/2: // Audio Channel register
            verboselog(&m_machine, 0, "cdic_r: Audio Channel Register = %04x & %04x\n", m_audio_channel, mem_mask);
            return m_audio_channel;

        case 0x3ff4/2: // ABUF
        {
            UINT16 temp = m_audio_buffer;
            m_audio_buffer &= 0x7fff;
            if(!((m_audio_buffer | m_x_buffer) & 0x8000))
            {
                cputag_set_input_line(&m_machine, "maincpu", M68K_IRQ_4, CLEAR_LINE);
                verboselog(&m_machine, 0, "Clearing CDIC interrupt line\n" );
                ////printf("Clearing CDIC interrupt line\n" );
            }
            verboselog(&m_machine, 0, "cdic_r: Audio Buffer Register = %04x & %04x\n", temp, mem_mask);
            return temp;
        }

        case 0x3ff6/2: // XBUF
        {
            UINT16 temp = m_x_buffer;
            m_x_buffer &= 0x7fff;
            if(!((m_audio_buffer | m_x_buffer) & 0x8000))
            {
                cputag_set_input_line(&m_machine, "maincpu", M68K_IRQ_4, CLEAR_LINE);
                verboselog(&m_machine, 0, "Clearing CDIC interrupt line\n" );
                ////printf("Clearing CDIC interrupt line\n" );
            }
            verboselog(&m_machine, 0, "cdic_r: X-Buffer Register = %04x & %04x\n", temp, mem_mask);
            return temp;
        }

        case 0x3ffa/2: // AUDCTL
        {
            if(attotime_is_never(timer_timeleft(m_audio_sample_timer)))
            {
                m_z_buffer ^= 0x0001;
            }
            verboselog(&m_machine, 0, "cdic_r: Z-Buffer Register = %04x & %04x\n", m_z_buffer, mem_mask);
            return m_z_buffer;
        }

        case 0x3ffe/2:
        {
            verboselog(&m_machine, 0, "cdic_r: Data buffer Register = %04x & %04x\n", m_data_buffer, mem_mask);
            return m_data_buffer;
        }
        default:
            verboselog(&m_machine, 0, "cdic_r: UNIMPLEMENTED: Unknown address: %04x & %04x\n", addr*2, mem_mask);
            return 0;
    }
}

WRITE16_DEVICE_HANDLER( cdic_w )
{
    downcast<cdicdic_device *>(device)->register_write(offset, data, mem_mask);
}

void cdicdic_device::register_write(const UINT32 offset, const UINT16 data, const UINT16 mem_mask)
{
    cdi_state *state = m_machine.driver_data<cdi_state>();

	UINT32 addr = offset + 0x3c00/2;

	switch(addr)
	{
		case 0x3c00/2: // Command register
            verboselog(&m_machine, 0, "cdic_w: Command Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&m_command);
			break;

		case 0x3c02/2: // Time register (MSW)
            m_time &= ~(mem_mask << 16);
            m_time |= (data & mem_mask) << 16;
            verboselog(&m_machine, 0, "cdic_w: Time Register (MSW) = %04x & %04x\n", data, mem_mask);
			break;

		case 0x3c04/2: // Time register (LSW)
            m_time &= ~mem_mask;
            m_time |= data & mem_mask;
            verboselog(&m_machine, 0, "cdic_w: Time Register (LSW) = %04x & %04x\n", data, mem_mask);
			break;

		case 0x3c06/2: // File register
            verboselog(&m_machine, 0, "cdic_w: File Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&m_file);
			break;

		case 0x3c08/2: // Channel register (MSW)
            m_channel &= ~(mem_mask << 16);
            m_channel |= (data & mem_mask) << 16;
            verboselog(&m_machine, 0, "cdic_w: Channel Register (MSW) = %04x & %04x\n", data, mem_mask);
			break;

		case 0x3c0a/2: // Channel register (LSW)
            m_channel &= ~mem_mask;
            m_channel |= data & mem_mask;
            verboselog(&m_machine, 0, "cdic_w: Channel Register (LSW) = %04x & %04x\n", data, mem_mask);
			break;

		case 0x3c0c/2: // Audio Channel register
            verboselog(&m_machine, 0, "cdic_w: Audio Channel Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&m_audio_channel);
			break;

		case 0x3ff4/2:
            verboselog(&m_machine, 0, "cdic_w: Audio Buffer Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&m_audio_buffer);
			break;

		case 0x3ff6/2:
            verboselog(&m_machine, 0, "cdic_w: X Buffer Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&m_x_buffer);
			break;

		case 0x3ff8/2:
		{
			scc68070_regs_t *scc68070 = &state->scc68070_regs;
			UINT32 start = scc68070->dma.channel[0].memory_address_counter;
			UINT32 count = scc68070->dma.channel[0].transfer_counter;
			UINT32 index = 0;
			UINT32 device_index = (data & 0x3fff) >> 1;
			UINT16 *memory = state->planea;
            verboselog(&m_machine, 0, "memory address counter: %08x\n", scc68070->dma.channel[0].memory_address_counter);
            verboselog(&m_machine, 0, "cdic_w: DMA Control Register = %04x & %04x\n", data, mem_mask);
            verboselog(&m_machine, 0, "Doing copy, transferring %04x bytes\n", count * 2 );
			////printf("Doing copy, transferring %04x bytes\n", count * 2 );
			if((start & 0x00f00000) == 0x00200000)
			{
				start -= 0x00200000;
				memory = state->planeb;
			}
			for(index = start / 2; index < (start / 2 + count); index++)
			{
				if(scc68070->dma.channel[0].operation_control & OCR_D)
				{
                    memory[index] = m_ram[device_index++];
				}
				else
				{
                    m_ram[device_index++] = memory[index];
				}
			}
			scc68070->dma.channel[0].memory_address_counter += scc68070->dma.channel[0].transfer_counter * 2;
			break;
		}

		case 0x3ffa/2:
		{
            verboselog(&m_machine, 0, "cdic_w: Z-Buffer Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&m_z_buffer);
            if(m_z_buffer & 0x2000)
			{
                attotime period = timer_timeleft(m_audio_sample_timer);
				if(attotime_is_never(period))
				{
                    m_decode_addr = m_z_buffer & 0x3a00;
                    m_decode_delay = 1;
                    timer_adjust_oneshot(m_audio_sample_timer, ATTOTIME_IN_HZ(75), 0);
				}
			}
			else
			{
                m_decode_addr = 0xffff;
                timer_adjust_oneshot(m_audio_sample_timer, attotime_never, 0);
			}
			break;
		}
		case 0x3ffc/2:
            verboselog(&m_machine, 0, "cdic_w: Interrupt Vector Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&m_interrupt_vector);
			break;
		case 0x3ffe/2:
		{
            verboselog(&m_machine, 0, "cdic_w: Data Buffer Register = %04x & %04x\n", data, mem_mask);
            COMBINE_DATA(&m_data_buffer);
            if(m_data_buffer & 0x8000)
			{
                switch(m_command)
				{
					//case 0x24: // Reset Mode 2
					case 0x2e: // Abort
					{
                        timer_adjust_oneshot(m_interrupt_timer, attotime_never, 0);
						dmadac_enable(&state->dmadac[0], 2, 0);
                        //m_data_buffer &= 0xbfff;
						break;
					}
					case 0x2b: // Stop CDDA
						cdda_stop_audio(m_machine.device("cdda"));
                        timer_adjust_oneshot(m_interrupt_timer, attotime_never, 0);
						break;
					case 0x23: // Reset Mode 1
					case 0x29: // Read Mode 1
					case 0x2a: // Read Mode 2
					case 0x28: // Play CDDA
					case 0x2c: // Seek
					{
                        attotime period = timer_timeleft(m_interrupt_timer);
						if(!attotime_is_never(period))
						{
                            timer_adjust_oneshot(m_interrupt_timer, period, 0);
						}
						else
						{
                            if(m_command != 0x23 && m_command != 0x24)
							{
                                timer_adjust_oneshot(m_interrupt_timer, ATTOTIME_IN_HZ(75), 0);
							}
						}
						break;
					}
					default:
                        verboselog(&m_machine, 0, "Unknown CDIC command: %02x\n", m_command );
						break;
				}
			}
            m_data_buffer &= 0x7fff;
			break;
		}
		default:
            verboselog(&m_machine, 0, "cdic_w: UNIMPLEMENTED: Unknown address: %04x = %04x & %04x\n", addr*2, data, mem_mask);
			break;
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdicdic_device - constructor
//-------------------------------------------------

cdicdic_device::cdicdic_device(running_machine &_machine, const cdicdic_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{
    init();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdicdic_device::device_start()
{
    register_globals();

    m_interrupt_timer = timer_alloc(&m_machine, trigger_readback_int, 0);
    timer_adjust_oneshot(m_interrupt_timer, attotime_never, 0);

    m_audio_sample_timer = timer_alloc(&m_machine, audio_sample_trigger, 0);
    timer_adjust_oneshot(m_audio_sample_timer, attotime_never, 0);

    m_ram = auto_alloc_array(&m_machine, UINT16, 0x3c00/2);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdicdic_device::device_reset()
{
    init();

	m_cd = cdrom_open(get_disk_handle(&m_machine, "cdrom"));
	cdda_set_cdrom(m_machine.device("cdda"), m_cd);
}

void cdicdic_device::init()
{
    m_command = 0;
    m_time = 0;
    m_file = 0;
    m_channel = 0xffffffff;
    m_audio_channel = 0xffff;
    m_audio_buffer = 0;
    m_x_buffer = 0;
    m_dma_control = 0;
    m_z_buffer = 0;
    m_interrupt_vector = 0;
    m_data_buffer = 0;

    m_audio_sample_freq = 0;
    m_audio_sample_size = 0;

    m_decode_addr = 0;
    m_decode_delay = 0;
}

void cdicdic_device::register_globals()
{
    state_save_register_device_item(this, 0, m_command);
    state_save_register_device_item(this, 0, m_time);
    state_save_register_device_item(this, 0, m_file);
    state_save_register_device_item(this, 0, m_channel);
    state_save_register_device_item(this, 0, m_audio_channel);
    state_save_register_device_item(this, 0, m_audio_buffer);
    state_save_register_device_item(this, 0, m_x_buffer);
    state_save_register_device_item(this, 0, m_dma_control);
    state_save_register_device_item(this, 0, m_z_buffer);
    state_save_register_device_item(this, 0, m_interrupt_vector);
    state_save_register_device_item(this, 0, m_data_buffer);

    state_save_register_device_item(this, 0, m_audio_sample_freq);
    state_save_register_device_item(this, 0, m_audio_sample_size);
}

WRITE16_DEVICE_HANDLER( cdic_ram_w )
{
    downcast<cdicdic_device *>(device)->ram_write(offset, data, mem_mask);
}

void cdicdic_device::ram_write(const UINT32 offset, const UINT16 data, const UINT16 mem_mask)
{
    verboselog(&m_machine, 5, "cdic_ram_w: %08x = %04x & %04x\n", 0x00300000 + offset*2, data, mem_mask);
    COMBINE_DATA(&m_ram[offset]);
}

READ16_DEVICE_HANDLER( cdic_ram_r )
{
    return downcast<cdicdic_device *>(device)->ram_read(offset, mem_mask);
}

UINT16 cdicdic_device::ram_read(const UINT32 offset, const UINT16 mem_mask)
{
    verboselog(&m_machine, 5, "cdic_ram_r: %08x = %04x & %04x\n", 0x00300000 + offset * 2, m_ram[offset], mem_mask);
    return m_ram[offset];
}

const device_type MACHINE_CDICDIC = cdicdic_device_config::static_alloc_device_config;
