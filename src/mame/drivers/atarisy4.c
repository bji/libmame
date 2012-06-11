/***************************************************************************

    Atari System IV hardware

    preliminary driver by Phil Bennett

    Games supported:
        * The Last Starfighter (prototype)
        * Air Race (prototype)

    To do:
        * Finish video implementation
        * Find missing HUD/text data for both games

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "video/poly.h"


class atarisy4_state : public driver_device
{
public:
	atarisy4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_r_color_table[256];
	UINT8 m_g_color_table[256];
	UINT8 m_b_color_table[256];
	UINT16 m_dsp_bank[2];
	UINT8 m_csr[2];
	UINT16 *m_m68k_ram;
	UINT16 *m_shared_ram[2];
	UINT16 *m_screen_ram;
	poly_manager *m_poly;
};



/*************************************
 *
 *  State
 *
 *************************************/

struct _gpu_
{
	/* Memory-mapped registers */
	UINT16 gr[8];	/* Command parameters */

	UINT16 bcrw;	/* Screen buffer W control */
	UINT16 bcrx;	/* Screen buffer X control */
	UINT16 bcry;	/* Screen buffer Y control */
	UINT16 bcrz;	/* Screen buffer Z control */
	UINT16 psrw;
	UINT16 psrx;
	UINT16 psry;
	UINT16 psrz;

	UINT16 dpr;
	UINT16 ctr;
	UINT16 lfr;
	UINT16 ifr;
	UINT16 ecr;		/* Execute command register */
	UINT16 far;
	UINT16 mcr;		/* Interrupt control */
	UINT16 qlr;
	UINT16 qar;

	UINT16 dhr;		/* Scanline counter */
	UINT16 dlr;

	/* Others */
	UINT16 idr;
	UINT16 icd;

	UINT8  transpose;
	UINT8  vblank_wait;

	/* Polygon points */
	struct
	{
		INT16 x;
		INT16 y;
	} points[16];

	UINT16 pt_idx;
	bool   poly_open;

	UINT16 clip_min_x;
	UINT16 clip_max_x;
	UINT16 clip_min_y;
	UINT16 clip_max_y;
} gpu;




typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	UINT16 color;
	UINT16 *screen_ram;
};


/*************************************
 *
 *  Forward declarations
 *
 *************************************/

static READ16_HANDLER( analog_r );
static MACHINE_RESET( atarisy4 );
static MACHINE_RESET( airrace );


/*************************************
 *
 *  Video hardware
 *
 *************************************/

static VIDEO_START( atarisy4 )
{
	atarisy4_state *state = machine.driver_data<atarisy4_state>();
	state->m_poly = poly_alloc(machine, 1024, sizeof(poly_extra_data), POLYFLAG_NO_WORK_QUEUE);
}

static VIDEO_RESET( atarisy4 )
{
	gpu.vblank_wait = 0;
}

static SCREEN_UPDATE_RGB32( atarisy4 )
{
	atarisy4_state *state = screen.machine().driver_data<atarisy4_state>();
	int y;
	UINT32 offset = 0;

	if (gpu.bcrw & 0x80)
	{
		offset = 0;
	}
	else if (gpu.bcrx & 0x80)
	{
		offset = 0x10 << 5;
	}

	//UINT32 offset = gpu.dpr << 5;

	for (y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		UINT16 *src = &state->m_screen_ram[(offset + (4096 * y)) / 2];
		UINT32 *dest = &bitmap.pix32(y, cliprect.min_x);
		int x;

		for (x = cliprect.min_x; x < cliprect.max_x; x += 2)
		{
			UINT16 data = *src++;

			*dest++ = screen.machine().pens[data & 0xff];
			*dest++ = screen.machine().pens[data >> 8];
		}
	}
	return 0;
}

INLINE UINT32 xy_to_screen_addr(UINT32 x, UINT32 y)
{
//  UINT32 offset = ((gpu.mcr >> 4) & 3) << 9;
	UINT32 offset = 0;

	if (~gpu.bcrw & 0x80)
	{
		offset = 0;
	}
	else if (~gpu.bcrx & 0x80)
	{
		offset = 0x10 << 5;
	}

	return (y * 4096) + offset + x;
}

