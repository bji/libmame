/*****************************************************************************
 *
 * includes/3do.h
 *
 ****************************************************************************/

#ifndef _3DO_H_
#define _3DO_H_

typedef struct {
	/* 03180000 - 0318003f - configuration group */
	/* 03180040 - 0318007f - diagnostic UART */

	UINT8	cg_r_count;
	UINT8	cg_w_count;
	UINT32	cg_input;
	UINT32	cg_output;
} SLOW2;


typedef struct {
	UINT32	revision;		/* 03300000 */
	UINT32	msysbits;		/* 03300004 */
	UINT32	mctl;			/* 03300008 */
	UINT32	sltime;			/* 0330000c */
	UINT32	abortbits;		/* 03300020 */
	UINT32	privbits;		/* 03300024 */
	UINT32	statbits;		/* 03300028 */
	UINT32	diag;			/* 03300040 */

	UINT32	ccobctl0;		/* 03300110 */
	UINT32	ppmpc;			/* 03300120 */

	UINT32	regctl0;		/* 03300130 */
	UINT32	regctl1;		/* 03300134 */
	UINT32	regctl2;		/* 03300138 */
	UINT32	regctl3;		/* 0330013c */
	UINT32	xyposh;			/* 03300140 */
	UINT32	xyposl;			/* 03300144 */
	UINT32	linedxyh;		/* 03300148 */
	UINT32	linedxyl;		/* 0330014c */
	UINT32	dxyh;			/* 03300150 */
	UINT32	dxyl;			/* 03300154 */
	UINT32	ddxyh;			/* 03300158 */
	UINT32	ddxyl;			/* 0330015c */

	UINT32	pip[16];		/* 03300180-033001bc (W); 03300180-033001fc (R) */
	UINT32	fence[16];		/* 03300200-0330023c (W); 03300200-0330027c (R) */
	UINT32	mmu[64];		/* 03300300-033003fc */
	UINT32	dma[32][4];		/* 03300400-033005fc */
	UINT32	mult[40];		/* 03300600-0330069c */
	UINT32	mult_control;	/* 033007f0-033007f4 */
	UINT32	mult_status;	/* 033007f8 */
} MADAM;


