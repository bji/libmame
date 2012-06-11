/***************************************************************************

X Tom 3D

TODO:
- clears a work RAM snippet then jumps to that snippet ... it doesn't do
  that if you soft reset emulation.
- understand how to load game ROMs

This game runs on PC-based hardware.
Major components are....

MAIN BOARD
----------
    CPU: Intel Celeron (socket 370) 333MHz
Chipset: Intel AGPset FW822443ZX, PCIset FW82371EB
    RAM: Samsung KMM366S823CTS 8M x 64-bit SDRAM DIMM
  Video: 3DFX 500-0013-04 PCB-mounted BGA
         EliteMT M32L1632512A video RAM (x4)
         14.31818MHz XTAL
   BIOS: Atmel 29C010 flash ROM
  Other: Holtek HT6542B i8042-based keyboard controller
         3V coin battery

SOUND BOARD
-----------
A40MX04 QFP84 CPLD
Yamaha YMZ280B + YAC516
16MHz XTAL
PIC12C508 (secured, not read)
Atmel 93C46 EEPROM
LM358 OP AMP (x3)

ROM BOARD
---------
MX29F1610MC 16M FlashROM (x7)

***************************************************************************/

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/i386/i386.h"
#include "memconv.h"
#include "devconv.h"
#include "machine/8237dma.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"
#include "machine/pci.h"
#include "machine/8042kbdc.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "video/pc_vga.h"


class xtom3d_state : public driver_device
{
public:
	xtom3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_pit8254(*this, "pit8254"),
		  m_dma8237_1(*this, "dma8237_1"),
		  m_dma8237_2(*this, "dma8237_2"),
		  m_pic8259_1(*this, "pic8259_1"),
		  m_pic8259_2(*this, "pic8259_2")
		  { }

	UINT32 *m_bios_ram;
	UINT32 *m_bios_ext1_ram;
	UINT32 *m_bios_ext2_ram;
	UINT32 *m_bios_ext3_ram;
	UINT32 *m_bios_ext4_ram;
	UINT32 *m_isa_ram1;
	UINT32 *m_isa_ram2;
	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];
	UINT8 m_mxtc_config_reg[256];
	UINT8 m_piix4_config_reg[4][256];

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<pit8254_device> m_pit8254;
	required_device<i8237_device> m_dma8237_1;
	required_device<i8237_device> m_dma8237_2;
	required_device<pic8259_device> m_pic8259_1;
	required_device<pic8259_device> m_pic8259_2;

	DECLARE_READ8_MEMBER( get_slave_ack );

	DECLARE_WRITE32_MEMBER( isa_ram1_w );
	DECLARE_WRITE32_MEMBER( isa_ram2_w );

	DECLARE_WRITE32_MEMBER( bios_ext1_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext2_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext3_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext4_ram_w );

	DECLARE_WRITE32_MEMBER( bios_ram_w );
};

// Intel 82439TX System Controller (MXTC)

static UINT8 mxtc_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	xtom3d_state *state = busdevice->machine().driver_data<xtom3d_state>();
//  mame_printf_debug("MXTC: read %d, %02X\n", function, reg);

	return state->m_mxtc_config_reg[reg];
}

