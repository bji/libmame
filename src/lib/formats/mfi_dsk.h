#ifndef MFI_DSK_H
#define MFI_DSK_H

#include "flopimg.h"

class mfi_format : public floppy_image_format_t
{
public:
	mfi_format();

	virtual int identify(io_generic *io);
	virtual bool load(io_generic *io, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;

private:
	enum {
		TIME_MASK = 0x0fffffff,
		MG_MASK   = 0xf0000000,
		MG_SHIFT  = 28,

		MG_A      = (0 << MG_SHIFT),
		MG_B      = (1 << MG_SHIFT),
		MG_N      = (2 << MG_SHIFT),
		MG_D      = (3 << MG_SHIFT)
	};

	static const char sign[16];

	struct header {
		char sign[16];
		unsigned int cyl_count, head_count;
	};

	struct entry {
		unsigned int offset, compressed_size, uncompressed_size;
	};
};

extern const floppy_format_type FLOPPY_MFI_FORMAT;

#endif /* MFI_DSK_H */
