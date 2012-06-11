/*************************************************************************

    Hana Yayoi & other Dynax games (using 1st version of their blitter)

*************************************************************************/

class hnayayoi_state : public driver_device
{
public:
	hnayayoi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video-related */
	UINT8      *m_pixmap[8];
	int        m_palbank;
	int        m_total_pixmaps;
	UINT8      m_blit_layer;
	UINT16     m_blit_dest;
	UINT32     m_blit_src;

	/* misc */
	int        m_keyb;
};


/*----------- defined in video/hnayayoi.c -----------*/

VIDEO_START( hnayayoi );
VIDEO_START( untoucha );
SCREEN_UPDATE_IND16( hnayayoi );

WRITE8_HANDLER( dynax_blitter_rev1_param_w );
WRITE8_HANDLER( dynax_blitter_rev1_start_w );
WRITE8_HANDLER( dynax_blitter_rev1_clear_w );
WRITE8_HANDLER( hnayayoi_palbank_w );
