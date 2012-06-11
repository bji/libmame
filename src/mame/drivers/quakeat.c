/*

 Quake Arcade Tournament

 This is unknown PC hardware, only the HDD is dumped.  The HDD is stickered 'Release Beta 2'

 I've also seen CDs of this for sale, so maybe there should be a CD too, for the music?

TODO:
can't be emulated without proper mb bios

 -- set info

Quake Arcade Tournament by Lazer-Tron

PC running Windows 95 with a Dongle on the parallel port

Created .chd with version 0.125

It found the following disk paramaters...

Input offset    511
Cyclinders  263
Heads       255
Sectors     63
Byte/Sector 512
Sectors/Hunk    8
Logical size    2,1163,248,864


The "backup" directory on hard disk was created by the dumper.


 -- Hardware info found in the following press release:
http://www.wave-report.com/archives/1998/98170702.htm

QUANTUM3D'S HEAVY METAL SYSTEM - HM233G
NLX form factor system that is based on the Intel 440LX chipset
233MHz Intel Pentium II processor with 512KB of L2 cache
32MB of SDRAM
Microsoft Windows 95 OSR2.5
shock-mounted 3.1GB Ultra DMA-33 hard drive
12-24x CD-ROM drive
1.44 MB floppy drive
16-bit per sample 3D audio
PCI-based 2D/VGA
built-in 10/100 Ethernet
Obsidian2 90-4440 AGP Voodoo2-based realtime 3D graphics accelerator
Quantum3D's GCI (Game Control Interface) - a unique, low-cost subsystem
    designed to interface coin-op and industrial input/output control devices to a PC

===============================================================================
TODO:
    * Add BIOS dump (custom 440LX motherboard or standard?)
    * Hook up PC hardware
    * Hook up the GCI (details? ROMs?)
    * What's the dongle do?
===============================================================================
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pic8259.h"
/* Insert IBM PC includes here */


class quakeat_state : public driver_device
{
public:
	quakeat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	device_t	*m_pic8259_1;
	device_t	*m_pic8259_2;
};


static VIDEO_START(quake)
{
}

static SCREEN_UPDATE_IND16(quake)
{
	return 0;
}

static ADDRESS_MAP_START( quake_map, AS_PROGRAM, 32, quakeat_state )
	AM_RANGE(0x00000000, 0x0000ffff) AM_ROM AM_REGION("pc_bios", 0) /* BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START( quake_io, AS_IO, 32, quakeat_state )
//  AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8_LEGACY("dma8237_1", dma8237_r, dma8237_w, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8_LEGACY("pic8259_1", pic8259_r, pic8259_w, 0xffffffff)
//  AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8_LEGACY("pit8254", pit8253_r, pit8253_w, 0xffffffff)
//  AM_RANGE(0x0060, 0x006f) AM_READWRITE_LEGACY(kbdc8042_32le_r,          kbdc8042_32le_w)
//  AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write, 0xffffffff)
//  AM_RANGE(0x0080, 0x009f) AM_READWRITE_LEGACY(at_page32_r,              at_page32_w)
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8_LEGACY("pic8259_2", pic8259_r, pic8259_w, 0xffffffff)
//  AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE_LEGACY("dma8237_2", at32_dma8237_2_r, at32_dma8237_2_w)
	AM_RANGE(0x00e8, 0x00eb) AM_NOP
//  AM_RANGE(0x01f0, 0x01f7) AM_DEVREADWRITE_LEGACY("ide", ide_r, ide_w)
	AM_RANGE(0x0300, 0x03af) AM_NOP
	AM_RANGE(0x03b0, 0x03df) AM_NOP
//  AM_RANGE(0x0278, 0x027b) AM_WRITE_LEGACY(pnp_config_w)
//  AM_RANGE(0x03f0, 0x03ff) AM_DEVREADWRITE_LEGACY("ide", fdc_r, fdc_w)
//  AM_RANGE(0x0a78, 0x0a7b) AM_WRITE_LEGACY(pnp_data_w)
//  AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE_LEGACY("pcibus", pci_32le_r,  pci_32le_w)
ADDRESS_MAP_END

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

static WRITE_LINE_DEVICE_HANDLER( quakeat_pic8259_1_set_int_line )
{
	cputag_set_input_line(device->machine(), "maincpu", 0, state ? HOLD_LINE : CLEAR_LINE);
}

static READ8_DEVICE_HANDLER( get_slave_ack )
{
	quakeat_state *state = device->machine().driver_data<quakeat_state>();
	if (offset==2) { // IRQ = 2
		return pic8259_acknowledge(state->m_pic8259_2);
	}
	return 0x00;
}

static const struct pic8259_interface quakeat_pic8259_1_config =
{
	DEVCB_LINE(quakeat_pic8259_1_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_HANDLER(get_slave_ack)
};

static const struct pic8259_interface quakeat_pic8259_2_config =
{
	DEVCB_DEVICE_LINE("pic8259_1", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

/*************************************************************/

static INPUT_PORTS_START( quake )
INPUT_PORTS_END

/*************************************************************/

static IRQ_CALLBACK(irq_callback)
{
	quakeat_state *state = device->machine().driver_data<quakeat_state>();
	return pic8259_acknowledge( state->m_pic8259_1);
}

static MACHINE_START(quakeat)
{
	quakeat_state *state = machine.driver_data<quakeat_state>();
	device_set_irq_callback(machine.device("maincpu"), irq_callback);

	state->m_pic8259_1 = machine.device( "pic8259_1" );
	state->m_pic8259_2 = machine.device( "pic8259_2" );
}
/*************************************************************/

static MACHINE_CONFIG_START( quake, quakeat_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PENTIUM, 233000000) /* Pentium II, 233MHz */
	MCFG_CPU_PROGRAM_MAP(quake_map)
	MCFG_CPU_IO_MAP(quake_io)

	MCFG_MACHINE_START(quakeat)

	MCFG_PIC8259_ADD( "pic8259_1", quakeat_pic8259_1_config )
	MCFG_PIC8259_ADD( "pic8259_2", quakeat_pic8259_2_config )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(quake)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_START(quake)
MACHINE_CONFIG_END


ROM_START(quake)
	ROM_REGION32_LE(0x20000, "pc_bios", 0)	/* motherboard bios */
	ROM_LOAD("quakearcadetournament.pcbios", 0x000000, 0x10000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "quakeat", 0, SHA1(c44695b9d521273c9d3c0e18c88f0dca0185bd7b) )
ROM_END


GAME( 1998, quake,  0,   quake, quake, 0, ROT0, "Lazer-Tron / iD Software", "Quake Arcade Tournament (Release Beta 2)", GAME_IS_SKELETON )
