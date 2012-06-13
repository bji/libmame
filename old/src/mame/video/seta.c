/***************************************************************************

                            -= Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q           shows layer 0
        W           shows layer 1
        A           shows the sprites

        Keys can be used together!


                        [ 0, 1 Or 2 Scrolling Layers ]

    Each layer consists of 2 tilemaps: only one can be displayed at a
    given time (the games usually flip continuously between the two).
    The two tilemaps share the same scrolling registers.

        Layer Size:             1024 x 512
        Tiles:                  16x16x4 (16x16x6 in some games)
        Tile Format:

            Offset + 0x0000:
                            f--- ---- ---- ----     Flip X
                            -e-- ---- ---- ----     Flip Y
                            --dc ba98 7654 3210     Code

            Offset + 0x1000:

                            fedc ba98 765- ----     -
                            ---- ---- ---4 3210     Color

            The other tilemap for this layer (always?) starts at
            Offset + 0x2000.


                            [ 1024 Sprites ]

    Sprites are 16x16x4. They are just like those in "The Newzealand Story",
    "Revenge of DOH" etc (tnzs.c). Obviously they're hooked to a 16 bit
    CPU here, so they're mapped a bit differently in memory. Additionally,
    there are two banks of sprites. The game can flip between the two to
    do double buffering, writing to a bit of a control register(see below)


        Spriteram16_2 + 0x000.w

                        f--- ---- ---- ----     Flip X
                        -e-- ---- ---- ----     Flip Y
                        --dc ba-- ---- ----     -
                        ---- --98 7654 3210     Code (Lower bits)

        Spriteram16_2 + 0x400.w

                        fedc b--- ---- ----     Color
                        ---- -a9- ---- ----     Code (Upper Bits)
                        ---- ---8 7654 3210     X

        Spriteram16   + 0x000.w

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Y



                            [ Floating Tilemap ]

    There's a floating tilemap made of vertical colums composed of 2x16
    "sprites". Each 32 consecutive "sprites" define a column.

    For column I:

        Spriteram16_2 + 0x800 + 0x40 * I:

                        f--- ---- ---- ----     Flip X
                        -e-- ---- ---- ----     Flip Y
                        --dc b--- ---- ----     -
                        ---- --98 7654 3210     Code (Lower bits)

        Spriteram16_2 + 0xc00 + 0x40 * I:

                        fedc b--- ---- ----     Color
                        ---- -a9- ---- ----     Code (Upper Bits)
                        ---- ---8 7654 3210     -

    Each column has a variable horizontal position and a vertical scrolling
    value (see also the Sprite Control Registers). For column I:


        Spriteram16   + 0x400 + 0x20 * I:

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Y

        Spriteram16   + 0x408 + 0x20 * I:

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Low Bits Of X



                        [ Sprites Control Registers ]


        Spriteram16   + 0x601.b

                        7--- ----       0
                        -6-- ----       Flip Screen
                        --5- ----       0
                        ---4 ----       1 (Sprite Enable?)
                        ---- 3210       ???

        Spriteram16   + 0x603.b

                        7--- ----       0
                        -6-- ----       Sprite Bank
                        --5- ----       0 = Sprite Buffering (blandia,msgundam,qzkklogy)
                        ---4 ----       0
                        ---- 3210       Columns To Draw (1 is the special value for 16)

        Spriteram16   + 0x605.b

                        7654 3210       High Bit Of X For Columns 7-0

        Spriteram16   + 0x607.b

                        7654 3210       High Bit Of X For Columns f-8




***************************************************************************/

#include "emu.h"
#include "sound/x1_010.h"
#include "includes/seta.h"


