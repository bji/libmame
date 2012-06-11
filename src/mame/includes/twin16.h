class twin16_state : public driver_device
{
public:
	twin16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram;
	UINT16 m_CPUA_register;
	UINT16 m_CPUB_register;
	UINT16 m_sound_command;
	int m_cuebrickj_nvram_bank;
	UINT16 m_cuebrickj_nvram[0x400*0x20];
	UINT16 m_custom_video;
	UINT16 *m_gfx_rom;
	UINT16 *m_text_ram;
	UINT16 *m_sprite_gfx_ram;
	UINT16 *m_tile_gfx_ram;
	UINT16 m_sprite_buffer[0x800];
	emu_timer *m_sprite_timer;
	int m_sprite_busy;
	int m_need_process_spriteram;
	UINT16 m_gfx_bank;
	UINT16 m_scrollx[3];
	UINT16 m_scrolly[3];
	UINT16 m_video_register;
	tilemap_t *m_text_tilemap;
};


/*----------- defined in drivers/twin16.c -----------*/

int twin16_spriteram_process_enable( running_machine &machine );


/*----------- defined in video/twin16.c -----------*/

WRITE16_HANDLER( twin16_text_ram_w );
WRITE16_HANDLER( twin16_paletteram_word_w );
WRITE16_HANDLER( fround_gfx_bank_w );
WRITE16_HANDLER( twin16_video_register_w );
READ16_HANDLER( twin16_sprite_status_r );

VIDEO_START( twin16 );
SCREEN_UPDATE_IND16( twin16 );
SCREEN_VBLANK( twin16 );

void twin16_spriteram_process( running_machine &machine );
