/*********************************************************************

    formats/pc_dsk.c

    PC disk images

*********************************************************************/

#include <string.h>

#include "formats/pc_dsk.h"
#include "formats/basicdsk.h"

struct pc_disk_sizes
{
	UINT32 image_size;
	int sectors;
	int heads;
};



static const struct pc_disk_sizes disk_sizes[] =
{
	{ 8*1*40*512,  8, 1},	/* 5 1/4 inch double density single sided */
	{ 8*2*40*512,  8, 2},	/* 5 1/4 inch double density */
	{ 9*1*40*512,  9, 1},	/* 5 1/4 inch double density single sided */
	{ 9*2*40*512,  9, 2},	/* 5 1/4 inch double density */
	{10*2*40*512, 10, 2},	/* 5 1/4 inch double density single sided */
	{ 9*2*80*512,  9, 2},	/* 80 tracks 5 1/4 inch drives rare in PCs */
	{ 9*2*80*512,  9, 2},	/* 3 1/2 inch double density */
	{15*2*80*512, 15, 2},	/* 5 1/4 inch high density (or japanese 3 1/2 inch high density) */
	{18*2*80*512, 18, 2},	/* 3 1/2 inch high density */
	{21*2*80*512, 21, 2},	/* 3 1/2 inch high density DMF */
	{36*2*80*512, 36, 2}	/* 3 1/2 inch enhanced density */
};



