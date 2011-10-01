class shangha3_state : public driver_device
{
public:
	shangha3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_prot_count;
	UINT16 *m_ram;
	size_t m_ram_size;

	int m_do_shadows;

	UINT16 m_gfxlist_addr;
	bitmap_t *m_rawbitmap;

	UINT8 m_drawmode_table[16];
};


/*----------- defined in video/shangha3.c -----------*/

WRITE16_HANDLER( shangha3_flipscreen_w );
WRITE16_HANDLER( shangha3_gfxlist_addr_w );
WRITE16_HANDLER( shangha3_blitter_go_w );
VIDEO_START( shangha3 );
SCREEN_UPDATE( shangha3 );
