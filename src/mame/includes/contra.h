/*************************************************************************

    Contra / Gryzor

*************************************************************************/

class contra_state : public driver_device
{
public:
	contra_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        spriteram;
	UINT8 *        spriteram_2;
	UINT8 *        paletteram;
	UINT8 *        bg_vram;
	UINT8 *        bg_cram;
	UINT8 *        fg_vram;
	UINT8 *        fg_cram;
	UINT8 *        tx_vram;
	UINT8 *        tx_cram;
	// this driver also uses a large generic spriteram region...

	/* video-related */
	tilemap_t *bg_tilemap, *fg_tilemap, *tx_tilemap;
	rectangle bg_clip, fg_clip, tx_clip;

	/* devices */
	device_t *audiocpu;
	device_t *k007121_1;
	device_t *k007121_2;
};


/*----------- defined in video/contra.c -----------*/

PALETTE_INIT( contra );

WRITE8_HANDLER( contra_fg_vram_w );
WRITE8_HANDLER( contra_fg_cram_w );
WRITE8_HANDLER( contra_bg_vram_w );
WRITE8_HANDLER( contra_bg_cram_w );
WRITE8_HANDLER( contra_text_vram_w );
WRITE8_HANDLER( contra_text_cram_w );

WRITE8_HANDLER( contra_K007121_ctrl_0_w );
WRITE8_HANDLER( contra_K007121_ctrl_1_w );

SCREEN_UPDATE( contra );
VIDEO_START( contra );
