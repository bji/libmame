/*************************************************************************

    Jack the Giant Killer

*************************************************************************/

class jack_state : public driver_device
{
public:
	jack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int m_timer_rate;
	int m_joinem_snd_bit;
	int m_question_address;
	int m_question_rom;
	int m_remap_address[16];


	/* devices */
	cpu_device *m_audiocpu;
};


/*----------- defined in video/jack.c -----------*/

WRITE8_HANDLER( jack_videoram_w );
WRITE8_HANDLER( jack_colorram_w );
WRITE8_HANDLER( jack_paletteram_w );
READ8_HANDLER( jack_flipscreen_r );
WRITE8_HANDLER( jack_flipscreen_w );

VIDEO_START( jack );
SCREEN_UPDATE( jack );

PALETTE_INIT( joinem );
VIDEO_START( joinem );
SCREEN_UPDATE( joinem );
