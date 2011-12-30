/*
  ATV Track
  (c)2002 Gaelco

ATV Track
Gaelco 2002

PCB Layout

GAELCO
REF. 020419
 |--------------------------------------------------------------|
 |                                                              |
 |   SW3                                               EPC1PC8  |
 |                                         K4S643232            |
 |    LC245A                                         7LB176    |-|
 |                        FLASH.IC14  FLASH.IC19     7LB176    | |
 |       |-----|                                     7LB176    | |DB9
 |       | SH4 |                                     7LB176    |-|
 |       |     |          FLASH.IC15  FLASH.IC20                |
 |       |-----|                                                |
 |                                                |----------|  |
|-|                            K4S643232          |ALTERA    |  |
| |  L4955                                        |FLEX      |  |
| |      |-----|               K4S643232          |EPF10K50  |  |
| |CN1   | SH4 |                                  |EQC240-3  |  |
| |      |     |                                  |----------|  |
| |      |-----|                                                |
|-|                                                             |
 |                                                              |
 |      33MHz                       K4S643232     |----------|  |
 |                                  K4S643232     | GFX      |  |
 |             LED                                |          | |-|
 |             LED                                |          | | |
 |                                  K4S643232     |          | | |DB9
 |                                  K4S643232     |----------| |-|
 | TL074C   TL074C                                     385-1    |
 |     TDA1387   TDA1387                          14.31818MHz   |
 |                                                              |
 |--------------------------------------------------------------|
Notes:
      SH4       - Hitachi HD6417750S SH4 CPU (BGA)
      K4S643232 - Samsung K4S643232E-TC70 64M x 32-bit SDRAM (TSSOP86)
      GFX       - Unknown BGA graphics chip (heatsinked)
      FLASH.IC* - Samsung K9F2808U0B 128MBit (16M + 512k Spare x 8-bit) FlashROM (TSOP48)
      EPF10K50  - Altera Flex EPF10K50EQC240-3 FPGA (QFP240)
      EPC1PC8   - Altera EPC1PC8 FPGA Configuration Device (DIP8)
      TL074C    - Texas Instruments TL074C Low Noise Quad JFet Operational Amplifier (SOIC14)
      TDA1387   - Philips TDA1387 Stereo Continuous Calibration DAC (SOIC8)
      L4955     - ST Microelectronics L4955 low-power, quad channel, 8-bit buffered voltage output DAC and amplifier
      7LB176    - Texas Instruments 7LB176 Differential Bus Tranceiver (SOIC8)
      385-1     - National LM385 Adjustable Micropower Voltage Reference Diode (SOIC8)
      CN1       - Multi-pin connector for filter board (input, video, power & controls connectors etc)
      DB9       - Probably used for cabinet linking
      SW3       - Push button switch

*/

#include "emu.h"
#include "cpu/sh4/sh4.h"
#include "debugger.h"

const memory_region *nandregion;
int nandcommand[4], nandoffset[4], nandaddressstep, nandaddress[4];
UINT32 area1_data[4];

//#define SPECIALMODE 1 // Alternate code path

class atvtrack_state : public driver_device
{
public:
	atvtrack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};

static void logbinary(UINT32 data,int high=31,int low=0)
{
	UINT32 s;
	int z;

	s=1 << high;
	for (z = high;z >= low;z--) {
		if (data & s)
			logerror("1");
		else
			logerror("0");
		s=s >> 1;
	}
}

static inline UINT32 decode64_32(offs_t offset64, UINT64 data, UINT64 mem_mask, offs_t &offset32)
{
	if (ACCESSING_BITS_0_31) {
		offset32 = offset64 << 1;
		return (UINT32)data;
	}
	if (ACCESSING_BITS_32_63) {
		offset32 = (offset64 << 1)+1;
		return (UINT32)(data >> 32);
	}
	logerror("Wrong word size in external access\n");
	//debugger_break(NULL);
	return 0;
}

