#include "machine/intelfsh.h"

#define FIFO_SIZE 512

class seibuspi_state : public driver_device
{
public:
	seibuspi_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 *m_spimainram;
	UINT32 *m_spi_scrollram;
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
};


/*----------- defined in machine/spisprit.c -----------*/

void seibuspi_sprite_decrypt(UINT8 *src, int romsize);


/*----------- defined in video/seibuspi.c -----------*/

VIDEO_START( spi );
SCREEN_UPDATE( spi );

VIDEO_START( sys386f2 );
SCREEN_UPDATE( sys386f2 );

READ32_HANDLER( spi_layer_bank_r );
WRITE32_HANDLER( spi_layer_bank_w );
WRITE32_HANDLER( spi_layer_enable_w );

void rf2_set_layer_banks(running_machine &machine, int banks);

WRITE32_HANDLER( tilemap_dma_start_w );
WRITE32_HANDLER( palette_dma_start_w );
WRITE32_HANDLER( video_dma_length_w );
WRITE32_HANDLER( video_dma_address_w );
WRITE32_HANDLER( sprite_dma_start_w );