static void image_mem_to_screen(atarisy4_state *state, bool clip)
{
	INT16 y = gpu.gr[1] - 0x200;
	UINT16 h = gpu.gr[3];

	if (h & 0x8000)
		h = -h;

	/* Not 100% sure of this */
	while (h--)
	{
		UINT16 w = gpu.gr[2];
		INT16 x = gpu.gr[0] - 0x400;

		if (w & 0x8000)
			w = -w;

		if (y >= 0 && y <= 511)
		{
			while (w--)
			{
				if (x >= 0 && x <= 511)
				{
					UINT16 pix = state->m_screen_ram[xy_to_screen_addr(x,y) >> 1];

					if (x & 1)
						pix = (pix & (0x00ff)) | gpu.idr << 8;
					else
						pix = (pix & (0xff00)) | gpu.idr;

					state->m_screen_ram[xy_to_screen_addr(x,y) >> 1] = pix;
				}
				++x;
			}
		}
		++y;
	}
}

static void draw_scanline(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	UINT16 color = extra->color;
	int x;

	for (x = extent->startx; x < extent->stopx; ++x)
	{
		UINT32 addr = xy_to_screen_addr(x, scanline);
		UINT16 pix = extra->screen_ram[addr >> 1];

		if (x & 1)
			pix = (pix & (0x00ff)) | color << 8;
		else
			pix = (pix & (0xff00)) | color;

		extra->screen_ram[addr >> 1] = pix;
	}
}

static void draw_polygon(atarisy4_state *state, UINT16 color)
{
	int i;
	rectangle clip;
	poly_vertex v1, v2, v3;
	poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(state->m_poly);

	clip.set(0, 511, 0, 511);

	extra->color = color;
	extra->screen_ram = state->m_screen_ram;

	v1.x = gpu.points[0].x;
	v1.y = gpu.points[0].y;

	v2.x = gpu.points[1].x;
	v2.y = gpu.points[1].y;

	/* Draw a triangle fan */
	for (i = 2; i <= gpu.pt_idx; ++i)
	{
		v3.x = gpu.points[i].x;
		v3.y = gpu.points[i].y;

		poly_render_triangle(state->m_poly, 0, clip, draw_scanline, 1, &v1, &v2, &v3);
		v2 = v3;
	}
}

