/************************************************************************

 'RISC PC' hardware

 See See Find Out [Icarus 1999]
 Pang Pang Car [Icarus 1999]
 Tetris Fighters [Sego Entertainment 2001]

 driver by
  Tomasz Slanina  analog[at]op.pl

TODO:
 - move PS7500 stuff to machine/ps7500.c
 - 24c01 eeprom (IOLINES)
 - timing
 - unknown reads/writes
 - sound

*************************************************************************************************************

See See Find Out
Icarus, 1999

PCB Layout
----------

ARIAMEDIA M/B Rev 1.0
|--------------------------------------|
|          B.U29          C.U12        |
|      A.U28     *     *     D.U13     |
|                               1008S-1|
|                    |------|          |
|    KM416C1204      |QS1000|   E.U14  |
|J       KM416C1204  |------| |--------|
|A                     24MHz  |DU2  DU3|
|M     |---------|            |        |
|M     |CL-PS7500|  |------|  |DU5  DU6|
|A     |         |  |QL2003|  |--------|
|      |         |  |      | 24C01A.U36|
|      |         |  |------|           |
|      |---------|          14.31818MHz|
|            54MHz  |------|           |
|                   |PRIME |           |
|                   |------|   DIPSW(8)|
|--------------------------------------|

Notes:

      Chips:
         QS1000: QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
           PRIME: LGS Prime 3C 9930R, clock input of 14.31818MHz (QFP100)
         QL2003: QuickLogic QL2003-XPL84C 0003BA FPGA (PLCC84)
      CL-PS7500: Cirrus Logic CL-PS7500FE-56QC-A 84903-951BD ARM 9843J
                 clock input of 54.000MHz, ARM710C; ARM7-core CPU (QFP240)
     KM416C1204: Samsung KM416C1204CJ-5 2M x8 DRAM (SOJ42)
              *: Unpopulated DIP32 socket

      ROMs:
           24C010A: Amtel 24C01A (128bytes x8Bit Serial EEPROM, DIP8)
        A, B, C, D: TMS27C040 512K x8 EPROM (DIP32)
                 E: ST M27C512 64K x8 EPROM (DIP28)
           1008S-1: HWASS 1008S-1 Wavetable Audio Samples chip, 1M x8 MaskROM (SOP32)
         DU2, DU3,: Samsung KM29W32000AT 32MBit NAND Flash 3.3V Serial EEPROM (TSOP44)
         DU5, DU6   These ROMs are mounted on a small plug-in daughterboard. There are additional
                    mounting pads for 4 more of these ROMs but they're not populated.


**************************************************************************************************************

Pong Pong Car
Icarus, 1999

This game runs on hardware that is similar to that used on 'See See Find Out'
The game is a rip-off of RallyX

PCB Layout
----------

|--------------------------------------|
|    KM416C1204                        |
|    KM416C1204    U24  U25  U26*  U27*|
|  DA1311A                             |
|  DA1311A        |---------|54MHz     |
|  4558           |CL-PS7500| LED      |
|J                |         | |--------|
|A                |         | |DU2  DU3|
|M                |         | |        |
|M EL2386       @ |---------| |DU5  DU6|
|A       &                    |--------|
|                                      |
|  7660             |------|           |
|                   |PRIME |14.31818MHz|
|                   |------|           |
| NASN9289  XILINX                     |
| QS1000    XCS10    MAX232    DIPSW(8)|
|  24MHz     #   7705                  |
|--------------------|DB9|-------------|
                     |---|
Notes:
      Chips:
         QS1000: QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
          PRIME: LGS Prime 3C 9849R, clock input of 14.31818MHz (QFP100)
   XILINX XCS10: Xilinx Spartan XCS10 FPGA (QFP144)
      CL-PS7500: Cirrus Logic CL-PS7500FE-56QC-A 84877-951BD ARM 9843J
                 clock input of 54.000MHz, ARM710C; ARM7-core CPU (QFP240)
     KM416C1204: Samsung KM416C1204CJ-5 2M x8 DRAM (SOJ42)
           7705: Reset/Watchdog IC (SOIC8)
           7660: DC-DC Voltage Convertor (SOIC8)
         EL2386: Elantec Semiconductor 250MHz Triple Current Feedback Op Amp with Disable (SOIC16)
              *: Unpopulated DIP32 sockets
              &: Unpopulated location for QFP100 IC
              #: Unpopulated location for SOJ42 RAM
              @: Unpopulated location for OSC1

      ROMs:
          U24, U25: AMD 29F040B 512k x8 FlashROM (DIP32)
          NASN9289: Re-badged SOP32 ROM. Should be compatible with existing QS100x Wavetable Audio Sample ROMs,
                    Dumped as 1M x8 SOP32 MaskROM
         DU2, DU3,: Samsung KM29N32000TS 32MBit NAND Flash 3.3V Serial EEPROM (TSOP44)
         DU5, DU6   These ROMs are mounted on a small plug-in daughterboard. There are additional
                    mounting pads for 4 more of these ROMs but they're not populated.

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

enum
{
	IOCR=0,
	KBDDAT,
	KBDCR,
	IOLINES,
	IRQSTA,
	IRQRQA,
	IRQMSKA,
	SUSMODE,
	IRQSTB,
	IRQRQB,
	IRQMSKB,
	STOPMODE,
	FIQST,
	FIQRQ,
	FIQMSK,
	CLKCTL,
	T0low,
	T0high,
	T0GO,
	T0LAT,
	T1low,
	T1high,
	T1GO,
	T1LAT,
	IRQSTC,
	IRQRQC,
	IRQMSKC,
	VIDMUX,
	IRQSTD,
	IRQRQD,
	IRQMSKD,

	ROMCR0=32,
	ROMCR1,

	REFCR=35,

	ID0=37,
	ID1,
	VERSION,

	MSEDAT=42,
	MSECR,

	IOTCR=49,
	ECTCR,
	ASTCR,
	DRAMCR,
	SELREF,

	ATODICR=56,
	ATODSR,
	ATODCC,
	ATODCNT1,
	ATODCNT2,
	ATODCNT3,
	ATODCNT4,

	SD0CURA=96,
	SD0ENDA,
	SD0CURB,
	SD0ENDB,
	SD0CR,
	SD0ST,

	CURSCUR=112,
	CURSINIT,
	VIDCURB,

	VIDCURA=116,
	VIDEND,
	VIDSTART,
	VIDINITA,
	VIDCR,

	VIDINITB=122,

	DMAST=124,
	DMARQ,
	DMASK,
	MAXIO=128
};


class ssfindo_state : public driver_device
{
public:
	ssfindo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 m_PS7500_IO[MAXIO];
	UINT32 m_PS7500_FIFO[256];
	UINT32 *m_vram;
	UINT32 m_flashAdr;
	UINT32 m_flashOffset;
	UINT32 m_adrLatch;
	UINT32 m_flashType;
	UINT32 m_flashN;
	emu_timer *m_PS7500timer0;
	emu_timer *m_PS7500timer1;
	int m_iocr_hack;
};


static void PS7500_startTimer0(running_machine &machine);
static void PS7500_startTimer1(running_machine &machine);


static SCREEN_UPDATE(ssfindo)
{
	ssfindo_state *state = screen->machine().driver_data<ssfindo_state>();
	int s,x,y;

	if( state->m_PS7500_IO[VIDCR]&0x20) //video DMA enabled
	{
		s=( (state->m_PS7500_IO[VIDINITA]&0x1fffffff)-0x10000000)/4;

		if(s>=0 && s<(0x10000000/4))
		{
			for(y=0;y<256;y++)
				for(x=0;x<320;x+=4)
				{
					*BITMAP_ADDR16(bitmap, y, x+0) = state->m_vram[s]&0xff;
					*BITMAP_ADDR16(bitmap, y, x+1) = (state->m_vram[s]>>8)&0xff;
					*BITMAP_ADDR16(bitmap, y, x+2) = (state->m_vram[s]>>16)&0xff;
					*BITMAP_ADDR16(bitmap, y, x+3) = (state->m_vram[s]>>24)&0xff;
					s++;
				}
		}
	}

	return 0;
}

static WRITE32_HANDLER(FIFO_w)
{
	ssfindo_state *state = space->machine().driver_data<ssfindo_state>();
	state->m_PS7500_FIFO[data>>28]=data;

	if(!(data>>28))
	{
		palette_set_color_rgb(space->machine(), state->m_PS7500_FIFO[1]&0xff, data&0xff,(data>>8)&0xff,(data>>16)&0xff);
		state->m_PS7500_FIFO[1]++; //autoinc
	}
}
static TIMER_CALLBACK( PS7500_Timer0_callback )
{
	ssfindo_state *state = machine.driver_data<ssfindo_state>();
	state->m_PS7500_IO[IRQSTA]|=0x20;
	if(state->m_PS7500_IO[IRQMSKA]&0x20)
	{
		generic_pulse_irq_line(machine.device("maincpu"), ARM7_IRQ_LINE);
	}
}

static void PS7500_startTimer0(running_machine &machine)
{
	ssfindo_state *state = machine.driver_data<ssfindo_state>();
	int val=((state->m_PS7500_IO[T0low]&0xff)|((state->m_PS7500_IO[T0high]&0xff)<<8))>>1;

	if(val==0)
		state->m_PS7500timer0->adjust(attotime::never);
	else
		state->m_PS7500timer0->adjust(attotime::from_usec(val ), 0, attotime::from_usec(val ));
}

static TIMER_CALLBACK( PS7500_Timer1_callback )
{
	ssfindo_state *state = machine.driver_data<ssfindo_state>();
	state->m_PS7500_IO[IRQSTA]|=0x40;
	if(state->m_PS7500_IO[IRQMSKA]&0x40)
	{
		generic_pulse_irq_line(machine.device("maincpu"), ARM7_IRQ_LINE);
	}
}

static void PS7500_startTimer1(running_machine &machine)
{
	ssfindo_state *state = machine.driver_data<ssfindo_state>();
	int val=((state->m_PS7500_IO[T1low]&0xff)|((state->m_PS7500_IO[T1high]&0xff)<<8))>>1;
	if(val==0)
		state->m_PS7500timer1->adjust(attotime::never);
	else
		state->m_PS7500timer1->adjust(attotime::from_usec(val ), 0, attotime::from_usec(val ));
}

static INTERRUPT_GEN( ssfindo_interrupt )
{
	ssfindo_state *state = device->machine().driver_data<ssfindo_state>();
	state->m_PS7500_IO[IRQSTA]|=0x08;
		if(state->m_PS7500_IO[IRQMSKA]&0x08)
		{
			generic_pulse_irq_line(device, ARM7_IRQ_LINE);
		}
}

static void PS7500_reset(running_machine &machine)
{
	ssfindo_state *state = machine.driver_data<ssfindo_state>();
		state->m_PS7500_IO[IOCR]			=	0x3f;
		state->m_PS7500_IO[VIDCR]		=	0;

		state->m_PS7500timer0->adjust( attotime::never);
		state->m_PS7500timer1->adjust( attotime::never);
}

typedef void (*ssfindo_speedup_func)(address_space *space);
ssfindo_speedup_func ssfindo_speedup;

static void ssfindo_speedups(address_space* space)
{
	if (cpu_get_pc(&space->device())==0x2d6c8) // ssfindo
		device_spin_until_time(&space->device(), attotime::from_usec(20));
	else if (cpu_get_pc(&space->device())==0x2d6bc) // ssfindo
		device_spin_until_time(&space->device(), attotime::from_usec(20));
}

static void ppcar_speedups(address_space* space)
{
	if (cpu_get_pc(&space->device())==0x000bc8) // ppcar
		device_spin_until_time(&space->device(), attotime::from_usec(20));
	else if (cpu_get_pc(&space->device())==0x000bbc) // ppcar
		device_spin_until_time(&space->device(), attotime::from_usec(20));
}


static READ32_HANDLER(PS7500_IO_r)
{
	ssfindo_state *state = space->machine().driver_data<ssfindo_state>();
	switch(offset)
	{
		case MSECR:
			return space->machine().rand();

		case IOLINES: //TODO: eeprom  24c01
#if 0
		mame_printf_debug("IOLINESR %i @%x\n", offset, cpu_get_pc(&space->device()));
#endif

		if(state->m_flashType == 1)
			return 0;
		else
			return space->machine().rand();

		case IRQSTA:
			return (state->m_PS7500_IO[offset] & (~2)) | 0x80;

		case IRQRQA:
			return (state->m_PS7500_IO[IRQSTA] & state->m_PS7500_IO[IRQMSKA]) | 0x80;

		case IOCR: //TODO: nINT1, OD[n] p.81
			if (ssfindo_speedup) ssfindo_speedup(space);

			if( state->m_iocr_hack)
			{
				return (input_port_read(space->machine(), "PS7500") & 0x80) | 0x34 | (space->machine().rand()&3); //eeprom read ?
			}

			return (input_port_read(space->machine(), "PS7500") & 0x80) | 0x37;

		case VIDCR:
			return (state->m_PS7500_IO[offset] | 0x50) & 0xfffffff0;

		case T1low:
		case T0low:
		case T1high:
		case T0high:
		case IRQMSKA:
		case VIDEND:
		case VIDSTART:
		case VIDINITA: //TODO: bits 29 ("equal") and 30 (last bit)  p.105

			return state->m_PS7500_IO[offset];


	}
	return space->machine().rand();//state->m_PS7500_IO[offset];
}

static WRITE32_HANDLER(PS7500_IO_w)
{
	ssfindo_state *state = space->machine().driver_data<ssfindo_state>();
	UINT32 temp=state->m_PS7500_IO[offset];

	COMBINE_DATA(&temp);

	switch(offset)
	{
		case IOLINES: //TODO: eeprom  24c01
			state->m_PS7500_IO[offset]=data;
				if(data&0xc0)
					state->m_adrLatch=0;

			if(cpu_get_pc(&space->device()) == 0xbac0 && state->m_flashType == 1)
			{
				state->m_flashN=data&1;
			}

#if 0
				logerror("IOLINESW %i = %x  @%x\n",offset,data,cpu_get_pc(&space->device()));
#endif
			break;

		case IRQRQA:
			state->m_PS7500_IO[IRQSTA]&=~temp;
		break;

		case IRQMSKA:
			state->m_PS7500_IO[IRQMSKA]=(temp&(~2))|0x80;
		break;

		case T1GO:
				PS7500_startTimer1(space->machine());
			break;

		case T0GO:
			PS7500_startTimer0(space->machine());
		break;

		case VIDEND:
		case VIDSTART:
			COMBINE_DATA(&state->m_PS7500_IO[offset]);
			state->m_PS7500_IO[offset]&=0xfffffff0; // qword align
		break;

		case IOCR:
		case REFCR:
		case DRAMCR:
		case SD0CR:
		case ROMCR0:
		case VIDMUX:
		case CLKCTL:
		case T1low:
		case T0low:
		case T1high:
		case T0high:
		case VIDCR:
		case VIDINITA: //TODO: bit 30 (last bit) p.105
					COMBINE_DATA(&state->m_PS7500_IO[offset]);
		break;



	}
}

static READ32_HANDLER(io_r)
{
	ssfindo_state *state = space->machine().driver_data<ssfindo_state>();
	UINT16 *FLASH = (UINT16 *)space->machine().region("user2")->base(); //16 bit - WORD access

	int adr=state->m_flashAdr*0x200+(state->m_flashOffset);


	switch(state->m_flashType)
	{
		case 0:
			if(state->m_PS7500_IO[IOLINES]&1) //bit 0 of IOLINES  = flash select ( 5/6 or 3/2 )
				adr+=0x400000;
		break;

		case 1:
			adr+=0x400000*state->m_flashN;
		break;
	}

	if(adr<0x400000*2)
	{
		state->m_flashOffset++;
		return FLASH[adr];
	}
	return 0;
}

static WRITE32_HANDLER(io_w)
{
	ssfindo_state *state = space->machine().driver_data<ssfindo_state>();
	UINT32 temp = 0;
	COMBINE_DATA(&temp);

#if 0
	logerror("[io_w] = %x @%x [latch=%x]\n",data,cpu_get_pc(&space->device()),state->m_adrLatch);
#endif

	if(state->m_adrLatch==1)
		state->m_flashAdr=(temp>>16)&0xff;
	if(state->m_adrLatch==2)
	{
		state->m_flashAdr|=(temp>>16)&0xff00;
		state->m_flashOffset=0;
	}
	state->m_adrLatch=(state->m_adrLatch+1)%3;
}

static WRITE32_HANDLER(debug_w)
{
#if 0
	mame_printf_debug("%c",data&0xff); //debug texts - malloc (ie "64 KBytes allocated, elapsed : 378 KBytes, free : 2231 KBytes")
#endif
}

static READ32_HANDLER(ff4_r)
{
	return space->machine().rand()&0x20;
}

static READ32_HANDLER(SIMPLEIO_r)
{
	return space->machine().rand()&1;
}

static READ32_HANDLER(randomized_r)
{
	return space->machine().rand();
}

static ADDRESS_MAP_START( ssfindo_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x03200000, 0x032001ff) AM_READWRITE(PS7500_IO_r,PS7500_IO_w)
	AM_RANGE(0x03012e60, 0x03012e67) AM_NOP
	AM_RANGE(0x03012fe0, 0x03012fe3) AM_WRITE(debug_w)
	AM_RANGE(0x03012ff0, 0x03012ff3) AM_NOP
	AM_RANGE(0x03012ff4, 0x03012ff7) AM_WRITENOP AM_READ(ff4_r) //status flag ?
	AM_RANGE(0x03012ff8, 0x03012fff) AM_NOP
	AM_RANGE(0x03240000, 0x03240003) AM_READ_PORT("IN0") AM_WRITENOP
	AM_RANGE(0x03241000, 0x03241003) AM_READ_PORT("IN1") AM_WRITENOP
	AM_RANGE(0x03242000, 0x03242003) AM_READ(io_r) AM_WRITE(io_w)
	AM_RANGE(0x03243000, 0x03243003) AM_READ_PORT("DSW") AM_WRITENOP
	AM_RANGE(0x0324f000, 0x0324f003) AM_READ(SIMPLEIO_r)
	AM_RANGE(0x03245000, 0x03245003) AM_WRITENOP /* sound ? */
	AM_RANGE(0x03400000, 0x03400003) AM_WRITE(FIFO_w)
	AM_RANGE(0x10000000, 0x11ffffff) AM_RAM AM_BASE_MEMBER(ssfindo_state, m_vram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ppcar_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x03200000, 0x032001ff) AM_READWRITE(PS7500_IO_r,PS7500_IO_w)
	AM_RANGE(0x03012b00, 0x03012bff) AM_READ(randomized_r) AM_WRITENOP
	AM_RANGE(0x03012e60, 0x03012e67) AM_WRITENOP
	AM_RANGE(0x03012ff8, 0x03012ffb) AM_READ_PORT("IN0") AM_WRITENOP
	AM_RANGE(0x032c0000, 0x032c0003) AM_READ_PORT("IN1") AM_WRITENOP
	AM_RANGE(0x03340000, 0x03340007) AM_WRITENOP
	AM_RANGE(0x03341000, 0x0334101f) AM_WRITENOP
	AM_RANGE(0x033c0000, 0x033c0003) AM_READ(io_r) AM_WRITE(io_w)
	AM_RANGE(0x03400000, 0x03400003) AM_WRITE(FIFO_w)
	AM_RANGE(0x08000000, 0x08ffffff) AM_RAM
	AM_RANGE(0x10000000, 0x10ffffff) AM_RAM AM_BASE_MEMBER(ssfindo_state, m_vram)
