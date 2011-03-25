
class homedata_state : public driver_device
{
public:
	homedata_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  vreg;
	UINT8 *  videoram;

	/* video-related */
	tilemap_t *bg_tilemap[2][4];
	int      visible_page;
	int      priority;
	UINT8    reikaids_which;
	int      flipscreen;
	UINT8	   gfx_bank[2];	// pteacher only uses the first one
	UINT8	   blitter_bank;
	int      blitter_param_count;
	UINT8	   blitter_param[4];		/* buffers last 4 writes to 0x8006 */


	/* misc */
	int      vblank;
	int      sndbank;
	int      keyb;
	int      snd_command;
	int      upd7807_porta, upd7807_portc;
	int      to_cpu, from_cpu;

	/* device */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *dac;
	device_t *ym;
	device_t *sn;
};



/*----------- defined in video/homedata.c -----------*/

WRITE8_HANDLER( mrokumei_videoram_w );
WRITE8_HANDLER( reikaids_videoram_w );
WRITE8_HANDLER( reikaids_gfx_bank_w );
WRITE8_HANDLER( pteacher_gfx_bank_w );
WRITE8_HANDLER( homedata_blitter_param_w );
WRITE8_HANDLER( mrokumei_blitter_bank_w );
WRITE8_HANDLER( reikaids_blitter_bank_w );
WRITE8_HANDLER( pteacher_blitter_bank_w );
WRITE8_HANDLER( mrokumei_blitter_start_w );
WRITE8_HANDLER( reikaids_blitter_start_w );
WRITE8_HANDLER( pteacher_blitter_start_w );

PALETTE_INIT( mrokumei );
PALETTE_INIT( reikaids );
PALETTE_INIT( pteacher );
PALETTE_INIT( mirderby );

VIDEO_START( mrokumei );
VIDEO_START( reikaids );
VIDEO_START( pteacher );
VIDEO_START( lemnangl );
VIDEO_START( mirderby );
SCREEN_UPDATE( mrokumei );
SCREEN_UPDATE( reikaids );
SCREEN_UPDATE( pteacher );
SCREEN_UPDATE( mirderby );
SCREEN_EOF( homedata );
