/****************************************************************************

    printer.c

    Code for handling printer devices

****************************************************************************/

#include "emu.h"
#include "printer.h"


// device type definition
const device_type PRINTER = &device_creator<printer_image_device>;

//-------------------------------------------------
//  printer_image_device - constructor
//-------------------------------------------------

printer_image_device::printer_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, PRINTER, "Printer", tag, owner, clock),
	  device_image_interface(mconfig, *this)
{

}

//-------------------------------------------------
//  printer_image_device - destructor
//-------------------------------------------------

printer_image_device::~printer_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void printer_image_device::device_config_complete()
{
	// inherit a copy of the static data
	const printer_interface *intf = reinterpret_cast<const printer_interface *>(static_config());
	if (intf != NULL)
		*static_cast<printer_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_online, 0, sizeof(m_online));
	}

	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void printer_image_device::device_start()
{
    m_online_func.resolve(m_online, *this);
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    printer_is_ready - checks to see if a printer
    is ready
-------------------------------------------------*/

int printer_image_device::is_ready()
{
	return exists() != 0;
}



/*-------------------------------------------------
    printer_output - outputs data to a printer
-------------------------------------------------*/

void printer_image_device::output(UINT8 data)
{
	if (exists())
	{
		fwrite(&data, 1);
	}
}


/*-------------------------------------------------
    DEVICE_IMAGE_LOAD( printer )
-------------------------------------------------*/
bool printer_image_device::call_load()
{
	/* send notify that the printer is now online */
	if (!m_online_func.isnull())
		m_online_func(TRUE);

	/* we don't need to do anything special */
	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
    DEVICE_IMAGE_UNLOAD( printer )
-------------------------------------------------*/
void printer_image_device::call_unload()
{
	/* send notify that the printer is now offline */
	if (!m_online_func.isnull())
		m_online_func(FALSE);
}

