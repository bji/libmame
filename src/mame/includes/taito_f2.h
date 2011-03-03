
#include "sound/okim6295.h"

struct f2_tempsprite
{
	int code, color;
	int flipx, flipy;
	int x, y;
	int zoomx, zoomy;
	int primask;
};

class taitof2_state : public driver_device
{
public:
	taitof2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  oki(*this, "oki") { }

	/* memory pointers */
	UINT16 *        sprite_extension;
	UINT16 *        spriteram;
	UINT16 *        spriteram_buffered;
	UINT16 *        spriteram_delayed;
	UINT16 *        cchip2_ram;	// for megablst only
//  UINT16 *        paletteram;    // currently this uses generic palette handling
	size_t          spriteram_size;
	size_t          spriteext_size;


	/* video-related */
	struct f2_tempsprite *spritelist;
	int             sprite_type;

	UINT16          spritebank[8];
//  UINT16          spritebank_eof[8];
	UINT16          spritebank_buffered[8];

	INT32           sprites_disabled, sprites_active_area, sprites_master_scrollx, sprites_master_scrolly;
	/* remember flip status over frames because driftout can fail to set it */
	INT32           sprites_flipscreen;

	/* On the left hand screen edge (assuming horiz screen, no
       screenflip: in screenflip it is the right hand edge etc.)
       there may be 0-3 unwanted pixels in both tilemaps *and*
       sprites. To erase this we use f2_hide_pixels (0 to +3). */

	INT32           hide_pixels;
	INT32           flip_hide_pixels;	/* Different in some games */

	INT32           pivot_xdisp;	/* Needed in games with a pivot layer */
	INT32           pivot_ydisp;

	INT32           game;

	UINT8           tilepri[6]; // todo - move into taitoic.c
	UINT8           spritepri[6]; // todo - move into taitoic.c
	UINT8           spriteblendmode; // todo - move into taitoic.c

	int             prepare_sprites;

	/* misc */
	INT32           mjnquest_input;
	int             last[2], nibble;
	INT32           driveout_sound_latch;
	INT32           oki_bank;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	optional_device<okim6295_device> oki;
	device_t *tc0100scn;
	device_t *tc0100scn_1;
	device_t *tc0100scn_2;
	device_t *tc0360pri;
	device_t *tc0280grd;
	device_t *tc0430grw;
	device_t *tc0480scp;
};

/*----------- defined in video/taito_f2.c -----------*/

VIDEO_START( taitof2_default );
VIDEO_START( taitof2_quiz );
VIDEO_START( taitof2_finalb );
VIDEO_START( taitof2_megab );
VIDEO_START( taitof2_solfigtr );
VIDEO_START( taitof2_koshien );
VIDEO_START( taitof2_driftout );
VIDEO_START( taitof2_dondokod );
VIDEO_START( taitof2_thundfox );
VIDEO_START( taitof2_growl );
VIDEO_START( taitof2_yuyugogo );
VIDEO_START( taitof2_mjnquest );
VIDEO_START( taitof2_footchmp );
VIDEO_START( taitof2_hthero );
VIDEO_START( taitof2_ssi );
VIDEO_START( taitof2_gunfront );
VIDEO_START( taitof2_ninjak );
VIDEO_START( taitof2_pulirula );
VIDEO_START( taitof2_metalb );
VIDEO_START( taitof2_qzchikyu );
VIDEO_START( taitof2_yesnoj );
VIDEO_START( taitof2_deadconx );
VIDEO_START( taitof2_deadconxj );
VIDEO_START( taitof2_dinorex );
SCREEN_EOF( taitof2_no_buffer );
SCREEN_EOF( taitof2_full_buffer_delayed );
SCREEN_EOF( taitof2_partial_buffer_delayed );
SCREEN_EOF( taitof2_partial_buffer_delayed_thundfox );
SCREEN_EOF( taitof2_partial_buffer_delayed_qzchikyu );

SCREEN_UPDATE( taitof2 );
SCREEN_UPDATE( taitof2_pri );
SCREEN_UPDATE( taitof2_pri_roz );
SCREEN_UPDATE( taitof2_ssi );
SCREEN_UPDATE( taitof2_thundfox );
SCREEN_UPDATE( taitof2_deadconx );
SCREEN_UPDATE( taitof2_metalb );
SCREEN_UPDATE( taitof2_yesnoj );

WRITE16_HANDLER( taitof2_spritebank_w );
WRITE16_HANDLER( koshien_spritebank_w );
WRITE16_HANDLER( taitof2_sprite_extension_w );
