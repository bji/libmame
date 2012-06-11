/* Megadrive VDP */


#include "includes/megadriv.h"



int megadrive_visible_scanlines;
int megadrive_irq6_scanline;
int megadrive_z80irq_scanline;
int megadrive_imode = 0;
int megadriv_framerate;
int megadrive_total_scanlines;
int megadrive_vblank_flag = 0;
int genesis_scanline_counter = 0;
int megadrive_irq6_pending = 0;
int megadrive_irq4_pending = 0;

static int irq4counter;

static int megadrive_imode_odd_frame = 0;

int segac2_bg_pal_lookup[4];
int segac2_sp_pal_lookup[4];

// hacks for C2
int genvdp_use_cram = 0; // c2 uses it's own palette ram
int genesis_always_irq6 = 0; // c2 never enables the irq6, different source??
int genesis_other_hacks = 0; // misc hacks



static UINT8* sprite_renderline;
static UINT8* highpri_renderline;
static UINT32* video_renderline;
UINT16* megadrive_vdp_palette_lookup;
UINT16* megadrive_vdp_palette_lookup_sprite; // for C2
UINT16* megadrive_vdp_palette_lookup_shadow;
UINT16* megadrive_vdp_palette_lookup_highlight;
UINT16* megadrive_ram;
static UINT8 megadrive_vram_fill_pending = 0;
static UINT16 megadrive_vram_fill_length = 0;
static int megadrive_sprite_collision = 0;
static int megadrive_max_hposition;
int megadrive_region_export;
int megadrive_region_pal;




/*  The VDP occupies addresses C00000h to C0001Fh.

 C00000h    -   Data port (8=r/w, 16=r/w)
 C00002h    -   Data port (mirror)
 C00004h    -   Control port (8=r/w, 16=r/w)
 C00006h    -   Control port (mirror)
 C00008h    -   HV counter (8/16=r/o)
 C0000Ah    -   HV counter (mirror)
 C0000Ch    -   HV counter (mirror)
 C0000Eh    -   HV counter (mirror)
 C00011h    -   SN76489 PSG (8=w/o)
 C00013h    -   SN76489 PSG (mirror)
 C00015h    -   SN76489 PSG (mirror)
 C00017h    -   SN76489 PSG (mirror)
*/

#define MEGADRIV_VDP_VRAM(address) megadrive_vdp_vram[(address)&0x7fff]



static int megadrive_vdp_command_pending; // 2nd half of command pending..
static UINT16 megadrive_vdp_command_part1;
static UINT16 megadrive_vdp_command_part2;
static UINT8  megadrive_vdp_code;
static UINT16 megadrive_vdp_address;
static UINT16 megadrive_vdp_register[0x20];
static UINT16* megadrive_vdp_vram;
static UINT16* megadrive_vdp_cram;
static UINT16* megadrive_vdp_vsram;
/* The VDP keeps a 0x400 byte on-chip cache of the Sprite Attribute Table
   to speed up processing */
static UINT16* megadrive_vdp_internal_sprite_attribute_table;

/*

 $00 - Mode Set Register No. 1
 -----------------------------

 d7 - No effect
 d6 - No effect
 d5 - No effect
 d4 - IE1 (Horizontal interrupt enable)
 d3 - 1= Invalid display setting
 d2 - Palette select
 d1 - M3 (HV counter latch enable)
 d0 - Display disable

 */

#define MEGADRIVE_REG0_UNUSED          ((megadrive_vdp_register[0x00]&0xc0)>>6)
#define MEGADRIVE_REG0_BLANK_LEFT      ((megadrive_vdp_register[0x00]&0x20)>>5) // like SMS, not used by any commercial games?
#define MEGADRIVE_REG0_IRQ4_ENABLE     ((megadrive_vdp_register[0x00]&0x10)>>4)
#define MEGADRIVE_REG0_INVALID_MODE    ((megadrive_vdp_register[0x00]&0x08)>>3) // invalid display mode, unhandled
#define MEGADRIVE_REG0_SPECIAL_PAL     ((megadrive_vdp_register[0x00]&0x04)>>2) // strange palette mode, unhandled
#define MEGADRIVE_REG0_HVLATCH_ENABLE  ((megadrive_vdp_register[0x00]&0x02)>>1) // HV Latch, used by lightgun games
#define MEGADRIVE_REG0_DISPLAY_DISABLE ((megadrive_vdp_register[0x00]&0x01)>>0)

/*

 $01 - Mode Set Register No. 2
 -----------------------------

 d7 - TMS9918 / Genesis display select
 d6 - DISP (Display Enable)
 d5 - IE0 (Vertical Interrupt Enable)
 d4 - M1 (DMA Enable)
 d3 - M2 (PAL / NTSC)
 d2 - SMS / Genesis display select
 d1 - 0 (No effect)
 d0 - 0 (See notes)

*/

#define MEGADRIVE_REG01_TMS9918_SELECT  ((megadrive_vdp_register[0x01]&0x80)>>7)
#define MEGADRIVE_REG01_DISP_ENABLE     ((megadrive_vdp_register[0x01]&0x40)>>6)
#define MEGADRIVE_REG01_IRQ6_ENABLE     ((megadrive_vdp_register[0x01]&0x20)>>5)
#define MEGADRIVE_REG01_DMA_ENABLE      ((megadrive_vdp_register[0x01]&0x10)>>4)
#define MEGADRIVE_REG01_240_LINE        ((megadrive_vdp_register[0x01]&0x08)>>3)
#define MEGADRIVE_REG01_SMS_SELECT      ((megadrive_vdp_register[0x01]&0x04)>>2)
#define MEGADRIVE_REG01_UNUSED          ((megadrive_vdp_register[0x01]&0x02)>>1)
#define MEGADRIVE_REG01_STRANGE_VIDEO   ((megadrive_vdp_register[0x01]&0x01)>>0) // unhandled, does strange things to the display

#define MEGADRIVE_REG02_UNUSED1         ((megadrive_vdp_register[0x02]&0xc0)>>6)
#define MEGADRIVE_REG02_PATTERN_ADDR_A  ((megadrive_vdp_register[0x02]&0x38)>>3)
#define MEGADRIVE_REG02_UNUSED2         ((megadrive_vdp_register[0x02]&0x07)>>0)

#define MEGADRIVE_REG03_UNUSED1         ((megadrive_vdp_register[0x03]&0xc0)>>6)
#define MEGADRIVE_REG03_PATTERN_ADDR_W  ((megadrive_vdp_register[0x03]&0x3e)>>1)
#define MEGADRIVE_REG03_UNUSED2         ((megadrive_vdp_register[0x03]&0x01)>>0)

#define MEGADRIVE_REG04_UNUSED          ((megadrive_vdp_register[0x04]&0xf8)>>3)
#define MEGADRIVE_REG04_PATTERN_ADDR_B  ((megadrive_vdp_register[0x04]&0x07)>>0)

#define MEGADRIVE_REG05_UNUSED          ((megadrive_vdp_register[0x05]&0x80)>>7)
#define MEGADRIVE_REG05_SPRITE_ADDR     ((megadrive_vdp_register[0x05]&0x7f)>>0)

/* 6? */

#define MEGADRIVE_REG07_UNUSED          ((megadrive_vdp_register[0x07]&0xc0)>>6)
#define MEGADRIVE_REG07_BGCOLOUR        ((megadrive_vdp_register[0x07]&0x3f)>>0)

/* 8? */
/* 9? */

#define MEGADRIVE_REG0A_HINT_VALUE      ((megadrive_vdp_register[0x0a]&0xff)>>0)

#define MEGADRIVE_REG0B_UNUSED          ((megadrive_vdp_register[0x0b]&0xf0)>>4)
#define MEGADRIVE_REG0B_IRQ2_ENABLE     ((megadrive_vdp_register[0x0b]&0x08)>>3)
#define MEGADRIVE_REG0B_VSCROLL_MODE    ((megadrive_vdp_register[0x0b]&0x04)>>2)
#define MEGADRIVE_REG0B_HSCROLL_MODE    ((megadrive_vdp_register[0x0b]&0x03)>>0)

#define MEGADRIVE_REG0C_RS0             ((megadrive_vdp_register[0x0c]&0x80)>>7)
#define MEGADRIVE_REG0C_UNUSED1         ((megadrive_vdp_register[0x0c]&0x40)>>6)
#define MEGADRIVE_REG0C_SPECIAL         ((megadrive_vdp_register[0x0c]&0x20)>>5)
#define MEGADRIVE_REG0C_UNUSED2         ((megadrive_vdp_register[0x0c]&0x10)>>4)
#define MEGADRIVE_REG0C_SHADOW_HIGLIGHT ((megadrive_vdp_register[0x0c]&0x08)>>3)
#define MEGADRIVE_REG0C_INTERLEAVE      ((megadrive_vdp_register[0x0c]&0x06)>>1)
#define MEGADRIVE_REG0C_RS1             ((megadrive_vdp_register[0x0c]&0x01)>>0)

#define MEGADRIVE_REG0D_UNUSED          ((megadrive_vdp_register[0x0d]&0xc0)>>6)
#define MEGADRIVE_REG0D_HSCROLL_ADDR    ((megadrive_vdp_register[0x0d]&0x3f)>>0)

/* e? */

#define MEGADRIVE_REG0F_AUTO_INC        ((megadrive_vdp_register[0x0f]&0xff)>>0)

#define MEGADRIVE_REG10_UNUSED1        ((megadrive_vdp_register[0x10]&0xc0)>>6)
#define MEGADRIVE_REG10_VSCROLL_SIZE   ((megadrive_vdp_register[0x10]&0x30)>>4)
#define MEGADRIVE_REG10_UNUSED2        ((megadrive_vdp_register[0x10]&0x0c)>>2)
#define MEGADRIVE_REG10_HSCROLL_SIZE   ((megadrive_vdp_register[0x10]&0x03)>>0)

#define MEGADRIVE_REG11_WINDOW_RIGHT   ((megadrive_vdp_register[0x11]&0x80)>>7)
#define MEGADRIVE_REG11_UNUSED         ((megadrive_vdp_register[0x11]&0x60)>>5)
#define MEGADRIVE_REG11_WINDOW_HPOS      ((megadrive_vdp_register[0x11]&0x1f)>>0)

#define MEGADRIVE_REG12_WINDOW_DOWN    ((megadrive_vdp_register[0x12]&0x80)>>7)
#define MEGADRIVE_REG12_UNUSED         ((megadrive_vdp_register[0x12]&0x60)>>5)
#define MEGADRIVE_REG12_WINDOW_VPOS      ((megadrive_vdp_register[0x12]&0x1f)>>0)

#define MEGADRIVE_REG13_DMALENGTH1     ((megadrive_vdp_register[0x13]&0xff)>>0)

#define MEGADRIVE_REG14_DMALENGTH2      ((megadrive_vdp_register[0x14]&0xff)>>0)

