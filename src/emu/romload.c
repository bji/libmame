/*********************************************************************

    romload.c

    ROM loading functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "hash.h"
#include "png.h"
#include "harddisk.h"
#include "config.h"
#include "ui.h"


#define LOG_LOAD 0
#define LOG(x) do { if (LOG_LOAD) debugload x; } while(0)


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TEMPBUFFER_MAX_SIZE		(1024 * 1024 * 1024)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _open_chd open_chd;
struct _open_chd
{
	open_chd *			next;					/* pointer to next in the list */
	const char *		region;					/* disk region we came from */
	chd_file *			origchd;				/* handle to the original CHD */
	emu_file *			origfile;				/* file handle to the original CHD file */
	chd_file *			diffchd;				/* handle to the diff CHD */
	emu_file *			difffile;				/* file handle to the diff CHD file */
};


typedef struct _romload_private rom_load_data;
struct _romload_private
{
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

	running_machine *m_machine;			/* machine object where needed */
	int				system_bios;		/* the system BIOS we wish to load */

	int				warnings;			/* warning count during processing */
	int				knownbad;			/* BAD_DUMP/NO_DUMP count during processing */
	int				errors;				/* error count during processing */

	int				romsloaded;			/* current ROMs loaded count */
	int				romstotal;			/* total number of ROMs to read */
	UINT32			romsloadedsize;		/* total size of ROMs loaded so far */
	UINT32			romstotalsize;		/* total size of ROMs to read */

	emu_file *		file;				/* current file */
	open_chd *		chd_list;			/* disks */
	open_chd **		chd_list_tailptr;

	memory_region *	region;				/* info about current region */

	astring			errorstring;		/* error string */
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void rom_exit(running_machine &machine);

/***************************************************************************
    HELPERS (also used by devimage.c)
 ***************************************************************************/

file_error common_process_file(emu_options &options, const char *location, const char *ext, const rom_entry *romp, emu_file **image_file)
{
	*image_file = global_alloc(emu_file(options.media_path(), OPEN_FLAG_READ));
	file_error filerr;

	if (location != NULL && strcmp(location, "") != 0)
		filerr = (*image_file)->open(location, PATH_SEPARATOR, ROM_GETNAME(romp), ext);
	else
		filerr = (*image_file)->open(ROM_GETNAME(romp), ext);

	if (filerr != FILERR_NONE)
	{
		global_free(*image_file);
		*image_file = NULL;
	}
	return filerr;
}

file_error common_process_file(emu_options &options, const char *location, bool has_crc, UINT32 crc, const rom_entry *romp, emu_file **image_file)
{
	*image_file = global_alloc(emu_file(options.media_path(), OPEN_FLAG_READ));
	file_error filerr;

	if (has_crc)
		filerr = (*image_file)->open(location, PATH_SEPARATOR, ROM_GETNAME(romp), crc);
	else
		filerr = (*image_file)->open(location, PATH_SEPARATOR, ROM_GETNAME(romp));

	if (filerr != FILERR_NONE)
	{
		global_free(*image_file);
		*image_file = NULL;
	}
	return filerr;
}


/***************************************************************************
    HARD DISK HANDLING
***************************************************************************/

/*-------------------------------------------------
    get_disk_handle - return a pointer to the
    CHD file associated with the given region
-------------------------------------------------*/

chd_file *get_disk_handle(running_machine &machine, const char *region)
{
	open_chd *curdisk;

	for (curdisk = machine.romload_data->chd_list; curdisk != NULL; curdisk = curdisk->next)
		if (strcmp(curdisk->region, region) == 0)
			return (curdisk->diffchd != NULL) ? curdisk->diffchd : curdisk->origchd;
	return NULL;
}


/*-------------------------------------------------
    add_disk_handle - add a disk to the to the
    list of CHD files
-------------------------------------------------*/

static void add_disk_handle(running_machine &machine, open_chd *chd)
{
	romload_private *romload_data = machine.romload_data;

	*romload_data->chd_list_tailptr = auto_alloc(machine, open_chd);
	**romload_data->chd_list_tailptr = *chd;
	romload_data->chd_list_tailptr = &(*romload_data->chd_list_tailptr)->next;
}


/*-------------------------------------------------
    set_disk_handle - set a pointer to the CHD
    file associated with the given region
-------------------------------------------------*/

void set_disk_handle(running_machine &machine, const char *region, emu_file &file, chd_file &chdfile)
{
	open_chd chd = { 0 };

	/* note the region we are in */
	chd.region = region;
	chd.origchd = &chdfile;
	chd.origfile = &file;

	/* we're okay, add to the list of disks */
	add_disk_handle(machine, &chd);
}



/***************************************************************************
    ROM LOADING
***************************************************************************/

/*-------------------------------------------------
    rom_first_source - return pointer to first ROM
    source
-------------------------------------------------*/

const rom_source *rom_first_source(const machine_config &config)
{
	/* look through devices */
	for (const device_config *devconfig = config.m_devicelist.first(); devconfig != NULL; devconfig = devconfig->next())
		if (devconfig->rom_region() != NULL)
			return devconfig;

	return NULL;
}


/*-------------------------------------------------
    rom_next_source - return pointer to next ROM
    source
-------------------------------------------------*/

const rom_source *rom_next_source(const rom_source &previous)
{
	/* look for further devices with ROM definitions */
	for (const device_config *devconfig = previous.next(); devconfig != NULL; devconfig = devconfig->next())
		if (devconfig->rom_region() != NULL)
			return (rom_source *)devconfig;

	return NULL;
}


/*-------------------------------------------------
    rom_first_region - return pointer to first ROM
    region
-------------------------------------------------*/

const rom_entry *rom_first_region(const rom_source &source)
{
	const rom_entry *romp = source.rom_region();
	return (romp != NULL && !ROMENTRY_ISEND(romp)) ? romp : NULL;
}


/*-------------------------------------------------
    rom_next_region - return pointer to next ROM
    region
-------------------------------------------------*/

const rom_entry *rom_next_region(const rom_entry *romp)
{
	romp++;
	while (!ROMENTRY_ISREGIONEND(romp))
		romp++;
	return ROMENTRY_ISEND(romp) ? NULL : romp;
}


/*-------------------------------------------------
    rom_first_file - return pointer to first ROM
    file
-------------------------------------------------*/

const rom_entry *rom_first_file(const rom_entry *romp)
{
	romp++;
	while (!ROMENTRY_ISFILE(romp) && !ROMENTRY_ISREGIONEND(romp))
		romp++;
	return ROMENTRY_ISREGIONEND(romp) ? NULL : romp;
}


/*-------------------------------------------------
    rom_next_file - return pointer to next ROM
    file
-------------------------------------------------*/

const rom_entry *rom_next_file(const rom_entry *romp)
{
	romp++;
	while (!ROMENTRY_ISFILE(romp) && !ROMENTRY_ISREGIONEND(romp))
		romp++;
	return ROMENTRY_ISREGIONEND(romp) ? NULL : romp;
}


/*-------------------------------------------------
    rom_region_name - return the appropriate name
    for a rom region
-------------------------------------------------*/

astring &rom_region_name(astring &result, const game_driver *drv, const rom_source *source, const rom_entry *romp)
{
	return source->subtag(result, ROMREGION_GETTAG(romp));
}


/*-------------------------------------------------
    rom_file_size - return the expected size of a
    file given the ROM description
-------------------------------------------------*/

UINT32 rom_file_size(const rom_entry *romp)
{
	UINT32 maxlength = 0;

	/* loop until we run out of reloads */
	do
	{
		UINT32 curlength;

		/* loop until we run out of continues/ignores */
		curlength = ROM_GETLENGTH(romp++);
		while (ROMENTRY_ISCONTINUE(romp) || ROMENTRY_ISIGNORE(romp))
			curlength += ROM_GETLENGTH(romp++);

		/* track the maximum length */
		maxlength = MAX(maxlength, curlength);
	}
	while (ROMENTRY_ISRELOAD(romp));

	return maxlength;
}


/*-------------------------------------------------
    debugload - log data to a file
-------------------------------------------------*/

static void CLIB_DECL ATTR_PRINTF(1,2) debugload(const char *string, ...)
{
	static int opened;
	va_list arg;
	FILE *f;

	f = fopen("romload.log", opened++ ? "a" : "w");
	if (f)
	{
		va_start(arg, string);
		vfprintf(f, string, arg);
		va_end(arg);
		fclose(f);
	}
}


/*-------------------------------------------------
    determine_bios_rom - determine system_bios
    from SystemBios structure and OPTION_BIOS
-------------------------------------------------*/

static void determine_bios_rom(rom_load_data *romdata)
{
	const char *specbios = romdata->machine().options().bios();
	const char *defaultname = NULL;
	const rom_entry *rom;
	int default_no = 1;
	int bios_count = 0;

	romdata->system_bios = 0;

	for (const rom_source *source = rom_first_source(romdata->machine().config()); source != NULL; source = rom_next_source(*source))
	{
		/* first determine the default BIOS name */
		for (rom = source->rom_region(); !ROMENTRY_ISEND(rom); rom++)
			if (ROMENTRY_ISDEFAULT_BIOS(rom))
				defaultname = ROM_GETNAME(rom);

		/* look for a BIOS with a matching name */
		for (rom = source->rom_region(); !ROMENTRY_ISEND(rom); rom++)
			if (ROMENTRY_ISSYSTEM_BIOS(rom))
			{
				const char *biosname = ROM_GETNAME(rom);
				int bios_flags = ROM_GETBIOSFLAGS(rom);
				char bios_number[20];

				/* Allow '-bios n' to still be used */
				sprintf(bios_number, "%d", bios_flags - 1);
				if (mame_stricmp(bios_number, specbios) == 0 || mame_stricmp(biosname, specbios) == 0)
					romdata->system_bios = bios_flags;
				if (defaultname != NULL && mame_stricmp(biosname, defaultname) == 0)
					default_no = bios_flags;
				bios_count++;
			}

		/* if none found, use the default */
		if (romdata->system_bios == 0 && bios_count > 0)
		{
			/* if we got neither an empty string nor 'default' then warn the user */
			if (specbios[0] != 0 && strcmp(specbios, "default") != 0 && romdata != NULL)
			{
				romdata->errorstring.catprintf("%s: invalid bios\n", specbios);
				romdata->warnings++;
			}

			/* set to default */
			romdata->system_bios = default_no;
		}

		LOG(("Using System BIOS: %d\n", romdata->system_bios));
	}
}


/*-------------------------------------------------
    count_roms - counts the total number of ROMs
    that will need to be loaded
-------------------------------------------------*/

static void count_roms(rom_load_data *romdata)
{
	const rom_entry *region, *rom;
	const rom_source *source;

	/* start with 0 */
	romdata->romstotal = 0;
	romdata->romstotalsize = 0;

	/* loop over regions, then over files */
	for (source = rom_first_source(romdata->machine().config()); source != NULL; source = rom_next_source(*source))
		for (region = rom_first_region(*source); region != NULL; region = rom_next_region(region))
			for (rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))
				if (ROM_GETBIOSFLAGS(rom) == 0 || ROM_GETBIOSFLAGS(rom) == romdata->system_bios)
				{
					romdata->romstotal++;
					romdata->romstotalsize += rom_file_size(rom);
				}
}