/*
    GPU commands

    REXP    0x00    Reset expansion
    SEXP    0x01    Set expansion
    ROFF    0x02    Reset offset on update
    SOFF    0x03    Set offset
    RTPS    0x04    Reset transposition
    STPS    0x05    Set transposition
    SIMD    0x06    Load IDR register
    SICD    0x07    Load partial transfer compare value
    RCW     0x08    Reset clipping window
    SCW     0x09    Set clipping window
    WVB     0x0B    Wait for VBlank
    FXOT    0x10    Fill x offset table
    FYOT    0x11    Fill y offset table
    FERT    0x14    Fill expansion table
    FCT     0x15    Fill color table
    SMCT    0x16    Screen memory to color table
    ITSN    0x20    Image memory to screen memory transfer
    ITSC    0x21    Image memory to screen memory transfer with clip
    PPA     0x24    Plot point absolute
    PFOA    0x28    Polygon open absolute
    PFOR    0x29    Polygon open relative
    PFVA    0x2A    Polygon vector absolute
    PFVR    0x2B    Polygon vector relative
    PFC     0x2C    Polygon close
*/
void execute_gpu_command(running_machine &machine)
{
	atarisy4_state *state = machine.driver_data<atarisy4_state>();
	switch (gpu.ecr)
	{
		case 0x04:
		{
			gpu.transpose = 0;
			break;
		}
		case 0x05:
		{
			gpu.transpose = 1;
			break;
		}
		case 0x06:
		{
			gpu.idr = gpu.gr[0];
			break;
		}
		case 0x07:
		{
			gpu.icd = gpu.gr[0];
			break;
		}
		case 0x09:
		{
			gpu.clip_max_x = gpu.gr[0];
			gpu.clip_min_x = gpu.gr[1];
			gpu.clip_max_y = gpu.gr[2];
			gpu.clip_min_y = gpu.gr[3];
			break;
		}
		case 0x0b:
		{
			// Wait for VBLANK and swap buffers?
			gpu.vblank_wait = 1;
			break;
		}
		case 0x16:
		{
			/*
                Copy screen RAM to color RAM
                GR0 : Color start X
                GR1 : Color start Y
                GR2 : Color table offset
                GR3 : Size
                GR4 : Channels to set (R: 0x10, G: 0x20, B: 0x40)
            */
			int i;
			int offset = xy_to_screen_addr(gpu.gr[0] - 0x400, gpu.gr[1] - 0x200);
			int table_offs = gpu.gr[2];

			for (i = 0; i < gpu.gr[3]; ++i)
			{
				UINT16 val = state->m_screen_ram[offset >> 1];
				val >>= (~offset & 1) << 3;

				if (gpu.gr[4] & 0x10)
					state->m_r_color_table[table_offs] = val;
				if (gpu.gr[4] & 0x20)
					state->m_g_color_table[table_offs] = val;
				if (gpu.gr[4] & 0x40)
					state->m_b_color_table[table_offs] = val;

				/* Update */
				palette_set_color(machine, table_offs, MAKE_RGB(state->m_r_color_table[table_offs], state->m_g_color_table[table_offs], state->m_b_color_table[table_offs]));

				++table_offs;
				++offset;
			}

			break;
		}
		case 0x20:
		{
			image_mem_to_screen(state, false);
			break;
		}
		case 0x21:
		{
			image_mem_to_screen(state, true);
			break;
		}
		case 0x28:
		{
			gpu.points[0].x = gpu.gr[0] - 0x400;
			gpu.points[0].y = gpu.gr[1] - 0x200;
			gpu.pt_idx = 0;
			break;
		}
		case 0x29:
		{
			gpu.points[0].x = gpu.points[gpu.pt_idx].x + gpu.gr[0];
			gpu.points[0].y = gpu.points[gpu.pt_idx].y + gpu.gr[1];
			gpu.pt_idx = 0;
			break;
		}
		case 0x2a:
		{
			++gpu.pt_idx;
			gpu.points[gpu.pt_idx].x = gpu.gr[0] - 0x400;
			gpu.points[gpu.pt_idx].y = gpu.gr[1] - 0x200;
			break;
		}
		case 0x2b:
		{
			UINT16 x = gpu.points[gpu.pt_idx].x + gpu.gr[0];
			UINT16 y = gpu.points[gpu.pt_idx].y + gpu.gr[1];
			++gpu.pt_idx;
			gpu.points[gpu.pt_idx].x = x;
			gpu.points[gpu.pt_idx].y = y;
			break;
		}
		case 0x2c:
		{
			draw_polygon(state, gpu.gr[2]);
			poly_wait(state->m_poly, "Normal");
			break;
		}
		default:
			logerror("GPU COMMAND: %x\n", gpu.ecr);
	}
}