#define MEGADRIVE_REG15_DMASOURCE1      ((megadrive_vdp_register[0x15]&0xff)>>0)
#define MEGADRIVE_REG16_DMASOURCE2      ((megadrive_vdp_register[0x16]&0xff)>>0)

#define MEGADRIVE_REG17_DMASOURCE3      ((megadrive_vdp_register[0x17]&0xff)>>0)
#define MEGADRIVE_REG17_DMATYPE         ((megadrive_vdp_register[0x17]&0xc0)>>6)
#define MEGADRIVE_REG17_UNUSED          ((megadrive_vdp_register[0x17]&0x3f)>>0)


static void vdp_vram_write(UINT16 data)
{

	UINT16 sprite_base_address = MEGADRIVE_REG0C_RS1?((MEGADRIVE_REG05_SPRITE_ADDR&0x7e)<<9):((MEGADRIVE_REG05_SPRITE_ADDR&0x7f)<<9);
	int spritetable_size = MEGADRIVE_REG0C_RS1?0x400:0x200;
	int lowlimit = sprite_base_address;
	int highlimit = sprite_base_address+spritetable_size;

	if (megadrive_vdp_address&1)
	{
		data = ((data&0x00ff)<<8)|((data&0xff00)>>8);
	}

	MEGADRIV_VDP_VRAM(megadrive_vdp_address>>1) = data;

	/* The VDP stores an Internal copy of any data written to the Sprite Attribute Table.
       This data is _NOT_ invalidated when the Sprite Base Address changes, thus allowing
       for some funky effects, as used by Castlevania Bloodlines Stage 6-3 */
	if (megadrive_vdp_address>=lowlimit && megadrive_vdp_address<highlimit)
	{
//      mame_printf_debug("spritebase is %04x-%04x vram address is %04x, write %04x\n",lowlimit, highlimit-1, megadrive_vdp_address, data);
		megadrive_vdp_internal_sprite_attribute_table[(megadrive_vdp_address&(spritetable_size-1))>>1] = data;
	}

	megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
	megadrive_vdp_address &= 0xffff;
}

static void vdp_vsram_write(UINT16 data)
{
	megadrive_vdp_vsram[(megadrive_vdp_address&0x7e)>>1] = data;

	//logerror("Wrote to VSRAM addr %04x data %04x\n",megadrive_vdp_address&0xfffe,megadrive_vdp_vsram[megadrive_vdp_address>>1]);

	megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;

	megadrive_vdp_address &=0xffff;
}

static void write_cram_value(running_machine &machine, int offset, int data)
{
	megadrive_vdp_cram[offset] = data;

	//logerror("Wrote to CRAM addr %04x data %04x\n",megadrive_vdp_address&0xfffe,megadrive_vdp_cram[megadrive_vdp_address>>1]);
	if (genvdp_use_cram)
	{
		int r,g,b;
		r = ((data >> 1)&0x07);
		g = ((data >> 5)&0x07);
		b = ((data >> 9)&0x07);
		palette_set_color_rgb(machine,offset,pal3bit(r),pal3bit(g),pal3bit(b));
		megadrive_vdp_palette_lookup[offset] = (b<<2) | (g<<7) | (r<<12);
		megadrive_vdp_palette_lookup_sprite[offset] = (b<<2) | (g<<7) | (r<<12);
		megadrive_vdp_palette_lookup_shadow[offset] = (b<<1) | (g<<6) | (r<<11);
		megadrive_vdp_palette_lookup_highlight[offset] = ((b|0x08)<<1) | ((g|0x08)<<6) | ((r|0x08)<<11);
	}
}

static void vdp_cram_write(running_machine &machine, UINT16 data)
{
	int offset;
	offset = (megadrive_vdp_address&0x7e)>>1;

	write_cram_value(machine, offset,data);

	megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;

	megadrive_vdp_address &=0xffff;
}


static void megadriv_vdp_data_port_w(running_machine &machine, int data)
{
	megadrive_vdp_command_pending = 0;

 /*
 0000b : VRAM read
 0001b : VRAM write
 0011b : CRAM write
 0100b : VSRAM read
 0101b : VSRAM write
 1000b : CRAM read
 */
//  logerror("write to vdp data port %04x with code %04x, write address %04x\n",data, megadrive_vdp_code, megadrive_vdp_address );

	if (megadrive_vram_fill_pending)
	{
		int count;

		megadrive_vdp_address&=0xffff;

		if (megadrive_vdp_address&1)
		{
			MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))&0xff00) | (data&0x00ff);
		}
		else
		{
			MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))&0x00ff) | ((data&0x00ff)<<8);
		}


		for (count=0;count<=megadrive_vram_fill_length;count++) // <= for james pond 3
		{
			if (megadrive_vdp_address&1)
			{
				MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))&0x00ff) | (data&0xff00);
			}
			else
			{
				MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))   = (MEGADRIV_VDP_VRAM((megadrive_vdp_address>>1))&0xff00) | ((data&0xff00)>>8);
			}

			megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			megadrive_vdp_address&=0xffff;

		}

		megadrive_vdp_register[0x13] = 0;
		megadrive_vdp_register[0x14] = 0;

	//  megadrive_vdp_register[0x15] = (source>>1) & 0xff;
	//  megadrive_vdp_register[0x16] = (source>>9) & 0xff;
	//  megadrive_vdp_register[0x17] = (source>>17) & 0xff;


	}
	else
	{

		switch (megadrive_vdp_code & 0x000f)
		{
			case 0x0000:
				logerror("Attempting to WRITE to DATA PORT in VRAM READ MODE\n");
				break;

			case 0x0001:
				vdp_vram_write(data);
				break;

			case 0x0003:
				vdp_cram_write(machine, data);
				break;

			case 0x0004:
				logerror("Attempting to WRITE to DATA PORT in VSRAM READ MODE\n");
				break;

			case 0x0005:
				vdp_vsram_write(data);
				break;

			case 0x0008:
				logerror("Attempting to WRITE to DATA PORT in CRAM READ MODE\n");
				break;

			default:
				logerror("Attempting to WRITE to DATA PORT in #UNDEFINED# MODE %1x %04x\n",megadrive_vdp_code&0xf, data);
				break;
		}
	}



}



static void megadrive_vdp_set_register(running_machine &machine, int regnum, UINT8 value)
{
	megadrive_vdp_register[regnum] = value;

	/* We need special handling for the IRQ enable registers, some games turn
       off the irqs before they are taken, delaying them until the IRQ is turned
       back on */

	if (regnum == 0x00)
	{
	//mame_printf_debug("setting reg 0, irq enable is now %d\n",MEGADRIVE_REG0_IRQ4_ENABLE);

		if (megadrive_irq4_pending)
		{
			if (MEGADRIVE_REG0_IRQ4_ENABLE)
				cputag_set_input_line(machine, "maincpu", 4, HOLD_LINE);
			else
				cputag_set_input_line(machine, "maincpu", 4, CLEAR_LINE);
		}

		/* ??? Fatal Rewind needs this but I'm not sure it's accurate behavior
           it causes flickering in roadrash */
	//  megadrive_irq6_pending = 0;
	//  megadrive_irq4_pending = 0;

	}

	if (regnum == 0x01)
	{
		if (megadrive_irq6_pending)
		{
			if (MEGADRIVE_REG01_IRQ6_ENABLE )
				cputag_set_input_line(machine, "maincpu", 6, HOLD_LINE);
			else
				cputag_set_input_line(machine, "maincpu", 6, CLEAR_LINE);
		}

		/* ??? */
	//  megadrive_irq6_pending = 0;
	//  megadrive_irq4_pending = 0;

	}


//  if (regnum == 0x0a)
//      mame_printf_debug("Set HINT Reload Register to %d on scanline %d\n",value, genesis_scanline_counter);

//  mame_printf_debug("%s: Setting VDP Register #%02x to %02x\n",machine.describe_context(), regnum,value);
}

static void update_megadrive_vdp_code_and_address(void)
{
	megadrive_vdp_code = ((megadrive_vdp_command_part1 & 0xc000) >> 14) |
	                     ((megadrive_vdp_command_part2 & 0x00f0) >> 2);

	megadrive_vdp_address = ((megadrive_vdp_command_part1 & 0x3fff) >> 0) |
                            ((megadrive_vdp_command_part2 & 0x0003) << 14);
}

UINT16 (*vdp_get_word_from_68k_mem)(running_machine &machine, UINT32 source);

UINT16 vdp_get_word_from_68k_mem_default(running_machine &machine, UINT32 source)
{
	// should we limit the valid areas here?
	// how does this behave with the segacd etc?
	// note, the RV bit on 32x is important for this to work, because it causes a normal cart mapping - see tempo
	address_space *space68k = machine.device<legacy_cpu_device>("maincpu")->space();

	//printf("vdp_get_word_from_68k_mem_default %08x\n", source);

	if ( source <= 0x3fffff )
	{
		if (_svp_cpu != NULL)
		{
			source -= 2; // the SVP introduces some kind of DMA 'lag', which we have to compensate for, this is obvious even on gfx DMAd from ROM (the Speedometer)
		}

		// likewise segaCD, at least when reading wordram?
		// we might need to check what mode we're in here..
		if (segacd_wordram_mapped)
		{
			source -= 2;
		}

		return space68k->read_word(source);
	}
	else if (( source >= 0xe00000 ) && ( source <= 0xffffff ))
	{
		return space68k->read_word(source);
	}
	else
	{
		printf("DMA Read unmapped %06x\n",source);
		return machine.rand();
	}


}

/*  Table from Charles Macdonald


    DMA Mode      Width       Display      Transfer Count
    -----------------------------------------------------
    68K > VDP     32-cell     Active       16
                              Blanking     167
                  40-cell     Active       18
                              Blanking     205
    VRAM Fill     32-cell     Active       15
                              Blanking     166
                  40-cell     Active       17
                              Blanking     204
    VRAM Copy     32-cell     Active       8
                              Blanking     83
                  40-cell     Active       9
                              Blanking     102

*/


/* Note, In reality this transfer is NOT instant, the 68k isn't paused
   as the 68k address bus isn't accessed */

/* Wani Wani World, James Pond 3, Pirates Gold! */
static void megadrive_do_insta_vram_copy(UINT32 source, UINT16 length)
{
	int x;

	for (x=0;x<length;x++)
	{
		UINT8 source_byte;

		//mame_printf_debug("vram copy length %04x source %04x dest %04x\n",length, source, megadrive_vdp_address );
		if (source&1) source_byte = MEGADRIV_VDP_VRAM((source&0xffff)>>1)&0x00ff;
		else  source_byte = (MEGADRIV_VDP_VRAM((source&0xffff)>>1)&0xff00)>>8;

		if (megadrive_vdp_address&1)
		{
			MEGADRIV_VDP_VRAM((megadrive_vdp_address&0xffff)>>1) = (MEGADRIV_VDP_VRAM((megadrive_vdp_address&0xffff)>>1)&0xff00) | source_byte;
		}
		else
		{
			MEGADRIV_VDP_VRAM((megadrive_vdp_address&0xffff)>>1) = (MEGADRIV_VDP_VRAM((megadrive_vdp_address&0xffff)>>1)&0x00ff) | (source_byte<<8);
		}

		source++;
		megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
		megadrive_vdp_address&=0xffff;
	}
}