/* note that drgnunit, stg and qzkklogy run on the same board, yet they need different alignment */
static const game_offset game_offsets[] =
{
	/* only sprites */
	{ "tndrcade", { -1,  0 } },				// correct (wall at beginning of game)
	{ "tndrcadej",{ -1,  0 } },				// "
	{ "wits",     {  0,  0 } },				// unknown
	{ "thunderl", {  0,  0 } },				// unknown
	{ "wiggie",   {  0,  0 } },				// some problems but they seem y co-ordinate related?
	{ "superbar", {  0,  0 } },				// "
	{ "pairlove", {  0,  0 } },				// unknown
	{ "blockcar", {  0,  0 } },				// unknown
	{ "neobattl", {  0,  0 } },				// correct (test grid)
	{ "umanclub", {  0,  0 } },				// correct (test grid)
	{ "atehate",  {  0,  0 } },				// correct (test grid)
	{ "kiwame",   {  0,-16 } },				// correct (test grid)
	{ "krzybowl", {  0,  0 } },				// correct (test grid)
	{ "orbs",     {  0,  0 } },				// unknown
	{ "keroppi",  {  0,  0 } },				// unknown

	/* 1 layer */
	{ "twineagl", {  0,  0 }, {  0, -3 } },	// unknown
	{ "downtown", {  1,  0 }, { -1,  0 } },	// sprites correct (test grid), tilemap unknown but at least -1 non-flipped to fix glitches later in the game
	{ "downtown2",{  1,  0 }, { -1,  0 } },	// "
	{ "downtownj",{  1,  0 }, { -1,  0 } },	// "
	{ "downtownp",{  1,  0 }, { -1,  0 } },	// "
	{ "usclssic", {  1,  2 }, {  0, -1 } },	// correct (test grid and bg)
	{ "calibr50", { -1,  2 }, { -3, -2 } },	// correct (test grid and roof in animation at beginning of game)
	{ "arbalest", {  0,  1 }, { -2, -1 } },	// correct (test grid and landing pad at beginning of game)
	{ "metafox",  {  0,  0 }, { 16,-19 } },	// sprites unknown, tilemap correct (test grid)
	{ "setaroul", {  0,  0 }, {  0,  0 } },	// unknown
	{ "drgnunit", {  2,  2 }, { -2, -2 } },	// correct (test grid and I/O test)
	{ "jockeyc",  {  0,  0 }, { -2,  0 } },	// sprites unknown, tilemap correct (test grid)
	{ "inttoote", {  0,  0 }, { -2,  0 } },	// "
	{ "inttootea",{  0,  0 }, { -2,  0 } },	// "
	{ "stg",      {  0,  0 }, { -2, -2 } },	// sprites correct? (panel), tilemap correct (test grid)
	{ "qzkklogy", {  1,  1 }, { -1, -1 } },	// correct (timer, test grid)
	{ "qzkklgy2", {  0,  0 }, { -1, -3 } },	// sprites unknown, tilemaps correct (test grid)

	/* 2 layers */
	{ "rezon",    {  0,  0 }, { -2, -2 } },	// correct (test grid)
	{ "rezont",   {  0,  0 }, { -2, -2 } },	// "
	{ "blandia",  {  0,  8 }, { -2,  6 } },	// correct (test grid, startup bg)
	{ "blandiap", {  0,  8 }, { -2,  6 } },	// "
	{ "zingzip",  {  0,  0 }, { -1, -2 } },	// sprites unknown, tilemaps correct (test grid)
	{ "eightfrc", {  3,  4 }, {  0,  0 } },	// unknown
	{ "daioh",    {  1,  1 }, { -1, -1 } },	// correct? (launch window and test grid are right, but planet is wrong)
	{ "msgundam", {  0,  0 }, { -2, -2 } },	// correct (test grid, banpresto logo)
	{ "msgundam1",{  0,  0 }, { -2, -2 } },	// "
	{ "oisipuzl", {  0,  0 }, { -1, -1 } },	// correct (test mode) flip screen not supported?
	{ "triplfun", {  0,  0 }, { -1, -1 } },	// "
	{ "wrofaero", {  0,  0 }, {  0,  0 } },	// unknown
	{ "jjsquawk", {  1,  1 }, { -1, -1 } },	// correct (test mode)
	{ "jjsquawkb",{  1,  1 }, { -1, -1 } },	// "
	{ "kamenrid", {  0,  0 }, { -2, -2 } },	// correct (map, banpresto logo)
	{ "extdwnhl", {  0,  0 }, { -2, -2 } },	// correct (test grid, background images)
	{ "sokonuke", {  0,  0 }, { -2, -2 } },	// correct (game selection, test grid)
	{ "gundhara", {  0,  0 }, {  0,  0 } },	// unknown, flip screen not supported?
	{ "zombraid", {  0,  0 }, { -2, -2 } },	// correct for normal, flip screen not working yet
	{ "madshark", {  0,  0 }, {  0,  0 } },	// unknown (wrong when flipped, but along y)
	{ "utoukond", {  0,  0 }, { -2,  0 } }, // unknown (wrong when flipped, but along y)
	{ "crazyfgt", {  0,  0 }, { -2,  0 } }, // wrong (empty background column in title screen, but aligned sprites in screen select)

	{ NULL }
};


/*  ---- 3---       Coin #1 Lock Out
    ---- -2--       Coin #0 Lock Out
    ---- --1-       Coin #1 Counter
    ---- ---0       Coin #0 Counter     */

void seta_coin_lockout_w(running_machine *machine, int data)
{
	seta_state *state = machine->driver_data<seta_state>();
	static const char *const seta_nolockout[8] = { "blandia", "gundhara", "kamenrid", "zingzip", "eightfrc", "extdwnhl", "sokonuke", "zombraid"};

	/* Only compute seta_coin_lockout when confronted with a new gamedrv */
	if (state->driver != machine->gamedrv)
	{
		int i;
		state->driver = machine->gamedrv;

		state->coin_lockout = 1;
		for (i=0; i<ARRAY_LENGTH(seta_nolockout); i++)
		{
			if (strcmp(state->driver->name, seta_nolockout[i]) == 0 ||
				strcmp(state->driver->parent, seta_nolockout[i]) == 0)
			{
				state->coin_lockout = 0;
				break;
			}
		}
	}

	coin_counter_w		(machine, 0, (( data) >> 0) & 1 );
	coin_counter_w		(machine, 1, (( data) >> 1) & 1 );

	/* blandia, gundhara, kamenrid & zingzip haven't the coin lockout device */
	if (	!state->coin_lockout )
		return;
	coin_lockout_w		(machine, 0, ((~data) >> 2) & 1 );
	coin_lockout_w		(machine, 1, ((~data) >> 3) & 1 );
}


