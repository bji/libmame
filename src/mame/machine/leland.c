/***************************************************************************

    Cinemat/Leland driver

    driver by Aaron Giles and Paul Leaman

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/eeprom.h"
#include "cpu/z80/z80.h"
#include "includes/leland.h"
#include "sound/ay8910.h"


/*************************************
 *
 *  Debugging
 *
 *************************************/

/* define these to 0 to disable, or to 1 to enable */
#define LOG_KEYCARDS		0
#define LOG_KEYCARDS_FULL	0
#define LOG_BANKSWITCHING_M	0
#define LOG_BANKSWITCHING_S	0
#define LOG_SOUNDPORT		0
#define LOG_EEPROM			0
#define LOG_BATTERY_RAM		0
#define LOG_XROM			0



/* Internal routines */
static TIMER_CALLBACK( leland_interrupt_callback );
static TIMER_CALLBACK( ataxx_interrupt_callback );



/*************************************
 *
 *  Generic dial encoding
 *
 *************************************/

static int dial_compute_value(running_machine &machine, int new_val, int indx)
{
	leland_state *state = machine.driver_data<leland_state>();
	int delta = new_val - (int)state->m_dial_last_input[indx];
	UINT8 result = state->m_dial_last_result[indx] & 0x80;

	state->m_dial_last_input[indx] = new_val;

	if (delta > 0x80)
		delta -= 0x100;
	else if (delta < -0x80)
		delta += 0x100;

	if (delta < 0)
	{
		result = 0x80;
		delta = -delta;
	}
	else if (delta > 0)
		result = 0x00;

	if (delta > 0x1f)
		delta = 0x1f;
	result |= (state->m_dial_last_result[indx] + delta) & 0x1f;

	state->m_dial_last_result[indx] = result;
	return result;
}



/*************************************
 *
 *  Cerberus inputs
 *
 *************************************/

READ8_HANDLER( cerberus_dial_1_r )
{
	int original = input_port_read(space->machine(), "IN0");
	int modified = dial_compute_value(space->machine(), input_port_read(space->machine(), "AN0"), 0);
	return (original & 0xc0) | ((modified & 0x80) >> 2) | (modified & 0x1f);
}


READ8_HANDLER( cerberus_dial_2_r )
{
	int original = input_port_read(space->machine(), "IN0");
	int modified = dial_compute_value(space->machine(), input_port_read(space->machine(), "AN1"), 1);
	return (original & 0xc0) | ((modified & 0x80) >> 2) | (modified & 0x1f);
}



/*************************************
 *
 *  Alley Master inputs
 *
 *************************************/


WRITE8_HANDLER( alleymas_joystick_kludge )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	/* catch the case where they clear this memory location at PC $1827 and change */
	/* the value written to be a 1 */
	if (cpu_get_previouspc(&space->device()) == 0x1827)
		*state->m_alleymas_kludge_mem = 1;
	else
		*state->m_alleymas_kludge_mem = data;

	/* while we're here, make sure the first 3 characters in battery RAM are a */
	/* valid name; otherwise, it will crash if you start a game and don't enter */
	/* your name */
	if (state->m_battery_ram[0] == 0)
	{
		state->m_battery_ram[0] = 'C';
		state->m_battery_ram[1] = 'I';
		state->m_battery_ram[2] = 'N';
	}
}



/*************************************
 *
 *  Danger Zone inputs
 *
 *************************************/

static void update_dangerz_xy(running_machine &machine)
{
	leland_state *state = machine.driver_data<leland_state>();
	UINT8 newy = input_port_read(machine, "AN0");
	UINT8 newx = input_port_read(machine, "AN1");
	int deltay = newy - state->m_dial_last_input[0];
	int deltax = newx - state->m_dial_last_input[1];

	if (deltay <= -128) deltay += 256;
	else if (deltay >= 128) deltay -= 256;
	if (deltax <= -128) deltax += 256;
	else if (deltax >= 128) deltax -= 256;

	state->m_dangerz_y += deltay;
	state->m_dangerz_x += deltax;
	if (state->m_dangerz_y < 0) state->m_dangerz_y = 0;
	else if (state->m_dangerz_y >= 1024) state->m_dangerz_y = 1023;
	if (state->m_dangerz_x < 0) state->m_dangerz_x = 0;
	else if (state->m_dangerz_x >= 1024) state->m_dangerz_x = 1023;

	state->m_dial_last_input[0] = newy;
	state->m_dial_last_input[1] = newx;
}


READ8_HANDLER( dangerz_input_y_r )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	update_dangerz_xy(space->machine());
	return state->m_dangerz_y & 0xff;
}


READ8_HANDLER( dangerz_input_x_r )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	update_dangerz_xy(space->machine());
	return state->m_dangerz_x & 0xff;
}


READ8_HANDLER( dangerz_input_upper_r )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	update_dangerz_xy(space->machine());
	return ((state->m_dangerz_y >> 2) & 0xc0) | ((state->m_dangerz_x >> 8) & 0x03);
}



/*************************************
 *
 *  Red Line Racer inputs
 *
 *************************************/

static const UINT8 redline_pedal_value[8] = { 0xf0, 0xe0, 0xc0, 0xd0, 0x90, 0xb0, 0x30, 0x70 };

READ8_HANDLER( redline_pedal_1_r )
{
	int pedal = input_port_read(space->machine(), "IN0");
	return redline_pedal_value[pedal >> 5] | 0x0f;
}


READ8_HANDLER( redline_pedal_2_r )
{
	int pedal = input_port_read(space->machine(), "IN2");
	return redline_pedal_value[pedal >> 5] | 0x0f;
}


READ8_HANDLER( redline_wheel_1_r )
{
	return dial_compute_value(space->machine(), input_port_read(space->machine(), "AN0"), 0);
}


