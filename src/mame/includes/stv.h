/*----------- defined in drivers/stv.c -----------*/
#define NEW_VIDEO_CODE 0

class saturn_state : public driver_device
{
public:
	saturn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32    *m_workram_l;
	UINT32    *m_workram_h;
	UINT8     *m_backupram;
	UINT8     *m_cart_backupram;
	UINT32    *m_scu_regs;
	UINT16    *m_sound_ram;
	UINT16    *m_scsp_regs;
	UINT16    *m_vdp2_regs;
	UINT32    *m_vdp2_vram;
	UINT32    *m_vdp2_cram;
    UINT32    *m_vdp1_vram;
    UINT16    *m_vdp1_regs;

	UINT8     m_NMI_reset;
	UINT8     m_en_68k;

	struct {
		UINT32    src[3];		/* Source DMA lv n address*/
		UINT32    dst[3];		/* Destination DMA lv n address*/
		UINT32    src_add[3];	/* Source Addition for DMA lv n*/
		UINT32    dst_add[3];	/* Destination Addition for DMA lv n*/
		UINT32    size[3];		/* Transfer DMA size lv n*/
		UINT32    index[3];
		int       start_factor[3];
		UINT8     enable_mask[3];
		UINT32    ist;
		UINT32    ism;
	}m_scu;

	int       m_minit_boost;
	int       m_sinit_boost;
	attotime  m_minit_boost_timeslice;
	attotime  m_sinit_boost_timeslice;

	struct {
		UINT16    **framebuffer_display_lines;
		int       framebuffer_mode;
		int       framebuffer_double_interlace;
		int	      fbcr_accessed;
        int       framebuffer_width;
        int       framebuffer_height;
        int       framebuffer_current_display;
        int	      framebuffer_current_draw;
        int	      framebuffer_clear_on_next_frame;
		rectangle system_cliprect;
		rectangle user_cliprect;
        UINT16	  *framebuffer[2];
        UINT16	  **framebuffer_draw_lines;
	    UINT8     *gfx_decode;
		UINT16    lopr;
		UINT16    copr;
		UINT16    ewdr;

		int       local_x;
		int       local_y;
	}m_vdp1;

	struct {
	    UINT8     *gfx_decode;
	    bitmap_rgb32 roz_bitmap[2];
	    UINT8     dotsel;
	    UINT8     pal;
	    UINT16    h_count;
	    UINT16    v_count;
	    UINT8     exltfg;
	    UINT8     exsyfg;
		int       old_crmd;
		int       old_tvmd;
	}m_vdp2;

	struct {
		UINT8 IOSEL1;
        UINT8 IOSEL2;
        UINT8 EXLE1;
        UINT8 EXLE2;
        UINT8 PDR1;
        UINT8 PDR2;
        UINT8 DDR1;
        UINT8 DDR2;
        UINT8 SF;
        UINT8 SR;
        UINT8 IREG[7];
        UINT8 OREG[32];
        int   intback_stage;
        int   pmode;
        UINT8 SMEM[4];
        UINT8 intback;
        UINT8 rtc_data[7];
	}m_smpc;

	struct {
		UINT8 status;
		UINT8 data;
	}m_keyb;

	/* Saturn specific*/
	int m_saturn_region;
	UINT8 m_cart_type;
	UINT32 *m_cart_dram;

	/* ST-V specific */
	UINT8     m_stv_multi_bank;
	UINT8     m_prev_bankswitch;
    emu_timer *m_stv_rtc_timer;
	UINT8     m_port_sel,m_mux_data;
	UINT8     m_system_output;
	UINT16    m_serial_tx;

	legacy_cpu_device* m_maincpu;
	legacy_cpu_device* m_slave;
	legacy_cpu_device* m_audiocpu;

	bitmap_rgb32 m_tmpbitmap;
};

#define MASTER_CLOCK_352 57272720
#define MASTER_CLOCK_320 53693174
#define CEF_1	state->m_vdp1_regs[0x010/2]|=0x0002
#define CEF_0   state->m_vdp1_regs[0x010/2]&=~0x0002
#define BEF_1	state->m_vdp1_regs[0x010/2]|=0x0001
#define BEF_0	state->m_vdp1_regs[0x010/2]&=~0x0001
#define STV_VDP1_TVMR ((state->m_vdp1_regs[0x000/2])&0xffff)
#define STV_VDP1_VBE  ((STV_VDP1_TVMR & 0x0008) >> 3)
#define STV_VDP1_TVM  ((STV_VDP1_TVMR & 0x0007) >> 0)

