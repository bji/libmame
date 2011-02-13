/**********************************************************************

    DALLAS DS2404

    RTC + BACKUP RAM

**********************************************************************/

#ifndef __DS2404_H__
#define __DS2404_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_DS2404_ADD(_tag, _ref_year, _ref_month, _ref_day) \
	MDRV_DEVICE_ADD(_tag, DS2404, 0) \
	MDRV_DS2404_REF_YEAR(_ref_year) \
	MDRV_DS2404_REF_MONTH(_ref_month) \
	MDRV_DS2404_REF_DAY(_ref_day)

#define MDRV_DS2404_REF_YEAR(_ref_year) \
	ds2404_device_config::static_set_ref_year(device, _ref_year);

#define MDRV_DS2404_REF_MONTH(_ref_month) \
	ds2404_device_config::static_set_ref_month(device, _ref_month);

#define MDRV_DS2404_REF_DAY(_ref_day) \
	ds2404_device_config::static_set_ref_day(device, _ref_day);


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ds2404_device_config

class ds2404_device_config :  public device_config,
							  public device_config_nvram_interface
{
    friend class ds2404_device;

    // construction/destruction
    ds2404_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

	// inline configuration helpers
	static void static_set_ref_year(device_config *device, UINT32 m_ref_year);
	static void static_set_ref_month(device_config *device, UINT8 m_ref_month);
	static void static_set_ref_day(device_config *device, UINT8 m_ref_day);

protected:
    // internal state goes here
	UINT32	m_ref_year;
	UINT8	m_ref_month;
	UINT8	m_ref_day;
};



// ======================> ds2404_device

class ds2404_device :  public device_t,
					   public device_nvram_interface
{
    friend class ds2404_device_config;

    // construction/destruction
    ds2404_device(running_machine &_machine, const ds2404_device_config &_config);

public:

	/* 1-wire interface reset  */
	void ds2404_1w_reset_w(UINT32 offset, UINT8 data);

	/* 3-wire interface reset  */
	void ds2404_3w_reset_w(UINT32 offset, UINT8 data);

	UINT8 ds2404_data_r(UINT32 offset);
	void ds2404_data_w(UINT32 offset, UINT8 data);
	void ds2404_clk_w(UINT32 offset, UINT8 data);

	void ds2404_tick();

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset() { }
    virtual void device_post_load() { }
    virtual void device_clock_changed() { }

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(mame_file &file);
	virtual void nvram_write(mame_file &file);

	static TIMER_CALLBACK( ds2404_tick_callback );

private:

	void ds2404_rom_cmd(UINT8 cmd);
	void ds2404_cmd(UINT8 cmd);

	UINT8 ds2404_readmem();
	void ds2404_writemem(UINT8 value);

	enum DS2404_STATE
	{
		DS2404_STATE_IDLE = 1,				/* waiting for ROM command, in 1-wire mode */
		DS2404_STATE_COMMAND,				/* waiting for memory command */
		DS2404_STATE_ADDRESS1,				/* waiting for address bits 0-7 */
		DS2404_STATE_ADDRESS2,				/* waiting for address bits 8-15 */
		DS2404_STATE_OFFSET,				/* waiting for ending offset */
		DS2404_STATE_INIT_COMMAND,
		DS2404_STATE_READ_MEMORY,			/* Read Memory command active */
		DS2404_STATE_WRITE_SCRATCHPAD,		/* Write Scratchpad command active */
		DS2404_STATE_READ_SCRATCHPAD,		/* Read Scratchpad command active */
		DS2404_STATE_COPY_SCRATCHPAD		/* Copy Scratchpad command active */
	};

	UINT16 m_address;
	UINT16 m_offset;
	UINT16 m_end_offset;
	UINT8 m_a1;
	UINT8 m_a2;
	UINT8 m_sram[512];	/* 4096 bits */
	UINT8 m_ram[32];	/* scratchpad ram, 256 bits */
	UINT8 m_rtc[5];		/* 40-bit RTC counter */
	DS2404_STATE m_state[8];
	int m_state_ptr;

    const ds2404_device_config &m_config;
};


// device type definition
extern const device_type DS2404;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* 1-wire interface reset */
WRITE8_DEVICE_HANDLER( ds2404_1w_reset_w );

/* 3-wire interface reset  */
WRITE8_DEVICE_HANDLER( ds2404_3w_reset_w );

READ8_DEVICE_HANDLER( ds2404_data_r );
WRITE8_DEVICE_HANDLER( ds2404_data_w );
WRITE8_DEVICE_HANDLER( ds2404_clk_w );

#endif
