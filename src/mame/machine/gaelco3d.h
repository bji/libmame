/***************************************************************************

    Gaelco 3D serial hardware

***************************************************************************/



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_GAELCO_SERIAL_ADD(_tag, _clock, _intrf) \
	MCFG_DEVICE_ADD(_tag, GAELCO_SERIAL, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)

/* external status bits */
#define GAELCOSER_STATUS_READY			0x01
#define GAELCOSER_STATUS_RTS			0x02

/* only RTS currently understood ! */
//#define GAELCOSER_STATUS_DTR          0x04

#define GAELCOSER_EXT_STATUS_MASK		0x03

/* internal bits follow ... */
#define GAELCOSER_STATUS_IRQ_ENABLE		0x10
#define GAELCOSER_STATUS_RESET			0x20
#define GAELCOSER_STATUS_SEND			0x40



/***************************************************************************
    DEVICE INTERFACE TYPE
***************************************************************************/

typedef struct _gaelco_serial_interface gaelco_serial_interface;
struct _gaelco_serial_interface
{
	devcb_write_line irq_func;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER( gaelco_serial_status_r);
WRITE8_DEVICE_HANDLER( gaelco_serial_data_w);
READ8_DEVICE_HANDLER( gaelco_serial_data_r);
WRITE8_DEVICE_HANDLER( gaelco_serial_rts_w );
/* Set to 1 during transmit, 0 for receive */
WRITE8_DEVICE_HANDLER( gaelco_serial_tr_w);


/* Big questions marks, related to serial i/o */

/* Not used in surfplnt, but in radikalb
 * Set at beginning of transfer sub, cleared at end
 */
WRITE8_DEVICE_HANDLER( gaelco_serial_unknown_w);


/* only used in radikalb, set at beginning of receive isr, cleared at end */
WRITE8_DEVICE_HANDLER( gaelco_serial_irq_enable );



/* ----- device interface ----- */

DECLARE_LEGACY_DEVICE(GAELCO_SERIAL, gaelco_serial);

//DEVICE_GET_INFO( gaelco_serial );