static WRITE16_HANDLER( gpu_w )
{
	switch (offset)
	{
		case 0x00:	gpu.gr[0] = data;	break;
		case 0x01:	gpu.gr[1] = data;	break;
		case 0x02:	gpu.gr[2] = data;	break;
		case 0x03:	gpu.gr[3] = data;	break;
		case 0x04:	gpu.gr[4] = data;	break;
		case 0x05:	gpu.gr[5] = data;	break;
		case 0x06:	gpu.gr[6] = data;	break;
		case 0x07:	gpu.gr[7] = data;	break;

		case 0x08:	gpu.bcrw = data;	break;
		case 0x09:	gpu.bcrx = data;	break;
		case 0x0a:	gpu.bcry = data;	break;
		case 0x0b:	gpu.bcrz = data;	break;
		case 0x0c:	gpu.psrw = data;	break;
		case 0x0d:	gpu.psrx = data;	break;
		case 0x0e:	gpu.psry = data;	break;
		case 0x0f:	gpu.psrz = data;	break;

		case 0x14:	gpu.dpr = data;		break;
		case 0x15:	gpu.ctr = data;		break;
		case 0x16:	gpu.ifr = data;		break;
		case 0x17:
		{
			gpu.ecr = data;
			execute_gpu_command(space->machine());
			break;
		}
		case 0x1a:	gpu.far = data;		break;
		case 0x20:
		{
			gpu.mcr = data;

			if (~data & 0x08)
				cputag_set_input_line(space->machine(), "maincpu", 6, CLEAR_LINE);

			break;
		}

		case 0x21:	gpu.qlr = data;		break;
		case 0x22:	gpu.qar = data;		break;
	}
}

static READ16_HANDLER( gpu_r )
{
	UINT16 res = 0;

	switch (offset)
	{
		case 0x08:	res = gpu.bcrw;		break;
		case 0x09:	res = gpu.bcrx;		break;
		case 0x0a:	res = gpu.bcry;		break;
		case 0x0b:	res = gpu.bcrz;		break;

		case 0x20:	res = gpu.mcr;		break;

		case 0x400:	res = 5;			break; // TODO!
		case 0x420:	res = 5;			break;

		default:	logerror("GPU_R[%x]\n", offset);
	}

	return res;
}

static INTERRUPT_GEN( vblank_int )
{
	if (gpu.mcr & 0x08)
		cputag_set_input_line(device->machine(), "maincpu", 6, ASSERT_LINE);
}


/*************************************
 *
 *  68000<->TMS comms
 *
 *************************************/

static READ16_HANDLER( m68k_shared_0_r )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	if (!BIT(state->m_csr[0], 3))
		return (state->m_shared_ram[0][offset]);
	else
		return 0xffff;
}

static WRITE16_HANDLER( m68k_shared_0_w )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	if (!BIT(state->m_csr[0], 3))
		COMBINE_DATA(&state->m_shared_ram[0][offset]);
}

static READ16_HANDLER( m68k_shared_1_r )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	if (!BIT(state->m_csr[1], 3))
		return (state->m_shared_ram[1][offset]);
	else
		return 0xffff;
}

static WRITE16_HANDLER( m68k_shared_1_w )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	if (!BIT(state->m_csr[1], 3))
		COMBINE_DATA(&state->m_shared_ram[1][offset]);
}

static READ16_HANDLER( dsp0_status_r )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	return state->m_csr[0];
}

static WRITE16_HANDLER( dsp0_control_w )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	cputag_set_input_line(space->machine(), "dsp0", INPUT_LINE_RESET, data & 0x01 ? CLEAR_LINE : ASSERT_LINE);
	cputag_set_input_line(space->machine(), "dsp0", 0, data & 0x02 ? ASSERT_LINE : CLEAR_LINE);

	state->m_csr[0] = data;
}

static READ16_HANDLER( dsp0_bio_r )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	return BIT(state->m_csr[0], 2);
}

static WRITE16_HANDLER( dsp0_bank_w )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	if (data & 0x4000)
	{
		/* Set TIDONE bit */
		state->m_csr[0] |= 0x10;

		if (BIT(state->m_csr[0], 5) == 1)
			fatalerror("68000 interrupt enable was set!");
	}

	data &= 0x3800;
	memory_set_bankptr(space->machine(), "dsp0_bank1", &state->m_shared_ram[0][data]);
	state->m_dsp_bank[0] = data;
}

static READ16_HANDLER( dsp1_status_r )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	return state->m_csr[1];
}

static WRITE16_HANDLER( dsp1_control_w )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	cputag_set_input_line(space->machine(), "dsp1", INPUT_LINE_RESET, data & 0x01 ? CLEAR_LINE : ASSERT_LINE);
	cputag_set_input_line(space->machine(), "dsp1", 0, data & 0x02 ? ASSERT_LINE : CLEAR_LINE);

	state->m_csr[1] = data;
}

