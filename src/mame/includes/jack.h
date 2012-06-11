/*************************************************************************

    Jack the Giant Killer

*************************************************************************/

class jack_state : public driver_device
{
public:
	jack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
//  UINT8 *    paletteram;  // currently this uses generic palette handling

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
	DECLARE_WRITE8_MEMBER(jack_sh_command_w);
	DECLARE_WRITE8_MEMBER(joinem_misc_w);
	DECLARE_READ8_MEMBER(striv_question_r);
	DECLARE_WRITE8_MEMBER(jack_videoram_w);
	DECLARE_WRITE8_MEMBER(jack_colorram_w);
	DECLARE_WRITE8_MEMBER(jack_paletteram_w);
	DECLARE_READ8_MEMBER(jack_flipscreen_r);
	DECLARE_WRITE8_MEMBER(jack_flipscreen_w);
	DECLARE_CUSTOM_INPUT_MEMBER(sound_check_r);
};


/*----------- defined in video/jack.c -----------*/


VIDEO_START( jack );
SCREEN_UPDATE_IND16( jack );

PALETTE_INIT( joinem );
VIDEO_START( joinem );
SCREEN_UPDATE_IND16( joinem );
