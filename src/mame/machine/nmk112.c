/*  NMK112 - NMK custom IC for bankswitching the sample ROMs of a pair of
    OKI6295 ADPCM chips

    The address space of each OKI6295 is divided into four banks, each one
    independently controlled. The sample table at the beginning of the
    address space may be divided in four pages as well, banked together
    with the sample data.  This allows each of the four voices on the chip
    to play a sample from a different bank at the same time. */

#include "emu.h"
#include "nmk112.h"

#define TABLESIZE   0x100
#define BANKSIZE    0x10000

typedef struct _nmk112_state nmk112_state;
struct _nmk112_state
{
	/* which chips have their sample address table divided into pages */
	UINT8 page_mask;

	UINT8 current_bank[8];

	UINT8 *rom0, *rom1;
	int   size0, size1;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE nmk112_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == NMK112);

	return (nmk112_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const nmk112_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == NMK112));
	return (const nmk112_interface *) device->baseconfig().static_config();
}

/*****************************************************************************
    STATIC FUNCTIONS
*****************************************************************************/

static void do_bankswitch( nmk112_state *nmk112, int offset, int data )
{
	int chip = (offset & 4) >> 2;
	int banknum = offset & 3;
	int paged = (nmk112->page_mask & (1 << chip));

	UINT8 *rom = chip ? nmk112->rom1 : nmk112->rom0;
	int size = chip ? nmk112->size1 : nmk112->size0;

	nmk112->current_bank[offset] = data;

	if (size == 0) return;

	int bankaddr = (data * BANKSIZE) % size;

	/* copy the samples */
	if ((paged) && (banknum == 0))
		memcpy(rom + 0x400, rom + 0x40000 + bankaddr + 0x400, BANKSIZE - 0x400);
	else
		memcpy(rom + banknum * BANKSIZE, rom + 0x40000 + bankaddr, BANKSIZE);

	/* also copy the sample address table, if it is paged on this chip */
	if (paged)
	{
		rom += banknum * TABLESIZE;
		memcpy(rom, rom + 0x40000 + bankaddr, TABLESIZE);
	}
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE8_DEVICE_HANDLER( nmk112_okibank_w )
{
	nmk112_state *nmk112 = get_safe_token(device);

	if (nmk112->current_bank[offset] != data)
		do_bankswitch(nmk112, offset, data);
}

WRITE16_DEVICE_HANDLER( nmk112_okibank_lsb_w )
{
	if (ACCESSING_BITS_0_7)
	{
		nmk112_okibank_w(device, offset, data & 0xff);
	}
}

static STATE_POSTLOAD( nmk112_postload_bankswitch )
{
	nmk112_state *nmk112 = (nmk112_state *)param;

	for (int i = 0; i < 8; i++)
		do_bankswitch(nmk112, i, nmk112->current_bank[i]);
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( nmk112 )
{
	nmk112_state *nmk112 = get_safe_token(device);
	const nmk112_interface *intf = get_interface(device);

	if (intf->rgn0 == NULL)
	{
		nmk112->rom0 = NULL;
		nmk112->size0 = 0;
	}
	else
	{
		nmk112->rom0 = device->machine->region(intf->rgn0)->base();
		nmk112->size0 = device->machine->region(intf->rgn0)->bytes() - 0x40000;
	}

	if (intf->rgn1 == NULL)
	{
		nmk112->rom1 = NULL;
		nmk112->size1 = 0;
	}
	else
	{
		nmk112->rom1 = device->machine->region(intf->rgn1)->base();
		nmk112->size1 = device->machine->region(intf->rgn1)->bytes() - 0x40000;
	}

	nmk112->page_mask = ~intf->disable_page_mask;

	device->save_item(NAME(nmk112->current_bank));
	device->machine->state().register_postload(nmk112_postload_bankswitch, nmk112);
}

static DEVICE_RESET( nmk112 )
{
	nmk112_state *nmk112 = get_safe_token(device);

	for (int i = 0; i < 8; i++)
	{
		nmk112->current_bank[i] = 0;
		do_bankswitch(nmk112, i, nmk112->current_bank[i]);
	}
}


/*****************************************************************************
    DEVICE DEFINITION
*****************************************************************************/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##nmk112##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME				"NMK 112"
#define DEVTEMPLATE_FAMILY				"NMK 112 Bankswitch IC"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(NMK112, nmk112);
