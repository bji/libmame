/**********************************************************************

    Motorola MC6845 and compatible CRT controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    The following variations exist that are different in
    functionality and not just in speed rating(1):
        * Motorola 6845, 6845-1
        * Hitachi 46505
        * Rockwell 6545, 6545-1 (= Synertek SY6545-1)
        * MOS Technology 6545-1

    (1) as per the document at
    http://www.6502.org/users/andre/hwinfo/crtc/diffs.html

    The various speed rated devices are identified by a letter,
    for example M68A45, M68B45, etc.

    The chip is originally designed by Hitachi, not by Motorola.

**********************************************************************/

#include "emu.h"
#include "mc6845.h"
#include "machine/devhelpr.h"


#define LOG		(0)


const device_type MC6845 = &device_creator<mc6845_device>;
const device_type MC6845_1 = &device_creator<mc6845_1_device>;
const device_type R6545_1 = &device_creator<r6545_1_device>;
const device_type C6545_1 = &device_creator<c6545_1_device>;
const device_type H46505 = &device_creator<h46505_device>;
const device_type HD6845 = &device_creator<hd6845_device>;
const device_type SY6545_1 = &device_creator<sy6545_1_device>;
const device_type SY6845E = &device_creator<sy6845e_device>;
const device_type HD6345 = &device_creator<hd6345_device>;


/* mode macros */
#define MODE_TRANSPARENT			((m_mode_control & 0x08) != 0)
#define MODE_TRANSPARENT_PHI2		((m_mode_control & 0x88) == 0x88)
/* FIXME: not supported yet */
#define MODE_TRANSPARENT_BLANK		((m_mode_control & 0x88) == 0x08)
#define MODE_UPDATE_STROBE			((m_mode_control & 0x40) != 0)
#define MODE_CURSOR_SKEW			((m_mode_control & 0x20) != 0)
#define MODE_DISPLAY_ENABLE_SKEW	((m_mode_control & 0x10) != 0)
#define MODE_ROW_COLUMN_ADDRESSING	((m_mode_control & 0x04) != 0)


void mc6845_device::device_config_complete()
{
	const mc6845_interface *intf = reinterpret_cast<const mc6845_interface *>(static_config());

	if ( intf != NULL )
	{
		*static_cast<mc6845_interface *>(this) = *intf;
	}
	else
	{
		m_screen_tag = NULL;
		m_hpixels_per_column = 0;
		m_begin_update = NULL;
		m_update_row = NULL;
		m_end_update = NULL;
		m_on_update_addr_changed = NULL;
		memset(&m_out_de_func, 0, sizeof(m_out_de_func));
		memset(&m_out_cur_func, 0, sizeof(m_out_cur_func));
		memset(&m_out_hsync_func, 0, sizeof(m_out_hsync_func));
		memset(&m_out_vsync_func, 0, sizeof(m_out_vsync_func));
	}
}


mc6845_device::mc6845_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock)
{
}

mc6845_device::mc6845_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MC6845, "mc6845", tag, owner, clock)
{
}


void mc6845_device::device_post_load()
{
	recompute_parameters(true);
}


void mc6845_device::call_on_update_address(int strobe)
{
	if (m_on_update_addr_changed)
		m_upd_trans_timer->adjust(attotime::zero, (m_update_addr << 8) | strobe);
	else
		fatalerror("M6845: transparent memory mode without handler\n");
}


WRITE8_MEMBER( mc6845_device::address_w )
{
	m_register_address_latch = data & 0x1f;
}


READ8_MEMBER( mc6845_device::status_r )
{
	UINT8 ret = 0;

	/* VBLANK bit */
	if (m_supports_status_reg_d5 && !m_line_enable_ff)
	   ret = ret | 0x20;

	/* light pen latched */
	if (m_supports_status_reg_d6 && m_light_pen_latched)
	   ret = ret | 0x40;

	/* UPDATE ready */
	if (m_supports_status_reg_d7 && m_update_ready_bit)
	   ret = ret | 0x80;

	return ret;
}


