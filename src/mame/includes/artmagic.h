/*************************************************************************

    Art & Magic hardware

**************************************************************************/

class artmagic_state : public driver_device
{
public:
	artmagic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_control;
	UINT8 m_tms_irq;
	UINT8 m_hack_irq;
	UINT8 m_prot_input[16];
	UINT8 m_prot_input_index;
	UINT8 m_prot_output[16];
	UINT8 m_prot_output_index;
	UINT8 m_prot_output_bit;
	UINT8 m_prot_bit_index;
	UINT16 m_prot_save;
	void (*m_protection_handler)(running_machine &);
	UINT16 *m_vram0;
	UINT16 *m_vram1;
	int m_xor[16];
	int m_is_stoneball;
	UINT16 *m_blitter_base;
	UINT32 m_blitter_mask;
	UINT16 m_blitter_data[8];
	UINT8 m_blitter_page;
	attotime m_blitter_busy_until;
};


/*----------- defined in video/artmagic.c -----------*/

VIDEO_START( artmagic );

void artmagic_to_shiftreg(address_space *space, offs_t address, UINT16 *data);
void artmagic_from_shiftreg(address_space *space, offs_t address, UINT16 *data);

READ16_HANDLER( artmagic_blitter_r );
WRITE16_HANDLER( artmagic_blitter_w );

void artmagic_scanline(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);
