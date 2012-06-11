/***************************************************************************

  Nintendo FamicomBox
  Driver by Mariusz Wojcieszek
  Thanks for Kevin Horton for hardware docs

In 1986, Nintendo Co., Ltd. began distributing the FamicomBox (SSS-CDS) which
allowed gamers to test out and play up to 15 different Famicom games which
could be installed in the unit itself.  Like it's sequel, the Super Famicom Box,
it was found in hotels often set up to accept 100 yen coins giving you an
adjusted amount of gametime (10 or 20 minutes - DIP selectable).  Sharp also
produced a version of the FamicomBox called FamicomStation which was more
of a consumer (non-coin accept) unit.  Besides that, the equipment and
capabilities between the two are thought to be virtually identical - save the
case style and cartridge/case color:  (FamicomBox = Black, FamicomStation = Gray).



Specific Hardware information
-----------------------------
The unit had 3 controllers - 2 standard NES controllers and a NES Zapper light
gun.  The cartridges are shaped and appear to be idential to NES 72-pin
cartridges.  Unfortunately, it was made to play only the games specifically
released for it.  Why?

- The FamicomBox will not run mmc3 games and many other advanced mappers
- There a special lockout chip, but the lockout chip connects to different pins on
  a FamicomBox cartridge's connector than a regular cart
- The lockout chips in the system and the games have to 'talk' before the system will
  load any games into its menu.

Here's a list of some of the games known to have come with the FamicomBox:
1943; Baseball; Bomber Man; Devil World; Donkey Kong; Donkey Kong Jr.; Duck Hunt;
Excite Bike; F1 Race; Fighting Golf; Golf; Gradius; Hogan?s Alley; Ice Climbers;
Ice Hockey; Knight Rider; Makaimura: Ghosts ?n Goblins; McKids; Mah-Jong; Mario Bros.;
Mike Tyson?s Punch-Out!!; Ninja Ryukenden; Operation Wolf (?); Punch-Out!!; Rock Man;
Rygar; Senjou no Ookami; Soccer League Winner?s Cup; Super Chinese 2; Super Mario Bros;
Tag Team Pro Wrestling; Takahashi Meijin no Boukenjima; Tennis; Twin Bee;
Volleyball; Wild Gunman; Wrecking Crew.

Here's a list of some of the games known to have come with the FamicomStation:
1943; Baseball; Donkey Kong; Duck Hunt; F1 Race; Golf; Kame no Ongaeshi:
Urashima Densetsu; Mah-Jong; Mario Bros.; Night Raider; Senjou no Ookami;
Soccer League Winner?s Cup; Super Chinese 2; Super Mario Bros; Tag Team Pro Wrestling;
Takahashi Meijin no Boukenjima; Tennis; Wild Gunman; Wrecking Crew.

FamicomBox menu code maintains internal database of games (rom checksums and game names
in ASCII). When checking game cartidges, it scans roms and tries to find matching game
in its internal database. Additionaly, games having standard Nintendo header are accepted too.
Current selection of games in driver is based on menu internal database.

Notes/ToDo:
- coin insertion sound is not emulated
- coin beep (before time out) is not emulated
- screen modulation (before time out) is not emulated
- zapper is not emulated
- self test fails on controller and zapper test
***************************************************************************/

#include "emu.h"
#include "video/ppu2c0x.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cartslot.h"
#include "sound/nes_apu.h"
#include "sound/dac.h"
#include "debugger.h"


class famibox_state : public driver_device
{
public:
	famibox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8* m_nt_ram;
	UINT8* m_nt_page[4];

	UINT32 m_in_0;
	UINT32 m_in_1;
	UINT32 m_in_0_shift;
	UINT32 m_in_1_shift;

	UINT8		m_exception_mask;
	UINT8		m_exception_cause;

	emu_timer*	m_attract_timer;
	UINT8		m_attract_timer_period;

	UINT32		m_coins;

	emu_timer*	m_gameplay_timer;
	UINT8		m_money_reg;

};

/******************************************************

   PPU external bus interface

*******************************************************/

