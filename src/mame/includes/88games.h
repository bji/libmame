/*************************************************************************

    88 Games

*************************************************************************/

class _88games_state : public driver_device
{
public:
	_88games_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *      m_ram;
	UINT8 *      m_banked_rom;
//  UINT8 *      m_paletteram_1000;   // this currently uses generic palette handling
//  UINT8 *      m_nvram; // this currently uses generic nvram handling
	UINT8 *m_paletteram_1000;

	/* video-related */
	int          m_k88games_priority;
	int          m_layer_colorbase[3];
	int          m_sprite_colorbase;
	int          m_zoom_colorbase;
	int          m_videobank;
	int          m_zoomreadroms;
	int          m_speech_chip;

	/* devices */
	device_t *m_audiocpu;
	device_t *m_k052109;
	device_t *m_k051960;
	device_t *m_k051316;
	device_t *m_upd_1;
	device_t *m_upd_2;
};


/*----------- defined in video/88games.c -----------*/

void _88games_sprite_callback(running_machine &machine, int *code, int *color, int *priority, int *shadow);
void _88games_tile_callback(running_machine &machine, int layer, int bank, int *code, int *color, int *flags, int *priority);
void _88games_zoom_callback(running_machine &machine, int *code, int *color, int *flags);

SCREEN_UPDATE_IND16( 88games );
