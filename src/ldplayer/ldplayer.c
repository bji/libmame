/*************************************************************************

    ldplayer.c

    Laserdisc player driver.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "uimenu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/ldpr8210.h"
#include "machine/ldv1000.h"
#include <ctype.h>

#include "pr8210.lh"



/*************************************
 *
 *  Constants
 *
 *************************************/

/*************************************
 *
 *  Globals
 *
 *************************************/

class ldplayer_state : public driver_device
{
public:
	// construction/destruction
	ldplayer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_last_controls(0),
		  m_playing(false),
		  m_laserdisc(*this, "laserdisc") { }

	// callback hook
	static chd_file *get_disc_static(device_t *device) { return device->machine().driver_data<ldplayer_state>()->get_disc(); }

protected:
	// device overrides
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void machine_start();
	virtual void machine_reset();

	// internal helpers
	chd_file *get_disc();
	void process_commands();

	// derived classes
	virtual void execute_command(int command) { assert(false); }

	// timer IDs
	enum
	{
		TIMER_ID_AUTOPLAY,
		TIMER_ID_VSYNC_UPDATE
	};

	// commands
	enum
	{
		CMD_SCAN_REVERSE,
		CMD_STEP_REVERSE,
		CMD_SLOW_REVERSE,
		CMD_FAST_REVERSE,
		CMD_SCAN_FORWARD,
		CMD_STEP_FORWARD,
		CMD_SLOW_FORWARD,
		CMD_FAST_FORWARD,
		CMD_PLAY,
		CMD_PAUSE,
		CMD_FRAME_TOGGLE,
		CMD_CHAPTER_TOGGLE,
		CMD_CH1_TOGGLE,
		CMD_CH2_TOGGLE,
		CMD_0,
		CMD_1,
		CMD_2,
		CMD_3,
		CMD_4,
		CMD_5,
		CMD_6,
		CMD_7,
		CMD_8,
		CMD_9,
		CMD_SEARCH
	};

	// internal state
	astring m_filename;
	input_port_value m_last_controls;
	bool m_playing;
	required_device<laserdisc_device> m_laserdisc;
};


class pr8210_state : public ldplayer_state
{
public:
	// construction/destruction
	pr8210_state(const machine_config &mconfig, device_type type, const char *tag)
		: ldplayer_state(machine, config),
		  m_bit_timer(timer_alloc(TIMER_ID_BIT)),
		  m_command_buffer_in(0),
		  m_command_buffer_out(0) { }

protected:
	// device overrides
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void machine_reset();

	// command execution hook
	virtual void execute_command(int command);

	// internal helpers
	inline void add_command(UINT8 command);

	// timer IDs
	enum
	{
		TIMER_ID_BIT = 100,
		TIMER_ID_BIT_OFF
	};

	// internal state
	emu_timer *m_bit_timer;
	UINT32 m_command_buffer_in;
	UINT32 m_command_buffer_out;
	UINT8 m_command_buffer[10];
};


class ldv1000_state : public ldplayer_state
{
public:
	// construction/destruction
	ldv1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: ldplayer_state(machine, config) { }

protected:
	// command execution hook
	virtual void execute_command(int command);
};



/*************************************
 *
 *  Disc location
 *
 *************************************/

chd_file *ldplayer_state::get_disc()
{
	// open a path to the ROMs and find the first CHD file
	file_enumerator path(machine().options().media_path());

	// iterate while we get new objects
	const osd_directory_entry *dir;
	emu_file *image_file = NULL;
	chd_file *image_chd = NULL;
	while ((dir = path.next()) != NULL)
	{
		int length = strlen(dir->name);

		// look for files ending in .chd
		if (length > 4 &&
			dir->name[length - 4] == '.' &&
			tolower(dir->name[length - 3]) == 'c' &&
			tolower(dir->name[length - 2]) == 'h' &&
			tolower(dir->name[length - 1]) == 'd')
		{
			// open the file itself via our search path
			image_file = auto_alloc(machine(), emu_file(machine().options().media_path(), OPEN_FLAG_READ));
			file_error filerr = image_file->open(dir->name);
			if (filerr == FILERR_NONE)
			{
				// try to open the CHD
				chd_error chderr = chd_open_file(*image_file, CHD_OPEN_READ, NULL, &image_chd);
				if (chderr == CHDERR_NONE)
				{
					set_disk_handle(machine(), "laserdisc", *image_file, *image_chd);
					m_filename.cpy(dir->name);
					break;
				}
			}

			// close the file on failure
			auto_free(machine(), image_file);
			image_file = NULL;
		}
	}

	// if we failed, pop a message and exit
	if (image_file == NULL)
		throw emu_fatalerror("No valid image file found!\n");

	return get_disk_handle(machine(), "laserdisc");
}



