/*********************************************************/
/*    SEGA 16ch 8bit PCM                                 */
/*********************************************************/

#include "emu.h"
#include "streams.h"
#include "segapcm.h"

typedef struct _segapcm_state segapcm_state;
struct _segapcm_state
{
	UINT8  *ram;
	UINT8 low[16];
	const UINT8 *rom;
	int bankshift;
	int bankmask;
	int rgnmask;
	sound_stream * stream;
};

INLINE segapcm_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SEGAPCM);
	return (segapcm_state *)downcast<legacy_device_base *>(device)->token();
}

static STREAM_UPDATE( SEGAPCM_update )
{
	segapcm_state *spcm = (segapcm_state *)param;
	int rgnmask = spcm->rgnmask;
	int ch;

	/* clear the buffers */
	memset(outputs[0], 0, samples*sizeof(*outputs[0]));
	memset(outputs[1], 0, samples*sizeof(*outputs[1]));

	// reg      function
	// ------------------------------------------------
	// 0x00     ?
	// 0x01     ?
	// 0x02     volume left
	// 0x03     volume right
	// 0x04     loop address (08-15)
	// 0x05     loop address (16-23)
	// 0x06     end address
	// 0x07     address delta
	// 0x80     ?
	// 0x81     ?
	// 0x82     ?
	// 0x83     ?
	// 0x84     current address (08-15), 00-07 is internal?
	// 0x85     current address (16-23)
	// 0x86     bit 0: channel disable?
	//          bit 1: loop disable
	//          other bits: bank
	// 0x87     ?

	/* loop over channels */
	for (ch = 0; ch < 16; ch++)
	{
		UINT8 *regs = spcm->ram+8*ch;

		/* only process active channels */
		if (!(regs[0x86]&1))
		{
			const UINT8 *rom = spcm->rom + ((regs[0x86] & spcm->bankmask) << spcm->bankshift);
			UINT32 addr = (regs[0x85] << 16) | (regs[0x84] << 8) | spcm->low[ch];
			UINT32 loop = (regs[0x05] << 16) | (regs[0x04] << 8);
			UINT8 end = regs[6] + 1;
			int i;

			/* loop over samples on this channel */
			for (i = 0; i < samples; i++)
			{
				INT8 v = 0;

				/* handle looping if we've hit the end */
				if ((addr >> 16) == end)
				{
					if (regs[0x86] & 2)
					{
						regs[0x86] |= 1;
						break;
					}
					else addr = loop;
				}

				/* fetch the sample */
				v = rom[(addr >> 8) & rgnmask] - 0x80;

				/* apply panning and advance */
				outputs[0][i] += v * regs[2];
				outputs[1][i] += v * regs[3];
				addr = (addr + regs[7]) & 0xffffff;
			}

			/* store back the updated address */
			regs[0x84] = addr >> 8;
			regs[0x85] = addr >> 16;
			spcm->low[ch] = regs[0x86] & 1 ? 0 : addr;
		}
	}
}

static DEVICE_START( segapcm )
{
	const sega_pcm_interface *intf = (const sega_pcm_interface *)device->baseconfig().static_config();
	int mask, rom_mask, len;
	segapcm_state *spcm = get_safe_token(device);

	spcm->rom = *device->region();
	spcm->ram = auto_alloc_array(device->machine, UINT8, 0x800);

	memset(spcm->ram, 0xff, 0x800);

	spcm->bankshift = (UINT8)(intf->bank);
	mask = intf->bank >> 16;
	if(!mask)
		mask = BANK_MASK7>>16;

	len = device->region()->bytes();
	spcm->rgnmask = len - 1;

	for(rom_mask = 1; rom_mask < len; rom_mask *= 2);

	rom_mask--;

	spcm->bankmask = mask & (rom_mask >> spcm->bankshift);

	spcm->stream = stream_create(device, 0, 2, device->clock() / 128, spcm, SEGAPCM_update);

	state_save_register_device_item_array(device, 0, spcm->low);
	state_save_register_device_item_pointer(device, 0, spcm->ram, 0x800);
}


WRITE8_DEVICE_HANDLER( sega_pcm_w )
{
	segapcm_state *spcm = get_safe_token(device);
	stream_update(spcm->stream);
	spcm->ram[offset & 0x07ff] = data;
}

READ8_DEVICE_HANDLER( sega_pcm_r )
{
	segapcm_state *spcm = get_safe_token(device);
	stream_update(spcm->stream);
	return spcm->ram[offset & 0x07ff];
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( segapcm )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(segapcm_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( segapcm );		break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Sega PCM");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Sega custom");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(SEGAPCM, segapcm);
