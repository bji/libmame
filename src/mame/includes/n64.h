#ifndef _INCLUDES_N64_H_
#define _INCLUDES_N64_H_

#include "cpu/rsp/rsp.h"
#include "video/n64.h"
#include "sound/dmadac.h"
#include "includes/n64.h"

/*----------- forward decls -----------*/

/*----------- driver state -----------*/

class _n64_state : public driver_device
{
public:
	_n64_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video-related */
	N64::RDP::Processor m_rdp;
};

/*----------- devices -----------*/

#define MCFG_N64_PERIPHS_ADD(_tag) \
    MCFG_DEVICE_ADD(_tag, N64PERIPH, 0)

#define AUDIO_DMA_DEPTH     2

class n64_periphs : public device_t
{
private:
	typedef struct
	{
		UINT32 address;
		UINT32 length;
	} AUDIO_DMA;

public:
    // construction/destruction
    n64_periphs(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ32_MEMBER( is64_r );
	DECLARE_WRITE32_MEMBER( is64_w );
	DECLARE_READ32_MEMBER( open_r );
	DECLARE_WRITE32_MEMBER( open_w );
	DECLARE_READ32_MEMBER( rdram_reg_r );
	DECLARE_WRITE32_MEMBER( rdram_reg_w );
	DECLARE_READ32_MEMBER( mi_reg_r );
	DECLARE_WRITE32_MEMBER( mi_reg_w );
	DECLARE_READ32_MEMBER( vi_reg_r );
	DECLARE_WRITE32_MEMBER( vi_reg_w );
	DECLARE_READ32_MEMBER( ai_reg_r );
	DECLARE_WRITE32_MEMBER( ai_reg_w );
	DECLARE_READ32_MEMBER( pi_reg_r );
	DECLARE_WRITE32_MEMBER( pi_reg_w );
	DECLARE_READ32_MEMBER( ri_reg_r );
	DECLARE_WRITE32_MEMBER( ri_reg_w );
	DECLARE_READ32_MEMBER( si_reg_r );
	DECLARE_WRITE32_MEMBER( si_reg_w );
	DECLARE_READ32_MEMBER( pif_ram_r );
	DECLARE_WRITE32_MEMBER( pif_ram_w );

	UINT32 sp_reg_r(UINT32 offset);
	void sp_reg_w(UINT32 offset, UINT32 data, UINT32 mem_mask);

	void sp_set_status(UINT32 status);
	void signal_rcp_interrupt(int interrupt);

	void ai_timer_tick();
	void pi_dma_tick();

	// Video Interface (VI) registers
	UINT32 vi_width;
	UINT32 vi_origin;
	UINT32 vi_control;
	UINT32 vi_blank;
	UINT32 vi_hstart;
	UINT32 vi_vstart;
	UINT32 vi_xscale;
	UINT32 vi_yscale;
	UINT32 vi_burst;
	UINT32 vi_vsync;
	UINT32 vi_hsync;
	UINT32 vi_leap;
	UINT32 vi_intr;
	UINT32 vi_vburst;

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();

private:
	address_space *mem_map;
	device_t *maincpu;
	device_t *rspcpu;

	void clear_rcp_interrupt(int interrupt);

	UINT8 is64_buffer[0x10000];

	// Audio Interface (AI) registers and functions
	void ai_dma();
	AUDIO_DMA *ai_fifo_get_top();
	void ai_fifo_push(UINT32 address, UINT32 length);
	void ai_fifo_pop();

	dmadac_sound_device *ai_dac[2];
	UINT32 ai_dram_addr;
	UINT32 ai_len;
	UINT32 ai_control;
	int ai_dacrate;
	int ai_bitrate;
	UINT32 ai_status;

	emu_timer *ai_timer;

	AUDIO_DMA ai_fifo[AUDIO_DMA_DEPTH];
	int ai_fifo_wpos;
	int ai_fifo_rpos;
	int ai_fifo_num;

	// Memory Interface (MI) registers
	UINT32 mi_version;
	UINT32 mi_interrupt;
	UINT32 mi_intr_mask;
	UINT32 mi_mode;

	// RDRAM Interface (RI) registers
	UINT32 rdram_regs[10];
	UINT32 ri_regs[8];

	// RSP Interface (SP) registers
	void sp_dma(int direction);

	UINT32 sp_mem_addr;
	UINT32 sp_dram_addr;
	int sp_dma_length;
	int sp_dma_count;
	int sp_dma_skip;
	UINT32 sp_semaphore;
	UINT32 dp_clock;

	// Peripheral Interface (PI) registers and functions
	void pi_dma();
	emu_timer *pi_dma_timer;
	UINT32 pi_dram_addr;
	UINT32 pi_cart_addr;
	UINT32 pi_first_dma;
	UINT32 pi_rd_len;
	UINT32 pi_wr_len;
	UINT32 pi_status;
	UINT32 pi_bsd_dom1_lat;
	UINT32 pi_bsd_dom1_pwd;
	UINT32 pi_bsd_dom1_pgs;
	UINT32 pi_bsd_dom1_rls;
	UINT32 pi_bsd_dom2_lat;
	UINT32 pi_bsd_dom2_pwd;
	UINT32 pi_bsd_dom2_pgs;
	UINT32 pi_bsd_dom2_rls;
	UINT32 pi_dma_dir;

	// Serial Interface (SI) registers and functions
	void pif_dma(int direction);
	void handle_pif();
	int pif_channel_handle_command(int channel, int slength, UINT8 *sdata, int rlength, UINT8 *rdata);
	UINT8 calc_mempack_crc(UINT8 *buffer, int length);
	UINT8 pif_ram[0x40];
	UINT8 pif_cmd[0x40];
	UINT32 si_dram_addr;
	UINT32 si_pif_addr;
	UINT32 si_pif_addr_rd64b;
	UINT32 si_pif_addr_wr64b;
	UINT32 si_status;
	UINT8 eeprom[512];
	UINT8 mempack[0x8000];
	UINT32 cic_status;

	// Video Interface (VI) functions
	void vi_recalculate_resolution();
};

// device type definition
extern const device_type N64PERIPH;

/*----------- defined in video/n64.c -----------*/

extern VIDEO_START( n64 );
extern SCREEN_UPDATE_RGB32( n64 );

#define DACRATE_NTSC	(48681812)
#define DACRATE_PAL	(49656530)
#define DACRATE_MPAL	(48628316)

/*----------- defined in machine/n64.c -----------*/

#define SP_INTERRUPT	0x1
#define SI_INTERRUPT	0x2
#define AI_INTERRUPT	0x4
#define VI_INTERRUPT	0x8
#define PI_INTERRUPT	0x10
#define DP_INTERRUPT	0x20

#define SP_STATUS_HALT			0x0001
#define SP_STATUS_BROKE			0x0002
#define SP_STATUS_DMABUSY		0x0004
#define SP_STATUS_DMAFULL		0x0008
#define SP_STATUS_IOFULL		0x0010
#define SP_STATUS_SSTEP			0x0020
#define SP_STATUS_INTR_BREAK	0x0040
#define SP_STATUS_SIGNAL0		0x0080
#define SP_STATUS_SIGNAL1		0x0100
#define SP_STATUS_SIGNAL2		0x0200
#define SP_STATUS_SIGNAL3		0x0400
#define SP_STATUS_SIGNAL4		0x0800
#define SP_STATUS_SIGNAL5		0x1000
#define SP_STATUS_SIGNAL6		0x2000
#define SP_STATUS_SIGNAL7		0x4000

#define DP_STATUS_XBUS_DMA		0x01
#define DP_STATUS_FREEZE		0x02
#define DP_STATUS_FLUSH			0x04

extern const rsp_config n64_rsp_config;

extern UINT32 *rdram;
extern UINT32 *rsp_imem;
extern UINT32 *rsp_dmem;

extern void dp_full_sync(running_machine &machine);
extern void signal_rcp_interrupt(running_machine &machine, int interrupt);

extern READ32_DEVICE_HANDLER( n64_sp_reg_r );
extern WRITE32_DEVICE_HANDLER( n64_sp_reg_w );
extern READ32_DEVICE_HANDLER( n64_dp_reg_r );
extern WRITE32_DEVICE_HANDLER( n64_dp_reg_w );

MACHINE_START( n64 );
MACHINE_RESET( n64 );

#endif