READ8_MEMBER( mc6845_device::register_r )
{
	UINT8 ret = 0;

	switch (m_register_address_latch)
	{
		case 0x0c:  ret = m_supports_disp_start_addr_r ? (m_disp_start_addr >> 8) & 0xff : 0; break;
		case 0x0d:  ret = m_supports_disp_start_addr_r ? (m_disp_start_addr >> 0) & 0xff : 0; break;
		case 0x0e:  ret = (m_cursor_addr    >> 8) & 0xff; break;
		case 0x0f:  ret = (m_cursor_addr    >> 0) & 0xff; break;
		case 0x10:  ret = (m_light_pen_addr >> 8) & 0xff; m_light_pen_latched = false; break;
		case 0x11:  ret = (m_light_pen_addr >> 0) & 0xff; m_light_pen_latched = false; break;
		case 0x1f:
			if (m_supports_transparent && MODE_TRANSPARENT)
			{
				if(MODE_TRANSPARENT_PHI2)
				{
					m_update_addr++;
					m_update_addr &= 0x3fff;
					call_on_update_address(0);
				}
				else
				{
					/* MODE_TRANSPARENT_BLANK */
					if(m_update_ready_bit)
					{
						m_update_ready_bit = false;
						update_upd_adr_timer();
					}
				}
			}
			break;

		/* all other registers are write only and return 0 */
		default: break;
	}

	return ret;
}


WRITE8_MEMBER( mc6845_device::register_w )
{
	if (LOG)  logerror("%s:M6845 reg 0x%02x = 0x%02x\n", machine().describe_context(), m_register_address_latch, data);

	switch (m_register_address_latch)
	{
		case 0x00:  m_horiz_char_total =   data & 0xff; break;
		case 0x01:  m_horiz_disp       =   data & 0xff; break;
		case 0x02:  m_horiz_sync_pos   =   data & 0xff; break;
		case 0x03:  m_sync_width       =   data & 0xff; break;
		case 0x04:  m_vert_char_total  =   data & 0x7f; break;
		case 0x05:  m_vert_total_adj   =   data & 0x1f; break;
		case 0x06:  m_vert_disp        =   data & 0x7f; break;
		case 0x07:  m_vert_sync_pos    =   data & 0x7f; break;
		case 0x08:  m_mode_control     =   data & 0xff; break;
		case 0x09:  m_max_ras_addr     =   data & 0x1f; break;
		case 0x0a:  m_cursor_start_ras =   data & 0x7f; break;
		case 0x0b:  m_cursor_end_ras   =   data & 0x1f; break;
		case 0x0c:  m_disp_start_addr  = ((data & 0x3f) << 8) | (m_disp_start_addr & 0x00ff); break;
		case 0x0d:  m_disp_start_addr  = ((data & 0xff) << 0) | (m_disp_start_addr & 0xff00); break;
		case 0x0e:  m_cursor_addr      = ((data & 0x3f) << 8) | (m_cursor_addr & 0x00ff); break;
		case 0x0f:  m_cursor_addr      = ((data & 0xff) << 0) | (m_cursor_addr & 0xff00); break;
		case 0x10: /* read-only */ break;
		case 0x11: /* read-only */ break;
		case 0x12:
			if (m_supports_transparent)
			{
				m_update_addr = ((data & 0x3f) << 8) | (m_update_addr & 0x00ff);
				if(MODE_TRANSPARENT_PHI2)
					call_on_update_address(0);
			}
			break;
		case 0x13:
			if (m_supports_transparent)
			{
				m_update_addr = ((data & 0xff) << 0) | (m_update_addr & 0xff00);
				if(MODE_TRANSPARENT_PHI2)
					call_on_update_address(0);
			}
			break;
		case 0x1f:
			if (m_supports_transparent && MODE_TRANSPARENT)
			{
				if(MODE_TRANSPARENT_PHI2)
				{
					m_update_addr++;
					m_update_addr &= 0x3fff;
					call_on_update_address(0);
				}
				else
				{
					/* MODE_TRANSPARENT_BLANK */
					if(m_update_ready_bit)
					{
						m_update_ready_bit = false;
						update_upd_adr_timer();
					}
				}
			}
			break;
		default: break;
	}

	/* display message if the Mode Control register is not zero */
	if ((m_register_address_latch == 0x08) && (m_mode_control != 0))
		if (!m_supports_transparent)
			popmessage("Mode Control %02X is not supported!!!", m_mode_control);

	recompute_parameters(false);
}


