/****************************************************************************

    NEC V25/V35 special function registers and internal ram access

****************************************************************************/

#include "emu.h"
#include "nec.h"
#include "v25priv.h"

static UINT8 read_irqcontrol(v25_state_t *nec_state, INTSOURCES source, UINT8 priority)
{
	return	(((nec_state->pending_irq & source)		? 0x80 : 0x00)
			| ((nec_state->unmasked_irq & source)	? 0x00 : 0x40)
			| ((nec_state->bankswitch_irq & source)	? 0x10 : 0x00)
			| priority);
}

static UINT8 read_sfr(v25_state_t *nec_state, unsigned o)
{
	UINT8 ret;

	switch(o)
	{
		case 0x00: /* P0 */
			ret = nec_state->io->read_byte(V25_PORT_P0);
			break;
		case 0x08: /* P1 */
			/* P1 is combined with the interrupt lines */
			ret = ((nec_state->io->read_byte(V25_PORT_P1) & 0xF0)
					| (nec_state->nmi_state		? 0x00 : 0x01)
					| (nec_state->intp_state[0]	? 0x00 : 0x02)
					| (nec_state->intp_state[1]	? 0x00 : 0x04)
					| (nec_state->intp_state[2]	? 0x00 : 0x08));
			break;
		case 0x10: /* P2 */
			ret = nec_state->io->read_byte(V25_PORT_P2);
			break;
		case 0x38: /* PT */
			ret = nec_state->io->read_byte(V25_PORT_PT);
			break;
		case 0x4C: /* EXIC0 */
			ret = read_irqcontrol(nec_state, INTP0, nec_state->priority_intp);
			break;
		case 0x4D: /* EXIC1 */
			ret = read_irqcontrol(nec_state, INTP1, 7);
			break;
		case 0x4E: /* EXIC2 */
			ret = read_irqcontrol(nec_state, INTP2, 7);
			break;
		case 0x9C: /* TMIC0 */
			ret = read_irqcontrol(nec_state, INTTU0, nec_state->priority_inttu);
			break;
		case 0x9D: /* TMIC1 */
			ret = read_irqcontrol(nec_state, INTTU1, 7);
			break;
		case 0x9E: /* TMIC2 */
			ret = read_irqcontrol(nec_state, INTTU2, 7);
			break;
		case 0xEA: /* FLAG */
			ret = ((nec_state->F0 << 3) | (nec_state->F1 << 5));
			break;
		case 0xEB: /* PRC */
			ret = (nec_state->RAMEN ? 0x40 : 0);
			switch (nec_state->TB)
			{
				case 10:
					break;
				case 13:
					ret |= 0x04;
					break;
				case 16:
					ret |= 0x08;
					break;
				case 20:
					ret |= 0x0C;
					break;
			}
			switch (nec_state->PCK)
			{
				case 2:
					break;
				case 4:
					ret |= 0x01;
					break;
				case 8:
					ret |= 0x02;
					break;
			}
			break;
		case 0xEC: /* TBIC */
			ret = read_irqcontrol(nec_state, INTTB, 7);
			break;
		case 0xEF: /* IRQS */
			ret = nec_state->IRQS;
			break;
		case 0xFC: /* ISPR */
			ret = nec_state->ISPR;
			break;
		case 0xFF: /* IDB */
			ret = (nec_state->IDB >> 12);
			break;
		default:
			logerror("%06x: Read from special function register %02x\n",PC(nec_state),o);
			ret = 0;
	}
	return ret;
}

static UINT16 read_sfr_word(v25_state_t *nec_state, unsigned o)
{
	UINT16 ret;

	switch(o)
	{
		case 0x80:	/* TM0 */
			logerror("%06x: Warning: read back TM0\n",PC(nec_state));
			ret = nec_state->TM0;
			break;
		case 0x82: /* MD0 */
			logerror("%06x: Warning: read back MD0\n",PC(nec_state));
			ret = nec_state->MD0;
			break;
		case 0x88:	/* TM1 */
			logerror("%06x: Warning: read back TM1\n",PC(nec_state));
			ret = nec_state->TM1;
			break;
		case 0x8A: /* MD1 */
			logerror("%06x: Warning: read back MD1\n",PC(nec_state));
			ret = nec_state->MD1;
			break;
		default:
			ret = (read_sfr(nec_state, o) | (read_sfr(nec_state, o+1) << 8));
	}
	return ret;
}

