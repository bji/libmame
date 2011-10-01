#include "video/poly.h"

struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class galastrm_state : public driver_device
{
public:
	galastrm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 m_coin_word;
	UINT16 m_frame_counter;
	UINT32 *m_ram;
	int m_tc0110pcr_addr;
	int m_tc0610_0_addr;
	int m_tc0610_1_addr;
	UINT32 m_mem[2];
	INT16 m_tc0610_ctrl_reg[2][8];
	struct tempsprite *m_spritelist;
	struct tempsprite *m_sprite_ptr_pre;
	bitmap_t *m_tmpbitmaps;
	bitmap_t *m_polybitmap;
	poly_manager *m_poly;
	int m_rsxb;
	int m_rsyb;
	int m_rsxoffs;
	int m_rsyoffs;
	UINT32 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/galastrm.c -----------*/

VIDEO_START( galastrm );
SCREEN_UPDATE( galastrm );