WRITE16_HANDLER( seta_vregs_w )
{
	seta_state *state = space->machine->driver_data<seta_state>();
	COMBINE_DATA(&state->vregs[offset]);
	switch (offset)
	{
		case 0/2:

/*      fedc ba98 76-- ----
        ---- ---- --5- ----     Sound Enable
        ---- ---- ---4 ----     toggled in IRQ1 by many games, irq acknowledge?
                                [original comment for the above: ?? 1 in oisipuzl, sokonuke (layers related)]
        ---- ---- ---- 3---     Coin #1 Lock Out
        ---- ---- ---- -2--     Coin #0 Lock Out
        ---- ---- ---- --1-     Coin #1 Counter
        ---- ---- ---- ---0     Coin #0 Counter     */
			if (ACCESSING_BITS_0_7)
			{
				device_t *x1_010 = space->machine->device("x1snd");
				seta_coin_lockout_w (space->machine, data & 0x0f);
				if (x1_010 != NULL)
					seta_sound_enable_w (x1_010, data & 0x20);
				coin_counter_w(space->machine, 0,data & 0x01);
				coin_counter_w(space->machine, 1,data & 0x02);
			}
			break;

		case 2/2:
			if (ACCESSING_BITS_0_7)
			{
				int new_bank;

				/* Partly handled in vh_screenrefresh:

                        fedc ba98 76-- ----
                        ---- ---- --54 3---     Samples Bank (in blandia, eightfrc, zombraid)
                        ---- ---- ---- -2--
                        ---- ---- ---- --1-     Sprites Above Frontmost Layer
                        ---- ---- ---- ---0     Layer 0 Above Layer 1
                */

				new_bank = (data >> 3) & 0x7;

				if (new_bank != state->samples_bank)
				{
					UINT8 *rom = space->machine->region("x1snd")->base();
					int samples_len = space->machine->region("x1snd")->bytes();
					int addr;

					state->samples_bank = new_bank;

					if (samples_len == 0x240000)	/* blandia, eightfrc */
					{
						addr = 0x40000 * new_bank;
						if (new_bank >= 3)	addr += 0x40000;

						if ( (samples_len > 0x100000) && ((addr+0x40000) <= samples_len) )
							memcpy(&rom[0xc0000],&rom[addr],0x40000);
						else
							logerror("PC %06X - Invalid samples bank %02X !\n", cpu_get_pc(space->cpu), new_bank);
					}
					else if (samples_len == 0x480000)	/* zombraid */
					{
						/* bank 1 is never explicitly selected, 0 is used in its place */
						if (new_bank == 0) new_bank = 1;
						addr = 0x80000 * new_bank;
						if (new_bank > 0) addr += 0x80000;

						memcpy(&rom[0x80000],&rom[addr],0x80000);
					}
				}

			}
			break;


		case 4/2:	// ?
			break;
	}
}




/***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

Offset + 0x0000:
                    f--- ---- ---- ----     Flip X
                    -e-- ---- ---- ----     Flip Y
                    --dc ba98 7654 3210     Code

Offset + 0x1000:

                    fedc ba98 765- ----     -
                    ---- ---- ---4 3210     Color


                      [ TileMaps Control Registers]

Offset + 0x0:                               Scroll X
Offset + 0x2:                               Scroll Y
Offset + 0x4:
                    fedc ba98 7654 3210     -
                    ---- ---- ---4 ----     Tilemap color mode switch (used in blandia and the other games using 6bpp graphics)
                    ---- ---- ---- 3---     Tilemap Select (There Are 2 Tilemaps Per Layer)
                    ---- ---- ---- -21-     0 (1 only in eightfrc, when flip is on!)
                    ---- ---- ---- ---0     ?

***************************************************************************/

INLINE void twineagl_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index, int offset )
{
	seta_state *state = machine->driver_data<seta_state>();
	UINT16 *vram = state->vram_0 + offset;
	UINT16 code =	vram[ tile_index ];
	UINT16 attr =	vram[ tile_index + 0x800 ];
	if ((code & 0x3e00) == 0x3e00)
		code = (code & 0xc07f) | ((state->twineagl_tilebank[(code & 0x0180) >> 7] >> 1) << 7);
	SET_TILE_INFO( 1, (code & 0x3fff), attr & 0x1f, TILE_FLIPXY((code & 0xc000) >> 14) );
}

static TILE_GET_INFO( twineagl_get_tile_info_0 ) { twineagl_tile_info( machine, tileinfo, tile_index, 0x0000 ); }
static TILE_GET_INFO( twineagl_get_tile_info_1 ) { twineagl_tile_info( machine, tileinfo, tile_index, 0x1000 ); }


INLINE void get_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index, int layer, int offset )
{
	seta_state *state = machine->driver_data<seta_state>();
	int gfx = 1 + layer;
	UINT16 *vram = (layer == 0) ? state->vram_0 + offset : state->vram_2 + offset;
	UINT16 *vctrl = (layer == 0) ? state->vctrl_0 : state->vctrl_2;
	UINT16 code =	vram[ tile_index ];
	UINT16 attr =	vram[ tile_index + 0x800 ];

	if(machine->gfx[gfx + ((vctrl[ 4/2 ] & 0x10) >> state->color_mode_shift)] != NULL)
	{
		gfx += (vctrl[ 4/2 ] & 0x10) >> state->color_mode_shift;
	}
	else
	{
		popmessage("Missing Color Mode = 1 for Layer = %d. Contact MAMETesters.",layer);
	}

	SET_TILE_INFO( gfx, state->tiles_offset + (code & 0x3fff), attr & 0x1f, TILE_FLIPXY((code & 0xc000) >> 14) );
}

static TILE_GET_INFO( get_tile_info_0 ) { get_tile_info( machine, tileinfo, tile_index, 0, 0x0000 ); }
static TILE_GET_INFO( get_tile_info_1 ) { get_tile_info( machine, tileinfo, tile_index, 0, 0x1000 ); }
static TILE_GET_INFO( get_tile_info_2 ) { get_tile_info( machine, tileinfo, tile_index, 1, 0x0000 ); }
static TILE_GET_INFO( get_tile_info_3 ) { get_tile_info( machine, tileinfo, tile_index, 1, 0x1000 ); }


