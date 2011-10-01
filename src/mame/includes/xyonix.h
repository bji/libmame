class xyonix_state : public driver_device
{
public:
	xyonix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_vidram;
	tilemap_t *m_tilemap;

	int m_e0_data;
	int m_credits;
	int m_coins;
	int m_prev_coin;
};


/*----------- defined in video/xyonix.c -----------*/

PALETTE_INIT( xyonix );
WRITE8_HANDLER( xyonix_vidram_w );
VIDEO_START(xyonix);
SCREEN_UPDATE(xyonix);