/* Instant, but we pause the 68k a bit */
static void megadrive_do_insta_68k_to_vram_dma(running_machine &machine, UINT32 source,int length)
{
	int count;

	if (length==0x00) length = 0xffff;

	/* This is a hack until real DMA timings are implemented */
	device_spin_until_time(machine.device("maincpu"), attotime::from_nsec(length * 1000 / 3500));

	for (count = 0;count<(length>>1);count++)
	{
		vdp_vram_write(vdp_get_word_from_68k_mem(machine, source));
		source+=2;
		if (source>0xffffff) source = 0xe00000;
	}

	megadrive_vdp_address&=0xffff;

	megadrive_vdp_register[0x13] = 0;
	megadrive_vdp_register[0x14] = 0;

	megadrive_vdp_register[0x15] = (source>>1) & 0xff;
	megadrive_vdp_register[0x16] = (source>>9) & 0xff;
	megadrive_vdp_register[0x17] = (source>>17) & 0xff;
}


static void megadrive_do_insta_68k_to_cram_dma(running_machine &machine,UINT32 source,UINT16 length)
{
	int count;

	if (length==0x00) length = 0xffff;

	for (count = 0;count<(length>>1);count++)
	{
		//if (megadrive_vdp_address>=0x80) return; // abandon

		write_cram_value(machine, (megadrive_vdp_address&0x7e)>>1, vdp_get_word_from_68k_mem(machine, source));
		source+=2;

		if (source>0xffffff) source = 0xfe0000;

		megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
		megadrive_vdp_address&=0xffff;
	}

	megadrive_vdp_register[0x13] = 0;
	megadrive_vdp_register[0x14] = 0;

	megadrive_vdp_register[0x15] = (source>>1) & 0xff;
	megadrive_vdp_register[0x16] = (source>>9) & 0xff;
	megadrive_vdp_register[0x17] = (source>>17) & 0xff;

}

static void megadrive_do_insta_68k_to_vsram_dma(running_machine &machine,UINT32 source,UINT16 length)
{
	int count;

	if (length==0x00) length = 0xffff;

	for (count = 0;count<(length>>1);count++)
	{
		if (megadrive_vdp_address>=0x80) return; // abandon

		megadrive_vdp_vsram[(megadrive_vdp_address&0x7e)>>1] = vdp_get_word_from_68k_mem(machine, source);
		source+=2;

		if (source>0xffffff) source = 0xfe0000;

		megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
		megadrive_vdp_address&=0xffff;
	}

	megadrive_vdp_register[0x13] = 0;
	megadrive_vdp_register[0x14] = 0;

	megadrive_vdp_register[0x15] = (source>>1) & 0xff;
	megadrive_vdp_register[0x16] = (source>>9) & 0xff;
	megadrive_vdp_register[0x17] = (source>>17) & 0xff;
}

/* This can be simplified quite a lot.. */
static void handle_dma_bits(running_machine &machine)
{
#if 0
	if (megadrive_vdp_code&0x20)
	{
		UINT32 source;
		UINT16 length;
		source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0xff)<<16))<<1;
		length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;
		mame_printf_debug("%s 68k DMAtran set source %06x length %04x dest %04x enabled %01x code %02x %02x\n", machine.describe_context(), source, length, megadrive_vdp_address,MEGADRIVE_REG01_DMA_ENABLE, megadrive_vdp_code,MEGADRIVE_REG0F_AUTO_INC);
	}
