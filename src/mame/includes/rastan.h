/*************************************************************************

    Rastan

*************************************************************************/

class rastan_state : public driver_device
{
public:
	rastan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT16 *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	UINT16      m_sprite_ctrl;
	UINT16      m_sprites_flipscreen;

	/* misc */
	int         m_adpcm_pos;
	int         m_adpcm_data;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_pc090oj;
	device_t *m_pc080sn;
};


/*----------- defined in video/rastan.c -----------*/

WRITE16_HANDLER( rastan_spritectrl_w );

SCREEN_UPDATE_IND16( rastan );
