#define NVRAM_UNLOCK_SEQ_LEN 10

class coolpool_state : public driver_device
{
public:
	coolpool_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	UINT16 *m_vram_base;
	required_shared_ptr<UINT16> m_nvram;

	UINT8 m_cmd_pending;
	UINT16 m_iop_cmd;
	UINT16 m_iop_answer;
	int m_iop_romaddr;

	UINT8 m_newx[3];
	UINT8 m_newy[3];
	UINT8 m_oldx[3];
	UINT8 m_oldy[3];
	int m_dx[3];
	int m_dy[3];

	UINT16 m_result;
	UINT16 m_lastresult;

	device_t *m_maincpu;
	device_t *m_dsp;
	UINT16 m_nvram_write_seq[NVRAM_UNLOCK_SEQ_LEN];
	UINT8 m_nvram_write_enable;
	UINT8 m_old_cmd;
	UINT8 m_same_cmd_count;
};
