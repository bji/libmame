/*************************************************************************

    Express Raider

*************************************************************************/


class exprraid_state : public driver_device
{
public:
	exprraid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *        m_main_ram;
	UINT8 *        m_videoram;
	UINT8 *        m_colorram;
	UINT8 *        m_spriteram;
	size_t         m_spriteram_size;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	int            m_bg_index[4];

	/* misc */
	//int          m_coin;    // used in the commented out INTERRUPT_GEN - can this be removed?

	/* devices */
	device_t *m_maincpu;
	device_t *m_slave;
};


/*----------- defined in video/exprraid.c -----------*/

extern WRITE8_HANDLER( exprraid_videoram_w );
extern WRITE8_HANDLER( exprraid_colorram_w );
extern WRITE8_HANDLER( exprraid_flipscreen_w );
extern WRITE8_HANDLER( exprraid_bgselect_w );
extern WRITE8_HANDLER( exprraid_scrollx_w );
extern WRITE8_HANDLER( exprraid_scrolly_w );

extern VIDEO_START( exprraid );
extern SCREEN_UPDATE_IND16( exprraid );