#if 0
static void set_mirroring(famibox_state *state, int mirroring)
{
	switch(mirroring)
	{
	case PPU_MIRROR_LOW:
		state->m_nt_page[0] = state->m_nt_page[1] = state->m_nt_page[2] = state->m_nt_page[3] = state->m_nt_ram;
		break;
	case PPU_MIRROR_HIGH:
		state->m_nt_page[0] = state->m_nt_page[1] = state->m_nt_page[2] = state->m_nt_page[3] = state->m_nt_ram + 0x400;
		break;
	case PPU_MIRROR_HORZ:
		state->m_nt_page[0] = state->m_nt_ram;
		state->m_nt_page[1] = state->m_nt_ram;
		state->m_nt_page[2] = state->m_nt_ram + 0x400;
		state->m_nt_page[3] = state->m_nt_ram + 0x400;
		break;
	case PPU_MIRROR_VERT:
		state->m_nt_page[0] = state->m_nt_ram;
		state->m_nt_page[1] = state->m_nt_ram + 0x400;
		state->m_nt_page[2] = state->m_nt_ram;
		state->m_nt_page[3] = state->m_nt_ram + 0x400;
		break;
	case PPU_MIRROR_NONE:
	default:
		state->m_nt_page[0] = state->m_nt_ram;
		state->m_nt_page[1] = state->m_nt_ram + 0x400;
		state->m_nt_page[2] = state->m_nt_ram + 0x800;
		state->m_nt_page[3] = state->m_nt_ram + 0xc00;
		break;
	}
}
#endif

static WRITE8_HANDLER (famibox_nt_w)
{
	famibox_state *state = space->machine().driver_data<famibox_state>();
	int page = ((offset & 0xc00) >> 10);
	state->m_nt_page[page][offset & 0x3ff] = data;
}


static READ8_HANDLER (famibox_nt_r)
{
	famibox_state *state = space->machine().driver_data<famibox_state>();
	int page = ((offset & 0xc00) >> 10);
	return state->m_nt_page[page][offset & 0x3ff];
}

/******************************************************

   NES interface

*******************************************************/

static WRITE8_HANDLER( sprite_dma_w )
{
	ppu2c0x_device *ppu = space->machine().device<ppu2c0x_device>("ppu");
	int source = (data & 7);
	ppu->spriteram_dma(space, source);
}

static READ8_DEVICE_HANDLER( psg_4015_r )
{
	return nes_psg_r(device, 0x15);
}

static WRITE8_DEVICE_HANDLER( psg_4015_w )
{
	nes_psg_w(device, 0x15, data);
}

static WRITE8_DEVICE_HANDLER( psg_4017_w )
{
	nes_psg_w(device, 0x17, data);
}

/******************************************************

   Inputs

*******************************************************/


static READ8_HANDLER( famibox_IN0_r )
{
	famibox_state *state = space->machine().driver_data<famibox_state>();
	return ((state->m_in_0 >> state->m_in_0_shift++) & 0x01) | 0x40;
}

static WRITE8_HANDLER( famibox_IN0_w )
{
	famibox_state *state = space->machine().driver_data<famibox_state>();
	if (data & 0x01)
	{
		return;
	}

	state->m_in_0_shift = 0;
	state->m_in_1_shift = 0;

	state->m_in_0 = input_port_read(space->machine(), "P1");
	state->m_in_1 = input_port_read(space->machine(), "P2");
}

static READ8_HANDLER( famibox_IN1_r )
{
	famibox_state *state = space->machine().driver_data<famibox_state>();
	return ((state->m_in_1 >> state->m_in_1_shift++) & 0x01) | 0x40;
}

