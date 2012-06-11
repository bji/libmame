/*

    Galaxy Game

    Driver by Mariusz Wojcieszek

    First commercial video game.

*/

#include "emu.h"
#include "cpu/t11/t11.h"


#define MAX_POINTS 2048

class galaxygame_state : public driver_device
{
public:
	galaxygame_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) {}

	UINT16 m_clk;

	UINT16 m_x;
	UINT16 m_y;

	INT16 m_mq;
	INT16 m_ac;

	struct
	{
		UINT16 x;
		UINT16 y;
	} m_point_work_list[MAX_POINTS], m_point_display_list[MAX_POINTS];

	int m_point_work_list_index;
	int m_point_display_list_index;
	int m_interrupt;
	DECLARE_READ16_MEMBER(ke_r);
	DECLARE_WRITE16_MEMBER(ke_w);
	DECLARE_READ16_MEMBER(x_r);
	DECLARE_WRITE16_MEMBER(x_w);
	DECLARE_READ16_MEMBER(y_r);
	DECLARE_WRITE16_MEMBER(y_w);
	DECLARE_WRITE16_MEMBER(clk_w);
};

/*************************************
 *
 *  KE11 Extended Artithmetic Element
 *
 *************************************/

READ16_MEMBER(galaxygame_state::ke_r)
{

	UINT16 ret;

	switch( offset )
	{
		case 0:	// DIV
			ret = 0;
			break;
		case 1: // AC
			ret = m_ac;
			break;
		case 2: // MQ
			ret = m_mq;
			break;
		case 7: // ASH
			ret = 0;
			break;
		default:
			ret = 0;
			logerror("Unhandled KE read (%d)\n", offset );
			break;
	}
	return ret;
}

WRITE16_MEMBER(galaxygame_state::ke_w)
{

	switch( offset )
	{
		case 0: // DIV
			{
				if ( data != 0 )
				{
					INT32 dividend = (INT32)((UINT32)((UINT16)m_ac << 16) | (UINT16)(m_mq));
					m_mq = dividend / (INT16)data;
					m_ac = dividend % (INT16)data;
				}
				else
				{
					m_mq = 0;
					m_ac = 0;
				}
			}
			break;
		case 1: // AC
			m_ac = (INT16)data;
			break;
		case 2: // MQ
			m_mq = (INT16)data;
			if (m_mq < 0)
			{
				m_ac = -1;
			}
			else
			{
				m_ac = 0;
			}
			break;
		case 3: // X
			{
				INT32 mulres = (INT32)m_mq*(INT32)(INT16)data;
				m_ac = mulres >> 16;
				m_mq = mulres & 0xffff;
			}
			break;
		case 6: // LSH
			{
				data &= 63;
				INT32 val = (INT32)((UINT32)((UINT16)m_ac << 16) | (UINT16)(m_mq));
				if ( data < 32 )
				{
					val = val << data;
				}
				else
				{
					val = val >> (64 - data);
				}
				m_mq = val & 0xffff;
				m_ac = (val >> 16) & 0xffff;
			}
			break;
		case 7: // ASH
			{
				data &= 63;
				INT32 val = (INT32)((UINT32)((UINT16)m_ac << 16) | (UINT16)(m_mq));
				if ( data < 32 )
				{
					val = val << data;
				}
				else
				{
					val = val >> (64 - data);
				}
				m_mq = val & 0xffff;
				m_ac = (val >> 16) & 0xffff;
			}
			break;
		default:
			logerror("Unhandled KE write (%d)\n", offset);
			break;
	}

}

/*************************************
 *
 *  Video
 *
 *************************************/

static SCREEN_UPDATE_IND16( galaxygame )
{
	galaxygame_state *state = screen.machine().driver_data<galaxygame_state>();
	bitmap.fill(get_black_pen(screen.machine()), cliprect);
	for (int i = 0; i < state->m_point_display_list_index; i++ )
	{
		bitmap.pix16(state->m_point_display_list[i].x >> 7, state->m_point_display_list[i].y >> 7) = 1;
	}
	return 0;
}

READ16_MEMBER(galaxygame_state::x_r)
{
	return m_x;
}

