/***********************************************************************
 PGM IGS027A (55857F* type) ARM protection emulation
  *guess, the part number might not be directly tied to behavior, see note below

 these are emulation of the 'kov2' type ARM device
 used by

 Knights of Valor 2 (kov2)
 Knights of Valor 2 Nine Dragons (kov2p) *
 DoDonPachi 2 - Bee Storm (ddp2)
 Martial Masters (martmast)

 * using a hacked kov2 internal ROM

 the following also use this device, but the internal
 ROMs are not yet dumped

 Dragon World 2001 (dw2001)
 Dragon World Pretty Chance (dwpc)

 ----

  These games use a larger region of shared RAM than the 55857E
  type and have a communication based on interrupts

 ----

 All of these games have an external ARM rom.

 The external ARM roms are encrypted, the internal ARM rom uploads
 the decryption tables.

 Game Region is supplied by internal ARM rom.

 ***********************************************************************/

#include "emu.h"
#include "includes/pgm.h"


static READ32_HANDLER( arm7_latch_arm_r )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();

	device_set_input_line(state->m_prot, ARM7_FIRQ_LINE, CLEAR_LINE ); // guess

	if (PGMARM7LOGERROR)
		logerror("ARM7: Latch read: %08x (%08x) (%06x)\n", state->m_kov2_latchdata_68k_w, mem_mask, cpu_get_pc(&space->device()));
	return state->m_kov2_latchdata_68k_w;
}

static WRITE32_HANDLER( arm7_latch_arm_w )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();

	if (PGMARM7LOGERROR)
		logerror("ARM7: Latch write: %08x (%08x) (%06x)\n", data, mem_mask, cpu_get_pc(&space->device()));

	COMBINE_DATA(&state->m_kov2_latchdata_arm_w);
}

static READ32_HANDLER( arm7_shareram_r )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();

	if (PGMARM7LOGERROR)
		logerror("ARM7: ARM7 Shared RAM Read: %04x = %08x (%08x) (%06x)\n", offset << 2, state->m_arm7_shareram[offset], mem_mask, cpu_get_pc(&space->device()));
	return state->m_arm7_shareram[offset];
}

static WRITE32_HANDLER( arm7_shareram_w )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();

	if (PGMARM7LOGERROR)
		logerror("ARM7: ARM7 Shared RAM Write: %04x = %08x (%08x) (%06x)\n", offset << 2, data, mem_mask, cpu_get_pc(&space->device()));
	COMBINE_DATA(&state->m_arm7_shareram[offset]);
}

static READ16_HANDLER( arm7_latch_68k_r )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();

	if (PGMARM7LOGERROR)
		logerror("M68K: Latch read: %04x (%04x) (%06x)\n", state->m_kov2_latchdata_arm_w & 0x0000ffff, mem_mask, cpu_get_pc(&space->device()));
	return state->m_kov2_latchdata_arm_w;
}

static WRITE16_HANDLER( arm7_latch_68k_w )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();

	if (PGMARM7LOGERROR)
		logerror("M68K: Latch write: %04x (%04x) (%06x)\n", data & 0x0000ffff, mem_mask, cpu_get_pc(&space->device()));
	COMBINE_DATA(&state->m_kov2_latchdata_68k_w);

	device_set_input_line(state->m_prot, ARM7_FIRQ_LINE, ASSERT_LINE ); // guess
}

static READ16_HANDLER( arm7_ram_r )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();
	UINT16 *share16 = reinterpret_cast<UINT16 *>(state->m_arm7_shareram.target());

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Read: %04x = %04x (%08x) (%06x)\n", BYTE_XOR_LE(offset), share16[BYTE_XOR_LE(offset)], mem_mask, cpu_get_pc(&space->device()));
	return share16[BYTE_XOR_LE(offset)];
}