/******************************************************

   System

*******************************************************/
static void famicombox_bankswitch(running_machine &machine, UINT8 bank)
{
	struct
	{
		UINT8 bank;
		const char* memory_region;
		offs_t bank1_offset;
		offs_t bank2_offset;
		offs_t ppubank_offset;
	} famicombox_banks[] =
	{
		{ 0x11, "donkeykong",   0, 0, 0x4000 },
		{ 0x12, "donkeykongjr", 0, 0, 0x4000 },
		{ 0x13, "popeye",       0, 0, 0x4000 },
		{ 0x14, "eigoasobi",    0, 0, 0x4000 },
		{ 0x15, "mahjong",      0, 0, 0x4000 },
		{ 0x26, "gomokunarabe", 0, 0, 0x4000 },
		{ 0x27, "baseball",     0, 0, 0x4000 },
		{ 0x28, "empty",        0, 0, 0x4000 },
		{ 0x29, "empty",        0, 0, 0x4000 },
		{ 0x2a, "empty",        0, 0, 0x4000 },
		{ 0x3b, "empty",        0, 0, 0x4000 },
		{ 0x3c, "empty",        0, 0, 0x4000 },
		{ 0x3d, "empty",        0, 0, 0x4000 },
		{ 0x3e, "empty",        0, 0, 0x4000 },
		{ 0x3f, "empty",        0, 0, 0x4000 },
		{ 0x00, "menu",         0, 0x4000, 0x8000 },
	};

//  famibox_state *state = machine.driver_data<famibox_state>();

	for (int i = 0; i < sizeof(famicombox_banks)/sizeof(famicombox_banks[0]); i++ )
	{
		if ( bank == famicombox_banks[i].bank ||
			 famicombox_banks[i].bank == 0 )
		{
			memory_set_bankptr(machine, "cpubank1", machine.region(famicombox_banks[i].memory_region)->base() + famicombox_banks[i].bank1_offset);
			memory_set_bankptr(machine, "cpubank2", machine.region(famicombox_banks[i].memory_region)->base() + famicombox_banks[i].bank2_offset);
			memory_set_bankptr(machine, "ppubank1", machine.region(famicombox_banks[i].memory_region)->base() + famicombox_banks[i].ppubank_offset);
			break;
		}
	}
}

static void famicombox_reset(running_machine &machine)
{
	famicombox_bankswitch(machine, 0);
	machine.device("maincpu")->reset();
}

static TIMER_CALLBACK(famicombox_attract_timer_callback)
{
	famibox_state *state = machine.driver_data<famibox_state>();

	state->m_attract_timer->adjust(attotime::never, 0, attotime::never);
	if ( BIT(state->m_exception_mask,1) )
	{
		state->m_exception_cause &= ~0x02;
		famicombox_reset(machine);
	}
}

static TIMER_CALLBACK(famicombox_gameplay_timer_callback)
{
	famibox_state *state = machine.driver_data<famibox_state>();

	if (state->m_coins > 0)
		state->m_coins--;

	if (state->m_coins == 0)
	{
		state->m_gameplay_timer->adjust(attotime::never, 0, attotime::never);
		if ( BIT(state->m_exception_mask,4) )
		{
			state->m_exception_cause &= ~0x10;
			famicombox_reset(machine);
		}
	}
}

static READ8_HANDLER( famibox_system_r )
{
	famibox_state *state = space->machine().driver_data<famibox_state>();
	switch( offset & 0x07 )
	{
		case 0: /* device which caused exception */
			{
				UINT8 ret = state->m_exception_cause;
				state->m_exception_cause = 0xff;
				return ret;
			}
		case 2:
			return input_port_read(space->machine(), "DSW");
		case 3:
			return input_port_read(space->machine(), "KEYSWITCH");
		case 7:
			return 0x02;
		default:
			logerror("%s: Unhandled famibox_system_r(%x)\n", space->machine().describe_context(), offset );
			return 0;
	}
}

