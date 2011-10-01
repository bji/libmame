class exzisus_state : public driver_device
{
public:
	exzisus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_sharedram_ab;
	UINT8 *m_sharedram_ac;
	int m_cpua_bank;
	int m_cpub_bank;
	UINT8 *m_videoram0;
	UINT8 *m_videoram1;
	UINT8 *m_objectram0;
	UINT8 *m_objectram1;
	size_t m_objectram_size0;
	size_t m_objectram_size1;
};


/*----------- defined in video/exzisus.c -----------*/

READ8_HANDLER( exzisus_videoram_0_r );
READ8_HANDLER( exzisus_videoram_1_r );
READ8_HANDLER( exzisus_objectram_0_r );
READ8_HANDLER( exzisus_objectram_1_r );
WRITE8_HANDLER( exzisus_videoram_0_w );
WRITE8_HANDLER( exzisus_videoram_1_w );
WRITE8_HANDLER( exzisus_objectram_0_w );
WRITE8_HANDLER( exzisus_objectram_1_w );

SCREEN_UPDATE( exzisus );