static void mxtc_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	xtom3d_state *state = busdevice->machine().driver_data<xtom3d_state>();
	printf("MXTC: write %d, %02X, %02X\n",  function, reg, data);

	/*
    memory banking with North Bridge:
    0x59 (PAM0) xxxx ---- BIOS area 0xf0000-0xfffff
                ---- xxxx Reserved
    0x5a (PAM1) xxxx ---- ISA add-on BIOS 0xc4000 - 0xc7fff
                ---- xxxx ISA add-on BIOS 0xc0000 - 0xc3fff
    0x5b (PAM2) xxxx ---- ISA add-on BIOS 0xcc000 - 0xcffff
                ---- xxxx ISA add-on BIOS 0xc8000 - 0xcbfff
    0x5c (PAM3) xxxx ---- ISA add-on BIOS 0xd4000 - 0xd7fff
                ---- xxxx ISA add-on BIOS 0xd0000 - 0xd3fff
    0x5d (PAM4) xxxx ---- ISA add-on BIOS 0xdc000 - 0xdffff
                ---- xxxx ISA add-on BIOS 0xd8000 - 0xdbfff
    0x5e (PAM5) xxxx ---- BIOS extension 0xe4000 - 0xe7fff
                ---- xxxx BIOS extension 0xe0000 - 0xe3fff
    0x5f (PAM6) xxxx ---- BIOS extension 0xec000 - 0xeffff
                ---- xxxx BIOS extension 0xe8000 - 0xebfff

    3210 -> 3 = reserved, 2 = Cache Enable, 1 = Write Enable, 0 = Read Enable
    */

	switch(reg)
	{
		case 0x59: // PAM0
		{
			if (data & 0x10)		// enable RAM access to region 0xf0000 - 0xfffff
				memory_set_bankptr(busdevice->machine(), "bios_bank", state->m_bios_ram);
			else					// disable RAM access (reads go to BIOS ROM)
				memory_set_bankptr(busdevice->machine(), "bios_bank", busdevice->machine().region("bios")->base() + 0x10000);
			break;
		}
		case 0x5a: // PAM1
		{
			if (data & 0x1)
				memory_set_bankptr(busdevice->machine(), "video_bank1", state->m_isa_ram1);
			else
				memory_set_bankptr(busdevice->machine(), "video_bank1", busdevice->machine().region("video_bios")->base() + 0);

			if (data & 0x10)
				memory_set_bankptr(busdevice->machine(), "video_bank2", state->m_isa_ram2);
			else
				memory_set_bankptr(busdevice->machine(), "video_bank2", busdevice->machine().region("video_bios")->base() + 0x4000);

			break;
		}
		case 0x5e: // PAM5
		{
			if (data & 0x1)
				memory_set_bankptr(busdevice->machine(), "bios_ext1", state->m_bios_ext1_ram);
			else
				memory_set_bankptr(busdevice->machine(), "bios_ext1", busdevice->machine().region("bios")->base() + 0);

			if (data & 0x10)
				memory_set_bankptr(busdevice->machine(), "bios_ext2", state->m_bios_ext2_ram);
			else
				memory_set_bankptr(busdevice->machine(), "bios_ext2", busdevice->machine().region("bios")->base() + 0x4000);

			break;
		}
		case 0x5f: // PAM6
		{
			if (data & 0x1)
				memory_set_bankptr(busdevice->machine(), "bios_ext3", state->m_bios_ext3_ram);
			else
				memory_set_bankptr(busdevice->machine(), "bios_ext3", busdevice->machine().region("bios")->base() + 0x8000);

			if (data & 0x10)
				memory_set_bankptr(busdevice->machine(), "bios_ext4", state->m_bios_ext4_ram);
			else
				memory_set_bankptr(busdevice->machine(), "bios_ext4", busdevice->machine().region("bios")->base() + 0xc000);

			break;
		}
	}

	state->m_mxtc_config_reg[reg] = data;
}

static void intel82439tx_init(running_machine &machine)
{
	xtom3d_state *state = machine.driver_data<xtom3d_state>();
	state->m_mxtc_config_reg[0x60] = 0x02;
	state->m_mxtc_config_reg[0x61] = 0x02;
	state->m_mxtc_config_reg[0x62] = 0x02;
	state->m_mxtc_config_reg[0x63] = 0x02;
	state->m_mxtc_config_reg[0x64] = 0x02;
	state->m_mxtc_config_reg[0x65] = 0x02;
}

static UINT32 intel82439tx_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= mxtc_config_r(busdevice, device, function, reg + 0) << 0;
	}
	return r;
}