ADDRESS_MAP_END

static READ32_HANDLER(tetfight_unk_r)
{
	//sound status ?
	return space->machine().rand();
}

static WRITE32_HANDLER(tetfight_unk_w)
{
	//sound latch ?
}

static ADDRESS_MAP_START( tetfight_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x03200000, 0x032001ff) AM_READWRITE(PS7500_IO_r,PS7500_IO_w)
	AM_RANGE(0x03400000, 0x03400003) AM_WRITE(FIFO_w)
	AM_RANGE(0x03240000, 0x03240003) AM_READ_PORT("DSW")
	AM_RANGE(0x03240004, 0x03240007) AM_READ_PORT("IN0")
	AM_RANGE(0x03240008, 0x0324000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x03240020, 0x03240023) AM_READWRITE( tetfight_unk_r, tetfight_unk_w)
	AM_RANGE(0x10000000, 0x14ffffff) AM_RAM AM_BASE_MEMBER(ssfindo_state, m_vram)
ADDRESS_MAP_END

static MACHINE_RESET( ssfindo )
{
	PS7500_reset(machine);
}

static INPUT_PORTS_START( ssfindo )
	PORT_START("PS7500")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("IN0")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED	)				// IPT_START2 ??
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1	) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START1	)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1	)

	PORT_START("IN1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1	) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2	)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW 2" )
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW 3" )
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x010, "DSW 4" )
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW 5" )
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x040, "DSW 6" )
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ppcar )
	PORT_START("PS7500")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1	)

	PORT_START("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2	) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1	) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1	)
INPUT_PORTS_END

static INPUT_PORTS_START( tetfight )
	PORT_START("PS7500")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "DSW 0" )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW 1" )
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW 2" )
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW 3" )
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x010, "DSW 4" )
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW 5" )
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x040, "DSW 6" )
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW 7" )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW 1" )
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW 2" )
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Initialize" )
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x010, "DSW 4" )
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW 5" )
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x040, "DSW 6" )
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW 7" )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1	) PORT_PLAYER(1) //guess

	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1	) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2	) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3	) PORT_PLAYER(1)
