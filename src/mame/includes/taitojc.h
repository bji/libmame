#include "video/poly.h"

class taitojc_state : public driver_device
{
public:
	taitojc_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int texture_x;
	int texture_y;

	UINT32 dsp_rom_pos;
	UINT16 dsp_tex_address;
	UINT16 dsp_tex_offset;


	int first_dsp_reset;
	int viewport_data[3];

	INT32 projected_point_x;
	INT32 projected_point_y;
	INT32 projection_data[3];

	INT32 intersection_data[3];

	UINT8 *texture;
	bitmap_t *framebuffer;
	bitmap_t *zbuffer;

	UINT32 *vram;
	UINT32 *objlist;

	//int debug_tex_pal;

	int gfx_index;

	UINT32 *char_ram;
	UINT32 *tile_ram;
	tilemap_t *tilemap;

	poly_manager *poly;

	UINT32 *main_ram;
	UINT16 *dsp_shared_ram;
	UINT32 *palette_ram;

	UINT16 *polygon_fifo;
	int polygon_fifo_ptr;

	UINT8 mcu_comm_main;
	UINT8 mcu_comm_hc11;
	UINT8 mcu_data_main;
	UINT8 mcu_data_hc11;

	UINT16 debug_dsp_ram[0x8000];
};


/*----------- defined in video/taitojc.c -----------*/

READ32_HANDLER(taitojc_tile_r);
WRITE32_HANDLER(taitojc_tile_w);
READ32_HANDLER(taitojc_char_r);
WRITE32_HANDLER(taitojc_char_w);
void taitojc_clear_frame(running_machine *machine);
void taitojc_render_polygons(running_machine *machine, UINT16 *polygon_fifo, int length);

VIDEO_START(taitojc);
SCREEN_UPDATE(taitojc);