READ_LINE_MEMBER( mc6845_device::de_r )
{
	return m_de;
}


READ_LINE_MEMBER( mc6845_device::cursor_r )
{
	return m_cur;
}


READ_LINE_MEMBER( mc6845_device::hsync_r )
{
	return m_hsync;
}


READ_LINE_MEMBER( mc6845_device::vsync_r )
{
	return m_vsync;
}


void mc6845_device::recompute_parameters(bool postload)
{
	UINT16 hsync_on_pos, hsync_off_pos, vsync_on_pos, vsync_off_pos;

	/* compute the screen sizes */
	UINT16 horiz_pix_total = (m_horiz_char_total + 1) * m_hpixels_per_column;
	UINT16 vert_pix_total = (m_vert_char_total + 1) * (m_max_ras_addr + 1) + m_vert_total_adj;

	/* determine the visible area, avoid division by 0 */
	UINT16 max_visible_x = m_horiz_disp * m_hpixels_per_column - 1;
	UINT16 max_visible_y = m_vert_disp * (m_max_ras_addr + 1) - 1;

	/* determine the syncing positions */
	UINT8 horiz_sync_char_width = m_sync_width & 0x0f;
	UINT8 vert_sync_pix_width = m_supports_vert_sync_width ? (m_sync_width >> 4) & 0x0f : 0x10;

	if (horiz_sync_char_width == 0)
		horiz_sync_char_width = 0x10;

	if (vert_sync_pix_width == 0)
		vert_sync_pix_width = 0x10;

	/* determine the transparent update cycle time, 1 update every 4 character clocks */
	m_upd_time = attotime::from_hz(m_clock) * (4 * m_hpixels_per_column);

	hsync_on_pos = m_horiz_sync_pos * m_hpixels_per_column;
	hsync_off_pos = hsync_on_pos + (horiz_sync_char_width * m_hpixels_per_column);
	vsync_on_pos = m_vert_sync_pos * (m_max_ras_addr + 1);
	vsync_off_pos = vsync_on_pos + vert_sync_pix_width;

	/* the Commodore PET computers program a horizontal synch pulse that extends
       past the scanline width.  I assume that the real device will clamp it */
	if (hsync_off_pos > horiz_pix_total)
		hsync_off_pos = horiz_pix_total;

	if (vsync_off_pos > vert_pix_total)
		vsync_off_pos = vert_pix_total;

	/* update only if screen parameters changed, unless we are coming here after loading the saved state */
	if (postload ||
	    (horiz_pix_total != m_horiz_pix_total) || (vert_pix_total != m_vert_pix_total) ||
		(max_visible_x != m_max_visible_x) || (max_visible_y != m_max_visible_y) ||
		(hsync_on_pos != m_hsync_on_pos) || (vsync_on_pos != m_vsync_on_pos) ||
		(hsync_off_pos != m_hsync_off_pos) || (vsync_off_pos != m_vsync_off_pos))
	{
		/* update the screen if we have valid data */
		if ((horiz_pix_total > 0) && (max_visible_x < horiz_pix_total) &&
			(vert_pix_total > 0) && (max_visible_y < vert_pix_total) &&
			(hsync_on_pos <= horiz_pix_total) && (vsync_on_pos <= vert_pix_total) &&
			(hsync_on_pos != hsync_off_pos))
		{
			rectangle visarea;

			attoseconds_t refresh = HZ_TO_ATTOSECONDS(m_clock) * (m_horiz_char_total + 1) * vert_pix_total;

			visarea.min_x = 0;
			visarea.min_y = 0;
			visarea.max_x = max_visible_x;
			visarea.max_y = max_visible_y;

			if (LOG) logerror("M6845 config screen: HTOTAL: 0x%x  VTOTAL: 0x%x  MAX_X: 0x%x  MAX_Y: 0x%x  HSYNC: 0x%x-0x%x  VSYNC: 0x%x-0x%x  Freq: %ffps\n",
							  horiz_pix_total, vert_pix_total, max_visible_x, max_visible_y, hsync_on_pos, hsync_off_pos - 1, vsync_on_pos, vsync_off_pos - 1, 1 / ATTOSECONDS_TO_DOUBLE(refresh));

			if ( m_screen != NULL )
				m_screen->configure(horiz_pix_total, vert_pix_total, visarea, refresh);

			m_has_valid_parameters = true;
		}
		else
			m_has_valid_parameters = false;

		m_horiz_pix_total = horiz_pix_total;
		m_vert_pix_total = vert_pix_total;
		m_max_visible_x = max_visible_x;
		m_max_visible_y = max_visible_y;
		m_hsync_on_pos = hsync_on_pos;
		m_hsync_off_pos = hsync_off_pos;
		m_vsync_on_pos = vsync_on_pos;
		m_vsync_off_pos = vsync_off_pos;
	}
}


