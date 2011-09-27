/***************************************************************************

    Game Driver for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

***************************************************************************/

class fromance_state : public driver_device
{
public:
	fromance_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers (used by pipedrm) */
	UINT8 *  m_videoram;
	UINT8 *  m_spriteram;
//  UINT8 *  m_paletteram;    // currently this uses generic palette handling
	size_t   m_videoram_size;
	size_t   m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	UINT8    *m_local_videoram[2];
	UINT8    *m_local_paletteram;
	UINT8    m_selected_videoram;
	UINT8    m_selected_paletteram;
	UINT32   m_scrollx[2];
	UINT32   m_scrolly[2];
	UINT8    m_gfxreg;
	UINT8    m_flipscreen;
	UINT8    m_flipscreen_old;
	UINT32   m_scrolly_ofs;
	UINT32   m_scrollx_ofs;

	UINT8    m_crtc_register;
	UINT8    m_crtc_data[0x10];
	emu_timer *m_crtc_timer;

	/* misc */
	UINT8    m_directionflag;
	UINT8    m_commanddata;
	UINT8    m_portselect;
	UINT8    m_adpcm_reset;
	UINT8    m_adpcm_data;
	UINT8    m_vclk_left;
	UINT8    m_pending_command;
	UINT8    m_sound_command;

	/* devices */
	device_t *m_subcpu;
};


/*----------- defined in video/fromance.c -----------*/

VIDEO_START( fromance );
VIDEO_START( nekkyoku );
VIDEO_START( pipedrm );
VIDEO_START( hatris );
SCREEN_UPDATE( fromance );
SCREEN_UPDATE( pipedrm );

WRITE8_HANDLER( fromance_crtc_data_w );
WRITE8_HANDLER( fromance_crtc_register_w );

WRITE8_HANDLER( fromance_gfxreg_w );

WRITE8_HANDLER( fromance_scroll_w );

READ8_HANDLER( fromance_paletteram_r );
WRITE8_HANDLER( fromance_paletteram_w );

READ8_HANDLER( fromance_videoram_r );
WRITE8_HANDLER( fromance_videoram_w );
