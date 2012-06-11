 /*
   emulation of Seta sprite chips
   X1-001A  X1-002A (SDIP64)

   these always seem to be used as a pair, some board have been seen without the
   'A', so it's probably a chip revision / bugfix.

  used by:

  seta.c
  taito_x.c
  tnzs.c
  srmp2.c
  champbwl.c
  cchance.c

  note: the data bus is almost certainly 8-bit, dating back to the earliest
        hardware the games were used on.  the RAM arrangements changes
        slightly between games depending on how the RAM is hooked up to the
        main cpu.

  'y' low bits are NEVER buffered?

 */

#include "emu.h"
#include "seta001.h"


const device_type SETA001_SPRITE = &device_creator<seta001_device>;

seta001_device::seta001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SETA001_SPRITE, "seta001_device", tag, owner, clock)
{
}

void seta001_device::device_start()
{
	// chukatai draws a column on the left from uninitialized RAM which causes garbage in a debug build
	// if we initialize ram this is a single line in the top left instead.
	// maybe there is less RAM actually present and it should mirror, or there is another flaw?
	memset(m_spritectrl,0xff,4);
	memset(m_spriteylow,0xff,0x300);
	memset(m_spritecodelow,0xff,0x2000);
	memset(m_spritecodehigh,0xff,0x2000);


	m_fg_flipxoffs = 0;
	m_fg_noflipxoffs = 0;

	m_fg_flipyoffs = 0;
	m_fg_noflipyoffs = 0;

	m_bg_flipyoffs = 0;
	m_bg_noflipyoffs = 0;

	m_bg_flipxoffs = 0;
	m_bg_noflipxoffs = 0;

	m_transpen = 0;
	m_colorbase = 0;

	m_spritelimit = 0x1ff;

	m_bgflag = 0x00;

	m_bankcallback = NULL;

	save_pointer(NAME(m_spriteylow), 0x300);
	save_pointer(NAME(m_spritecodelow), 0x2000);
	save_pointer(NAME(m_spritecodehigh), 0x2000);


}

void seta001_device::device_reset()
{

}

READ16_DEVICE_HANDLER( spritectrl_r16 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spritectrl[offset];
}

WRITE16_DEVICE_HANDLER( spritectrl_w16 )
{
	seta001_device *dev = (seta001_device *)device;

	if (ACCESSING_BITS_0_7)
	{
		dev->m_spritectrl[offset] = data;
	}
}

READ8_DEVICE_HANDLER( spritectrl_r8 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spritectrl[offset];
}

WRITE8_DEVICE_HANDLER( spritectrl_w8 )
{
	seta001_device *dev = (seta001_device *)device;
	dev->m_spritectrl[offset] = data;
}

READ16_DEVICE_HANDLER( spriteylow_r16 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spriteylow[offset];
}

WRITE16_DEVICE_HANDLER( spriteylow_w16 )
{
	seta001_device *dev = (seta001_device *)device;

	if (ACCESSING_BITS_0_7)
	{
		dev->m_spriteylow[offset] = data;
	}
}

READ8_DEVICE_HANDLER( spriteylow_r8 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spriteylow[offset];
}

WRITE8_DEVICE_HANDLER( spriteylow_w8 )
{
	seta001_device *dev = (seta001_device *)device;
	dev->m_spriteylow[offset] = data;
}


READ8_DEVICE_HANDLER( spritecodelow_r8 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spritecodelow[offset];
}

WRITE8_DEVICE_HANDLER( spritecodelow_w8 )
{
	seta001_device *dev = (seta001_device *)device;
	dev->m_spritecodelow[offset] = data;
}

READ8_DEVICE_HANDLER( spritecodehigh_r8 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spritecodehigh[offset];
}

WRITE8_DEVICE_HANDLER( spritecodehigh_w8 )
{
	seta001_device *dev = (seta001_device *)device;
	dev->m_spritecodehigh[offset] = data;
}

READ16_DEVICE_HANDLER( spritecode_r16 )
{
	seta001_device *dev = (seta001_device *)device;
	UINT16 ret = dev->m_spritecodelow[offset];
	ret |= dev->m_spritecodehigh[offset] << 8;
	return ret;
}

WRITE16_DEVICE_HANDLER( spritecode_w16 )
{
	seta001_device *dev = (seta001_device *)device;
	if (ACCESSING_BITS_0_7) dev->m_spritecodelow[offset] = data & 0x00ff;
	if (ACCESSING_BITS_8_15)  dev->m_spritecodehigh[offset] = (data & 0xff00)>>8;
}

