class djmain_state : public driver_device
{
public:
	djmain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_sndram_bank;
	UINT8 *m_sndram;
	int m_turntable_select;
	UINT8 m_turntable_last_pos[2];
	UINT16 m_turntable_pos[2];
	UINT8 m_pending_vb_int;
	UINT16 m_v_ctrl;
	UINT32 m_obj_regs[0xa0/4];
	const UINT8 *m_ide_user_password;
	const UINT8 *m_ide_master_password;
	UINT32 *m_obj_ram;
};


/*----------- defined in video/djmain.c -----------*/

SCREEN_UPDATE_RGB32( djmain );
VIDEO_START( djmain );

void djmain_tile_callback(running_machine& machine, int layer, int *code, int *color, int *flags);