READ8_HANDLER( redline_wheel_2_r )
{
	return dial_compute_value(space->machine(), input_port_read(space->machine(), "AN1"), 1);
}



/*************************************
 *
 *  Super Offroad inputs
 *
 *************************************/

READ8_HANDLER( offroad_wheel_1_r )
{
	return dial_compute_value(space->machine(), input_port_read(space->machine(), "AN3"), 0);
}


READ8_HANDLER( offroad_wheel_2_r )
{
	return dial_compute_value(space->machine(), input_port_read(space->machine(), "AN4"), 1);
}


READ8_HANDLER( offroad_wheel_3_r )
{
	return dial_compute_value(space->machine(), input_port_read(space->machine(), "AN5"), 2);
}



/*************************************
 *
 *  Ataxx inputs
 *
 *************************************/

READ8_HANDLER( ataxx_trackball_r )
{
	static const char *const tracknames[] = { "AN0", "AN1", "AN2", "AN3" };

	return dial_compute_value(space->machine(), input_port_read(space->machine(), tracknames[offset]), offset);
}



/*************************************
 *
 *  Indy Heat inputs
 *
 *************************************/

READ8_HANDLER( indyheat_wheel_r )
{
	static const char *const tracknames[] = { "AN0", "AN1", "AN2" };

	return dial_compute_value(space->machine(), input_port_read(space->machine(), tracknames[offset]), offset);
}


READ8_HANDLER( indyheat_analog_r )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	switch (offset)
	{
		case 0:
			return 0;

		case 1:
			return state->m_analog_result;

		case 2:
			return 0;

		case 3:
			logerror("Unexpected analog read(%02X)\n", 8 + offset);
			break;
	}
	return 0xff;
}


WRITE8_HANDLER( indyheat_analog_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	static const char *const tracknames[] = { "AN3", "AN4", "AN5" };

	switch (offset)
	{
		case 3:
			state->m_analog_result = input_port_read(space->machine(), tracknames[data]);
			break;

		case 0:
		case 1:
		case 2:
			logerror("Unexpected analog write(%02X) = %02X\n", 8 + offset, data);
			break;
	}
}



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

MACHINE_START( leland )
{
	leland_state *state = machine.driver_data<leland_state>();
	/* allocate extra stuff */
	state->m_battery_ram = reinterpret_cast<UINT8 *>(memory_get_shared(machine, "battery"));

	/* start scanline interrupts going */
	state->m_master_int_timer = machine.scheduler().timer_alloc(FUNC(leland_interrupt_callback));
}


MACHINE_RESET( leland )
{
	leland_state *state = machine.driver_data<leland_state>();
	state->m_master_int_timer->adjust(machine.primary_screen->time_until_pos(8), 8);

	/* reset globals */
	state->m_gfx_control = 0x00;
	leland_sound_port_w(machine.device("ay8910.1"), 0, 0xff);
	state->m_wcol_enable = 0;

	state->m_dangerz_x = 512;
	state->m_dangerz_y = 512;
	state->m_analog_result = 0xff;
	memset(state->m_dial_last_input, 0, sizeof(state->m_dial_last_input));
	memset(state->m_dial_last_result, 0, sizeof(state->m_dial_last_result));

	state->m_keycard_shift = 0;
	state->m_keycard_bit = 0;
	state->m_keycard_state = 0;
	state->m_keycard_clock = 0;
	memset(state->m_keycard_command, 0, sizeof(state->m_keycard_command));

	state->m_top_board_bank = 0;
	state->m_sound_port_bank = 0;
	state->m_alternate_bank = 0;

	/* initialize the master banks */
	state->m_master_length = machine.region("master")->bytes();
	state->m_master_base = machine.region("master")->base();
	(*state->m_update_master_bank)(machine);

	/* initialize the slave banks */
	state->m_slave_length = machine.region("slave")->bytes();
	state->m_slave_base = machine.region("slave")->base();
	if (state->m_slave_length > 0x10000)
		memory_set_bankptr(machine, "bank3", &state->m_slave_base[0x10000]);
}


MACHINE_START( ataxx )
{
	leland_state *state = machine.driver_data<leland_state>();
	/* set the odd data banks */
	state->m_battery_ram = reinterpret_cast<UINT8 *>(memory_get_shared(machine, "battery"));
	state->m_extra_tram = auto_alloc_array(machine, UINT8, ATAXX_EXTRA_TRAM_SIZE);

	/* start scanline interrupts going */
	state->m_master_int_timer = machine.scheduler().timer_alloc(FUNC(ataxx_interrupt_callback));
}


MACHINE_RESET( ataxx )
{
	leland_state *state = machine.driver_data<leland_state>();
	memset(state->m_extra_tram, 0, ATAXX_EXTRA_TRAM_SIZE);
	state->m_master_int_timer->adjust(machine.primary_screen->time_until_pos(8), 8);

	/* initialize the XROM */
	state->m_xrom_length = machine.region("user1")->bytes();
	state->m_xrom_base = machine.region("user1")->base();
	state->m_xrom1_addr = 0;
	state->m_xrom2_addr = 0;

	/* reset globals */
	state->m_wcol_enable = 0;

	state->m_analog_result = 0xff;
	memset(state->m_dial_last_input, 0, sizeof(state->m_dial_last_input));
	memset(state->m_dial_last_result, 0, sizeof(state->m_dial_last_result));

	state->m_master_bank = 0;

	/* initialize the master banks */
	state->m_master_length = machine.region("master")->bytes();
	state->m_master_base = machine.region("master")->base();
	ataxx_bankswitch(machine);

	/* initialize the slave banks */
	state->m_slave_length = machine.region("slave")->bytes();
	state->m_slave_base = machine.region("slave")->base();
	if (state->m_slave_length > 0x10000)
		memory_set_bankptr(machine, "bank3", &state->m_slave_base[0x10000]);
}



/*************************************
 *
 *  Master CPU interrupt handling
 *
 *************************************/

