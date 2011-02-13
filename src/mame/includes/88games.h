/*************************************************************************

    88 Games

*************************************************************************/

class _88games_state : public driver_device
{
public:
	_88games_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *      ram;
	UINT8 *      banked_rom;
//  UINT8 *      paletteram_1000;   // this currently uses generic palette handling
//  UINT8 *      nvram; // this currently uses generic nvram handling

	/* video-related */
	int          k88games_priority;
	int          layer_colorbase[3], sprite_colorbase, zoom_colorbase;
	int          videobank;
	int          zoomreadroms;
	int          speech_chip;

	/* devices */
	device_t *audiocpu;
	device_t *k052109;
	device_t *k051960;
	device_t *k051316;
	device_t *upd_1;
	device_t *upd_2;
};


/*----------- defined in video/88games.c -----------*/

void _88games_sprite_callback(running_machine *machine, int *code, int *color, int *priority, int *shadow);
void _88games_tile_callback(running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority);
void _88games_zoom_callback(running_machine *machine, int *code, int *color, int *flags);

VIDEO_UPDATE( 88games );