#endif
	if (megadrive_vdp_code==0x20)
	{
		mame_printf_debug("DMA bit set 0x20 but invalid??\n");
	}
	else if (megadrive_vdp_code==0x21 || megadrive_vdp_code==0x31) /* 0x31 used by tecmo cup */
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0 || MEGADRIVE_REG17_DMATYPE==0x1)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0x7f)<<16))<<1;
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;

			/* The 68k is frozen during this transfer, it should be safe to throw a few cycles away and do 'instant' DMA because the 68k can't detect it being in progress (can the z80?) */
			//mame_printf_debug("68k->VRAM DMA transfer source %06x length %04x dest %04x enabled %01x\n", source, length, megadrive_vdp_address,MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_68k_to_vram_dma(machine,source,length);

		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			//mame_printf_debug("vram fill length %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE)
			{
				megadrive_vram_fill_pending = 1;
				megadrive_vram_fill_length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8));
			}
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8)); // source (byte offset)
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8)); // length in bytes
			//mame_printf_debug("setting vram copy mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);

			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_vram_copy(source, length);
		}
	}
	else if (megadrive_vdp_code==0x23)
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0 || MEGADRIVE_REG17_DMATYPE==0x1)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0x7f)<<16))<<1;
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;

			/* The 68k is frozen during this transfer, it should be safe to throw a few cycles away and do 'instant' DMA because the 68k can't detect it being in progress (can the z80?) */
			//mame_printf_debug("68k->CRAM DMA transfer source %06x length %04x dest %04x enabled %01x\n", source, length, megadrive_vdp_address,MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_68k_to_cram_dma(machine,source,length);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			//mame_printf_debug("vram fill length %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE)
			{
				megadrive_vram_fill_pending = 1;
				megadrive_vram_fill_length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8));
			}
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			mame_printf_debug("setting vram copy (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
	}
	else if (megadrive_vdp_code==0x25)
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0 || MEGADRIVE_REG17_DMATYPE==0x1)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8) | ((MEGADRIVE_REG17_DMASOURCE3&0x7f)<<16))<<1;
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8))<<1;

			/* The 68k is frozen during this transfer, it should be safe to throw a few cycles away and do 'instant' DMA because the 68k can't detect it being in progress (can the z80?) */
			//mame_printf_debug("68k->VSRAM DMA transfer source %06x length %04x dest %04x enabled %01x\n", source, length, megadrive_vdp_address,MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_68k_to_vsram_dma(machine,source,length);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			//mame_printf_debug("vram fill length %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
			if (MEGADRIVE_REG01_DMA_ENABLE)
			{
				megadrive_vram_fill_pending = 1;
				megadrive_vram_fill_length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8));
			}
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			mame_printf_debug("setting vram copy (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
	}
	else if (megadrive_vdp_code==0x30)
	{
		if (MEGADRIVE_REG17_DMATYPE==0x0)
		{
			mame_printf_debug("setting vram 68k->vram (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x1)
		{
			mame_printf_debug("setting vram 68k->vram (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x2)
		{
			mame_printf_debug("setting vram fill (INVALID?) mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);
		}
		else if (MEGADRIVE_REG17_DMATYPE==0x3)
		{
			UINT32 source;
			UINT16 length;
			source = (MEGADRIVE_REG15_DMASOURCE1 | (MEGADRIVE_REG16_DMASOURCE2<<8)); // source (byte offset)
			length = (MEGADRIVE_REG13_DMALENGTH1 | (MEGADRIVE_REG14_DMALENGTH2<<8)); // length in bytes
			//mame_printf_debug("setting vram copy mode length registers are %02x %02x other regs! %02x %02x %02x(Mode Bits %02x) Enable %02x\n", MEGADRIVE_REG13_DMALENGTH1, MEGADRIVE_REG14_DMALENGTH2, MEGADRIVE_REG15_DMASOURCE1, MEGADRIVE_REG16_DMASOURCE2, MEGADRIVE_REG17_DMASOURCE3, MEGADRIVE_REG17_DMATYPE, MEGADRIVE_REG01_DMA_ENABLE);

			if (MEGADRIVE_REG01_DMA_ENABLE) megadrive_do_insta_vram_copy(source, length);
		}
	}
}

static void megadriv_vdp_ctrl_port_w(running_machine &machine, int data)
{
//  logerror("write to vdp control port %04x\n",data);
	megadrive_vram_fill_pending = 0; // ??

	if (megadrive_vdp_command_pending)
	{
		/* 2nd part of 32-bit command */
		megadrive_vdp_command_pending = 0;
		megadrive_vdp_command_part2 = data;

		update_megadrive_vdp_code_and_address();
		handle_dma_bits(machine);

		//logerror("VDP Write Part 2 setting Code %02x Address %04x\n",megadrive_vdp_code, megadrive_vdp_address);

	}
	else
	{
		if ((data & 0xc000) == 0x8000)
		{	/* Register Setting Command */
			int regnum = (data & 0x3f00) >> 8;
			int value  = (data & 0x00ff);

			if (regnum &0x20) mame_printf_debug("reg error\n");

			megadrive_vdp_set_register(machine, regnum&0x1f,value);
			megadrive_vdp_code = 0;
			megadrive_vdp_address = 0;
		}
		else
		{
			megadrive_vdp_command_pending = 1;
			megadrive_vdp_command_part1 = data;
			update_megadrive_vdp_code_and_address();
			//logerror("VDP Write Part 1 setting Code %02x Address %04x\n",megadrive_vdp_code, megadrive_vdp_address);
		}

	}
}

WRITE16_HANDLER( megadriv_vdp_w )
{
	switch (offset<<1)
	{
		case 0x00:
		case 0x02:
			if (!ACCESSING_BITS_8_15)
			{
				data = (data&0x00ff) | data<<8;
			//  mame_printf_debug("8-bit write VDP data port access, offset %04x data %04x mem_mask %04x\n",offset,data,mem_mask);
			}
			else if (!ACCESSING_BITS_0_7)
			{
				data = (data&0xff00) | data>>8;
			//  mame_printf_debug("8-bit write VDP data port access, offset %04x data %04x mem_mask %04x\n",offset,data,mem_mask);
			}
			megadriv_vdp_data_port_w(space->machine(), data);
			break;

		case 0x04:
		case 0x06:
			if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit write VDP control port access, offset %04x data %04x mem_mask %04x\n",offset,data,mem_mask);
			megadriv_vdp_ctrl_port_w(space->machine(), data);
			break;

		case 0x08:
		case 0x0a:
		case 0x0c:
		case 0x0e:
			logerror("Attempt to Write to HV counters!!\n");
			break;

		case 0x10:
		case 0x12:
		case 0x14:
		case 0x16:
			if (ACCESSING_BITS_0_7) sn76496_w(space->machine().device("snsnd"), 0, data & 0xff);
			//if (ACCESSING_BITS_8_15) sn76496_w(space->machine().device("snsnd"), 0, (data >>8) & 0xff);
			break;

		default:
		mame_printf_debug("write to unmapped vdp port\n");
	}
}

static UINT16 vdp_vram_r(void)
{
	return MEGADRIV_VDP_VRAM((megadrive_vdp_address&0xfffe)>>1);
}

static UINT16 vdp_vsram_r(void)
{
	return megadrive_vdp_vsram[(megadrive_vdp_address&0x7e)>>1];
}

static UINT16 vdp_cram_r(void)
{

	return megadrive_vdp_cram[(megadrive_vdp_address&0x7e)>>1];
}

static UINT16 megadriv_vdp_data_port_r(running_machine &machine)
{
	UINT16 retdata=0;

	//return machine.rand();

	megadrive_vdp_command_pending = 0;

	switch (megadrive_vdp_code & 0x000f)
	{
		case 0x0000:
			retdata = vdp_vram_r();
			megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			megadrive_vdp_address&=0xffff;
			break;

		case 0x0001:
			logerror("Attempting to READ from DATA PORT in VRAM WRITE MODE\n");
			retdata = machine.rand();
			break;

		case 0x0003:
			logerror("Attempting to READ from DATA PORT in CRAM WRITE MODE\n");
			retdata = machine.rand();
			break;

		case 0x0004:
			retdata = vdp_vsram_r();
			megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			megadrive_vdp_address&=0xffff;
			break;

		case 0x0005:
			logerror("Attempting to READ from DATA PORT in VSRAM WRITE MODE\n");
			break;

		case 0x0008:
			retdata = vdp_cram_r();
			megadrive_vdp_address+=MEGADRIVE_REG0F_AUTO_INC;
			megadrive_vdp_address&=0xffff;
			break;

		default:
			logerror("Attempting to READ from DATA PORT in #UNDEFINED# MODE\n");
			retdata = machine.rand();
			break;
	}

//  mame_printf_debug("vdp_data_port_r %04x %04x %04x\n",megadrive_vdp_code, megadrive_vdp_address, retdata);

//  logerror("Read VDP Data Port\n");
	return retdata;
}

/*

 NTSC, 256x224
 -------------

 Lines  Description

 224    Active display
 8      Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 11     Top border

 V counter values
 00-EA, E5-FF

PAL, 256x224
 ------------

 Lines  Description

 224    Active display
 32     Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 38     Top border

 V counter values
 00-FF, 00-02, CA-FF

 PAL, 256x240
 ------------

 Lines  Description

 240    Active display
 24     Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 30     Top border

 V counter values
 00-FF, 00-0A, D2-FF



 Pixels H.Cnt   Description
  256 : 00-7F : Active display
   15 : 80-87 : Right border
    8 : 87-8B : Right blanking
   26 : 8B-ED : Horizontal sync
    2 : ED-EE : Left blanking
   14 : EE-F5 : Color burst
    8 : F5-F9 : Left blanking
   13 : F9-FF : Left border

*/



static UINT16 megadriv_vdp_ctrl_port_r(void)
{
	/* Battletoads is very fussy about the vblank flag
       it wants it to be 1. in scanline 224 */

	/* Double Dragon 2 is very sensitive to hblank timing */
	/* xperts is very fussy too */

	/* Game no Kanzume Otokuyou (J) [!] is also fussy
      - it cares about the bits labeled always 0, always 1.. (!)
     */

	/* Megalo Mania also fussy - cares about pending flag*/

	int megadrive_sprite_overflow = 0;
	int megadrive_odd_frame = megadrive_imode_odd_frame^1;
	int megadrive_hblank_flag = 0;
	int megadrive_dma_active = 0;
	int vblank;
	int fifo_empty = 1;
	int fifo_full = 0;

	UINT16 hpos = get_hposition();

	if (hpos>400) megadrive_hblank_flag = 1;
	if (hpos>460) megadrive_hblank_flag = 0;

	vblank = megadrive_vblank_flag;

	/* extra case */
	if (MEGADRIVE_REG01_DISP_ENABLE==0) vblank = 1;

/*

// these aren't *always* 0/1 some of them are open bus return
 d15 - Always 0
 d14 - Always 0
 d13 - Always 1
 d12 - Always 1

 d11 - Always 0
 d10 - Always 1
 d9  - FIFO Empty
 d8  - FIFO Full

 d7  - Vertical interrupt pending
 d6  - Sprite overflow on current scan line
 d5  - Sprite collision
 d4  - Odd frame

 d3  - Vertical blanking
 d2  - Horizontal blanking
 d1  - DMA in progress
 d0  - PAL mode flag
*/

	return (0<<15) | // ALWAYS 0
	       (0<<14) | // ALWAYS 0
	       (1<<13) | // ALWAYS 1
	       (1<<12) | // ALWAYS 1
	       (0<<11) | // ALWAYS 0
	       (1<<10) | // ALWAYS 1
	       (fifo_empty<<9 ) | // FIFO EMPTY
	       (fifo_full<<8 ) | // FIFO FULL
	       (megadrive_irq6_pending << 7) | // exmutants has a tight loop checking this ..
	       (megadrive_sprite_overflow << 6) |
	       (megadrive_sprite_collision << 5) |
	       (megadrive_odd_frame << 4) |
	       (vblank << 3) |
	       (megadrive_hblank_flag << 2) |
	       (megadrive_dma_active << 1 ) |
	       (megadrive_region_pal<<0); // PAL MODE FLAG checked by striker for region prot..
}

static const UINT8 vc_ntsc_224[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,/**/0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,    0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const UINT8 vc_ntsc_240[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};

static const UINT8 vc_pal_224[] =
{
    0x00, 0x01, 0x02,    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12,    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22,    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32,    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42,    0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52,    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62,    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72,    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82,    0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92,    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2,    0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2,    0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2,    0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2,    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2,    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2,    0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02,/**/0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9,    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9,    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const UINT8 vc_pal_240[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,    0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa,    0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,/**/0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1,    0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1,    0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};



static UINT16 megadriv_read_hv_counters(void)
{
	/* Bubble and Squeek wants vcount=0xe0 */
	/* Dracula is very sensitive to this */
	/* Marvel Land is sensitive to this */

	int vpos = genesis_scanline_counter;
	UINT16 hpos = get_hposition();

//  if (hpos>424) vpos++; // fixes dracula, breaks road rash
	if (hpos>460) vpos++; // when does vpos increase.. also on sms, check game gear manual..

	/* shouldn't happen.. */
	if (vpos<0)
	{
		vpos = megadrive_total_scanlines;
		mame_printf_debug("negative vpos?!\n");
	}

	if (MEGADRIVE_REG01_240_LINE)
	{
		if (!megadrive_region_pal)
		{
			vpos = vc_ntsc_240[vpos%megadrive_total_scanlines];
		}
		else
		{
			vpos = vc_pal_240[vpos%megadrive_total_scanlines];
		}

	}
	else
	{
		if (!megadrive_region_pal)
		{
			vpos = vc_ntsc_224[vpos%megadrive_total_scanlines];
		}
		else
		{
			vpos = vc_pal_224[vpos%megadrive_total_scanlines];
		}
	}

	if (hpos>0xf7) hpos -=0x49;

	return ((vpos&0xff)<<8)|(hpos&0xff);

}

READ16_HANDLER( megadriv_vdp_r )
{
	UINT16 retvalue = 0;



	switch (offset<<1)
	{

		case 0x00:
		case 0x02:
			if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit VDP read data port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_vdp_data_port_r(space->machine());
			break;

		case 0x04:
		case 0x06:
		//  if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit VDP read control port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_vdp_ctrl_port_r();
		//  retvalue = space->machine().rand();
		//  mame_printf_debug("%06x: Read Control Port at scanline %d hpos %d (return %04x)\n",cpu_get_pc(&space->device()),genesis_scanline_counter, get_hposition(),retvalue);
			break;

		case 0x08:
		case 0x0a:
		case 0x0c:
		case 0x0e:
		//  if ((!ACCESSING_BITS_8_15) || (!ACCESSING_BITS_0_7)) mame_printf_debug("8-bit VDP read HV counter port access, offset %04x mem_mask %04x\n",offset,mem_mask);
			retvalue = megadriv_read_hv_counters();
		//  retvalue = space->machine().rand();
		//  mame_printf_debug("%06x: Read HV counters at scanline %d hpos %d (return %04x)\n",cpu_get_pc(&space->device()),genesis_scanline_counter, get_hposition(),retvalue);
			break;

		case 0x10:
		case 0x12:
		case 0x14:
		case 0x16:
			logerror("Attempting to read PSG!\n");
			retvalue = 0;
			break;
	}
	return retvalue;
}

READ8_DEVICE_HANDLER( megadriv_68k_YM2612_read)
{
	//mame_printf_debug("megadriv_68k_YM2612_read %02x %04x\n",offset,mem_mask);
	if ( (genz80.z80_has_bus==0) && (genz80.z80_is_reset==0) )
	{
		return ym2612_r(device, offset);
	}
	else
	{
		logerror("%s: 68000 attempting to access YM2612 (read) without bus\n", device->machine().describe_context());
		return 0;
	}

	return -1;
}


// line length = 342

/*
 The V counter counts up from 00h to EAh, then it jumps back to E5h and
 continues counting up to FFh. This allows it to cover the entire 262 line
 display.

 The H counter counts up from 00h to E9h, then it jumps back to 93h and
 continues counting up to FFh. This allows it to cover an entire 342 pixel
 line.
*/

/*

 - The 80th sprite has been drawn in 40-cell mode.
 - The 64th sprite has been drawn in 32-cell mode.
 - Twenty sprites on the same scanline have been drawn in 40 cell mode.
 - Sixteen sprites on the same scanline have been drawn in 32 cell mode.
 - 320 pixels worth of sprite data has been drawn on the same scanline
   in 40 cell mode.
 - 256 pixels worth of sprite data has been drawn on the same scanline
   in 32 cell mode.
 - The currently drawn sprite has a link field of zero.

*/

/*

 $05 - Sprite Attribute Table Base Address
 -----------------------------------------

 Bits 6-0 of this register correspond to bits A15-A09 of the sprite
 attribute table.

 In 40-cell mode, A09 is always forced to zero.

*/

static void genesis_render_spriteline_to_spritebuffer(int scanline)
{
	int screenwidth;
	int maxsprites=0;
	int maxpixels=0;
	UINT16 base_address=0;



	screenwidth = MEGADRIVE_REG0C_RS0 | (MEGADRIVE_REG0C_RS1 << 1);

	switch (screenwidth&3)
	{
		case 0: maxsprites = 64; maxpixels = 256; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7f)<<9; break;
		case 1: maxsprites = 64; maxpixels = 256; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7f)<<9; break;
		case 2: maxsprites = 80; maxpixels = 320; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7e)<<9; break;
		case 3: maxsprites = 80; maxpixels = 320; base_address = (MEGADRIVE_REG05_SPRITE_ADDR&0x7e)<<9; break;
	}


	/* Clear our Render Buffer */
	memset(sprite_renderline, 0, 1024);


	{
		int spritenum;
		int ypos,xpos,addr;
		int drawypos;
		int /*drawwidth,*/ drawheight;
		int spritemask = 0;
		UINT8 height,width=0,link=0,xflip,yflip,colour,pri;

		/* Get Sprite Attribs */
		spritenum = 0;

		//if (scanline==40) mame_printf_debug("spritelist start base %04x\n",base_address);

		do
		{
			//UINT16 value1,value2,value3,value4;

			//value1 = megadrive_vdp_vram[((base_address>>1)+spritenum*4)+0x0];
			//value2 = megadrive_vdp_vram[((base_address>>1)+spritenum*4)+0x1];
			//value3 = megadrive_vdp_vram[((base_address>>1)+spritenum*4)+0x2];
			//value4 = megadrive_vdp_vram[((base_address>>1)+spritenum*4)+0x3];

			ypos  = (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x0] & 0x01ff)>>0; /* 0x03ff? */ // puyo puyo requires 0x1ff mask, not 0x3ff, see speech bubble corners
			height= (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x1] & 0x0300)>>8;
			width = (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x1] & 0x0c00)>>10;
			link  = (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x1] & 0x007f)>>0;
			xpos  = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x3) & 0x01ff)>>0; /* 0x03ff? */ // pirates gold has a sprite with co-ord 0x200...

			if(megadrive_imode==3)
			{
				ypos  = (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x0] & 0x03ff)>>0; /* 0x3ff requried in interlace mode (sonic 2 2 player) */
				drawypos = ypos - 256;
				drawheight = (height+1)*16;
			}
			else
			{
				ypos  = (megadrive_vdp_internal_sprite_attribute_table[(spritenum*4)+0x0] & 0x01ff)>>0; /* 0x03ff? */ // puyo puyo requires 0x1ff mask, not 0x3ff, see speech bubble corners
				drawypos = ypos - 128;
				drawheight = (height+1)*8;
			}



			//if (scanline==40) mame_printf_debug("xpos %04x ypos %04x\n",xpos,ypos);

			if ((drawypos<=scanline) && ((drawypos+drawheight)>scanline))
			{

				addr  = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x07ff)>>0;
				xflip = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x0800)>>11;
				yflip = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x1000)>>12;
				colour= (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x6000)>>13;
				pri   = (MEGADRIV_VDP_VRAM(((base_address>>1)+spritenum*4)+0x2) & 0x8000)>>15;

				if(megadrive_imode==3)
				{
					addr<<=1;
					addr &=0x7ff;
				}

				//drawwidth = (width+1)*8;
				if (pri==1) pri = 0x80;
				else pri = 0x40;

				/* todo: fix me, I'm sure this isn't right but sprite 0 + other sprite seem to do something..
                   maybe spritemask|=2 should be set for anything < 0x40 ?*/
				if (xpos==0x00) spritemask|=1;

				//if (xpos==0x01) spritemask|=2;
				//if (xpos==0x04) spritemask|=2;  // sonic 2 title screen
				//if (xpos==0x08) spritemask|=2;  // rocket night adventures
				//if (xpos==0x10) spritemask|=2;  // mercs l1 boss
				//if (xpos==0x0a) spritemask|=2;  // legend of galahad
				//if (xpos==0x21) spritemask|=2;  // shadow of the beast?
				if ((xpos>0) && (xpos<0x40)) spritemask|=2;

				if (spritemask==0x3)
					return;
				/* end todo: */

				{
					//int xdraw;
					int xtile;
					int yline = scanline - drawypos;

					for (xtile=0;xtile<width+1;xtile++)
					{
						int dat;

						if (!xflip)
						{
							UINT16 base_addr;
							int xxx;
							UINT32 gfxdata;
							int loopcount;

							if(megadrive_imode==3)
							{
								if (!yflip) base_addr = (addr<<4)+(xtile*((height+1)*(2*16)))+(yline*2);
								else base_addr = (addr<<4)+(xtile*((height+1)*(2*16)))+((((height+1)*16)-yline-1)*2);
							}
							else
							{
								if (!yflip) base_addr = (addr<<4)+(xtile*((height+1)*(2*8)))+(yline*2);
								else base_addr = (addr<<4)+(xtile*((height+1)*(2*8)))+((((height+1)*8)-yline-1)*2);
							}

							xxx = (xpos+xtile*8)&0x1ff;

							gfxdata = MEGADRIV_VDP_VRAM(base_addr+1) | (MEGADRIV_VDP_VRAM(base_addr+0)<<16);

							for(loopcount=0;loopcount<8;loopcount++)
							{
								dat = (gfxdata & 0xf0000000)>>28; gfxdata <<=4;
								if (dat) { if (!sprite_renderline[xxx]) { sprite_renderline[xxx] = dat | (colour<<4)| pri; } else { megadrive_sprite_collision = 1; } }
								xxx++;xxx&=0x1ff;
								if (--maxpixels == 0x00) return;
							}

						}
						else
						{
							UINT16 base_addr;
							int xxx;
							UINT32 gfxdata;

							int loopcount;

							if(megadrive_imode==3)
							{
								if (!yflip) base_addr = (addr<<4)+(((width-xtile))*((height+1)*(2*16)))+(yline*2);
								else base_addr =      (addr<<4)+(((width-xtile))*((height+1)*(2*16)))+((((height+1)*16)-yline-1)*2);

							}
							else
							{
								if (!yflip) base_addr = (addr<<4)+(((width-xtile))*((height+1)*(2*8)))+(yline*2);
								else base_addr =        (addr<<4)+(((width-xtile))*((height+1)*(2*8)))+((((height+1)*8)-yline-1)*2);
							}

							xxx = (xpos+xtile*8)&0x1ff;

							gfxdata = MEGADRIV_VDP_VRAM((base_addr+1)&0x7fff) | (MEGADRIV_VDP_VRAM((base_addr+0)&0x7fff)<<16);

							for(loopcount=0;loopcount<8;loopcount++)
							{
								dat = (gfxdata & 0x0000000f)>>0; gfxdata >>=4;
								if (dat) { if (!sprite_renderline[xxx]) { sprite_renderline[xxx] = dat | (colour<<4)| pri; } else { megadrive_sprite_collision = 1; } }
								xxx++;xxx&=0x1ff;
								if (--maxpixels == 0x00) return;
							}

						}
					}
				}
			}

			spritenum = link;
			maxsprites--;
		}
		while ((maxsprites>=0) && (link!=0));


	}
}

