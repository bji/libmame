/*************************************************************************

    Venture Line Super Rider driver

**************************************************************************/

class suprridr_state : public driver_device
{
public:
	suprridr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_nmi_enable;
	UINT8 m_sound_data;
	UINT8 *m_fgram;
	UINT8 *m_bgram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg_tilemap_noscroll;
	UINT8 m_flipx;
	UINT8 m_flipy;
	UINT8 *m_spriteram;
};


/*----------- defined in video/suprridr.c -----------*/

VIDEO_START( suprridr );
PALETTE_INIT( suprridr );

WRITE8_HANDLER( suprridr_flipx_w );
WRITE8_HANDLER( suprridr_flipy_w );
WRITE8_HANDLER( suprridr_fgdisable_w );
WRITE8_HANDLER( suprridr_fgscrolly_w );
WRITE8_HANDLER( suprridr_bgscrolly_w );
int suprridr_is_screen_flipped(running_machine &machine);

WRITE8_HANDLER( suprridr_fgram_w );
WRITE8_HANDLER( suprridr_bgram_w );

SCREEN_UPDATE_IND16( suprridr );
