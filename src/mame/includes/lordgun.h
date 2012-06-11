/*************************************************************************

                      -= IGS Lord Of Gun =-

*************************************************************************/

typedef struct _lordgun_gun_data lordgun_gun_data;
struct _lordgun_gun_data
{
	int		scr_x,	scr_y;
	UINT16	hw_x,	hw_y;
};

class lordgun_state : public driver_device
{
public:
	lordgun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_priority_ram(*this, "priority_ram"),
		  m_scrollram(*this, "scrollram"),
	      m_spriteram(*this, "spriteram"),
	      m_vram(*this, "vram"),
	      m_scroll_x(*this, "scroll_x"),
	      m_scroll_y(*this, "scroll_y") { }

	required_shared_ptr<UINT16> m_priority_ram;
	required_shared_ptr<UINT16> m_scrollram;
	required_shared_ptr<UINT16> m_spriteram;

	UINT8 m_old;
	UINT8 m_aliencha_dip_sel;
	UINT16 m_priority;
	required_shared_ptr_array<UINT16, 4> m_vram;
	required_shared_ptr_array<UINT16, 4> m_scroll_x;
	required_shared_ptr_array<UINT16, 4> m_scroll_y;
	int m_whitescreen;
	lordgun_gun_data m_gun[2];
	tilemap_t *m_tilemap[4];
	bitmap_ind16 *m_bitmaps[5];

	DECLARE_WRITE16_MEMBER(lordgun_priority_w);
	DECLARE_READ16_MEMBER(lordgun_gun_0_x_r);
	DECLARE_READ16_MEMBER(lordgun_gun_0_y_r);
	DECLARE_READ16_MEMBER(lordgun_gun_1_x_r);
	DECLARE_READ16_MEMBER(lordgun_gun_1_y_r);
	DECLARE_WRITE16_MEMBER(lordgun_soundlatch_w);
	DECLARE_WRITE16_MEMBER(lordgun_paletteram_w);
	DECLARE_WRITE16_MEMBER(lordgun_vram_0_w);
	DECLARE_WRITE16_MEMBER(lordgun_vram_1_w);
	DECLARE_WRITE16_MEMBER(lordgun_vram_2_w);
	DECLARE_WRITE16_MEMBER(lordgun_vram_3_w);
};


/*----------- defined in video/lordgun.c -----------*/


float lordgun_crosshair_mapper(const ioport_field *field, float linear_value);
void lordgun_update_gun(running_machine &machine, int i);

VIDEO_START( lordgun );
SCREEN_UPDATE_IND16( lordgun );
