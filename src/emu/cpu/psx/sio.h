/*
 * PlayStation Serial I/O emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#pragma once

#ifndef __PSXSIO_H__
#define __PSXSIO_H__

#include "emu.h"

extern const device_type PSX_SIO;

typedef void ( *psx_sio_handler )( running_machine &, int );

#define PSX_SIO_OUT_DATA ( 1 )	/* COMMAND */
#define PSX_SIO_OUT_DTR ( 2 )	/* ATT */
#define PSX_SIO_OUT_RTS ( 4 )
#define PSX_SIO_OUT_CLOCK ( 8 )	/* CLOCK */
#define PSX_SIO_IN_DATA ( 1 )	/* DATA */
#define PSX_SIO_IN_DSR ( 2 )	/* ACK */
#define PSX_SIO_IN_CTS ( 4 )

#define SIO_BUF_SIZE ( 8 )

#define SIO_STATUS_TX_RDY ( 1 << 0 )
#define SIO_STATUS_RX_RDY ( 1 << 1 )
#define SIO_STATUS_TX_EMPTY ( 1 << 2 )
#define SIO_STATUS_OVERRUN ( 1 << 4 )
#define SIO_STATUS_DSR ( 1 << 7 )
#define SIO_STATUS_IRQ ( 1 << 9 )

#define SIO_CONTROL_TX_ENA ( 1 << 0 )
#define SIO_CONTROL_IACK ( 1 << 4 )
#define SIO_CONTROL_RESET ( 1 << 6 )
#define SIO_CONTROL_TX_IENA ( 1 << 10 )
#define SIO_CONTROL_RX_IENA ( 1 << 11 )
#define SIO_CONTROL_DSR_IENA ( 1 << 12 )
#define SIO_CONTROL_DTR ( 1 << 13 )

typedef struct _psx_sio psx_sio;
struct _psx_sio
{
	UINT32 n_status;
	UINT32 n_mode;
	UINT32 n_control;
	UINT32 n_baud;
	UINT32 n_tx;
	UINT32 n_rx;
	UINT32 n_tx_prev;
	UINT32 n_rx_prev;
	UINT32 n_tx_data;
	UINT32 n_rx_data;
	UINT32 n_tx_shift;
	UINT32 n_rx_shift;
	UINT32 n_tx_bits;
	UINT32 n_rx_bits;

	emu_timer *timer;
	psx_sio_handler fn_handler;
};

class psxsio_device : public device_t
{
public:
	psxsio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void install_handler( int n_port, psx_sio_handler p_f_sio_handler );

	WRITE32_MEMBER( write );
	READ32_MEMBER( read );

	void input( int n_port, int n_mask, int n_data );

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();

private:
	void sio_interrupt( int n_port );
	void sio_timer_adjust( int n_port );
	void sio_clock(void *ptr, int param);

	psx_sio port[2];
};

WRITE32_HANDLER( psx_sio_w );
READ32_HANDLER( psx_sio_r );

#endif
