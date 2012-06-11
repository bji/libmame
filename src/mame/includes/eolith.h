class eolith_state : public driver_device
{
public:
	eolith_state(const machine_config &mconfig, device_type type, const char *tag)
		:	driver_device(mconfig, type, tag),
			m_maincpu(*this,"maincpu")
			{ }

	int m_coin_counter_bit;
	int m_buffer;
	UINT32 *m_vram;

	required_device<cpu_device> m_maincpu;
};


/*----------- defined in video/eolith.c -----------*/

READ32_HANDLER( eolith_vram_r );
WRITE32_HANDLER( eolith_vram_w );
VIDEO_START( eolith );
SCREEN_UPDATE_IND16( eolith );