WRITE16_HANDLER( seta_vram_0_w )
{
	seta_state *state = space->machine->driver_data<seta_state>();

	COMBINE_DATA(&state->vram_0[offset]);
	if (offset & 0x1000)
		tilemap_mark_tile_dirty(state->tilemap_1, offset & 0x7ff);
	else
		tilemap_mark_tile_dirty(state->tilemap_0, offset & 0x7ff);
}

WRITE16_HANDLER( seta_vram_2_w )
{
	seta_state *state = space->machine->driver_data<seta_state>();

	COMBINE_DATA(&state->vram_2[offset]);
	if (offset & 0x1000)
		tilemap_mark_tile_dirty(state->tilemap_3, offset & 0x7ff);
	else
		tilemap_mark_tile_dirty(state->tilemap_2, offset & 0x7ff);
}

WRITE16_HANDLER( twineagl_tilebank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		seta_state *state = space->machine->driver_data<seta_state>();
		data &= 0xff;
		if (state->twineagl_tilebank[offset] != data)
		{
			state->twineagl_tilebank[offset] = data;
			tilemap_mark_all_tiles_dirty_all(space->machine);
		}
	}
}



/* 2 layers */
VIDEO_START( seta_2_layers )
{
	seta_state *state = machine->driver_data<seta_state>();

	VIDEO_START_CALL( seta_no_layers );

	/* Each layer consists of 2 tilemaps: only one can be displayed
       at any given time */

	/* layer 0 */
	state->tilemap_0 = tilemap_create(	machine, get_tile_info_0, tilemap_scan_rows,
								 16,16, 64,32 );

	state->tilemap_1 = tilemap_create(	machine, get_tile_info_1, tilemap_scan_rows,
								 16,16, 64,32 );


	/* layer 1 */
	state->tilemap_2 = tilemap_create(	machine, get_tile_info_2, tilemap_scan_rows,
								 16,16, 64,32 );

	state->tilemap_3 = tilemap_create(	machine, get_tile_info_3, tilemap_scan_rows,
								 16,16, 64,32 );

	state->tilemaps_flip = 0;
	state->color_mode_shift = 3;

	tilemap_set_transparent_pen(state->tilemap_0, 0);
	tilemap_set_transparent_pen(state->tilemap_1, 0);
	tilemap_set_transparent_pen(state->tilemap_2, 0);
	tilemap_set_transparent_pen(state->tilemap_3, 0);
}


/* 1 layer */
VIDEO_START( seta_1_layer )
{
	seta_state *state = machine->driver_data<seta_state>();

	VIDEO_START_CALL( seta_no_layers );

	/* Each layer consists of 2 tilemaps: only one can be displayed
       at any given time */

	/* layer 0 */
	state->tilemap_0 = tilemap_create(	machine, get_tile_info_0, tilemap_scan_rows,
								 16,16, 64,32 );

	state->tilemap_1 = tilemap_create(	machine, get_tile_info_1, tilemap_scan_rows,
								 16,16, 64,32 );

	state->color_mode_shift = 4;

	tilemap_set_transparent_pen(state->tilemap_0, 0);
	tilemap_set_transparent_pen(state->tilemap_1, 0);
}

VIDEO_START( twineagl_1_layer )
{
	seta_state *state = machine->driver_data<seta_state>();

	VIDEO_START_CALL( seta_no_layers );

	/* Each layer consists of 2 tilemaps: only one can be displayed
       at any given time */

	/* layer 0 */
	state->tilemap_0 = tilemap_create(	machine, twineagl_get_tile_info_0, tilemap_scan_rows,
								 16,16, 64,32 );

	state->tilemap_1 = tilemap_create(	machine, twineagl_get_tile_info_1, tilemap_scan_rows,
								 16,16, 64,32 );

	tilemap_set_transparent_pen(state->tilemap_0, 0);
	tilemap_set_transparent_pen(state->tilemap_1, 0);
}


/* NO layers, only sprites */
VIDEO_START( seta_no_layers )
{
	seta_state *state = machine->driver_data<seta_state>();

	state->tilemap_0 = 0;
	state->tilemap_1 = 0;
	state->tilemap_2 = 0;
	state->tilemap_3 = 0;

	state->tilemaps_flip = 0;

	state->global_offsets = game_offsets;
	while (state->global_offsets->gamename && strcmp(machine->gamedrv->name, state->global_offsets->gamename))
		state->global_offsets++;
	state->samples_bank = -1;	// set the samples bank to an out of range value at start-up
}

VIDEO_START( oisipuzl_2_layers )
{
	seta_state *state = machine->driver_data<seta_state>();

	VIDEO_START_CALL(seta_2_layers);
	state->tilemaps_flip = 1;
}


/***************************************************************************


                            Palette Init Functions


***************************************************************************/


