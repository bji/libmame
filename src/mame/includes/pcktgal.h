class pcktgal_state : public driver_device
{
public:
	pcktgal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_msm5205next;
	int m_toggle;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/pcktgal.c -----------*/


PALETTE_INIT( pcktgal );
SCREEN_UPDATE( pcktgal );
SCREEN_UPDATE( pcktgalb );
