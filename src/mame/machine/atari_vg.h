/***************************************************************************

    Atari vector hardware

***************************************************************************/

#ifndef __ATARIVGEAROM_H__
#define __ATARIVGEAROM_H__

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_ATARIVGEAROM_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ATARIVGEAROM, 0)


#define EAROM_SIZE	0x40

// ======================> atari_vg_earom_device

class atari_vg_earom_device :	public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	atari_vg_earom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);
public:
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE8_MEMBER( ctrl_w );
private:
	int m_offset;
	int m_data;
	char m_rom[EAROM_SIZE];
};

// device type definition
extern const device_type ATARIVGEAROM;

#endif
