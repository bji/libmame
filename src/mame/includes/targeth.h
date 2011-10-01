class targeth_state : public driver_device
{
public:
	targeth_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_vregs;
	UINT16 *m_videoram;
	UINT16 *m_spriteram;
	tilemap_t *m_pant[2];
};


/*----------- defined in video/targeth.c -----------*/

WRITE16_HANDLER( targeth_vram_w );
VIDEO_START( targeth );
SCREEN_UPDATE( targeth );