/*************************************
 *
 *  Timers and sync
 *
 *************************************/

void ldplayer_state::process_commands()
{
	input_port_value controls = input_port_read(machine(), "controls");
	int number;

	// step backwards
	if (!(m_last_controls & 0x01) && (controls & 0x01))
		execute_command(CMD_STEP_REVERSE);

	// step forwards
	if (!(m_last_controls & 0x02) && (controls & 0x02))
		execute_command(CMD_STEP_FORWARD);

	// scan backwards
	if (controls & 0x04)
		execute_command(CMD_SCAN_REVERSE);

	// scan forwards
	if (controls & 0x08)
		execute_command(CMD_SCAN_FORWARD);

	// slow backwards
	if (!(m_last_controls & 0x10) && (controls & 0x10))
		execute_command(CMD_SLOW_REVERSE);

	// slow forwards
	if (!(m_last_controls & 0x20) && (controls & 0x20))
		execute_command(CMD_SLOW_FORWARD);

	// fast backwards
	if (controls & 0x40)
		execute_command(CMD_FAST_REVERSE);

	// fast forwards
	if (controls & 0x80)
		execute_command(CMD_FAST_FORWARD);

	// play/pause
	if (!(m_last_controls & 0x100) && (controls & 0x100))
	{
		m_playing = !m_playing;
		execute_command(m_playing ? CMD_PLAY : CMD_PAUSE);
	}

	// toggle frame display
	if (!(m_last_controls & 0x200) && (controls & 0x200))
		execute_command(CMD_FRAME_TOGGLE);

	// toggle chapter display
	if (!(m_last_controls & 0x400) && (controls & 0x400))
		execute_command(CMD_CHAPTER_TOGGLE);

	// toggle left channel
	if (!(m_last_controls & 0x800) && (controls & 0x800))
		execute_command(CMD_CH1_TOGGLE);

	// toggle right channel
	if (!(m_last_controls & 0x1000) && (controls & 0x1000))
		execute_command(CMD_CH2_TOGGLE);

	// numbers
	for (number = 0; number < 10; number++)
		if (!(m_last_controls & (0x10000 << number)) && (controls & (0x10000 << number)))
			execute_command(CMD_0 + number);

	// enter
	if (!(m_last_controls & 0x4000000) && (controls & 0x4000000))
		execute_command(CMD_SEARCH);

	m_last_controls = controls;
}


void ldplayer_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_ID_VSYNC_UPDATE:
		{
			// handle commands
			if (param == 0)
				process_commands();

			// set a timer to go off on the next VBLANK
			int vblank_scanline = machine().primary_screen->visible_area().max_y + 1;
			attotime target = machine().primary_screen->time_until_pos(vblank_scanline);
			timer_set(target, TIMER_ID_VSYNC_UPDATE);
			break;
		}

		case TIMER_ID_AUTOPLAY:
			// start playing
			execute_command(CMD_PLAY);
			m_playing = true;
			break;
	}
}


void ldplayer_state::machine_start()
{
	// start the vsync timer going
	timer_set(attotime::zero, TIMER_ID_VSYNC_UPDATE, 1);
}


void ldplayer_state::machine_reset()
{
	// set up a timer to start playing immediately
	timer_set(attotime::zero, TIMER_ID_AUTOPLAY);

	// indicate the name of the file we opened
	popmessage("Opened %s\n", m_filename.cstr());
}



/*************************************
 *
 *  PR-8210 implementation
 *
 *************************************/

void pr8210_state::add_command(UINT8 command)
{
	m_command_buffer[m_command_buffer_in++ % ARRAY_LENGTH(m_command_buffer)] = (command & 0x1f) | 0x20;
	m_command_buffer[m_command_buffer_in++ % ARRAY_LENGTH(m_command_buffer)] = 0x00 | 0x20;
}