/*-------------------------------------------------
    fill_random - fills an area of memory with
    random data
-------------------------------------------------*/

static void fill_random(running_machine &machine, UINT8 *base, UINT32 length)
{
	while (length--)
		*base++ = machine.rand();
}


/*-------------------------------------------------
    handle_missing_file - handles error generation
    for missing files
-------------------------------------------------*/

static void handle_missing_file(rom_load_data *romdata, const rom_entry *romp)
{
	/* optional files are okay */
	if (ROM_ISOPTIONAL(romp))
	{
		romdata->errorstring.catprintf("OPTIONAL %s NOT FOUND\n", ROM_GETNAME(romp));
		romdata->warnings++;
	}

	/* no good dumps are okay */
	else if (hash_collection(ROM_GETHASHDATA(romp)).flag(hash_collection::FLAG_NO_DUMP))
	{
		romdata->errorstring.catprintf("%s NOT FOUND (NO GOOD DUMP KNOWN)\n", ROM_GETNAME(romp));
		romdata->knownbad++;
	}

	/* anything else is bad */
	else
	{
		romdata->errorstring.catprintf("%s NOT FOUND\n", ROM_GETNAME(romp));
		romdata->errors++;
	}
}


/*-------------------------------------------------
    dump_wrong_and_correct_checksums - dump an
    error message containing the wrong and the
    correct checksums for a given ROM
-------------------------------------------------*/

static void dump_wrong_and_correct_checksums(rom_load_data *romdata, const hash_collection &hashes, const hash_collection &acthashes)
{
	astring tempstr;
	romdata->errorstring.catprintf("    EXPECTED: %s\n", hashes.macro_string(tempstr));
	romdata->errorstring.catprintf("       FOUND: %s\n", acthashes.macro_string(tempstr));

	// warn about any ill-formed hashes
	for (hash_base *hash = hashes.first(); hash != NULL; hash = hash->next())
		if (hash->parse_error())
		{
			romdata->errorstring.catprintf("\tInvalid %s checksum treated as 0 (check leading zeros)\n", hash->name());
			romdata->warnings++;
		}
}