/* 2 layers, 6 bit deep.

   The game can select to repeat every 16 colors to fill the 64 colors for the 6bpp gfx
   or to use the first 64 colors of the palette regardless of the color code!
*/
PALETTE_INIT( blandia )
{
	int color, pen;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x600*2);

	for (color = 0; color < 0x20; color++)
	{
		for (pen = 0; pen < 0x40; pen++)
		{
			// layer 2-3
			colortable_entry_set_value(machine->colortable, 0x0200 + ((color << 6) | pen), 0x200 + ((color << 4) | (pen & 0x0f)));
			colortable_entry_set_value(machine->colortable, 0x1200 + ((color << 6) | pen), 0x200 + pen);

			// layer 0-1
			colortable_entry_set_value(machine->colortable, 0x0a00 + ((color << 6) | pen), 0x400 + ((color << 4) | (pen & 0x0f)));
			colortable_entry_set_value(machine->colortable, 0x1a00 + ((color << 6) | pen), 0x400 + pen);
		}
	}

	// setup the colortable for the effect palette.
	// what are used for palette from 0x800 to 0xBFF?
	for(int i = 0; i < 0x2200; i++)
	{
		colortable_entry_set_value(machine->colortable, 0x2200 + i, 0x600 + (i & 0x1ff));
	}
}



/* layers have 6 bits per pixel, but the color code has a 16 colors granularity,
   even if the low 2 bits are ignored (so there are only 4 different palettes) */
PALETTE_INIT( gundhara )
{
	int color, pen;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x600);

	for (color = 0; color < 0x20; color++)
		for (pen = 0; pen < 0x40; pen++)
		{
			colortable_entry_set_value(machine->colortable, 0x0200 + ((color << 6) | pen), 0x400 + pen); // untested
			colortable_entry_set_value(machine->colortable, 0x1200 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff));

			colortable_entry_set_value(machine->colortable, 0x0a00 + ((color << 6) | pen), 0x200 + pen); // untested
			colortable_entry_set_value(machine->colortable, 0x1a00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
}



/* layers have 6 bits per pixel, but the color code has a 16 colors granularity */
PALETTE_INIT( jjsquawk )
{
	int color, pen;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x600);

	for (color = 0; color < 0x20; color++)
		for (pen = 0; pen < 0x40; pen++)
		{
			colortable_entry_set_value(machine->colortable, 0x0200 + ((color << 6) | pen), 0x400 + pen); // untested
			colortable_entry_set_value(machine->colortable, 0x1200 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff));

			colortable_entry_set_value(machine->colortable, 0x0a00 + ((color << 6) | pen), 0x200 + pen); // untested
			colortable_entry_set_value(machine->colortable, 0x1a00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
}


/* layer 0 is 6 bit per pixel, but the color code has a 16 colors granularity */
PALETTE_INIT( zingzip )
{
	int color, pen;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x600);

	for (color = 0; color < 0x20; color++)
		for (pen = 0; pen < 0x40; pen++)
		{
			colortable_entry_set_value(machine->colortable, 0x400 + ((color << 6) | pen), 0x400 + pen); // untested
			colortable_entry_set_value(machine->colortable, 0xc00 + ((color << 6) | pen), 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
}

// color prom
PALETTE_INIT( inttoote )
{
	int x;
	for (x = 0; x < 0x200 ; x++)
	{
		int data = (color_prom[x*2] <<8) | color_prom[x*2+1];
		palette_set_color_rgb(machine, x, pal5bit(data >> 10),pal5bit(data >> 5),pal5bit(data >> 0));
	}
}

PALETTE_INIT( setaroul )
{
	machine->gfx[0]->color_granularity=16;
	machine->gfx[1]->color_granularity=16;

	PALETTE_INIT_CALL(inttoote);
}

PALETTE_INIT( usclssic )
{
	int color, pen;
	int x;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x400);

	/* DECODE PROM */
	for (x = 0; x < 0x200 ; x++)
	{
		UINT16 data = (color_prom[x*2] <<8) | color_prom[x*2+1];

		rgb_t color = MAKE_RGB(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (x >= 0x100)
			colortable_palette_set_color(machine->colortable, x + 0x000, color);
		else
			colortable_palette_set_color(machine->colortable, x + 0x300, color);
	}

	for (color = 0; color < 0x20; color++)
		for (pen = 0; pen < 0x40; pen++)
		{
			colortable_entry_set_value(machine->colortable, 0x200 + ((color << 6) | pen), 0x200 + pen); // untested
			colortable_entry_set_value(machine->colortable, 0xa00 + ((color << 6) | pen), 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff));
		}
}


