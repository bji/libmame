
#include "emu.h"
#include "coreutil.h"
#include "cpu/m68000/m68000.h"
#include "cpu/sh2/sh2.h"
#include "cpu/sh2/sh2comn.h"
#include "cpu/z80/z80.h"
#include "sound/2612intf.h"
#include "sound/cdda.h"
#include "sound/dac.h"
#include "sound/rf5c68.h"
#include "sound/sn76496.h"
#include "imagedev/chd_cd.h"
#include "machine/nvram.h"
#include "cpu/ssp1601/ssp1601.h"

#define MASTER_CLOCK_NTSC 53693175
#define MASTER_CLOCK_PAL  53203424
#define SEGACD_CLOCK      12500000



/*----------- defined in machine/megadriv.c -----------*/

extern DRIVER_INIT( megadriv_c2 );
extern DRIVER_INIT( megadrie );
extern DRIVER_INIT( megadriv );
extern DRIVER_INIT( megadrij );
extern DRIVER_INIT( _32x );
extern DRIVER_INIT( mpnew );

INPUT_PORTS_EXTERN( md_common );
INPUT_PORTS_EXTERN( megadriv );
INPUT_PORTS_EXTERN( megadri6 );
INPUT_PORTS_EXTERN( ssf2mdb );
INPUT_PORTS_EXTERN( mk3mdb );
INPUT_PORTS_EXTERN( megdsvp );

MACHINE_CONFIG_EXTERN( megadriv_timers );
MACHINE_CONFIG_EXTERN( md_ntsc );
MACHINE_CONFIG_EXTERN( md_pal );
MACHINE_CONFIG_EXTERN( md_svp );

MACHINE_CONFIG_EXTERN( megdsvppal );
MACHINE_CONFIG_EXTERN( megadriv );
MACHINE_CONFIG_EXTERN( megadpal );
MACHINE_CONFIG_EXTERN( megdsvp );
MACHINE_CONFIG_EXTERN( genesis_32x );
MACHINE_CONFIG_EXTERN( genesis_32x_pal );
MACHINE_CONFIG_EXTERN( genesis_scd );
MACHINE_CONFIG_EXTERN( genesis_scd_scd );
MACHINE_CONFIG_EXTERN( genesis_scd_mcd );
MACHINE_CONFIG_EXTERN( genesis_scd_mcdj );
MACHINE_CONFIG_EXTERN( genesis_32x_scd );
MACHINE_CONFIG_EXTERN( md_bootleg );	// for topshoot.c & hshavoc.c

extern UINT16* megadriv_backupram;
extern int megadriv_backupram_length;
extern UINT16* megadrive_ram;

extern UINT8 megatech_bios_port_cc_dc_r(running_machine &machine, int offset, int ctrl);
extern void megadriv_stop_scanline_timer(void);

void megatech_set_megadrive_z80_as_megadrive_z80(running_machine &machine, const char* tag);

extern READ16_HANDLER( megadriv_vdp_r );
extern WRITE16_HANDLER( megadriv_vdp_w );

/* These handlers are needed by megaplay.c */
extern READ16_HANDLER( megadriv_68k_io_read );
extern WRITE16_HANDLER( megadriv_68k_io_write );

/* These handlers are needed by puckpkmn.c for his memory map */
extern READ8_DEVICE_HANDLER( megadriv_68k_YM2612_read);
extern WRITE8_DEVICE_HANDLER( megadriv_68k_YM2612_write);

/* These are needed to create external input handlers (see e.g. MESS) */
/* Regs are also used by Megaplay! */
extern UINT8 (*megadrive_io_read_data_port_ptr)(running_machine &machine, int offset);
extern void (*megadrive_io_write_data_port_ptr)(running_machine &machine, int offset, UINT16 data);
extern UINT8 megadrive_io_data_regs[3];
extern UINT8 megadrive_io_ctrl_regs[3];

MACHINE_START( megadriv );
MACHINE_RESET( megadriv );
VIDEO_START( megadriv );
SCREEN_UPDATE_RGB32( megadriv );
SCREEN_VBLANK( megadriv );