void mc6845_device::update_counters()
{
	m_character_counter = m_line_timer->elapsed( ).as_ticks( m_clock );

	if ( m_hsync_off_timer ->enabled( ) )
	{
		m_hsync_width_counter = m_hsync_off_timer ->elapsed( ).as_ticks( m_clock );
	}
}


void mc6845_device::set_de(int state)
{
	if ( m_de != state )
	{
		m_de = state;

		if ( m_de )
		{
			/* If the upd_adr_timer was running, cancel it */
			m_upd_adr_timer->adjust(attotime::never);
		}
		else
		{
			/* if transparent update was requested fire the update timer */
			if(!m_update_ready_bit)
				update_upd_adr_timer();
		}

		if ( !m_res_out_de_func.isnull() )
			m_res_out_de_func( m_de );
	}
}


void mc6845_device::set_hsync(int state)
{
	if ( m_hsync != state )
	{
		m_hsync = state;

		if ( !m_res_out_hsync_func.isnull() )
			m_res_out_hsync_func( m_hsync );
	}
}


void mc6845_device::set_vsync(int state)
{
	if ( m_vsync != state )
	{
		m_vsync = state;

		if ( !m_res_out_vsync_func.isnull() )
			m_res_out_vsync_func( m_vsync );
	}
}


void mc6845_device::set_cur(int state)
{
	if ( m_cur != state )
	{
		m_cur = state;

		if ( !m_res_out_cur_func.isnull() )
			m_res_out_cur_func( m_cur );
	}
}


void mc6845_device::update_upd_adr_timer()
{
	if (! m_de && m_supports_transparent)
		m_upd_adr_timer->adjust(m_upd_time);
}