/* Clean up this function (!) */
static void genesis_render_videoline_to_videobuffer(int scanline)
{
	UINT16 base_a;
	UINT16 base_w=0;
	UINT16 base_b;

	UINT16 size;
	UINT16 hsize = 64;
	UINT16 vsize = 64;
	UINT16 window_right;
//  UINT16 window_hpos;
	UINT16 window_down;
//  UINT16 window_vpos;
	UINT16 hscroll_base;
//  UINT8  vscroll_mode;
//  UINT8  hscroll_mode;
	int window_firstline;
	int window_lastline;
	int window_firstcol;
	int window_lastcol;
	int screenwidth;
	int numcolumns = 0;
	int hscroll_a = 0;
	int hscroll_b = 0;
	int x;
	int window_hsize=0;
	int window_vsize=0;
	int window_is_bugged = 0;
	int non_window_firstcol;
	int non_window_lastcol;
	int screenheight = MEGADRIVE_REG01_240_LINE?240:224;

	/* Clear our Render Buffer */
	for (x=0;x<320;x++)
	{
		video_renderline[x]=MEGADRIVE_REG07_BGCOLOUR | 0x20000; // mark as BG
	}

	memset(highpri_renderline, 0, 320);

	/* is this line enabled? */
	if (!MEGADRIVE_REG01_DISP_ENABLE)
	{
		//mame_printf_debug("line disabled %d\n",scanline);
		return;
	}

	/* looks different? */
	if (MEGADRIVE_REG0_DISPLAY_DISABLE)
	{
		return;
	}



	base_a = MEGADRIVE_REG02_PATTERN_ADDR_A << 13;

	base_b = MEGADRIVE_REG04_PATTERN_ADDR_B << 13;
	size  = MEGADRIVE_REG10_HSCROLL_SIZE | (MEGADRIVE_REG10_VSCROLL_SIZE<<4);
	window_right = MEGADRIVE_REG11_WINDOW_RIGHT;
//  window_hpos = MEGADRIVE_REG11_WINDOW_HPOS;
	window_down = MEGADRIVE_REG12_WINDOW_DOWN;
//  window_vpos = MEGADRIVE_REG12_WINDOW_VPOS;

	screenwidth = MEGADRIVE_REG0C_RS0 | (MEGADRIVE_REG0C_RS1 << 1);

	switch (screenwidth)
	{
		case 0: numcolumns = 32; window_hsize = 32; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1f) << 11; break;
		case 1: numcolumns = 32; window_hsize = 32; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1f) << 11; break;
		case 2: numcolumns = 40; window_hsize = 64; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1e) << 11; break;
		case 3: numcolumns = 40; window_hsize = 64; window_vsize = 32; base_w = (MEGADRIVE_REG03_PATTERN_ADDR_W&0x1e) << 11; break; // talespin cares about base mask, used for status bar
	}

	//mame_printf_debug("screenwidth %d\n",screenwidth);

	//base_w = Machine->rand()&0xff;

	/* Calculate Exactly where we're going to draw the Window, and if the Window Bug applies */
	window_is_bugged = 0;
	if (window_right)
	{
		window_firstcol = MEGADRIVE_REG11_WINDOW_HPOS*16;
		window_lastcol = numcolumns*8;
		if (window_firstcol>window_lastcol) window_firstcol = window_lastcol;

		non_window_firstcol = 0;
		non_window_lastcol = window_firstcol;
	}
	else
	{
		window_firstcol = 0;
		window_lastcol = MEGADRIVE_REG11_WINDOW_HPOS*16;
		if (window_lastcol>numcolumns*8) window_lastcol = numcolumns*8;

		non_window_firstcol = window_lastcol;
		non_window_lastcol = numcolumns*8;

		if (window_lastcol!=0) window_is_bugged=1;
	}

	if (window_down)
	{
		window_firstline = MEGADRIVE_REG12_WINDOW_VPOS*8;
		window_lastline = screenheight; // 240 in PAL?
		if (window_firstline>screenheight) window_firstline = screenheight;
	}
	else
	{
		window_firstline = 0;
		window_lastline = MEGADRIVE_REG12_WINDOW_VPOS*8;
		if (window_lastline>screenheight) window_lastline = screenheight;
	}

	/* if we're on a window scanline between window_firstline and window_lastline the window is the full width of the screen */
	if (scanline>=window_firstline && scanline < window_lastline)
	{
		window_firstcol = 0; window_lastcol = numcolumns*8; // window is full-width of the screen
		non_window_firstcol = 0; non_window_lastcol=0; // disable non-window
	}


