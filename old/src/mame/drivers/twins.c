/*

Twins
Electronic Devices, 1994

PCB Layout
----------

This is a very tiny PCB,
only about 6 inches square.

|-----------------------|
|     6116   16MHz 62256|
|TEST 6116 24C02        |
|             PAL  62256|
|J   62256 |--------|   |
|A         |TPC1020 | 2 |
|M   62256 |AFN-084C|   |
|M         |        | 1 |
|A         |--------|   |
| AY3-8910              |
|                 D70116|
|-----------------------|
Notes:
    V30 clock      : 8.000MHz (16/2)
    AY3-8910 clock : 2.000MHz (16/8)
    VSync          : 50Hz



seems a similar board to hotblocks

same TPC1020 AFN-084C chip
same 24c02 eeprom
V30 instead of I8088
AY3-8910 instead of YM2149 (compatible)

video is not banked in this case instead palette data is sent to the ports
strange palette format.

todo:
hook up eeprom
takes a long time to boot (eeprom?)


Electronic Devices was printed on rom labels
1994 date string is in ROM

*/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "sound/ay8910.h"


class twins_state : public driver_device
{
public:
	twins_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	UINT16 *pal;
	UINT16 paloff;
};



/* port 4 is eeprom */
static READ16_HANDLER( twins_port4_r )
{
	return 0xffff;
}

static WRITE16_HANDLER( twins_port4_w )
{
}

static WRITE16_HANDLER( port6_pal0_w )
{
	twins_state *state = space->machine->driver_data<twins_state>();
	COMBINE_DATA(&state->pal[state->paloff]);
	state->paloff = (state->paloff + 1) & 0xff;
}

/* ??? weird ..*/
static WRITE16_HANDLER( porte_paloff0_w )
{
	twins_state *state = space->machine->driver_data<twins_state>();
	state->paloff = 0;
}

static ADDRESS_MAP_START( twins_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM
	AM_RANGE(0x10000, 0x1ffff) AM_RAM AM_BASE_MEMBER(twins_state, videoram)
	AM_RANGE(0x20000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( twins_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x0000, 0x0003) AM_DEVWRITE8("aysnd", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0x0002, 0x0003) AM_DEVREAD8("aysnd", ay8910_r, 0x00ff)
	AM_RANGE(0x0004, 0x0005) AM_READWRITE(twins_port4_r, twins_port4_w)
	AM_RANGE(0x0006, 0x0007) AM_WRITE(port6_pal0_w)
	AM_RANGE(0x000e, 0x000f) AM_WRITE(porte_paloff0_w)
ADDRESS_MAP_END

static VIDEO_START(twins)
{
	twins_state *state = machine->driver_data<twins_state>();
	state->pal = auto_alloc_array(machine, UINT16, 0x100);
}

static SCREEN_UPDATE(twins)
{
	twins_state *state = screen->machine->driver_data<twins_state>();
	int y,x,count;
	int i;
	static const int xxx=320,yyy=204;

	bitmap_fill(bitmap, 0, get_black_pen(screen->machine));

	for (i=0;i<0x100;i++)
	{
		int dat,r,g,b;
		dat = state->pal[i];

		r = dat & 0x1f;
		r = BITSWAP8(r,7,6,5,0,1,2,3,4);

		g = (dat>>5) & 0x1f;
		g = BITSWAP8(g,7,6,5,0,1,2,3,4);

		b = (dat>>10) & 0x1f;
		b = BITSWAP8(b,7,6,5,0,1,2,3,4);

		palette_set_color_rgb(screen->machine,i, pal5bit(r),pal5bit(g),pal5bit(b));
	}

	count=0;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			*BITMAP_ADDR16(bitmap, y, x) = ((UINT8 *)state->videoram)[BYTE_XOR_LE(count)];
			count++;
		}
	}
	return 0;
}


static INPUT_PORTS_START(twins)
	PORT_START("P1")	/* 8bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("P2")	/* 8bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("P1"),
	DEVCB_INPUT_PORT("P2")
};

static MACHINE_CONFIG_START( twins, twins_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, 8000000)
	MCFG_CPU_PROGRAM_MAP(twins_map)
	MCFG_CPU_IO_MAP(twins_io)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(320,256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE(twins)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_START(twins)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 2000000)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/* The second set has different palette hardware and a different port map */


