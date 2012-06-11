/*********************************************************************

    formats/hxcmfm_dsk.h

    HxC Floppy Emulator disk images

*********************************************************************/

#ifndef HXCMFM_DSK_H
#define HXCMFM_DSK_H

#include "flopimg.h"

class mfm_format : public floppy_image_format_t
{
public:
	mfm_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;
};

extern const floppy_format_type FLOPPY_MFM_FORMAT;

#endif /* HXCMFM_DSK_H */
