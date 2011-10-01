#include "video/segaic24.h"

class segas24_state : public driver_device
{
public:
	segas24_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }


	static const UINT8  mahmajn_mlt[8];
	static const UINT8 mahmajn2_mlt[8];
	static const UINT8      qgh_mlt[8];
	static const UINT8 bnzabros_mlt[8];
	static const UINT8   qrouka_mlt[8];
	static const UINT8 quizmeku_mlt[8];
	static const UINT8   dcclub_mlt[8];

	int fdc_status;
	int fdc_track;
	int fdc_sector;
	int fdc_data;
	int fdc_phys_track;
	int fdc_irq;
	int fdc_drq;
	int fdc_span;
	int fdc_index_count;
	UINT8 *fdc_pt;
	int track_size;
	int cur_input_line;
	UINT8 hotrod_ctrl_cur;
	UINT8 resetcontrol;
	UINT8 prev_resetcontrol;
	UINT8 curbank;
	UINT8 mlatch;
	const UINT8 *mlatch_table;

	UINT16 irq_tdata, irq_tval;
	UINT8 irq_tmode, irq_allow0, irq_allow1;
	int irq_timer_pend0;
	int irq_timer_pend1;
	int irq_yms;
	int irq_vblank;
	int irq_sprite;
	attotime irq_synctime, irq_vsynctime;
	timer_device *irq_timer;
	timer_device *irq_timer_clear;

	UINT16 *shared_ram;
	UINT8 (segas24_state::*io_r)(UINT8 port);
	void (segas24_state::*io_w)(UINT8 port, UINT8 data);
	UINT8 io_cnt, io_dir;

	segas24_tile *vtile;
	segas24_sprite *vsprite;
	segas24_mixer *vmixer;

	DECLARE_READ16_MEMBER(  sys16_paletteram_r );
	DECLARE_WRITE16_MEMBER( sys16_paletteram_w );
	DECLARE_READ16_MEMBER(  irq_r );
	DECLARE_WRITE16_MEMBER( irq_w );
	DECLARE_READ16_MEMBER(  fdc_r );
	DECLARE_WRITE16_MEMBER( fdc_w );
	DECLARE_READ16_MEMBER(  fdc_status_r );
	DECLARE_WRITE16_MEMBER( fdc_ctrl_w );
	DECLARE_READ16_MEMBER(  curbank_r );
	DECLARE_WRITE16_MEMBER( curbank_w );
	DECLARE_READ16_MEMBER(  mlatch_r );
	DECLARE_WRITE16_MEMBER( mlatch_w );
	DECLARE_READ16_MEMBER(  hotrod3_ctrl_r );
	DECLARE_WRITE16_MEMBER( hotrod3_ctrl_w );
	DECLARE_READ16_MEMBER(  iod_r );
	DECLARE_WRITE16_MEMBER( iod_w );
	DECLARE_READ16_MEMBER ( sys16_io_r );
	DECLARE_WRITE16_MEMBER( sys16_io_w );

	UINT8 hotrod_io_r(UINT8 port);
	UINT8 dcclub_io_r(UINT8 port);
	UINT8 mahmajn_io_r(UINT8 port);

	void hotrod_io_w(UINT8 port, UINT8 data);
	void mahmajn_io_w(UINT8 port, UINT8 data);

	void fdc_init();
	void reset_reset();
	void reset_bank();
	void irq_init();
	void irq_timer_sync();
	void irq_timer_start(int old_tmode);
	void reset_control_w(UINT8 data);
};

/*----------- defined in machine/s24fd.c -----------*/

extern void s24_fd1094_machine_init(running_machine &machine);
extern void s24_fd1094_driver_init(running_machine &machine);


/*----------- defined in video/segas24.c -----------*/

SCREEN_UPDATE(system24);
