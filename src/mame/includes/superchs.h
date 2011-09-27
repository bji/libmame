struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class superchs_state : public driver_device
{
public:
	superchs_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 m_coin_word;
	UINT32 *m_ram;
	UINT32 *m_shared_ram;
	int m_steer;
	struct tempsprite *m_spritelist;
	UINT32 m_mem[2];
	UINT32 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/superchs.c -----------*/

VIDEO_START( superchs );
SCREEN_UPDATE( superchs );
