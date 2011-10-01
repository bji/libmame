static const UINT16 dsp56k_bank00_size = 0x1000;
static const UINT16 dsp56k_bank01_size = 0x1000;
static const UINT16 dsp56k_bank02_size = 0x4000;
static const UINT16 dsp56k_shared_ram_16_size = 0x2000;
static const UINT16 dsp56k_bank04_size = 0x1fc0;

class polygonet_state : public driver_device
{
public:
	polygonet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* 68k-side shared ram */
	UINT32* m_shared_ram;

	UINT16* m_dsp56k_p_mirror;
	UINT16* m_dsp56k_p_8000;
	int m_cur_sound_region;

	direct_update_delegate m_dsp56k_update_handler;

	/* TTL text plane stuff */
	int m_ttl_gfx_index;
	tilemap_t *m_ttl_tilemap;
	tilemap_t *m_roz_tilemap;
	UINT16 m_ttl_vram[0x800];
	UINT16 m_roz_vram[0x800];

	/* memory buffers */
	UINT16 m_dsp56k_bank00_ram[2 * 8 * dsp56k_bank00_size];	/* 2 bank sets, 8 potential banks each */
	UINT16 m_dsp56k_bank01_ram[2 * 8 * dsp56k_bank01_size];
	UINT16 m_dsp56k_bank02_ram[2 * 8 * dsp56k_bank02_size];
	UINT16 m_dsp56k_shared_ram_16[2 * 8 * dsp56k_shared_ram_16_size];
	UINT16 m_dsp56k_bank04_ram[2 * 8 * dsp56k_bank04_size];
};

/*----------- defined in video/plygonet.c -----------*/

VIDEO_START( polygonet );
SCREEN_UPDATE( polygonet );

READ32_HANDLER( polygonet_ttl_ram_r );
WRITE32_HANDLER( polygonet_ttl_ram_w );
READ32_HANDLER( polygonet_roz_ram_r );
WRITE32_HANDLER( polygonet_roz_ram_w );
