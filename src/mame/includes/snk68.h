class snk68_state : public driver_device
{
public:
	snk68_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int invert_controls;
	int sound_status;

	UINT16* pow_fg_videoram;

	UINT16 *spriteram;
	UINT16 *paletteram;
	int sprite_flip_axis;
	tilemap_t *fg_tilemap;
	int flipscreen;
	UINT32 fg_tile_offset;
};


/*----------- defined in video/snk68.c -----------*/

VIDEO_START( pow );
VIDEO_START( searchar );
VIDEO_UPDATE( pow );
WRITE16_HANDLER( pow_paletteram16_word_w );
WRITE16_HANDLER( pow_flipscreen16_w );
WRITE16_HANDLER( searchar_flipscreen16_w );
READ16_HANDLER( pow_spriteram_r );
WRITE16_HANDLER( pow_spriteram_w );
READ16_HANDLER( pow_fg_videoram_r );
WRITE16_HANDLER( pow_fg_videoram_w );
WRITE16_HANDLER( searchar_fg_videoram_w );