static void intel82439tx_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		mxtc_config_w(busdevice, device, function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		mxtc_config_w(busdevice, device, function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		mxtc_config_w(busdevice, device, function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		mxtc_config_w(busdevice, device, function, reg + 0, (data >> 0) & 0xff);
	}
}

// Intel 82371AB PCI-to-ISA / IDE bridge (PIIX4)

static UINT8 piix4_config_r(device_t *busdevice, device_t *device, int function, int reg)
{
	xtom3d_state *state = busdevice->machine().driver_data<xtom3d_state>();
//  mame_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	return state->m_piix4_config_reg[function][reg];
}

static void piix4_config_w(device_t *busdevice, device_t *device, int function, int reg, UINT8 data)
{
	xtom3d_state *state = busdevice->machine().driver_data<xtom3d_state>();
//  mame_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", machine.describe_context(), function, reg, data);
	state->m_piix4_config_reg[function][reg] = data;
}

static UINT32 intel82371ab_pci_r(device_t *busdevice, device_t *device, int function, int reg, UINT32 mem_mask)
{
	UINT32 r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= piix4_config_r(busdevice, device, function, reg + 0) << 0;
	}
	return r;
}

static void intel82371ab_pci_w(device_t *busdevice, device_t *device, int function, int reg, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		piix4_config_w(busdevice, device, function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		piix4_config_w(busdevice, device, function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		piix4_config_w(busdevice, device, function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		piix4_config_w(busdevice, device, function, reg + 0, (data >> 0) & 0xff);
	}
}


WRITE32_MEMBER(xtom3d_state::isa_ram1_w)
{
	if (m_mxtc_config_reg[0x5a] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram1 + offset);
	}
}

WRITE32_MEMBER(xtom3d_state::isa_ram2_w)
{
	if (m_mxtc_config_reg[0x5a] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram2 + offset);
	}
}

WRITE32_MEMBER(xtom3d_state::bios_ext1_ram_w)
{
	if (m_mxtc_config_reg[0x5e] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext1_ram + offset);
	}
}


WRITE32_MEMBER(xtom3d_state::bios_ext2_ram_w)
{
	if (m_mxtc_config_reg[0x5e] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext2_ram + offset);
	}
}


WRITE32_MEMBER(xtom3d_state::bios_ext3_ram_w)
{
	if (m_mxtc_config_reg[0x5f] & 0x2)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext3_ram + offset);
	}
}


WRITE32_MEMBER(xtom3d_state::bios_ext4_ram_w)
{
	if (m_mxtc_config_reg[0x5f] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext4_ram + offset);
	}
}


WRITE32_MEMBER(xtom3d_state::bios_ram_w)
{
	if (m_mxtc_config_reg[0x59] & 0x20)		// write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram + offset);
	}
}

static READ32_DEVICE_HANDLER( ide_r )
{
	return -1; // crashes otherwise

	return ide_controller32_r(device, 0x1f0/4 + offset, mem_mask);
}

static WRITE32_DEVICE_HANDLER( ide_w )
{
	if(0) // crashes otherwise
		ide_controller32_w(device, 0x1f0/4 + offset, data, mem_mask);
}

static READ32_DEVICE_HANDLER( fdc_r )
{
	return ide_controller32_r(device, 0x3f0/4 + offset, mem_mask);
}

static WRITE32_DEVICE_HANDLER( fdc_w )
{
	//mame_printf_debug("FDC: write %08X, %08X, %08X\n", data, offset, mem_mask);
	ide_controller32_w(device, 0x3f0/4 + offset, data, mem_mask);
}