struct genesis_z80_vars
{
	int z80_is_reset;
	int z80_has_bus;
	UINT32 z80_bank_addr;
	UINT8* z80_prgram;
};

extern genesis_z80_vars genz80;

extern UINT16* megadrive_vdp_palette_lookup;
extern UINT16* megadrive_vdp_palette_lookup_sprite; // for C2
extern UINT16* megadrive_vdp_palette_lookup_shadow;
extern UINT16* megadrive_vdp_palette_lookup_highlight;

extern int segac2_bg_pal_lookup[4];
extern int segac2_sp_pal_lookup[4];

extern int genvdp_use_cram;
extern int genesis_always_irq6;
extern int genesis_other_hacks;

extern int megadrive_6buttons_pad;
extern int megadrive_region_export;
extern int megadrive_region_pal;

/* Megaplay - Megatech specific */
/* It might be possible to move the following structs in the drivers */

#define MP_ROM  0x10
#define MP_GAME 0

class md_base_state : public driver_device
{
public:
	md_base_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag) { }
};

class md_boot_state : public md_base_state
{
public:
	md_boot_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag) { m_protcount = 0;}

	// bootleg specific
	int m_aladmdb_mcu_port;

	int m_protcount;

	// jzth protection
	DECLARE_WRITE16_MEMBER( bl_710000_w )
	{
		int pc = cpu_get_pc(&space.device());

		logerror("%06x writing to bl_710000_w %04x %04x\n", pc, data, mem_mask);

		// protection value is read from  0x710000 after a series of writes.. and stored at ff0007
		// startup
		/*
        059ce0 writing to bl_710000_w ff08 ffff
        059d04 writing to bl_710000_w 000a ffff
        059d04 writing to bl_710000_w 000b ffff
        059d04 writing to bl_710000_w 000c ffff
        059d04 writing to bl_710000_w 000f ffff
        059d1c writing to bl_710000_w ff09 ffff
        059d2a reading from bl_710000_r  (wants 0xe)
        059ce0 writing to bl_710000_w ff08 ffff
        059d04 writing to bl_710000_w 000a ffff
        059d04 writing to bl_710000_w 000b ffff
        059d04 writing to bl_710000_w 000c ffff
        059d04 writing to bl_710000_w 000f ffff
        059d1c writing to bl_710000_w ff09 ffff
        059d2a reading from bl_710000_r  (wants 0xe)
        */
		// before lv stage 3
		/*
        059ce0 writing to bl_710000_w 0008 ffff
        059d04 writing to bl_710000_w 000b ffff
        059d04 writing to bl_710000_w 000f ffff
        059d1c writing to bl_710000_w ff09 ffff
        059d2a reading from bl_710000_r  (wants 0x4)
        */
		// start level 3
		/*
        059ce0 writing to bl_710000_w ff08 ffff
        059d04 writing to bl_710000_w 000b ffff
        059d04 writing to bl_710000_w 000c ffff
        059d04 writing to bl_710000_w 000e ffff
        059d1c writing to bl_710000_w ff09 ffff
        059d2a reading from bl_710000_r  (wants 0x5)

        // after end sequence
        059ce0 writing to bl_710000_w 0008 ffff
        059d04 writing to bl_710000_w 000a ffff
        059d04 writing to bl_710000_w 000b ffff
        059d04 writing to bl_710000_w 000c ffff
        059d04 writing to bl_710000_w 000f ffff
        059d1c writing to bl_710000_w ff09 ffff
        059d2a reading from bl_710000_r  (wants 0xe)

        */
		m_protcount++;
	}


	DECLARE_READ16_MEMBER( bl_710000_r )
	{
		UINT16 ret;
		int pc = cpu_get_pc(&space.device());
		logerror("%06x reading from bl_710000_r\n", pc);

		if (m_protcount==6) { ret = 0xe; }
		else if (m_protcount==5) { ret = 0x5; }
		else if (m_protcount==4) { ret = 0x4; }
		else ret = 0xf;

		m_protcount = 0;
		return ret;
	}
};

