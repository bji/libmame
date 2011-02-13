
#define STARFIRE_MASTER_CLOCK	(20000000)
#define STARFIRE_CPU_CLOCK		(STARFIRE_MASTER_CLOCK / 8)
#define STARFIRE_PIXEL_CLOCK	(STARFIRE_MASTER_CLOCK / 4)
#define STARFIRE_HTOTAL			(0x13f)  /* could be 0x140, but I think this is right */
#define STARFIRE_HBEND			(0x000)
#define STARFIRE_HBSTART		(0x100)
#define STARFIRE_VTOTAL			(0x106)
#define STARFIRE_VBEND			(0x020)
#define STARFIRE_VBSTART		(0x100)
#define	STARFIRE_NUM_PENS       (0x40)

class starfire_state : public driver_device
{
public:
	starfire_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

    read8_space_func input_read;

	UINT8 fireone_select;

    UINT8 starfire_vidctrl;
    UINT8 starfire_vidctrl1;
    UINT8 starfire_color;
    UINT16 starfire_colors[STARFIRE_NUM_PENS];

    UINT8 *starfire_videoram;
    UINT8 *starfire_colorram;

    emu_timer* scanline_timer;
    bitmap_t *starfire_screen;
};

/*----------- defined in video/starfire.c -----------*/

VIDEO_UPDATE( starfire );
VIDEO_START( starfire );

WRITE8_HANDLER( starfire_videoram_w );
READ8_HANDLER( starfire_videoram_r );
WRITE8_HANDLER( starfire_colorram_w );
READ8_HANDLER( starfire_colorram_r );