static READ16_HANDLER( dsp1_bio_r )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	return BIT(state->m_csr[1], 2);
}

static WRITE16_HANDLER( dsp1_bank_w )
{
	atarisy4_state *state = space->machine().driver_data<atarisy4_state>();
	if (data & 0x4000)
	{
		/* Set TIDONE bit */
		state->m_csr[1] |= 0x10;

		if (BIT(state->m_csr[1], 5) == 1)
			fatalerror("68000 interrupt enable was set!");
	}

	data &= 0x3800;
	memory_set_bankptr(space->machine(), "dsp1_bank1", &state->m_shared_ram[1][data]);
	state->m_dsp_bank[1] = data;
}


/*************************************
 *
 *  Main CPU memory map
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_BASE_MEMBER(atarisy4_state, m_m68k_ram)
	AM_RANGE(0x010000, 0x01ffff) AM_RAM
	AM_RANGE(0x580000, 0x580001) AM_READ_PORT("JOYSTICK")
	AM_RANGE(0x588000, 0x588001) AM_READ(analog_r)
	AM_RANGE(0x598000, 0x598001) AM_NOP /* Sound board */
	AM_RANGE(0x7c0000, 0x7c4fff) AM_READWRITE(m68k_shared_1_r, m68k_shared_1_w)
	AM_RANGE(0x7c6000, 0x7c6001) AM_READWRITE(dsp1_status_r, dsp1_control_w)
	AM_RANGE(0x7f0000, 0x7f4fff) AM_READWRITE(m68k_shared_0_r, m68k_shared_0_w)
	AM_RANGE(0x7f6000, 0x7f6001) AM_READWRITE(dsp0_status_r, dsp0_control_w)
	AM_RANGE(0xa00400, 0xbfffff) AM_RAM AM_BASE_MEMBER(atarisy4_state, m_screen_ram)
	AM_RANGE(0xff8000, 0xff8fff) AM_READWRITE(gpu_r, gpu_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Mathbox DSP memory map
 *
 *************************************/

static ADDRESS_MAP_START( dsp0_map, AS_PROGRAM, 16 )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x07ff) AM_RAMBANK("dsp0_bank0")
	AM_RANGE(0x0800, 0x0fff) AM_RAMBANK("dsp0_bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp0_io_map, AS_IO, 16 )
	AM_RANGE(0x00, 0x01) AM_WRITE(dsp0_bank_w)
	AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(dsp0_bio_r)
ADDRESS_MAP_END


/*************************************
 *
 *  Mathbox DSP memory map
 *
 *************************************/

static ADDRESS_MAP_START( dsp1_map, AS_PROGRAM, 16 )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x07ff) AM_RAMBANK("dsp1_bank0")
	AM_RANGE(0x0800, 0x0fff) AM_RAMBANK("dsp1_bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp1_io_map, AS_IO, 16 )
	AM_RANGE(0x00, 0x01) AM_WRITE(dsp1_bank_w)
	AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(dsp1_bio_r)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

 static READ16_HANDLER( analog_r )
{
	return (input_port_read(space->machine(), "STICKX") << 8) | input_port_read(space->machine(), "STICKY");
}

static INPUT_PORTS_START( atarisy4 )
	PORT_START("JOYSTICK")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON6 )

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( atarisy4, atarisy4_state )
	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT("screen", vblank_int)

	MCFG_CPU_ADD("dsp0", TMS32010, 16000000)
	MCFG_CPU_PROGRAM_MAP(dsp0_map)
	MCFG_CPU_IO_MAP(dsp0_io_map)

	MCFG_VIDEO_RESET(atarisy4)
	MCFG_MACHINE_RESET(atarisy4)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(32000000/2, 660, 0, 512, 404, 0, 384)
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_UPDATE_STATIC(atarisy4)
	MCFG_PALETTE_LENGTH(256)

	MCFG_VIDEO_START(atarisy4)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( airrace, atarisy4 )

	MCFG_CPU_ADD("dsp1", TMS32010, 16000000)
	MCFG_CPU_PROGRAM_MAP(dsp1_map)
	MCFG_CPU_IO_MAP(dsp1_io_map)

	MCFG_MACHINE_RESET(airrace)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( laststar )
	ROM_REGION( 0x20000, "code", 0 )
	ROM_LOAD( "demo.hex",  0x00000, 0x1c2c4, CRC(7f9e344e) SHA1(ff1462f4f3fa01c47b74d365c240b1c3fdd36755) )

	ROM_REGION( 0x20000, "data", 0 )
	ROM_LOAD( "data1.hex", 0x00000, 0x077cb, CRC(0e75efc0) SHA1(badfcadd92625637bb3223bee986a29428c8d35a) )

	ROM_REGION( 0x20000, "dsp", 0 )
	ROM_LOAD( "mathbs.lda", 0x00000, 0x01e00, CRC(4378739a) SHA1(7c360da99a1366a3315f4846244ed3b2514ef7ec) )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD( "2d_data", 0x00000, 0x077cb, NO_DUMP )
