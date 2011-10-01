
class mexico86_state : public driver_device
{
public:
	mexico86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *     m_protection_ram;
	UINT8 *     m_videoram;
	UINT8 *     m_objectram;
	size_t      m_objectram_size;

	/* video-related */
	int      m_charbank;

	/* mcu */
	/* mexico86 68705 protection */
	UINT8    m_port_a_in;
	UINT8    m_port_a_out;
	UINT8    m_ddr_a;
	UINT8    m_port_b_in;
	UINT8    m_port_b_out;
	UINT8    m_ddr_b;
	int      m_address;
	int      m_latch;
	/* kikikai mcu simulation */
	int      m_mcu_running;
	int      m_mcu_initialised;
	int      m_coin_last;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_subcpu;
	device_t *m_mcu;

	/* queue */
	UINT8 m_queue[64];
	int m_qfront;
	int m_qstate;
};


/*----------- defined in machine/mexico86.c -----------*/

WRITE8_HANDLER( mexico86_f008_w );
INTERRUPT_GEN( kikikai_interrupt );
INTERRUPT_GEN( mexico86_m68705_interrupt );
READ8_HANDLER( mexico86_68705_port_a_r );
WRITE8_HANDLER( mexico86_68705_port_a_w );
WRITE8_HANDLER( mexico86_68705_ddr_a_w );
READ8_HANDLER( mexico86_68705_port_b_r );
WRITE8_HANDLER( mexico86_68705_port_b_w );
WRITE8_HANDLER( mexico86_68705_ddr_b_w );


/*----------- defined in video/mexico86.c -----------*/

WRITE8_HANDLER( mexico86_bankswitch_w );

SCREEN_UPDATE( mexico86 );
SCREEN_UPDATE( kikikai );