static VIDEO_START(twinsa)
{
	twins_state *state = machine->driver_data<twins_state>();
	state->pal = auto_alloc_array(machine, UINT16, 0x1000);
}

static SCREEN_UPDATE(twinsa)
{
	twins_state *state = screen->machine->driver_data<twins_state>();
	int y,x,count;
	int i;
	static const int xxx=320,yyy=204;

	bitmap_fill(bitmap, 0, get_black_pen(screen->machine));

	for (i=0;i<0x1000-3;i+=3)
	{
		int r,g,b;
		r = state->pal[i];
		g = state->pal[i+1];
		b = state->pal[i+2];

		palette_set_color_rgb(screen->machine,i/3, pal6bit(r), pal6bit(g), pal6bit(b));
	}

	count=0;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			*BITMAP_ADDR16(bitmap, y, x) = ((UINT8 *)state->videoram)[BYTE_XOR_LE(count)];
			count++;
		}
	}
	return 0;
}

static WRITE16_HANDLER( twinsa_port4_w )
{
	twins_state *state = space->machine->driver_data<twins_state>();
	state->pal[state->paloff&0xfff] = data;
	state->paloff++;
//  printf("paloff %04x\n",state->paloff);
}

static READ16_HANDLER( twinsa_unk_r )
{
	return 0xffff;
}

static ADDRESS_MAP_START( twinsa_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x0000, 0x0001) AM_READWRITE(twinsa_unk_r, porte_paloff0_w)
	AM_RANGE(0x0002, 0x0003) AM_WRITE(porte_paloff0_w)
	AM_RANGE(0x0004, 0x0005) AM_WRITE(twinsa_port4_w) // palette on this set
	AM_RANGE(0x0008, 0x0009) AM_DEVWRITE8("aysnd", ay8910_address_w, 0x00ff)
	AM_RANGE(0x0010, 0x0011) AM_DEVREADWRITE8("aysnd", ay8910_r, ay8910_data_w, 0x00ff)
	AM_RANGE(0x0018, 0x0019) AM_READ(twins_port4_r) AM_WRITE(twins_port4_w)
ADDRESS_MAP_END


static MACHINE_CONFIG_START( twinsa, twins_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, XTAL_16MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(twins_map)
	MCFG_CPU_IO_MAP(twinsa_io)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(320,256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE(twinsa)

	MCFG_PALETTE_LENGTH(0x1000)

	MCFG_VIDEO_START(twinsa)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_16MHz/8) /* verified on pcb */
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( twins )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.bin", 0x000000, 0x080000, CRC(d5ef7b0d) SHA1(7261dca5bb0aef755b4f2b85a159b356e7ac8219) )
	ROM_LOAD16_BYTE( "2.bin", 0x000001, 0x080000, CRC(8a5392f4) SHA1(e6a2ecdb775138a87d27aa4ad267bdec33c26baa) )
ROM_END

/*
Shang Hay Twins
Electronic Devices

1x nec9328n8-v30-d70116c-8 (main)
2x ay-3-8910a (sound)
1x blank (z80?)
1x oscillator 8.000

2x M27c4001

1x jamma edge connector
1x trimmer (volume)

hmm, we're only emulating 1x ay-3-8910, is the other at port 0 on this?

*/

ROM_START( twinsa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lp.bin", 0x000000, 0x080000, CRC(4f07862e) SHA1(fbda1973f79c6938c7f026a4db706e78781c2df8) )
	ROM_LOAD16_BYTE( "hp.bin", 0x000001, 0x080000, CRC(aaf74b83) SHA1(09bd76b9fc5cb7ba6ffe1a2581ffd5633fe440b3) )
ROM_END

GAME( 1994, twins,  0,     twins,  twins, 0, ROT0, "Electronic Devices", "Twins (set 1)", 0 )
GAME( 1994, twinsa, twins, twinsa, twins, 0, ROT0, "Electronic Devices", "Twins (set 2)", 0 )