typedef struct {
	screen_device *screen;

	UINT32	revision;		/* 03400000 */
	UINT32	csysbits;		/* 03400004 */
	UINT32	vint0;			/* 03400008 */
	UINT32	vint1;			/* 0340000c */
	UINT32	audin;			/* 03400020 */
	UINT32	audout;			/* 03400024 */
	UINT32	cstatbits;		/* 03400028 */
	UINT32	wdog;			/* 0340002c */
	UINT32	hcnt;			/* 03400030 */
	UINT32	vcnt;			/* 03400034 */
	UINT32	seed;			/* 03400038 */
	UINT32	random;			/* 0340004c */
	UINT32	irq0;			/* 03400040 / 03400044 */
	UINT32	irq0_enable;	/* 03400048 / 0340004c */
	UINT32	mode;			/* 03400050 / 03400054 */
	UINT32	badbits;		/* 03400058 */
	UINT32	spare;			/* 0340005c */
	UINT32	irq1;			/* 03400060 / 03400064 */
	UINT32	irq1_enable;	/* 03400068 / 0340006c */
	UINT32	hdelay;			/* 03400080 */
	UINT32	adbio;			/* 03400084 */
	UINT32	adbctl;			/* 03400088 */
							/* Timers */
	UINT32	timer0;			/* 03400100 */
	UINT32	timerback0;		/* 03400104 */
	UINT32	timer1;			/* 03400108 */
	UINT32	timerback1;		/* 0340010c */
	UINT32	timer2;			/* 03400110 */
	UINT32	timerback2;		/* 03400114 */
	UINT32	timer3;			/* 03400118 */
	UINT32	timerback3;		/* 0340011c */
	UINT32	timer4;			/* 03400120 */
	UINT32	timerback4;		/* 03400124 */
	UINT32	timer5;			/* 03400128 */
	UINT32	timerback5;		/* 0340012c */
	UINT32	timer6;			/* 03400130 */
	UINT32	timerback6;		/* 03400134 */
	UINT32	timer7;			/* 03400138 */
	UINT32	timerback7;		/* 0340013c */
	UINT32	timer8;			/* 03400140 */
	UINT32	timerback8;		/* 03400144 */
	UINT32	timer9;			/* 03400148 */
	UINT32	timerback9;		/* 0340014c */
	UINT32	timer10;		/* 03400150 */
	UINT32	timerback10;	/* 03400154 */
	UINT32	timer11;		/* 03400158 */
	UINT32	timerback11;	/* 0340015c */
	UINT32	timer12;		/* 03400160 */
	UINT32	timerback12;	/* 03400164 */
	UINT32	timer13;		/* 03400168 */
	UINT32	timerback13;	/* 0340016c */
	UINT32	timer14;		/* 03400170 */
	UINT32	timerback14;	/* 03400174 */
	UINT32	timer15;		/* 03400178 */
	UINT32	timerback15;	/* 0340017c */
	UINT32	settm0;			/* 03400200 */
	UINT32	clrtm0;			/* 03400204 */
	UINT32	settm1;			/* 03400208 */
	UINT32	clrtm1;			/* 0340020c */
	UINT32	slack;			/* 03400220 */
							/* DMA */
	UINT32	dmareqdis;		/* 03400308 */
							/* Expansion bus */
	UINT32	expctl;			/* 03400400/03400404 */
	UINT32	type0_4;		/* 03400408 */
	UINT32	dipir1;			/* 03400410 */
	UINT32	dipir2;			/* 03400414 */
							/* Bus signals */
	UINT32	sel;			/* 03400500 - 0340053f */
	UINT32	poll;			/* 03400540 - 0340057f */
	UINT32	cmdstat;		/* 03400580 - 034005bf */
	UINT32	data;			/* 034005c0 - 034005ff */
							/* DSPP */
	UINT32	semaphore;		/* 034017d0 */
	UINT32	semaack;		/* 034017d4 */
	UINT32	dsppdma;		/* 034017e0 */
	UINT32	dspprst0;		/* 034017e4 */
	UINT32	dspprst1;		/* 034017e8 */
	UINT32	dspppc;			/* 034017f4 */
	UINT32	dsppnr;			/* 034017f8 */
	UINT32	dsppgw;			/* 034017fc */
	UINT32	dsppn[0x400];	/* 03401800 - 03401bff DSPP N stack (32bit writes) */
							/* 03402000 - 034027ff DSPP N stack (16bit writes) */
	UINT32	dsppei[0x100];	/* 03403000 - 034030ff DSPP EI stack (32bit writes) */
							/* 03403400 - 034035ff DSPP EI stack (16bit writes) */
	UINT32	dsppeo[0x1f];	/* 03403800 - 0340381f DSPP EO stack (32bit reads) */
							/* 03403c00 - 03403c3f DSPP EO stack (32bit reads) */
	UINT32	dsppclkreload;	/* 034039dc / 03403fbc */
							/* UNCLE */
	UINT32	unclerev;		/* 0340c000 */
	UINT32	uncle_soft_rev;	/* 0340c004 */
	UINT32	uncle_addr;		/* 0340c008 */
	UINT32	uncle_rom;		/* 0340c00c */
} CLIO;


typedef struct {
	UINT32	sport[512];
	UINT32	color;
} SVF;


class _3do_state : public driver_device
{
public:
	_3do_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	legacy_cpu_device* m_maincpu;
	UINT32 *m_dram;
	UINT32 *m_vram;
	SLOW2 m_slow2;
	MADAM m_madam;
	CLIO m_clio;
	SVF m_svf;
	UINT8 m_video_bits[512];
};

/*----------- defined in machine/3do.c -----------*/

READ32_HANDLER( _3do_nvarea_r );
WRITE32_HANDLER( _3do_nvarea_w );

READ32_HANDLER( _3do_slow2_r );
WRITE32_HANDLER( _3do_slow2_w );
void _3do_slow2_init( running_machine &machine );

READ32_HANDLER( _3do_svf_r );
WRITE32_HANDLER( _3do_svf_w );

READ32_HANDLER( _3do_madam_r );
WRITE32_HANDLER( _3do_madam_w );
void _3do_madam_init( running_machine &machine );

READ32_HANDLER( _3do_clio_r );
WRITE32_HANDLER( _3do_clio_w );
void _3do_clio_init( running_machine &machine, screen_device *screen );


VIDEO_START( _3do );
SCREEN_UPDATE_RGB32( _3do );

#endif /* _3DO_H_ */
