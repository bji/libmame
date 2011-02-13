#pragma once

#ifndef __ICS2115_H__
#define __ICS2115_H__

#include "streams.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ICS2115_ADD(_tag, _clock, _irqf) \
	MCFG_DEVICE_ADD(_tag, ICS2115, _clock) \
	MCFG_IRQ_FUNC(_irqf) \

#define MCFG_ICS2115_REPLACE(_tag, _clock, _irqf) \
	MCFG_DEVICE_REPLACE(_tag, ICS2115, _clock) \
	MCFG_IRQ_FUNC(_irqf)

#define MCFG_IRQ_FUNC(_irqf) \
	ics2115_device_config::static_set_irqf(device, _irqf); \

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct ics2115_voice {
	struct {
		INT32 left;
		UINT32 acc, start, end;
		UINT16 fc;
		UINT8 ctl, saddr;
	} osc;

	struct {
		INT32 left;
		UINT32 add;
		UINT32 start, end;
		UINT32 acc;
		UINT16 regacc;
		UINT8 incr;
		UINT8 pan, mode;
	} vol;

	union {
        struct {
            UINT8 ulaw       : 1;
            UINT8 stop		 : 1;	//stops wave + vol envelope
            UINT8 eightbit   : 1;
            UINT8 loop       : 1;
            UINT8 loop_bidir : 1;
            UINT8 irq        : 1;
            UINT8 invert     : 1;
            UINT8 irq_pending: 1;
            //IRQ on variable?
        };
        UINT8 value;
    } osc_conf;

    union {
        struct {
            UINT8 done       : 1;	//indicates ramp has stopped
            UINT8 stop		 : 1;	//stops the ramp
            UINT8 rollover   : 1;	//rollover (TODO)
            UINT8 loop       : 1;
            UINT8 loop_bidir : 1;
            UINT8 irq        : 1;	//enable IRQ generation
            UINT8 invert     : 1;	//invert direction
            UINT8 irq_pending: 1;	//(read only) IRQ pending
            //noenvelope == (done | disable)
        };
        UINT8 value;
    } vol_ctrl;

    //Possibly redundant state. => improvements of wavetable logic
    //may lead to its elimination.
    union {
        struct {
            UINT8 on         : 1;
            UINT8 ramp       : 7;       // 100 0000 = 0x40 maximum
        };
        UINT8 value;
    } state;

    bool playing();
    int update_volume_envelope();
    int update_oscillator();
    void update_ramp();
};

// ======================> ics2115_device_config

class ics2115_device_config :	public device_config, public device_config_sound_interface
{
	friend class ics2115_device;

	// construction/destruction
	ics2115_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

	// inline configuration helpers
	static void static_set_irqf(device_config *device, void (*irqf)(device_t *device, int state));

protected:
	// inline data
	void (*m_irq_func)(device_t *device, int state);
};

// ======================> ics2115_device

class ics2115_device : public device_t, public device_sound_interface
{
	friend class ics2115_device_config;

	// construction/destruction
	ics2115_device(running_machine &_machine, const ics2115_device_config &config);

public:
	static READ8_DEVICE_HANDLER(read);
    static WRITE8_DEVICE_HANDLER(write);
	//UINT8 read(offs_t offset);
	//void write(offs_t offset, UINT8 data);
    static TIMER_CALLBACK(timer_cb_0);
	static TIMER_CALLBACK(timer_cb_1);

	sound_stream *m_stream;

	static const UINT16 revision = 0x1;

protected:

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// internal callbacks
	static STREAM_UPDATE( static_stream_generate );
	virtual void stream_generate(stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// internal state
	const ics2115_device_config &m_config;

	void (*m_irq_cb)(device_t *device, int state);

	UINT8 *m_rom;
	INT16 m_ulaw[256];
	UINT16 m_volume[4096];
    static const int volume_bits = 15;

	ics2115_voice m_voice[32];
	struct {
		UINT8 scale, preset;
		emu_timer *timer;
		UINT64 period;	/* in nsec */
	} m_timer[2];

	UINT8 m_active_osc;
	UINT8 m_osc_select;
	UINT8 m_reg_select;
	UINT8 m_irq_enabled, m_irq_pending;
	bool m_irq_on;

    //Unknown variable, seems to be effected by 0x12. Further investigation
    //Required.
    UINT8 m_vmode;

	//internal register helper functions
	UINT16 reg_read();
	void reg_write(UINT8 data, bool msb);
	void recalc_timer(int timer);
    void keyon();
	void recalc_irq();

	//stream helper functions
	int fill_output(ics2115_voice& voice, stream_sample_t *outputs[2], int samples);
	stream_sample_t get_sample(ics2115_voice& voice);
};


// device type definition
extern const device_type ICS2115;

#endif /* __ICS2115_H__ */
