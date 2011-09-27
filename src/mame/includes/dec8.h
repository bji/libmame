
class dec8_state : public driver_device
{
public:
	dec8_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_bg_data;
	UINT8 *  m_pf1_data;
	UINT8 *  m_row;
//  UINT8 *  m_paletteram;    // currently this uses generic palette handling
//  UINT8 *  m_paletteram_2;  // currently this uses generic palette handling
//  UINT8 *  m_spriteram; // currently this uses buffered_spriteram in some games
	size_t   m_videoram_size;
	UINT16   m_buffered_spriteram16[0x800/2]; // for the mxc06 sprite chip emulation (oscar, cobra)

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_pf1_tilemap;
	tilemap_t  *m_fix_tilemap;
	//int      m_scroll1[4];
	int      m_scroll2[4];
	int      m_bg_control[0x20];
	int      m_pf1_control[0x20];
	int      m_game_uses_priority;

	/* misc */
	int      m_i8751_port0;
	int      m_i8751_port1;
	int      m_nmi_enable;
	int      m_i8751_return;
	int      m_i8751_value;
	int      m_coin1;
	int      m_coin2;
	int      m_latch;
	int      m_snd;
	int      m_msm5205next;
	int      m_toggle;

	/* devices */
	device_t *m_maincpu;
	device_t *m_subcpu;
	device_t *m_audiocpu;
	device_t *m_mcu;
};

/*----------- defined in video/dec8.c -----------*/


PALETTE_INIT( ghostb );
SCREEN_UPDATE( cobracom );
SCREEN_UPDATE( ghostb );
SCREEN_UPDATE( srdarwin );
SCREEN_UPDATE( gondo );
SCREEN_UPDATE( garyoret );
SCREEN_UPDATE( lastmisn );
SCREEN_UPDATE( shackled );
SCREEN_UPDATE( oscar );
VIDEO_START( cobracom );
VIDEO_START( oscar );
VIDEO_START( ghostb );
VIDEO_START( lastmisn );
VIDEO_START( shackled );
VIDEO_START( srdarwin );
VIDEO_START( gondo );
VIDEO_START( garyoret );

WRITE8_HANDLER( dec8_bac06_0_w );
WRITE8_HANDLER( dec8_bac06_1_w );
WRITE8_HANDLER( dec8_bg_data_w );
WRITE8_HANDLER( dec8_pf1_data_w );
READ8_HANDLER( dec8_bg_data_r );
READ8_HANDLER( dec8_pf1_data_r );
WRITE8_HANDLER( srdarwin_videoram_w );
WRITE8_HANDLER( dec8_scroll2_w );
WRITE8_HANDLER( srdarwin_control_w );
WRITE8_HANDLER( gondo_scroll_w );
WRITE8_HANDLER( shackled_control_w );
WRITE8_HANDLER( lastmisn_control_w );
WRITE8_HANDLER( lastmisn_scrollx_w );
WRITE8_HANDLER( lastmisn_scrolly_w );
WRITE8_HANDLER( dec8_videoram_w );
