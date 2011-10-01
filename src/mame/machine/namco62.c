/***************************************************************************

    Namco 62XX

    This custom chip is a Fujitsu MB8843 MCU programmed to act as an I/O
    device. It is used by just one game: Gaplus.

    TODO: Chip pin description/layout/notes

***************************************************************************/

#include "emu.h"
#include "namco62.h"
#include "cpu/mb88xx/mb88xx.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

typedef struct _namco_62xx_state namco_62xx_state;
struct _namco_62xx_state
{
	device_t* m_cpu;
	devcb_resolved_read8 m_in[4];
	devcb_resolved_write8 m_out[2];
};

INLINE namco_62xx_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == NAMCO_62XX);

	return (namco_62xx_state *)downcast<legacy_device_base *>(device)->token();
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static ADDRESS_MAP_START( namco_62xx_map_io, AS_IO, 8 )
//  AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_62xx_K_r)
//  AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_62xx_O_w)
//  AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ(namco_62xx_R0_r)
//  AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_READ(namco_62xx_R2_r)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( namco_62xx )
	MCFG_CPU_ADD("mcu", MB8843, DERIVED_CLOCK(1,1))		/* parent clock, internally divided by 6 (TODO: Correct?) */
	MCFG_CPU_IO_MAP(namco_62xx_map_io)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END

ROM_START( namco_62xx )
	ROM_REGION( 0x800, "mcu", ROMREGION_LOADBYNAME )
	ROM_LOAD( "62xx.bin", 0x0000, 0x0800, CRC(308dc115) SHA1(fe0a60fc339ac2eeed4879a64c1aab130f9d4cfe) )
ROM_END


/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( namco_62xx )
{
	const namco_62xx_interface *config = (const namco_62xx_interface *)device->static_config();
	namco_62xx_state *state = get_safe_token(device);
	astring tempstring;

	assert(config != NULL);

	/* find our CPU */
	state->m_cpu = device->subdevice("mcu");
	assert(state->m_cpu != NULL);

	/* resolve our read callbacks */
	state->m_in[0].resolve(config->in[0], *device);
	state->m_in[1].resolve(config->in[1], *device);
	state->m_in[2].resolve(config->in[2], *device);
	state->m_in[3].resolve(config->in[3], *device);

	/* resolve our write callbacks */
	state->m_out[0].resolve(config->out[0], *device);
	state->m_out[1].resolve(config->out[1], *device);
}


/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##namco_62xx##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_ROM_REGION | DT_HAS_MACHINE_CONFIG
#define DEVTEMPLATE_NAME		"Namco 62xx"
#define DEVTEMPLATE_SHORTNAME   "namco62"
#define DEVTEMPLATE_FAMILY		"Namco I/O"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(NAMCO_62XX, namco_62xx);
