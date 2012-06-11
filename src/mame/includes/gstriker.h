#ifndef __GSTRIKER_H
#define __GSTRIKER_H

/*** VS920A **********************************************/

#define MAX_VS920A 2

typedef struct
{
	tilemap_t* tmap;
	UINT16* vram;
	UINT16 pal_base;
	UINT8 gfx_region;

} sVS920A;

/*** MB60553 **********************************************/

#define MAX_MB60553 2

typedef struct
{
	tilemap_t* tmap;
	UINT16* vram;
	UINT16 regs[8];
	UINT8 bank[8];
	UINT16 pal_base;
	UINT8 gfx_region;

} tMB60553;

/*** CG10103 **********************************************/

#define MAX_CG10103 2

typedef struct
{
	UINT16* vram;
	UINT16 pal_base;
	UINT8 gfx_region;
	UINT8 transpen;

} tCG10103;

class gstriker_state : public driver_device
{
public:
	gstriker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 m_dmmy_8f_ret;
	int m_pending_command;
	UINT16 *m_work_ram;
	int m_gametype;
	UINT16 m_mcu_data;
	UINT16 m_prot_reg[2];
	UINT16 *m_lineram;
	sVS920A m_VS920A[MAX_VS920A];
	tMB60553 m_MB60553[MAX_MB60553];
	tCG10103 m_CG10103[MAX_CG10103];
	sVS920A* m_VS920A_cur_chip;
	tMB60553 *m_MB60553_cur_chip;
	tCG10103* m_CG10103_cur_chip;
};


/*----------- defined in video/gstriker.c -----------*/

WRITE16_HANDLER( VS920A_0_vram_w );
WRITE16_HANDLER( VS920A_1_vram_w );
WRITE16_HANDLER( MB60553_0_regs_w );
WRITE16_HANDLER( MB60553_1_regs_w );
WRITE16_HANDLER( MB60553_0_vram_w );
WRITE16_HANDLER( MB60553_1_vram_w );

SCREEN_UPDATE_IND16( gstriker );
VIDEO_START( gstriker );
VIDEO_START( twrldc94 );
VIDEO_START( vgoalsoc );
#endif
