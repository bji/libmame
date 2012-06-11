/***************************************************************************

    Technos Mysterious Stones hardware

    driver by Nicola Salmoria

***************************************************************************/


#define	MYSTSTON_MASTER_CLOCK	(XTAL_12MHz)


class mystston_state : public driver_device
{
public:
	mystston_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_ay8910_data(*this, "ay8910_data"),
		m_ay8910_select(*this, "ay8910_select"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_scroll(*this, "scroll"),
		m_video_control(*this, "video_control") { }

	/* machine state */
	required_shared_ptr<UINT8> m_ay8910_data;
	required_shared_ptr<UINT8> m_ay8910_select;

	/* video state */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	emu_timer *m_interrupt_timer;
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_video_control;
	DECLARE_WRITE8_MEMBER(irq_clear_w);
	DECLARE_WRITE8_MEMBER(mystston_ay8910_select_w);
	DECLARE_WRITE8_MEMBER(mystston_video_control_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
};


/*----------- defined in drivers/mystston.c -----------*/

void mystston_on_scanline_interrupt(running_machine &machine);


/*----------- defined in video/mystston.c -----------*/

MACHINE_CONFIG_EXTERN( mystston_video );
