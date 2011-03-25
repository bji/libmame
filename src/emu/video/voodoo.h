/***************************************************************************

    voodoo.h

    3dfx Voodoo Graphics SST-1/2 emulator.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#ifndef __VOODOO_H__
#define __VOODOO_H__

#pragma once

#include "devlegcy.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* enumeration specifying which model of Voodoo we are emulating */
enum
{
	VOODOO_1,
	VOODOO_2,
	VOODOO_BANSHEE,
	VOODOO_3,
	MAX_VOODOO_TYPES
};


#define STD_VOODOO_1_CLOCK			50000000
#define STD_VOODOO_2_CLOCK			90000000
#define STD_VOODOO_BANSHEE_CLOCK	90000000
#define STD_VOODOO_3_CLOCK			132000000


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*voodoo_vblank_func)(device_t *device, int state);
typedef void (*voodoo_stall_func)(device_t *device, int state);


typedef struct _voodoo_config voodoo_config;
struct _voodoo_config
{
	int					type;
	UINT8				fbmem;
	UINT8				tmumem0;
	UINT8				tmumem1;
	const char *		screen;
	const char *		cputag;
	voodoo_vblank_func	vblank;
	voodoo_stall_func	stall;
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_3DFX_VOODOO_ADD(_tag, _type, _clock, _fbmem, _screen) \
	MCFG_DEVICE_ADD(_tag, VOODOO_GRAPHICS, _clock) \
	MCFG_DEVICE_CONFIG_DATA32(voodoo_config, type, _type) \
	MCFG_DEVICE_CONFIG_DATA32(voodoo_config, fbmem, _fbmem) \
	MCFG_DEVICE_CONFIG_DATAPTR(voodoo_config, screen, _screen)

#define MCFG_3DFX_VOODOO_1_ADD(_tag, _clock, _fbmem, _screen) \
	MCFG_3DFX_VOODOO_ADD(_tag, VOODOO_1, _clock, _fbmem, _screen)

#define MCFG_3DFX_VOODOO_2_ADD(_tag, _clock, _fbmem, _screen) \
	MCFG_3DFX_VOODOO_ADD(_tag, VOODOO_2, _clock, _fbmem, _screen)

#define MCFG_3DFX_VOODOO_BANSHEE_ADD(_tag, _clock, _fbmem, _screen) \
	MCFG_3DFX_VOODOO_ADD(_tag, VOODOO_BANSHEE, _clock, _fbmem, _screen)

#define MCFG_3DFX_VOODOO_3_ADD(_tag, _clock, _fbmem, _screen) \
	MCFG_3DFX_VOODOO_ADD(_tag, VOODOO_3, _clock, _fbmem, _screen)

#define MCFG_3DFX_VOODOO_TMU_MEMORY(_tmu, _tmumem) \
	MCFG_DEVICE_CONFIG_DATA32(voodoo_config, tmumem##_tmu, _tmumem)

#define MCFG_3DFX_VOODOO_VBLANK(_vblank) \
	MCFG_DEVICE_CONFIG_DATAPTR(voodoo_config, vblank, _vblank)

#define MCFG_3DFX_VOODOO_STALL(_stall) \
	MCFG_DEVICE_CONFIG_DATAPTR(voodoo_config, stall, _stall)

#define MCFG_3DFX_VOODOO_CPU(_cputag) \
	MCFG_DEVICE_CONFIG_DATAPTR(voodoo_config, cputag, _cputag)

#define MCFG_3DFX_VOODOO_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int voodoo_update(device_t *device, bitmap_t *bitmap, const rectangle *cliprect);
int voodoo_get_type(device_t *device);
int voodoo_is_stalled(device_t *device);
void voodoo_set_init_enable(device_t *device, UINT32 newval);

READ32_DEVICE_HANDLER( voodoo_r );
WRITE32_DEVICE_HANDLER( voodoo_w );

READ32_DEVICE_HANDLER( banshee_r );
WRITE32_DEVICE_HANDLER( banshee_w );
READ32_DEVICE_HANDLER( banshee_fb_r );
WRITE32_DEVICE_HANDLER( banshee_fb_w );
READ32_DEVICE_HANDLER( banshee_io_r );
WRITE32_DEVICE_HANDLER( banshee_io_w );
READ32_DEVICE_HANDLER( banshee_rom_r );


/* ----- device interface ----- */

DECLARE_LEGACY_DEVICE(VOODOO_GRAPHICS, voodoo);

#endif