//    vscroll_mode = MEGADRIVE_REG0B_VSCROLL_MODE;
//    hscroll_mode = MEGADRIVE_REG0B_HSCROLL_MODE;
    hscroll_base = MEGADRIVE_REG0D_HSCROLL_ADDR<<10;

	switch (size)
	{
		case 0x00: hsize = 32; vsize = 32; break;
		case 0x01: hsize = 64; vsize = 32; break;
		case 0x02: hsize = 64; vsize = 1; /* mame_printf_debug("Invalid HSize! %02x\n",size);*/ break;
		case 0x03: hsize = 128;vsize = 32; break;

		case 0x10: hsize = 32; vsize = 64; break;
		case 0x11: hsize = 64; vsize = 64; break;
		case 0x12: hsize = 64; vsize = 1; /*mame_printf_debug("Invalid HSize! %02x\n",size);*/ break;
		case 0x13: hsize = 128;vsize = 32;/*mame_printf_debug("Invalid Total Size! %02x\n",size);*/break;

		case 0x20: hsize = 32; vsize = 64; mame_printf_debug("Invalid VSize!\n"); break;
		case 0x21: hsize = 64; vsize = 64; mame_printf_debug("Invalid VSize!\n"); break;
		case 0x22: hsize = 64; vsize = 1; /*mame_printf_debug("Invalid HSize & Invalid VSize!\n");*/ break;
		case 0x23: hsize = 128;vsize = 64; mame_printf_debug("Invalid VSize!\n"); break;

		case 0x30: hsize = 32; vsize = 128; break;
		case 0x31: hsize = 64; vsize = 64; /*mame_printf_debug("Invalid Total Size! %02x\n",size);*/break; // super skidmarks attempts this..
		case 0x32: hsize = 64; vsize = 1; /*mame_printf_debug("Invalid HSize & Invalid Total Size!\n");*/ break;
		case 0x33: hsize = 128;vsize = 128; mame_printf_debug("Invalid Total Size! %02x\n",size);break;
	}

	switch (MEGADRIVE_REG0B_HSCROLL_MODE)
	{
		case 0x00: // Full Screen Scroll
			hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0);
			hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1);
			break;

		case 0x01: // 'Broken' Line Scroll
			if(megadrive_imode==3)
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+((scanline>>1)&7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+((scanline>>1)&7)*2);
			}
			else
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+(scanline&7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+(scanline&7)*2);
			}
			break;

		case 0x02: // Cell Scroll
			if(megadrive_imode==3)
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+((scanline>>1)&~7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+((scanline>>1)&~7)*2);
			}
			else
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+(scanline&~7)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+(scanline&~7)*2);
			}
			break;

		case 0x03: // Full Line Scroll
			if(megadrive_imode==3)
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+(scanline>>1)*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+(scanline>>1)*2);
			}
			else
			{
				hscroll_a = MEGADRIV_VDP_VRAM((hscroll_base>>1)+0+scanline*2);
				hscroll_b = MEGADRIV_VDP_VRAM((hscroll_base>>1)+1+scanline*2);
			}
			break;
	}

	/* Low Priority B Tiles */
	{
		int column;
		int vscroll;

		for (column=0;column<numcolumns/2;column++)
		{	/* 20x 16x1 blocks */
			int vcolumn;
			int dpos;

			/* Get V Scroll Value for this block */

			dpos = column*16;

			{
				/* hscroll is not divisible by 8, this segment will contain 3 tiles, 1 partial, 1 whole, 1 partial */
				int hscroll_part = 8-(hscroll_b%8);
				int hcolumn;
				int tile_base;
				int tile_dat;
				int tile_addr;
				int tile_xflip;
				int tile_yflip;
				int tile_colour;
				int tile_pri;
				int dat;

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_b&0xf) vscroll = megadrive_vdp_vsram[((column-1)*2+1)&0x3f];
					else vscroll = megadrive_vdp_vsram[((column)*2+1)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[1];
				}

				hcolumn = ((column*2-1)-(hscroll_b>>3))&(hsize-1);

				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_b>>1)+((vcolumn>>4)*hsize)+hcolumn;

				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_b>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}


				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_b&0xf) vscroll = megadrive_vdp_vsram[((column-1)*2+1)&0x3f];
					else vscroll = megadrive_vdp_vsram[((column)*2+1)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[1];
				}

				hcolumn = ((column*2)-(hscroll_b>>3))&(hsize-1);

				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_b>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_b>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}

				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;

					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4))&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					vscroll = megadrive_vdp_vsram[((column)*2+1)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[1];
				}

				hcolumn = ((column*2+1)-(hscroll_b>>3))&(hsize-1);

				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_b>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_b>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}

				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}


				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;  if (!tile_pri) { if(dat) video_renderline[dpos] = dat | (tile_colour<<4); }  else highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						dpos++;
					}
				}
			}
		}
		/* END */
	}
	/* Low Priority A Tiles + Window(!) */

	{
		int column;
		int vscroll;

		for (column=window_firstcol/16;column<window_lastcol/16;column++)
		{
			int vcolumn;
			int dpos;

			int hcolumn;
			int tile_base;
			int tile_dat;
			int tile_addr;
			int tile_xflip;
			int tile_yflip;
			int tile_colour;
			int tile_pri;
			int dat;

			vcolumn = scanline&((window_vsize*8)-1);
			dpos = column*16;
			hcolumn = (column*2)&(window_hsize-1);

			if(megadrive_imode==3)
			{
				tile_base = (base_w>>1)+((vcolumn>>4)*window_hsize)+hcolumn;
			}
			else
			{
				tile_base = (base_w>>1)+((vcolumn>>3)*window_hsize)+hcolumn;
			}

			tile_base &=0x7fff;
			tile_dat = MEGADRIV_VDP_VRAM(tile_base);
			tile_xflip = (tile_dat&0x0800);
			tile_yflip = (tile_dat&0x1000);
			tile_colour =(tile_dat&0x6000)>>13;
			tile_pri = (tile_dat&0x8000)>>15;
			tile_addr = ((tile_dat&0x07ff)<<4);

			if(megadrive_imode==3)
			{
				tile_addr <<=1;
				tile_addr &=0x7fff;
			}

			if(megadrive_imode==3)
			{
				if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
				else tile_addr+=((0xf-vcolumn)&0xf)*2;
			}
			else
			{
				if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
				else tile_addr+=((7-vcolumn)&7)*2;
			}

			if (!tile_xflip)
			{
				/* 8 pixels */
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;

				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(28-(shift*4)))&0x000f;
					if (!tile_pri)
					{
						if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
					}
					dpos++;
				}
			}
			else
			{
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;
				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(shift*4) )&0x000f;
					if (!tile_pri)
					{
						if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
					}
					dpos++;

				}
			}


			hcolumn = (column*2+1)&(window_hsize-1);
			if(megadrive_imode==3)
			{
				tile_base = (base_w>>1)+((vcolumn>>4)*window_hsize)+hcolumn;
			}
			else
			{
				tile_base = (base_w>>1)+((vcolumn>>3)*window_hsize)+hcolumn;
			}
			tile_base &=0x7fff;
			tile_dat = MEGADRIV_VDP_VRAM(tile_base);
			tile_xflip = (tile_dat&0x0800);
			tile_yflip = (tile_dat&0x1000);
			tile_colour =(tile_dat&0x6000)>>13;
			tile_pri = (tile_dat&0x8000)>>15;
			tile_addr = ((tile_dat&0x07ff)<<4);

			if(megadrive_imode==3)
			{
				tile_addr <<=1;
				tile_addr &=0x7fff;
			}

			if(megadrive_imode==3)
			{
				if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
				else tile_addr+=((0xf-vcolumn)&0xf)*2;
			}
			else
			{
				if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
				else tile_addr+=((7-vcolumn)&7)*2;
			}

			if (!tile_xflip)
			{
				/* 8 pixels */
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;

				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(28-(shift*4)))&0x000f;
					if (!tile_pri)
					{
						if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
					}
					dpos++;
				}
			}
			else
			{
				UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
				int shift;
				for (shift=0;shift<8;shift++)
				{
					dat = (gfxdata>>(shift*4) )&0x000f;
					if (!tile_pri)
					{
						if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
					}
					else
					{
						if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
						else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
					}
					dpos++;
				}
			}
		}

		/* Non Window Part */

		for (column=non_window_firstcol/16;column<non_window_lastcol/16;column++)
		{	/* 20x 16x1 blocks */
		//  int xx;
			int vcolumn;
			int dpos;

			dpos = column*16;

			{	/* hscroll is not divisible by 8, this segment will contain 3 tiles, 1 partial, 1 whole, 1 partial */
				int hscroll_part = 8-(hscroll_a%8);
				int hcolumn;
				int tile_base;
				int tile_dat;
				int tile_addr;
				int tile_xflip;
				int tile_yflip;
				int tile_colour;
				int tile_pri;
				int dat;

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_a&0xf) vscroll = megadrive_vdp_vsram[((column-1)*2+0)&0x3f];
					else vscroll = megadrive_vdp_vsram[((column)*2+0)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[0];
				}


				if ((!window_is_bugged) || ((hscroll_a&0xf)==0) || (column>non_window_firstcol/16)) hcolumn = ((column*2-1)-(hscroll_a>>3))&(hsize-1);
				else hcolumn = ((column*2+1)-(hscroll_a>>3))&(hsize-1);

				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
				}

				if(megadrive_imode==3)
				{
					tile_base = (base_a>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					tile_base = (base_a>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}


				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=hscroll_part;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					if (hscroll_a&0xf) vscroll = megadrive_vdp_vsram[((column-1)*2+0)&0x3f];
					else vscroll = megadrive_vdp_vsram[((column)*2+0)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[0];
				}

				if ((!window_is_bugged) || ((hscroll_a&0xf)==0) || (column>non_window_firstcol/16)) hcolumn = ((column*2)-(hscroll_a>>3))&(hsize-1); // not affected by bug?
				else
				{
					if ((hscroll_a&0xf)<8) hcolumn = ((column*2)-(hscroll_a>>3))&(hsize-1);
					else hcolumn = ((column*2+2)-(hscroll_a>>3))&(hsize-1);
				}


				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_a>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_a>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}

				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);


				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<8;shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}

				if (MEGADRIVE_REG0B_VSCROLL_MODE)
				{
					vscroll = megadrive_vdp_vsram[((column)*2+0)&0x3f];
				}
				else
				{
					vscroll = megadrive_vdp_vsram[0];
				}

				if ((!window_is_bugged) || ((hscroll_a&0xf)==0) || (column>non_window_firstcol/16)) hcolumn = ((column*2+1)-(hscroll_a>>3))&(hsize-1);
				else hcolumn = ((column*2+1)-(hscroll_a>>3))&(hsize-1);

				if(megadrive_imode==3)
				{
					vcolumn = (vscroll + scanline)&((vsize*16)-1);
					tile_base = (base_a>>1)+((vcolumn>>4)*hsize)+hcolumn;
				}
				else
				{
					vcolumn = (vscroll + scanline)&((vsize*8)-1);
					tile_base = (base_a>>1)+((vcolumn>>3)*hsize)+hcolumn;
				}
				tile_base &=0x7fff;
				tile_dat = MEGADRIV_VDP_VRAM(tile_base);
				tile_xflip = (tile_dat&0x0800);
				tile_yflip = (tile_dat&0x1000);
				tile_colour =(tile_dat&0x6000)>>13;
				tile_pri = (tile_dat&0x8000)>>15;
				tile_addr = ((tile_dat&0x07ff)<<4);

				if(megadrive_imode==3)
				{
					tile_addr <<=1;
					tile_addr &=0x7fff;
				}

				if(megadrive_imode==3)
				{
					if (!tile_yflip) tile_addr+=(vcolumn&0xf)*2;
					else tile_addr+=((0xf-vcolumn)&0xf)*2;
				}
				else
				{
					if (!tile_yflip) tile_addr+=(vcolumn&7)*2;
					else tile_addr+=((7-vcolumn)&7)*2;
				}

				if (!tile_xflip)
				{
					/* 8 pixels */
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;

					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(28-(shift*4)))&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
				else
				{
					UINT32 gfxdata = (MEGADRIV_VDP_VRAM(tile_addr+0)<<16)|MEGADRIV_VDP_VRAM(tile_addr+1);
					int shift;
					for (shift=0;shift<(hscroll_part);shift++)
					{
						dat = (gfxdata>>(shift*4) )&0x000f;
						if (!tile_pri)
						{
							if(dat) video_renderline[dpos] = dat | (tile_colour<<4);
						}
						else
						{
							if (dat) highpri_renderline[dpos]  = dat | (tile_colour<<4) | 0x80;
							else highpri_renderline[dpos] = highpri_renderline[dpos]|0x80;
						}
						dpos++;
					}
				}
			}
		}
	}
		/* END */

