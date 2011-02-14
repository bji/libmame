#include "emu.h"
#include "includes/rollrace.h"



#define	RA_FGCHAR_BASE	0
#define RA_BGCHAR_BASE	4
#define RA_SP_BASE	5

WRITE8_HANDLER( rollrace_charbank_w)
{
	rollrace_state *state = space->machine->driver_data<rollrace_state>();

	state->ra_charbank[offset&1] = data;
	state->ra_chrbank = state->ra_charbank[0] | (state->ra_charbank[1] << 1) ;
}


WRITE8_HANDLER( rollrace_bkgpen_w)
{
	rollrace_state *state = space->machine->driver_data<rollrace_state>();
	state->ra_bkgpen = data;
}

WRITE8_HANDLER(rollrace_spritebank_w)
{
	rollrace_state *state = space->machine->driver_data<rollrace_state>();
	state->ra_spritebank = data;
}

WRITE8_HANDLER(rollrace_backgroundpage_w)
{
	rollrace_state *state = space->machine->driver_data<rollrace_state>();

	state->ra_bkgpage = data & 0x1f;
	state->ra_bkgflip = ( data & 0x80 ) >> 7;

	/* 0x80 flip vertical */
}

WRITE8_HANDLER( rollrace_backgroundcolor_w )
{
	rollrace_state *state = space->machine->driver_data<rollrace_state>();
	state->ra_bkgcol = data;
}

WRITE8_HANDLER( rollrace_flipy_w )
{
	rollrace_state *state = space->machine->driver_data<rollrace_state>();
	state->ra_flipy = data & 0x01;
}

WRITE8_HANDLER( rollrace_flipx_w )
{
	rollrace_state *state = space->machine->driver_data<rollrace_state>();
	state->ra_flipx = data & 0x01;
}

VIDEO_UPDATE( rollrace )
{
	rollrace_state *state = screen->machine->driver_data<rollrace_state>();
	UINT8 *spriteram = screen->machine->generic.spriteram.u8;
	int offs;
	int sx, sy;
	int scroll;
	int col;
	const UINT8 *mem = screen->machine->region("user1")->base();

	/* fill in background colour*/
	bitmap_fill(bitmap,cliprect,state->ra_bkgpen);

	/* draw road */
	for (offs = 0x3ff; offs >= 0; offs--)
		{
			if(!(state->ra_bkgflip))
				{
				sy = ( 31 - offs / 32 ) ;
				}
			else
				sy = ( offs / 32 ) ;

			sx = ( offs%32 ) ;

			if(state->ra_flipx)
				sx = 31-sx ;

			if(state->ra_flipy)
				sy = 31-sy ;

			drawgfx_transpen(bitmap,
				cliprect,screen->machine->gfx[RA_BGCHAR_BASE],
				mem[offs + ( state->ra_bkgpage * 1024 )]
				+ ((( mem[offs + 0x4000 + ( state->ra_bkgpage * 1024 )] & 0xc0 ) >> 6 ) * 256 ) ,
				state->ra_bkgcol,
				state->ra_flipx,(state->ra_bkgflip^state->ra_flipy),
				sx*8,sy*8,0);


		}




	/* sprites */
	for ( offs = 0x80-4 ; offs >=0x0 ; offs -= 4)
	{
		int s_flipy = 0;
		int bank = 0;

		sy=spriteram[offs] - 16;
		sx=spriteram[offs+3] - 16;

		if(sx && sy)
		{

		if(state->ra_flipx)
			sx = 224 - sx;
		if(state->ra_flipy)
			sy = 224 - sy;

		if(spriteram[offs+1] & 0x80)
			s_flipy = 1;

		bank = (( spriteram[offs+1] & 0x40 ) >> 6 ) ;

		if(bank)
			bank += state->ra_spritebank;

		drawgfx_transpen(bitmap, cliprect,screen->machine->gfx[ RA_SP_BASE + bank ],
			spriteram[offs+1] & 0x3f ,
			spriteram[offs+2] & 0x1f,
			state->ra_flipx,!(s_flipy^state->ra_flipy),
			sx,sy,0);
		}
	}




	/* draw foreground characters */
	for (offs = 0x3ff; offs >= 0; offs--)
	{

		sx =  offs % 32;
		sy =  offs / 32;

		scroll = ( 8 * sy + state->colorram[2 * sx] ) % 256;
		col = state->colorram[ sx * 2 + 1 ]&0x1f;

		if (!state->ra_flipy)
		{
		   scroll = (248 - scroll) % 256;
		}

		if (state->ra_flipx) sx = 31 - sx;

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[RA_FGCHAR_BASE + state->ra_chrbank]  ,
			state->videoram[ offs ]  ,
			col,
			state->ra_flipx,state->ra_flipy,
			8*sx,scroll,0);

	}



	return 0;
}
