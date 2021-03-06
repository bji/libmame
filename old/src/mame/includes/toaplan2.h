/**************** Machine stuff ******************/
//#define USE_HD64x180          /* Define if CPU support is available */
#define USE_ENCRYPTED_V25S    /* Define to enable V25 even on games where it is encrypted */

/* sub cpu */
#define CPU_2_NONE		0x00
#define CPU_2_Z80		0x5a
#define CPU_2_HD647180	0xa5
#define CPU_2_V25		0xff

// we encode priority with colour in the tilemaps, so need a larger palette
#define T2PALETTE_LENGTH 0x10000

/* vdp related */

#include "video/gp9001.h"

// cache the vdps for faster access
class toaplan2_state : public driver_device
{
public:
	toaplan2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config)
	{
		vdp0 = NULL;
		vdp1 = NULL;
	}

	gp9001vdp_device* vdp0;
	gp9001vdp_device* vdp1;

	UINT8 *z80_shared_ram;
	UINT16 *shared_ram16;	/* Really 8bit RAM connected to Z180 */

#ifndef USE_ENCRYPTED_V25S
	UINT16 *V25_shared_ram;			/* Really 8bit RAM connected to Z180 */
#endif
	UINT8* batsugun_share;


	int sub_cpu_type;
	device_t *sub_cpu;

	UINT16 mcu_data;
	UINT16 video_status;
	INT8 old_p1_paddle_h;		/* For Ghox */
	INT8 old_p2_paddle_h;
	UINT8 v25_reset_line;		/* 0x20 for dogyuun/batsugun, 0x10 for vfive, 0x08 for fixeight */
	INT8 current_bank;			/* Z80 bank used in Battle Garegga and Batrider */
	int sndirq_line;		/* IRQ4 for batrider, IRQ2 for bbakraid */
	UINT16 z80_busreq;
	int unlimited_ver;

	UINT16 *txvideoram16;
	UINT16 *txvideoram16_offs;
	UINT16 *txscrollram16;
	UINT16 *tx_gfxram16;

	size_t tx_vram_size;
	size_t tx_offs_vram_size;
	size_t tx_scroll_vram_size;
	size_t paletteram16_size;

	bitmap_t* custom_priority_bitmap;
	bitmap_t* secondary_render_bitmap;

	tilemap_t *tx_tilemap;	/* Tilemap for extra-text-layer */
	UINT8 tx_flip;
	int display_tx;
	int displog;
};


/*----------- defined in audio/toaplan2.c -----------*/

void fixeight_okisnd_w(device_t *device, int data);

/*----------- defined in video/toaplan2.c -----------*/

SCREEN_EOF( toaplan2 );
VIDEO_START( toaplan2 );
VIDEO_START( truxton2 );
VIDEO_START( fixeighb );
VIDEO_START( bgaregga );
VIDEO_START( batrider );

SCREEN_UPDATE( toaplan2 );
SCREEN_UPDATE( truxton2 );
SCREEN_UPDATE( batrider );
SCREEN_UPDATE( dogyuun );
SCREEN_UPDATE( batsugun );

/* non-vdp text layer */
READ16_HANDLER ( toaplan2_txvideoram16_r );
WRITE16_HANDLER( toaplan2_txvideoram16_w );
READ16_HANDLER ( toaplan2_txvideoram16_offs_r );
WRITE16_HANDLER( toaplan2_txvideoram16_offs_w );
READ16_HANDLER ( toaplan2_txscrollram16_r );
WRITE16_HANDLER( toaplan2_txscrollram16_w );
READ16_HANDLER ( toaplan2_tx_gfxram16_r );
WRITE16_HANDLER( toaplan2_tx_gfxram16_w );
READ16_HANDLER ( raizing_tx_gfxram16_r );
WRITE16_HANDLER( raizing_tx_gfxram16_w );

WRITE16_HANDLER( batrider_objectbank_w );
WRITE16_HANDLER( batrider_textdata_decode );