static void set_pens(running_machine *machine)
{
	seta_state *state = machine->driver_data<seta_state>();
	offs_t i;

	for (i = 0; i < state->paletteram_size / 2; i++)
	{
		UINT16 data = state->paletteram[i];

		rgb_t color = MAKE_RGB(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (machine->colortable != NULL)
			colortable_palette_set_color(machine->colortable, i, color);
		else
			palette_set_color(machine, i, color);
	}

	if(state->paletteram2 != NULL)
	{
		for (i = 0; i < state->paletteram2_size / 2; i++)
		{
			UINT16 data = state->paletteram2[i];

			rgb_t color = MAKE_RGB(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

			if (machine->colortable != NULL)
				colortable_palette_set_color(machine->colortable, i + state->paletteram_size / 2, color);
			else
				palette_set_color(machine, i + state->paletteram_size / 2, color);
		}
	}
}


static void usclssic_set_pens(running_machine *machine)
{
	seta_state *state = machine->driver_data<seta_state>();
	offs_t i;

	for (i = 0; i < 0x200; i++)
	{
		UINT16 data = state->paletteram[i];

		rgb_t color = MAKE_RGB(pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));

		if (i >= 0x100)
			colortable_palette_set_color(machine->colortable, i - 0x100, color);
		else
			colortable_palette_set_color(machine->colortable, i + 0x200, color);
	}
}



/***************************************************************************


                                Sprites Drawing


***************************************************************************/


static void draw_sprites_map(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	seta_state *state = machine->driver_data<seta_state>();
	UINT16 *spriteram16 = state->spriteram;
	int offs, col;
	int xoffs, yoffs;

	int total_color_codes	=	machine->config->m_gfxdecodeinfo[0].total_color_codes;

	int ctrl	=	spriteram16[ 0x600/2 ];
	int ctrl2	=	spriteram16[ 0x602/2 ];

	int flip	=	ctrl & 0x40;
	int numcol	=	ctrl2 & 0x000f;

	/* Sprites Banking and/or Sprites Buffering */
	UINT16 *src = state->spriteram2 + ( ((ctrl2 ^ (~ctrl2<<1)) & 0x40) ? 0x2000/2 : 0 );

	int upper	=	( spriteram16[ 0x604/2 ] & 0xFF ) +
					( spriteram16[ 0x606/2 ] & 0xFF ) * 256;

	int max_y	=	0xf0;

	int col0;		/* Kludge, needed for krzybowl and kiwame */
	switch (ctrl & 0x0f)
	{
		case 0x01:	col0	=	0x4;	break;	// krzybowl
		case 0x06:	col0	=	0x8;	break;	// kiwame

		default:	col0	=	0x0;
	}

	xoffs = 0;
	yoffs = flip ? 1 : -1;

	/* Number of columns to draw - the value 1 seems special, meaning:
       draw every column */
	if (numcol == 1)
		numcol = 16;


	/* The first column is the frontmost, see twineagl test mode
        BM 071204 - first column frontmost breaks superman.
    */
//  for ( col = numcol - 1 ; col >= 0; col -- )
	for ( col = 0 ; col < numcol; col ++ )
	{
		int x	=	spriteram16[(col * 0x20 + 0x08 + 0x400)/2] & 0xff;
		int y	=	spriteram16[(col * 0x20 + 0x00 + 0x400)/2] & 0xff;

		/* draw this column */
		for ( offs = 0 ; offs < 0x40/2; offs += 2/2 )
		{
			int	code	=	src[((col+col0)&0xf) * 0x40/2 + offs + 0x800/2];
			int	color	=	src[((col+col0)&0xf) * 0x40/2 + offs + 0xc00/2];

			int	flipx	=	code & 0x8000;
			int	flipy	=	code & 0x4000;

			int bank	=	(color & 0x0600) >> 9;

/*
twineagl:   010 02d 0f 10   (ship)
tndrcade:   058 02d 07 18   (start of game - yes, flip on!)
arbalest:   018 02d 0f 10   (logo)
metafox :   018 021 0f f0   (bomb)
zingzip :   010 02c 00 0f   (bomb)
wrofaero:   010 021 00 ff   (test mode)
thunderl:   010 06c 00 ff   (always?)
krzybowl:   011 028 c0 ff   (game)
kiwame  :   016 021 7f 00   (logo)
oisipuzl:   059 020 00 00   (game - yes, flip on!)

superman:   010 021 07 38   (game)
twineagl:   000 027 00 0f   (test mode)
*/

			int sx		=	  x + xoffs  + (offs & 1) * 16;
			int sy		=	-(y + yoffs) + (offs / 2) * 16;

			if (upper & (1 << col))	sx += 256;

			if (flip)
			{
				sy = max_y - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			color	=	( color >> (16-5) ) % total_color_codes;
			code	=	(code & 0x3fff) + (bank * 0x4000);

			drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
					code,
					color,
					flipx, flipy,
					((sx + 0x10) & 0x1ff) - 0x10,((sy + 8) & 0x0ff) - 8,0);
		}
	/* next column */
	}

}



static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	seta_state *state = machine->driver_data<seta_state>();
	UINT16 *spriteram16 = state->spriteram;
	int offs;
	int xoffs, yoffs;

	int total_color_codes	=	machine->config->m_gfxdecodeinfo[0].total_color_codes;

	int ctrl	=	spriteram16[ 0x600/2 ];
	int ctrl2	=	spriteram16[ 0x602/2 ];

	int flip	=	ctrl & 0x40;

	/* Sprites Banking and/or Sprites Buffering */
	UINT16 *src = state->spriteram2 + ( ((ctrl2 ^ (~ctrl2<<1)) & 0x40) ? 0x2000/2 : 0 );

	int max_y	=	0xf0;


	draw_sprites_map(machine,bitmap,cliprect);


	xoffs = state->global_offsets->sprite_offs[flip ? 1 : 0];
	yoffs = -2;

	for ( offs = (0x400-2)/2 ; offs >= 0/2; offs -= 2/2 )
	{
		int	code	=	src[offs + 0x000/2];
		int	x		=	src[offs + 0x400/2];

		int	y		=	spriteram16[offs + 0x000/2] & 0xff;

		int	flipx	=	code & 0x8000;
		int	flipy	=	code & 0x4000;

		int bank	=	(x & 0x0600) >> 9;
		int color	=	( x >> (16-5) ) % total_color_codes;

		if (flip)
		{
			y = (0x100 - machine->primary_screen->height()) + max_y - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		code = (code & 0x3fff) + (bank * 0x4000);

		y = max_y - y;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				code,
				color,
				flipx, flipy,
				((x + xoffs + 0x10) & 0x1ff) - 0x10,((y - yoffs + 8) & 0x0ff) - 8,0);
	}

}

