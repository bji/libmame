#include "video/bufsprite.h"

class lemmings_state : public driver_device
{
public:
	lemmings_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_bitmap0(2048, 256),
		  m_spriteram(*this, "spriteram"),
		  m_spriteram2(*this, "spriteram2") ,
		m_paletteram(*this, "paletteram"),
		m_control_data(*this, "control_data"),
		m_vram_data(*this, "vram_data"),
		m_pixel_0_data(*this, "pixel_0_data"),
		m_pixel_1_data(*this, "pixel_1_data"){ }

	/* video-related */
	bitmap_ind16 m_bitmap0;
	tilemap_t *m_vram_tilemap;
	UINT16 m_sprite_triple_buffer_0[0x800];
	UINT16 m_sprite_triple_buffer_1[0x800];
	UINT8 m_vram_buffer[2048 * 64];	// 64 bytes per VRAM character
	/* devices */
	device_t *m_audiocpu;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<buffered_spriteram16_device> m_spriteram2;
	/* memory pointers */
	required_shared_ptr<UINT16> m_paletteram;
	required_shared_ptr<UINT16> m_control_data;
	required_shared_ptr<UINT16> m_vram_data;
	required_shared_ptr<UINT16> m_pixel_0_data;
	required_shared_ptr<UINT16> m_pixel_1_data;

	DECLARE_WRITE16_MEMBER(lemmings_control_w);
	DECLARE_READ16_MEMBER(lemmings_trackball_r);
	DECLARE_READ16_MEMBER(lemmings_prot_r);
	DECLARE_WRITE16_MEMBER(lemmings_palette_24bit_w);
	DECLARE_WRITE16_MEMBER(lemmings_sound_w);
	DECLARE_WRITE8_MEMBER(lemmings_sound_ack_w);
	DECLARE_WRITE16_MEMBER(lemmings_pixel_0_w);
	DECLARE_WRITE16_MEMBER(lemmings_pixel_1_w);
	DECLARE_WRITE16_MEMBER(lemmings_vram_w);
};


/*----------- defined in video/lemmings.c -----------*/


VIDEO_START( lemmings );
SCREEN_VBLANK( lemmings );
SCREEN_UPDATE_RGB32( lemmings );

