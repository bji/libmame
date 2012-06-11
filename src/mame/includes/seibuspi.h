#include "machine/intelfsh.h"

#define FIFO_SIZE 512

class seibuspi_state : public driver_device
{
public:
	seibuspi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spi_scrollram(*this, "spi_scrollram"),
		m_spimainram(*this, "spimainram"){ }

	optional_shared_ptr<UINT32> m_spi_scrollram;
	required_shared_ptr<UINT32> m_spimainram;

	intel_e28f008sa_device *m_flash[2];
	UINT8 *m_z80_rom;
	int m_z80_prg_fifo_pos;
	int m_z80_lastbank;
	int m_fifoin_rpos;
	int m_fifoin_wpos;
	UINT8 m_fifoin_data[FIFO_SIZE];
	int m_fifoin_read_request;
	int m_fifoout_rpos;
	int m_fifoout_wpos;
	UINT8 m_fifoout_data[FIFO_SIZE];
	int m_fifoout_read_request;
	UINT8 m_sb_coin_latch;
	UINT8 m_ejsakura_input_port;
	tilemap_t *m_text_layer;
	tilemap_t *m_back_layer;
	tilemap_t *m_mid_layer;
	tilemap_t *m_fore_layer;
	UINT32 m_layer_bank;
	UINT32 m_layer_enable;
	UINT32 m_video_dma_length;
	UINT32 m_video_dma_address;
	UINT32 m_sprite_dma_length;
	int m_rf2_layer_bank[3];
	UINT32 *m_tilemap_ram;
	UINT32 *m_palette_ram;
	UINT32 *m_sprite_ram;
	int m_mid_layer_offset;
	int m_fore_layer_offset;
	int m_text_layer_offset;
	UINT32 m_bg_fore_layer_position;
	UINT8 m_alpha_table[8192];
	UINT8 m_sprite_bpp;
	DECLARE_READ32_MEMBER(ejanhs_speedup_r);
	DECLARE_READ32_MEMBER(spi_layer_bank_r);
	DECLARE_WRITE32_MEMBER(spi_layer_bank_w);
	DECLARE_READ32_MEMBER(spi_layer_enable_r);
	DECLARE_WRITE32_MEMBER(spi_layer_enable_w);
	DECLARE_WRITE32_MEMBER(tilemap_dma_start_w);
	DECLARE_WRITE32_MEMBER(palette_dma_start_w);
	DECLARE_WRITE32_MEMBER(sprite_dma_start_w);
	DECLARE_WRITE32_MEMBER(video_dma_length_w);
	DECLARE_WRITE32_MEMBER(video_dma_address_w);
	DECLARE_CUSTOM_INPUT_MEMBER(ejsakura_keyboard_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ejanhs_encode);
	DECLARE_READ32_MEMBER(sb_coin_r);
	DECLARE_WRITE8_MEMBER(sb_coin_w);
	DECLARE_READ32_MEMBER(sound_fifo_r);
	DECLARE_WRITE32_MEMBER(sound_fifo_w);
	DECLARE_READ32_MEMBER(sound_fifo_status_r);
	DECLARE_READ32_MEMBER(spi_int_r);
	DECLARE_READ32_MEMBER(spi_unknown_r);
	DECLARE_WRITE32_MEMBER(z80_prg_fifo_w);
	DECLARE_WRITE32_MEMBER(z80_enable_w);
	DECLARE_READ32_MEMBER(spi_controls1_r);
	DECLARE_READ32_MEMBER(spi_controls2_r);
	DECLARE_READ8_MEMBER(z80_soundfifo_r);
	DECLARE_WRITE8_MEMBER(z80_soundfifo_w);
	DECLARE_READ8_MEMBER(z80_soundfifo_status_r);
	DECLARE_WRITE8_MEMBER(z80_bank_w);
	DECLARE_READ8_MEMBER(z80_jp1_r);
	DECLARE_READ8_MEMBER(z80_coin_r);
	DECLARE_READ32_MEMBER(soundrom_r);
	DECLARE_WRITE32_MEMBER(input_select_w);
	DECLARE_READ32_MEMBER(senkyu_speedup_r);
	DECLARE_READ32_MEMBER(senkyua_speedup_r);
	DECLARE_READ32_MEMBER(batlball_speedup_r);
	DECLARE_READ32_MEMBER(rdft_speedup_r);
	DECLARE_READ32_MEMBER(viprp1_speedup_r);
	DECLARE_READ32_MEMBER(viprp1o_speedup_r);
	DECLARE_READ32_MEMBER(rf2_speedup_r);
	DECLARE_READ32_MEMBER(rfjet_speedup_r);
};


/*----------- defined in machine/spisprit.c -----------*/

void seibuspi_sprite_decrypt(UINT8 *src, int romsize);


/*----------- defined in video/seibuspi.c -----------*/

VIDEO_START( spi );
SCREEN_UPDATE_RGB32( spi );

VIDEO_START( sys386f2 );
SCREEN_UPDATE_RGB32( sys386f2 );


void rf2_set_layer_banks(running_machine &machine, int banks);