static TIMER_CALLBACK( leland_interrupt_callback )
{
	leland_state *state = machine.driver_data<leland_state>();
	int scanline = param;

	/* interrupts generated on the VA10 line, which is every */
	/* 16 scanlines starting with scanline #8 */
	cputag_set_input_line(machine, "master", 0, HOLD_LINE);

	/* set a timer for the next one */
	scanline += 16;
	if (scanline > 248)
		scanline = 8;
	state->m_master_int_timer->adjust(machine.primary_screen->time_until_pos(scanline), scanline);
}


static TIMER_CALLBACK( ataxx_interrupt_callback )
{
	leland_state *state = machine.driver_data<leland_state>();
	int scanline = param;

	/* interrupts generated according to the interrupt control register */
	cputag_set_input_line(machine, "master", 0, HOLD_LINE);

	/* set a timer for the next one */
	state->m_master_int_timer->adjust(machine.primary_screen->time_until_pos(scanline), scanline);
}


INTERRUPT_GEN( leland_master_interrupt )
{
	/* check for coins here */
	if ((input_port_read(device->machine(), "IN1") & 0x0e) != 0x0e)
		device_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE);
}



/*************************************
 *
 *  Master CPU bankswitch handlers
 *
 *************************************/

WRITE8_HANDLER( leland_master_alt_bankswitch_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	/* update any bankswitching */
	if (LOG_BANKSWITCHING_M)
		if ((state->m_alternate_bank ^ data) & 0x0f)
			logerror("%04X:alternate_bank = %02X\n", cpu_get_pc(&space->device()), data & 0x0f);
	state->m_alternate_bank = data & 15;
	(*state->m_update_master_bank)(space->machine());

	/* sound control is in the rest */
	leland_80186_control_w(space->machine().device("custom"), offset, data);
}


/* bankswitching for Cerberus */
void cerberus_bankswitch(running_machine &machine)
{
	/* no bankswitching */
}


/* bankswitching for Mayhem 2002, Power Play, World Series Baseball, and Alley Master */
void mayhem_bankswitch(running_machine &machine)
{
	leland_state *state = machine.driver_data<leland_state>();
	UINT8 *address;

	state->m_battery_ram_enable = ((state->m_sound_port_bank & 0x24) == 0);

	address = (!(state->m_sound_port_bank & 0x04)) ? &state->m_master_base[0x10000] : &state->m_master_base[0x1c000];
	memory_set_bankptr(machine, "bank1", address);

	address = state->m_battery_ram_enable ? state->m_battery_ram : &address[0x8000];
	memory_set_bankptr(machine, "bank2", address);
}


/* bankswitching for Danger Zone */
void dangerz_bankswitch(running_machine &machine)
{
	leland_state *state = machine.driver_data<leland_state>();
	UINT8 *address;

	state->m_battery_ram_enable = ((state->m_top_board_bank & 0x80) != 0);

	address = (!(state->m_alternate_bank & 1)) ? &state->m_master_base[0x02000] : &state->m_master_base[0x12000];
	memory_set_bankptr(machine, "bank1", address);

	address = state->m_battery_ram_enable ? state->m_battery_ram : &address[0x8000];
	memory_set_bankptr(machine, "bank2", address);
}


/* bankswitching for Baseball the Season II, Super Baseball, and Strike Zone */
void basebal2_bankswitch(running_machine &machine)
{
	leland_state *state = machine.driver_data<leland_state>();
	UINT8 *address;

	state->m_battery_ram_enable = (state->m_top_board_bank & 0x80);

	if (!state->m_battery_ram_enable)
		address = (!(state->m_sound_port_bank & 0x04)) ? &state->m_master_base[0x10000] : &state->m_master_base[0x1c000];
	else
		address = (!(state->m_top_board_bank & 0x40)) ? &state->m_master_base[0x28000] : &state->m_master_base[0x30000];
	memory_set_bankptr(machine, "bank1", address);

	address = state->m_battery_ram_enable ? state->m_battery_ram : &address[0x8000];
	memory_set_bankptr(machine, "bank2", address);
}


/* bankswitching for Red Line Racer */
void redline_bankswitch(running_machine &machine)
{
	leland_state *state = machine.driver_data<leland_state>();
	static const UINT32 bank_list[] = { 0x10000, 0x18000, 0x02000, 0x02000 };
	UINT8 *address;

	state->m_battery_ram_enable = ((state->m_alternate_bank & 3) == 1);

	address = &state->m_master_base[bank_list[state->m_alternate_bank & 3]];
	memory_set_bankptr(machine, "bank1", address);

	address = state->m_battery_ram_enable ? state->m_battery_ram : &state->m_master_base[0xa000];
	memory_set_bankptr(machine, "bank2", address);
}


/* bankswitching for Viper, Quarterback, Team Quarterback, and All American Football */
void viper_bankswitch(running_machine &machine)
{
	leland_state *state = machine.driver_data<leland_state>();
	static const UINT32 bank_list[] = { 0x02000, 0x10000, 0x18000, 0x02000 };
	UINT8 *address;

	state->m_battery_ram_enable = ((state->m_alternate_bank & 0x04) != 0);

	address = &state->m_master_base[bank_list[state->m_alternate_bank & 3]];
	if (bank_list[state->m_alternate_bank & 3] >= state->m_master_length)
	{
		logerror("%s:Master bank %02X out of range!\n", machine.describe_context(), state->m_alternate_bank & 3);
		address = &state->m_master_base[bank_list[0]];
	}
	memory_set_bankptr(machine, "bank1", address);

	address = state->m_battery_ram_enable ? state->m_battery_ram : &state->m_master_base[0xa000];
	memory_set_bankptr(machine, "bank2", address);
}


