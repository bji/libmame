class legionna_state : public driver_device
{
public:
	legionna_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_back_data;
	UINT16 *m_fore_data;
	UINT16 *m_mid_data;
	UINT16 *m_scrollram16;
	UINT16 *m_textram;
	UINT16 m_layer_disable;
	int m_sprite_xoffs;
	int m_sprite_yoffs;
	tilemap_t *m_background_layer;
	tilemap_t *m_foreground_layer;
	tilemap_t *m_midground_layer;
	tilemap_t *m_text_layer;
	int m_has_extended_banking;
	int m_has_extended_priority;
	UINT16 m_back_gfx_bank;
	UINT16 m_fore_gfx_bank;
	UINT16 m_mid_gfx_bank;
};


/*----------- defined in video/legionna.c -----------*/

void heatbrl_setgfxbank(running_machine &machine, UINT16 data);
void denjinmk_setgfxbank(running_machine &machine, UINT16 data);
WRITE16_HANDLER( legionna_background_w );
WRITE16_HANDLER( legionna_foreground_w );
WRITE16_HANDLER( legionna_midground_w );
WRITE16_HANDLER( legionna_text_w );

VIDEO_START( legionna );
VIDEO_START( cupsoc );
VIDEO_START( denjinmk );
VIDEO_START( grainbow );
VIDEO_START( godzilla );
SCREEN_UPDATE( legionna );
SCREEN_UPDATE( godzilla );
SCREEN_UPDATE( grainbow );