void mc6845_device::handle_line_timer()
{
	int new_vsync = m_vsync;

	m_character_counter = 0;
	m_cursor_x = -1;

	/* Check if VSYNC is active */
	if ( m_vsync_ff )
	{
		UINT8 vsync_width = m_supports_vert_sync_width ? (m_sync_width >> 4) & 0x0f : 0;

		m_vsync_width_counter = ( m_vsync_width_counter + 1 ) & 0x0F;

		/* Check if we've reached end of VSYNC */
		if ( m_vsync_width_counter == vsync_width )
		{
			m_vsync_ff = 0;

			new_vsync = FALSE;
		}
	}

	if ( m_raster_counter == m_max_ras_addr )
	{
		/* Check if we have reached the end of the vertical area */
		if ( m_line_counter == m_vert_char_total )
		{
			m_adjust_counter = 0;
			m_adjust_active = 1;
		}

		m_raster_counter = 0;
		m_line_counter = ( m_line_counter + 1 ) & 0x7F;
		m_line_address = ( m_line_address + m_horiz_disp ) & 0x3fff;

		/* Check if we've reached the end of active display */
		if ( m_line_counter == m_vert_disp )
		{
			m_line_enable_ff = false;
		}

		/* Check if VSYNC should be enabled */
		if ( m_line_counter == m_vert_sync_pos )
		{
			m_vsync_width_counter = 0;
			m_vsync_ff = 1;

			new_vsync = TRUE;
		}
	}
	else
	{
		m_raster_counter = ( m_raster_counter + 1 ) & 0x1F;
	}

	if ( m_adjust_active )
	{
		/* Check if we have reached the end of a full cycle */
		if ( m_adjust_counter == m_vert_total_adj )
		{
			m_adjust_active = 0;
			m_raster_counter = 0;
			m_line_counter = 0;
			m_line_address = m_disp_start_addr;
			m_line_enable_ff = true;
			/* also update the cursor state now */
			update_cursor_state();

			if (m_screen != NULL)
				m_screen->reset_origin();
		}
		else
		{
			m_adjust_counter = ( m_adjust_counter + 1 ) & 0x1F;
		}
	}

	if ( m_line_enable_ff )
	{
		/* Schedule DE off signal change */
		m_de_off_timer->adjust(attotime::from_ticks( m_horiz_disp, m_clock ));

		/* Is cursor visible on this line? */
		if ( m_cursor_state &&
			(m_raster_counter >= (m_cursor_start_ras & 0x1f)) &&
			(m_raster_counter <= m_cursor_end_ras) &&
			(m_cursor_addr >= m_line_address) &&
			(m_cursor_addr < (m_line_address + m_horiz_disp)) )
		{
			m_cursor_x = m_cursor_addr - m_line_address;

			/* Schedule CURSOR ON signal */
			m_cur_on_timer->adjust( attotime::from_ticks( m_cursor_x, m_clock ) );
		}
	}

	/* Schedule HSYNC on signal */
	m_hsync_on_timer->adjust( attotime::from_ticks( m_horiz_sync_pos, m_clock ) );

	/* Schedule our next callback */
	m_line_timer->adjust( attotime::from_ticks( m_horiz_char_total + 1, m_clock ) );

	/* Set VSYNC and DE signals */
	set_vsync( new_vsync );
	set_de( m_line_enable_ff ? TRUE : FALSE );
}


void mc6845_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_LINE:
		handle_line_timer();
		break;

	case TIMER_DE_OFF:
		set_de( FALSE );
		break;

	case TIMER_CUR_ON:
		set_cur( TRUE );

		/* Schedule CURSOR off signal */
		m_cur_off_timer->adjust( attotime::from_ticks( 1, m_clock ) );
		break;

	case TIMER_CUR_OFF:
		set_cur( FALSE );
		break;

	case TIMER_HSYNC_ON:
		{
			UINT8 hsync_width = ( m_sync_width & 0x0f ) ? ( m_sync_width & 0x0f ) : 0x10;

			m_hsync_width_counter = 0;
			set_hsync( TRUE );

			/* Schedule HSYNC off signal */
			m_hsync_off_timer->adjust( attotime::from_ticks( hsync_width, m_clock ) );
		}
		break;

	case TIMER_HSYNC_OFF:
		set_hsync( FALSE );
		break;

	case TIMER_LIGHT_PEN_LATCH:
		m_light_pen_addr = get_ma();
		m_light_pen_latched = true;
		break;

	case TIMER_UPD_ADR:
		/* fire a update address strobe */
		call_on_update_address(0);
		break;

	case TIMER_UPD_TRANS:
		{
			int addr = (param >> 8);
			int strobe = (param & 0xff);

			/* call the callback function -- we know it exists */
			m_on_update_addr_changed(this, addr, strobe);

			if(!m_update_ready_bit && MODE_TRANSPARENT_BLANK)
			{
				m_update_addr++;
				m_update_addr &= 0x3fff;
				m_update_ready_bit = true;
			}
		}
		break;

	}
}


UINT16 mc6845_device::get_ma()
{
	update_counters();

	return ( m_line_address + m_character_counter ) & 0x3fff;
}


UINT8 mc6845_device::get_ra()
{
	return m_raster_counter;
}


void mc6845_device::assert_light_pen_input()
{
	/* compute the pixel coordinate of the NEXT character -- this is when the light pen latches */
	/* set the timer that will latch the display address into the light pen registers */
	m_light_pen_latch_timer->adjust(attotime::from_ticks( 1, m_clock ));
}


void mc6845_device::set_clock(int clock)
{
	/* validate arguments */
	assert(clock > 0);

	if (clock != m_clock)
	{
		m_clock = clock;
		recompute_parameters(true);
	}
}