static READ64_HANDLER( area1_r )
{
	UINT32 addr;

	addr = 0;
	decode64_32(offset, 0, mem_mask, addr);
	if (addr == (0x00020000-0x00020000)/4)
		return -1;
	else if (addr == (0x00020004-0x00020000)/4)
		return -1;
	return -1;
}

static WRITE64_HANDLER( area1_w )
{
	UINT32 addr, dat; //, old;

	addr = 0;
	dat = decode64_32(offset, data, mem_mask, addr);
//  old = area1_data[addr];
	area1_data[addr] = dat;
	if (addr == (0x00020000-0x00020000)/4) {
		if (data & 4) {
			device_execute_interface *exec = dynamic_cast<device_execute_interface *>(space->machine().device("subcpu"));
			exec->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		}
	}
	logerror("Write %08x at %08x ",dat, 0x20000+addr*4+0);
	logbinary(dat);
	logerror("\n");
}

static READ64_HANDLER( area2_r )
{
	UINT32 addr, dat;
	int c;

	addr = 0;
	dat = decode64_32(offset, 0, mem_mask, addr);
	if (addr == 0) {
		dat = 0;
		for (c = 3;c >= 0;c--) {
			if (nandcommand[c] <= 0x50) {
				addr = nandaddress[c]+nandoffset[c];
				dat = (dat << 8) | nandregion->u8(addr+c);
				nandoffset[c] += 4;
			} else
				dat = (dat << 8) | 0xc0;
		}
		return dat;
	} else
		;
	return 0;
}

static WRITE64_HANDLER( area2_w )
{
//  UINT32 addr, dat;

//  addr = 0;
//  dat = decode64_32(offset, data, mem_mask, addr);
//  if (addr == 0)
//      ;
//  else
//      ;
}

static READ64_HANDLER( area3_r )
{
//  UINT32 addr, dat;

//  addr = 0;
//  dat = decode64_32(offset, 0, mem_mask, addr);
//  if (addr == 0)
//      ;
//  else
//      ;
	return 0;
}

static WRITE64_HANDLER( area3_w )
{
	UINT32 addr; //, dat;
	int c;

	addr = 0;
//  dat = decode64_32(offset, data, mem_mask, addr);
	if (addr == 0) {
		for (c = 0;c < 4;c++) {
			nandcommand[c] = data & 0xff;
			if (nandcommand[c] == 0x00) {
				nandoffset[c] = 0;
			} else if (nandcommand[c] == 0x01) {
				nandoffset[c] = 256*4;
			} else if (nandcommand[c] == 0x50) {
				nandoffset[c] = 512*4;
			} else if (nandcommand[c] == 0x90) {
			} else if (nandcommand[c] == 0xff) {
			} else if (nandcommand[c] == 0x80) {
			} else if (nandcommand[c] == 0x60) {
			} else if (nandcommand[c] == 0x70) {
			} else if (nandcommand[c] == 0x10) {
			} else if (nandcommand[c] == 0xd0) {
			} else {
				nandcommand[c] = 0xff;
			}
			data=data >> 8;
		}
		nandaddressstep = 0;
	} else
		;
}

static READ64_HANDLER( area4_r )
{
//  UINT32 addr, dat;

//  addr = 0;
//  dat = decode64_32(offset, 0, mem_mask, addr);
//  if (addr == 0)
//      ;
//  else
//      ;
	return 0;
}

static WRITE64_HANDLER( area4_w )
{
	UINT32 addr; //, dat;
	int c;

	addr = 0;
//  dat = decode64_32(offset, data, mem_mask, addr);
	if (addr == 0) {
		for (c = 0;c < 4;c++) {
			if (nandaddressstep == 0) {
				nandaddress[c] = (data & 0xff)*4;
			} else if (nandaddressstep == 1) {
				nandaddress[c] = nandaddress[c]+(data & 0xff)*0x840;
			} else if (nandaddressstep == 2) {
				nandaddress[c] = nandaddress[c]+(data & 0xff)*0x84000;
			}
			data = data >> 8;
		}
		nandaddressstep++;
	} else
		;
}

