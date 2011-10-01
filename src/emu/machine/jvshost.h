#ifndef __JVSHOST_H__
#define __JVSHOST_H__

#include "emu.h"

class jvs_device;

class jvs_host : public device_t {
public:
	jvs_host(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	void add_device(jvs_device *dev);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();


	void push(UINT8 val);
	void commit_raw();
	void commit_encoded();

	void get_raw_reply(const UINT8 *&buffer, UINT32 &size);
	void get_encoded_reply(const UINT8 *&buffer, UINT32 &size);

	bool get_presence_line();
	bool get_address_set_line();

private:
	enum { BUFFER_SIZE = 512 };

	jvs_device *first_device;

	UINT32 send_size, recv_size;
	UINT8 send_buffer[BUFFER_SIZE];
	UINT8 recv_buffer[BUFFER_SIZE];

	bool recv_is_encoded;

	void encode(UINT8 *buffer, UINT32 &size);
	void decode(UINT8 *buffer, UINT32 &size);
};

#endif