static void draw_tilemap_palette_effect(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, tilemap_t *tilemap, int scrollx, int scrolly, int gfxnum, int flipscreen)
{
	int y;
	const gfx_element *gfx_tilemap = machine->gfx[gfxnum];
	const bitmap_t *src_bitmap = tilemap_get_pixmap(tilemap);
	int width_mask, height_mask;
	int opaque_mask = gfx_tilemap->color_granularity - 1;
	int pixel_effect_mask = gfx_tilemap->color_base + (gfx_tilemap->total_colors - 1) * gfx_tilemap->color_granularity;
	int p;

	width_mask = src_bitmap->width - 1;
	height_mask = src_bitmap->height - 1;

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, 0);

		int x;
		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			if(!flipscreen)
			{
				p = *BITMAP_ADDR16(src_bitmap, (y + scrolly) & height_mask, (x + scrollx) & width_mask);
			}
			else
			{
				p = *BITMAP_ADDR16(src_bitmap, (y - scrolly - 256) & height_mask, (x - scrollx - 512) & width_mask);
			}

			// draw not transparent pixels
			if(p & opaque_mask)
			{
				// pixels with the last color are not drawn and the 2nd palette is added to the current bitmap color
				if((p & pixel_effect_mask) == pixel_effect_mask)
				{
					dest[x] = machine->total_colors() / 2 + dest[x];
				}
				else
				{
					dest[x] = machine->pens[p];
				}
			}
		}
	}
}



/***************************************************************************


                                Screen Drawing


***************************************************************************/

/* For games without tilemaps */
SCREEN_UPDATE( seta_no_layers )
{
	set_pens(screen->machine);
	bitmap_fill(bitmap,cliprect,0x1f0);
	draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}