INPUT_PORTS_END


static MACHINE_CONFIG_START( ssfindo, ssfindo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM7, 54000000) // guess...
	MCFG_CPU_PROGRAM_MAP(ssfindo_map)

	MCFG_CPU_VBLANK_INT("screen", ssfindo_interrupt)
	MCFG_MACHINE_RESET(ssfindo)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(ssfindo)

	MCFG_PALETTE_LENGTH(256)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ppcar, ssfindo )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ppcar_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tetfight, ssfindo )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(tetfight_map)
MACHINE_CONFIG_END

ROM_START( ssfindo )
	ROM_REGION(0x100000, "user1", 0 ) /* ARM 32 bit code */
	ROM_LOAD16_BYTE( "a.u28",	0x000000, 0x80000, CRC(c93edbd3) SHA1(9c703cfef49b59ccd5d68bab9bd59344bd18d67e) )
	ROM_LOAD16_BYTE( "b.u29",	0x000001, 0x80000, CRC(39ecb9e4) SHA1(9ebd3962d8014b97c68c364729248ed22f9298a4) )

	ROM_REGION(0x1000000, "user2", 0 ) /* flash roms */
	ROM_LOAD16_BYTE( "du5",		0x000000, 0x400000, CRC(b32bd453) SHA1(6d5694bfcc67102256f857932b83b38f62ca2010) )
	ROM_LOAD16_BYTE( "du6",		0x000001, 0x400000, CRC(00559591) SHA1(543aefddc02f6a521d3bd5e6e3d8e42127ff9baa) )

	ROM_LOAD16_BYTE( "du3",		0x800000, 0x400000, CRC(d1e8afb2) SHA1(598dfcbba14435a1d0571dcefe0ec62fec657fca) )
	ROM_LOAD16_BYTE( "du2",		0x800001, 0x400000, CRC(56998515) SHA1(9b71a44f56a545ff0c1170775c839d21bd01f545) )

	ROM_REGION(0x80, "user3", 0 ) /* eeprom */
	ROM_LOAD( "24c01a.u36",		0x00, 0x80, CRC(b4f4849b) SHA1(f8f17dc94b2a305048693cfb78d14be57310ce56) )

	ROM_REGION(0x10000, "user4", 0 ) /* qdsp code */
	ROM_LOAD( "e.u14",		0x000000, 0x10000, CRC(49976f7b) SHA1(eba5b97b81736f3c184ae0c19f1b10c5ae250d51) )

	ROM_REGION(0x100000, "user5", 0 ) /* HWASS 1008S-1  qdsp samples */
	ROM_LOAD( "1008s-1.u16",	0x000000, 0x100000, CRC(9aef9545) SHA1(f23ef72c3e3667923768dfdd0c5b4951b23dcbcf) )

	ROM_REGION(0x100000, "user6", 0 ) /* samples - same internal structure as qdsp samples  */
	ROM_LOAD( "c.u12",		0x000000, 0x80000, CRC(d24b5e56) SHA1(d89983cf4b0a6e0e4137f3799bdbcfd72c7bebe4) )
	ROM_LOAD( "d.u11",		0x080000, 0x80000, CRC(c0fdd82a) SHA1(a633045e0f5c144b4e24e04fb9446522fdb222f4) )
