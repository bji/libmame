class rollrace_state : public driver_device
{
public:
	rollrace_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	int m_ra_charbank[2];
	int m_ra_bkgpage;
	int m_ra_bkgflip;
	int m_ra_chrbank;
	int m_ra_bkgpen;
	int m_ra_bkgcol;
	int m_ra_flipy;
	int m_ra_flipx;
	int m_ra_spritebank;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/rollrace.c -----------*/

PALETTE_INIT( rollrace );
SCREEN_UPDATE( rollrace );

WRITE8_HANDLER( rollrace_charbank_w );
WRITE8_HANDLER( rollrace_backgroundpage_w );
WRITE8_HANDLER( rollrace_backgroundcolor_w );
WRITE8_HANDLER( rollrace_bkgpen_w );
WRITE8_HANDLER( rollrace_flipy_w );
WRITE8_HANDLER( rollrace_spritebank_w );
WRITE8_HANDLER( rollrace_flipx_w );