static floperr_t pc_dsk_compute_geometry(floppy_image_legacy *floppy, struct basicdsk_geometry *geometry)
{
	int i;
	UINT64 size;

	memset(geometry, 0, sizeof(*geometry));
	size = floppy_image_size(floppy);

	for (i = 0; i < ARRAY_LENGTH(disk_sizes); i++)
	{
		if (disk_sizes[i].image_size == size)
		{
			geometry->sectors = disk_sizes[i].sectors;
			geometry->heads = disk_sizes[i].heads;
			geometry->sector_length = 512;
			geometry->first_sector_id = 1;
			geometry->tracks = (int) (size / disk_sizes[i].sectors / disk_sizes[i].heads / geometry->sector_length);
			return FLOPPY_ERROR_SUCCESS;
		}
	}

	if (size >= 0x1a)
	{
		/*
         * get info from boot sector.
         * not correct on all disks
         */
		UINT8 scl, spt, heads;
		floppy_image_read(floppy, &scl, 0x0c, 1);
		floppy_image_read(floppy, &spt, 0x18, 1);
		floppy_image_read(floppy, &heads, 0x1A, 1);

		if (size == ((UINT64) scl) * spt * heads * 0x200)
		{
			geometry->sectors = spt;
			geometry->heads = heads;
			geometry->sector_length = 512;
			geometry->first_sector_id = 1;
			geometry->tracks = scl;
			return FLOPPY_ERROR_SUCCESS;
		}
	}

	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_IDENTIFY(pc_dsk_identify)
{
	floperr_t err;
	struct basicdsk_geometry geometry;

	err = pc_dsk_compute_geometry(floppy, &geometry);
	if (err)
		return err;

	*vote = geometry.heads ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_CONSTRUCT(pc_dsk_construct)
{
	floperr_t err;
	struct basicdsk_geometry geometry;

	if (params)
	{
		/* create */
		memset(&geometry, 0, sizeof(geometry));
		geometry.heads = option_resolution_lookup_int(params, PARAM_HEADS);
		geometry.tracks = option_resolution_lookup_int(params, PARAM_TRACKS);
		geometry.sectors = option_resolution_lookup_int(params, PARAM_SECTORS);
		geometry.first_sector_id = 1;
		geometry.sector_length = 512;
	}
	else
	{
		/* open */
		err = pc_dsk_compute_geometry(floppy, &geometry);
		if (err)
			return err;
	}

	return basicdsk_construct(floppy, &geometry);
}



/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( pc )
	LEGACY_FLOPPY_OPTION( pc_dsk, "dsk,ima,img,ufi,360",		"PC floppy disk image",	pc_dsk_identify, pc_dsk_construct, NULL,
		HEADS([1]-2)
		TRACKS(40/[80])
		SECTORS(8/[9]/10/15/18/36))
LEGACY_FLOPPY_OPTIONS_END


const floppy_image_format_t::desc_e pc_format::pc_desc[] = {
	{ MFM, 0x4e, 80 },
	{ MFM, 0x00, 12 },
	{ RAW, 0x5224, 3 },
	{ MFM, 0xfc, 1 },
	{ MFM, 0x4e, 50 },
	{ MFM, 0x00, 12 },
	{ SECTOR_LOOP_START, 0, 17 },
	{   CRC_CCITT_START, 1 },
	{     RAW, 0x4489, 3 },
	{     MFM, 0xfe, 1 },
	{     TRACK_ID },
	{     HEAD_ID },
	{     SECTOR_ID },
	{     SIZE_ID },
	{   CRC_END, 1 },
	{   CRC, 1 },
	{   MFM, 0x4e, 22 },
	{   MFM, 0x00, 12 },
	{   CRC_CCITT_START, 2 },
	{     RAW, 0x4489, 3 },
	{     MFM, 0xfb, 1 },
	{     SECTOR_DATA, -1 },
	{   CRC_END, 2 },
	{   CRC, 2 },
	{   MFM, 0x4e, 84 },
	{   MFM, 0x00, 12 },
	{ SECTOR_LOOP_END },
	{ MFM, 0x4e, 498 },
	{ END }
};

pc_format::pc_format()
{
}

const char *pc_format::name() const
{
	return "pc";
}

const char *pc_format::description() const
{
	return "PC floppy disk image";
}

const char *pc_format::extensions() const
{
	return "dsk,ima,img,ufi,360";
}

bool pc_format::supports_save() const
{
	return true;
}

void pc_format::find_size(io_generic *io, UINT32 form_factor, int &track_count, int &head_count, int &sector_count)
{
	int size = io_generic_size(io);
	if(size == 512*18*2*80) {
		track_count = 80;
		head_count = 2;
		sector_count = 18;
	} else
		track_count = head_count = sector_count = 0;
}

int pc_format::identify(io_generic *io, UINT32 form_factor)
{
	int track_count, head_count, sector_count;
	find_size(io, form_factor, track_count, head_count, sector_count);

	if(track_count)
		return 50;
	return 0;
}

bool pc_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int track_count, head_count, sector_count;
	find_size(io, form_factor, track_count, head_count, sector_count);

	UINT8 sectdata[36*512];
	desc_s sectors[36];
	for(int i=0; i<sector_count; i++) {
		sectors[i].data = sectdata + 512*i;
		sectors[i].size = 512;
		sectors[i].sector_id = i + 1;
	}

	int track_size = sector_count*512;
	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			io_generic_read(io, sectdata, (track*head_count + head)*track_size, track_size);
			generate_track(pc_desc,
						   track, head, sectors, sector_count, 200000, image);
		}
	}

	image->set_variant(floppy_image::DSHD);

	return true;
}

bool pc_format::save(io_generic *io, floppy_image *image)
{
	int track_count, head_count, sector_count;
	get_geometry_mfm_pc(image, 1000, track_count, head_count, sector_count);

	if(track_count < 80)
		track_count = 80;
	else if(track_count > 82)
		track_count = 82;

	// Happens for a fully unformatted floppy
	if(!head_count)
		head_count = 1;

	if(sector_count > 36)
		sector_count = 36;
	else if(sector_count < 9)
		sector_count = 9;

	UINT8 sectdata[36*512];
	int track_size = sector_count*512;

	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			get_track_data_mfm_pc(track, head, image, 1000, 512, sector_count, sectdata);
			io_generic_write(io, sectdata, (track*head_count + head)*track_size, track_size);
		}
	}

	return true;
}

const floppy_format_type FLOPPY_PC_FORMAT = &floppy_image_format_creator<pc_format>;
