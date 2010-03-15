/***************************************************************************

    Z80 SIO (Z8440) implementation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __Z80SIO_H__
#define __Z80SIO_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _z80sio_interface z80sio_interface;
struct _z80sio_interface
{
	void (*irq_cb)(running_device *device, int state);
	write8_device_func dtr_changed_cb;
	write8_device_func rts_changed_cb;
	write8_device_func break_changed_cb;
	write8_device_func transmit_cb;
	int (*receive_poll_cb)(running_device *device, int channel);
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_Z80SIO_ADD(_tag, _clock, _intrf) \
	MDRV_DEVICE_ADD(_tag, Z80SIO, _clock) \
	MDRV_DEVICE_CONFIG(_intrf)



/***************************************************************************
    CONTROL/DATA REGISTER READ/WRITE
***************************************************************************/

/* register access (A1=C/_D A0=B/_A) */
READ8_DEVICE_HANDLER( z80sio_cd_ba_r );
WRITE8_DEVICE_HANDLER( z80sio_cd_ba_w );

/* register access (A1=B/_A A0=C/_D) */
READ8_DEVICE_HANDLER( z80sio_ba_cd_r );
WRITE8_DEVICE_HANDLER( z80sio_ba_cd_w );

WRITE8_DEVICE_HANDLER( z80sio_c_w );
READ8_DEVICE_HANDLER( z80sio_c_r );

WRITE8_DEVICE_HANDLER( z80sio_d_w );
READ8_DEVICE_HANDLER( z80sio_d_r );


/***************************************************************************
    CONTROL LINE READ/WRITE
***************************************************************************/

READ8_DEVICE_HANDLER( z80sio_get_dtr );
READ8_DEVICE_HANDLER( z80sio_get_rts );
WRITE8_DEVICE_HANDLER( z80sio_set_cts );
WRITE8_DEVICE_HANDLER( z80sio_set_dcd );
WRITE8_DEVICE_HANDLER( z80sio_receive_data );


/* ----- device interface ----- */

#define Z80SIO DEVICE_GET_INFO_NAME(z80sio)
DEVICE_GET_INFO( z80sio );

#endif