/*-------------------------------------------------
    verify_length_and_hash - verify the length
    and hash signatures of a file
-------------------------------------------------*/

static void verify_length_and_hash(rom_load_data *romdata, const char *name, UINT32 explength, const hash_collection &hashes)
{
	/* we've already complained if there is no file */
	if (romdata->file == NULL)
		return;

	/* verify length */
	UINT32 actlength = romdata->file->size();
	if (explength != actlength)
	{
		romdata->errorstring.catprintf("%s WRONG LENGTH (expected: %08x found: %08x)\n", name, explength, actlength);
		romdata->warnings++;
	}

	/* If there is no good dump known, write it */
	astring tempstr;
	hash_collection &acthashes = romdata->file->hashes(hashes.hash_types(tempstr));
	if (hashes.flag(hash_collection::FLAG_NO_DUMP))
	{
		romdata->errorstring.catprintf("%s NO GOOD DUMP KNOWN\n", name);
		romdata->knownbad++;
	}
	/* verify checksums */
	else if (hashes != acthashes)
	{
		/* otherwise, it's just bad */
		romdata->errorstring.catprintf("%s WRONG CHECKSUMS:\n", name);
		dump_wrong_and_correct_checksums(romdata, hashes, acthashes);
		romdata->warnings++;
	}
	/* If it matches, but it is actually a bad dump, write it */
	else if (hashes.flag(hash_collection::FLAG_BAD_DUMP))
	{
		romdata->errorstring.catprintf("%s ROM NEEDS REDUMP\n",name);
		romdata->knownbad++;
	}
}


/*-------------------------------------------------
    display_loading_rom_message - display
    messages about ROM loading to the user
-------------------------------------------------*/

static void display_loading_rom_message(rom_load_data *romdata, const char *name)
{
	char buffer[200];

	// 2010-04, FP - FIXME: in MESS, load_software_part_region sometimes calls this with romstotalsize = 0!
	// as a temp workaround, I added a check for romstotalsize !=0.
	if (name != NULL && romdata->romstotalsize)
    {
        int pct_complete = (UINT32)(100 * (UINT64)romdata->romsloadedsize / (UINT64)romdata->romstotalsize);
        sprintf(buffer, "Loading (%d%%)", pct_complete);
        /* 100% complete will be reported below, so don't report it here */
        if (pct_complete != 100)
        {
            romdata->machine().announce_init_phase(STARTUP_PHASE_LOADING_ROMS,
                                                  pct_complete);
        }
    }
	else
    {
		sprintf(buffer, "Loading Complete");
        romdata->machine().announce_init_phase(STARTUP_PHASE_LOADING_ROMS, 100);
    }

	ui_set_startup_text(romdata->machine(), buffer, FALSE);
}


/*-------------------------------------------------
    display_rom_load_results - display the final
    results of ROM loading
-------------------------------------------------*/

static void display_rom_load_results(rom_load_data *romdata)
{
	/* final status display */
	display_loading_rom_message(romdata, NULL);

	/* if we had errors, they are fatal */
	if (romdata->errors != 0)
	{
		/* create the error message and exit fatally */
		mame_printf_error("%s", romdata->errorstring.cstr());
		fatalerror_exitcode(romdata->machine(), MAMERR_MISSING_FILES, "ERROR: required files are missing, the "GAMENOUN" cannot be run.");
	}

	/* if we had warnings, output them, but continue */
	if ((romdata-> warnings) || (romdata->knownbad))
	{
		romdata->errorstring.cat("WARNING: the "GAMENOUN" might not run correctly.");
		mame_printf_warning("%s\n", romdata->errorstring.cstr());
	}
}


/*-------------------------------------------------
    region_post_process - post-process a region,
    byte swapping and inverting data as necessary
-------------------------------------------------*/

static void region_post_process(rom_load_data *romdata, const char *rgntag, bool invert)
{
	const memory_region *region = romdata->machine().region(rgntag);
	UINT8 *base;
	int i, j;

	// do nothing if no region
	if (region == NULL)
		return;

	LOG(("+ datawidth=%d little=%d\n", region->width(), region->endianness() == ENDIANNESS_LITTLE));

	/* if the region is inverted, do that now */
	if (invert)
	{
		LOG(("+ Inverting region\n"));
		for (i = 0, base = region->base(); i < region->bytes(); i++)
			*base++ ^= 0xff;
	}

	/* swap the endianness if we need to */
	if (region->width() > 1 && region->endianness() != ENDIANNESS_NATIVE)
	{
		LOG(("+ Byte swapping region\n"));
		int datawidth = region->width();
		for (i = 0, base = region->base(); i < region->bytes(); i += datawidth)
		{
			UINT8 temp[8];
			memcpy(temp, base, datawidth);
			for (j = datawidth - 1; j >= 0; j--)
				*base++ = temp[j];
		}
	}
}


/*-------------------------------------------------
    open_rom_file - open a ROM file, searching
    up the parent and loading by checksum
-------------------------------------------------*/