void mc6845_device::set_hpixels_per_column(int hpixels_per_column)
{
	/* validate arguments */
	assert(hpixels_per_column > 0);

	if (hpixels_per_column != m_hpixels_per_column)
	{
		m_hpixels_per_column = hpixels_per_column;
		recompute_parameters(false);
	}
}


void mc6845_device::update_cursor_state()
{
	/* save and increment cursor counter */
	UINT8 last_cursor_blink_count = m_cursor_blink_count;
	m_cursor_blink_count = m_cursor_blink_count + 1;

	/* switch on cursor blinking mode */
	switch (m_cursor_start_ras & 0x60)
	{
		/* always on */
		case 0x00: m_cursor_state = true; break;

		/* always off */
		default:
		case 0x20: m_cursor_state = false; break;

		/* fast blink */
		case 0x40:
			if ((last_cursor_blink_count & 0x10) != (m_cursor_blink_count & 0x10))
				m_cursor_state = !m_cursor_state;
			break;

		/* slow blink */
		case 0x60:
			if ((last_cursor_blink_count & 0x20) != (m_cursor_blink_count & 0x20))
				m_cursor_state = !m_cursor_state;
			break;
	}
}


void mc6845_device::update(bitmap_t *bitmap, const rectangle *cliprect)
{
	assert(bitmap != NULL);
	assert(cliprect != NULL);

	if (m_has_valid_parameters)
	{
		UINT16 y;

		void *param = NULL;

		assert(m_update_row != NULL);

		/* call the set up function if any */
		if (m_begin_update != NULL)
			param = m_begin_update(this, bitmap, cliprect);

		if (cliprect->min_y == 0)
		{
			/* read the start address at the beginning of the frame */
			m_current_disp_addr = m_disp_start_addr;
		}

		/* for each row in the visible region */
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			/* compute the current raster line */
			UINT8 ra = y % (m_max_ras_addr + 1);

			/* check if the cursor is visible and is on this scanline */
			int cursor_visible = m_cursor_state &&
								(ra >= (m_cursor_start_ras & 0x1f)) &&
								(ra <= m_cursor_end_ras) &&
								(m_cursor_addr >= m_current_disp_addr) &&
								(m_cursor_addr < (m_current_disp_addr + m_horiz_disp));

			/* compute the cursor X position, or -1 if not visible */
			INT8 cursor_x = cursor_visible ? (m_cursor_addr - m_current_disp_addr) : -1;

			/* call the external system to draw it */
			if (MODE_ROW_COLUMN_ADDRESSING)
			{
				UINT8 cc = 0;
				UINT8 cr = y / (m_max_ras_addr + 1);
				UINT16 ma = (cr << 8) | cc;

				m_update_row(this, bitmap, cliprect, ma, ra, y, m_horiz_disp, cursor_x, param);
			}
			else
			{
				m_update_row(this, bitmap, cliprect, m_current_disp_addr, ra, y, m_horiz_disp, cursor_x, param);
			}

			/* update MA if the last raster address */
			if (ra == m_max_ras_addr)
				m_current_disp_addr = (m_current_disp_addr + m_horiz_disp) & 0x3fff;
		}

		/* call the tear down function if any */
		if (m_end_update != NULL)
			m_end_update(this, bitmap, cliprect, param);
	}
	else
		popmessage("Invalid MC6845 screen parameters - display disabled!!!");
}


