/***************************************************************************

    audit.c

    ROM set auditing functions.

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

#include "emu.h"
#include "emuopts.h"
#include "audit.h"
#include "harddisk.h"
#include "sound/samples.h"


//**************************************************************************
//  CORE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  media_auditor - constructor
//-------------------------------------------------

media_auditor::media_auditor(const driver_enumerator &enumerator)
	: m_enumerator(enumerator),
	  m_validation(AUDIT_VALIDATE_FULL),
	  m_searchpath(NULL)
{
}


//-------------------------------------------------
//  audit_media - audit the media described by the
//  currently-enumerated driver
//-------------------------------------------------

media_auditor::summary media_auditor::audit_media(const char *validation)
{
	// start fresh
	m_record_list.reset();

	// store validation for later
	m_validation = validation;

// temporary hack until romload is update: get the driver path and support it for
// all searches
const char *driverpath = m_enumerator.config().root_device().searchpath();

	int found = 0;
	int required = 0;
	int shared_found = 0;
	int shared_required = 0;

	// iterate over devices and regions
	device_iterator deviter(m_enumerator.config().root_device());
	for (device_t *device = deviter.first(); device != NULL; device = deviter.next())
	{
		// determine the search path for this source and iterate through the regions
		m_searchpath = device->searchpath();

		// now iterate over regions and ROMs within
		for (const rom_entry *region = rom_first_region(*device); region != NULL; region = rom_next_region(region))
		{
// temporary hack: add the driver path & region name
astring combinedpath(device->searchpath(), ";", driverpath);
if (device->shortname())
	combinedpath.cat(";").cat(device->shortname());
m_searchpath = combinedpath;

			for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			{
				hash_collection hashes(ROM_GETHASHDATA(rom));
				device_t *shared_device = find_shared_device(*device, hashes, ROM_GETLENGTH(rom));

				// count the number of files with hashes
				if (!hashes.flag(hash_collection::FLAG_NO_DUMP) && !ROM_ISOPTIONAL(rom))
				{
					required++;
					if (shared_device != NULL)
						shared_required++;
				}

				// audit a file
				audit_record *record = NULL;
				if (ROMREGION_ISROMDATA(region))
					record = audit_one_rom(rom);

				// audit a disk
				else if (ROMREGION_ISDISKDATA(region))
					record = audit_one_disk(rom);

				if (record != NULL)
				{
					// count the number of files that are found.
					if (record->status() == audit_record::STATUS_GOOD || (record->status() == audit_record::STATUS_FOUND_INVALID && find_shared_device(*device, record->actual_hashes(), record->actual_length()) == NULL))
					{
						found++;
						if (shared_device != NULL)
							shared_found++;
					}

					record->set_shared_device(shared_device);
				}
			}
		}
	}

	// if we only find files that are in the parent & either the set has no unique files or the parent is not found, then assume we don't have the set at all
	if (found == shared_found && required > 0 && (required != shared_required || shared_found == 0))
	{
		m_record_list.reset();
		return NOTFOUND;
	}

	// return a summary
	return summarize(m_enumerator.driver().name);
}


//-------------------------------------------------
//  audit_device - audit the device
//-------------------------------------------------

media_auditor::summary media_auditor::audit_device(device_t *device, const char *validation)
{
	// start fresh
	m_record_list.reset();

	// store validation for later
	m_validation = validation;
	m_searchpath = device->shortname();

	int found = 0;
	int required = 0;

	// now iterate over regions and ROMs within
	for (const rom_entry *region = rom_first_region(*device); region != NULL; region = rom_next_region(region))
	{
		for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
		{
			hash_collection hashes(ROM_GETHASHDATA(rom));

			// count the number of files with hashes
			if (!hashes.flag(hash_collection::FLAG_NO_DUMP) && !ROM_ISOPTIONAL(rom))
			{
				required++;
			}

			// audit a file
			audit_record *record = NULL;
			if (ROMREGION_ISROMDATA(region))
				record = audit_one_rom(rom);

			// audit a disk
			else if (ROMREGION_ISDISKDATA(region))
				record = audit_one_disk(rom);

			// count the number of files that are found.
			if (record != NULL && (record->status() == audit_record::STATUS_GOOD || record->status() == audit_record::STATUS_FOUND_INVALID))
			{
				found++;
			}
		}
	}

	if (found == 0 && required > 0)
	{
		m_record_list.reset();
		return NOTFOUND;
	}

	// return a summary
	return summarize(device->shortname());
}


//-------------------------------------------------
//  audit_samples - validate the samples for the
//  currently-enumerated driver
//-------------------------------------------------

media_auditor::summary media_auditor::audit_samples()
{
	// start fresh
	m_record_list.reset();

	int required = 0;
	int found = 0;

	// iterate over sample entries
	samples_device_iterator iter(m_enumerator.config().root_device());
	for (samples_device *device = iter.first(); device != NULL; device = iter.next())
	{
		// by default we just search using the driver name
		astring searchpath(m_enumerator.driver().name);

		// add the alternate path if present
		samples_iterator iter(*device);
		if (iter.altbasename() != NULL)
			searchpath.cat(";").cat(iter.altbasename());

		// iterate over samples in this entry
		for (const char *samplename = iter.first(); samplename != NULL; samplename = iter.next())
		{
			required++;

			// create a new record
			audit_record &record = m_record_list.append(*global_alloc(audit_record(samplename, audit_record::MEDIA_SAMPLE)));

			// look for the files
			emu_file file(m_enumerator.options().sample_path(), OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD);
			path_iterator path(searchpath);
			astring curpath;
			while (path.next(curpath, samplename))
			{
				// attempt to access the file (.flac) or (.wav)
				file_error filerr = file.open(curpath, ".flac");
				if (filerr != FILERR_NONE)
					filerr = file.open(curpath, ".wav");

				if (filerr == FILERR_NONE)
				{
					record.set_status(audit_record::STATUS_GOOD, audit_record::SUBSTATUS_GOOD);
					found++;
				}
				else
					record.set_status(audit_record::STATUS_NOT_FOUND, audit_record::SUBSTATUS_NOT_FOUND);
			}
		}
	}

	if (found == 0 && required > 0)
	{
		m_record_list.reset();
		return NOTFOUND;
	}

	// return a summary
	return summarize(m_enumerator.driver().name);
}


//-------------------------------------------------
//  summary - generate a summary, with an optional
//  string format
//-------------------------------------------------

media_auditor::summary media_auditor::summarize(const char *name, astring *string)
{
	if (m_record_list.count() == 0)
	{
		return NONE_NEEDED;
	}

	// loop over records
	summary overall_status = CORRECT;
	for (audit_record *record = m_record_list.first(); record != NULL; record = record->next())
	{
		summary best_new_status = INCORRECT;

		// skip anything that's fine
		if (record->substatus() == audit_record::SUBSTATUS_GOOD)
			continue;

		// output the game name, file name, and length (if applicable)
		if (string != NULL)
		{
			string->catprintf("%-12s: %s", name, record->name());
			if (record->expected_length() > 0)
				string->catprintf(" (%d bytes)", record->expected_length());
			string->catprintf(" - ");
		}

		// use the substatus for finer details
		switch (record->substatus())
		{
			case audit_record::SUBSTATUS_GOOD_NEEDS_REDUMP:
				if (string != NULL) string->catprintf("NEEDS REDUMP\n");
				best_new_status = BEST_AVAILABLE;
				break;

			case audit_record::SUBSTATUS_FOUND_NODUMP:
				if (string != NULL) string->catprintf("NO GOOD DUMP KNOWN\n");
				best_new_status = BEST_AVAILABLE;
				break;

			case audit_record::SUBSTATUS_FOUND_BAD_CHECKSUM:
				if (string != NULL)
				{
					astring tempstr;
					string->catprintf("INCORRECT CHECKSUM:\n");
					string->catprintf("EXPECTED: %s\n", record->expected_hashes().macro_string(tempstr));
					string->catprintf("   FOUND: %s\n", record->actual_hashes().macro_string(tempstr));
				}
				break;

			case audit_record::SUBSTATUS_FOUND_WRONG_LENGTH:
				if (string != NULL) string->catprintf("INCORRECT LENGTH: %d bytes\n", record->actual_length());
				break;

			case audit_record::SUBSTATUS_NOT_FOUND:
				if (string != NULL)
				{
					device_t *shared_device = record->shared_device();
					if (shared_device == NULL)
						string->catprintf("NOT FOUND\n");
					else
						string->catprintf("NOT FOUND (%s)\n", shared_device->shortname());
				}
				break;

			case audit_record::SUBSTATUS_NOT_FOUND_NODUMP:
				if (string != NULL) string->catprintf("NOT FOUND - NO GOOD DUMP KNOWN\n");
				best_new_status = BEST_AVAILABLE;
				break;

			case audit_record::SUBSTATUS_NOT_FOUND_OPTIONAL:
				if (string != NULL) string->catprintf("NOT FOUND BUT OPTIONAL\n");
				best_new_status = BEST_AVAILABLE;
				break;

			default:
				assert(false);
		}

		// downgrade the overall status if necessary
		overall_status = MAX(overall_status, best_new_status);
	}
	return overall_status;
}


//-------------------------------------------------
//  audit_one_rom - validate a single ROM entry
//-------------------------------------------------

audit_record *media_auditor::audit_one_rom(const rom_entry *rom)
{
	// allocate and append a new record
	audit_record &record = m_record_list.append(*global_alloc(audit_record(*rom, audit_record::MEDIA_ROM)));

	// see if we have a CRC and extract it if so
	UINT32 crc = 0;
	bool has_crc = record.expected_hashes().crc(crc);

	// find the file and checksum it, getting the file length along the way
	emu_file file(m_enumerator.options().media_path(), OPEN_FLAG_READ | OPEN_FLAG_NO_PRELOAD);
	path_iterator path(m_searchpath);
	astring curpath;
	while (path.next(curpath, record.name()))
	{
		// open the file if we can
		file_error filerr;
		if (has_crc)
			filerr = file.open(curpath, crc);
		else
			filerr = file.open(curpath);

		// if it worked, get the actual length and hashes, then stop
		if (filerr == FILERR_NONE)
		{
			record.set_actual(file.hashes(m_validation), file.size());
			break;
		}
	}

	// compute the final status
	compute_status(record, rom, record.actual_length() != 0);
	return &record;
}


//-------------------------------------------------
//  audit_one_disk - validate a single disk entry
//-------------------------------------------------

audit_record *media_auditor::audit_one_disk(const rom_entry *rom)
{
	// allocate and append a new record
	audit_record &record = m_record_list.append(*global_alloc(audit_record(*rom, audit_record::MEDIA_DISK)));

	// open the disk
	chd_file source;
	chd_error err = chd_error(open_disk_image(m_enumerator.options(), &m_enumerator.driver(), rom, source, NULL));

	// if we succeeded, get the hashes
	if (err == CHDERR_NONE)
	{
		hash_collection hashes;

		// if there's a SHA1 hash, add them to the output hash
		if (source.sha1() != sha1_t::null)
			hashes.add_sha1(source.sha1());

		// update the actual values
		record.set_actual(hashes);
	}

	// compute the final status
	compute_status(record, rom, err == CHDERR_NONE);
	return &record;
}


//-------------------------------------------------
//  compute_status - compute a detailed status
//  based on the information we have
//-------------------------------------------------

void media_auditor::compute_status(audit_record &record, const rom_entry *rom, bool found)
{
	// if not found, provide more details
	if (!found)
	{
		// no good dump
		if (record.expected_hashes().flag(hash_collection::FLAG_NO_DUMP))
			record.set_status(audit_record::STATUS_NOT_FOUND, audit_record::SUBSTATUS_NOT_FOUND_NODUMP);

		// optional ROM
		else if (ROM_ISOPTIONAL(rom))
			record.set_status(audit_record::STATUS_NOT_FOUND, audit_record::SUBSTATUS_NOT_FOUND_OPTIONAL);

		// just plain old not found
		else
			record.set_status(audit_record::STATUS_NOT_FOUND, audit_record::SUBSTATUS_NOT_FOUND);
	}

	// if found, provide more details
	else
	{
		// length mismatch
		if (record.expected_length() != record.actual_length())
			record.set_status(audit_record::STATUS_FOUND_INVALID, audit_record::SUBSTATUS_FOUND_WRONG_LENGTH);

		// found but needs a dump
		else if (record.expected_hashes().flag(hash_collection::FLAG_NO_DUMP))
			record.set_status(audit_record::STATUS_GOOD, audit_record::SUBSTATUS_FOUND_NODUMP);

		// incorrect hash
		else if (record.expected_hashes() != record.actual_hashes())
			record.set_status(audit_record::STATUS_FOUND_INVALID, audit_record::SUBSTATUS_FOUND_BAD_CHECKSUM);

		// correct hash but needs a redump
		else if (record.expected_hashes().flag(hash_collection::FLAG_BAD_DUMP))
			record.set_status(audit_record::STATUS_GOOD, audit_record::SUBSTATUS_GOOD_NEEDS_REDUMP);

		// just plain old good
		else
			record.set_status(audit_record::STATUS_GOOD, audit_record::SUBSTATUS_GOOD);
	}
}


//-------------------------------------------------
//  find_shared_device - return the source that
//  shares a media entry with the same hashes
//-------------------------------------------------

device_t *media_auditor::find_shared_device(device_t &device, const hash_collection &romhashes, UINT64 romlength)
{
	// doesn't apply to NO_DUMP items
	if (romhashes.flag(hash_collection::FLAG_NO_DUMP))
		return NULL;

	// special case for non-root devices
	device_t *highest_device = NULL;
	if (device.owner() != NULL)
	{
		for (const rom_entry *region = rom_first_region(device); region != NULL; region = rom_next_region(region))
			for (const rom_entry *rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))
				if (ROM_GETLENGTH(rom) == romlength)
				{
					hash_collection hashes(ROM_GETHASHDATA(rom));
					if (hashes == romhashes)
						highest_device = &device;
				}
	}
	else
	{
		// iterate up the parent chain
		for (int drvindex = m_enumerator.find(m_enumerator.driver().parent); drvindex != -1; drvindex = m_enumerator.find(m_enumerator.driver(drvindex).parent))
		{
			device_iterator deviter(m_enumerator.config(drvindex).root_device());
			for (device_t *scandevice = deviter.first(); scandevice != NULL; scandevice = deviter.next())
				for (const rom_entry *region = rom_first_region(*scandevice); region; region = rom_next_region(region))
					for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
						if (ROM_GETLENGTH(rom) == romlength)
						{
							hash_collection hashes(ROM_GETHASHDATA(rom));
							if (hashes == romhashes)
								highest_device = scandevice;
						}
		}
	}

	return highest_device;
}


//-------------------------------------------------
//  audit_record - constructor
//-------------------------------------------------

audit_record::audit_record(const rom_entry &media, media_type type)
	: m_next(NULL),
	  m_type(type),
	  m_status(STATUS_ERROR),
	  m_substatus(SUBSTATUS_ERROR),
	  m_name(ROM_GETNAME(&media)),
	  m_explength(rom_file_size(&media)),
	  m_length(0),
	  m_shared_device(NULL)
{
	m_exphashes.from_internal_string(ROM_GETHASHDATA(&media));
}

audit_record::audit_record(const char *name, media_type type)
	: m_next(NULL),
	  m_type(type),
	  m_status(STATUS_ERROR),
	  m_substatus(SUBSTATUS_ERROR),
	  m_name(name),
	  m_explength(0),
	  m_length(0),
	  m_shared_device(NULL)
{
}