void pr8210_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_ID_BIT:
		{
			attotime duration = attotime::from_msec(30);
			UINT8 bitsleft = param >> 16;
			UINT8 data = param;

			// if we have bits, process
			if (bitsleft != 0)
			{
				// assert the line and set a timer for deassertion
				laserdisc_line_w(m_laserdisc, LASERDISC_LINE_CONTROL, ASSERT_LINE);
				timer_set(attotime::from_usec(250), TIMER_ID_BIT_OFF);

				// space 0 bits apart by 1msec, and 1 bits by 2msec
				duration = attotime::from_msec((data & 0x80) ? 2 : 1);
				data <<= 1;
				bitsleft--;
			}

			// if we're out of bits, queue up the next command
			else if (bitsleft == 0 && m_command_buffer_in != m_command_buffer_out)
			{
				data = m_command_buffer[m_command_buffer_out++ % ARRAY_LENGTH(m_command_buffer)];
				bitsleft = 12;
			}
			m_bit_timer->adjust(duration, (bitsleft << 16) | data);
			break;
		}

		// deassert the control line
		case TIMER_ID_BIT_OFF:
			laserdisc_line_w(m_laserdisc, LASERDISC_LINE_CONTROL, CLEAR_LINE);
			break;

		// others to the parent class
		default:
			ldplayer_state::device_timer(timer, id, param, ptr);
			break;
	}
}


void pr8210_state::machine_reset()
{
	ldplayer_state::machine_reset();
	m_bit_timer->adjust(attotime::zero);
}


void pr8210_state::execute_command(int command)
{
	static const UINT8 digits[10] = { 0x01, 0x11, 0x09, 0x19, 0x05, 0x15, 0x0d, 0x1d, 0x03, 0x13 };

	switch (command)
	{
		case CMD_SCAN_REVERSE:
			if (m_command_buffer_in == m_command_buffer_out ||
				m_command_buffer_in == (m_command_buffer_out + 1) % ARRAY_LENGTH(m_command_buffer))
			{
				add_command(0x1c);
				m_playing = true;
			}
			break;

		case CMD_STEP_REVERSE:
			add_command(0x12);
			m_playing = false;
			break;

		case CMD_SLOW_REVERSE:
			add_command(0x02);
			m_playing = true;
			break;

		case CMD_FAST_REVERSE:
			if (m_command_buffer_in == m_command_buffer_out ||
				m_command_buffer_in == (m_command_buffer_out + 1) % ARRAY_LENGTH(m_command_buffer))
			{
				add_command(0x0c);
				m_playing = true;
			}
			break;

		case CMD_SCAN_FORWARD:
			if (m_command_buffer_in == m_command_buffer_out ||
				m_command_buffer_in == (m_command_buffer_out + 1) % ARRAY_LENGTH(m_command_buffer))
			{
				add_command(0x08);
				m_playing = true;
			}
			break;

		case CMD_STEP_FORWARD:
			add_command(0x04);
			m_playing = false;
			break;

		case CMD_SLOW_FORWARD:
			add_command(0x18);
			m_playing = true;
			break;

		case CMD_FAST_FORWARD:
			if (m_command_buffer_in == m_command_buffer_out ||
				m_command_buffer_in == (m_command_buffer_out + 1) % ARRAY_LENGTH(m_command_buffer))
			{
				add_command(0x10);
				m_playing = true;
			}
			break;

		case CMD_PLAY:
			add_command(0x14);
			m_playing = true;
			break;

		case CMD_PAUSE:
			add_command(0x0a);
			m_playing = false;
			break;

		case CMD_FRAME_TOGGLE:
			add_command(0x0b);
			break;

		case CMD_CHAPTER_TOGGLE:
			add_command(0x06);
			break;

		case CMD_CH1_TOGGLE:
			add_command(0x0e);
			break;

		case CMD_CH2_TOGGLE:
			add_command(0x16);
			break;

		case CMD_0:
		case CMD_1:
		case CMD_2:
		case CMD_3:
		case CMD_4:
		case CMD_5:
		case CMD_6:
		case CMD_7:
		case CMD_8:
		case CMD_9:
			add_command(digits[command - CMD_0]);
			break;

		case CMD_SEARCH:
			add_command(0x1a);
			m_playing = false;
			break;
	}
}