static WRITE16_HANDLER( arm7_ram_w )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();
	UINT16 *share16 = reinterpret_cast<UINT16 *>(state->m_arm7_shareram.target());

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Write: %04x = %04x (%04x) (%06x)\n", BYTE_XOR_LE(offset), data, mem_mask, cpu_get_pc(&space->device()));
	COMBINE_DATA(&share16[BYTE_XOR_LE(offset)]);
}

/* 55857F? */
/* Knights of Valor 2, Martial Masters, DoDonpachi 2 */
/*  no execute only space? */
static ADDRESS_MAP_START( kov2_mem, AS_PROGRAM, 16, pgm_arm_type2_state )
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x5fffff) AM_ROMBANK("bank1") /* Game ROM */
	AM_RANGE(0xd00000, 0xd0ffff) AM_READWRITE_LEGACY(arm7_ram_r, arm7_ram_w) /* ARM7 Shared RAM */
	AM_RANGE(0xd10000, 0xd10001) AM_READWRITE_LEGACY(arm7_latch_68k_r, arm7_latch_68k_w) /* ARM7 Latch */
ADDRESS_MAP_END


static ADDRESS_MAP_START( 55857F_arm7_map, AS_PROGRAM, 32, pgm_arm_type2_state )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x08000000, 0x083fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM
	AM_RANGE(0x18000000, 0x1800ffff) AM_RAM AM_SHARE("arm_ram")
	AM_RANGE(0x38000000, 0x38000003) AM_READWRITE_LEGACY(arm7_latch_arm_r, arm7_latch_arm_w) /* 68k Latch */
	AM_RANGE(0x48000000, 0x4800ffff) AM_READWRITE_LEGACY(arm7_shareram_r, arm7_shareram_w) AM_SHARE("arm7_shareram")
	AM_RANGE(0x50000000, 0x500003ff) AM_RAM
ADDRESS_MAP_END

MACHINE_START( pgm_arm_type2 )
{
	MACHINE_START_CALL(pgm);
	pgm_arm_type2_state *state = machine.driver_data<pgm_arm_type2_state>();

	state->m_prot = machine.device<cpu_device>("prot");

	/* register type specific Save State stuff here */

}

/******* ARM 55857F *******/

MACHINE_CONFIG_START( pgm_arm_type2, pgm_arm_type2_state )
	MCFG_FRAGMENT_ADD(pgmbase)

	MCFG_MACHINE_START( pgm_arm_type2 )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(kov2_mem)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000)	// 55857F
	MCFG_CPU_PROGRAM_MAP(55857F_arm7_map)
MACHINE_CONFIG_END




static void kov2_latch_init( running_machine &machine )
{
	pgm_arm_type2_state *state = machine.driver_data<pgm_arm_type2_state>();

	state->m_kov2_latchdata_68k_w = 0;
	state->m_kov2_latchdata_arm_w = 0;

	state->save_item(NAME(state->m_kov2_latchdata_68k_w));
	state->save_item(NAME(state->m_kov2_latchdata_arm_w));
}

static WRITE32_HANDLER( kov2_arm_region_w )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();
	int pc = cpu_get_pc(&space->device());
	int regionhack = state->ioport("RegionHack")->read();
	if (pc==0x190 && regionhack != 0xff) data = (data & 0xffff0000) | (regionhack << 0);
	COMBINE_DATA(&state->m_arm7_shareram[0x138/4]);
}

DRIVER_INIT( kov2 )
{
	pgm_basic_init(machine);
	pgm_kov2_decrypt(machine);
	kov2_latch_init(machine);

	// we only have a HK internal ROM dumped for now, allow us to override that for debugging purposes.
	machine.device("prot")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x48000138, 0x4800013b, FUNC(kov2_arm_region_w));
}