static WRITE8_HANDLER( famibox_system_w )
{
	famibox_state *state = space->machine().driver_data<famibox_state>();
	switch( offset & 0x07 )
	{
		case 0:
			logerror("%s: Interrupt enable\n", space->machine().describe_context());
			logerror("6.82Hz interrupt source (0 = enable): %d\n", BIT(data,0));
			logerror("8 bit timer expiration @ 5003W (1 = enable): %d\n", BIT(data,1));
			logerror("controller reads (1 = enable): %d\n", BIT(data,2));
			logerror("keyswitch rotation (1 = enable): %d\n", BIT(data,3));
			logerror("money insertion (1 = enable): %d\n", BIT(data,4));
			logerror("reset button (1 = enable): %d\n", BIT(data,5));
			logerror("\"CATV connector\" pin 4 detection (1 = enable): %d\n", BIT(data,7));
			state->m_exception_mask = data;
			if ( BIT(state->m_exception_mask,1) && ( state->m_attract_timer_period != 0 ) )
			{
				if (state->m_attract_timer->start() != attotime::never)
				{
					state->m_attract_timer->adjust(attotime::from_seconds((INT32)((double)1.0/6.8274*state->m_attract_timer_period)), 0, attotime::never);
				}
			}
			break;
		case 1:
			state->m_money_reg = data;
			logerror("%s: Money handling register: %02x\n", space->machine().describe_context(), data);
			break;
		case 2:
			logerror("%s: LED & memory protect register: %02x\n", space->machine().describe_context(), data);
			break;
		case 3:
			logerror("%s: 8 bit down counter, for attract mode timing: %02x\n", space->machine().describe_context(), data);
			state->m_attract_timer_period = data;
			if ( BIT(state->m_exception_mask,1) && ( data != 0 ) )
			{
				state->m_attract_timer->adjust(attotime::from_hz(6.8274/state->m_attract_timer_period), 0, attotime::never);
			}
			break;
		case 4:
			logerror("%s: bankswitch %x\n", space->machine().describe_context(), data );
			famicombox_bankswitch(space->machine(), data & 0x3f);
			break;
		default:
			logerror("%s: Unhandled famibox_system_w(%x,%02x)\n", space->machine().describe_context(), offset, data );
	}
}

/******************************************************

   Memory map

*******************************************************/