static int open_rom_file(rom_load_data *romdata, const char *regiontag, const rom_entry *romp)
{
	file_error filerr = FILERR_NOT_FOUND;
	UINT32 romsize = rom_file_size(romp);
	const game_driver *drv;

	/* update status display */
	display_loading_rom_message(romdata, ROM_GETNAME(romp));

	/* extract CRC to use for searching */
	UINT32 crc = 0;
	bool has_crc = hash_collection(ROM_GETHASHDATA(romp)).crc(crc);

	/* attempt reading up the chain through the parents. It automatically also
     attempts any kind of load by checksum supported by the archives. */
	romdata->file = NULL;
	for (drv = &romdata->machine().system(); romdata->file == NULL && drv != NULL; drv = driver_get_clone(drv))
		if (drv->name != NULL && *drv->name != 0)
			filerr = common_process_file(romdata->machine().options(), drv->name, has_crc, crc, romp, &romdata->file);

	/* if the region is load by name, load the ROM from there */
	if (romdata->file == NULL && regiontag != NULL)
	{
		// check if we are dealing with softwarelists. if so, locationtag
		// is actually a concatenation of: listname + setname + parentname
		// separated by '%' (parentname being present only for clones)
		astring tag1(regiontag), tag2, tag3, tag4, tag5;
		bool is_list = FALSE;
		bool has_parent = FALSE;

		int separator1 = tag1.chr(0, '%');
		if (separator1 != -1)
		{
			is_list = TRUE;

			// we are loading through softlists, split the listname from the regiontag
			tag4.cpysubstr(tag1, separator1 + 1, tag1.len() - separator1 + 1);
			tag1.del(separator1, tag1.len() - separator1);
			tag1.cat(PATH_SEPARATOR);

			// check if we are loading a clone (if this is the case also tag1 have a separator '%')
			int separator2 = tag4.chr(0, '%');
			if (separator2 != -1)
			{
				has_parent = TRUE;

				// we are loading a clone through softlists, split the setname from the parentname
				tag5.cpysubstr(tag4, separator2 + 1, tag4.len() - separator2 + 1);
				tag4.del(separator2, tag4.len() - separator2);
			}

			// prepare locations where we have to load from: list/parentname & list/clonename
			astring swlist(tag1.cstr());
			tag2.cpy(swlist.cat(tag4));
			if (has_parent)
			{
				swlist.cpy(tag1);
				tag3.cpy(swlist.cat(tag5));
			}
		}

		if (tag5.chr(0, '%') != -1)
			fatalerror("We do not support clones of clones!\n");

		// try to load from the available location(s):
		// - if we are not using lists, we have regiontag only;
		// - if we are using lists, we have: list/clonename, list/parentname, clonename, parentname
		if (!is_list)
			filerr = common_process_file(romdata->machine().options(), tag1.cstr(), has_crc, crc, romp, &romdata->file);
		else
		{
			// try to load from list/setname
			if ((romdata->file == NULL) && (tag2.cstr() != NULL))
				filerr = common_process_file(romdata->machine().options(), tag2.cstr(), has_crc, crc, romp, &romdata->file);
			// try to load from list/parentname
			if ((romdata->file == NULL) && has_parent && (tag3.cstr() != NULL))
				filerr = common_process_file(romdata->machine().options(), tag3.cstr(), has_crc, crc, romp, &romdata->file);
			// try to load from setname
			if ((romdata->file == NULL) && (tag4.cstr() != NULL))
				filerr = common_process_file(romdata->machine().options(), tag4.cstr(), has_crc, crc, romp, &romdata->file);
			// try to load from parentname
			if ((romdata->file == NULL) && has_parent && (tag5.cstr() != NULL))
				filerr = common_process_file(romdata->machine().options(), tag5.cstr(), has_crc, crc, romp, &romdata->file);
		}
	}

	/* update counters */
	romdata->romsloaded++;
	romdata->romsloadedsize += romsize;

	/* return the result */
	return (filerr == FILERR_NONE);
}


/*-------------------------------------------------
    rom_fread - cheesy fread that fills with
    random data for a NULL file
-------------------------------------------------*/

static int rom_fread(rom_load_data *romdata, UINT8 *buffer, int length)
{
	/* files just pass through */
	if (romdata->file != NULL)
		return romdata->file->read(buffer, length);

	/* otherwise, fill with randomness */
	else
		fill_random(romdata->machine(), buffer, length);

	return length;
}


/*-------------------------------------------------
    read_rom_data - read ROM data for a single
    entry
-------------------------------------------------*/

static int read_rom_data(rom_load_data *romdata, const rom_entry *romp)
{
	int datashift = ROM_GETBITSHIFT(romp);
	int datamask = ((1 << ROM_GETBITWIDTH(romp)) - 1) << datashift;
	int numbytes = ROM_GETLENGTH(romp);
	int groupsize = ROM_GETGROUPSIZE(romp);
	int skip = ROM_GETSKIPCOUNT(romp);
	int reversed = ROM_ISREVERSED(romp);
	int numgroups = (numbytes + groupsize - 1) / groupsize;
	UINT8 *base = romdata->region->base() + ROM_GETOFFSET(romp);
	UINT32 tempbufsize;
	UINT8 *tempbuf;
	int i;

	LOG(("Loading ROM data: offs=%X len=%X mask=%02X group=%d skip=%d reverse=%d\n", ROM_GETOFFSET(romp), numbytes, datamask, groupsize, skip, reversed));

	/* make sure the length was an even multiple of the group size */
	if (numbytes % groupsize != 0)
		mame_printf_warning("Warning in RomModule definition: %s length not an even multiple of group size\n", ROM_GETNAME(romp));

	/* make sure we only fill within the region space */
	if (ROM_GETOFFSET(romp) + numgroups * groupsize + (numgroups - 1) * skip > romdata->region->bytes())
		fatalerror("Error in RomModule definition: %s out of memory region space\n", ROM_GETNAME(romp));

	/* make sure the length was valid */
	if (numbytes == 0)
		fatalerror("Error in RomModule definition: %s has an invalid length\n", ROM_GETNAME(romp));

	/* special case for simple loads */
	if (datamask == 0xff && (groupsize == 1 || !reversed) && skip == 0)
		return rom_fread(romdata, base, numbytes);

	/* use a temporary buffer for complex loads */
	tempbufsize = MIN(TEMPBUFFER_MAX_SIZE, numbytes);
	tempbuf = auto_alloc_array(romdata->machine(), UINT8, tempbufsize);

	/* chunky reads for complex loads */
	skip += groupsize;
	while (numbytes > 0)
	{
		int evengroupcount = (tempbufsize / groupsize) * groupsize;
		int bytesleft = (numbytes > evengroupcount) ? evengroupcount : numbytes;
		UINT8 *bufptr = tempbuf;

		/* read as much as we can */
		LOG(("  Reading %X bytes into buffer\n", bytesleft));
		if (rom_fread(romdata, bufptr, bytesleft) != bytesleft)
		{
			auto_free(romdata->machine(), tempbuf);
			return 0;
		}
		numbytes -= bytesleft;

		LOG(("  Copying to %p\n", base));

		/* unmasked cases */
		if (datamask == 0xff)
		{
			/* non-grouped data */
			if (groupsize == 1)
				for (i = 0; i < bytesleft; i++, base += skip)
					*base = *bufptr++;

			/* grouped data -- non-reversed case */
			else if (!reversed)
				while (bytesleft)
				{
					for (i = 0; i < groupsize && bytesleft; i++, bytesleft--)
						base[i] = *bufptr++;
					base += skip;
				}

			/* grouped data -- reversed case */
			else
				while (bytesleft)
				{
					for (i = groupsize - 1; i >= 0 && bytesleft; i--, bytesleft--)
						base[i] = *bufptr++;
					base += skip;
				}
		}

		/* masked cases */
		else
		{
			/* non-grouped data */
			if (groupsize == 1)
				for (i = 0; i < bytesleft; i++, base += skip)
					*base = (*base & ~datamask) | ((*bufptr++ << datashift) & datamask);

			/* grouped data -- non-reversed case */
			else if (!reversed)
				while (bytesleft)
				{
					for (i = 0; i < groupsize && bytesleft; i++, bytesleft--)
						base[i] = (base[i] & ~datamask) | ((*bufptr++ << datashift) & datamask);
					base += skip;
				}

			/* grouped data -- reversed case */
			else
				while (bytesleft)
				{
					for (i = groupsize - 1; i >= 0 && bytesleft; i--, bytesleft--)
						base[i] = (base[i] & ~datamask) | ((*bufptr++ << datashift) & datamask);
					base += skip;
				}
		}
	}
	auto_free(romdata->machine(), tempbuf);

	LOG(("  All done\n"));
	return ROM_GETLENGTH(romp);
}