WRITE16_MEMBER(galaxygame_state::x_w)
{
	m_x = data;
}

READ16_MEMBER(galaxygame_state::y_r)
{
	return m_y;
}

WRITE16_MEMBER(galaxygame_state::y_w)
{
	m_y = data;
	if ( data == 0x0101 )
	{
		// send points list to display device
		// seems to happen on first 0x0101 write after interrupt
		if ( m_interrupt )
		{
			for ( int i = 0; i < m_point_work_list_index ; i++ )
			{
				m_point_display_list[i].x = m_point_work_list[i].x;
				m_point_display_list[i].y = m_point_work_list[i].y;
			}
			m_point_display_list_index = m_point_work_list_index;
			m_point_work_list_index = 0;
			m_interrupt = 0;
		}
	}
	else
	{
		if ( m_point_work_list_index >= MAX_POINTS )
		{
			logerror("Work list overflow\n");
		}
		else
		{
			m_point_work_list[m_point_work_list_index].x = m_x;
			m_point_work_list[m_point_work_list_index].y = m_y;
			m_point_work_list_index++;
		}
	}
}

/*************************************
 *
 *  Inputs
 *
 *************************************/

static INPUT_PORTS_START( galaxygame )
	PORT_START("COINAC")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_COIN3 ) // 25 cents, left
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_COIN1 ) // 10 cents, left
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_COIN4 ) // 25 cents, right
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_COIN2 ) // 10 cents, right

	PORT_START("SR")
	PORT_DIPNAME( 0x8000, 0x0000, "Gravity" )
	PORT_DIPSETTING(      0x0000, "Positive Gravity" )
	PORT_DIPSETTING(      0x8000, "Negative Gravity" )
	PORT_DIPNAME( 0x4000, 0x0000, "Sun" )
	PORT_DIPSETTING(      0x0000, "Sun (& Gravity)" )
	PORT_DIPSETTING(      0x4000, "No Sun (& No Gravity)" )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) // fire
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) // hyper
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) // thrust
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)

	PORT_DIPNAME( 0x0080, 0x0000, "Speed" )
	PORT_DIPSETTING(      0x0000, "Slow Speed" )
	PORT_DIPSETTING(      0x0080, "Fast Speed" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Players ) )
	PORT_DIPSETTING(      0x0000, "Two Players" )
	PORT_DIPSETTING(      0x0040, "One Player" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
INPUT_PORTS_END

/*************************************
 *
 *  Machine
 *
 *************************************/

WRITE16_MEMBER(galaxygame_state::clk_w)
{
	m_clk = data;
}

static ADDRESS_MAP_START( galaxygame_map, AS_PROGRAM, 16, galaxygame_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0xfec0, 0xfecf) AM_READWRITE(ke_r, ke_w)
	AM_RANGE(0xff52, 0xff53) AM_READWRITE(y_r, y_w) // 177522 Y
	AM_RANGE(0xff54, 0xff55) AM_READ_PORT("COINAC") // 177524 COINAC
	AM_RANGE(0xff5a, 0xff5b) AM_READWRITE(x_r, x_w)	// 177532 X
	AM_RANGE(0xff5c, 0xff5d) AM_READ_PORT("SR")		// 177534 SR
	AM_RANGE(0xff66, 0xff67) AM_WRITE(clk_w)		// 177546 KW11 line frequency clock
ADDRESS_MAP_END


static PALETTE_INIT( galaxygame )
{
	palette_set_color(machine,0,RGB_BLACK); /* black */
	palette_set_color(machine,1,RGB_WHITE); /* white */
}

static IRQ_CALLBACK(galaxygame_irq_callback)
{
	device_set_input_line(device, 0, CLEAR_LINE);
	return 0x40;
}

static INTERRUPT_GEN(galaxygame_irq)
{
	galaxygame_state *state = device->machine().driver_data<galaxygame_state>();
	if ( state->m_clk & 0x40 )
	{
		device_set_input_line(device, 0, ASSERT_LINE);
		state->m_interrupt = 1;
	}
}

static MACHINE_RESET( galaxygame )
{
	galaxygame_state *state = machine.driver_data<galaxygame_state>();
	state->m_clk = 0x00;
	state->m_point_work_list_index = 0;
	state->m_point_display_list_index = 0;
	state->m_interrupt = 0;

	device_set_irq_callback(machine.device("maincpu"), galaxygame_irq_callback);
}

static const struct t11_setup t11_data =
{
	5 << 13
};

static MACHINE_CONFIG_START( galaxygame, galaxygame_state )

	MCFG_CPU_ADD("maincpu", T11, 3000000 )
	MCFG_CPU_PROGRAM_MAP(galaxygame_map)
	MCFG_CPU_CONFIG(t11_data)
	MCFG_CPU_PERIODIC_INT(galaxygame_irq,60)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 511)
	MCFG_SCREEN_UPDATE_STATIC(galaxygame)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(galaxygame)

	MCFG_MACHINE_RESET(galaxygame)