static ADDRESS_MAP_START( famibox_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_DEVREADWRITE_MODERN("ppu", ppu2c0x_device, read, write)
	AM_RANGE(0x4000, 0x4013) AM_DEVREADWRITE("nes", nes_psg_r, nes_psg_w)			/* PSG primary registers */
	AM_RANGE(0x4014, 0x4014) AM_WRITE(sprite_dma_w)
	AM_RANGE(0x4015, 0x4015) AM_DEVREADWRITE("nes", psg_4015_r, psg_4015_w)			/* PSG status / first control register */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(famibox_IN0_r, famibox_IN0_w)	/* IN0 - input port 1 */
	AM_RANGE(0x4017, 0x4017) AM_READ(famibox_IN1_r) AM_DEVWRITE("nes", psg_4017_w)		/* IN1 - input port 2 / PSG second control register */
	AM_RANGE(0x5000, 0x5fff) AM_READWRITE(famibox_system_r, famibox_system_w)
	AM_RANGE(0x6000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("cpubank1")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("cpubank2")
ADDRESS_MAP_END

/******************************************************

   Inputs

*******************************************************/

static INPUT_CHANGED( famibox_keyswitch_changed )
{
	famibox_state *state = field.machine().driver_data<famibox_state>();

	if ( BIT(state->m_exception_mask, 3) )
	{
		state->m_exception_cause &= ~0x08;
		famicombox_reset(field.machine());
	}
}

static INPUT_CHANGED( coin_inserted )
{
	famibox_state *state = field.machine().driver_data<famibox_state>();

	if ( newval )
	{
		state->m_coins++;
		if (state->m_attract_timer->start() != attotime::never)
		{
			state->m_gameplay_timer->adjust(attotime::from_seconds(60*(state->m_money_reg == 0x22 ? 20 : 10)), 0, attotime::never);
		}

		if ( BIT(state->m_exception_mask,4) && (state->m_coins == 1) )
		{
			state->m_exception_cause &= ~0x10;
			famicombox_reset(field.machine());
		}
	}
}

static CUSTOM_INPUT( famibox_coin_r )
{
	famibox_state *state = field.machine().driver_data<famibox_state>();
	return state->m_coins > 0;
}

static INPUT_PORTS_START( famibox )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)	/* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)	/* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Self Test" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Coin timeout period" )
	PORT_DIPSETTING(    0x00, "10 min" )
	PORT_DIPSETTING(    0x02, "20 min" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Famicombox menu time" )
	PORT_DIPSETTING(    0x00, "7 sec" )
	PORT_DIPSETTING(    0x08, "12 sec" )
	PORT_DIPNAME( 0x30, 0x00, "Attract time" )
	PORT_DIPSETTING(    0x00, "12 sec" )
	PORT_DIPSETTING(    0x10, "23 sec" )
	PORT_DIPSETTING(    0x20, "17 sec" )
	PORT_DIPSETTING(    0x30, "7 sec" )
	PORT_DIPNAME( 0xc0, 0x80, "Operational mode" )
	PORT_DIPSETTING(    0x00, "KEY MODE" )
	PORT_DIPSETTING(    0x40, "CATV MODE" )
	PORT_DIPSETTING(    0x80, "COIN MODE" )
	PORT_DIPSETTING(    0xc0, "FREEPLAY" )


	PORT_START("KEYSWITCH")
	PORT_DIPNAME( 0x3f, 0x01, "Key switch" ) PORT_CHANGED(famibox_keyswitch_changed, 0)
	PORT_DIPSETTING(    0x01, "Key position 1" )
	PORT_DIPSETTING(    0x02, "Key position 2" )
	PORT_DIPSETTING(    0x04, "Key position 3" )
	PORT_DIPSETTING(    0x08, "Key position 4" )
	PORT_DIPSETTING(    0x10, "Key position 5" )
	PORT_DIPSETTING(    0x20, "Key position 6" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(famibox_coin_r, NULL)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED(coin_inserted, 0)

INPUT_PORTS_END

/******************************************************

   PPU/Video interface

*******************************************************/

static const nes_interface famibox_interface_1 =
{
	"maincpu"
};

static PALETTE_INIT( famibox )
{
	ppu2c0x_device *ppu = machine.device<ppu2c0x_device>("ppu");
	ppu->init_palette(machine, 0);
}

static void ppu_irq( device_t *device, int *ppu_regs )
{
	cputag_set_input_line(device->machine(), "maincpu", INPUT_LINE_NMI, PULSE_LINE);
}

/* our ppu interface                                            */
static const ppu2c0x_interface ppu_interface =
{
	"maincpu",
	"screen",
	0,					/* gfxlayout num */
	0,					/* color base */
	PPU_MIRROR_NONE,	/* mirroring */
	ppu_irq				/* irq */
};

static VIDEO_START( famibox )
{
}

static SCREEN_UPDATE_IND16( famibox )
{
	/* render the ppu */
	ppu2c0x_device *ppu = screen.machine().device<ppu2c0x_device>("ppu");
	ppu->render(bitmap, 0, 0, 0, 0);
	return 0;
}

static GFXDECODE_START( famibox )
	/* none, the ppu generates one */
GFXDECODE_END

static MACHINE_RESET( famibox )
{
	famicombox_bankswitch(machine, 0);
}

static MACHINE_START( famibox )
{
	famibox_state *state = machine.driver_data<famibox_state>();
	state->m_nt_ram = auto_alloc_array(machine, UINT8, 0x1000);
	state->m_nt_page[0] = state->m_nt_ram;
	state->m_nt_page[1] = state->m_nt_ram + 0x400;
	state->m_nt_page[2] = state->m_nt_ram + 0x800;
	state->m_nt_page[3] = state->m_nt_ram + 0xc00;

	machine.device("ppu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x2000, 0x3eff, FUNC(famibox_nt_r), FUNC(famibox_nt_w));
	machine.device("ppu")->memory().space(AS_PROGRAM)->install_read_bank(0x0000, 0x1fff, "ppubank1");

	famicombox_bankswitch(machine, 0);


	state->m_attract_timer = machine.scheduler().timer_alloc(FUNC(famicombox_attract_timer_callback));
	state->m_gameplay_timer = machine.scheduler().timer_alloc(FUNC(famicombox_gameplay_timer_callback));
	state->m_exception_cause = 0xff;
	state->m_exception_mask = 0;
	state->m_attract_timer_period = 0;
	state->m_money_reg = 0;
	state->m_coins = 0;
}

static MACHINE_CONFIG_START( famibox, famibox_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", N2A03, N2A03_DEFAULTCLOCK)
	MCFG_CPU_PROGRAM_MAP(famibox_map)

	MCFG_MACHINE_START( famibox )
	MCFG_MACHINE_RESET( famibox )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(famibox)

	MCFG_GFXDECODE(famibox)
	MCFG_PALETTE_LENGTH(8*4*16)

	MCFG_PALETTE_INIT(famibox)
	MCFG_VIDEO_START(famibox)

	MCFG_PPU2C04_ADD("ppu", ppu_interface)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("nes", NES, N2A03_DEFAULTCLOCK)
	MCFG_SOUND_CONFIG(famibox_interface_1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


ROM_START(famibox)
	ROM_REGION(0xa000, "menu", 0)
	ROM_LOAD("sss_menu.prg", 0x0000, 0x8000, CRC(da1eb8d2) SHA1(943e3b0edfbf9bd3ee87dc5f298621b9ddc98db8))
	ROM_LOAD("sss_menu.chr", 0x8000, 0x2000, CRC(a43d4435) SHA1(ee56b4d2110aff394bf2c8cd3414ca175ace01bd))

	ROM_REGION(0x6000, "donkeykong", 0)
	ROM_LOAD("0.prg", 0x0000, 0x4000, CRC(06d1a012) SHA1(a6f92ae0a991c532e6377db2b3ab7f5c13d27675) )
	ROM_LOAD("0.chr", 0x4000, 0x2000, CRC(a21d7c2e) SHA1(97c16cd6b1f3656428b682a23e6e4248c1ca3607) )

	ROM_REGION(0x6000, "donkeykongjr", 0)
	ROM_LOAD("hvc-jr-1 prg", 0x0000, 0x4000, CRC(cf6c88b6) SHA1(cefc276e7601d14c6a20e545f334281b7a9fe8db) )
	ROM_LOAD("hvc-jr-0 chr", 0x4000, 0x2000, CRC(852778ab) SHA1(307f2245cce164491012f75897eb984af0c3f456) )

	ROM_REGION(0x6000, "popeye", 0)
	ROM_LOAD("hvc-pp-1 prg", 0x0000, 0x4000, CRC(0fa63a45) SHA1(50c594a6d8dcbeee2d83bca8c54c42cf57093aba) )
	ROM_LOAD("hvc-pp-0 chr", 0x4000, 0x2000, CRC(a5fd8d98) SHA1(09d229404babb6c89b417ac541bab80fb06d2ba9) )

	ROM_REGION(0x6000, "eigoasobi", 0)
	ROM_LOAD("hvc-en-0 prg", 0x0000, 0x4000, CRC(2dbfa36a) SHA1(0f4301d78d3dfa163239e7b7b7c4dff8a7e21bac) )
	ROM_LOAD("hvc-en-0 chr", 0x4000, 0x2000, CRC(fccc0f36) SHA1(3566709c3c74960ce2ee1e60a85d026e70d7fd2c) )

	ROM_REGION(0x6000, "mahjong", 0)
	ROM_LOAD("mahjong.prg", 0x0000, 0x4000, CRC(f86d8d8a) SHA1(2904137a030ae2370a8cd3e068078a1d59a4f229) )
	ROM_LOAD("mahjong.chr", 0x4000, 0x2000, CRC(6bb45576) SHA1(5974787496dfa27a4b7fe6023473fae930ea41dc) )

	ROM_REGION(0x6000, "gomokunarabe", 0)
	ROM_LOAD("hvc-go-0 prg", 0x0000, 0x4000, CRC(5603f579) SHA1(f2b007e3b13a777f9f88ff58f87ead6ae8f26327) )
	ROM_LOAD("hvc-go-0 chr", 0x4000, 0x2000, CRC(97ea7144) SHA1(47d354c654285275d0a9420cc6eb3564f0453eb0) )

	ROM_REGION(0x6000, "baseball", 0)
	ROM_LOAD("hvc-ba-0 prg", 0x0000, 0x4000, CRC(d18a3dde) SHA1(91f7d3e4c9d18c1969ca1fffdc811b763508a0a2) )
	ROM_LOAD("hvc-ba-0 chr", 0x4000, 0x2000, CRC(c27eef20) SHA1(d5bd643b3ba98846e520b4d3f38aae45a29cf250) )

	ROM_REGION(0x6000, "empty", ROMREGION_ERASEFF)

ROM_END

GAME( 1986,  famibox,      0,  famibox,  famibox,  0, ROT0, "Nintendo", "FamicomBox", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND)

