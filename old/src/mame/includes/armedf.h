



class armedf_state : public driver_device
{
public:
	armedf_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *  text_videoram;
	UINT16 *  bg_videoram;
	UINT16 *  fg_videoram;
	UINT16 *  legion_cmd;	// legion only!
//  UINT16 *  spriteram;    // currently this uses generic buffered_spriteram
//  UINT16 *  paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *bg_tilemap,*fg_tilemap, *tx_tilemap;
	UINT16   scroll_msb;
	UINT16   vreg;
	UINT16   fg_scrollx, fg_scrolly;
	UINT16   bg_scrollx, bg_scrolly;
	int      scroll_type, sprite_offy, mcu_mode, old_mcu_mode;
	int      waiting_msb;
	int      oldmode;
};


/*----------- defined in video/armedf.c -----------*/

SCREEN_UPDATE( armedf );
SCREEN_EOF( armedf );
VIDEO_START( armedf );

WRITE16_HANDLER( armedf_bg_videoram_w );
WRITE16_HANDLER( armedf_fg_videoram_w );
WRITE16_HANDLER( armedf_text_videoram_w );
WRITE16_HANDLER( terraf_fg_scrollx_w );
WRITE16_HANDLER( terraf_fg_scrolly_w );
WRITE16_HANDLER( terraf_fg_scroll_msb_arm_w );
WRITE16_HANDLER( armedf_fg_scrollx_w );
WRITE16_HANDLER( armedf_fg_scrolly_w );
WRITE16_HANDLER( armedf_bg_scrollx_w );
WRITE16_HANDLER( armedf_bg_scrolly_w );
WRITE16_HANDLER( armedf_mcu_cmd );