class segac2_state : public md_base_state
{
public:
	segac2_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag),
	  m_paletteram(*this, "paletteram") { }

	// for Print Club only
	int m_cam_data;

	int m_segac2_enable_display;

	required_shared_ptr<UINT16> m_paletteram;

	/* internal states */
	UINT8		m_misc_io_data[0x10];	/* holds values written to the I/O chip */

	/* protection-related tracking */
	int (*m_prot_func)(int in);		/* emulation of protection chip */
	UINT8		m_prot_write_buf;		/* remembers what was written */
	UINT8		m_prot_read_buf;		/* remembers what was returned */

	/* palette-related variables */
	UINT8		m_segac2_alt_palette_mode;
	UINT8		m_palbank;
	UINT8		m_bg_palbase;
	UINT8		m_sp_palbase;

	/* sound-related variables */
	UINT8		m_sound_banks;		/* number of sound banks */
};

class mplay_state : public md_base_state
{
public:
	mplay_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag),
	m_ic3_ram(*this, "ic3_ram") { }

	UINT32 m_bios_mode;  // determines whether ROM banks or Game data
	// is to read from 0x8000-0xffff

	UINT32 m_bios_bank; // ROM bank selection
	UINT16 m_game_banksel;  // Game bank selection
	UINT32 m_readpos;  // serial bank selection position (9-bit)
	UINT32 m_mp_bios_bank_addr;

	UINT32 m_bios_width;  // determines the way the game info ROM is read
	UINT8 m_bios_ctrl[6];
	UINT8 m_bios_6600;
	UINT8 m_bios_6403;
	UINT8 m_bios_6404;

	UINT16 *m_genesis_io_ram;
	required_shared_ptr<UINT8> m_ic3_ram;
	UINT8* m_ic37_ram;
	UINT16 *m_ic36_ram;
};

class mtech_state : public md_base_state
{
public:
	mtech_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag) { }

	UINT8 m_mt_cart_select_reg;
	UINT32 m_bios_port_ctrl;
	int m_current_game_is_sms; // is the current game SMS based (running on genesis z80, in VDP compatibility mode)
	UINT32 m_bios_ctrl_inputs;
	UINT8 m_bios_ctrl[6];

	int m_mt_bank_addr;

	int m_cart_is_genesis[8];

	/* Megatech BIOS specific */
	UINT8* m_megatech_banked_ram;
};

typedef struct _megadriv_cart  megadriv_cart;
struct _megadriv_cart
{
	int type;

	// SRAM related
	UINT16 *sram;
	int last_loaded_image_length;
	int sram_start, sram_end;
	int sram_active, sram_readonly;
	int sram_handlers_installed;
	int sram_detected;

	// EEPROM related
	int has_serial_eeprom;

	// I2C related
	UINT8 i2c_mem, i2c_clk;

	// mapper related (mostly for pirate carts)
	UINT16 squirrel_king_extra;
	UINT16 lion2_prot1_data, lion2_prot2_data;
	UINT16 realtec_bank_addr, realtec_bank_size, realtec_old_bank_addr;
	UINT16 l3alt_pdat, l3alt_pcmd;
	int ssf2_lastoff, ssf2_lastdata;
};

class md_cons_state : public md_base_state
{
public:
	md_cons_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag) { }

	emu_timer *m_mess_io_timeout[3];
	int m_mess_io_stage[3];
	UINT8 m_jcart_io_data[2];

	megadriv_cart m_md_cart;
};

class pico_state : public md_cons_state
{
public:
	pico_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_cons_state(mconfig, type, tag) { }

	UINT8 m_page_register;
};

class mdsvp_state : public md_cons_state
{
public:
	mdsvp_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_cons_state(mconfig, type, tag) { }

	UINT8 *m_iram; // IRAM (0-0x7ff)
	UINT8 *m_dram; // [0x20000];
	UINT32 m_pmac_read[6];	// read modes/addrs for PM0-PM5
	UINT32 m_pmac_write[6];	// write ...
	PAIR m_pmc;
	UINT32 m_emu_status;
	UINT16 m_XST;		// external status, mapped at a15000 and a15002 on 68k side.
	UINT16 m_XST2;		// status of XST (bit1 set when 68k writes to XST)
};