/* bankswitching for Super Offroad, Super Offroad Track Pack, and Pig Out */
void offroad_bankswitch(running_machine &machine)
{
	leland_state *state = machine.driver_data<leland_state>();
	static const UINT32 bank_list[] = { 0x02000, 0x02000, 0x10000, 0x18000, 0x20000, 0x28000, 0x30000, 0x38000 };
	UINT8 *address;

	state->m_battery_ram_enable = ((state->m_alternate_bank & 7) == 1);

	address = &state->m_master_base[bank_list[state->m_alternate_bank & 7]];
	if (bank_list[state->m_alternate_bank & 7] >= state->m_master_length)
	{
		logerror("%s:Master bank %02X out of range!\n", machine.describe_context(), state->m_alternate_bank & 7);
		address = &state->m_master_base[bank_list[0]];
	}
	memory_set_bankptr(machine, "bank1", address);

	address = state->m_battery_ram_enable ? state->m_battery_ram : &state->m_master_base[0xa000];
	memory_set_bankptr(machine, "bank2", address);
}


/* bankswitching for Ataxx, WSF, Indy Heat, and Brute Force */
void ataxx_bankswitch(running_machine &machine)
{
	leland_state *state = machine.driver_data<leland_state>();
	static const UINT32 bank_list[] =
	{
		0x02000, 0x18000, 0x20000, 0x28000, 0x30000, 0x38000, 0x40000, 0x48000,
		0x50000, 0x58000, 0x60000, 0x68000, 0x70000, 0x78000, 0x80000, 0x88000
	};
	UINT8 *address;

	state->m_battery_ram_enable = ((state->m_master_bank & 0x30) == 0x10);

	address = &state->m_master_base[bank_list[state->m_master_bank & 15]];
	if (bank_list[state->m_master_bank & 15] >= state->m_master_length)
	{
		logerror("%s:Master bank %02X out of range!\n", machine.describe_context(), state->m_master_bank & 15);
		address = &state->m_master_base[bank_list[0]];
	}
	memory_set_bankptr(machine, "bank1", address);

	if (state->m_battery_ram_enable)
		address = state->m_battery_ram;
	else if ((state->m_master_bank & 0x30) == 0x20)
		address = &state->m_ataxx_qram[(state->m_master_bank & 0xc0) << 8];
	else
		address = &state->m_master_base[0xa000];
	memory_set_bankptr(machine, "bank2", address);

	state->m_wcol_enable = ((state->m_master_bank & 0x30) == 0x30);
}



/*************************************
 *
 *  EEPROM handling (64 x 16bits)
 *
 *************************************/

void leland_init_eeprom(running_machine &machine, UINT8 default_val, const UINT16 *data, UINT8 serial_offset, UINT8 serial_type)
{
	UINT8 xorval = (serial_type == SERIAL_TYPE_ADD_XOR || serial_type == SERIAL_TYPE_ENCRYPT_XOR) ? 0xff : 0x00;
	UINT8 eeprom_data[64*2];
	UINT32 serial;

	/*
        NOTE: This code is just illustrative, and explains how to generate the
        serial numbers for the classic Leland games. We currently load default
        EEPROM data from the ROMs rather than generating it.

        Here are the input parameters for various games:

            game        default_val     serial_offset   serial_type
            cerberus    0x00            0               SERIAL_TYPE_NONE
            mayhem      0x00            0x28            SERIAL_TYPE_ADD
            powrplay    0xff            0x2d            SERIAL_TYPE_ADD_XOR
            wseries     0xff            0x12            SERIAL_TYPE_ENCRYPT_XOR
            alleymas    0xff            0x0c            SERIAL_TYPE_ENCRYPT_XOR
            upyoural    0xff            0x0c            SERIAL_TYPE_ENCRYPT_XOR
            dangerz     0xff            0x10            SERIAL_TYPE_ENCRYPT_XOR
            basebal2    0xff            0x12            SERIAL_TYPE_ENCRYPT_XOR
            dblplay     0xff            0x11            SERIAL_TYPE_ENCRYPT_XOR
            strkzone    0xff            0x0f            SERIAL_TYPE_ENCRYPT_XOR
            redlin2p    0xff            0x18            SERIAL_TYPE_ENCRYPT_XOR
            quarterb    0xff            0x24            SERIAL_TYPE_ENCRYPT_XOR
            viper       0xff            0x0c            SERIAL_TYPE_ENCRYPT_XOR
            teamqb      0xff            0x1a            SERIAL_TYPE_ENCRYPT_XOR
            aafb        0xff            0x1a            SERIAL_TYPE_ENCRYPT_XOR
            offroad     0xff            0x00            SERIAL_TYPE_ENCRYPT_XOR
            pigout      0xff            0x00            SERIAL_TYPE_ENCRYPT
    */

	/* initialize everything to the default value */
	memset(eeprom_data, default_val, sizeof(eeprom_data));

	/* fill in the preset data */
	while (*data != 0xffff)
	{
		int offset = *data++;
		int value = *data++;
		eeprom_data[offset * 2 + 0] = value >> 8;
		eeprom_data[offset * 2 + 1] = value & 0xff;
	}

	/* pick a serial number -- examples of real serial numbers:

        Team QB:      21101957
        AAFB:         26101119 and 26101039
    */
	serial = 0x12345678;

	/* switch off the serial number type */
	switch (serial_type)
	{
		case SERIAL_TYPE_ADD:
		case SERIAL_TYPE_ADD_XOR:
		{
			int i;
			for (i = 0; i < 10; i++)
			{
				int digit;

				if (i >= 8)
					digit = 0;
				else
					digit = ((serial << (i * 4)) >> 28) & 15;
				digit = ('0' + digit) * 2;

				eeprom_data[serial_offset * 2 +  0 + (i ^ 1)] = (digit / 3) ^ xorval;
				eeprom_data[serial_offset * 2 + 10 + (i ^ 1)] = (digit / 3) ^ xorval;
				eeprom_data[serial_offset * 2 + 20 + (i ^ 1)] = (digit - (2 * (digit / 3))) ^ xorval;
			}
			break;
		}

		case SERIAL_TYPE_ENCRYPT:
		case SERIAL_TYPE_ENCRYPT_XOR:
		{
			int d, e, h, l;

			/* break the serial number out into pieces */
			l = (serial >> 24) & 0xff;
			h = (serial >> 16) & 0xff;
			e = (serial >> 8) & 0xff;
			d = serial & 0xff;

			/* decrypt the data */
			h = ((h ^ 0x2a ^ l) ^ 0xff) + 5;
			d = ((d + 0x2a) ^ e) ^ 0xff;
			l ^= e;
			e ^= 0x2a;

			/* store the bytes */
			eeprom_data[serial_offset * 2 + 0] = h ^ xorval;
			eeprom_data[serial_offset * 2 + 1] = l ^ xorval;
			eeprom_data[serial_offset * 2 + 2] = d ^ xorval;
			eeprom_data[serial_offset * 2 + 3] = e ^ xorval;
			break;
		}
	}
}