ROM_END

ROM_START( ppcar )
	ROM_REGION(0x100000, "user1", 0 ) /* ARM 32 bit code */
	ROM_LOAD16_BYTE( "fk0.u24",	0x000000, 0x80000, CRC(1940a483) SHA1(9456361fd25bf037b53bd2d04764a33b299d96dd) )
	ROM_LOAD16_BYTE( "fk1.u25",	0x000001, 0x80000, CRC(75ad8679) SHA1(392288e56350e3cc49aaca82edf26f2a9e346f21) )

	ROM_REGION(0x1000000, "user2", 0 ) /* flash roms */
	ROM_LOAD16_BYTE( "du5",		0x000000, 0x400000, CRC(d4b7374a) SHA1(54c93a4235f495ba3794aea511b19db821a8acb1) )
	ROM_LOAD16_BYTE( "du6",		0x000001, 0x400000, CRC(e95a3a62) SHA1(2b1c889d208a749e3d7e4c75588c9c1f979e88d9) )

	ROM_LOAD16_BYTE( "du3",		0x800000, 0x400000, CRC(73882474) SHA1(191b64e662542b5322160c99af8e00079420d473) )
	ROM_LOAD16_BYTE( "du2",		0x800001, 0x400000, CRC(9250124a) SHA1(650f4b89c92fe4fb63fc89d4e08c4c4c611bebbc) )

	ROM_REGION(0x10000, "user4", ROMREGION_ERASE00 ) /* qdsp code */
	/* none */

	ROM_REGION(0x100000, "user5", 0 ) /* HWASS 1008S-1  qdsp samples */
	ROM_LOAD( "nasn9289.u9",	0x000000, 0x100000, CRC(9aef9545) SHA1(f23ef72c3e3667923768dfdd0c5b4951b23dcbcf) )

	ROM_REGION(0x100000, "user6", ROMREGION_ERASE00 ) /* samples - same internal structure as qdsp samples  */
	/* none */