ADDRESS_MAP_EXTERN( svp_ssp_map, driver_device );
ADDRESS_MAP_EXTERN( svp_ext_map, driver_device );
extern void svp_init(running_machine &machine);
extern cpu_device *_svp_cpu;



UINT8 megadrive_io_read_data_port_3button(running_machine &machine, int portnum);

class _32x_state : public md_base_state
{
public:
	_32x_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag) { }
};

// Fifa96 needs the CPUs swapped for the gameplay to enter due to some race conditions
// when using the DRC core.  Needs further investigation, the non-DRC core works either
// way
#define _32X_SWAP_MASTER_SLAVE_HACK

extern int _32x_is_connected;
extern cpu_device *_32x_master_cpu;
extern cpu_device *_32x_slave_cpu;

// called from out main scanline timers...
extern void _32x_scanline_cb0(running_machine& machine);
extern void _32x_scanline_cb1(void);
extern void _32x_check_framebuffer_swap(void);

extern int _32x_fifo_available_callback(device_t *device, UINT32 src, UINT32 dst, UINT32 data, int size);
extern MACHINE_RESET( _32x );
ADDRESS_MAP_EXTERN( sh2_main_map, driver_device );
ADDRESS_MAP_EXTERN( sh2_slave_map, driver_device );
extern emu_timer *_32x_pwm_timer;
extern TIMER_CALLBACK( _32x_pwm_callback );
UINT32* _32x_render_videobuffer_to_screenbuffer_helper(running_machine &machine, int scanline);

extern int _32x_displaymode;
extern int _32x_videopriority;
extern int _32x_hcount_compare_val;
extern int megadrive_vblank_flag;
extern UINT16 get_hposition(void);
extern int genesis_scanline_counter;

class segacd_state : public _32x_state	// use _32x_state as base to make easier the combo 32X + SCD
{
public:
	segacd_state(const machine_config &mconfig, device_type type, const char *tag)
	: _32x_state(mconfig, type, tag) { }
};

extern int sega_cd_connected;
extern int segacd_wordram_mapped;
extern cpu_device *_segacd_68k_cpu;
extern MACHINE_RESET( segacd );
ADDRESS_MAP_EXTERN( segacd_map, driver_device);
extern TIMER_DEVICE_CALLBACK( scd_dma_timer_callback );
extern timer_device* scd_dma_timer;
extern void segacd_init_main_cpu( running_machine& machine );

/*----------- defined in machine/md_cart.c -----------*/

MACHINE_CONFIG_EXTERN( genesis_cartslot );
MACHINE_CONFIG_EXTERN( _32x_cartslot );
MACHINE_CONFIG_EXTERN( pico_cartslot );
MACHINE_START( md_sram );

/*----------- defined in drivers/megadriv.c -----------*/

/* These are needed to handle J-Cart inputs */
extern WRITE16_HANDLER( jcart_ctrl_w );
extern READ16_HANDLER( jcart_ctrl_r );

/* vdp / video */
extern UINT16 (*vdp_get_word_from_68k_mem)(running_machine &machine, UINT32 source);
extern UINT16 vdp_get_word_from_68k_mem_default(running_machine &machine, UINT32 source);
extern int megadrive_visible_scanlines;
extern int megadrive_irq6_scanline;
extern int megadrive_z80irq_scanline;
extern int megadrive_imode;
extern int megadriv_framerate;
extern int megadrive_total_scanlines;
extern int megadrive_vblank_flag;
extern int genesis_scanline_counter;
extern timer_device* megadriv_render_timer;
extern TIMER_DEVICE_CALLBACK( megadriv_scanline_timer_callback );
extern TIMER_DEVICE_CALLBACK( megadriv_render_timer_callback );
extern TIMER_DEVICE_CALLBACK( irq6_on_callback );
extern int megadrive_irq6_pending;
extern int megadrive_irq4_pending;
extern bitmap_ind16* megadriv_render_bitmap;
extern timer_device* megadriv_scanline_timer;
extern timer_device* irq6_on_timer;
extern timer_device* irq4_on_timer;
extern void megadriv_reset_vdp(void);