static READ8_HANDLER(at_page8_r)
{
	xtom3d_state *state = space->machine().driver_data<xtom3d_state>();
	UINT8 data = state->m_at_pages[offset % 0x10];

	switch(offset % 8) {
	case 1:
		data = state->m_dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = state->m_dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = state->m_dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = state->m_dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


static WRITE8_HANDLER(at_page8_w)
{
	xtom3d_state *state = space->machine().driver_data<xtom3d_state>();
	state->m_at_pages[offset % 0x10] = data;

	switch(offset % 8) {
	case 1:
		state->m_dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		state->m_dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		state->m_dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		state->m_dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}

static READ32_HANDLER(at_page32_r)
{
	return read32le_with_read8_handler(at_page8_r, space, offset, mem_mask);
}


static WRITE32_HANDLER(at_page32_w)
{
	write32le_with_write8_handler(at_page8_w, space, offset, data, mem_mask);
}

static READ8_DEVICE_HANDLER(at_dma8237_2_r)
{
	return i8237_r(device, offset / 2);
}

static WRITE8_DEVICE_HANDLER(at_dma8237_2_w)
{
	i8237_w(device, offset / 2, data);
}

static READ32_DEVICE_HANDLER(at32_dma8237_2_r)
{
	return read32le_with_read8_device_handler(at_dma8237_2_r, device, offset, mem_mask);
}

static WRITE32_DEVICE_HANDLER(at32_dma8237_2_w)
{
	write32le_with_write8_device_handler(at_dma8237_2_w, device, offset, data, mem_mask);
}




static WRITE_LINE_DEVICE_HANDLER( pc_dma_hrq_changed )
{
	cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( device, state );
}


static READ8_HANDLER( pc_dma_read_byte )
{
	xtom3d_state *state = space->machine().driver_data<xtom3d_state>();
	offs_t page_offset = (((offs_t) state->m_dma_offset[0][state->m_dma_channel]) << 16)
		& 0xFF0000;

	return space->read_byte(page_offset + offset);
}


static WRITE8_HANDLER( pc_dma_write_byte )
{
	xtom3d_state *state = space->machine().driver_data<xtom3d_state>();
	offs_t page_offset = (((offs_t) state->m_dma_offset[0][state->m_dma_channel]) << 16)
		& 0xFF0000;

	space->write_byte(page_offset + offset, data);
}

static void set_dma_channel(device_t *device, int channel, int state)
{
	xtom3d_state *drvstate = device->machine().driver_data<xtom3d_state>();
	if (!state) drvstate->m_dma_channel = channel;
}

static WRITE_LINE_DEVICE_HANDLER( pc_dack0_w ) { set_dma_channel(device, 0, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack1_w ) { set_dma_channel(device, 1, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack2_w ) { set_dma_channel(device, 2, state); }
static WRITE_LINE_DEVICE_HANDLER( pc_dack3_w ) { set_dma_channel(device, 3, state); }

static I8237_INTERFACE( dma8237_1_config )
{
	DEVCB_LINE(pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, pc_dma_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, pc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_LINE(pc_dack0_w), DEVCB_LINE(pc_dack1_w), DEVCB_LINE(pc_dack2_w), DEVCB_LINE(pc_dack3_w) }
};

static I8237_INTERFACE( dma8237_2_config )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL }
};

static ADDRESS_MAP_START(xtom3d_map, AS_PROGRAM, 32, xtom3d_state)
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM
	AM_RANGE(0x000c0000, 0x000c3fff) AM_ROMBANK("video_bank1") AM_WRITE(isa_ram1_w)
	AM_RANGE(0x000c4000, 0x000c7fff) AM_ROMBANK("video_bank2") AM_WRITE(isa_ram2_w)
	AM_RANGE(0x000e0000, 0x000e3fff) AM_ROMBANK("bios_ext1") AM_WRITE(bios_ext1_ram_w)
	AM_RANGE(0x000e4000, 0x000e7fff) AM_ROMBANK("bios_ext2") AM_WRITE(bios_ext2_ram_w)
	AM_RANGE(0x000e8000, 0x000ebfff) AM_ROMBANK("bios_ext3") AM_WRITE(bios_ext3_ram_w)
	AM_RANGE(0x000ec000, 0x000effff) AM_ROMBANK("bios_ext4") AM_WRITE(bios_ext4_ram_w)
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROMBANK("bios_bank") AM_WRITE(bios_ram_w)
	AM_RANGE(0x00100000, 0x01ffffff) AM_RAM
	AM_RANGE(0xfffe0000, 0xffffffff) AM_ROM AM_REGION("bios", 0)	/* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(xtom3d_io, AS_IO, 32, xtom3d_state)
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8_LEGACY("dma8237_1", i8237_r, i8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_READWRITE_LEGACY(kbdc8042_32le_r,			kbdc8042_32le_w)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write, 0xffffffff) /* todo: nvram (CMOS Setup Save)*/
	AM_RANGE(0x0080, 0x009f) AM_READWRITE_LEGACY(at_page32_r,				at_page32_w)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE_LEGACY("dma8237_2", at32_dma8237_2_r, at32_dma8237_2_w)
	AM_RANGE(0x00e8, 0x00ef) AM_NOP

	AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE_LEGACY("ide", ide_r, ide_w)
	AM_RANGE(0x03f0, 0x03f7) AM_DEVREADWRITE_LEGACY("ide", fdc_r, fdc_w)

	AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE_LEGACY("pcibus", pci_32le_r,	pci_32le_w)
ADDRESS_MAP_END


static const struct pit8253_config xtom3d_pit8254_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir0_w)
		}, {
			4772720/4,				/* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};

static WRITE_LINE_DEVICE_HANDLER( xtom3d_pic8259_1_set_int_line )
{
	xtom3d_state *drvstate = device->machine().driver_data<xtom3d_state>();
	device_set_input_line(drvstate->m_maincpu, 0, state ? HOLD_LINE : CLEAR_LINE);
}

READ8_MEMBER( xtom3d_state::get_slave_ack )
{
	if (offset==2) { // IRQ = 2
		logerror("pic8259_slave_ACK!\n");
		return pic8259_acknowledge(m_pic8259_2);
	}
	return 0x00;
}

static const struct pic8259_interface xtom3d_pic8259_1_config =
{
	DEVCB_LINE(xtom3d_pic8259_1_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_MEMBER(xtom3d_state,get_slave_ack)
};

static const struct pic8259_interface xtom3d_pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

static void set_gate_a20(running_machine &machine, int a20)
{
	xtom3d_state *state = machine.driver_data<xtom3d_state>();

	device_set_input_line(state->m_maincpu, INPUT_LINE_A20, a20);
}

static void keyboard_interrupt(running_machine &machine, int state)
{
	xtom3d_state *drvstate = machine.driver_data<xtom3d_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

static int xtom3d_get_out2(running_machine &machine)
{
	xtom3d_state *state = machine.driver_data<xtom3d_state>();
	return pit8253_get_output(state->m_pit8254, 2 );
}

static const struct kbdc8042_interface at8042 =
{
	KBDC8042_AT386, set_gate_a20, keyboard_interrupt, NULL, xtom3d_get_out2
};

static void xtom3d_set_keyb_int(running_machine &machine, int state)
{
	xtom3d_state *drvstate = machine.driver_data<xtom3d_state>();
	pic8259_ir1_w(drvstate->m_pic8259_1, state);
}

static IRQ_CALLBACK(irq_callback)
{
	xtom3d_state *state = device->machine().driver_data<xtom3d_state>();
	return pic8259_acknowledge( state->m_pic8259_1);
}

static void ide_interrupt(device_t *device, int state)
{
	xtom3d_state *drvstate = device->machine().driver_data<xtom3d_state>();
	pic8259_ir6_w(drvstate->m_pic8259_2, state);
}

static READ8_HANDLER( vga_setting ) { return 0xff; } // hard-code to color

static MACHINE_START( xtom3d )
{
	xtom3d_state *state = machine.driver_data<xtom3d_state>();

	state->m_bios_ram = auto_alloc_array(machine, UINT32, 0x10000/4);
	state->m_bios_ext1_ram = auto_alloc_array(machine, UINT32, 0x4000/4);
	state->m_bios_ext2_ram = auto_alloc_array(machine, UINT32, 0x4000/4);
	state->m_bios_ext3_ram = auto_alloc_array(machine, UINT32, 0x4000/4);
	state->m_bios_ext4_ram = auto_alloc_array(machine, UINT32, 0x4000/4);
	state->m_isa_ram1 = auto_alloc_array(machine, UINT32, 0x4000/4);
	state->m_isa_ram2 = auto_alloc_array(machine, UINT32, 0x4000/4);

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, xtom3d_set_keyb_int);

	device_set_irq_callback(state->m_maincpu, irq_callback);
	intel82439tx_init(machine);

	kbdc8042_init(machine, &at8042);
	pc_vga_init(machine, vga_setting, NULL);
	pc_vga_io_init(machine, machine.device("maincpu")->memory().space(AS_PROGRAM), 0xa0000, machine.device("maincpu")->memory().space(AS_IO), 0x0000);
}

static MACHINE_RESET( xtom3d )
{
	memory_set_bankptr(machine, "bios_bank", machine.region("bios")->base() + 0x10000);
	memory_set_bankptr(machine, "bios_ext1", machine.region("bios")->base() + 0);
	memory_set_bankptr(machine, "bios_ext2", machine.region("bios")->base() + 0x4000);
	memory_set_bankptr(machine, "bios_ext3", machine.region("bios")->base() + 0x8000);
	memory_set_bankptr(machine, "bios_ext4", machine.region("bios")->base() + 0xc000);
	memory_set_bankptr(machine, "video_bank1", machine.region("video_bios")->base() + 0);
	memory_set_bankptr(machine, "video_bank2", machine.region("video_bios")->base() + 0x4000);
}


static MACHINE_CONFIG_START( xtom3d, xtom3d_state )
	MCFG_CPU_ADD("maincpu", PENTIUM, 450000000/16)	// actually Pentium II 450
	MCFG_CPU_PROGRAM_MAP(xtom3d_map)
	MCFG_CPU_IO_MAP(xtom3d_io)

	MCFG_MACHINE_START(xtom3d)
	MCFG_MACHINE_RESET(xtom3d)

	MCFG_PIT8254_ADD( "pit8254", xtom3d_pit8254_config )
	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, dma8237_1_config )
	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, dma8237_2_config )
	MCFG_PIC8259_ADD( "pic8259_1", xtom3d_pic8259_1_config )
	MCFG_PIC8259_ADD( "pic8259_2", xtom3d_pic8259_2_config )

	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )

	MCFG_PCI_BUS_ADD("pcibus", 0)
	MCFG_PCI_BUS_DEVICE(0, NULL, intel82439tx_pci_r, intel82439tx_pci_w)
	MCFG_PCI_BUS_DEVICE(7, NULL, intel82371ab_pci_r, intel82371ab_pci_w)

	MCFG_IDE_CONTROLLER_ADD("ide", ide_interrupt)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )
MACHINE_CONFIG_END


ROM_START( xtom3d )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_LOAD( "bios.u22", 0x000000, 0x020000, CRC(f7c58044) SHA1(fd967d009e0d3c8ed9dd7be852946f2b9dee7671) )

	ROM_REGION( 0x8000, "video_bios", 0 ) // TODO: no VGA card is hooked up, to be removed
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, BAD_DUMP CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )

	ROM_REGION(0xe00000, "user2", 0)
	ROM_LOAD( "u3",  0x000000, 0x200000, CRC(f332e030) SHA1(f04fc7fc97e6ada8122ea7d111455043d7cc42df) )
	ROM_LOAD( "u4",  0x200000, 0x200000, CRC(ac40ea0b) SHA1(6fcb86f493885d62d20df6bddaa1a1b19d478c65) )
	ROM_LOAD( "u5",  0x400000, 0x200000, CRC(0fb98a20) SHA1(d21f33b0ca65dc6f90a411a9682f960e9c60244c) )
	ROM_LOAD( "u6",  0x600000, 0x200000, CRC(5c092c58) SHA1(d347e1ed957cc989dc71f4f347af926589ae926d) )
	ROM_LOAD( "u7",  0x800000, 0x200000, CRC(833c179c) SHA1(586555f5a4066a762fc05a43ef01be9fa202bb7f) )
	ROM_LOAD( "u19", 0xa00000, 0x200000, CRC(a1ae73d0) SHA1(232c73bfee426b5f651a015c505c26b8ed7176b7) )
	ROM_LOAD( "u20", 0xc00000, 0x200000, CRC(452131d9) SHA1(f62a0f1a7da9025ac1f7d5de4df90166871ac1e5) )
ROM_END


GAME(1999, xtom3d, 0, xtom3d, at_keyboard, 0, ROT0, "Jamie System Development", "X Tom 3D", GAME_IS_SKELETON)