ROM_END


ROM_START( airrace )
	ROM_REGION( 0x30000, "code", 0 )
	ROM_LOAD( "demo.hex",  0x00000, 0x25d23, CRC(937fa025) SHA1(c393a05645ad5df9268f03fffad294477e6f090b) )

	ROM_REGION( 0x20000, "dsp", 0 )
	ROM_LOAD( "mathbs.lda", 0x00000, 0x01e00, CRC(d215acd4) SHA1(853e792b343dc6bcb5b11d455af478a79edfd740) )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD( "2d_data", 0x00000, 0x077cb, NO_DUMP )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

INLINE UINT8 hex_to_ascii(UINT8 in)
{
	if (in < 0x3a)
		return in - 0x30;
	else if (in < 0x47)
		return in - 0x37;
	else
		return in;
}

void load_ldafile(address_space *space, const UINT8 *file)
{
#define READ_CHAR()		file[i++]
	int i = 0;

	while (true)
	{
		UINT8 c;
		UINT8 sum = 1;
		UINT16 len;
		UINT16 addr;

		if (READ_CHAR() != 0x01)
			fatalerror("Bad .LDA file");

		if (READ_CHAR() != 0x00)
			fatalerror("Bad .LDA file");

		len = READ_CHAR();
		sum += len;

		c = READ_CHAR();
		len |= c << 8;
		sum += c;

		/* End of file */
		if (len == 6)
			break;

		addr = READ_CHAR();
		sum += addr;

		c = READ_CHAR();
		addr |= c << 8;
		sum += c;

		len -= 6;

		addr <<= 1;
		do
		{
			UINT8 data = READ_CHAR();
			sum += data;
			space->write_byte(addr++, data);
		} while (--len);

		sum += READ_CHAR();

		if (sum != 0)
			fatalerror(".LDA checksum failure");
	}
}