#define IRQ_VBLANK_IN  1 << 0
#define IRQ_VBLANK_OUT 1 << 1
#define IRQ_HBLANK_IN  1 << 2
#define IRQ_TIMER_0    1 << 3
#define IRQ_TIMER_1    1 << 4
#define IRQ_DSP_END    1 << 5
#define IRQ_SOUND_REQ  1 << 6
#define IRQ_SMPC       1 << 7
#define IRQ_PAD        1 << 8
#define IRQ_DMALV2     1 << 9
#define IRQ_DMALV1     1 << 10
#define IRQ_DMALV0     1 << 11
#define IRQ_DMAILL     1 << 12
#define IRQ_VDP1_END   1 << 13
#define IRQ_ABUS       1 << 15

DRIVER_INIT ( stv );


/*----------- defined in drivers/stv.c -----------*/

void install_stvbios_speedups(running_machine &machine);
DRIVER_INIT(mausuke);
DRIVER_INIT(puyosun);
DRIVER_INIT(shienryu);
DRIVER_INIT(prikura);
DRIVER_INIT(hanagumi);
DRIVER_INIT(cottonbm);
DRIVER_INIT(cotton2);
DRIVER_INIT(fhboxers);
DRIVER_INIT(dnmtdeka);
DRIVER_INIT(groovef);
DRIVER_INIT(danchih);
DRIVER_INIT(astrass);
DRIVER_INIT(thunt);
DRIVER_INIT(grdforce);
DRIVER_INIT(batmanfr);
DRIVER_INIT(winterht);
DRIVER_INIT(seabass);
DRIVER_INIT(vfremix);
DRIVER_INIT(diehard);
DRIVER_INIT(sss);
DRIVER_INIT(othellos);
DRIVER_INIT(sasissu);
DRIVER_INIT(gaxeduel);
DRIVER_INIT(suikoenb);
DRIVER_INIT(sokyugrt);
DRIVER_INIT(znpwfv);
DRIVER_INIT(twcup98);
DRIVER_INIT(smleague);
DRIVER_INIT(maruchan);
DRIVER_INIT(sandor);
DRIVER_INIT(colmns97);
DRIVER_INIT(pblbeach);
DRIVER_INIT(shanhigw);
DRIVER_INIT(finlarch);
DRIVER_INIT(elandore);
DRIVER_INIT(rsgun);
DRIVER_INIT(ffreveng);
DRIVER_INIT(decathlt);
DRIVER_INIT(nameclv3);

/*----------- defined in video/stvvdp1.c -----------*/

extern UINT16	**stv_framebuffer_display_lines;
extern int stv_framebuffer_double_interlace;
extern int stv_framebuffer_mode;
extern UINT8* stv_vdp1_gfx_decode;

int stv_vdp1_start ( running_machine &machine );
void video_update_vdp1(running_machine &machine);
void stv_vdp2_dynamic_res_change(running_machine &machine);

READ16_HANDLER ( saturn_vdp1_regs_r );
READ32_HANDLER ( saturn_vdp1_vram_r );
READ32_HANDLER ( saturn_vdp1_framebuffer0_r );

WRITE16_HANDLER ( saturn_vdp1_regs_w );
WRITE32_HANDLER ( saturn_vdp1_vram_w );
WRITE32_HANDLER ( saturn_vdp1_framebuffer0_w );

/*----------- defined in video/stvvdp2.c -----------*/

READ32_HANDLER ( saturn_vdp2_vram_r );
READ32_HANDLER ( saturn_vdp2_cram_r );
READ16_HANDLER ( saturn_vdp2_regs_r );

WRITE32_HANDLER ( saturn_vdp2_vram_w );
WRITE32_HANDLER ( saturn_vdp2_cram_w );
WRITE16_HANDLER ( saturn_vdp2_regs_w );

VIDEO_START ( stv_vdp2 );
SCREEN_UPDATE_RGB32( stv_vdp2 );
#if NEW_VIDEO_CODE
SCREEN_UPDATE_RGB32( saturn );
#endif