/*************************************
 *
 *  EEPROM handling (128 x 16bits)
 *
 *************************************/

void ataxx_init_eeprom(running_machine &machine, const UINT16 *data)
{
	UINT8 eeprom_data[128*2];
	UINT8 serial_offset = 0;
	UINT8 default_val = 0;
	UINT32 serial;

	/*
        NOTE: This code is just illustrative, and explains how to generate the
        serial numbers for the classic Leland games. We currently load default
        EEPROM data from the ROMs rather than generating it.
    */

	/* initialize everything to the default value */
	memset(eeprom_data, default_val, sizeof(eeprom_data));

	/* fill in the preset data */
	while (*data != 0xffff)
	{
		int offset = *data++;
		int value = *data++;
		eeprom_data[offset * 2 + 0] = value >> 8;
		eeprom_data[offset * 2 + 1] = value & 0xff;
	}

	/* pick a serial number -- examples of real serial numbers:

        WSF:         30101190
        Indy Heat:   31201339
    */
	serial = 0x12345678;

	/* encrypt the serial number */
	{
		int d, e, h, l;

		/* break the serial number out into pieces */
		l = (serial >> 24) & 0xff;
		h = (serial >> 16) & 0xff;
		e = (serial >> 8) & 0xff;
		d = serial & 0xff;

		/* decrypt the data */
		h = ((h ^ 0x2a ^ l) ^ 0xff) + 5;
		d = ((d + 0x2a) ^ e) ^ 0xff;
		l ^= e;
		e ^= 0x2a;

		/* store the bytes */
		eeprom_data[serial_offset * 2 + 0] = h;
		eeprom_data[serial_offset * 2 + 1] = l;
		eeprom_data[serial_offset * 2 + 2] = d;
		eeprom_data[serial_offset * 2 + 3] = e;
	}

	/* compute the checksum */
	{
		int i, sum = 0;
		for (i = 0; i < 0x7f * 2; i++)
			sum += eeprom_data[i];
		sum ^= 0xffff;
		eeprom_data[0x7f * 2 + 0] = (sum >> 8) & 0xff;
		eeprom_data[0x7f * 2 + 1] = sum & 0xff;
	}
}



/*************************************
 *
 *  Ataxx EEPROM interfacing
 *
 *************************************/

READ8_DEVICE_HANDLER( ataxx_eeprom_r )
{
	int port = input_port_read(device->machine(), "IN2");
	if (LOG_EEPROM) logerror("%s:EE read\n", device->machine().describe_context());
	return port;
}