WRITE8_DEVICE_HANDLER( spritebgflag_w8 )
{
	seta001_device *dev = (seta001_device *)device;
	dev->m_bgflag = data;
}

/***************************************************************************


                                Sprites Drawing


***************************************************************************/

/*
twineagl:   10 2d 0f 10   (ship)
tndrcade:   58 2d 07 18   (start of game - yes, flip on!)
arbalest:   18 2d 0f 10   (logo)
metafox :   18 21 0f f0   (bomb)
zingzip :   10 2c 00 0f   (bomb)
wrofaero:   10 21 00 ff   (test mode)
thunderl:   10 6c 00 ff   (always?)
krzybowl:   11 28 c0 ff   (game)
kiwame  :   16 21 7f 00   (logo)
oisipuzl:   59 20 00 00   (game - yes, flip on!)
superman:   10 21 07 38   (game)
twineagl:   00 27 00 0f   (test mode)
doraemon:   19 2a 00 03   (always)
*/




void seta001_device::seta001_draw_background( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size, int setac_type)
{
	int transpen;

	int offs, col;
	int xoffs, yoffs;

	int total_color_codes	=	machine.config().m_gfxdecodeinfo[0].total_color_codes;

	int ctrl	=	m_spritectrl[0];
	int ctrl2	=	m_spritectrl[1];

	int flip	=	ctrl & 0x40;
	int numcol	=	ctrl2 & 0x0f;

	int scrollx, scrolly;

	UINT32 upper;

	UINT8* scrollram = m_spriteylow+0x200;

	/* Sprites Banking and/or Sprites Buffering */
	UINT16 bank = ( ((ctrl2 ^ (~ctrl2<<1)) & 0x40) ? bank_size : 0 );

	int max_y	=	0xf0;

	// HACKS
	// used in conjuntion with setac_type
	int col0;		/* Kludge, needed for krzybowl and kiwame */
	switch (ctrl & 0x0f)
	{
		case 0x01:	col0	=	0x4;	break;	// krzybowl
		case 0x09:	col0	=	0x4;	break;	// doraemon
		case 0x06:	col0	=	0x8;	break;	// kiwame

		default:	col0	=	0x0;
	}


	xoffs = flip ? m_bg_flipxoffs : m_bg_noflipxoffs;
	yoffs = flip ? m_bg_flipyoffs : m_bg_noflipyoffs;


	if (m_bgflag & 0x80)
		transpen = -1;
	else
		transpen = m_transpen;

	if (numcol == 1)
		numcol = 16;

	upper = m_spritectrl[2] + m_spritectrl[3] * 256;

	for (col = 0; col < numcol; col++)
	{

		scrollx = scrollram[col * 0x10 + 4];
		scrolly = scrollram[col * 0x10];

		/* draw this column */
		for ( offs = 0 ; offs < 0x20; offs += 1 )
		{
			int i;
			// HACKS
			if (setac_type) i = ((col+col0)&0xf) * 32 + offs;
			else i = 32 * (col ^ 8) + 2 * (offs>>1) + (offs&1);

			int code = ((m_spritecodehigh[i+0x400+bank]) << 8) | m_spritecodelow[i+0x400+bank];
			int color =((m_spritecodehigh[i+0x600+bank]) << 8) | m_spritecodelow[i+0x600+bank];

			int	flipx	=	code & 0x8000;
			int	flipy	=	code & 0x4000;

			int sx		=	  scrollx + xoffs  + (offs & 1) * 16;
			int sy		=	-(scrolly + yoffs) + (offs / 2) * 16;

			if (upper & (1 << col))	sx -= 256;

			if (flip)
			{
				sy = max_y - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			color	=	( color >> (16-5) ) % total_color_codes;
			code &= 0x3fff;

			drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff),((sy) & 0x0ff),
					transpen);

			drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff)-512,((sy) & 0x0ff),
					transpen);

			drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff),((sy) & 0x0ff)-256,
					transpen);

			drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
					code,
					color,
					flipx,flipy,
					((sx) & 0x1ff)-512,((sy) & 0x0ff)-256,
					transpen);

		}
	}
}


