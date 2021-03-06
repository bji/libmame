/***************************************************************************

    Jailbreak

***************************************************************************/

#define MASTER_CLOCK        XTAL_18_432MHz
#define VOICE_CLOCK         XTAL_3_579545MHz

class jailbrek_state : public driver_device
{
public:
	jailbrek_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *      videoram;
	UINT8 *      colorram;
	UINT8 *      spriteram;
	UINT8 *      scroll_x;
	UINT8 *      scroll_dir;
	size_t       spriteram_size;

	/* video-related */
	tilemap_t      *bg_tilemap;

	/* misc */
	UINT8        irq_enable, nmi_enable;
};


/*----------- defined in video/jailbrek.c -----------*/

WRITE8_HANDLER( jailbrek_videoram_w );
WRITE8_HANDLER( jailbrek_colorram_w );

PALETTE_INIT( jailbrek );
VIDEO_START( jailbrek );
SCREEN_UPDATE( jailbrek );