static READ64_HANDLER( ioport_r )
{
	if (offset == SH4_IOPORT_16/8) {
		// much simplified way
		if (strcmp(space->device().tag(), "maincpu") == 0)
#ifndef SPECIALMODE
			return -1; // normal
#else
			return 0; // testing
#endif
		else
			return 0; // unknown
	}
	return 0;
}

static WRITE64_HANDLER( ioport_w )
{
#ifdef SPECIALMODE
	UINT64 d;
	static int cnt=0;
	sh4_device_dma dm;
#endif

	if (offset == SH4_IOPORT_16/8) {
		logerror("SH4 16bit i/o port write ");
		logbinary((UINT32)data,15,0);
		logerror("\n");
	}
#ifdef SPECIALMODE
	if (offset == SH4_IOPORT_DMA/8) {
		dm.buffer = &d;
		dm.channel = data & 0xffff;
		dm.length = 1;
		dm.size = 4;
		if (cnt == 0)
			d=0x12340153;
		else
			d=0x11223344;
		if (cnt == 0)
			sh4_dma_data(space->cpu,&dm);
		else
			sh4_dma_data(space->cpu,&dm);
		cnt++;
	}
#endif
}

VIDEO_START(atvtrack)
{
}

SCREEN_UPDATE(atvtrack)
{
	return 0;
}

static MACHINE_START(atvtrack)
{
	UINT8 *src, *dst;
	address_space *as;

	nandaddressstep = 0;
	nandregion = machine.region("maincpu");
	as = machine.device("maincpu")->memory().space(AS_PROGRAM);
	dst = (UINT8 *)(as->get_write_ptr(0x0c7f0000));
	src = nandregion->base()+0x10;
	// copy 0x10000 bytes from region "maincpu" offset 0x10 to 0x0c7f0000
	memcpy(dst, src, 0x10000);
}