void seta001_device::seta001_draw_foreground( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size)
{
	int screenflip = (m_spritectrl[0] & 0x40) >> 6;
	int i;
	int ctrl2 = m_spritectrl[1];
	int xoffs, yoffs;

	int total_color_codes	=	machine.config().m_gfxdecodeinfo[0].total_color_codes;

	UINT8 *char_pointer = m_spritecodelow + 0x0000;
	UINT8 *x_pointer = m_spritecodelow + 0x0200;
	UINT8 *ctrl_pointer = m_spritecodehigh + 0x0000;
	UINT8 *color_pointer = m_spritecodehigh + 0x0200;

	xoffs	=	screenflip ? m_fg_flipxoffs : m_fg_noflipxoffs;
	yoffs	=	screenflip ? m_fg_flipyoffs : m_fg_noflipyoffs;

	if ((ctrl2 ^ (~ctrl2 << 1)) & 0x40)
	{
		char_pointer += bank_size;
		x_pointer += bank_size;
		ctrl_pointer += bank_size;
		color_pointer += bank_size;
	}

	int max_y = machine.primary_screen->height();


	/* Draw up to 512 sprites, mjyuugi has glitches if you draw them all.. */
	for (i = m_spritelimit; i >= 0; i--)
	{
		int code, color, sx, sy, flipx, flipy;

		code = char_pointer[i] + ((ctrl_pointer[i] & 0x3f) << 8);
		color = (color_pointer[i] & 0xf8) >> 3;


		sx = x_pointer[i] - ((color_pointer[i] & 1) << 8);
		sy =  (m_spriteylow[i] & 0xff);
		flipx = ctrl_pointer[i] & 0x80;
		flipy = ctrl_pointer[i] & 0x40;

		if (m_bankcallback) code = m_bankcallback(machine, code, color_pointer[i]);

		color %= total_color_codes;

		color += m_colorbase;

		if (screenflip)
		{
			sy = max_y - sy
				+(machine.primary_screen->height() - (machine.primary_screen->visible_area().max_y + 1));
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff),
				max_y - ((sy + yoffs) & 0x0ff),m_transpen);

		/* wrap around x */
		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff) - 512,
				max_y - ((sy + yoffs) & 0x0ff),m_transpen);


		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff),
				max_y - ((sy + yoffs) & 0x0ff)-256,m_transpen);

		/* wrap around x */
		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				code,
				color,
				flipx,flipy,
				((sx + xoffs)&0x1ff) - 512,
				max_y - ((sy + yoffs) & 0x0ff)-256,m_transpen);

	}
}



void seta001_device::setac_eof()
{
	// is this handling right?
	// it differs to tnzs, and thundercade has sprite flickering issues (not related to the devicification)

	int ctrl2	=	m_spritectrl[1];

	if (~ctrl2 & 0x20)
	{
		if (ctrl2 & 0x40)
		{
			memcpy( &m_spritecodelow[0x0000], &m_spritecodelow[0x1000],0x800);
			memcpy(&m_spritecodehigh[0x0000],&m_spritecodehigh[0x1000],0x800);
		}
		else
		{
			memcpy( &m_spritecodelow[0x1000], &m_spritecodelow[0x0000],0x800);
			memcpy(&m_spritecodehigh[0x1000],&m_spritecodehigh[0x0000],0x800);
		}
	}
}

void seta001_device::tnzs_eof( void )
{
	int ctrl2 =	m_spritectrl[1];
	if (~ctrl2 & 0x20)
	{
		// note I copy sprites only. setac implementation also copies the "floating tilemap"
		if (ctrl2 & 0x40)
		{
			memcpy( &m_spritecodelow[0x0000],  &m_spritecodelow[0x0800], 0x0400);
			memcpy(&m_spritecodehigh[0x0000], &m_spritecodehigh[0x0800], 0x0400);
		}
		else
		{
			memcpy( &m_spritecodelow[0x0800],  &m_spritecodelow[0x0000], 0x0400);
			memcpy(&m_spritecodehigh[0x0800], &m_spritecodehigh[0x0000], 0x0400);
		}

		// and I copy the "floating tilemap" BACKWARDS - this fixes kabukiz
		memcpy( &m_spritecodelow[0x0400],  &m_spritecodelow[0x0c00], 0x0400);
		memcpy(&m_spritecodehigh[0x0400], &m_spritecodehigh[0x0c00], 0x0400);
	}

}

void seta001_device::seta001_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size, int setac)
{
	seta001_draw_background(machine, bitmap, cliprect, bank_size, setac);
	seta001_draw_foreground(machine, bitmap, cliprect, bank_size);
}
