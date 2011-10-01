/*************************************************************************

    Konami Battlantis Hardware

*************************************************************************/

class battlnts_state : public driver_device
{
public:
	battlnts_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT8 *      paletteram;    // this currently uses generic palette handling

	/* video-related */
	int m_spritebank;
	int m_layer_colorbase[2];


	/* devices */
	device_t *m_audiocpu;
	device_t *m_k007342;
	device_t *m_k007420;
};

/*----------- defined in video/battlnts.c -----------*/

WRITE8_HANDLER( battlnts_spritebank_w );

SCREEN_UPDATE( battlnts );

void battlnts_tile_callback(running_machine &machine, int layer, int bank, int *code, int *color, int *flags);
void battlnts_sprite_callback(running_machine &machine, int *code, int *color);
