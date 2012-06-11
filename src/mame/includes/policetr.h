/*************************************************************************

    P&P Marketing Police Trainer hardware

**************************************************************************/

class policetr_state : public driver_device
{
public:
	policetr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 m_control_data;
	UINT32 m_bsmt_data_bank;
	UINT32 m_bsmt_data_offset;
	UINT32 *m_speedup_data;
	UINT64 m_last_cycles;
	UINT32 m_loop_count;
	offs_t m_speedup_pc;
	UINT32 *	m_rambase;
	UINT32 m_palette_offset;
	UINT8 m_palette_index;
	UINT8 m_palette_data[3];
	rectangle m_render_clip;
	UINT8 *m_srcbitmap;
	UINT8 *m_dstbitmap;
	UINT16 m_src_xoffs;
	UINT16 m_src_yoffs;
	UINT16 m_dst_xoffs;
	UINT16 m_dst_yoffs;
	UINT8 m_video_latch;
	UINT32 m_srcbitmap_height_mask;
};


/*----------- defined in video/policetr.c -----------*/

WRITE32_HANDLER( policetr_video_w );
READ32_HANDLER( policetr_video_r );

WRITE32_HANDLER( policetr_palette_offset_w );
WRITE32_HANDLER( policetr_palette_data_w );

VIDEO_START( policetr );
SCREEN_UPDATE_IND16( policetr );
