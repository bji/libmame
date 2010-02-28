/***************************************************************************

    i8243.h

    Intel 8243 Port Expander

    Copyright Aaron Giles

***************************************************************************/

#pragma once

#ifndef __I8243_H__
#define __I8243_H__

#include "cpu/mcs48/mcs48.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _i8243_config i8243_config;
struct _i8243_config
{
	read8_device_func		readhandler;
	write8_device_func		writehandler;
};



/***************************************************************************
    MACROS
***************************************************************************/

#define I8243 DEVICE_GET_INFO_NAME( i8243 )


#define MDRV_I8243_ADD(_tag, _read, _write) \
	MDRV_DEVICE_ADD(_tag, I8243, 0) \
	MDRV_DEVICE_CONFIG_DATAPTR(i8243_config, readhandler, _read) \
	MDRV_DEVICE_CONFIG_DATAPTR(i8243_config, writehandler, _write)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( i8243 );

READ8_DEVICE_HANDLER( i8243_p2_r );
WRITE8_DEVICE_HANDLER( i8243_p2_w );

WRITE8_DEVICE_HANDLER( i8243_prog_w );


#endif  /* __I8243_H__ */
