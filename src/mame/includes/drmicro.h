/*************************************************************************

    Dr. Micro

*************************************************************************/


class drmicro_state : public driver_device
{
public:
	drmicro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_videoram;

	/* video-related */
	tilemap_t        *m_bg1;
	tilemap_t        *m_bg2;
	int            m_flipscreen;

	/* misc */
	int            m_nmi_enable;
	int            m_pcm_adr;

	/* devices */
	device_t *m_msm;
};


/*----------- defined in video/drmicro.c -----------*/

PALETTE_INIT( drmicro );
VIDEO_START( drmicro );
SCREEN_UPDATE( drmicro );

WRITE8_HANDLER( drmicro_videoram_w );
