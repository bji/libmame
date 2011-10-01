#ifndef __MAPLE_DC_H__
#define __MAPLE_DC_H__

#include "cpu/sh4/sh4.h"

#define MCFG_MAPLE_DC_ADD(_tag, _maincpu_tag) \
	MCFG_DEVICE_ADD(_tag, MAPLE_DC, 0) \
	maple_dc_device::static_set_maincpu_tag(*device, _maincpu_tag);

class maple_device;

class maple_dc_device : public device_t
{
public:
	maple_dc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	static void static_set_maincpu_tag(device_t &device, const char *maincpu_tag);

	DECLARE_READ32_MEMBER(sb_mdstar_r);  // 5f6c04
	DECLARE_WRITE32_MEMBER(sb_mdstar_w);
	DECLARE_READ32_MEMBER(sb_mden_r);    // 5f6c14
	DECLARE_WRITE32_MEMBER(sb_mden_w);
	DECLARE_READ32_MEMBER(sb_mdst_r);    // 5f6c18
	DECLARE_WRITE32_MEMBER(sb_mdst_w);
	DECLARE_READ32_MEMBER(sb_msys_r);    // 5f6c80
	DECLARE_WRITE32_MEMBER(sb_msys_w);
	DECLARE_WRITE32_MEMBER(sb_mdapro_w); // 5f6c8c

	void end_of_reply();
	void register_port(int port, maple_device *device);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	enum {
		DMA_IDLE,
		DMA_DONE,
		DMA_SEND,
		DMA_WAIT_NOP,
		DMA_WAIT_RESET,
		DMA_WAIT_REPLY,
		DMA_GOT_REPLY,
		DMA_TIMEOUT
	};

	maple_device *devices[4];

	sh4_device *cpu;
	emu_timer *timer;

	UINT32 mdstar, mden, mdst, msys;

	UINT32 dma_state, dma_adr, dma_port, dma_dest;
	bool dma_endflag;

	void dma_step();

	const char *maincpu_tag;
};

extern const device_type MAPLE_DC;

#endif /* __MAPLE_DC_H__ */