/*************************************
 *
 *  LD-V1000 implementation
 *
 *************************************/

void ldv1000_state::execute_command(int command)
{
	static const UINT8 digits[10] = { 0x3f, 0x0f, 0x8f, 0x4f, 0x2f, 0xaf, 0x6f, 0x1f, 0x9f, 0x5f };
	switch (command)
	{
		case CMD_SCAN_REVERSE:
			laserdisc_data_w(m_laserdisc, 0xf8);
			m_playing = true;
			break;

		case CMD_STEP_REVERSE:
			laserdisc_data_w(m_laserdisc, 0xfe);
			m_playing = false;
			break;

		case CMD_SCAN_FORWARD:
			laserdisc_data_w(m_laserdisc, 0xf0);
			m_playing = true;
			break;

		case CMD_STEP_FORWARD:
			laserdisc_data_w(m_laserdisc, 0xf6);
			m_playing = false;
			break;

		case CMD_PLAY:
			laserdisc_data_w(m_laserdisc, 0xfd);
			m_playing = true;
			break;

		case CMD_PAUSE:
			laserdisc_data_w(m_laserdisc, 0xa0);
			m_playing = false;
			break;

		case CMD_FRAME_TOGGLE:
			laserdisc_data_w(m_laserdisc, 0xf1);
			break;

		case CMD_0:
		case CMD_1:
		case CMD_2:
		case CMD_3:
		case CMD_4:
		case CMD_5:
		case CMD_6:
		case CMD_7:
		case CMD_8:
		case CMD_9:
			laserdisc_data_w(m_laserdisc, digits[command - CMD_0]);
			break;

		case CMD_SEARCH:
			laserdisc_data_w(m_laserdisc, 0xf7);
			m_playing = false;
			break;
	}
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( ldplayer )
	PORT_START("controls")
	PORT_BIT( 0x0000001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Step reverse") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x0000002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Step forward") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x0000004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Scan reverse") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x0000008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Scan forward") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x0000010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Slow reverse") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x0000020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Slow forward") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x0000040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Fast reverse") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x0000080, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Fast forward") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x0000100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Play/Pause") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x0000200, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Toggle frame display") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x0000400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Toggle chapter display") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0000800, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Toggle left channel") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x0001000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Toggle right channel") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x0010000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("0") PORT_PLAYER(2) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x0020000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("1") PORT_PLAYER(2) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x0040000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("2") PORT_PLAYER(2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x0080000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("3") PORT_PLAYER(2) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x0100000, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("4") PORT_PLAYER(2) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x0200000, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("5") PORT_PLAYER(2) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x0400000, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("6") PORT_PLAYER(2) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x0800000, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("7") PORT_PLAYER(2) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x1000000, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("8") PORT_PLAYER(2) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x2000000, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("9") PORT_PLAYER(2) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x4000000, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Enter") PORT_PLAYER(2) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( ldplayer_ntsc, ldplayer_state )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_LASERDISC_SCREEN_ADD_NTSC("screen", "laserdisc")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED_CLASS( ldv1000, ldplayer_ntsc, ldv1000_state )
	MCFG_LASERDISC_LDV1000_ADD("laserdisc")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
	MCFG_LASERDISC_GET_DISC(ldplayer_state::get_disc_static)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED_CLASS( pr8210, ldplayer_ntsc, pr8210_state )
	MCFG_LASERDISC_PR8210_ADD("laserdisc")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
	MCFG_LASERDISC_GET_DISC(ldplayer_state::get_disc_static)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( ldv1000 )
	DISK_REGION( "laserdisc" )
ROM_END


ROM_START( pr8210 )
	DISK_REGION( "laserdisc" )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 2008, ldv1000, 0, ldv1000, ldplayer, 0, ROT0, "MAME", "Pioneer LDV-1000 Simulator", 0 )
GAMEL(2008, pr8210,  0, pr8210,  ldplayer, 0, ROT0, "MAME", "Pioneer PR-8210 Simulator", 0, layout_pr8210 )
