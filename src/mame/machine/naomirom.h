#ifndef _NAOMIROM_H_
#define _NAOMIROM_H_

#include "naomibd.h"

#define MCFG_NAOMI_ROM_BOARD_ADD(_tag, _eeprom_tag, _maincpu_tag, _irq_cb) \
	MCFG_NAOMI_BOARD_ADD(_tag, NAOMI_ROM_BOARD, _eeprom_tag, _maincpu_tag, _irq_cb)

class naomi_rom_board : public naomi_board
{
public:
	naomi_rom_board(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();

	virtual void board_setup_address(UINT32 address, bool is_dma);
	virtual void board_get_buffer(UINT8 *&base, UINT32 &limit);
	virtual void board_advance(UINT32 size);

private:
	UINT32 rom_cur_address;
};

extern const device_type NAOMI_ROM_BOARD;

#endif
