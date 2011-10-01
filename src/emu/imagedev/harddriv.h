/*********************************************************************

    harddriv.h

    Interface to the CHD code

*********************************************************************/

#ifndef HARDDRIV_H
#define HARDDRIV_H

#include "image.h"
#include "harddisk.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> harddisk_interface

struct harddisk_interface
{
	device_image_load_func		m_device_image_load;
	device_image_unload_func	m_device_image_unload;
};

// ======================> harddisk_image_device

class harddisk_image_device :	public device_t,
								public harddisk_interface,
								public device_image_interface
{
public:
	// construction/destruction
	harddisk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~harddisk_image_device();

	// image-level overrides
	virtual bool call_load();
	virtual bool call_create(int create_format, option_resolution *create_args);
	virtual void call_unload();

	virtual iodevice_t image_type() const { return IO_HARDDISK; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *file_extensions() const { return "chd,hd"; }
	virtual const option_guide *create_option_guide() const;

	// specific implementation
	hard_disk_file *get_hard_disk_file() { return m_hard_disk_handle; }
	chd_file *get_chd_file();

protected:
	// device-level overrides
    virtual void device_config_complete();
	virtual void device_start();

	int internal_load_hd(const char *metadata);

	chd_file		*m_chd;
	hard_disk_file	*m_hard_disk_handle;

	image_device_format m_format;
};

// device type definition
extern const device_type HARDDISK;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_HARDDISK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, HARDDISK, 0) \

#define MCFG_HARDDISK_CONFIG_ADD(_tag,_config) \
	MCFG_DEVICE_ADD(_tag, HARDDISK, 0) \
	MCFG_DEVICE_CONFIG(_config)	\

#endif /* HARDDRIV_H */
