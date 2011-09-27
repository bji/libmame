/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

#define EXIDY_MASTER_CLOCK				(XTAL_11_289MHz)
#define EXIDY_CPU_CLOCK					(EXIDY_MASTER_CLOCK / 16)
#define EXIDY_PIXEL_CLOCK				(EXIDY_MASTER_CLOCK / 2)
#define EXIDY_HTOTAL					(0x150)
#define EXIDY_HBEND						(0x000)
#define EXIDY_HBSTART					(0x100)
#define EXIDY_HSEND						(0x140)
#define EXIDY_HSSTART					(0x120)
#define EXIDY_VTOTAL					(0x118)
#define EXIDY_VBEND						(0x000)
#define EXIDY_VBSTART					(0x100)
#define EXIDY_VSEND						(0x108)
#define EXIDY_VSSTART					(0x100)


class exidy_state : public driver_device
{
public:
	exidy_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 m_last_dial;
	UINT8 *m_videoram;
	UINT8 *m_characterram;
	UINT8 *m_color_latch;
	UINT8 *m_sprite1_xpos;
	UINT8 *m_sprite1_ypos;
	UINT8 *m_sprite2_xpos;
	UINT8 *m_sprite2_ypos;
	UINT8 *m_spriteno;
	UINT8 *m_sprite_enable;
	UINT8 m_collision_mask;
	UINT8 m_collision_invert;
	int m_is_2bpp;
	UINT8 m_int_condition;
	bitmap_t *m_background_bitmap;
	bitmap_t *m_motion_object_1_vid;
	bitmap_t *m_motion_object_2_vid;
	bitmap_t *m_motion_object_2_clip;
};


/*----------- defined in video/exidy.c -----------*/

void exidy_video_config(running_machine &machine, UINT8 _collision_mask, UINT8 _collision_invert, int _is_2bpp);
VIDEO_START( exidy );
SCREEN_UPDATE( exidy );

INTERRUPT_GEN( exidy_vblank_interrupt );
INTERRUPT_GEN( teetert_vblank_interrupt );

READ8_HANDLER( exidy_interrupt_r );
