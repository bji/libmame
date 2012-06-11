#include "sound/samples.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"

class senjyo_state : public driver_device
{
public:
	senjyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_fgscroll(*this, "fgscroll"),
		m_scrollx1(*this, "scrollx1"),
		m_scrolly1(*this, "scrolly1"),
		m_scrollx2(*this, "scrollx2"),
		m_scrolly2(*this, "scrolly2"),
		m_scrollx3(*this, "scrollx3"),
		m_scrolly3(*this, "scrolly3"),
		m_fgvideoram(*this, "fgvideoram"),
		m_fgcolorram(*this, "fgcolorram"),
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_bg3videoram(*this, "bg3videoram"),
		m_radarram(*this, "radarram"),
		m_bgstripesram(*this, "bgstripesram"){ }

	int m_int_delay_kludge;
	UINT8 m_sound_cmd;
	INT16 *m_single_data;
	int m_single_rate;
	int m_single_volume;

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_fgscroll;
	required_shared_ptr<UINT8> m_scrollx1;
	required_shared_ptr<UINT8> m_scrolly1;
	required_shared_ptr<UINT8> m_scrollx2;
	required_shared_ptr<UINT8> m_scrolly2;
	required_shared_ptr<UINT8> m_scrollx3;
	required_shared_ptr<UINT8> m_scrolly3;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_fgcolorram;
	required_shared_ptr<UINT8> m_bg1videoram;
	required_shared_ptr<UINT8> m_bg2videoram;
	required_shared_ptr<UINT8> m_bg3videoram;
	required_shared_ptr<UINT8> m_radarram;
	required_shared_ptr<UINT8> m_bgstripesram;
	int m_is_senjyo;
	int m_scrollhack;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_bg2_tilemap;
	tilemap_t *m_bg3_tilemap;

	int m_bgstripes;
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(senjyo_paletteram_w);
	DECLARE_WRITE8_MEMBER(starforb_scrolly2);
	DECLARE_WRITE8_MEMBER(starforb_scrollx2);
	DECLARE_WRITE8_MEMBER(senjyo_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(senjyo_fgcolorram_w);
	DECLARE_WRITE8_MEMBER(senjyo_bg1videoram_w);
	DECLARE_WRITE8_MEMBER(senjyo_bg2videoram_w);
	DECLARE_WRITE8_MEMBER(senjyo_bg3videoram_w);
	DECLARE_WRITE8_MEMBER(senjyo_bgstripes_w);
	DECLARE_WRITE8_MEMBER(senjyo_volume_w);
};


/*----------- defined in audio/senjyo.c -----------*/

extern const z80_daisy_config senjyo_daisy_chain[];
extern const z80pio_interface senjyo_pio_intf;
extern const z80ctc_interface senjyo_ctc_intf;

SAMPLES_START( senjyo_sh_start );


/*----------- defined in video/senjyo.c -----------*/


VIDEO_START( senjyo );
SCREEN_UPDATE_IND16( senjyo );
