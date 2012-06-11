class nbmj8991_state : public driver_device
{
public:
	nbmj8991_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_scrollx;
	int m_scrolly;
	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_src_addr;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_gfxrom;
	int m_dispflag;
	int m_flipscreen;
	int m_clutsel;
	int m_screen_refresh;
	bitmap_ind16 m_tmpbitmap;
	UINT8 *m_videoram;
	UINT8 *m_clut;
	int m_flipscreen_old;
};


/*----------- defined in video/nbmj8991.c -----------*/

SCREEN_UPDATE_IND16( nbmj8991_type1 );
SCREEN_UPDATE_IND16( nbmj8991_type2 );
VIDEO_START( nbmj8991 );

WRITE8_HANDLER( nbmj8991_palette_type1_w );
WRITE8_HANDLER( nbmj8991_palette_type2_w );
WRITE8_HANDLER( nbmj8991_palette_type3_w );
WRITE8_HANDLER( nbmj8991_blitter_w );
READ8_HANDLER( nbmj8991_clut_r );
WRITE8_HANDLER( nbmj8991_clut_w );
