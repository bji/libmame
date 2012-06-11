/*************************************************************************

Crazy Ballooon

*************************************************************************/


#define CRBALOON_MASTER_XTAL	(XTAL_9_987MHz)


class crbaloon_state : public driver_device
{
public:
	crbaloon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_pc3092_data(*this, "pc3092_data"){ }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_pc3092_data;
	UINT16 m_collision_address;
	UINT8 m_collision_address_clear;
	tilemap_t *m_bg_tilemap;
	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(pc3092_w);
	DECLARE_READ8_MEMBER(pc3259_r);
	DECLARE_WRITE8_MEMBER(port_sound_w);
	DECLARE_WRITE8_MEMBER(crbaloon_videoram_w);
	DECLARE_WRITE8_MEMBER(crbaloon_colorram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(pc3092_r);
};


/*----------- defined in audio/crbaloon.c -----------*/

WRITE8_DEVICE_HANDLER( crbaloon_audio_set_music_freq );
WRITE8_DEVICE_HANDLER( crbaloon_audio_set_music_enable );
void crbaloon_audio_set_explosion_enable(device_t *sn, int enabled);
void crbaloon_audio_set_breath_enable(device_t *sn, int enabled);
void crbaloon_audio_set_appear_enable(device_t *sn, int enabled);
WRITE8_DEVICE_HANDLER( crbaloon_audio_set_laugh_enable );

MACHINE_CONFIG_EXTERN( crbaloon_audio );


/*----------- defined in video/crbaloon.c -----------*/

PALETTE_INIT( crbaloon );
VIDEO_START( crbaloon );
SCREEN_UPDATE_IND16( crbaloon );


UINT16 crbaloon_get_collision_address(running_machine &machine);
void crbaloon_set_clear_collision_address(running_machine &machine, int _crbaloon_collision_address_clear);