WRITE8_DEVICE_HANDLER( ataxx_eeprom_w )
{
	if (LOG_EEPROM) logerror("%s:EE write %d%d%d\n", device->machine().describe_context(),
			(data >> 6) & 1, (data >> 5) & 1, (data >> 4) & 1);
	eeprom_write_bit     (device, (data & 0x10) >> 4);
	eeprom_set_clock_line(device, (data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
	eeprom_set_cs_line   (device, (~data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Battery backed RAM
 *
 *************************************/

WRITE8_HANDLER( leland_battery_ram_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	if (state->m_battery_ram_enable)
	{
		if (LOG_BATTERY_RAM) logerror("%04X:BatteryW@%04X=%02X\n", cpu_get_pc(&space->device()), offset, data);
		state->m_battery_ram[offset] = data;
	}
	else
		logerror("%04X:BatteryW@%04X (invalid!)\n", cpu_get_pc(&space->device()), offset);
}


WRITE8_HANDLER( ataxx_battery_ram_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	if (state->m_battery_ram_enable)
	{
		if (LOG_BATTERY_RAM) logerror("%04X:BatteryW@%04X=%02X\n", cpu_get_pc(&space->device()), offset, data);
		state->m_battery_ram[offset] = data;
	}
	else if ((state->m_master_bank & 0x30) == 0x20)
		state->m_ataxx_qram[((state->m_master_bank & 0xc0) << 8) + offset] = data;
	else
		logerror("%04X:BatteryW@%04X (invalid!)\n", cpu_get_pc(&space->device()), offset);
}



/*************************************
 *
 *  Master CPU keycard I/O
 *
 ************************************/

/*--------------------------------------------------------------------

  A note about keycards:

  These were apparently static programmable ROMs that could be
  inserted into certain games. The games would then save/load
  statistics on them.

  The data is accessed via a serial protocol, which is
  implemented below. There are two known commands that are
  written; each command is 3 bytes and accesses 128 bytes of
  data from the keycard:

        62 00 80
        9D 00 80

  the last byte appears to specify the length of data to transfer.

  The format of the data on the card is pretty heavily encrypted.
  The first 7 bytes read serves as a header:

        D5 43 49 4E 2A xx yy

  where xx is a game-specific key, and yy is the complement of the
  key. For example, World Series Baseball uses 04/FB. Alley Master
  uses 05/FA.

  The last 112 bytes of data is encrypted via the following method:

        for (i = 16, b = 0x70, r = 0x08; i < 128; i++, b--, r += 0x10)
        {
            UINT8 a = original_data[i] ^ 0xff;
            a = (a >> 3) | (a << 5);
            a = (((a ^ r) + 1 + b) ^ b) - b;
            encrypted_data[i] = a;
        }

  The data that is encrypted is stored alternating with a checksum
  byte. The checksum for a value A is ((A ^ 0xa5) + 0x27) ^ 0x34.

--------------------------------------------------------------------*/

static int keycard_r(running_machine &machine)
{
	leland_state *state = machine.driver_data<leland_state>();
	int result = 0;

	if (LOG_KEYCARDS_FULL) logerror("  (%s:keycard_r)\n", machine.describe_context());

	/* if we have a valid keycard read state, we're reading from the keycard */
	if (state->m_keycard_state & 0x80)
	{
		/* clock in new data */
		if (state->m_keycard_bit == 1)
		{
			state->m_keycard_shift = 0xff;	/* no data, but this is where we would clock it in */
			if (LOG_KEYCARDS) logerror("  (clocked in %02X)\n", state->m_keycard_shift);
		}

		/* clock in the bit */
		result = (~state->m_keycard_shift & 1) << ((state->m_keycard_state >> 4) & 3);
		if (LOG_KEYCARDS) logerror("  (read %02X)\n", result);
	}
	return result;
}

static void keycard_w(running_machine &machine, int data)
{
	leland_state *state = machine.driver_data<leland_state>();
	int new_state = data & 0xb0;
	int new_clock = data & 0x40;

	if (LOG_KEYCARDS_FULL) logerror("  (%s:keycard_w=%02X)\n", machine.describe_context(), data);

	/* check for going active */
	if (!state->m_keycard_state && new_state)
	{
		state->m_keycard_command[0] = state->m_keycard_command[1] = state->m_keycard_command[2] = 0;
		if (LOG_KEYCARDS) logerror("keycard going active (state=%02X)\n", new_state);
	}

	/* check for going inactive */
	else if (state->m_keycard_state && !new_state)
	{
		state->m_keycard_command[0] = state->m_keycard_command[1] = state->m_keycard_command[2] = 0;
		if (LOG_KEYCARDS) logerror("keycard going inactive\n");
	}

	/* check for clocks */
	else if (state->m_keycard_state == new_state)
	{
		/* work off of falling edge */
		if (!new_clock && state->m_keycard_clock)
		{
			state->m_keycard_shift >>= 1;
			state->m_keycard_bit = (state->m_keycard_bit + 1) & 7;
		}

		/* look for a bit write */
		else if (!new_clock && !state->m_keycard_clock && !(data & 0x80))
		{
			if (LOG_KEYCARDS) logerror("  (write %02X)\n", data);

			state->m_keycard_shift &= ~0x80;
			if (data & (1 << ((new_state >> 4) & 3)))
				state->m_keycard_shift |= 0x80;

			/* clock out the data on the last bit */
			if (state->m_keycard_bit == 7)
			{
				if (LOG_KEYCARDS) logerror("  (clocked out %02X)\n", state->m_keycard_shift);
				state->m_keycard_command[0] = state->m_keycard_command[1];
				state->m_keycard_command[1] = state->m_keycard_command[2];
				state->m_keycard_command[2] = state->m_keycard_shift;
				if (state->m_keycard_command[0] == 0x62 && state->m_keycard_command[1] == 0x00 && state->m_keycard_command[2] == 0x80)
				{
					if (LOG_KEYCARDS) logerror("  (got command $62)\n");
				}
			}
		}
	}

	/* error case */
	else
	{
		/* only an error if the selected bit changes; read/write transitions are okay */
		if ((new_state & 0x30) != (state->m_keycard_state & 0x30))
			if (LOG_KEYCARDS) logerror("ERROR: Caught keycard state transition %02X -> %02X\n", state->m_keycard_state, new_state);
	}

	state->m_keycard_state = new_state;
	state->m_keycard_clock = new_clock;
}



/*************************************
 *
 *  Master CPU analog and keycard I/O
 *
 *************************************/

READ8_HANDLER( leland_master_analog_key_r )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	int result = 0;

	switch (offset)
	{
		case 0x00:	/* FD = analog data read */
			result = state->m_analog_result;
			break;

		case 0x01:	/* FE = analog status read */
			/* bit 7 indicates the analog input is busy for some games */
			result = 0x00;
			break;

		case 0x02:	/* FF = keycard serial data read */
			result = keycard_r(space->machine());

			/* bit 7 indicates the analog input is busy for some games */
			result &= ~0x80;
			break;
	}
	return result;
}



WRITE8_HANDLER( leland_master_analog_key_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	static const char *const portnames[] = { "AN0", "AN1", "AN2", "AN3", "AN4", "AN5" };

	switch (offset)
	{
		case 0x00:	/* FD = analog port trigger */
			break;

		case 0x01:	/* FE = analog port select/bankswitch */
			state->m_analog_result = input_port_read(space->machine(), portnames[data & 15]);

			/* update top board banking for some games */
			if (LOG_BANKSWITCHING_M)
				if ((state->m_top_board_bank ^ data) & 0xc0)
					logerror("%04X:top_board_bank = %02X\n", cpu_get_pc(&space->device()), data & 0xc0);
			state->m_top_board_bank = data & 0xc0;
			(*state->m_update_master_bank)(space->machine());
			break;

		case 0x02:	/* FF = keycard data write */
			keycard_w(space->machine(), data);
			break;
	}
}



/*************************************
 *
 *  Master CPU internal I/O
 *
 *************************************/

READ8_HANDLER( leland_master_input_r )
{
	int result = 0xff;

	switch (offset)
	{
		case 0x00:	/* /GIN0 */
			result = input_port_read(space->machine(), "IN0");
			break;

		case 0x01:	/* /GIN1 */
			result = input_port_read(space->machine(), "IN1");
			if (cpu_get_reg(space->machine().device("slave"), Z80_HALT))
				result ^= 0x01;
			break;

		case 0x02:	/* /GIN2 */
		case 0x12:
			cputag_set_input_line(space->machine(), "master", INPUT_LINE_NMI, CLEAR_LINE);
			break;

		case 0x03:	/* /IGID */
		case 0x13:
			result = ay8910_r(space->machine().device("ay8910.1"), offset);
			break;

		case 0x10:	/* /GIN0 */
			result = input_port_read(space->machine(), "IN2");
			break;

		case 0x11:	/* /GIN1 */
			result = input_port_read(space->machine(), "IN3");
			if (LOG_EEPROM) logerror("%04X:EE read\n", cpu_get_pc(&space->device()));
			break;

		default:
			logerror("Master I/O read offset %02X\n", offset);
			break;
	}
	return result;
}


WRITE8_HANDLER( leland_master_output_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	device_t *eeprom;

	switch (offset)
	{
		case 0x09:	/* /MCONT */
			cputag_set_input_line(space->machine(), "slave", INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
			state->m_wcol_enable = (data & 0x02);
			cputag_set_input_line(space->machine(), "slave", INPUT_LINE_NMI, (data & 0x04) ? CLEAR_LINE : ASSERT_LINE);
			cputag_set_input_line(space->machine(), "slave", 0, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);

			eeprom = space->machine().device("eeprom");
			if (LOG_EEPROM) logerror("%04X:EE write %d%d%d\n", cpu_get_pc(&space->device()),
					(data >> 6) & 1, (data >> 5) & 1, (data >> 4) & 1);
			eeprom_write_bit     (eeprom, (data & 0x10) >> 4);
			eeprom_set_clock_line(eeprom, (data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
			eeprom_set_cs_line   (eeprom, (~data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
			break;

		case 0x0a:	/* /OGIA */
		case 0x0b:	/* /OGID */
			ay8910_address_data_w(space->machine().device("ay8910.1"), offset, data);
			break;

		case 0x0c:	/* /BKXL */
		case 0x0d:	/* /BKXH */
		case 0x0e:	/* /BKYL */
		case 0x0f:	/* /BKYH */
			leland_scroll_w(space, offset - 0x0c, data);
			break;

		default:
			logerror("Master I/O write offset %02X=%02X\n", offset, data);
			break;
	}
}


READ8_HANDLER( ataxx_master_input_r )
{
	int result = 0xff;

	switch (offset)
	{
		case 0x06:	/* /GIN0 */
			result = input_port_read(space->machine(), "IN0");
			break;

		case 0x07:	/* /SLVBLK */
			result = input_port_read(space->machine(), "IN1");
			if (cpu_get_reg(space->machine().device("slave"), Z80_HALT))
				result ^= 0x01;
			break;

		default:
			logerror("Master I/O read offset %02X\n", offset);
			break;
	}
	return result;
}


WRITE8_HANDLER( ataxx_master_output_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	switch (offset)
	{
		case 0x00:	/* /BKXL */
		case 0x01:	/* /BKXH */
		case 0x02:	/* /BKYL */
		case 0x03:	/* /BKYH */
			leland_scroll_w(space, offset, data);
			break;

		case 0x04:	/* /MBNK */
			if (LOG_BANKSWITCHING_M)
				if ((state->m_master_bank ^ data) & 0xff)
					logerror("%04X:master_bank = %02X\n", cpu_get_pc(&space->device()), data & 0xff);
			state->m_master_bank = data;
			ataxx_bankswitch(space->machine());
			break;

		case 0x05:	/* /SLV0 */
			cputag_set_input_line(space->machine(), "slave", 0, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
			cputag_set_input_line(space->machine(), "slave", INPUT_LINE_NMI, (data & 0x04) ? CLEAR_LINE : ASSERT_LINE);
			cputag_set_input_line(space->machine(), "slave", INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 0x08:	/*  */
			state->m_master_int_timer->adjust(space->machine().primary_screen->time_until_pos(data + 1), data + 1);
			break;

		default:
			logerror("Master I/O write offset %02X=%02X\n", offset, data);
			break;
	}
}



/*************************************
 *
 *  Master CPU palette gates
 *
 *************************************/

WRITE8_HANDLER( leland_gated_paletteram_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	if (state->m_wcol_enable)
		paletteram_BBGGGRRR_w(space, offset, data);
}


READ8_HANDLER( leland_gated_paletteram_r )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	if (state->m_wcol_enable)
		return space->machine().generic.paletteram.u8[offset];
	return 0xff;
}


WRITE8_HANDLER( ataxx_paletteram_and_misc_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	if (state->m_wcol_enable)
		paletteram_xxxxRRRRGGGGBBBB_le_w(space, offset, data);
	else if (offset == 0x7f8 || offset == 0x7f9)
		leland_master_video_addr_w(space, offset - 0x7f8, data);
	else if (offset == 0x7fc)
	{
		state->m_xrom1_addr = (state->m_xrom1_addr & 0xff00) | (data & 0x00ff);
		if (LOG_XROM) logerror("%04X:XROM1 address low write = %02X (addr=%04X)\n", cpu_get_pc(&space->device()), data, state->m_xrom1_addr);
	}
	else if (offset == 0x7fd)
	{
		state->m_xrom1_addr = (state->m_xrom1_addr & 0x00ff) | ((data << 8) & 0xff00);
		if (LOG_XROM) logerror("%04X:XROM1 address high write = %02X (addr=%04X)\n", cpu_get_pc(&space->device()), data, state->m_xrom1_addr);
	}
	else if (offset == 0x7fe)
	{
		state->m_xrom2_addr = (state->m_xrom2_addr & 0xff00) | (data & 0x00ff);
		if (LOG_XROM) logerror("%04X:XROM2 address low write = %02X (addr=%04X)\n", cpu_get_pc(&space->device()), data, state->m_xrom2_addr);
	}
	else if (offset == 0x7ff)
	{
		state->m_xrom2_addr = (state->m_xrom2_addr & 0x00ff) | ((data << 8) & 0xff00);
		if (LOG_XROM) logerror("%04X:XROM2 address high write = %02X (addr=%04X)\n", cpu_get_pc(&space->device()), data, state->m_xrom2_addr);
	}
	else
		state->m_extra_tram[offset] = data;
}


READ8_HANDLER( ataxx_paletteram_and_misc_r )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	if (state->m_wcol_enable)
		return space->machine().generic.paletteram.u8[offset];
	else if (offset == 0x7fc || offset == 0x7fd)
	{
		int result = state->m_xrom_base[0x00000 | state->m_xrom1_addr | ((offset & 1) << 16)];
		if (LOG_XROM) logerror("%04X:XROM1 read(%d) = %02X (addr=%04X)\n", cpu_get_pc(&space->device()), offset - 0x7fc, result, state->m_xrom1_addr);
		return result;
	}
	else if (offset == 0x7fe || offset == 0x7ff)
	{
		int result = state->m_xrom_base[0x20000 | state->m_xrom2_addr | ((offset & 1) << 16)];
		if (LOG_XROM) logerror("%04X:XROM2 read(%d) = %02X (addr=%04X)\n", cpu_get_pc(&space->device()), offset - 0x7fc, result, state->m_xrom2_addr);
		return result;
	}
	else
		return state->m_extra_tram[offset];
}



/*************************************
 *
 *  AY8910-controlled graphics latch
 *
 *************************************/

READ8_DEVICE_HANDLER( leland_sound_port_r )
{
	leland_state *state = device->machine().driver_data<leland_state>();
	return state->m_gfx_control;
}


WRITE8_DEVICE_HANDLER( leland_sound_port_w )
{
	leland_state *state = device->machine().driver_data<leland_state>();
	/* update the graphics banking */
	leland_gfx_port_w(device, 0, data);

	/* set the new value */
	state->m_gfx_control = data;
	state->m_dac_control = data & 3;

	/* some bankswitching occurs here */
	if (LOG_BANKSWITCHING_M)
		if ((state->m_sound_port_bank ^ data) & 0x24)
			logerror("%s:sound_port_bank = %02X\n", device->machine().describe_context(), data & 0x24);
	state->m_sound_port_bank = data & 0x24;
	(*state->m_update_master_bank)(device->machine());
}



/*************************************
 *
 *  Slave CPU bankswitching
 *
 *************************************/

WRITE8_HANDLER( leland_slave_small_banksw_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	int bankaddress = 0x10000 + 0xc000 * (data & 1);

	if (bankaddress >= state->m_slave_length)
	{
		logerror("%04X:Slave bank %02X out of range!", cpu_get_pc(&space->device()), data & 1);
		bankaddress = 0x10000;
	}
	memory_set_bankptr(space->machine(), "bank3", &state->m_slave_base[bankaddress]);

	if (LOG_BANKSWITCHING_S) logerror("%04X:Slave bank = %02X (%05X)\n", cpu_get_pc(&space->device()), data & 1, bankaddress);
}


WRITE8_HANDLER( leland_slave_large_banksw_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	int bankaddress = 0x10000 + 0x8000 * (data & 15);

	if (bankaddress >= state->m_slave_length)
	{
		logerror("%04X:Slave bank %02X out of range!", cpu_get_pc(&space->device()), data & 15);
		bankaddress = 0x10000;
	}
	memory_set_bankptr(space->machine(), "bank3", &state->m_slave_base[bankaddress]);

	if (LOG_BANKSWITCHING_S) logerror("%04X:Slave bank = %02X (%05X)\n", cpu_get_pc(&space->device()), data & 15, bankaddress);
}


WRITE8_HANDLER( ataxx_slave_banksw_w )
{
	leland_state *state = space->machine().driver_data<leland_state>();
	int bankaddress, bank = data & 15;

	if (bank == 0)
		bankaddress = 0x2000;
	else
	{
		bankaddress = 0x10000 * bank + 0x8000 * ((data >> 4) & 1);
		if (state->m_slave_length > 0x100000)
			bankaddress += 0x100000 * ((data >> 5) & 1);
	}

	if (bankaddress >= state->m_slave_length)
	{
		logerror("%04X:Slave bank %02X out of range!", cpu_get_pc(&space->device()), data & 0x3f);
		bankaddress = 0x2000;
	}
	memory_set_bankptr(space->machine(), "bank3", &state->m_slave_base[bankaddress]);

	if (LOG_BANKSWITCHING_S) logerror("%04X:Slave bank = %02X (%05X)\n", cpu_get_pc(&space->device()), data, bankaddress);
}



/*************************************
 *
 *  Slave CPU I/O
 *
 *************************************/

READ8_HANDLER( leland_raster_r )
{
	return space->machine().primary_screen->vpos();
}



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

/* also called by Ataxx */
void leland_rotate_memory(running_machine &machine, const char *cpuname)
{
	int startaddr = 0x10000;
	int banks = (machine.region(cpuname)->bytes() - startaddr) / 0x8000;
	UINT8 *ram = machine.region(cpuname)->base();
	UINT8 temp[0x2000];
	int i;

	for (i = 0; i < banks; i++)
	{
		memmove(temp, &ram[startaddr + 0x0000], 0x2000);
		memmove(&ram[startaddr + 0x0000], &ram[startaddr + 0x2000], 0x6000);
		memmove(&ram[startaddr + 0x6000], temp, 0x2000);
		startaddr += 0x8000;
	}
}
