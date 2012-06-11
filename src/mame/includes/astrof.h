/***************************************************************************

    Astro Fighter hardware

****************************************************************************/

#include "sound/samples.h"

class astrof_state : public driver_device
{
public:
	astrof_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_astrof_color(*this, "astrof_color"),
		m_tomahawk_protection(*this, "tomahawk_prot"){ }

	/* video-related */
	required_shared_ptr<UINT8> m_videoram;

	UINT8 *    m_colorram;
	required_shared_ptr<UINT8> m_astrof_color;
	optional_shared_ptr<UINT8> m_tomahawk_protection;

	UINT8      m_astrof_palette_bank;
	UINT8      m_red_on;
	UINT8      m_flipscreen;
	UINT8      m_screen_off;
	UINT16     m_abattle_count;

	/* sound-related */
	UINT8      m_port_1_last;
	UINT8      m_port_2_last;
	UINT8      m_astrof_start_explosion;
	UINT8      m_astrof_death_playing;
	UINT8      m_astrof_bosskill_playing;

	/* devices */
	device_t *m_maincpu;
	samples_device *m_samples;	// astrof & abattle
	device_t *m_sn;	// tomahawk
	DECLARE_READ8_MEMBER(irq_clear_r);
	DECLARE_WRITE8_MEMBER(astrof_videoram_w);
	DECLARE_WRITE8_MEMBER(tomahawk_videoram_w);
	DECLARE_WRITE8_MEMBER(video_control_1_w);
	DECLARE_WRITE8_MEMBER(astrof_video_control_2_w);
	DECLARE_WRITE8_MEMBER(spfghmk2_video_control_2_w);
	DECLARE_WRITE8_MEMBER(tomahawk_video_control_2_w);
	DECLARE_READ8_MEMBER(shoot_r);
	DECLARE_READ8_MEMBER(abattle_coin_prot_r);
	DECLARE_READ8_MEMBER(afire_coin_prot_r);
	DECLARE_READ8_MEMBER(tomahawk_protection_r);
	DECLARE_CUSTOM_INPUT_MEMBER(astrof_p1_controls_r);
	DECLARE_CUSTOM_INPUT_MEMBER(astrof_p2_controls_r);
	DECLARE_CUSTOM_INPUT_MEMBER(tomahawk_controls_r);
	DECLARE_WRITE8_MEMBER(astrof_audio_1_w);
	DECLARE_WRITE8_MEMBER(astrof_audio_2_w);
	DECLARE_WRITE8_MEMBER(spfghmk2_audio_w);
	DECLARE_WRITE8_MEMBER(tomahawk_audio_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(service_coin_inserted);
};

/*----------- defined in audio/astrof.c -----------*/

MACHINE_CONFIG_EXTERN( astrof_audio );

MACHINE_CONFIG_EXTERN( spfghmk2_audio );

MACHINE_CONFIG_EXTERN( tomahawk_audio );