/* MEGADRIVE_REG0C_SHADOW_HIGLIGHT */
		/* Low Priority Sprites */
		for (x=0;x<320;x++)
		{
			if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
			{
				if (sprite_renderline[x+128] & 0x40)
				{
					video_renderline[x] = sprite_renderline[x+128]&0x3f;
					video_renderline[x] |= 0x10000; // mark as sprite pixel
				}
			}
			else
			{	/* Special Shadow / Highlight processing */

				if (sprite_renderline[x+128] & 0x40)
				{
					UINT8 spritedata;
					spritedata = sprite_renderline[x+128]&0x3f;

					if ((spritedata==0x0e) || (spritedata==0x1e) || (spritedata==0x2e))
					{
						/* BUG in sprite chip, these colours are always normal intensity */
						video_renderline[x] = spritedata | 0x4000;
						video_renderline[x] |= 0x10000; // mark as sprite pixel
					}
					else if (spritedata==0x3e)
					{
						/* Everything below this is half colour, mark with 0x8000 to mark highlight' */
						video_renderline[x] = video_renderline[x]|0x8000; // spiderwebs..
					}
					else if (spritedata==0x3f)
					{
						/* This is a Shadow operator, but everything below is already low pri, no effect */
						video_renderline[x] = video_renderline[x]|0x2000;

					}
					else
					{
						video_renderline[x] = spritedata;
						video_renderline[x] |= 0x10000; // mark as sprite pixel
					}

				}
			}
		}
		/* High Priority A+B Tiles */
		for (x=0;x<320;x++)
		{
			if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
			{
				/* Normal Processing */
				int dat;
				dat = highpri_renderline[x];

				if (dat&0x80)
				{
					 if (dat&0x0f) video_renderline[x] = highpri_renderline[x]&0x3f;
				}
			}
			else
			{
				/* Shadow / Highlight Mode */
				int dat;
				dat = highpri_renderline[x];

				if (dat&0x80)
				{
					 if (dat&0x0f) video_renderline[x] = (highpri_renderline[x]&0x3f) | 0x4000;
					 else video_renderline[x] = video_renderline[x] | 0x4000; // set 'normal'
				}
			}
		}

		/* High Priority Sprites */
		for (x=0;x<320;x++)
		{
			if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
			{
				/* Normal */
				if (sprite_renderline[x+128] & 0x80)
				{
					video_renderline[x] = sprite_renderline[x+128]&0x3f;
					video_renderline[x] |= 0x10000; // mark as sprite pixel
				}
			}
			else
			{
				if (sprite_renderline[x+128] & 0x80)
				{
					UINT8 spritedata;
					spritedata = sprite_renderline[x+128]&0x3f;

					if (spritedata==0x3e)
					{
						/* set flag 0x8000 to indicate highlight */
						video_renderline[x] = video_renderline[x]|0x8000;
					}
					else if (spritedata==0x3f)
					{
						/* This is a Shadow operator set shadow bit */
						video_renderline[x] = video_renderline[x]|0x2000;
					}
					else
					{
						video_renderline[x] = spritedata | 0x4000;
						video_renderline[x] |= 0x10000; // mark as sprite pixel
					}
				}
			}
		}
}


/* This converts our render buffer to real screen colours */
static void genesis_render_videobuffer_to_screenbuffer(running_machine &machine, int scanline)
{
	UINT16*lineptr;
	int x;
	lineptr = &megadriv_render_bitmap->pix16(scanline);

	UINT32* _32x_linerender = _32x_render_videobuffer_to_screenbuffer_helper(machine, scanline);



	if (!MEGADRIVE_REG0C_SHADOW_HIGLIGHT)
	{

		for (x=0;x<320;x++)
		{
			UINT32 dat;
			dat = video_renderline[x];
			int drawn = 0;

			// low priority 32x - if it's the bg pen, we have a 32x, and it's display is enabled...
			if ((dat&0x20000) && (_32x_is_connected) && (_32x_displaymode != 0))
			{
				if (!_32x_videopriority)
				{
					if (!(_32x_linerender[x]&0x8000))
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
						drawn = 1;
					}
				}
				else
				{
					if ((_32x_linerender[x]&0x8000))
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
						drawn = 1;
					}
				}
			}


			if (drawn==0)
			{
				if (dat&0x10000)
					lineptr[x] = megadrive_vdp_palette_lookup_sprite[(dat&0x0f) | segac2_sp_pal_lookup[(dat&0x30)>>4]];
				else
					lineptr[x] = megadrive_vdp_palette_lookup[(dat&0x0f) | segac2_bg_pal_lookup[(dat&0x30)>>4]];
			}



		}
	}
	else
	{

		for (x=0;x<320;x++)
		{
			UINT32 dat;
			dat = video_renderline[x];

			int drawn = 0;

			// low priority 32x - if it's the bg pen, we have a 32x, and it's display is enabled...
			if ((dat&0x20000) && (_32x_is_connected) && (_32x_displaymode != 0))
			{
				if (!_32x_videopriority)
				{
					if (!(_32x_linerender[x]&0x8000))
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
						drawn = 1;
					}
				}
				else
				{
					if ((_32x_linerender[x]&0x8000))
					{
						lineptr[x] = _32x_linerender[x]&0x7fff;
						drawn = 1;
					}
				}
			}


			if (drawn==0)
			{
				/* Verify my handling.. I'm not sure all cases are correct */
				switch (dat&0x1e000)
				{
					case 0x00000: // low priority, no shadow sprite, no highlight = shadow
					case 0x02000: // low priority, shadow sprite, no highlight = shadow
					case 0x06000: // normal pri,   shadow sprite, no highlight = shadow?
					case 0x10000: // (sprite) low priority, no shadow sprite, no highlight = shadow
					case 0x12000: // (sprite) low priority, shadow sprite, no highlight = shadow
					case 0x16000: // (sprite) normal pri,   shadow sprite, no highlight = shadow?
						lineptr[x] = megadrive_vdp_palette_lookup_shadow[(dat&0x0f)  | segac2_bg_pal_lookup[(dat&0x30)>>4]];
						break;

					case 0x4000: // normal pri, no shadow sprite, no highlight = normal;
					case 0x8000: // low pri, highlight sprite = normal;
						lineptr[x] = megadrive_vdp_palette_lookup[(dat&0x0f)  | segac2_bg_pal_lookup[(dat&0x30)>>4]];
						break;

					case 0x14000: // (sprite) normal pri, no shadow sprite, no highlight = normal;
					case 0x18000: // (sprite) low pri, highlight sprite = normal;
						lineptr[x] = megadrive_vdp_palette_lookup_sprite[(dat&0x0f)  | segac2_sp_pal_lookup[(dat&0x30)>>4]];
						break;


					case 0x0c000: // normal pri, highlight set = highlight?
					case 0x1c000: // (sprite) normal pri, highlight set = highlight?
						lineptr[x] = megadrive_vdp_palette_lookup_highlight[(dat&0x0f) | segac2_bg_pal_lookup[(dat&0x30)>>4]];
						break;

					case 0x0a000: // shadow set, highlight set - not possible
					case 0x0e000: // shadow set, highlight set, normal set, not possible
					case 0x1a000: // (sprite)shadow set, highlight set - not possible
					case 0x1e000: // (sprite)shadow set, highlight set, normal set, not possible
					default:
						lineptr[x] = machine.rand()&0x3f;
					break;
				}
			}



		}

	}


	// high priority 32x
	if ((_32x_is_connected) && (_32x_displaymode != 0))
	{
		for (x=0;x<320;x++)
		{
			if (!_32x_videopriority)
			{
				if ((_32x_linerender[x]&0x8000))
					lineptr[x] = _32x_linerender[x]&0x7fff;
			}
			else
			{
				if (!(_32x_linerender[x]&0x8000))
					lineptr[x] = _32x_linerender[x]&0x7fff;
			}
		}
	}
}

static void genesis_render_scanline(running_machine &machine, int scanline)
{
	//if (MEGADRIVE_REG01_DMA_ENABLE==0) mame_printf_debug("off\n");
	genesis_render_spriteline_to_spritebuffer(genesis_scanline_counter);
	genesis_render_videoline_to_videobuffer(scanline);
	genesis_render_videobuffer_to_screenbuffer(machine, scanline);
}

UINT16 get_hposition(void)
{
//  static int lowest = 99999;
//  static int highest = -99999;

	attotime time_elapsed_since_megadriv_scanline_timer;
	UINT16 value4;

	time_elapsed_since_megadriv_scanline_timer = megadriv_scanline_timer->time_elapsed();

	if (time_elapsed_since_megadriv_scanline_timer.attoseconds<(ATTOSECONDS_PER_SECOND/megadriv_framerate /megadrive_total_scanlines))
	{
		value4 = (UINT16)(megadrive_max_hposition*((double)(time_elapsed_since_megadriv_scanline_timer.attoseconds) / (double)(ATTOSECONDS_PER_SECOND/megadriv_framerate /megadrive_total_scanlines)));
	}
	else /* in some cases (probably due to rounding errors) we get some stupid results (the odd huge value where the time elapsed is much higher than the scanline time??!).. hopefully by clamping the result to the maximum we limit errors */
	{
		value4 = megadrive_max_hposition;
	}

//  if (value4>highest) highest = value4;
//  if (value4<lowest) lowest = value4;

	//mame_printf_debug("%d low %d high %d scancounter %d\n", value4, lowest, highest,genesis_scanline_counter);

	return value4;
}