void mc6845_device::device_start()
{
	assert(m_clock > 0);
	assert(m_hpixels_per_column > 0);

	/* resolve callbacks */
	m_res_out_de_func.resolve(m_out_de_func, *this);
	m_res_out_cur_func.resolve(m_out_cur_func, *this);
	m_res_out_hsync_func.resolve(m_out_hsync_func, *this);
	m_res_out_vsync_func.resolve(m_out_vsync_func, *this);

	/* get the screen device */
	if ( m_screen_tag != NULL )
	{
		m_screen = downcast<screen_device *>(machine().device(m_screen_tag));
		if (m_screen == NULL) {
			astring tempstring;
			m_screen = downcast<screen_device *>(machine().device(owner()->subtag(tempstring,m_screen_tag)));
		}
		assert(m_screen != NULL);
	}
	else
		m_screen = NULL;

	/* create the timers */
	m_line_timer = timer_alloc(TIMER_LINE);
	m_de_off_timer = timer_alloc(TIMER_DE_OFF);
	m_cur_on_timer = timer_alloc(TIMER_CUR_ON);
	m_cur_off_timer = timer_alloc(TIMER_CUR_OFF);
	m_hsync_on_timer = timer_alloc(TIMER_HSYNC_ON);
	m_hsync_off_timer = timer_alloc(TIMER_HSYNC_OFF);
	m_light_pen_latch_timer = timer_alloc(TIMER_LIGHT_PEN_LATCH);
	m_upd_adr_timer = timer_alloc(TIMER_UPD_ADR);
	m_upd_trans_timer = timer_alloc(TIMER_UPD_TRANS);

	/* Use some large startup values */
	m_horiz_char_total = 0xff;
	m_max_ras_addr = 0x1f;
	m_vert_char_total = 0x7f;

	m_supports_disp_start_addr_r = true;
	m_supports_vert_sync_width = false;
	m_supports_status_reg_d5 = false;
	m_supports_status_reg_d6 = false;
	m_supports_status_reg_d7 = false;
	m_supports_transparent = false;
	m_has_valid_parameters = false;
	m_line_enable_ff = false;
	m_vsync_ff = 0;
	m_raster_counter = 0;
	m_adjust_active = 0;
	m_horiz_sync_pos = 1;
	m_vert_sync_pos = 1;
	m_de = 0;
	m_sync_width = 1;
	m_vert_disp = 0;
	m_horiz_pix_total = m_vert_pix_total = 0;
	m_max_visible_x = m_max_visible_y = 0;
	m_hsync_on_pos = m_vsync_on_pos = 0;
	m_hsync_off_pos = m_vsync_off_pos = 0;
	m_vsync = m_hsync = 0;
	m_line_counter = 0;
	m_horiz_disp = m_vert_disp = 0;
	m_vert_sync_pos = 0;
	m_vert_total_adj = 0;
	m_cursor_start_ras = m_cursor_end_ras = 0;
	m_cursor_blink_count = 0;
	m_cursor_state = 0;
	m_update_ready_bit = 0;
	m_cursor_state = 0;
	m_raster_counter = 0;
	m_cursor_start_ras = 0;
	m_cursor_end_ras = m_cursor_addr = 0;
	m_line_address = 0;
	m_current_disp_addr = 0;
	m_line_address = 0;

	save_item(NAME(m_hpixels_per_column));
	save_item(NAME(m_register_address_latch));
	save_item(NAME(m_horiz_char_total));
	save_item(NAME(m_horiz_disp));
	save_item(NAME(m_horiz_sync_pos));
	save_item(NAME(m_sync_width));
	save_item(NAME(m_vert_char_total));
	save_item(NAME(m_vert_total_adj));
	save_item(NAME(m_vert_disp));
	save_item(NAME(m_vert_sync_pos));
	save_item(NAME(m_mode_control));
	save_item(NAME(m_max_ras_addr));
	save_item(NAME(m_cursor_start_ras));
	save_item(NAME(m_cursor_end_ras));
	save_item(NAME(m_disp_start_addr));
	save_item(NAME(m_cursor_addr));
	save_item(NAME(m_light_pen_addr));
	save_item(NAME(m_light_pen_latched));
	save_item(NAME(m_cursor_state));
	save_item(NAME(m_cursor_blink_count));
	save_item(NAME(m_update_addr));
	save_item(NAME(m_update_ready_bit));
	save_item(NAME(m_cur));
	save_item(NAME(m_hsync));
	save_item(NAME(m_vsync));
	save_item(NAME(m_de));
	save_item(NAME(m_character_counter));
	save_item(NAME(m_hsync_width_counter));
	save_item(NAME(m_line_counter));
	save_item(NAME(m_raster_counter));
	save_item(NAME(m_adjust_counter));
	save_item(NAME(m_vsync_width_counter));
	save_item(NAME(m_line_enable_ff));
	save_item(NAME(m_vsync_ff));
	save_item(NAME(m_adjust_active));
	save_item(NAME(m_line_address));
	save_item(NAME(m_cursor_x));
	save_item(NAME(m_has_valid_parameters));
}