DRIVER_INIT( kov2p )
{
	// this hacks the identification of the kov2 rom to return the string required for kov2p
	// this isn't guaranteed to work properly (and definitely wouldn't on real hardware due to the internal
	// ROM uploading the encryption table)  The internal ROM should be dumped properly.
	pgm_basic_init(machine);
	pgm_kov2p_decrypt(machine);
	kov2_latch_init(machine);

	UINT8 *mem8 = (UINT8 *)machine.root_device().memregion("user1")->base();
	mem8[0xDE] = 0xC0;
	mem8[0xDF] = 0x46;
	mem8[0x4ED8] = 0xA8;// B0
	mem8[0x4EDC] = 0x9C;// A4
	mem8[0x4EE0] = 0x5C;// 64
	mem8[0x4EE4] = 0x94;// 9C
	mem8[0x4EE8] = 0xE8;// F0
	mem8[0x4EEC] = 0x6C;// 74
	mem8[0x4EF0] = 0xD4;// DC
	mem8[0x4EF4] = 0x50;// 58
	mem8[0x4EF8] = 0x80;// 88
	mem8[0x4EFC] = 0x9C;// A4
	mem8[0x4F00] = 0x28;// 30
	mem8[0x4F04] = 0x30;// 38
	mem8[0x4F08] = 0x34;// 3C
	mem8[0x4F0C] = 0x1C;// 24
	mem8[0x1FFFFC] = 0x33;
	mem8[0x1FFFFD] = 0x99;

	// we only have a HK internal ROM dumped for now, allow us to override that for debugging purposes.
	machine.device("prot")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x48000138, 0x4800013b, FUNC(kov2_arm_region_w));
}

static WRITE32_HANDLER( martmast_arm_region_w )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();
	int pc = cpu_get_pc(&space->device());
	int regionhack = state->ioport("RegionHack")->read();
	if (pc==0x170 && regionhack != 0xff) data = (data & 0xffff0000) | (regionhack << 0);
	COMBINE_DATA(&state->m_arm7_shareram[0x138/4]);
}


DRIVER_INIT( martmast )
{
	pgm_basic_init(machine);
	pgm_mm_decrypt(machine);
	kov2_latch_init(machine);

	// we only have a USA / CHINA internal ROMs dumped for now, allow us to override that for debugging purposes.
	machine.device("prot")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x48000138, 0x4800013b, FUNC(martmast_arm_region_w));
}


static WRITE32_HANDLER( ddp2_arm_region_w )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();
	int pc = cpu_get_pc(&space->device());
	int regionhack = state->ioport("RegionHack")->read();
	if (pc==0x0174 && regionhack != 0xff) data = (data & 0x0000ffff) | (regionhack << 16);
	COMBINE_DATA(&state->m_arm7_shareram[0x0]);
}

static READ32_HANDLER( ddp2_speedup_r )
{
	pgm_arm_type2_state *state = space->machine().driver_data<pgm_arm_type2_state>();
	int pc = cpu_get_pc(&space->device());
	UINT32 data = state->m_arm_ram[0x300c/4];

	if (pc==0x080109b4)
	{
		/* if we've hit the loop where this is read and both values are 0 then the only way out is an interrupt */
		int r4 = (cpu_get_reg(&space->device(), ARM7_R4));
		r4 += 0xe;

		if (r4==0x18002f9e)
		{
			UINT32 data2 =  state->m_arm_ram[0x2F9C/4]&0xffff0000;
			if ((data==0x00000000) && (data2==0x00000000)) device_spin_until_interrupt(&space->device());
		}
	}

	return data;
}

static READ16_HANDLER( ddp2_main_speedup_r )
{

	UINT16 data = pgm_mainram[0x0ee54/2];
	int pc = cpu_get_pc(&space->device());

	if (pc == 0x149dce) device_spin_until_interrupt(&space->device());
	if (pc == 0x149cfe) device_spin_until_interrupt(&space->device());

	return data;

}

DRIVER_INIT( ddp2 )
{
	pgm_basic_init(machine);
	pgm_ddp2_decrypt(machine);
	kov2_latch_init(machine);

	// we only have a Japan internal ROM dumped for now, allow us to override that for debugging purposes.
	machine.device("prot")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x48000000, 0x48000003, FUNC(ddp2_arm_region_w));

	machine.device("prot")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x1800300c, 0x1800300f, FUNC(ddp2_speedup_r));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x80ee54, 0x80ee55, FUNC(ddp2_main_speedup_r));
}