static MACHINE_RESET(atvtrack)
{
	address_space *as;

	// Probably just after reset the cpu executes some bootsrtap routine from a memory inside the fpga.
	// The routine initializes the cpu, copies the boot program from the flash memories into the cpu sdram
	// and finally executes it.
	// Here there is the setup of the cpu, the boot program is copied in machine_start
	as = machine.device("maincpu")->memory().space(AS_PROGRAM);
	// set cpu PC register to 0x0c7f0000
	cpu_set_reg(machine.device("maincpu"), STATE_GENPC, 0x0c7f0000);
	// set BCR2 to 1
	sh4_internal_w(as, 0x3001, 1, 0xffffffff);
	device_execute_interface *exec = dynamic_cast<device_execute_interface *>(machine.device("subcpu"));
	exec->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

static ADDRESS_MAP_START( atvtrack_main_map, AS_PROGRAM, 64 )
	AM_RANGE(0x00000000, 0x000003ff) AM_RAM AM_SHARE("sharedmem")
	AM_RANGE(0x00020000, 0x00020007) AM_READWRITE(area1_r, area1_w)
	AM_RANGE(0x14000000, 0x14000007) AM_READWRITE(area2_r, area2_w) // data
	AM_RANGE(0x14100000, 0x14100007) AM_READWRITE(area3_r, area3_w) // command
	AM_RANGE(0x14200000, 0x14200007) AM_READWRITE(area4_r, area4_w) // address
	AM_RANGE(0x0c000000, 0x0cffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( atvtrack_main_port, AS_IO, 64 )
	AM_RANGE(0x00, 0x1f) AM_READWRITE(ioport_r, ioport_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( atvtrack_sub_map, AS_PROGRAM, 64 )
	AM_RANGE(0x00000000, 0x000003ff) AM_RAM AM_SHARE("sharedmem")
	AM_RANGE(0x0c000000, 0x0cffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( atvtrack_sub_port, AS_IO, 64 )
	/*AM_RANGE(0x00, 0x1f) AM_READWRITE(ioport_r, ioport_w) */
ADDRESS_MAP_END


static INPUT_PORTS_START( atvtrack )
INPUT_PORTS_END

// ?
#define ATV_CPU_CLOCK 200000000
// ?
static const struct sh4_config sh4cpu_config = {  1,  0,  1,  0,  0,  0,  1,  1,  0, ATV_CPU_CLOCK };

static MACHINE_CONFIG_START( atvtrack, atvtrack_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH4LE, ATV_CPU_CLOCK)
	MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(atvtrack_main_map)
	MCFG_CPU_IO_MAP(atvtrack_main_port)

	MCFG_CPU_ADD("subcpu", SH4LE, ATV_CPU_CLOCK)
	MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(atvtrack_sub_map)
	MCFG_CPU_IO_MAP(atvtrack_sub_port)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))  /* not accurate */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE(atvtrack)

	MCFG_PALETTE_LENGTH(0x1000)

	MCFG_MACHINE_RESET(atvtrack)
	MCFG_MACHINE_START(atvtrack)
	MCFG_VIDEO_START(atvtrack)
MACHINE_CONFIG_END

ROM_START( atvtrack )
	ROM_REGION( 0x4200000, "maincpu", ROMREGION_ERASEFF) // NAND roms, contain additional data hence the sizes
	ROM_LOAD32_BYTE("15.bin", 0x0000000, 0x1080000, CRC(84eaede7) SHA1(6e6230165c3bb35e49c660dfd0d07c132ed89e6a) )
	ROM_LOAD32_BYTE("20.bin", 0x0000001, 0x1080000, CRC(649dc331) SHA1(0cac2d0c15dd564c7fdebdf4365422958f453d63) )
	ROM_LOAD32_BYTE("14.bin", 0x0000002, 0x1080000, CRC(67983453) SHA1(05389a0ffc1a1bae9bac16a53a97d78b6eccc626) )
	ROM_LOAD32_BYTE("19.bin", 0x0000003, 0x1080000, CRC(9fc5c579) SHA1(8829329ef229564952aea2108ef1750dc226cbac) )

	ROM_REGION( 0x20000, "eeprom", ROMREGION_ERASEFF)
	ROM_LOAD("epc1pc8.ic23", 0x0000000, 0x1ff01, CRC(752444c7) SHA1(c77e8fcfcbe15b53eda25553763bdac45f0ef7df) ) // contains configuration data for the fpga, maybe used for some form of protection
ROM_END

ROM_START( atvtracka )
	ROM_REGION( 0x4200000, "maincpu", ROMREGION_ERASEFF) // NAND roms, contain additional data hence the sizes
	ROM_LOAD32_BYTE("k9f2808u0b.ic15", 0x0000000, 0x1080000, CRC(10730001) SHA1(48c685a6ff7135abd074dc7fb7d10834c44da58f) )
	ROM_LOAD32_BYTE("k9f2808u0b.ic20", 0x0000001, 0x1080000, CRC(b0c34433) SHA1(852c79bb3d7082cd2c056140071ae7d71679ec1d) )
	ROM_LOAD32_BYTE("k9f2808u0b.ic14", 0x0000002, 0x1080000, CRC(02a12085) SHA1(acb112c9c7b29d92610465fb92268ce787ca06f4) )
	ROM_LOAD32_BYTE("k9f2808u0b.ic19", 0x0000003, 0x1080000, CRC(856c1e6a) SHA1(a6b2839120d61811c36cc6b4095de9cefceb394b) )
ROM_END

GAME( 2002, atvtrack,  0,          atvtrack,    atvtrack,    0, ROT0, "Gaelco", "ATV Track (set 1)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2002, atvtracka, atvtrack,   atvtrack,    atvtrack,    0, ROT0, "Gaelco", "ATV Track (set 2)", GAME_NOT_WORKING | GAME_NO_SOUND )