/*-------------------------------------------------
    fill_rom_data - fill a region of ROM space
-------------------------------------------------*/

static void fill_rom_data(rom_load_data *romdata, const rom_entry *romp)
{
	UINT32 numbytes = ROM_GETLENGTH(romp);
	UINT8 *base = romdata->region->base() + ROM_GETOFFSET(romp);

	/* make sure we fill within the region space */
	if (ROM_GETOFFSET(romp) + numbytes > romdata->region->bytes())
		fatalerror("Error in RomModule definition: FILL out of memory region space\n");

	/* make sure the length was valid */
	if (numbytes == 0)
		fatalerror("Error in RomModule definition: FILL has an invalid length\n");

	/* fill the data (filling value is stored in place of the hashdata) */
	memset(base, (FPTR)ROM_GETHASHDATA(romp) & 0xff, numbytes);
}


/*-------------------------------------------------
    copy_rom_data - copy a region of ROM space
-------------------------------------------------*/

static void copy_rom_data(rom_load_data *romdata, const rom_entry *romp)
{
	UINT8 *base = romdata->region->base() + ROM_GETOFFSET(romp);
	const char *srcrgntag = ROM_GETNAME(romp);
	UINT32 numbytes = ROM_GETLENGTH(romp);
	UINT32 srcoffs = (FPTR)ROM_GETHASHDATA(romp);  /* srcoffset in place of hashdata */

	/* make sure we copy within the region space */
	if (ROM_GETOFFSET(romp) + numbytes > romdata->region->bytes())
		fatalerror("Error in RomModule definition: COPY out of target memory region space\n");

	/* make sure the length was valid */
	if (numbytes == 0)
		fatalerror("Error in RomModule definition: COPY has an invalid length\n");

	/* make sure the source was valid */
	const memory_region *region = romdata->machine().region(srcrgntag);
	if (region == NULL)
		fatalerror("Error in RomModule definition: COPY from an invalid region\n");

	/* make sure we find within the region space */
	if (srcoffs + numbytes > region->bytes())
		fatalerror("Error in RomModule definition: COPY out of source memory region space\n");

	/* fill the data */
	memcpy(base, region->base() + srcoffs, numbytes);
}


/*-------------------------------------------------
    process_rom_entries - process all ROM entries
    for a region
-------------------------------------------------*/

static void process_rom_entries(rom_load_data *romdata, const char *regiontag, const rom_entry *romp)
{
	UINT32 lastflags = 0;

	/* loop until we hit the end of this region */
	while (!ROMENTRY_ISREGIONEND(romp))
	{
		/* if this is a continue entry, it's invalid */
		if (ROMENTRY_ISCONTINUE(romp))
			fatalerror("Error in RomModule definition: ROM_CONTINUE not preceded by ROM_LOAD\n");

		/* if this is an ignore entry, it's invalid */
		if (ROMENTRY_ISIGNORE(romp))
			fatalerror("Error in RomModule definition: ROM_IGNORE not preceded by ROM_LOAD\n");

		/* if this is a reload entry, it's invalid */
		if (ROMENTRY_ISRELOAD(romp))
			fatalerror("Error in RomModule definition: ROM_RELOAD not preceded by ROM_LOAD\n");

		/* handle fills */
		if (ROMENTRY_ISFILL(romp))
			fill_rom_data(romdata, romp++);

		/* handle copies */
		else if (ROMENTRY_ISCOPY(romp))
			copy_rom_data(romdata, romp++);

		/* handle files */
		else if (ROMENTRY_ISFILE(romp))
		{
			int irrelevantbios = (ROM_GETBIOSFLAGS(romp) != 0 && ROM_GETBIOSFLAGS(romp) != romdata->system_bios);
			const rom_entry *baserom = romp;
			int explength = 0;

			/* open the file if it is a non-BIOS or matches the current BIOS */
			LOG(("Opening ROM file: %s\n", ROM_GETNAME(romp)));
			if (!irrelevantbios && !open_rom_file(romdata, regiontag, romp))
				handle_missing_file(romdata, romp);

			/* loop until we run out of reloads */
			do
			{
				/* loop until we run out of continues/ignores */
				do
				{
					rom_entry modified_romp = *romp++;
					//int readresult;

					/* handle flag inheritance */
					if (!ROM_INHERITSFLAGS(&modified_romp))
						lastflags = modified_romp._flags;
					else
						modified_romp._flags = (modified_romp._flags & ~ROM_INHERITEDFLAGS) | lastflags;

					explength += ROM_GETLENGTH(&modified_romp);

					/* attempt to read using the modified entry */
					if (!ROMENTRY_ISIGNORE(&modified_romp) && !irrelevantbios)
						/*readresult = */read_rom_data(romdata, &modified_romp);
				}
				while (ROMENTRY_ISCONTINUE(romp) || ROMENTRY_ISIGNORE(romp));

				/* if this was the first use of this file, verify the length and CRC */
				if (baserom)
				{
					LOG(("Verifying length (%X) and checksums\n", explength));
					verify_length_and_hash(romdata, ROM_GETNAME(baserom), explength, hash_collection(ROM_GETHASHDATA(baserom)));
					LOG(("Verify finished\n"));
				}

				/* reseek to the start and clear the baserom so we don't reverify */
				if (romdata->file != NULL)
					romdata->file->seek(0, SEEK_SET);
				baserom = NULL;
				explength = 0;
			}
			while (ROMENTRY_ISRELOAD(romp));

			/* close the file */
			if (romdata->file != NULL)
			{
				LOG(("Closing ROM file\n"));
				global_free(romdata->file);
				romdata->file = NULL;
			}
		}
		else
		{
			romp++;	/* something else; skip */
		}
	}
}


/*-------------------------------------------------
    open_disk_image - open a disk image,
    searching up the parent and loading by
    checksum
-------------------------------------------------*/

