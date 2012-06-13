#ifndef NAMCO50_H
#define NAMCO50_H

#include "devlegcy.h"


#define MCFG_NAMCO_50XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NAMCO_50XX, _clock) \


READ8_DEVICE_HANDLER( namco_50xx_read );
void namco_50xx_read_request(device_t *device);
WRITE8_DEVICE_HANDLER( namco_50xx_write );


/* device get info callback */
DECLARE_LEGACY_DEVICE(NAMCO_50XX, namco_50xx);


#endif	/* NAMCO50_H */