ROM_END

ROM_START( tetfight )
	ROM_REGION(0x200000, "user1", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "u42",		0x000000, 0x200000, CRC(9101c4d2) SHA1(39da953de734e687ebbf976c821bf1017830f36c) )

	ROM_REGION(0x1000000, "user2", ROMREGION_ERASEFF ) /* flash roms */
	/* nothing? */

	ROM_REGION(0x100, "user3", 0 ) /* eeprom */
	ROM_LOAD( "u1",		0x00, 0x100, CRC(dd207b40) SHA1(6689d9dfa980bdfbd4e4e6cef7973e22ebbfe22e) )

	ROM_REGION(0x10000, "user4", 0 ) /* qdsp code */
	ROM_LOAD( "u12",		0x000000, 0x10000, CRC(49976f7b) SHA1(eba5b97b81736f3c184ae0c19f1b10c5ae250d51) ) // = e.u14 on ssfindo

	ROM_REGION(0x100000, "user5", ROMREGION_ERASE00 )/*  qdsp samples */
	// probably the same, but wasn't dumped
	//ROM_LOAD( "1008s-1.u16",  0x000000, 0x100000, CRC(9aef9545) SHA1(f23ef72c3e3667923768dfdd0c5b4951b23dcbcf) )

	ROM_REGION(0x100000, "user6", 0 ) /* samples - same internal structure as qdsp samples  */
	ROM_LOAD( "u11",		0x000000, 0x80000, CRC(073050f6) SHA1(07f362f3ba468bde2341a99e6b26931d11459a92) )
	ROM_LOAD( "u15",		0x080000, 0x80000, CRC(477f8089) SHA1(8084facb254d60da7983d628d5945d27b9494e65) )
