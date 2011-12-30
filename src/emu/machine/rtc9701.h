/***************************************************************************

    rtc9701.h

    Serial rtc9701s.

***************************************************************************/

#pragma once

#ifndef __rtc9701DEV_H__
#define __rtc9701DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RTC9701_ADD(_tag, _clock ) \
	MCFG_DEVICE_ADD(_tag, rtc9701, _clock) \


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


typedef enum
{
	RTC9701_CMD_WAIT = 0,
	RTC9701_RTC_READ,
	RTC9701_RTC_WRITE,
	RTC9701_EEPROM_READ,
	RTC9701_EEPROM_WRITE,
	RTC9701_AFTER_WRITE_ENABLE,

} rtc9701_state_t;

typedef struct
{
	UINT8 sec, min, hour, day, wday, month, year;
} rtc_regs_t;


// ======================> rtc9701_device

class rtc9701_device :	public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	rtc9701_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);


	// I/O operations
	DECLARE_WRITE_LINE_MEMBER( write_bit );
	DECLARE_READ_LINE_MEMBER( read_bit );
	DECLARE_WRITE_LINE_MEMBER( set_cs_line );
	DECLARE_WRITE_LINE_MEMBER( set_clock_line );
	void timer_callback();

protected:
	// device-level overrides
	virtual bool device_validity_check(emu_options &options, const game_driver &driver) const;
	virtual void device_start();
	virtual void device_reset();

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);
	inline UINT8 rtc_read(UINT8 offset);
	inline void rtc_write(UINT8 offset,UINT8 data);

	static TIMER_CALLBACK( rtc_inc_callback );

	int 					m_latch;
	int						m_reset_line;
	int						m_clock_line;


	rtc9701_state_t rtc_state;
	int cmd_stream_pos;
	int current_cmd;

	int rtc9701_address_pos;
	int rtc9701_current_address;

	UINT16 rtc9701_current_data;
	int rtc9701_data_pos;

	UINT16 rtc9701_data[0x100];

	rtc_regs_t m_rtc;
};


// device type definition
extern const device_type rtc9701;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