/* For games with 1 or 2 tilemaps */
static SCREEN_UPDATE( seta_layers )
{
	seta_state *state = screen->machine->driver_data<seta_state>();
	int layers_ctrl = -1;
	int enab_0, enab_1, x_0, x_1=0, y_0, y_1=0;

	int order	=	0;
	int flip	=	(state->spriteram[ 0x600/2 ] & 0x40) >> 6;

	const rectangle &visarea = screen->visible_area();
	int vis_dimy = visarea.max_y - visarea.min_y + 1;

	// check tilemaps color modes

	if(state->current_tilemap_mode[0] != (state->vctrl_0[ 4/2 ] & 0x10))
	{
		state->current_tilemap_mode[0] = state->vctrl_0[ 4/2 ] & 0x10;
		tilemap_mark_all_tiles_dirty(state->tilemap_0);
		tilemap_mark_all_tiles_dirty(state->tilemap_1);
	}

	if(state->tilemap_2 != NULL && state->tilemap_3 != NULL)
	{
		if(state->current_tilemap_mode[1] != (state->vctrl_2[ 4/2 ] & 0x10))
		{
			state->current_tilemap_mode[1] = state->vctrl_2[ 4/2 ] & 0x10;
			tilemap_mark_all_tiles_dirty(state->tilemap_2);
			tilemap_mark_all_tiles_dirty(state->tilemap_3);
		}
	}

	flip ^= state->tilemaps_flip;

	tilemap_set_flip_all(screen->machine, flip ? (TILEMAP_FLIPX|TILEMAP_FLIPY) : 0 );

	x_0		=	state->vctrl_0[ 0/2 ];
	y_0		=	state->vctrl_0[ 2/2 ];
	enab_0	=	state->vctrl_0[ 4/2 ];

	/* Only one tilemap per layer is enabled! */
	tilemap_set_enable(state->tilemap_0, (!(enab_0 & 0x0008)) /*&& (enab_0 & 0x0001)*/ );
	tilemap_set_enable(state->tilemap_1, ( (enab_0 & 0x0008)) /*&& (enab_0 & 0x0001)*/ );

	/* the hardware wants different scroll values when flipped */

	/*  bg x scroll      flip
        metafox     0000 025d = 0, $400-$1a3 = $400 - $190 - $13
        eightfrc    ffe8 0272
                    fff0 0260 = -$10, $400-$190 -$10
                    ffe8 0272 = -$18, $400-$190 -$18 + $1a      */

	x_0 += 0x10 - state->global_offsets->tilemap_offs[flip ? 1 : 0];
	y_0 -= (256 - vis_dimy)/2;
	if (flip)
	{
		x_0 = -x_0 - 512;
		y_0 = y_0 - vis_dimy;
	}

	tilemap_set_scrollx(state->tilemap_0, 0, x_0);
	tilemap_set_scrollx(state->tilemap_1, 0, x_0);
	tilemap_set_scrolly(state->tilemap_0, 0, y_0);
	tilemap_set_scrolly(state->tilemap_1, 0, y_0);

	if (state->tilemap_2)
	{
		x_1		=	state->vctrl_2[ 0/2 ];
		y_1		=	state->vctrl_2[ 2/2 ];
		enab_1	=	state->vctrl_2[ 4/2 ];

		tilemap_set_enable(state->tilemap_2, (!(enab_1 & 0x0008)) /*&& (enab_1 & 0x0001)*/ );
		tilemap_set_enable(state->tilemap_3, ( (enab_1 & 0x0008)) /*&& (enab_1 & 0x0001)*/ );

		x_1 += 0x10 - state->global_offsets->tilemap_offs[flip ? 1 : 0];
		y_1 -= (256 - vis_dimy)/2;
		if (flip)
		{
			x_1 = -x_1 - 512;
			y_1 = y_1 - vis_dimy;
		}

		tilemap_set_scrollx(state->tilemap_2, 0, x_1);
		tilemap_set_scrollx(state->tilemap_3, 0, x_1);
		tilemap_set_scrolly(state->tilemap_2, 0, y_1);
		tilemap_set_scrolly(state->tilemap_3, 0, y_1);

		order	=	state->vregs[ 2/2 ];
	}


#ifdef MAME_DEBUG
if (input_code_pressed(screen->machine, KEYCODE_Z))
{	int msk = 0;
	if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= 2;
	if (input_code_pressed(screen->machine, KEYCODE_A))	msk |= 8;
	if (msk != 0) layers_ctrl &= msk;

	if (state->tilemap_2)
		popmessage("VR:%04X-%04X-%04X L0:%04X L1:%04X",
			state->vregs[0], state->vregs[1], state->vregs[2], state->vctrl_0[4/2], state->vctrl_2[4/2]);
	else if (state->tilemap_0)	popmessage("L0:%04X", state->vctrl_0[4/2]);
}
#endif

	bitmap_fill(bitmap,cliprect,0);

	if (order & 1)	// swap the layers?
	{
		if (state->tilemap_2)
		{
			if (layers_ctrl & 2)	tilemap_draw(bitmap, cliprect, state->tilemap_2, TILEMAP_DRAW_OPAQUE, 0);
			if (layers_ctrl & 2)	tilemap_draw(bitmap, cliprect, state->tilemap_3, TILEMAP_DRAW_OPAQUE, 0);
		}

		if (order & 2)	// layer-sprite priority?
		{
			if (layers_ctrl & 8)	draw_sprites(screen->machine,bitmap,cliprect);

			if(order & 4)
			{
				popmessage("Missing palette effect. Contact MAMETesters.");
			}

			if (layers_ctrl & 1)	tilemap_draw(bitmap, cliprect, state->tilemap_0, 0, 0);
			if (layers_ctrl & 1)	tilemap_draw(bitmap, cliprect, state->tilemap_1, 0, 0);
		}
		else
		{
			if(order & 4)
			{
				popmessage("Missing palette effect. Contact MAMETesters.");
			}

			if (layers_ctrl & 1)	tilemap_draw(bitmap, cliprect, state->tilemap_0,  0, 0);
			if (layers_ctrl & 1)	tilemap_draw(bitmap, cliprect, state->tilemap_1,  0, 0);

			if (layers_ctrl & 8)	draw_sprites(screen->machine, bitmap,cliprect);
		}
	}
	else
	{
		if (layers_ctrl & 1)	tilemap_draw(bitmap, cliprect, state->tilemap_0,  TILEMAP_DRAW_OPAQUE, 0);
		if (layers_ctrl & 1)	tilemap_draw(bitmap, cliprect, state->tilemap_1,  TILEMAP_DRAW_OPAQUE, 0);

		if (order & 2)	// layer-sprite priority?
		{
			if (layers_ctrl & 8)	draw_sprites(screen->machine, bitmap,cliprect);

			if((order & 4) && state->paletteram2 != NULL)
			{
				if(tilemap_get_enable(state->tilemap_2))
				{
					draw_tilemap_palette_effect(screen->machine, bitmap, cliprect, state->tilemap_2, x_1, y_1, 2 + ((state->vctrl_2[ 4/2 ] & 0x10) >> state->color_mode_shift), flip);
				}
				else
				{
					draw_tilemap_palette_effect(screen->machine, bitmap, cliprect, state->tilemap_3, x_1, y_1, 2 + ((state->vctrl_2[ 4/2 ] & 0x10) >> state->color_mode_shift), flip);
				}
			}
			else
			{
				if(order & 4)
				{
					popmessage("Missing palette effect. Contact MAMETesters.");
				}

				if (state->tilemap_2)
				{
					if (layers_ctrl & 2)	tilemap_draw(bitmap, cliprect, state->tilemap_2, 0, 0);
					if (layers_ctrl & 2)	tilemap_draw(bitmap, cliprect, state->tilemap_3, 0, 0);
				}
			}
		}
		else
		{
			if((order & 4) && state->paletteram2 != NULL)
			{
				if(tilemap_get_enable(state->tilemap_2))
				{
					draw_tilemap_palette_effect(screen->machine, bitmap, cliprect, state->tilemap_2, x_1, y_1, 2 + ((state->vctrl_2[ 4/2 ] & 0x10) >> state->color_mode_shift), flip);
				}
				else
				{
					draw_tilemap_palette_effect(screen->machine, bitmap, cliprect, state->tilemap_3, x_1, y_1, 2 + ((state->vctrl_2[ 4/2 ] & 0x10) >> state->color_mode_shift), flip);
				}
			}
			else
			{
				if(order & 4)
				{
					popmessage("Missing palette effect. Contact MAMETesters.");
				}

				if (state->tilemap_2)
				{
					if (layers_ctrl & 2)	tilemap_draw(bitmap, cliprect, state->tilemap_2, 0, 0);
					if (layers_ctrl & 2)	tilemap_draw(bitmap, cliprect, state->tilemap_3, 0, 0);
				}
			}

			if (layers_ctrl & 8)	draw_sprites(screen->machine, bitmap,cliprect);
		}
	}
	return 0;
}


SCREEN_UPDATE( seta )
{
	set_pens(screen->machine);
	return SCREEN_UPDATE_CALL(seta_layers);
}


SCREEN_UPDATE( usclssic )
{
	usclssic_set_pens(screen->machine);
	return SCREEN_UPDATE_CALL(seta_layers);
}


SCREEN_UPDATE( inttoote )
{
	/* no palette to set */
	return SCREEN_UPDATE_CALL(seta_layers);
}