MACHINE_CONFIG_END

ROM_START(galgame)
	// Original Galaxy Game listing, the one used in the 2nd hardware revision (blue dual cabinet)
	// PALX11 V413R 19-NOV-71 8:46
	ROM_REGION( 0x20000, "code", ROMREGION_ERASE00 )
	ROM_LOAD( "sw97.lst", 0x00000, 0x1f062, CRC(838018a5) SHA1(e3c47c5cf78299650b031ec49fde7d9e4024a759) )
ROM_END

/*************************************
 *
 *  Code loading
 *
 *************************************/

static UINT8 read_uint16(UINT16 *pval, int pos, const UINT8* line, int linelen)
{
	int i;

	*pval = 0;
	if ( linelen < (pos + 6) )
	{
		return 0;
	}

	for ( i = 0; i < 6; i++ )
	{
		*pval <<= 3;
		*pval |= line[pos + i] - 0x30;
	}
	return 1;
}

static UINT8 read_uint8(UINT8 *pval, int pos, const UINT8* line, int linelen)
{
	int i;

	*pval = 0;
	if ( linelen < (pos + 3) )
	{
		return 0;
	}

	for ( i = 0; i < 3; i++ )
	{
		*pval <<= 3;
		*pval |= line[pos + i] - 0x30;
	}
	return 1;
}

static DRIVER_INIT(galaxygame)
{
	address_space *main = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *code = machine.root_device().memregion("code")->base();

	int filepos = 0, linepos, linelen;
	UINT8 line[256];
	UINT16 address;
	UINT16 val;
	UINT8 val8;

	//load lst file
	while( code[filepos] != 0 )
	{
		linepos = 0;
		while( code[filepos] != 0x0d )
		{
			line[linepos++] = code[filepos++];
		}
		line[linepos] = 0;
		filepos += 2;
		linelen = linepos;

		if ( linelen == 0 )
		{
			continue;
		}
		if ( ( line[8] != ' ' ) && read_uint16(&address, 7, line, linelen ) )
		{
			if ( (linelen >= 15+6) && (line[15] != ' ') )
			{
				read_uint16(&val, 15, line, linelen);
				main->write_word(address, val, 0xffff);
				address += 2;

				if ( (linelen >= 22+6) && (line[22] != ' ') )
				{
					read_uint16(&val, 22, line, linelen);
					main->write_word(address, val, 0xffff);
					address += 2;
				}

				if ( (linelen >= 29+6) && (line[29] != ' ') )
				{
					read_uint16(&val, 29, line, linelen);
					main->write_word(address, val, 0xffff);
					address += 2;
				}

			}
			else
			{
				if ( (linelen >= 18+3) && (line[18] != ' ') )
				{
					read_uint8(&val8, 18, line, linelen);
					main->write_byte(address, val8);
					address += 1;
				}
			}

		}
	}

	// set startup code
	main->write_word(0, 012700); /* MOV #0, R0 */
	main->write_word(2, 0);
	main->write_word(4, 0x8d00); /* MTPS R0 */
	main->write_word(6, 000167); /* JMP 0500*/
	main->write_word(8, 000500 - 10);
}

GAME(1971, galgame, 0, galaxygame, galaxygame, galaxygame, ROT270, "Computer Recreations, Inc", "Galaxy Game", GAME_NO_SOUND_HW )