DRIVER_INIT( dw2001 )
{
	pgm_arm_type2_state *state = machine.driver_data<pgm_arm_type2_state>();
	UINT16 *mem16 = (UINT16 *)state->memregion("maincpu")->base();

	pgm_basic_init(machine);
	kov2_latch_init(machine);
	pgm_mm_decrypt(machine); // encryption is the same as martial masters

	mem16[0x11e90c / 2] = 0x4e71;
	mem16[0x11e90e / 2] = 0x4e71;

	mem16[0x11e91a / 2] = 0x4e71;

	mem16[0x11eaf6 / 2] = 0x4e71;
	mem16[0x11eaf8 / 2] = 0x4e71;

	mem16[0x11eb04 / 2] = 0x4e71;

	/* patch ARM area with fake code */
	UINT16 *temp16 = (UINT16 *)state->memregion("prot")->base();
	temp16[(0x0000)/2] = 0xd088;
	temp16[(0x0002)/2] = 0xe59f;
	temp16[(0x0004)/2] = 0x0680;
	temp16[(0x0006)/2] = 0xe3a0;
	temp16[(0x0008)/2] = 0x0001;
	temp16[(0x000a)/2] = 0xe280;
	temp16[(0x000c)/2] = 0xff10;
	temp16[(0x000e)/2] = 0xe12f;

	temp16[(0x0090)/2] = 0x0400;
	temp16[(0x0092)/2] = 0x1000;
}

DRIVER_INIT( dwpc )
{
	pgm_arm_type2_state *state = machine.driver_data<pgm_arm_type2_state>();
	UINT16 *mem16 = (UINT16 *)state->memregion("maincpu")->base();

	pgm_basic_init(machine);
	kov2_latch_init(machine);
	pgm_mm_decrypt(machine); // encryption is the same as martial masters

	mem16[0x11EDDA / 2] = 0x4e71;
	mem16[0x11EDDC / 2] = 0x4e71;

	mem16[0x11EDE8 / 2] = 0x4e71;

	mem16[0x11EFC4 / 2] = 0x4e71;
	mem16[0x11EFC6 / 2] = 0x4e71;

	mem16[0x11EFD2 / 2] = 0x4e71;


	/* patch ARM area with fake code */
	UINT16 *temp16 = (UINT16 *)state->memregion("prot")->base();
	temp16[(0x0000)/2] = 0xd088;
	temp16[(0x0002)/2] = 0xe59f;
	temp16[(0x0004)/2] = 0x0680;
	temp16[(0x0006)/2] = 0xe3a0;
	temp16[(0x0008)/2] = 0x0001;
	temp16[(0x000a)/2] = 0xe280;
	temp16[(0x000c)/2] = 0xff10;
	temp16[(0x000e)/2] = 0xe12f;

	temp16[(0x0090)/2] = 0x0400;
	temp16[(0x0092)/2] = 0x1000;

}

INPUT_PORTS_START( ddp2 )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")	/* Region - actually supplied by protection device */
	PORT_CONFNAME( 0x00ff, 0x00ff, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Cave license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
	PORT_CONFSETTING(      0x00ff, "Untouched" ) // don't hack the region
INPUT_PORTS_END

INPUT_PORTS_START( kov2 )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")	/* Region - actually supplied by protection device */
	PORT_CONFNAME( 0x00ff, 0x00ff, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Cave license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
	PORT_CONFSETTING(      0x00ff, "Untouched" ) // don't hack the region
INPUT_PORTS_END

INPUT_PORTS_START( martmast )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")	/* Region - actually supplied by protection device */
	PORT_CONFNAME( 0x00ff, 0x00ff, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, DEF_STR( Japan ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0006, DEF_STR( USA ) )
	PORT_CONFSETTING(      0x00ff, "Untouched" ) // don't hack the region
INPUT_PORTS_END


INPUT_PORTS_START( dw2001 )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0005, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END