/* Load memory space with data from a Tektronix-Extended HEX file */
void load_hexfile(address_space *space, const UINT8 *file)
{
#define READ_HEX_CHAR()		hex_to_ascii(file[i++])

	UINT32 i = 0;
	UINT32 line = 1;
	bool end = false;

	while (true)
	{
		UINT8 len;
		UINT8 record;
		UINT8 checksum;
		UINT8 sum = 0;
		UINT8 addr_len;
		UINT32 addr = 0;

		/* Ignore blank lines */
		while (file[i] == '\n') i++;

		/* First character of each line should be a '%' */
		if (file[i++] != '%')
			fatalerror("Error on line %d - invalid line start character", line);

		/* Get the line length */
		len = READ_HEX_CHAR() << 4;
		len |= READ_HEX_CHAR();

		sum += len & 0xf;
		sum += len >> 4;

		/* Get the record type */
		record = READ_HEX_CHAR();
		sum += record;

		/* Record type */
		if (record == 8)
		{
			end = true;
		}
		else if (record == 3)
		{
			/* Ignore this type */
			i += len - 2;
			goto next_line;
		}
		else if (record != 6)
		{
			fatalerror("Error on line %d - Invalid record type %d\n", line, record);
		}

		/* Get the checksum for this line */
		checksum = READ_HEX_CHAR() << 4;
		checksum |= READ_HEX_CHAR();

		/* Get the number of address digits */
		addr_len = READ_HEX_CHAR();
		sum += addr_len;

		len = len - 6 - addr_len;

		/* Form the address */
		while (addr_len)
		{
			UINT8 digit;

			addr <<= 4;
			digit = READ_HEX_CHAR();
			sum += digit;
			addr |= digit;
			--addr_len;
		}

		/* Now read the data */
		while (len)
		{
			UINT8 data;

			data = READ_HEX_CHAR() << 4;
			data |= READ_HEX_CHAR();
			sum += data >> 4;
			sum += data & 0xf;

			if (record == 6)
				space->write_byte(addr++, data);

			len -= 2;
		}

		/* Eat the carriage return */
		++i;

		if (sum != checksum)
			fatalerror("Checksum mismatch on line %d (Found 0x%.2x but expected 0x%.2x)\n", line, sum, checksum);

next_line:
		++line;

		if (end == true)
			break;
	}
}

static DRIVER_INIT( laststar )
{
	atarisy4_state *state = machine.driver_data<atarisy4_state>();
	address_space *main = machine.device("maincpu")->memory().space(AS_PROGRAM);

	/* Allocate 16kB of shared RAM */
	state->m_shared_ram[0] = auto_alloc_array_clear(machine, UINT16, 0x2000);

	/* Populate the 68000 address space with data from the HEX files */
	load_hexfile(main, machine.region("code")->base());
	load_hexfile(main, machine.region("data")->base());

	/* Set up the DSP */
	memory_set_bankptr(machine, "dsp0_bank0", state->m_shared_ram[0]);
	memory_set_bankptr(machine, "dsp0_bank1", &state->m_shared_ram[0][0x800]);
	load_ldafile(machine.device("dsp0")->memory().space(AS_PROGRAM), machine.region("dsp")->base());
}

static DRIVER_INIT( airrace )
{
	atarisy4_state *state = machine.driver_data<atarisy4_state>();
	/* Allocate two sets of 32kB shared RAM */
	state->m_shared_ram[0] = auto_alloc_array_clear(machine, UINT16, 0x4000);
	state->m_shared_ram[1] = auto_alloc_array_clear(machine, UINT16, 0x4000);

	/* Populate RAM with data from the HEX files */
	load_hexfile(machine.device("maincpu")->memory().space(AS_PROGRAM), machine.region("code")->base());

	/* Set up the first DSP */
	memory_set_bankptr(machine, "dsp0_bank0", state->m_shared_ram[0]);
	memory_set_bankptr(machine, "dsp0_bank1", &state->m_shared_ram[0][0x800]);
	load_ldafile(machine.device("dsp0")->memory().space(AS_PROGRAM), machine.region("dsp")->base());

	/* Set up the second DSP */
	memory_set_bankptr(machine, "dsp1_bank0", state->m_shared_ram[1]);
	memory_set_bankptr(machine, "dsp1_bank1", &state->m_shared_ram[1][0x800]);
	load_ldafile(machine.device("dsp1")->memory().space(AS_PROGRAM), machine.region("dsp")->base());
}

static MACHINE_RESET( atarisy4 )
{
	cputag_set_input_line(machine, "dsp0", INPUT_LINE_RESET, ASSERT_LINE);
}

static MACHINE_RESET( airrace )
{
	cputag_set_input_line(machine, "dsp0", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "dsp1", INPUT_LINE_RESET, ASSERT_LINE);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1984, laststar, 0, atarisy4, atarisy4, laststar, ROT0, "Atari Games", "The Last Starfighter (prototype)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND_HW )
GAME( 1985, airrace,  0, airrace,  atarisy4, airrace,  ROT0, "Atari Games", "Air Race (prototype)",             GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND_HW )
