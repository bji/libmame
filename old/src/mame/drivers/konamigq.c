/***************************************************************************

  Konami GQ System - Arcade PSX Hardware
  ======================================
  Driver by R. Belmont & smf

  Crypt Killer
  Konami, 1995

  PCB Layout
  ----------

  GQ420  PWB354905B
  |----------------------------------------------------------|
  |CN14     420A01.2G  420A02.3M           CN6  CN7   CN8    |
  |056602    6264                                            |
  |LA4705    6264         058141  424800                     |
  |          68000                                           |
  |  18.432MHz      PAL   056800  424800                     |
  |  32MHz    48MHz PAL           424800                     |
  |  RESET_SW             TMS57002                           |
  |                               M514256                    |
  |                               M514256                    |
  |J  000180                                                 |
  |A                              MACH110                    |
  |M         53.693175MHz         (000619A)                  |
  |M         KM4216V256                                      |
  |A         KM4216V256     CXD8538Q        PAL (000613)     |
  |          KM4216V256                     PAL (000612)     |
  | TEST_SW  KM4216V256                     PAL (000618)     |
  |   CXD2923AR   67.7376MHz                                 |
  | DIPSW(8)                   93C46       NCR 53CF96-2      |
  | KM48V514  KM48V514         420B03.27P    HD_LED          |
  | KM48V514  KM48V514                                       |
  | CN3                CXD8530BQ      TOSHIBA MK1924FBV      |
  | KM48V514  KM48V514                (U420UAA04)            |
  | KM48V514  KM48V514                                       |
  |----------------------------------------------------------|

  Notes:
        CN6, CN7, CN8: For connection of guns.
        CN3 : For connection of extra controls/buttons.
        CN14: For connection of additional speaker for stereo output.
        68000 clock: 8.000MHz
        Vsync: 60Hz
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mips/psx.h"
#include "includes/psx.h"
#include "includes/konamigx.h"
#include "machine/eeprom.h"
#include "machine/am53cf96.h"
#include "harddisk.h"
#include "sound/k054539.h"

class konamigq_state : public psx_state
{
public:
	konamigq_state(running_machine &machine, const driver_device_config_base &config)
		: psx_state(machine, config) { }

	UINT8 sndto000[ 16 ];
	UINT8 sndtor3k[ 16 ];
	UINT8 *p_n_pcmram;
	UINT8 sector_buffer[ 512 ];
};

/* Sound */

static WRITE32_HANDLER( soundr3k_w )
{
	konamigq_state *state = space->machine->driver_data<konamigq_state>();

	if( ACCESSING_BITS_16_31 )
	{
		state->sndto000[ ( offset << 1 ) + 1 ] = data >> 16;
		if( offset == 3 )
		{
			cputag_set_input_line(space->machine, "soundcpu", 1, HOLD_LINE );
		}
	}
	if( ACCESSING_BITS_0_15 )
	{
		state->sndto000[ offset << 1 ] = data;
	}
}

static READ32_HANDLER( soundr3k_r )
{
	konamigq_state *state = space->machine->driver_data<konamigq_state>();
	UINT32 data;

	data = ( state->sndtor3k[ ( offset << 1 ) + 1 ] << 16 ) | state->sndtor3k[ offset << 1 ];

	/* hack to help the main program start up */
	if( offset == 1 )
	{
		data = 0;
	}

	return data;
}

/* UART */

static WRITE32_HANDLER( mb89371_w )
{
}

static READ32_HANDLER( mb89371_r )
{
	return 0xffffffff;
}

/* EEPROM */

static const UINT16 konamigq_def_eeprom[64] =
{
	0x292b, 0x5256, 0x2094, 0x4155, 0x0041, 0x1414, 0x0003, 0x0101,
	0x0103, 0x0000, 0x0707, 0x0001, 0xaa00, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
};

static WRITE32_HANDLER( eeprom_w )
{
	input_port_write(space->machine, "EEPROMOUT", data & 0x07, 0xff);
	cputag_set_input_line(space->machine, "soundcpu", INPUT_LINE_RESET, ( data & 0x40 ) ? CLEAR_LINE : ASSERT_LINE );
}


/* PCM RAM */

static WRITE32_HANDLER( pcmram_w )
{
	konamigq_state *state = space->machine->driver_data<konamigq_state>();

	if( ACCESSING_BITS_0_7 )
	{
		state->p_n_pcmram[ offset << 1 ] = data;
	}
	if( ACCESSING_BITS_16_23 )
	{
		state->p_n_pcmram[ ( offset << 1 ) + 1 ] = data >> 16;
	}
}