VIDEO_START(megadriv)
{
	int x;

	megadriv_render_bitmap = auto_bitmap_ind16_alloc(machine, machine.primary_screen->width(), machine.primary_screen->height());

	megadrive_vdp_vram  = auto_alloc_array(machine, UINT16, 0x10000/2);
	megadrive_vdp_cram  = auto_alloc_array(machine, UINT16, 0x80/2);
	megadrive_vdp_vsram = auto_alloc_array(machine, UINT16, 0x80/2);
	megadrive_vdp_internal_sprite_attribute_table = auto_alloc_array(machine, UINT16, 0x400/2);

	for (x=0;x<0x20;x++)
		megadrive_vdp_register[x]=0;
//  memset(megadrive_vdp_vram, 0xff, 0x10000);
//  memset(megadrive_vdp_cram, 0xff, 0x80);
//  memset(megadrive_vdp_vsram, 0xff, 0x80);

	memset(megadrive_vdp_vram, 0x00, 0x10000);
	memset(megadrive_vdp_cram, 0x00, 0x80);
	memset(megadrive_vdp_vsram, 0x00, 0x80);
	memset(megadrive_vdp_internal_sprite_attribute_table, 0x00, 0x400);

	megadrive_max_hposition = 480;

	sprite_renderline = auto_alloc_array(machine, UINT8, 1024);
	highpri_renderline = auto_alloc_array(machine, UINT8, 320);
	video_renderline = auto_alloc_array(machine, UINT32, 320);

	megadrive_vdp_palette_lookup = auto_alloc_array(machine, UINT16, 0x40);
	megadrive_vdp_palette_lookup_sprite = auto_alloc_array(machine, UINT16, 0x40);

	megadrive_vdp_palette_lookup_shadow = auto_alloc_array(machine, UINT16, 0x40);
	megadrive_vdp_palette_lookup_highlight = auto_alloc_array(machine, UINT16, 0x40);

	memset(megadrive_vdp_palette_lookup,0x00,0x40*2);
	memset(megadrive_vdp_palette_lookup_sprite,0x00,0x40*2);

	memset(megadrive_vdp_palette_lookup_shadow,0x00,0x40*2);
	memset(megadrive_vdp_palette_lookup_highlight,0x00,0x40*2);

	/* no special lookups */
	segac2_bg_pal_lookup[0] = 0x00;
	segac2_bg_pal_lookup[1] = 0x10;
	segac2_bg_pal_lookup[2] = 0x20;
	segac2_bg_pal_lookup[3] = 0x30;

	segac2_sp_pal_lookup[0] = 0x00;
	segac2_sp_pal_lookup[1] = 0x10;
	segac2_sp_pal_lookup[2] = 0x20;
	segac2_sp_pal_lookup[3] = 0x30;
}


TIMER_DEVICE_CALLBACK( megadriv_scanline_timer_callback )
{
	/* This function is called at the very start of every scanline starting at the very
       top-left of the screen.  The first scanline is scanline 0 (we set scanline to -1 in
       VIDEO_EOF) */

	timer.machine().scheduler().synchronize();
	/* Compensate for some rounding errors

       When the counter reaches 261 we should have reached the end of the frame, however due
       to rounding errors in the timer calculation we're not quite there.  Let's assume we are
       still in the previous scanline for now.
    */

	if (genesis_scanline_counter!=(megadrive_total_scanlines-1))
	{
		genesis_scanline_counter++;
//      mame_printf_debug("scanline %d\n",genesis_scanline_counter);
		megadriv_scanline_timer->adjust(attotime::from_hz(megadriv_framerate) / megadrive_total_scanlines);
		megadriv_render_timer->adjust(attotime::from_usec(1));

		if (genesis_scanline_counter==megadrive_irq6_scanline )
		{
		//  mame_printf_debug("x %d",genesis_scanline_counter);
			irq6_on_timer->adjust(attotime::from_usec(6));
			megadrive_irq6_pending = 1;
			megadrive_vblank_flag = 1;

			// 32x interrupt!
			if (_32x_is_connected)
			{
				_32x_scanline_cb0(timer.machine());
			}

		}



		_32x_check_framebuffer_swap();


	//  if (genesis_scanline_counter==0) irq4counter = MEGADRIVE_REG0A_HINT_VALUE;
		// irq4counter = MEGADRIVE_REG0A_HINT_VALUE;

		if (genesis_scanline_counter<=224)
		{
			irq4counter--;

			if (irq4counter==-1)
			{
				if (megadrive_imode==3) irq4counter = MEGADRIVE_REG0A_HINT_VALUE*2;
				else irq4counter=MEGADRIVE_REG0A_HINT_VALUE;

				megadrive_irq4_pending = 1;

				if (MEGADRIVE_REG0_IRQ4_ENABLE)
				{
					irq4_on_timer->adjust(attotime::from_usec(1));
					//mame_printf_debug("irq4 on scanline %d reload %d\n",genesis_scanline_counter,MEGADRIVE_REG0A_HINT_VALUE);
				}
			}
		}
		else
		{
			if (megadrive_imode==3) irq4counter = MEGADRIVE_REG0A_HINT_VALUE*2;
			else irq4counter=MEGADRIVE_REG0A_HINT_VALUE;
		}

		//if (genesis_scanline_counter==0) irq4_on_timer->adjust(attotime::from_usec(2));

		if(_32x_is_connected)
		{
			_32x_scanline_cb1();
		}


		if (timer.machine().device("genesis_snd_z80") != NULL)
		{
			if (genesis_scanline_counter == megadrive_z80irq_scanline)
			{
				if ((genz80.z80_has_bus == 1) && (genz80.z80_is_reset == 0))
					cputag_set_input_line(timer.machine(), "genesis_snd_z80", 0, HOLD_LINE);
			}
			if (genesis_scanline_counter == megadrive_z80irq_scanline + 1)
			{
				cputag_set_input_line(timer.machine(), "genesis_snd_z80", 0, CLEAR_LINE);
			}
		}

	}
	else /* pretend we're still on the same scanline to compensate for rounding errors */
	{
		genesis_scanline_counter = megadrive_total_scanlines - 1;
	}

}

TIMER_DEVICE_CALLBACK( irq6_on_callback )
{
	//mame_printf_debug("irq6 active on %d\n",genesis_scanline_counter);

	{
//      megadrive_irq6_pending = 1;
		if (MEGADRIVE_REG01_IRQ6_ENABLE || genesis_always_irq6)
			cputag_set_input_line(timer.machine(), "maincpu", 6, HOLD_LINE);
	}
}


SCREEN_VBLANK(megadriv)
{
	// rising edge
	if (vblank_on)
	{
		rectangle visarea;
		int scr_width = 320;

		megadrive_vblank_flag = 0;
		//megadrive_irq6_pending = 0; /* NO! (breaks warlock) */

		/* Set it to -1 here, so it becomes 0 when the first timer kicks in */
		genesis_scanline_counter = -1;
		megadrive_sprite_collision=0;//? when to reset this ..
		megadrive_imode = MEGADRIVE_REG0C_INTERLEAVE; // can't change mid-frame..
		megadrive_imode_odd_frame^=1;
//      cputag_set_input_line(machine, "genesis_snd_z80", 0, CLEAR_LINE); // if the z80 interrupt hasn't happened by now, clear it..

		if (screen.machine().root_device().ioport("RESET")->read_safe(0x00) & 0x01)
			cputag_set_input_line(screen.machine(), "maincpu", INPUT_LINE_RESET, PULSE_LINE);


		if (MEGADRIVE_REG01_240_LINE)
		{
			if (!megadrive_region_pal)
			{
				/* this is invalid! */
				megadrive_visible_scanlines = 240;
				megadrive_total_scanlines = 262;
				megadrive_irq6_scanline = 240;
				megadrive_z80irq_scanline = 240;
			}
			else
			{
				megadrive_visible_scanlines = 240;
				megadrive_total_scanlines = 313;
				megadrive_irq6_scanline = 240;
				megadrive_z80irq_scanline = 240;
			}
		}
		else
		{
			if (!megadrive_region_pal)
			{
				megadrive_visible_scanlines = 224;
				megadrive_total_scanlines=262;
				megadrive_irq6_scanline = 224;
				megadrive_z80irq_scanline = 224;
			}
			else
			{
				megadrive_visible_scanlines = 224;
				megadrive_total_scanlines=313;
				megadrive_irq6_scanline = 224;
				megadrive_z80irq_scanline = 224;
			}
		}

		if (megadrive_imode==3)
		{
			megadrive_visible_scanlines<<=1;
			megadrive_total_scanlines<<=1;
			megadrive_irq6_scanline <<=1;
			megadrive_z80irq_scanline <<=1;
		}


		//get_hposition();
		switch (MEGADRIVE_REG0C_RS0 | (MEGADRIVE_REG0C_RS1 << 1))
		{
			 /* note, add 240 mode + init new timings! */
			case 0:scr_width = 256;break;// configure_screen(0, 256-1, megadrive_visible_scanlines-1,(double)megadriv_framerate); break;
			case 1:scr_width = 256;break;// configure_screen(0, 256-1, megadrive_visible_scanlines-1,(double)megadriv_framerate); mame_printf_debug("invalid screenmode!\n"); break;
			case 2:scr_width = 320;break;// configure_screen(0, 320-1, megadrive_visible_scanlines-1,(double)megadriv_framerate); break; /* technically invalid, but used in rare cases */
			case 3:scr_width = 320;break;// configure_screen(0, 320-1, megadrive_visible_scanlines-1,(double)megadriv_framerate); break;
		}
//      mame_printf_debug("my mode %02x", megadrive_vdp_register[0x0c]);

		visarea.set(0, scr_width-1, 0, megadrive_visible_scanlines-1);

		screen.machine().primary_screen->configure(scr_width, megadrive_visible_scanlines, visarea, HZ_TO_ATTOSECONDS(megadriv_framerate));

		megadriv_scanline_timer->adjust(attotime::zero);

		if(_32x_is_connected)
			_32x_hcount_compare_val = -1;
	}
}

timer_device* megadriv_render_timer;

TIMER_DEVICE_CALLBACK( megadriv_render_timer_callback )
{
	if (genesis_scanline_counter>=0 && genesis_scanline_counter<megadrive_visible_scanlines)
	{
		genesis_render_scanline(timer.machine(), genesis_scanline_counter);
	}
}

void megadriv_reset_vdp(void)
{
	megadrive_imode = 0;
	irq4counter = -1;
	megadrive_total_scanlines = 262;
	megadrive_visible_scanlines = 224;
	megadrive_irq6_scanline = 224;
	megadrive_z80irq_scanline = 226;
}