ROM_END

static DRIVER_INIT(common)
{
	ssfindo_state *state = machine.driver_data<ssfindo_state>();
	ssfindo_speedup = 0;
	state->m_PS7500timer0 = machine.scheduler().timer_alloc(FUNC(PS7500_Timer0_callback));
	state->m_PS7500timer1 = machine.scheduler().timer_alloc(FUNC(PS7500_Timer1_callback));

}

static DRIVER_INIT(ssfindo)
{
	ssfindo_state *state = machine.driver_data<ssfindo_state>();
	DRIVER_INIT_CALL(common);
	state->m_flashType=0;
	ssfindo_speedup = ssfindo_speedups;
	state->m_iocr_hack=0;
}

static DRIVER_INIT(ppcar)
{
	ssfindo_state *state = machine.driver_data<ssfindo_state>();
	DRIVER_INIT_CALL(common);
	state->m_flashType=1;
	ssfindo_speedup = ppcar_speedups;
	state->m_iocr_hack=0;
}

static DRIVER_INIT(tetfight)
{
	ssfindo_state *state = machine.driver_data<ssfindo_state>();
	DRIVER_INIT_CALL(common);
	state->m_flashType=0;
	state->m_iocr_hack=1;
}

GAME( 1999, ssfindo, 0,        ssfindo,  ssfindo,  ssfindo,	ROT0, "Icarus", "See See Find Out", GAME_NO_SOUND )
GAME( 1999, ppcar,   0,        ppcar,    ppcar,    ppcar,	ROT0, "Icarus", "Pang Pang Car", GAME_NO_SOUND )
GAME( 2001, tetfight,0,        tetfight, tetfight,  tetfight,ROT0, "Sego", "Tetris Fighters", GAME_NO_SOUND|GAME_NOT_WORKING )
