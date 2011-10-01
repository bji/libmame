/*************************************************************************

    Universal 8106-A2 + 8106-B PCB set

    and Zero Hour / Red Clash

*************************************************************************/

class ladybug_state : public driver_device
{
public:
	ladybug_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	UINT8 *    m_grid_data;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_grid_tilemap;	// ladybug
	tilemap_t    *m_fg_tilemap;	// redclash
	UINT8      m_grid_color;
	int        m_star_speed;
	int        m_gfxbank;	// redclash only
	UINT8      m_stars_enable;
	UINT8      m_stars_speed;
	UINT32     m_stars_state;
	UINT16     m_stars_offset;
	UINT8      m_stars_count;

	/* misc */
	UINT8      m_sound_low;
	UINT8      m_sound_high;
	UINT8      m_weird_value[8];
	UINT8      m_sraider_0x30;
	UINT8      m_sraider_0x38;

	/* devices */
	device_t *m_maincpu;
};


/*----------- defined in video/ladybug.c -----------*/

WRITE8_HANDLER( ladybug_videoram_w );
WRITE8_HANDLER( ladybug_colorram_w );
WRITE8_HANDLER( ladybug_flipscreen_w );
WRITE8_HANDLER( sraider_io_w );

PALETTE_INIT( ladybug );
VIDEO_START( ladybug );
SCREEN_UPDATE( ladybug );

PALETTE_INIT( sraider );
VIDEO_START( sraider );
SCREEN_UPDATE( sraider );
SCREEN_EOF( sraider );

/*----------- defined in video/redclash.c -----------*/

WRITE8_HANDLER( redclash_videoram_w );
WRITE8_HANDLER( redclash_gfxbank_w );
WRITE8_HANDLER( redclash_flipscreen_w );

WRITE8_HANDLER( redclash_star0_w );
WRITE8_HANDLER( redclash_star1_w );
WRITE8_HANDLER( redclash_star2_w );
WRITE8_HANDLER( redclash_star_reset_w );

PALETTE_INIT( redclash );
VIDEO_START( redclash );
SCREEN_UPDATE( redclash );
SCREEN_EOF( redclash );

/* sraider uses the zerohour star generator board */
void redclash_set_stars_enable(running_machine &machine, UINT8 on);
void redclash_update_stars_state(running_machine &machine);
void redclash_set_stars_speed(running_machine &machine, UINT8 speed);
void redclash_draw_stars(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 palette_offset, UINT8 sraider, UINT8 firstx, UINT8 lastx);
