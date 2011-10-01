/*************************************************************************

    Operation Wolf

*************************************************************************/

class opwolf_state : public driver_device
{
public:
	opwolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *      m_cchip_ram;

	/* video-related */
	UINT16       m_sprite_ctrl;
	UINT16       m_sprites_flipscreen;

	/* misc */
	UINT8        m_adpcm_b[0x08];
	UINT8        m_adpcm_c[0x08];
	UINT32       m_adpcm_pos[2];
	UINT32       m_adpcm_end[2];
	int          m_adpcm_data[2];

	int          m_opwolf_gun_xoffs;
	int          m_opwolf_gun_yoffs;

	/* c-chip */
	int          m_opwolf_region;

	UINT8        m_current_bank;
	UINT8        m_current_cmd;
	UINT8        m_cchip_last_7a;
	UINT8        m_cchip_last_04;
	UINT8        m_cchip_last_05;
	UINT8        m_cchip_coins_for_credit[2];
	UINT8        m_cchip_credits_for_coin[2];
	UINT8        m_cchip_coins[2];
	UINT8        m_c588;
	UINT8        m_c589;
	UINT8        m_c58a; // These variables derived from the bootleg

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_pc080sn;
	device_t *m_pc090oj;
	device_t *m_msm1;
	device_t *m_msm2;
};


/*----------- defined in machine/opwolf.c -----------*/

void opwolf_cchip_init(running_machine &machine);

READ16_HANDLER( opwolf_cchip_status_r );
READ16_HANDLER( opwolf_cchip_data_r );
WRITE16_HANDLER( opwolf_cchip_status_w );
WRITE16_HANDLER( opwolf_cchip_data_w );
WRITE16_HANDLER( opwolf_cchip_bank_w );


/*----------- defined in video/opwolf.c -----------*/

WRITE16_HANDLER( opwolf_spritectrl_w );

SCREEN_UPDATE( opwolf );