static void write_irqcontrol(v25_state_t *nec_state, INTSOURCES source, UINT8 d)
{
	if(d & 0x80)
		nec_state->pending_irq |= source;
	else
		nec_state->pending_irq &= ~source;

	if(d & 0x40)
		nec_state->unmasked_irq &= ~source;
	else
		nec_state->unmasked_irq |= source;

	if(d & 0x20)
		logerror("%06x: Warning: macro service function not implemented\n",PC(nec_state));

	if(d & 0x10)
		nec_state->bankswitch_irq |= source;
	else
		nec_state->bankswitch_irq &= ~source;
}

static void write_sfr(v25_state_t *nec_state, unsigned o, UINT8 d)
{
	int tmp;
	attotime time;

	static const int timebases[4] = { 10, 13, 16, 20 };
	static const int clocks[4] = { 2, 4, 8, 0 };

	switch(o)
	{
		case 0x00: /* P0 */
			nec_state->io->write_byte(V25_PORT_P0, d);
			break;
		case 0x08: /* P1 */
			/* only the upper four bits of P1 can be used as output */
			nec_state->io->write_byte(V25_PORT_P1, d & 0xF0);
			break;
		case 0x10: /* P2 */
			nec_state->io->write_byte(V25_PORT_P2, d);
			break;
		case 0x4C: /* EXIC0 */
			write_irqcontrol(nec_state, INTP0, d);
			nec_state->priority_intp = d & 0x7;
			break;
		case 0x4D: /* EXIC1 */
			write_irqcontrol(nec_state, INTP1, d);
			break;
		case 0x4E: /* EXIC2 */
			write_irqcontrol(nec_state, INTP2, d);
			break;
		case 0x90: /* TMC0 */
			nec_state->TMC0 = d;
			if(d & 1)	/* oneshot mode */
			{
				if(d & 0x80)
				{
					tmp = nec_state->TM0 * nec_state->PCK * ((d & 0x40) ? 128 : 12 );
					time = attotime_mul(ATTOTIME_IN_HZ(nec_state->device->unscaled_clock()), tmp);
					timer_adjust_oneshot(nec_state->timers[0], time, INTTU0);
				}
				else
					timer_adjust_oneshot(nec_state->timers[0], attotime_never, 0);

				if(d & 0x20)
				{
					tmp = nec_state->MD0 * nec_state->PCK * ((d & 0x10) ? 128 : 12 );
					time = attotime_mul(ATTOTIME_IN_HZ(nec_state->device->unscaled_clock()), tmp);
					timer_adjust_oneshot(nec_state->timers[1], time, INTTU1);
				}
				else
					timer_adjust_oneshot(nec_state->timers[1], attotime_never, 0);
			}
			else	/* interval mode */
			{
				if(d & 0x80)
				{
					tmp = nec_state->MD0 * nec_state->PCK * ((d & 0x40) ? 128 : 6 );
					time = attotime_mul(ATTOTIME_IN_HZ(nec_state->device->unscaled_clock()), tmp);
					timer_adjust_periodic(nec_state->timers[0], time, INTTU0, time);
					timer_adjust_oneshot(nec_state->timers[1], attotime_never, 0);
					nec_state->TM0 = nec_state->MD0;
				}
				else
				{
					timer_adjust_oneshot(nec_state->timers[0], attotime_never, 0);
					timer_adjust_oneshot(nec_state->timers[1], attotime_never, 0);
				}
			}
			break;
		case 0x91: /* TMC1 */
			nec_state->TMC1 = d & 0xC0;
			if(d & 0x80)
			{
				tmp = nec_state->MD1 * nec_state->PCK * ((d & 0x40) ? 128 : 6 );
				time = attotime_mul(ATTOTIME_IN_HZ(nec_state->device->unscaled_clock()), tmp);
				timer_adjust_periodic(nec_state->timers[2], time, INTTU2, time);
				nec_state->TM1 = nec_state->MD1;
			}
			else
				timer_adjust_oneshot(nec_state->timers[2], attotime_never, 0);
			break;
		case 0x9C: /* TMIC0 */
			write_irqcontrol(nec_state, INTTU0, d);
			nec_state->priority_inttu = d & 0x7;
			break;
		case 0x9D: /* TMIC1 */
			write_irqcontrol(nec_state, INTTU1, d);
			break;
		case 0x9E: /* TMIC2 */
			write_irqcontrol(nec_state, INTTU2, d);
			break;
		case 0xEA: /* FLAG */
			nec_state->F0 = ((d & 0x08) == 0x08);
			nec_state->F1 = ((d & 0x20) == 0x20);
			break;
		case 0xEB: /* PRC */
			logerror("%06x: PRC set to %02x\n", PC(nec_state), d);
			nec_state->RAMEN = ((d & 0x40) == 0x40);
			nec_state->TB = timebases[(d & 0x0C) >> 2];
			nec_state->PCK = clocks[d & 0x03];
			if (nec_state->PCK == 0)
			{
				logerror("        Warning: invalid clock divider\n");
				nec_state->PCK = 8;
			}
			tmp = nec_state->PCK << nec_state->TB;
			time = attotime_mul(ATTOTIME_IN_HZ(nec_state->device->unscaled_clock()), tmp);
			timer_adjust_periodic(nec_state->timers[3], time, INTTB, time);
			logerror("        Internal RAM %sabled\n", (nec_state->RAMEN ? "en" : "dis"));
			logerror("        Time base set to 2^%d\n", nec_state->TB);
			logerror("        Clock divider set to %d\n", nec_state->PCK);
			break;
		case 0xEC: /* TBIC */
			/* time base interrupt doesn't support macro service, bank switching or priority control */
			write_irqcontrol(nec_state, INTTB, d & 0xC0);
			break;
		case 0xFF: /* IDB */
			nec_state->IDB = (d << 12) | 0xE00;
			logerror("%06x: IDB set to %02x\n",PC(nec_state),d);
			break;
		default:
			logerror("%06x: Wrote %02x to special function register %02x\n",PC(nec_state),d,o);
	}
}