void mc6845_1_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = true;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = false;
	m_supports_status_reg_d6 = false;
	m_supports_status_reg_d7 = false;
	m_supports_transparent = false;
}


void c6545_1_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = false;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = true;
	m_supports_status_reg_d6 = true;
	m_supports_status_reg_d7 = false;
	m_supports_transparent = false;
}


void r6545_1_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = false;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = true;
	m_supports_status_reg_d6 = true;
	m_supports_status_reg_d7 = true;
	m_supports_transparent = true;
}


void h46505_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = false;
	m_supports_vert_sync_width = false;
	m_supports_status_reg_d5 = false;
	m_supports_status_reg_d6 = false;
	m_supports_status_reg_d7 = false;
	m_supports_transparent = false;
}


void hd6845_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = false;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = false;
	m_supports_status_reg_d6 = false;
	m_supports_status_reg_d7 = false;
	m_supports_transparent = false;
}


void sy6545_1_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = false;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = true;
	m_supports_status_reg_d6 = true;
	m_supports_status_reg_d7 = true;
	m_supports_transparent = true;
}


void sy6845e_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = false;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = true;
	m_supports_status_reg_d6 = true;
	m_supports_status_reg_d7 = true;
	m_supports_transparent = true;
}


void hd6345_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = true;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = true;
	m_supports_status_reg_d6 = true;
	m_supports_status_reg_d7 = true;
	m_supports_transparent = true;
}


void mc6845_device::device_reset()
{
	/* internal registers other than status remain unchanged, all outputs go low */
	if ( !m_res_out_de_func.isnull() )
		m_res_out_de_func( FALSE );

	if ( !m_res_out_hsync_func.isnull() )
		m_res_out_hsync_func( FALSE );

	if ( !m_res_out_vsync_func.isnull() )
		m_res_out_vsync_func( FALSE );

	if ( ! m_line_timer->enabled( ) )
	{
		m_line_timer->adjust( attotime::from_ticks( m_horiz_char_total + 1, m_clock ) );
	}

	m_light_pen_latched = false;

	m_cursor_addr = 0;
	m_line_address = 0;
	m_horiz_disp = 0;
	m_cursor_x = 0;
}


void r6545_1_device::device_reset() { mc6845_device::device_reset(); }
void h46505_device::device_reset() { mc6845_device::device_reset(); }
void mc6845_1_device::device_reset() { mc6845_device::device_reset(); }
void hd6845_device::device_reset() { mc6845_device::device_reset(); }
void c6545_1_device::device_reset() { mc6845_device::device_reset(); }
void sy6545_1_device::device_reset() { mc6845_device::device_reset(); }
void sy6845e_device::device_reset() { mc6845_device::device_reset(); }
void hd6345_device::device_reset() { mc6845_device::device_reset(); }


r6545_1_device::r6545_1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mc6845_device(mconfig, R6545_1, "R6545-1", tag, owner, clock)
{
}


h46505_device::h46505_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mc6845_device(mconfig, H46505, "H46505", tag, owner, clock)
{
}


mc6845_1_device::mc6845_1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mc6845_device(mconfig, MC6845_1, "MC6845-1", tag, owner, clock)
{
}


hd6845_device::hd6845_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mc6845_device(mconfig, HD6845, "HD6845", tag, owner, clock)
{
}


c6545_1_device::c6545_1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mc6845_device(mconfig, C6545_1, "C6545-1", tag, owner, clock)
{
}


sy6545_1_device::sy6545_1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mc6845_device(mconfig, SY6545_1, "SY6545-1", tag, owner, clock)
{
}


sy6845e_device::sy6845e_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mc6845_device(mconfig, SY6845E, "SY6845E", tag, owner, clock)
{
}


hd6345_device::hd6345_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: mc6845_device(mconfig, HD6345, "HD6345", tag, owner, clock)
{
}