chd_error open_disk_image(emu_options &options, const game_driver *gamedrv, const rom_entry *romp, emu_file **image_file, chd_file **image_chd, const char *locationtag)
{
	const game_driver *drv, *searchdrv;
	const rom_entry *region, *rom;
	const rom_source *source;
	file_error filerr;
	chd_error err;

	*image_file = NULL;
	*image_chd = NULL;

	/* attempt to open the properly named file, scanning up through parent directories */
	filerr = FILERR_NOT_FOUND;
	for (searchdrv = gamedrv; searchdrv != NULL && filerr != FILERR_NONE; searchdrv = driver_get_clone(searchdrv))
		filerr = common_process_file(options, searchdrv->name, ".chd", romp, image_file);

	if (filerr != FILERR_NONE)
		filerr = common_process_file(options, NULL, ".chd", romp, image_file);

	/* look for the disk in the locationtag too */
	if (filerr != FILERR_NONE && locationtag != NULL)
	{
		// check if we are dealing with softwarelists. if so, locationtag
		// is actually a concatenation of: listname + setname + parentname
		// separated by '%' (parentname being present only for clones)
		astring tag1(locationtag), tag2, tag3, tag4, tag5;
		bool is_list = FALSE;
		bool has_parent = FALSE;

		int separator1 = tag1.chr(0, '%');
		if (separator1 != -1)
		{
			is_list = TRUE;

			// we are loading through softlists, split the listname from the regiontag
			tag4.cpysubstr(tag1, separator1 + 1, tag1.len() - separator1 + 1);
			tag1.del(separator1, tag1.len() - separator1);
			tag1.cat(PATH_SEPARATOR);

			// check if we are loading a clone (if this is the case also tag1 have a separator '%')
			int separator2 = tag4.chr(0, '%');
			if (separator2 != -1)
			{
				has_parent = TRUE;

				// we are loading a clone through softlists, split the setname from the parentname
				tag5.cpysubstr(tag4, separator2 + 1, tag4.len() - separator2 + 1);
				tag4.del(separator2, tag4.len() - separator2);
			}

			// prepare locations where we have to load from: list/parentname (if any) & list/clonename
			astring swlist(tag1.cstr());
			tag2.cpy(swlist.cat(tag4));
			if (has_parent)
			{
				swlist.cpy(tag1);
				tag3.cpy(swlist.cat(tag5));
			}
		}

		if (tag5.chr(0, '%') != -1)
			fatalerror("We do not support clones of clones!\n");

		// try to load from the available location(s):
		// - if we are not using lists, we have locationtag only;
		// - if we are using lists, we have: list/clonename, list/parentname, clonename, parentname
		if (!is_list)
			filerr = common_process_file(options, locationtag, ".chd", romp, image_file);
		else
		{
			// try to load from list/setname
			if ((filerr != FILERR_NONE) && (tag2.cstr() != NULL))
				filerr = common_process_file(options, tag2.cstr(), ".chd", romp, image_file);
			// try to load from list/parentname (if any)
			if ((filerr != FILERR_NONE) && has_parent && (tag3.cstr() != NULL))
				filerr = common_process_file(options, tag3.cstr(), ".chd", romp, image_file);
			// try to load from setname
			if ((filerr != FILERR_NONE) && (tag4.cstr() != NULL))
				filerr = common_process_file(options, tag4.cstr(), ".chd", romp, image_file);
			// try to load from parentname (if any)
			if ((filerr != FILERR_NONE) && has_parent && (tag5.cstr() != NULL))
				filerr = common_process_file(options, tag5.cstr(), ".chd", romp, image_file);
			// only for CHD we also try to load from list/
			if ((filerr != FILERR_NONE) && (tag1.cstr() != NULL))
			{
				tag1.del(tag1.len() - 1, 1);	// remove the PATH_SEPARATOR
				filerr = common_process_file(options, tag1.cstr(), ".chd", romp, image_file);
			}
		}
	}

	/* did the file open succeed? */
	if (filerr == FILERR_NONE)
	{
		/* try to open the CHD */
		err = chd_open_file(**image_file, CHD_OPEN_READ, NULL, image_chd);
		if (err == CHDERR_NONE)
			return err;

		/* close the file on failure */
		global_free(*image_file);
		*image_file = NULL;
	}
	else
		err = CHDERR_FILE_NOT_FOUND;

	/* otherwise, look at our parents for a CHD with an identical checksum */
	/* and try to open that */
	hash_collection romphashes(ROM_GETHASHDATA(romp));
	for (drv = gamedrv; drv != NULL; drv = driver_get_clone(drv))
	{
		machine_config config(*drv, options);
		for (source = rom_first_source(config); source != NULL; source = rom_next_source(*source))
			for (region = rom_first_region(*source); region != NULL; region = rom_next_region(region))
				if (ROMREGION_ISDISKDATA(region))
					for (rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))

						/* look for a differing name but with the same hash data */
						if (strcmp(ROM_GETNAME(romp), ROM_GETNAME(rom)) != 0 &&
							romphashes == hash_collection(ROM_GETHASHDATA(rom)))
						{
							/* attempt to open the properly named file, scanning up through parent directories */
							filerr = FILERR_NOT_FOUND;
							for (searchdrv = drv; searchdrv != NULL && filerr != FILERR_NONE; searchdrv = driver_get_clone(searchdrv))
								filerr = common_process_file(options, searchdrv->name, ".chd", rom, image_file);

							if (filerr != FILERR_NONE)
								filerr = common_process_file(options, NULL, ".chd", rom, image_file);

							/* did the file open succeed? */
							if (filerr == FILERR_NONE)
							{
								/* try to open the CHD */
								err = chd_open_file(**image_file, CHD_OPEN_READ, NULL, image_chd);
								if (err == CHDERR_NONE)
									return err;

								/* close the file on failure */
								global_free(*image_file);
								*image_file = NULL;
							}
						}
	}

	return err;
}


/*-------------------------------------------------
    open_disk_diff - open a DISK diff file
-------------------------------------------------*/