static void write_sfr_word(v25_state_t *nec_state, unsigned o, UINT16 d)
{
	switch(o)
	{
		case 0x80:	/* TM0 */
			nec_state->TM0 = d;
			break;
		case 0x82: /* MD0 */
			nec_state->MD0 = d;
			break;
		case 0x88:	/* TM1 */
			nec_state->TM1 = d;
			break;
		case 0x8A: /* MD1 */
			nec_state->MD1 = d;
			break;
		default:
			write_sfr(nec_state, o, d);
			write_sfr(nec_state, o+1, d >> 8);
	}
}

UINT8 v25_read_byte(v25_state_t *nec_state, unsigned a)
{
	if((a & 0xFFE00) == nec_state->IDB || a == 0xFFFFF)
	{
		unsigned o = a & 0x1FF;

		if(nec_state->RAMEN && o < 0x100)
			return nec_state->ram.b[BYTE_XOR_LE(o)];

		if(o >= 0x100)
			return read_sfr(nec_state, o-0x100);
	}

	return nec_state->program->read_byte(a);
}

UINT16 v25_read_word(v25_state_t *nec_state, unsigned a)
{
	if( a & 1 )
		return (v25_read_byte(nec_state, a) | (v25_read_byte(nec_state, a + 1) << 8));

	if((a & 0xFFE00) == nec_state->IDB)
	{
		unsigned o = a & 0x1FF;

		if(nec_state->RAMEN && o < 0x100)
			return nec_state->ram.w[o/2];

		if(o >= 0x100)
			return read_sfr_word(nec_state, o-0x100);
	}

	if(a == 0xFFFFE)	/* not sure about this - manual says FFFFC-FFFFE are "reserved" */
		return (nec_state->program->read_byte(a) | (read_sfr(nec_state, 0xFF) << 8));

	return nec_state->program->read_word(a);
}

void v25_write_byte(v25_state_t *nec_state, unsigned a, UINT8 d)
{
	if((a & 0xFFE00) == nec_state->IDB || a == 0xFFFFF)
	{
		unsigned o = a & 0x1FF;

		if(nec_state->RAMEN && o < 0x100)
		{
			nec_state->ram.b[BYTE_XOR_LE(o)] = d;
			return;
		}

		if(o >= 0x100)
		{
			write_sfr(nec_state, o-0x100, d);
			return;
		}
	}

	nec_state->program->write_byte(a, d);
}

void v25_write_word(v25_state_t *nec_state, unsigned a, UINT16 d)
{
	if( a & 1 )
	{
		v25_write_byte(nec_state, a, d);
		v25_write_byte(nec_state, a + 1, d >> 8);
		return;
	}

	if((a & 0xFFE00) == nec_state->IDB)
	{
		unsigned o = a & 0x1FF;

		if(nec_state->RAMEN && o < 0x100)
		{
			nec_state->ram.w[o/2] = d;
			return;
		}

		if(o >= 0x100)
		{
			write_sfr_word(nec_state, o-0x100, d);
			return;
		}
	}

	if(a == 0xFFFFE)	/* not sure about this - manual says FFFFC-FFFFE are "reserved" */
	{
		nec_state->program->write_byte(a, d);
		write_sfr(nec_state, 0xFF, d >> 8);
		return;
	}

	nec_state->program->write_word(a, d);
}