static READ32_HANDLER( pcmram_r )
{
	konamigq_state *state = space->machine->driver_data<konamigq_state>();

	return ( state->p_n_pcmram[ ( offset << 1 ) + 1 ] << 16 ) | state->p_n_pcmram[ offset << 1 ];
}

/* Video */

static ADDRESS_MAP_START( konamigq_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM	AM_SHARE("share1") /* ram */
	AM_RANGE(0x1f000000, 0x1f00001f) AM_READWRITE(am53cf96_r, am53cf96_w)
	AM_RANGE(0x1f100000, 0x1f10000f) AM_WRITE(soundr3k_w)
	AM_RANGE(0x1f100010, 0x1f10001f) AM_READ(soundr3k_r)
	AM_RANGE(0x1f180000, 0x1f180003) AM_WRITE(eeprom_w)
	AM_RANGE(0x1f198000, 0x1f198003) AM_WRITENOP    		/* cabinet lamps? */
	AM_RANGE(0x1f1a0000, 0x1f1a0003) AM_WRITENOP    		/* indicates gun trigger */
	AM_RANGE(0x1f200000, 0x1f200003) AM_READ_PORT("GUNX1")
	AM_RANGE(0x1f208000, 0x1f208003) AM_READ_PORT("GUNY1")
	AM_RANGE(0x1f210000, 0x1f210003) AM_READ_PORT("GUNX2")
	AM_RANGE(0x1f218000, 0x1f218003) AM_READ_PORT("GUNY2")
	AM_RANGE(0x1f220000, 0x1f220003) AM_READ_PORT("GUNX3")
	AM_RANGE(0x1f228000, 0x1f228003) AM_READ_PORT("GUNY3")
	AM_RANGE(0x1f230000, 0x1f230003) AM_READ_PORT("P1_P2")
	AM_RANGE(0x1f230004, 0x1f230007) AM_READ_PORT("P3_SERVICE")
	AM_RANGE(0x1f238000, 0x1f238003) AM_READ_PORT("DSW")
	AM_RANGE(0x1f300000, 0x1f5fffff) AM_READWRITE(pcmram_r, pcmram_w)
	AM_RANGE(0x1f680000, 0x1f68001f) AM_READWRITE(mb89371_r, mb89371_w)
	AM_RANGE(0x1f780000, 0x1f780003) AM_WRITENOP /* watchdog? */
	AM_RANGE(0x1f800000, 0x1f8003ff) AM_RAM /* scratchpad */
	AM_RANGE(0x1f801000, 0x1f801007) AM_WRITENOP
	AM_RANGE(0x1f801008, 0x1f80100b) AM_RAM /* ?? */
	AM_RANGE(0x1f80100c, 0x1f80102f) AM_WRITENOP
	AM_RANGE(0x1f801010, 0x1f801013) AM_READNOP
	AM_RANGE(0x1f801014, 0x1f801017) AM_READNOP
	AM_RANGE(0x1f801040, 0x1f80105f) AM_READWRITE(psx_sio_r, psx_sio_w)
	AM_RANGE(0x1f801060, 0x1f80106f) AM_WRITENOP
	AM_RANGE(0x1f801070, 0x1f801077) AM_READWRITE(psx_irq_r, psx_irq_w)
	AM_RANGE(0x1f801080, 0x1f8010ff) AM_READWRITE(psx_dma_r, psx_dma_w)
	AM_RANGE(0x1f801100, 0x1f80112f) AM_READWRITE(psx_counter_r, psx_counter_w)
	AM_RANGE(0x1f801810, 0x1f801817) AM_READWRITE(psx_gpu_r, psx_gpu_w)
	AM_RANGE(0x1f801820, 0x1f801827) AM_READWRITE(psx_mdec_r, psx_mdec_w)
	AM_RANGE(0x1f801c00, 0x1f801dff) AM_NOP
	AM_RANGE(0x1f802020, 0x1f802033) AM_RAM /* ?? */
	AM_RANGE(0x1f802040, 0x1f802043) AM_WRITENOP
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_SHARE("share2") AM_REGION("user1", 0) /* bios */
	AM_RANGE(0x80000000, 0x803fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xa0000000, 0xa03fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END

/* SOUND CPU */

static READ16_HANDLER( dual539_r )
{
	UINT16 data;

	data = 0;
	if( ACCESSING_BITS_0_7 )
	{
		data |= k054539_r( space->machine->device("konami2"), offset );
	}
	if( ACCESSING_BITS_8_15 )
	{
		data |= k054539_r( space->machine->device("konami1"), offset ) << 8;
	}
	return data;
}

static WRITE16_HANDLER( dual539_w )
{
	if( ACCESSING_BITS_0_7 )
	{
		k054539_w( space->machine->device("konami2"), offset, data );
	}
	if( ACCESSING_BITS_8_15 )
	{
		k054539_w( space->machine->device("konami1"), offset, data >> 8 );
	}
}

static READ16_HANDLER( sndcomm68k_r )
{
	konamigq_state *state = space->machine->driver_data<konamigq_state>();

	return state->sndto000[ offset ];
}

static WRITE16_HANDLER( sndcomm68k_w )
{
	konamigq_state *state = space->machine->driver_data<konamigq_state>();

	state->sndtor3k[ offset ] = data;
}

static READ16_HANDLER(tms57002_data_word_r)
{
	return 0;
}

static WRITE16_HANDLER(tms57002_data_word_w)
{
}

static READ16_HANDLER(tms57002_status_word_r)
{
	return 0;
}

static WRITE16_HANDLER(tms57002_control_word_w)
{
}

/* 68000 memory handling */
static ADDRESS_MAP_START( konamigq_sound_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x200000, 0x2004ff) AM_READWRITE(dual539_r,dual539_w)
	AM_RANGE(0x300000, 0x300001) AM_READWRITE(tms57002_data_word_r,tms57002_data_word_w)
	AM_RANGE(0x400000, 0x40000f) AM_WRITE(sndcomm68k_w)
	AM_RANGE(0x400010, 0x40001f) AM_READ(sndcomm68k_r)
	AM_RANGE(0x500000, 0x500001) AM_READWRITE(tms57002_status_word_r,tms57002_control_word_w)
	AM_RANGE(0x580000, 0x580001) AM_WRITENOP /* ?? */
ADDRESS_MAP_END


static const k054539_interface k054539_config =
{
	"shared"
};

/* SCSI */

static void scsi_dma_read( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	konamigq_state *state = machine->driver_data<konamigq_state>();
	UINT32 *p_n_psxram = state->p_n_psxram;
	UINT8 *sector_buffer = state->sector_buffer;
	int i;
	int n_this;

	while( n_size > 0 )
	{
		if( n_size > sizeof( state->sector_buffer ) / 4 )
		{
			n_this = sizeof( state->sector_buffer ) / 4;
		}
		else
		{
			n_this = n_size;
		}
		am53cf96_read_data( n_this * 4, sector_buffer );
		n_size -= n_this;

		i = 0;
		while( n_this > 0 )
		{
			p_n_psxram[ n_address / 4 ] =
				( sector_buffer[ i + 0 ] << 0 ) |
				( sector_buffer[ i + 1 ] << 8 ) |
				( sector_buffer[ i + 2 ] << 16 ) |
				( sector_buffer[ i + 3 ] << 24 );
			n_address += 4;
			i += 4;
			n_this--;
		}
	}
}

static void scsi_dma_write( running_machine *machine, UINT32 n_address, INT32 n_size )
{
}

static void scsi_irq(running_machine *machine)
{
	psx_irq_set(machine, 0x400);
}

static const SCSIConfigTable dev_table =
{
	1, /* 1 SCSI device */
	{
		{ SCSI_ID_0, "disk", SCSI_DEVICE_HARDDISK } /* SCSI ID 0, using CHD 0, and it's a HDD */
	}
};

static const struct AM53CF96interface scsi_intf =
{
	&dev_table,		/* SCSI device table */
	&scsi_irq,		/* command completion IRQ */
};

static DRIVER_INIT( konamigq )
{
	konamigq_state *state = machine->driver_data<konamigq_state>();

	psx_driver_init(machine);

	state->p_n_pcmram = machine->region( "shared" )->base() + 0x80000;
}

static void konamigq_exit(running_machine &machine)
{
	am53cf96_exit(&scsi_intf);
}

static MACHINE_START( konamigq )
{
	konamigq_state *state = machine->driver_data<konamigq_state>();

	/* init the scsi controller and hook up it's DMA */
	am53cf96_init(machine, &scsi_intf);
	machine->add_notifier(MACHINE_NOTIFY_EXIT, konamigq_exit);
	psx_dma_install_read_handler(machine, 5, scsi_dma_read);
	psx_dma_install_write_handler(machine, 5, scsi_dma_write);

	state->save_pointer(NAME(state->p_n_pcmram), 0x380000);
	state->save_item(NAME(state->sndto000));
	state->save_item(NAME(state->sndtor3k));
	state->save_item(NAME(state->sector_buffer));
}

static MACHINE_RESET( konamigq )
{
	psx_machine_init(machine);
}

static MACHINE_CONFIG_START( konamigq, konamigq_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", PSXCPU, XTAL_67_7376MHz )
	MCFG_CPU_PROGRAM_MAP( konamigq_map)
	MCFG_CPU_VBLANK_INT("screen", psx_vblank)

	MCFG_CPU_ADD( "soundcpu", M68000, 8000000 )
	MCFG_CPU_PROGRAM_MAP( konamigq_sound_map)
	MCFG_CPU_PERIODIC_INT( irq2_line_hold, 480 )

	MCFG_MACHINE_START( konamigq )
	MCFG_MACHINE_RESET( konamigq )
	MCFG_EEPROM_93C46_ADD("eeprom")
	MCFG_EEPROM_DATA(konamigq_def_eeprom, 128)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC( 0 ))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE( 1024, 1024 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 639, 0, 479 )
	MCFG_SCREEN_UPDATE( psx )

	MCFG_PALETTE_LENGTH( 65536 )

	MCFG_PALETTE_INIT( psx )
	MCFG_VIDEO_START( psx_type1 )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("konami1", K054539, 48000)
	MCFG_SOUND_CONFIG(k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("konami2", K054539, 48000)
	MCFG_SOUND_CONFIG(k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static INPUT_PORTS_START( konamigq )
	PORT_START("GUNX1")
	PORT_BIT( 0x01ff, 0x011d, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x007d, 0x01bc ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNY1")
	PORT_BIT( 0x00ff, 0x0078, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNX2")
	PORT_BIT( 0x01ff, 0x011d, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x007d, 0x01bc ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("GUNY2")
	PORT_BIT( 0x00ff, 0x0078, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("GUNX3")
	PORT_BIT( 0x01ff, 0x011d, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x007d, 0x01bc ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(3)

	PORT_START("GUNY3")
	PORT_BIT( 0x00ff, 0x0078, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(3)

	PORT_START("P1_P2")
	PORT_BIT( 0x000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x000002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* trigger */
	PORT_BIT( 0x000010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) /* reload */
	PORT_BIT( 0x000020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x010000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x020000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* trigger */
	PORT_BIT( 0x100000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* reload */
	PORT_BIT( 0x200000, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x800000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_SERVICE")
	PORT_BIT( 0x000001, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x000002, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) /* trigger */
	PORT_BIT( 0x000010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) /* reload */
	PORT_BIT( 0x000020, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x010000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x800000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00000001, 0x01, DEF_STR( Stereo ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Stereo ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Mono ) )
	PORT_DIPNAME( 0x00000002, 0x00, "Stage Set" )
	PORT_DIPSETTING(    0x02, "Endless" )
	PORT_DIPSETTING(    0x00, "6st End" )
	PORT_DIPNAME( 0x00000004, 0x04, "Mirror" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00000008, 0x08, "Woofer" )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00000010, 0x10, "Number of Players" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPNAME( 0x00000020, 0x20, "Coin Mechanism (2p only)" )
	PORT_DIPSETTING(    0x20, "Common" )
	PORT_DIPSETTING(    0x00, "Independent" )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
INPUT_PORTS_END

ROM_START( cryptklr )
	ROM_REGION( 0x80000, "soundcpu", 0 ) /* 68000 sound program */
	ROM_LOAD16_WORD_SWAP( "420a01.2g", 0x000000, 0x080000, CRC(84fc2613) SHA1(e06f4284614d33c76529eb43b168d095200a9eac) )

	ROM_REGION( 0x400000, "shared", 0 )
	ROM_LOAD( "420a02.3m",    0x000000, 0x080000, CRC(2169c3c4) SHA1(6d525f10385791e19eb1897d18f0bab319640162) )

	ROM_REGION32_LE( 0x080000, "user1", 0 ) /* bios */
	ROM_LOAD( "420b03.27p",   0x0000000, 0x080000, CRC(aab391b1) SHA1(bf9dc7c0c8168c22a4be266fe6a66d3738df916b) )

	DISK_REGION( "disk" )
	DISK_IMAGE( "420uaa04", 0, SHA1(67cb1418fc0de2a89fc61847dc9efb9f1bebb347) )
ROM_END

GAME( 1995, cryptklr, 0, konamigq, konamigq, konamigq, ROT0, "Konami", "Crypt Killer (GQ420 UAA)", GAME_IMPERFECT_GRAPHICS )
