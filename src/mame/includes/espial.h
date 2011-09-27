/***************************************************************************

 Espial hardware games (drivers: espial.c)

***************************************************************************/

class espial_state : public driver_device
{
public:
	espial_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *   m_videoram;
	UINT8 *   m_colorram;
	UINT8 *   m_attributeram;
	UINT8 *   m_scrollram;
	UINT8 *   m_spriteram_1;
	UINT8 *   m_spriteram_2;
	UINT8 *   m_spriteram_3;

	/* video-related */
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	int       m_flipscreen;

	/* sound-related */
	UINT8     m_sound_nmi_enabled;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
};

/*----------- defined in video/espial.c -----------*/

PALETTE_INIT( espial );
VIDEO_START( espial );
VIDEO_START( netwars );
WRITE8_HANDLER( espial_videoram_w );
WRITE8_HANDLER( espial_colorram_w );
WRITE8_HANDLER( espial_attributeram_w );
WRITE8_HANDLER( espial_scrollram_w );
WRITE8_HANDLER( espial_flipscreen_w );
SCREEN_UPDATE( espial );
