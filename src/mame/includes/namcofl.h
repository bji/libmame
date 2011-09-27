#define NAMCOFL_HTOTAL		(288)	/* wrong */
#define NAMCOFL_HBSTART	(288)
#define NAMCOFL_VTOTAL		(262)	/* needs to be checked */
#define NAMCOFL_VBSTART	(224)

#define NAMCOFL_TILEMASKREGION		"tilemask"
#define NAMCOFL_TILEGFXREGION		"tile"
#define NAMCOFL_SPRITEGFXREGION	"sprite"
#define NAMCOFL_ROTMASKREGION		"rotmask"
#define NAMCOFL_ROTGFXREGION		"rot"

#define NAMCOFL_TILEGFX		0
#define NAMCOFL_SPRITEGFX		1
#define NAMCOFL_ROTGFX			2

class namcofl_state : public driver_device
{
public:
	namcofl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	emu_timer *m_raster_interrupt_timer;
	UINT32 *m_workram;
	UINT16 *m_shareram;
	UINT8 m_mcu_port6;
	UINT32 m_sprbank;
};


/*----------- defined in video/namcofl.c -----------*/

VIDEO_START( namcofl );
SCREEN_UPDATE( namcofl );

WRITE32_HANDLER( namcofl_spritebank_w );
