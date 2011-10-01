#include "jvs13551.h"

const device_type SEGA_837_13551 = &device_creator<sega_837_13551>;

WRITE_LINE_MEMBER(sega_837_13551::jvs13551_coin_1_w)
{
	if(state)
		inc_coin(0);
}

WRITE_LINE_MEMBER(sega_837_13551::jvs13551_coin_2_w)
{
	if(state)
		inc_coin(1);
}

static INPUT_PORTS_START(sega_837_13551_coins)
	PORT_START("COINS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, sega_837_13551, jvs13551_coin_1_w)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, sega_837_13551, jvs13551_coin_2_w)
INPUT_PORTS_END

void sega_837_13551::static_set_port_tag(device_t &device, int port, const char *tag)
{
	sega_837_13551 &ctrl = downcast<sega_837_13551 &>(device);
	ctrl.port_tag[port] = tag;
}

ioport_constructor sega_837_13551::device_input_ports() const
{
	return INPUT_PORTS_NAME(sega_837_13551_coins);
}

sega_837_13551::sega_837_13551(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) : jvs_device(mconfig, SEGA_837_13551, "SEGA-837-13551", tag, owner, clock)
{
	memset(port_tag, 0, sizeof(port_tag));
}

const char *sega_837_13551::device_id()
{
	return "SEGA ENTERPRISES,LTD.;I/O BD JVS;837-13551";
}

UINT8 sega_837_13551::command_format_version()
{
	return 0x11;
}

UINT8 sega_837_13551::jvs_standard_version()
{
	return 0x20;
}

UINT8 sega_837_13551::comm_method_version()
{
	return 0x10;
}

void sega_837_13551::device_start()
{
	jvs_device::device_start();
	save_item(NAME(coin_counter));
}

void sega_837_13551::device_reset()
{
	jvs_device::device_reset();
	coin_counter[0] = 0;
	coin_counter[1] = 0;
}

void sega_837_13551::inc_coin(int coin)
{
	coin_counter[coin]++;
	if(coin_counter[coin] == 16384)
		coin_counter[coin] = 0;
}


void sega_837_13551::function_list(UINT8 *&buf)
{
	// SW input - 2 players, 13 bits
	*buf++ = 0x01; *buf++ = 2; *buf++ = 13; *buf++ = 0;

	// Coin input - 2 slots
	*buf++ = 0x02; *buf++ = 2; *buf++ = 0; *buf++ = 0;

	// Analog input - 8 channels
	*buf++ = 0x03; *buf++ = 8; *buf++ = 16; *buf++ = 0;

	// Driver out - 6 channels
	*buf++ = 0x12; *buf++ = 6; *buf++ = 0; *buf++ = 0;
}

bool sega_837_13551::coin_counters(UINT8 *&buf, UINT8 count)
{
	if(count > 2)
		return false;

	*buf++ = coin_counter[0] >> 8; *buf++ = coin_counter[0];

	if(count > 1)
		*buf++ = coin_counter[1] >> 8; *buf++ = coin_counter[1];

	return true;
}

bool sega_837_13551::coin_add(UINT8 slot, INT32 count)
{
	if(slot < 1 || slot > 2)
		return false;

	coin_counter[slot-1] += count;

	return true;
}

bool sega_837_13551::switches(UINT8 *&buf, UINT8 count_players, UINT8 bytes_per_switch)
{
	if(count_players > 2 || bytes_per_switch > 2)
		return false;

	*buf++ = input_port_read_safe(machine(), port_tag[0], 0);
	for(int i=0; i<count_players; i++) {
		UINT32 val = input_port_read_safe(machine(), port_tag[1+i], 0);
		for(int j=0; j<bytes_per_switch; j++)
			*buf++ = val >> ((1-j) << 3);
	}
	return true;

}

bool sega_837_13551::analogs(UINT8 *&buf, UINT8 count)
{
	if(count > 8)
		return false;
	for(int i=0; i<count; i++) {
		UINT16 val = input_port_read_safe(machine(), port_tag[3+i], 0x8000);
		*buf++ = val >> 8;
		*buf++ = val;
	}
	return true;
}

bool sega_837_13551::swoutputs(UINT8 count, const UINT8 *vals)
{
	if(count > 1)
		return false;
	jvs_outputs = vals[0] & 0x3f;
	logerror("837-13551: output %02x\n", jvs_outputs);
	input_port_write_safe(machine(), port_tag[11], jvs_outputs, 0x3f);
	return true;
}

bool sega_837_13551::swoutputs(UINT8 id, UINT8 val)
{
	if(id > 6)
		return false;
	handle_output(port_tag[11], id, val);
	logerror("837-13551: output %d, %d\n", id, val);
	return true;
}

