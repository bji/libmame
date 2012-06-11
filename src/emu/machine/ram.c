/*************************************************************************

    RAM device

    Provides a configurable amount of RAM to drivers

**************************************************************************/

#include <stdio.h>
#include <ctype.h>

#include "emu.h"
#include "emuopts.h"
#include "ram.h"


/*****************************************************************************
    LIVE DEVICE
*****************************************************************************/

// device type definition
const device_type RAM = &device_creator<ram_device>;



//-------------------------------------------------
//  ram_device - constructor
//-------------------------------------------------

ram_device::ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, RAM, "RAM", tag, owner, clock)
{
	m_size = 0;
	m_pointer = NULL;
	m_default_size = NULL;
	m_extra_options = NULL;
	m_default_value = 0xCD;
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ram_device::device_start()
{
	/* the device named 'ram' can get ram options from command line */
	m_size = 0;
	if (strcmp(tag(), ":" RAM_TAG) == 0)
	{
		const char *ramsize_string = machine().options().ram_size();
		if ((ramsize_string != NULL) && (ramsize_string[0] != '\0'))
			m_size = parse_string(ramsize_string);
	}

	/* if we didn't get a size yet, use the default */
	if (m_size == 0)
		m_size = default_size();

	/* allocate space for the ram */
	m_pointer = auto_alloc_array(machine(), UINT8, m_size);

	/* reset ram to the default value */
	memset(m_pointer, m_default_value, m_size);

	/* register for state saving */
	save_item(NAME(m_size));
	save_pointer(NAME(m_pointer), m_size);
}


//-------------------------------------------------
//  device_validity_check - device-specific validity
//  checks
//-------------------------------------------------

void ram_device::device_validity_check(validity_checker &valid) const
{
	const char *ramsize_string = NULL;
	int is_valid = FALSE;
	UINT32 specified_ram = 0;
	const char *gamename_option = NULL;

	/* verify default ram value */
	if (default_size() == 0)
		mame_printf_error("Invalid default RAM option: %s\n", m_default_size);

	/* command line options are only parsed for the device named RAM_TAG */
	if (tag() != NULL && strcmp(tag(), RAM_TAG) == 0)
	{
		/* verify command line ram option */
		ramsize_string = mconfig().options().ram_size();
		gamename_option = mconfig().options().system_name();

		if ((ramsize_string != NULL) && (ramsize_string[0] != '\0'))
		{
			specified_ram = parse_string(ramsize_string);

			if (specified_ram == 0)
				mame_printf_error("Cannot recognize the RAM option %s\n", ramsize_string);

			if (gamename_option != NULL && *gamename_option != 0 && strcmp(gamename_option, mconfig().gamedrv().name) == 0)
			{
				/* compare command line option to default value */
				if (default_size() == specified_ram)
					is_valid = TRUE;

				/* verify extra ram options */
				if (m_extra_options != NULL)
				{
					int j;
					int size = strlen(m_extra_options);
					char * const s = mame_strdup(m_extra_options);
					char * const e = s + size;
					char *p = s;
					for (j=0;j<size;j++) {
						if (p[j]==',') p[j]=0;
					}

					/* try to parse each option */
					while(p <= e)
					{
						UINT32 option_ram_size = parse_string(p);

						if (option_ram_size == 0)
							mame_printf_error("Invalid RAM option: %s\n", p);

						if (option_ram_size == specified_ram)
							is_valid = TRUE;

						p += strlen(p);
						if (p == e)
							break;
						p += 1;
					}

					osd_free(s);
				}

			} else {
				/* if not for this driver then return ok */
				is_valid = TRUE;
			}
		}
		else
		{
			/* not specifying the ramsize on the command line is valid as well */
			is_valid = TRUE;
		}
	}
	else
		is_valid = TRUE;

	if (!is_valid)
	{
		astring output;
		output.catprintf("Cannot recognize the RAM option %s", ramsize_string);
		output.catprintf(" (valid options are %s", m_default_size);

		if (m_extra_options != NULL)
			output.catprintf(",%s).\n", m_extra_options);
		else
			output.catprintf(").\n");

		mame_printf_error("%s", output.cstr());
	}
}



//-------------------------------------------------
//  parse_string - convert a ram string to an
//  integer value
//-------------------------------------------------

UINT32 ram_device::parse_string(const char *s)
{
	UINT32 ram;
	char suffix = '\0';

	s += sscanf(s, "%u%c", &ram, &suffix);

	switch(tolower(suffix))
	{
		case 'k':
			/* kilobytes */
			ram *= 1024;
			break;

		case 'm':
			/* megabytes */
			ram *= 1024*1024;
			break;

		case '\0':
			/* no suffix */
			break;

		default:
			/* parse failure */
			ram = 0;
			break;
	}

	return ram;
}



//-------------------------------------------------
//  default_size
//-------------------------------------------------

UINT32 ram_device::default_size(void) const
{
	return parse_string(m_default_size);
}
