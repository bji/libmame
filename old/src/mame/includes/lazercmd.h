#define HORZ_RES		32
#define VERT_RES		24
#define HORZ_CHR        8
#define VERT_CHR		10
#define VERT_FNT		8

#define HORZ_BAR        0x40
#define VERT_BAR        0x80

#define MARKER_ACTIVE_L 0x03
#define MARKER_ACTIVE_R 0x04
#define MARKER_VERT_R   0x0a
#define MARKER_HORZ_L   0x0b
#define MARKER_VERT_L   0x0c
#define MARKER_HORZ_R   0x0d

#define MARKER_HORZ_ADJ -1
#define MARKER_VERT_ADJ -10

class lazercmd_state : public driver_device
{
public:
	lazercmd_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	size_t   videoram_size;

	/* video-related */
	int      marker_x, marker_y;

	/* misc */
	int      timer_count, sense_state, dac_data;

	/* device */
	device_t *dac;
};


/*----------- defined in video/lazercmd.c -----------*/

SCREEN_UPDATE( lazercmd );