static chd_error open_disk_diff(emu_options &options, const rom_entry *romp, chd_file *source, emu_file **diff_file, chd_file **diff_chd)
{
	astring fname(ROM_GETNAME(romp), ".dif");
	chd_error err;

	*diff_file = NULL;
	*diff_chd = NULL;

	/* try to open the diff */
	LOG(("Opening differencing image file: %s\n", fname.cstr()));
	*diff_file = global_alloc(emu_file(options.diff_directory(), OPEN_FLAG_READ | OPEN_FLAG_WRITE));
	file_error filerr = (*diff_file)->open(fname);
	if (filerr != FILERR_NONE)
	{
		/* didn't work; try creating it instead */
		LOG(("Creating differencing image: %s\n", fname.cstr()));
		(*diff_file)->set_openflags(OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		filerr = (*diff_file)->open(fname);
		if (filerr != FILERR_NONE)
		{
			err = CHDERR_FILE_NOT_FOUND;
			goto done;
		}

		/* create the CHD */
		err = chd_create_file(**diff_file, 0, 0, CHDCOMPRESSION_NONE, source);
		if (err != CHDERR_NONE)
			goto done;
	}

	LOG(("Opening differencing image file: %s\n", fname.cstr()));
	err = chd_open_file(**diff_file, CHD_OPEN_READWRITE, source, diff_chd);
	if (err != CHDERR_NONE)
		goto done;

done:
	if ((err != CHDERR_NONE) && (*diff_file != NULL))
	{
		global_free(*diff_file);
		*diff_file = NULL;
	}
	return err;
}


/*-------------------------------------------------
    process_disk_entries - process all disk entries
    for a region
-------------------------------------------------*/

static void process_disk_entries(rom_load_data *romdata, const char *regiontag, const rom_entry *romp, const char *locationtag)
{
	/* loop until we hit the end of this region */
	for ( ; !ROMENTRY_ISREGIONEND(romp); romp++)
	{
		/* handle files */
		if (ROMENTRY_ISFILE(romp))
		{
			hash_collection hashes(ROM_GETHASHDATA(romp));
			open_chd chd = { 0 };
			chd_header header;
			chd_error err;

			/* note the region we are in */
			chd.region = regiontag;

			/* make the filename of the source */
			astring filename(ROM_GETNAME(romp), ".chd");

			/* first open the source drive */
			LOG(("Opening disk image: %s\n", filename.cstr()));
			err = open_disk_image(romdata->machine().options(), &romdata->machine().system(), romp, &chd.origfile, &chd.origchd, locationtag);
			if (err != CHDERR_NONE)
			{
				if (err == CHDERR_FILE_NOT_FOUND)
					romdata->errorstring.catprintf("%s NOT FOUND\n", filename.cstr());
				else
					romdata->errorstring.catprintf("%s CHD ERROR: %s\n", filename.cstr(), chd_error_string(err));

				/* if this is NO_DUMP, keep going, though the system may not be able to handle it */
				if (hashes.flag(hash_collection::FLAG_NO_DUMP))
					romdata->knownbad++;
				else if (DISK_ISOPTIONAL(romp))
					romdata->warnings++;
				else
					romdata->errors++;
				continue;
			}

			/* get the header and extract the MD5/SHA1 */
			header = *chd_get_header(chd.origchd);
			hash_collection acthashes;
			acthashes.add_from_buffer(hash_collection::HASH_SHA1, header.sha1, sizeof(header.sha1));

			/* verify the hash */
			if (hashes != acthashes)
			{
				romdata->errorstring.catprintf("%s WRONG CHECKSUMS:\n", filename.cstr());
				dump_wrong_and_correct_checksums(romdata, hashes, acthashes);
				romdata->warnings++;
			}
			else if (hashes.flag(hash_collection::FLAG_BAD_DUMP))
			{
				romdata->errorstring.catprintf("%s CHD NEEDS REDUMP\n", filename.cstr());
				romdata->knownbad++;
			}

			/* if not read-only, make the diff file */
			if (!DISK_ISREADONLY(romp))
			{
				/* try to open or create the diff */
				err = open_disk_diff(romdata->machine().options(), romp, chd.origchd, &chd.difffile, &chd.diffchd);
				if (err != CHDERR_NONE)
				{
					romdata->errorstring.catprintf("%s DIFF CHD ERROR: %s\n", filename.cstr(), chd_error_string(err));
					romdata->errors++;
					continue;
				}
			}

			/* we're okay, add to the list of disks */
			LOG(("Assigning to handle %d\n", DISK_GETINDEX(romp)));
			add_disk_handle(romdata->machine(), &chd);
		}
	}
}


/*-------------------------------------------------
    normalize_flags_for_device - modify the region
    flags for the given device
-------------------------------------------------*/

static void normalize_flags_for_device(running_machine &machine, const char *rgntag, UINT8 &width, endianness_t &endian)
{
	device_t *device = machine.device(rgntag);
	device_memory_interface *memory;
	if (device->interface(memory))
	{
		const address_space_config *spaceconfig = memory->space_config();
		if (device != NULL && spaceconfig != NULL)
		{
			int buswidth;

			/* set the endianness */
			if (spaceconfig->m_endianness == ENDIANNESS_LITTLE)
				endian = ENDIANNESS_LITTLE;
			else
				endian = ENDIANNESS_BIG;

			/* set the width */
			buswidth = spaceconfig->m_databus_width;
			if (buswidth <= 8)
				width = 1;
			else if (buswidth <= 16)
				width = 2;
			else if (buswidth <= 32)
				width = 4;
			else
				width = 8;
		}
	}
}


/*-------------------------------------------------
    load_software_part_region - load a software part

    This is used by MESS when loading a piece of
    software. The code should be merged with
    process_region_list or updated to use a slight
    more general process_region_list.
-------------------------------------------------*/

void load_software_part_region(device_t *device, char *swlist, char *swname, rom_entry *start_region)
{
	astring locationtag(swlist), breakstr("%");
	rom_load_data *romdata = device->machine().romload_data;
	const rom_entry *region;
	astring regiontag;

	// attempt reading up the chain through the parents and create a locationtag astring in the format
	// " swlist % clonename % parentname "
	// open_rom_file contains the code to split the elements and to create paths to load from

	software_list *software_list_ptr = software_list_open(device->machine().options(), swlist, FALSE, NULL);
	if (software_list_ptr)
	{
		locationtag.cat(breakstr);

		for (software_info *swinfo = software_list_find(software_list_ptr, swname, NULL); swinfo != NULL; )
		{
			if (swinfo != NULL)
			{
				astring tmp(swinfo->shortname);
				locationtag.cat(tmp);
				locationtag.cat(breakstr);
				// printf("%s\n", locationtag.cstr());
			}
			const char *parentname = software_get_clone(device->machine().options(), swlist, swinfo->shortname);
			if (parentname != NULL)
				swinfo = software_list_find(software_list_ptr, parentname, NULL);
			else
				swinfo = NULL;
		}
		// strip the final '%'
		locationtag.del(locationtag.len() - 1, 1);
		software_list_close(software_list_ptr);
	}

	/* Make sure we are passed a device */
	assert(device != NULL);

	romdata->errorstring.reset();

	if (software_get_support(device->machine().options(), swlist, swname) == SOFTWARE_SUPPORTED_PARTIAL)
	{
		romdata->errorstring.catprintf("WARNING: support for software %s (in list %s) is only partial\n", swname, swlist);
		romdata->warnings++;
	}
	if (software_get_support(device->machine().options(), swlist, swname) == SOFTWARE_SUPPORTED_NO)
	{
		romdata->errorstring.catprintf("WARNING: support for software %s (in list %s) is only preliminary\n", swname, swlist);
		romdata->warnings++;
	}

	/* loop until we hit the end */
	for (region = start_region; region != NULL; region = rom_next_region(region))
	{
		UINT32 regionlength = ROMREGION_GETLENGTH(region);

		device->subtag(regiontag, ROMREGION_GETTAG(region));
		LOG(("Processing region \"%s\" (length=%X)\n", regiontag.cstr(), regionlength));

		/* the first entry must be a region */
		assert(ROMENTRY_ISREGION(region));

		/* if this is a device region, override with the device width and endianness */
		endianness_t endianness = ROMREGION_ISBIGENDIAN(region) ? ENDIANNESS_BIG : ENDIANNESS_LITTLE;
		UINT8 width = ROMREGION_GETWIDTH(region) / 8;
		const memory_region *memregion = romdata->machine().region(regiontag);
		if (memregion != NULL)
		{
			if (romdata->machine().device(regiontag) != NULL)
				normalize_flags_for_device(romdata->machine(), regiontag, width, endianness);

			/* clear old region (todo: should be moved to an image unload function) */
			romdata->machine().region_free(memregion->name());
		}

		/* remember the base and length */
		romdata->region = romdata->machine().region_alloc(regiontag, regionlength, width, endianness);
		LOG(("Allocated %X bytes @ %p\n", romdata->region->bytes(), romdata->region->base()));

		/* clear the region if it's requested */
		if (ROMREGION_ISERASE(region))
			memset(romdata->region->base(), ROMREGION_GETERASEVAL(region), romdata->region->bytes());

		/* or if it's sufficiently small (<= 4MB) */
		else if (romdata->region->bytes() <= 0x400000)
			memset(romdata->region->base(), 0, romdata->region->bytes());

#ifdef MAME_DEBUG
		/* if we're debugging, fill region with random data to catch errors */
		else
			fill_random(romdata->machine(), romdata->region->base(), romdata->region->bytes());
#endif

		/* now process the entries in the region */
		if (ROMREGION_ISROMDATA(region))
			process_rom_entries(romdata, locationtag, region + 1);
		else if (ROMREGION_ISDISKDATA(region))
			process_disk_entries(romdata, core_strdup(regiontag.cstr()), region + 1, locationtag);
	}

	/* now go back and post-process all the regions */
	for (region = start_region; region != NULL; region = rom_next_region(region))
		region_post_process(romdata, ROMREGION_GETTAG(region), ROMREGION_ISINVERTED(region));

	/* display the results and exit */
	display_rom_load_results(romdata);
}


/*-------------------------------------------------
    process_region_list - process a region list
-------------------------------------------------*/

static void process_region_list(rom_load_data *romdata)
{
	astring regiontag;
	const rom_source *source;
	const rom_entry *region;

	/* loop until we hit the end */
	for (source = rom_first_source(romdata->machine().config()); source != NULL; source = rom_next_source(*source))
		for (region = rom_first_region(*source); region != NULL; region = rom_next_region(region))
		{
			UINT32 regionlength = ROMREGION_GETLENGTH(region);

			rom_region_name(regiontag, &romdata->machine().system(), source, region);
			LOG(("Processing region \"%s\" (length=%X)\n", regiontag.cstr(), regionlength));

			/* the first entry must be a region */
			assert(ROMENTRY_ISREGION(region));

			if (ROMREGION_ISROMDATA(region))
			{
				/* if this is a device region, override with the device width and endianness */
				UINT8 width = ROMREGION_GETWIDTH(region) / 8;
				endianness_t endianness = ROMREGION_ISBIGENDIAN(region) ? ENDIANNESS_BIG : ENDIANNESS_LITTLE;
				if (romdata->machine().device(regiontag) != NULL)
					normalize_flags_for_device(romdata->machine(), regiontag, width, endianness);

				/* remember the base and length */
				romdata->region = romdata->machine().region_alloc(regiontag, regionlength, width, endianness);
				LOG(("Allocated %X bytes @ %p\n", romdata->region->bytes(), romdata->region->base()));

				/* clear the region if it's requested */
				if (ROMREGION_ISERASE(region))
					memset(romdata->region->base(), ROMREGION_GETERASEVAL(region), romdata->region->bytes());

				/* or if it's sufficiently small (<= 4MB) */
				else if (romdata->region->bytes() <= 0x400000)
					memset(romdata->region->base(), 0, romdata->region->bytes());

#ifdef MAME_DEBUG
				/* if we're debugging, fill region with random data to catch errors */
				else
					fill_random(romdata->machine(), romdata->region->base(), romdata->region->bytes());
#endif

				/* now process the entries in the region */
				process_rom_entries(romdata, ROMREGION_ISLOADBYNAME(region) ? ROMREGION_GETTAG(region) : NULL, region + 1);
			}
			else if (ROMREGION_ISDISKDATA(region))
				process_disk_entries(romdata, ROMREGION_GETTAG(region), region + 1, NULL);
		}

	/* now go back and post-process all the regions */
	for (source = rom_first_source(romdata->machine().config()); source != NULL; source = rom_next_source(*source))
		for (region = rom_first_region(*source); region != NULL; region = rom_next_region(region))
			region_post_process(romdata, ROMREGION_GETTAG(region), ROMREGION_ISINVERTED(region));
}


/*-------------------------------------------------
    rom_init - load the ROMs and open the disk
    images associated with the given machine
-------------------------------------------------*/

void rom_init(running_machine &machine)
{
	rom_load_data *romdata;

	/* allocate private data */
	machine.romload_data = romdata = auto_alloc_clear(machine, romload_private);

	/* make sure we get called back on the way out */
	machine.add_notifier(MACHINE_NOTIFY_EXIT, rom_exit);

	/* reset the romdata struct */
	romdata->m_machine = &machine;

	/* figure out which BIOS we are using */
	determine_bios_rom(romdata);

	/* count the total number of ROMs */
	count_roms(romdata);

	/* reset the disk list */
	romdata->chd_list = NULL;
	romdata->chd_list_tailptr = &machine.romload_data->chd_list;

	/* process the ROM entries we were passed */
	process_region_list(romdata);

	/* display the results and exit */
	display_rom_load_results(romdata);
}


/*-------------------------------------------------
    rom_exit - clean up after ourselves
-------------------------------------------------*/

static void rom_exit(running_machine &machine)
{
	open_chd *curchd;

	/* close all hard drives */
	for (curchd = machine.romload_data->chd_list; curchd != NULL; curchd = curchd->next)
	{
		if (curchd->diffchd != NULL)
			chd_close(curchd->diffchd);
		if (curchd->difffile != NULL)
			global_free(curchd->difffile);
		if (curchd->origchd != NULL)
			chd_close(curchd->origchd);
		if (curchd->origfile != NULL)
			global_free(curchd->origfile);
	}
}


/*-------------------------------------------------
    rom_load_warnings - return the number of
    warnings we generated
-------------------------------------------------*/

int rom_load_warnings(running_machine &machine)
{
	return machine.romload_data->warnings;
}


/*-------------------------------------------------
    rom_load_knownbad - return the number of
    BAD_DUMP/NO_DUMP warnings we generated
-------------------------------------------------*/

int rom_load_knownbad(running_machine &machine)
{
	return machine.romload_data->knownbad;
}
