/***************************************************************************

    clifront.c

    Command-line interface frontend for MAME.

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
#include "hash.h"
#include "jedparse.h"
#include "audit.h"
#include "info.h"
#include "unzip.h"
#include "validity.h"
#include "sound/samples.h"
#include "clifront.h"
#include "xmlfile.h"

#include <new>
#include <ctype.h>


//**************************************************************************
//  COMMAND-LINE OPTIONS
//**************************************************************************

const options_entry cli_options::s_option_entries[] =
{
	/* core commands */
	{ NULL,                            NULL,       OPTION_HEADER,     "CORE COMMANDS" },
	{ CLICOMMAND_HELP ";h;?",           "0",       OPTION_COMMAND,    "show help message" },
	{ CLICOMMAND_VALIDATE ";valid",     "0",       OPTION_COMMAND,    "perform driver validation on all game drivers" },

	/* configuration commands */
	{ NULL,                            NULL,       OPTION_HEADER,     "CONFIGURATION COMMANDS" },
	{ CLICOMMAND_CREATECONFIG ";cc",    "0",       OPTION_COMMAND,    "create the default configuration file" },
	{ CLICOMMAND_SHOWCONFIG ";sc",      "0",       OPTION_COMMAND,    "display running parameters" },
	{ CLICOMMAND_SHOWUSAGE ";su",       "0",       OPTION_COMMAND,    "show this help" },

	/* frontend commands */
	{ NULL,                            NULL,       OPTION_HEADER,     "FRONTEND COMMANDS" },
	{ CLICOMMAND_LISTXML ";lx",         "0",       OPTION_COMMAND,    "all available info on driver in XML format" },
	{ CLICOMMAND_LISTFULL ";ll",        "0",       OPTION_COMMAND,    "short name, full name" },
	{ CLICOMMAND_LISTSOURCE ";ls",      "0",       OPTION_COMMAND,    "driver sourcefile" },
	{ CLICOMMAND_LISTCLONES ";lc",      "0",       OPTION_COMMAND,    "show clones" },
	{ CLICOMMAND_LISTBROTHERS ";lb",    "0",       OPTION_COMMAND,    "show \"brothers\", or other drivers from same sourcefile" },
	{ CLICOMMAND_LISTCRC,               "0",       OPTION_COMMAND,    "CRC-32s" },
	{ CLICOMMAND_LISTROMS,              "0",       OPTION_COMMAND,    "list required roms for a driver" },
	{ CLICOMMAND_LISTSAMPLES,           "0",       OPTION_COMMAND,    "list optional samples for a driver" },
	{ CLICOMMAND_VERIFYROMS,            "0",       OPTION_COMMAND,    "report romsets that have problems" },
	{ CLICOMMAND_VERIFYSAMPLES,         "0",       OPTION_COMMAND,    "report samplesets that have problems" },
	{ CLICOMMAND_ROMIDENT,              "0",       OPTION_COMMAND,    "compare files with known MAME roms" },
	{ CLICOMMAND_LISTDEVICES ";ld",     "0",       OPTION_COMMAND,    "list available devices" },
	{ CLICOMMAND_LISTSLOTS ";lslot",    "0",       OPTION_COMMAND,    "list available slots and slot devices" },
	{ CLICOMMAND_LISTMEDIA ";lm",       "0",       OPTION_COMMAND,    "list available media for the system" },
	{ CLICOMMAND_LISTSOFTWARE ";lsoft", "0",       OPTION_COMMAND,    "list known software for the system" },

	{ NULL }
};



//**************************************************************************
//  CLI OPTIONS
//**************************************************************************

//-------------------------------------------------
//  cli_options - constructor
//-------------------------------------------------

cli_options::cli_options()
{
	add_entries(s_option_entries);
}



//**************************************************************************
//  CLI FRONTEND
//**************************************************************************

//-------------------------------------------------
//  cli_frontend - constructor
//-------------------------------------------------

cli_frontend::cli_frontend(cli_options &options, osd_interface &osd)
	: m_options(options),
	  m_osd(osd),
	  m_result(MAMERR_NONE)
{
	// begin tracking memory
	track_memory(true);
}


//-------------------------------------------------
//  ~cli_frontend - destructor
//-------------------------------------------------

cli_frontend::~cli_frontend()
{
	// report any unfreed memory on clean exits
	track_memory(false);
	if (m_result == MAMERR_NONE)
		dump_unfreed_mem();
}


//-------------------------------------------------
//  execute - execute a game via the standard
//  command line interface
//-------------------------------------------------

int cli_frontend::execute(int argc, char **argv)
{
	// wrap the core execution in a try/catch to field all fatal errors
	m_result = MAMERR_NONE;
	try
	{
		// parse the command line, adding any system-specific options
		astring option_errors;
		if (!m_options.parse_command_line(argc, argv, option_errors))
		{
			// if we failed, check for no command and a system name first; in that case error on the name
			if (strlen(m_options.command()) == 0 && m_options.system() == NULL && strlen(m_options.system_name()) > 0)
				throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "Unknown system '%s'", m_options.system_name());

			// otherwise, error on the options
			throw emu_fatalerror(MAMERR_INVALID_CONFIG, "%s", option_errors.trimspace().cstr());
		}
		if (option_errors)
			printf("Error in command line:\n%s\n", option_errors.trimspace().cstr());

		// determine the base name of the EXE
		astring exename;
		core_filename_extract_base(&exename, argv[0], TRUE);

		// if we have a command, execute that
		if (strlen(m_options.command()) != 0)
			execute_commands(exename);

		// otherwise, check for a valid system
		else
		{
			// if we can't find it, give an appropriate error
			const game_driver *system = m_options.system();
			if (system == NULL && strlen(m_options.system_name()) > 0)
				throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "Unknown system '%s'", m_options.system_name());

			if (strlen(m_options.software_name()) > 0) {
				machine_config config(*system, m_options);
				if (!config.devicelist().first(SOFTWARE_LIST))
					throw emu_fatalerror(MAMERR_FATALERROR, "No software lists defined for this system\n");

				bool found = FALSE;
				for (device_t *swlists = config.devicelist().first(SOFTWARE_LIST); swlists != NULL; swlists = swlists->typenext())
				{
					software_list_config *swlist = (software_list_config *)downcast<const legacy_device_base *>(swlists)->inline_config();

					for (int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++)
					{
						if (swlist->list_name[i] && *swlist->list_name[i])
						{
							software_list *list = software_list_open(m_options, swlist->list_name[i], FALSE, NULL);

							if (list)
							{
								software_info *swinfo = software_list_find(list, m_options.software_name(), NULL);
								if (swinfo!=NULL) {
									for (software_part *swpart = software_find_part(swinfo, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
									{
										const char *mount = software_part_get_feature(swpart, "automount");
										if (mount==NULL || strcmp(mount,"no")!=0) {
											// loop trough all parts
											// search for a device with the right interface
											const device_image_interface *image = NULL;
											for (bool gotone = config.devicelist().first(image); gotone; gotone = image->next(image))
											{
												const char *interface = image->image_interface();
												if (interface != NULL)
												{
													if (!strcmp(interface, swpart->interface_))
													{
														const char *option = m_options.value(image->brief_instance_name());
														// mount only if not already mounted
														if (strlen(option)==0) {
															astring val;
															astring error;
															val.printf("%s:%s",m_options.software_name(),swpart->name);
															m_options.set_value(image->brief_instance_name(), val.cstr(), OPTION_PRIORITY_CMDLINE, error);
															assert(!error);
														}
														break;
													}
												}
											}
										}
									}
									software_list_close(list);
									found = TRUE;
									break;
								}

							}
							software_list_close(list);
						}
						if (found) break;
					}
					if (found) break;
				}
				if (!found) {
					software_display_matches(config.devicelist(),m_options, NULL,m_options.software_name());
					throw emu_fatalerror(MAMERR_FATALERROR, "");
				}
			}
			// otherwise just run the game
			m_result = mame_execute(m_options, m_osd, false);
		}
	}

	// handle exceptions of various types
	catch (emu_fatalerror &fatal)
	{
		astring string(fatal.string());
		fprintf(stderr, "%s\n", string.trimspace().cstr());
		m_result = (fatal.exitcode() != 0) ? fatal.exitcode() : MAMERR_FATALERROR;

		// if a game was specified, wasn't a wildcard, and our error indicates this was the
		// reason for failure, offer some suggestions
		if (m_result == MAMERR_NO_SUCH_GAME && strlen(m_options.system_name()) > 0 && strchr(m_options.system_name(), '*') == NULL && m_options.system() == NULL)
		{
			// get the top 10 approximate matches
			driver_enumerator drivlist(m_options);
			int matches[10];
			drivlist.find_approximate_matches(m_options.system_name(), ARRAY_LENGTH(matches), matches);

			// print them out
			fprintf(stderr, "\n\"%s\" approximately matches the following\n"
					"supported " GAMESNOUN " (best match first):\n\n", m_options.system_name());
			for (int matchnum = 0; matchnum < ARRAY_LENGTH(matches); matchnum++)
				if (matches[matchnum] != -1)
					fprintf(stderr, "%-18s%s\n", drivlist.driver(matches[matchnum]).name, drivlist.driver(matches[matchnum]).description);
		}
	}
	catch (emu_exception &)
	{
		fprintf(stderr, "Caught unhandled emulator exception\n");
		m_result = MAMERR_FATALERROR;
	}
	catch (std::bad_alloc &)
	{
		fprintf(stderr, "Out of memory!\n");
		m_result = MAMERR_FATALERROR;
	}

	// handle any other exceptions
	catch (...)
	{
		fprintf(stderr, "Caught unhandled exception\n");
		m_result = MAMERR_FATALERROR;
	}

	return m_result;
}


//-------------------------------------------------
//  listxml - output the XML data for one or more
//  games
//-------------------------------------------------

void cli_frontend::listxml(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// create the XML and print it to stdout
	info_xml_creator creator(drivlist);
	creator.output(stdout);
}


//-------------------------------------------------
//  listfull - output the name and description of
//  one or more games
//-------------------------------------------------

void cli_frontend::listfull(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// print the header
	mame_printf_info("Name:             Description:\n");

	// iterate through drivers and output the info
	while (drivlist.next())
		if ((drivlist.driver().flags & GAME_NO_STANDALONE) == 0)
			mame_printf_info("%-18s\"%s\"\n", drivlist.driver().name, drivlist.driver().description);
}


//-------------------------------------------------
//  listsource - output the name and source
//  filename of one or more games
//-------------------------------------------------

void cli_frontend::listsource(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// iterate through drivers and output the info
	astring filename;
	while (drivlist.next())
		mame_printf_info("%-16s %s\n", drivlist.driver().name, core_filename_extract_base(&filename, drivlist.driver().source_file, FALSE)->cstr());
}


//-------------------------------------------------
//  listclones - output the name and parent of all
//  clones matching the given pattern
//-------------------------------------------------

void cli_frontend::listclones(const char *gamename)
{
	// start with a filtered list of drivers
	driver_enumerator drivlist(m_options, gamename);
	int original_count = drivlist.count();

	// iterate through the remaining ones to see if their parent matches
	while (drivlist.next_excluded())
	{
		// if we have a non-bios clone and it matches, keep it
		int clone_of = drivlist.clone();
		if (clone_of != -1 && (drivlist.driver(clone_of).flags & GAME_IS_BIOS_ROOT) == 0)
			if (drivlist.matches(gamename, drivlist.driver(clone_of).name))
				drivlist.include();
	}

	// return an error if none found
	if (drivlist.count() == 0)
	{
		// see if we match but just weren't a clone
		if (original_count == 0)
			throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);
		else
			mame_printf_info("Found %d matches for '%s' but none were clones\n", drivlist.count(), gamename);
		return;
	}

	// print the header
	mame_printf_info("Name:            Clone of:\n");

	// iterate through drivers and output the info
	drivlist.reset();
	while (drivlist.next())
	{
		int clone_of = drivlist.clone();
		if (clone_of != -1 && (drivlist.driver(clone_of).flags & GAME_IS_BIOS_ROOT) == 0)
			mame_printf_info("%-16s %-8s\n", drivlist.driver().name, drivlist.driver(clone_of).name);
	}
}


//-------------------------------------------------
//  listbrothers - for each matching game, output
//  the list of other games that share the same
//  source file
//-------------------------------------------------

void cli_frontend::listbrothers(const char *gamename)
{
	// start with a filtered list of drivers; return an error if none found
	driver_enumerator initial_drivlist(m_options, gamename);
	if (initial_drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// for the final list, start with an empty driver list
	driver_enumerator drivlist(m_options);
	drivlist.exclude_all();

	// scan through the initially-selected drivers
	while (initial_drivlist.next())
	{
		// if we are already marked in the final list, we don't need to do anything
		if (drivlist.included(initial_drivlist.current()))
			continue;

		// otherwise, walk excluded items in the final list and mark any that match
		drivlist.reset();
		while (drivlist.next_excluded())
			if (strcmp(drivlist.driver().source_file, initial_drivlist.driver().source_file) == 0)
				drivlist.include();
	}

	// print the header
	mame_printf_info("Source file:     Name:            Parent:\n");

	// output the entries found
	drivlist.reset();
	astring filename;
	while (drivlist.next())
	{
		int clone_of = drivlist.clone();
		mame_printf_info("%-16s %-16s %-16s\n", core_filename_extract_base(&filename, drivlist.driver().source_file, FALSE)->cstr(), drivlist.driver().name, (clone_of == -1 ? "" : drivlist.driver(clone_of).name));
	}
}


//-------------------------------------------------
//  listcrc - output the CRC and name of all ROMs
//  referenced by the emulator
//-------------------------------------------------

void cli_frontend::listcrc(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// iterate through matches, and then through ROMs
	while (drivlist.next())
		for (const rom_source *source = rom_first_source(drivlist.config()); source != NULL; source = rom_next_source(*source))
			for (const rom_entry *region = rom_first_region(*source); region; region = rom_next_region(region))
				for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				{
					// if we have a CRC, display it
					UINT32 crc;
					if (hash_collection(ROM_GETHASHDATA(rom)).crc(crc))
						mame_printf_info("%08x %-16s %s\n", crc, ROM_GETNAME(rom), drivlist.driver().description);
				}
}


//-------------------------------------------------
//  listroms - output the list of ROMs referenced
//  by a given game or set of games
//-------------------------------------------------

void cli_frontend::listroms(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// iterate through matches
	astring tempstr;
	bool first = true;
	while (drivlist.next())
	{
		// print a header
		if (!first)
			mame_printf_info("\n");
		first = false;
		mame_printf_info("ROMs required for driver \"%s\".\n"
				"Name                    Size Checksum\n", drivlist.driver().name);

		// iterate through roms
		for (const rom_source *source = rom_first_source(drivlist.config()); source != NULL; source = rom_next_source(*source))
			for (const rom_entry *region = rom_first_region(*source); region; region = rom_next_region(region))
				for (const rom_entry *rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				{
					// accumulate the total length of all chunks
					int length = -1;
					if (ROMREGION_ISROMDATA(region))
						length = rom_file_size(rom);

					// start with the name
					const char *name = ROM_GETNAME(rom);
					mame_printf_info("%-20s ", name);

					// output the length next
					if (length >= 0)
						mame_printf_info("%7d", length);
					else
						mame_printf_info("       ");

					// output the hash data
					hash_collection hashes(ROM_GETHASHDATA(rom));
					if (!hashes.flag(hash_collection::FLAG_NO_DUMP))
					{
						if (hashes.flag(hash_collection::FLAG_BAD_DUMP))
							mame_printf_info(" BAD");
						mame_printf_info(" %s", hashes.macro_string(tempstr));
					}
					else
						mame_printf_info(" NO GOOD DUMP KNOWN");

					// end with a CR
					mame_printf_info("\n");
				}
	}
}


//-------------------------------------------------
//  listsamples - output the list of samples
//  referenced by a given game or set of games
//-------------------------------------------------

void cli_frontend::listsamples(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// iterate over drivers, looking for SAMPLES devices
	bool first = true;
	while (drivlist.next())
	{
		// see if we have samples
		const device_t *device;
		for (device = drivlist.config().first_device(); device != NULL; device = device->next())
			if (device->type() == SAMPLES)
				break;
		if (device == NULL)
			continue;

		// print a header
		if (!first)
			mame_printf_info("\n");
		first = false;
		mame_printf_info("Samples required for driver \"%s\".\n", drivlist.driver().name);

		// iterate over samples devices
		for ( ; device != NULL; device = device->next())
			if (device->type() == SAMPLES)
			{
				// if the list is legit, walk it and print the sample info
				const char *const *samplenames = reinterpret_cast<const samples_interface *>(device->static_config())->samplenames;
				if (samplenames != NULL)
					for (int sampnum = 0; samplenames[sampnum] != NULL; sampnum++)
						if (samplenames[sampnum][0] != '*')
							mame_printf_info("%s\n", samplenames[sampnum]);
			}
	}
}


//-------------------------------------------------
//  listdevices - output the list of devices
//  referenced by a given game or set of games
//-------------------------------------------------

void cli_frontend::listdevices(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// iterate over drivers, looking for SAMPLES devices
	bool first = true;
	while (drivlist.next())
	{
		// print a header
		if (!first)
			printf("\n");
		first = false;
		printf("Driver %s (%s):\n", drivlist.driver().name, drivlist.driver().description);

		// iterate through devices
		for (const device_t *device = drivlist.config().first_device(); device != NULL; device = device->next())
		{
			printf("   %s ('%s')", device->name(), device->tag());

			UINT32 clock = device->clock();
			if (clock >= 1000000000)
				printf(" @ %d.%02d GHz\n", clock / 1000000000, (clock / 10000000) % 100);
			else if (clock >= 1000000)
				printf(" @ %d.%02d MHz\n", clock / 1000000, (clock / 10000) % 100);
			else if (clock >= 1000)
				printf(" @ %d.%02d kHz\n", clock / 1000, (clock / 10) % 100);
			else if (clock > 0)
				printf(" @ %d Hz\n", clock);
			else
				printf("\n");
		}
	}
}


//-------------------------------------------------
//  listslots - output the list of slot devices
//  referenced by a given game or set of games
//-------------------------------------------------

void cli_frontend::listslots(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// print header
	printf(" SYSTEM      SLOT NAME             SLOT OPTIONS SUPPORTED     \n");
	printf("----------  --------------------  ------------------------------------\n");

	// iterate over drivers
	while (drivlist.next())
	{
		// iterate
		const device_slot_interface *slot = NULL;
		bool first = true;
		for (bool gotone = drivlist.config().devicelist().first(slot); gotone; gotone = slot->next(slot))
		{
			// output the line, up to the list of extensions
			printf("%-13s%-20s   ", first ? drivlist.driver().name : "", slot->device().tag());

			// get the options and print them
			const slot_interface* intf = slot->get_slot_interfaces();
			for (int i = 0; intf[i].name != NULL; i++)
			{
				if (i==0) {
					printf("%s\n", intf[i].name);
				} else {
					printf("%-33s   %s\n", "",intf[i].name);
				}
			}

			// end the line
			printf("\n");
			first = false;
		}

		// if we didn't get any at all, just print a none line
		if (first)
			printf("%-13s(none)\n", drivlist.driver().name);
	}
}


//-------------------------------------------------
//  listmedia - output the list of image devices
//  referenced by a given game or set of games
//-------------------------------------------------

void cli_frontend::listmedia(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// print header
	printf(" SYSTEM      MEDIA NAME (brief)   IMAGE FILE EXTENSIONS SUPPORTED     \n");
	printf("----------  --------------------  ------------------------------------\n");

	// iterate over drivers
	while (drivlist.next())
	{
		// iterate
		const device_image_interface *imagedev = NULL;
		bool first = true;
		for (bool gotone = drivlist.config().devicelist().first(imagedev); gotone; gotone = imagedev->next(imagedev))
		{
			// extract the shortname with parentheses
			astring paren_shortname;
			paren_shortname.format("(%s)", imagedev->brief_instance_name());

			// output the line, up to the list of extensions
			printf("%-13s%-12s%-8s   ", first ? drivlist.driver().name : "", imagedev->instance_name(), paren_shortname.cstr());

			// get the extensions and print them
			astring extensions(imagedev->file_extensions());
			for (int start = 0, end = extensions.chr(0, ','); ; start = end + 1, end = extensions.chr(start, ','))
			{
				astring curext(extensions, start, (end == -1) ? extensions.len() - start : end - start);
				printf(".%-5s", curext.cstr());
				if (end == -1)
					break;
			}

			// end the line
			printf("\n");
			first = false;
		}

		// if we didn't get any at all, just print a none line
		if (first)
			printf("%-13s(none)\n", drivlist.driver().name);
	}
}


//-------------------------------------------------
//  verifyroms - verify the ROM sets of one or
//  more games
//-------------------------------------------------

void cli_frontend::verifyroms(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	int correct = 0;
	int incorrect = 0;
	int notfound = 0;

	// iterate over drivers
	media_auditor auditor(drivlist);
	while (drivlist.next())
	{
		// audit the ROMs in this set
		media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

		// output the summary of the audit
		astring summary_string;
		auditor.summarize(&summary_string);
		mame_printf_info("%s", summary_string.cstr());

		// if not found, count that and leave it at that
		if (summary == media_auditor::NOTFOUND)
			notfound++;

		// else display information about what we discovered
		else
		{
			// output the name of the driver and its clone
			mame_printf_info("romset %s ", drivlist.driver().name);
			int clone_of = drivlist.clone();
			if (clone_of != -1)
				mame_printf_info("[%s] ", drivlist.driver(clone_of).name);

			// switch off of the result
			switch (summary)
			{
				case media_auditor::INCORRECT:
					mame_printf_info("is bad\n");
					incorrect++;
					break;

				case media_auditor::CORRECT:
					mame_printf_info("is good\n");
					correct++;
					break;

				case media_auditor::BEST_AVAILABLE:
					mame_printf_info("is best available\n");
					correct++;
					break;

				default:
					break;
			}
		}
	}

	// clear out any cached files
	zip_file_cache_clear();

	// if we didn't get anything at all, display a generic end message
	if (correct + incorrect == 0)
	{
		if (notfound > 0)
			throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "romset \"%s\" not found!\n", gamename);
		else
			throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "romset \"%s\" not supported!\n", gamename);
	}

	// otherwise, print a summary
	else
	{
		if (incorrect > 0)
			throw emu_fatalerror(MAMERR_MISSING_FILES, "%d romsets found, %d were OK.\n", correct + incorrect, correct);
		mame_printf_info("%d romsets found, %d were OK.\n", correct, correct);
	}
}


//-------------------------------------------------
//  info_verifysamples - verify the sample sets of
//  one or more games
//-------------------------------------------------

void cli_frontend::verifysamples(const char *gamename)
{
	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	int correct = 0;
	int incorrect = 0;
	int notfound = 0;

	// iterate over drivers
	media_auditor auditor(drivlist);
	while (drivlist.next())
	{
		// audit the samples in this set
		media_auditor::summary summary = auditor.audit_samples();

		// output the summary of the audit
		astring summary_string;
		auditor.summarize(&summary_string);
		mame_printf_info("%s", summary_string.cstr());

		// if not found, print a message and set the flag
		if (summary == media_auditor::NOTFOUND)
		{
			mame_printf_error("sampleset \"%s\" not found!\n", drivlist.driver().name);
			notfound++;
		}

		// else display information about what we discovered
		else
		{
			// output the name of the driver and its clone
			mame_printf_info("sampleset %s ", drivlist.driver().name);
			int clone_of = drivlist.clone();
			if (clone_of != -1)
				mame_printf_info("[%s] ", drivlist.driver(clone_of).name);

			// switch off of the result
			switch (summary)
			{
				case media_auditor::INCORRECT:
					mame_printf_info("is bad\n");
					incorrect++;
					break;

				case media_auditor::CORRECT:
					mame_printf_info("is good\n");
					correct++;
					break;

				case media_auditor::BEST_AVAILABLE:
					mame_printf_info("is best available\n");
					correct++;
					break;

				default:
					break;
			}
		}
	}

	// clear out any cached files
	zip_file_cache_clear();

	// if we didn't get anything at all, display a generic end message
	if (correct + incorrect == 0)
	{
		if (notfound > 0)
			throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "sampleset \"%s\" not found!\n", gamename);
		else
			throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "sampleset \"%s\" not supported!\n", gamename);
	}

	// otherwise, print a summary
	else
	{
		if (incorrect > 0)
			throw emu_fatalerror(MAMERR_MISSING_FILES, "%d samplesets found, %d were OK.\n", correct + incorrect, correct);
		mame_printf_info("%d samplesets found, %d were OK.\n", correct, correct);
	}
}


/*-------------------------------------------------
    info_listsoftware - output the list of
    software supported by a given game or set of
    games
    TODO: Add all information read from the source files
    Possible improvement: use a sorted list for
        identifying duplicate lists.
-------------------------------------------------*/

void cli_frontend::listsoftware(const char *gamename)
{
	FILE *out = stdout;

	// determine which drivers to output; return an error if none found
	driver_enumerator drivlist(m_options, gamename);
	if (drivlist.count() == 0)
		throw emu_fatalerror(MAMERR_NO_SUCH_GAME, "No matching games found for '%s'", gamename);

	// first determine the maximum number of lists we might encounter
	int list_count = 0;
	while (drivlist.next())
		for (const device_t *dev = drivlist.config().devicelist().first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
		{
			software_list_config *swlist = (software_list_config *)downcast<const legacy_device_base *>(dev)->inline_config();

			for (int listnum = 0; listnum < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; listnum++)
				if (swlist->list_name[listnum] && *swlist->list_name[listnum] && swlist->list_type == SOFTWARE_LIST_ORIGINAL_SYSTEM)
					list_count++;
		}

	// allocate a list
	astring *lists = global_alloc_array(astring, list_count);

	if (list_count)
	{
		fprintf( out,
				"<?xml version=\"1.0\"?>\n"
				"<!DOCTYPE softwarelist [\n"
				"<!ELEMENT softwarelists (softwarelist*)>\n"
				"\t<!ELEMENT softwarelist (software+)>\n"
				"\t\t<!ATTLIST softwarelist name CDATA #REQUIRED>\n"
				"\t\t<!ATTLIST softwarelist description CDATA #IMPLIED>\n"
				"\t\t<!ELEMENT software (description, year?, publisher, info*, sharedfeat*, part*)>\n"
				"\t\t\t<!ATTLIST software name CDATA #REQUIRED>\n"
				"\t\t\t<!ATTLIST software cloneof CDATA #IMPLIED>\n"
				"\t\t\t<!ATTLIST software supported (yes|partial|no) \"yes\">\n"
				"\t\t\t<!ELEMENT description (#PCDATA)>\n"
				"\t\t\t<!ELEMENT year (#PCDATA)>\n"
				"\t\t\t<!ELEMENT publisher (#PCDATA)>\n"
				// we still do not store the info strings internally, so there is no output here
				// TODO: add parsing info in softlist.c and then add output here!
				"\t\t\t<!ELEMENT info EMPTY>\n"
				"\t\t\t\t<!ATTLIST info name CDATA #REQUIRED>\n"
				"\t\t\t\t<!ATTLIST info value CDATA #IMPLIED>\n"
				// shared features get stored in the part->feature below and are output there
				// this means that we don't output any <sharedfeat> and that -lsoft output will
				// be different from the list in hash/ when the list uses sharedfeat. But this
				// is by design: sharedfeat is only available to simplify the life to list creators,
				// to e.g. avoid manually adding the same feature to each disk of a 9 floppies game!
				"\t\t\t<!ELEMENT sharedfeat EMPTY>\n"
				"\t\t\t\t<!ATTLIST sharedfeat name CDATA #REQUIRED>\n"
				"\t\t\t\t<!ATTLIST sharedfeat value CDATA #IMPLIED>\n"
				"\t\t\t<!ELEMENT part (feature*, dataarea*, diskarea*, dipswitch*)>\n"
				"\t\t\t\t<!ATTLIST part name CDATA #REQUIRED>\n"
				"\t\t\t\t<!ATTLIST part interface CDATA #REQUIRED>\n"
				"\t\t\t\t<!ELEMENT feature EMPTY>\n"
				"\t\t\t\t\t<!ATTLIST feature name CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ATTLIST feature value CDATA #IMPLIED>\n"
				"\t\t\t\t<!ELEMENT dataarea (rom*)>\n"
				"\t\t\t\t\t<!ATTLIST dataarea name CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ATTLIST dataarea size CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ATTLIST dataarea databits (8|16|32|64) \"8\">\n"
				"\t\t\t\t\t<!ATTLIST dataarea endian (big|little) \"little\">\n"
				"\t\t\t\t\t<!ELEMENT rom EMPTY>\n"
				"\t\t\t\t\t\t<!ATTLIST rom name CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom size CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom length CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom crc CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom md5 CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom sha1 CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom offset CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom value CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST rom status (baddump|nodump|good) \"good\">\n"
				"\t\t\t\t\t\t<!ATTLIST rom loadflag (load16_byte|load16_word|load16_word_swap|load32_byte|load32_word|load32_word_swap|load32_dword|load64_word|load64_word_swap|reload|fill|continue) #IMPLIED>\n"
				"\t\t\t\t<!ELEMENT diskarea (disk*)>\n"
				"\t\t\t\t\t<!ATTLIST diskarea name CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ELEMENT disk EMPTY>\n"
				"\t\t\t\t\t\t<!ATTLIST disk name CDATA #REQUIRED>\n"
				"\t\t\t\t\t\t<!ATTLIST disk md5 CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST disk sha1 CDATA #IMPLIED>\n"
				"\t\t\t\t\t\t<!ATTLIST disk status (baddump|nodump|good) \"good\">\n"
				"\t\t\t\t\t\t<!ATTLIST disk writeable (yes|no) \"no\">\n"
				// we still do not store the dipswitch values internally, so there is no output here
				// TODO: add parsing dipsw in softlist.c and then add output here!
				"\t\t\t\t<!ELEMENT dipswitch (dipvalue*)>\n"
				"\t\t\t\t\t<!ATTLIST dipswitch name CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ATTLIST dipswitch tag CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ATTLIST dipswitch mask CDATA #REQUIRED>\n"
				"\t\t\t\t\t<!ELEMENT dipvalue EMPTY>\n"
				"\t\t\t\t\t\t<!ATTLIST dipvalue name CDATA #REQUIRED>\n"
				"\t\t\t\t\t\t<!ATTLIST dipvalue value CDATA #REQUIRED>\n"
				"\t\t\t\t\t\t<!ATTLIST dipvalue default (yes|no) \"no\">\n"
				"]>\n\n"
				"<softwarelists>\n"
				);
	}

	drivlist.reset();
	list_count = 0;
	while (drivlist.next())
		for (const device_t *dev = drivlist.config().devicelist().first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
		{
			software_list_config *swlist = (software_list_config *)downcast<const legacy_device_base *>(dev)->inline_config();

			for (int listnum = 0; listnum < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; listnum++)
			{
				if (swlist->list_name[listnum] && *swlist->list_name[listnum] && swlist->list_type == SOFTWARE_LIST_ORIGINAL_SYSTEM)
				{
					software_list *list = software_list_open(m_options, swlist->list_name[listnum], FALSE, NULL);

					if ( list )
					{
						/* Verify if we have encountered this list before */
						bool seen_before = false;
						for (int seen_index = 0; seen_index < list_count && !seen_before; seen_index++)
							if (lists[seen_index] == swlist->list_name[listnum])
								seen_before = true;

						if (!seen_before)
						{
							lists[list_count++] = swlist->list_name[listnum];
							software_list_parse( list, NULL, NULL );

							fprintf(out, "\t<softwarelist name=\"%s\" description=\"%s\">\n", swlist->list_name[listnum], xml_normalize_string(software_list_get_description(list)) );

							for ( software_info *swinfo = software_list_find( list, "*", NULL ); swinfo != NULL; swinfo = software_list_find( list, "*", swinfo ) )
							{
								fprintf( out, "\t\t<software name=\"%s\"", swinfo->shortname );
								if ( swinfo->parentname != NULL )
									fprintf( out, " cloneof=\"%s\"", swinfo->parentname );
								if ( swinfo->supported == SOFTWARE_SUPPORTED_PARTIAL )
									fprintf( out, " supported=\"partial\"" );
								if ( swinfo->supported == SOFTWARE_SUPPORTED_NO )
									fprintf( out, " supported=\"no\"" );
								fprintf( out, ">\n" );
								fprintf( out, "\t\t\t<description>%s</description>\n", xml_normalize_string(swinfo->longname) );
								fprintf( out, "\t\t\t<year>%s</year>\n", xml_normalize_string( swinfo->year ) );
								fprintf( out, "\t\t\t<publisher>%s</publisher>\n", xml_normalize_string( swinfo->publisher ) );

								for ( software_part *part = software_find_part( swinfo, NULL, NULL ); part != NULL; part = software_part_next( part ) )
								{
									fprintf( out, "\t\t\t<part name=\"%s\"", part->name );
									if ( part->interface_ )
										fprintf( out, " interface=\"%s\"", part->interface_ );

									fprintf( out, ">\n");

									if ( part->featurelist )
									{
										feature_list *list = part->featurelist;

										while( list )
										{
											fprintf( out, "\t\t\t\t<feature name=\"%s\" value=\"%s\" />\n", list->name, list->value );
											list = list->next;
										}
									}

									/* TODO: display rom region information */
									for ( const rom_entry *region = part->romdata; region; region = rom_next_region( region ) )
									{
										int is_disk = ROMREGION_ISDISKDATA(region);

										if (!is_disk)
											fprintf( out, "\t\t\t\t<dataarea name=\"%s\" size=\"%d\">\n", ROMREGION_GETTAG(region), ROMREGION_GETLENGTH(region) );
										else
											fprintf( out, "\t\t\t\t<diskarea name=\"%s\">\n", ROMREGION_GETTAG(region) );

										for ( const rom_entry *rom = rom_first_file( region ); rom && !ROMENTRY_ISREGIONEND(rom); rom++ )
										{
											if ( ROMENTRY_ISFILE(rom) )
											{
												if (!is_disk)
													fprintf( out, "\t\t\t\t\t<rom name=\"%s\" size=\"%d\"", xml_normalize_string(ROM_GETNAME(rom)), rom_file_size(rom) );
												else
													fprintf( out, "\t\t\t\t\t<disk name=\"%s\"", xml_normalize_string(ROM_GETNAME(rom)) );

												/* dump checksum information only if there is a known dump */
												hash_collection hashes(ROM_GETHASHDATA(rom));
												if (!hashes.flag(hash_collection::FLAG_NO_DUMP))
												{
													astring tempstr;
													for (hash_base *hash = hashes.first(); hash != NULL; hash = hash->next())
														fprintf(out, " %s=\"%s\"", hash->name(), hash->string(tempstr));
												}

												if (!is_disk)
													fprintf( out, " offset=\"0x%x\"", ROM_GETOFFSET(rom) );

												if ( hashes.flag(hash_collection::FLAG_BAD_DUMP) )
													fprintf( out, " status=\"baddump\"" );
												if ( hashes.flag(hash_collection::FLAG_NO_DUMP) )
													fprintf( out, " status=\"nodump\"" );

												if (is_disk)
													fprintf( out, " writeable=\"%s\"", (ROM_GETFLAGS(rom) & DISK_READONLYMASK) ? "no" : "yes");

												if ((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(1))
													fprintf( out, " loadflag=\"load16_byte\"" );

												if ((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(3))
													fprintf( out, " loadflag=\"load32_byte\"" );

												if (((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(2)) && ((ROM_GETFLAGS(rom) & ROM_GROUPMASK) == ROM_GROUPWORD))
												{
													if (!(ROM_GETFLAGS(rom) & ROM_REVERSEMASK))
														fprintf( out, " loadflag=\"load32_word\"" );
													else
														fprintf( out, " loadflag=\"load32_word_swap\"" );
												}

												if (((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_SKIP(6)) && ((ROM_GETFLAGS(rom) & ROM_GROUPMASK) == ROM_GROUPWORD))
												{
													if (!(ROM_GETFLAGS(rom) & ROM_REVERSEMASK))
														fprintf( out, " loadflag=\"load64_word\"" );
													else
														fprintf( out, " loadflag=\"load64_word_swap\"" );
												}

												if (((ROM_GETFLAGS(rom) & ROM_SKIPMASK) == ROM_NOSKIP) && ((ROM_GETFLAGS(rom) & ROM_GROUPMASK) == ROM_GROUPWORD))
												{
													if (!(ROM_GETFLAGS(rom) & ROM_REVERSEMASK))
														fprintf( out, " loadflag=\"load32_dword\"" );
													else
														fprintf( out, " loadflag=\"load16_word_swap\"" );
												}

												fprintf( out, "/>\n" );
											}
											else if ( ROMENTRY_ISRELOAD(rom) )
											{
												fprintf( out, "\t\t\t\t\t<rom size=\"%d\" offset=\"0x%x\" loadflag=\"reload\" />\n", ROM_GETLENGTH(rom), ROM_GETOFFSET(rom) );
											}
											else if ( ROMENTRY_ISCONTINUE(rom) )
											{
												fprintf( out, "\t\t\t\t\t<rom size=\"%d\" offset=\"0x%x\" loadflag=\"continue\" />\n", ROM_GETLENGTH(rom), ROM_GETOFFSET(rom) );
											}
											else if ( ROMENTRY_ISFILL(rom) )
											{
												fprintf( out, "\t\t\t\t\t<rom size=\"%d\" offset=\"0x%x\" loadflag=\"fill\" />\n", ROM_GETLENGTH(rom), ROM_GETOFFSET(rom) );
											}
										}

										if (!is_disk)
											fprintf( out, "\t\t\t\t</dataarea>\n" );
										else
											fprintf( out, "\t\t\t\t</diskarea>\n" );
									}

									fprintf( out, "\t\t\t</part>\n" );
								}

								fprintf( out, "\t\t</software>\n" );
							}

							fprintf(out, "\t</softwarelist>\n" );
						}

						software_list_close( list );
					}
				}
			}
		}

	if (list_count > 0)
		fprintf( out, "</softwarelists>\n" );
	else
		fprintf( out, "No software lists found for this system\n" );

	global_free( lists );
}


//-------------------------------------------------
//  romident - identify ROMs by looking for
//  matches in our internal database
//-------------------------------------------------

void cli_frontend::romident(const char *filename)
{
	media_identifier ident(m_options);

	// identify the file, then output results
	mame_printf_info("Identifying %s....\n", filename);
	ident.identify(filename);

	// return the appropriate error code
	if (ident.matches() == ident.total())
		return;
	else if (ident.matches() == ident.total() - ident.nonroms())
		throw emu_fatalerror(MAMERR_IDENT_NONROMS, "");
	else if (ident.matches() > 0)
		throw emu_fatalerror(MAMERR_IDENT_PARTIAL, "");
	else
		throw emu_fatalerror(MAMERR_IDENT_NONE, "");
}


//-------------------------------------------------
//  execute_commands - execute various frontend
//  commands
//-------------------------------------------------

void cli_frontend::execute_commands(const char *exename)
{
	// help?
	if (strcmp(m_options.command(), CLICOMMAND_HELP) == 0)
	{
		display_help();
		return;
	}

	// showusage?
	if (strcmp(m_options.command(), CLICOMMAND_SHOWUSAGE) == 0)
	{
		astring helpstring;
		mame_printf_info(USAGE "\n\nOptions:\n%s", exename, GAMENOUN, m_options.output_help(helpstring));
		return;
	}

	// validate?
	if (strcmp(m_options.command(), CLICOMMAND_VALIDATE) == 0)
	{
		validate_drivers(m_options);
		return;
	}

	// other commands need the INIs parsed
	astring option_errors;
	m_options.parse_standard_inis(option_errors);
	if (option_errors)
		printf("%s\n", option_errors.cstr());

	// createconfig?
	if (strcmp(m_options.command(), CLICOMMAND_CREATECONFIG) == 0)
	{
		// attempt to open the output file
		emu_file file(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (file.open(CONFIGNAME ".ini") != FILERR_NONE)
			throw emu_fatalerror("Unable to create file " CONFIGNAME ".ini\n");

		// generate the updated INI
		astring initext;
		file.puts(m_options.output_ini(initext));
		return;
	}

	// showconfig?
	if (strcmp(m_options.command(), CLICOMMAND_SHOWCONFIG) == 0)
	{
		// print the INI text
		astring initext;
		printf("%s\n", m_options.output_ini(initext));
		return;
	}

	// all other commands call out to one of these helpers
	static const struct
	{
		const char *option;
		void (cli_frontend::*function)(const char *gamename);
	} info_commands[] =
	{
		{ CLICOMMAND_LISTXML,		&cli_frontend::listxml },
		{ CLICOMMAND_LISTFULL,		&cli_frontend::listfull },
		{ CLICOMMAND_LISTSOURCE,	&cli_frontend::listsource },
		{ CLICOMMAND_LISTCLONES,	&cli_frontend::listclones },
		{ CLICOMMAND_LISTBROTHERS,	&cli_frontend::listbrothers },
		{ CLICOMMAND_LISTCRC,		&cli_frontend::listcrc },
		{ CLICOMMAND_LISTDEVICES,	&cli_frontend::listdevices },
		{ CLICOMMAND_LISTSLOTS,	    &cli_frontend::listslots },
		{ CLICOMMAND_LISTROMS,		&cli_frontend::listroms },
		{ CLICOMMAND_LISTSAMPLES,	&cli_frontend::listsamples },
		{ CLICOMMAND_VERIFYROMS,	&cli_frontend::verifyroms },
		{ CLICOMMAND_VERIFYSAMPLES,	&cli_frontend::verifysamples },
		{ CLICOMMAND_LISTMEDIA,		&cli_frontend::listmedia },
		{ CLICOMMAND_LISTSOFTWARE,  &cli_frontend::listsoftware },
		{ CLICOMMAND_ROMIDENT,		&cli_frontend::romident }
	};

	// find the command
	for (int cmdindex = 0; cmdindex < ARRAY_LENGTH(info_commands); cmdindex++)
		if (strcmp(m_options.command(), info_commands[cmdindex].option) == 0)
		{
			// parse any relevant INI files before proceeding
			const char *sysname = m_options.system_name();
			(this->*info_commands[cmdindex].function)((sysname[0] == 0) ? "*" : sysname);
			return;
		}

	// if we get here, we don't know what has been requested
	throw emu_fatalerror(MAMERR_INVALID_CONFIG, "Unknown command '%s' specified", m_options.command());
}


//-------------------------------------------------
//  display_help - display help to standard
//  output
//-------------------------------------------------

void cli_frontend::display_help()
{
	mame_printf_info(APPLONGNAME " v%s - " FULLLONGNAME "\n"
		   COPYRIGHT_INFO "\n\n", build_version);
	mame_printf_info("%s\n", DISCLAIMER);
	mame_printf_info(USAGE "\n\n"
		   "        " APPNAME " -showusage    for a brief list of options\n"
		   "        " APPNAME " -showconfig   for a list of configuration options\n"
		   "        " APPNAME " -listmedia    for a full list of supported media\n"
		   "        " APPNAME " -createconfig to create a " CONFIGNAME ".ini\n\n"
		   "For usage instructions, please consult the files config.txt and windows.txt.\n",APPNAME,GAMENOUN);
}


//-------------------------------------------------
//  display_suggestions - display 10 possible
//  matches for a given invalid gamename
//-------------------------------------------------

void cli_frontend::display_suggestions(const char *gamename)
{
}


//**************************************************************************
//  MEDIA IDENTIFIER
//**************************************************************************

//-------------------------------------------------
//  media_identifier - constructor
//-------------------------------------------------

media_identifier::media_identifier(cli_options &options)
	: m_drivlist(options),
	  m_total(0),
	  m_matches(0),
	  m_nonroms(0)
{
}


//-------------------------------------------------
//  identify - identify a directory, ZIP file,
//  or raw file
//-------------------------------------------------

void media_identifier::identify(const char *filename)
{
	// first try to open as a directory
	osd_directory *directory = osd_opendir(filename);
	if (directory != NULL)
	{
		// iterate over all files in the directory
		for (const osd_directory_entry *entry = osd_readdir(directory); entry != NULL; entry = osd_readdir(directory))
			if (entry->type == ENTTYPE_FILE)
			{
				astring curfile(filename, PATH_SEPARATOR, entry->name);
				identify_file(curfile);
			}

		// close the directory and be done
		osd_closedir(directory);
	}

	// if that failed, and the filename ends with .zip, identify as a ZIP file
	else if (core_filename_ends_with(filename, ".zip"))
	{
		// first attempt to examine it as a valid ZIP file
		zip_file *zip = NULL;
		zip_error ziperr = zip_file_open(filename, &zip);
		if (ziperr == ZIPERR_NONE && zip != NULL)
		{
			// loop over entries in the ZIP, skipping empty files and directories
			for (const zip_file_header *entry = zip_file_first_file(zip); entry != NULL; entry = zip_file_next_file(zip))
				if (entry->uncompressed_length != 0)
				{
					UINT8 *data = global_alloc_array(UINT8, entry->uncompressed_length);
					if (data != NULL)
					{
						// decompress data into RAM and identify it
						ziperr = zip_file_decompress(zip, data, entry->uncompressed_length);
						if (ziperr == ZIPERR_NONE)
							identify_data(entry->filename, data, entry->uncompressed_length);
						global_free(data);
					}
				}

			// close up
			zip_file_close(zip);
		}

		// clear out any cached files
		zip_file_cache_clear();
	}

	// otherwise, identify as a raw file
	else
		identify_file(filename);
}


//-------------------------------------------------
//  identify_file - identify a file
//-------------------------------------------------

void media_identifier::identify_file(const char *name)
{
	// CHD files need to be parsed and their hashes extracted from the header
	if (core_filename_ends_with(name, ".chd"))
	{
		// output the name
		astring basename;
		mame_printf_info("%-20s", core_filename_extract_base(&basename, name, FALSE)->cstr());
		m_total++;

		// attempt to open as a CHD; fail if not
		chd_file *chd;
		chd_error err = chd_open(name, CHD_OPEN_READ, NULL, &chd);
		if (err != CHDERR_NONE)
		{
			mame_printf_info("NOT A CHD\n");
			m_nonroms++;
			return;
		}

		// fetch the header and close the file
		chd_header header = *chd_get_header(chd);
		chd_close(chd);

		// error on writable CHDs
		if (header.flags & CHDFLAGS_IS_WRITEABLE)
		{
			mame_printf_info("is a writeable CHD\n");
			return;
		}

		// otherwise, get the hash collection for this CHD
		static const UINT8 nullhash[20] = { 0 };
		hash_collection hashes;

		if (memcmp(nullhash, header.md5, sizeof(header.md5)) != 0)
			hashes.add_from_buffer(hash_collection::HASH_MD5, header.md5, sizeof(header.md5));
		if (memcmp(nullhash, header.sha1, sizeof(header.sha1)) != 0)
			hashes.add_from_buffer(hash_collection::HASH_SHA1, header.sha1, sizeof(header.sha1));

		// determine whether this file exists
		int found = find_by_hash(hashes, header.logicalbytes);
		if (found == 0)
			mame_printf_info("NO MATCH\n");
		else
			m_matches++;
	}

	// all other files have their hashes computed directly
	else
	{
		// load the file and process if it opens and has a valid length
		UINT32 length;
		void *data;
		file_error filerr = core_fload(name, &data, &length);
		if (filerr == FILERR_NONE && length > 0)
			identify_data(name, reinterpret_cast<UINT8 *>(data), length);
	}
}


//-------------------------------------------------
//  identify_data - identify a buffer full of
//  data; if it comes from a .JED file, parse the
//  fusemap into raw data first
//-------------------------------------------------

void media_identifier::identify_data(const char *name, const UINT8 *data, int length)
{
	// if this is a '.jed' file, process it into raw bits first
	UINT8 *tempjed = NULL;
	jed_data jed;
	if (core_filename_ends_with(name, ".jed") && jed_parse(data, length, &jed) == JEDERR_NONE)
	{
		// now determine the new data length and allocate temporary memory for it
		length = jedbin_output(&jed, NULL, 0);
		tempjed = global_alloc_array(UINT8, length);
		jedbin_output(&jed, tempjed, length);
		data = tempjed;
	}

	// compute the hash of the data
	hash_collection hashes;
	hashes.compute(data, length, hash_collection::HASH_TYPES_CRC_SHA1);

	// output the name
	m_total++;
	astring basename;
	mame_printf_info("%-20s", core_filename_extract_base(&basename, name, FALSE)->cstr());

	// see if we can find a match in the ROMs
	int found = find_by_hash(hashes, length);

	// if we didn't find it, try to guess what it might be
	if (found == 0)
	{
		// if not a power of 2, assume it is a non-ROM file
		if ((length & (length - 1)) != 0)
		{
			mame_printf_info("NOT A ROM\n");
			m_nonroms++;
		}

		// otherwise, it's just not a match
		else
			mame_printf_info("NO MATCH\n");
	}

	// if we did find it, count it as a match
	else
		m_matches++;

	// free any temporary JED data
	global_free(tempjed);
}


//-------------------------------------------------
//  find_by_hash - scan for a file in the list
//  of drivers by hash
//-------------------------------------------------

int media_identifier::find_by_hash(const hash_collection &hashes, int length)
{
	int found = 0;

	// iterate over drivers
	m_drivlist.reset();
	while (m_drivlist.next())
	{
		// iterate over sources, regions and files within the region */
		for (const rom_source *source = rom_first_source(m_drivlist.config()); source != NULL; source = rom_next_source(*source))
			for (const rom_entry *region = rom_first_region(*source); region != NULL; region = rom_next_region(region))
				for (const rom_entry *rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))
				{
					hash_collection romhashes(ROM_GETHASHDATA(rom));
					if (!romhashes.flag(hash_collection::FLAG_NO_DUMP) && hashes == romhashes)
					{
						bool baddump = romhashes.flag(hash_collection::FLAG_BAD_DUMP);

						// output information about the match
						if (!found)
							mame_printf_info("                    ");
						mame_printf_info("= %s%-20s  %-10s %s\n", baddump ? "(BAD) " : "", ROM_GETNAME(rom), m_drivlist.driver().name, m_drivlist.driver().description);
						found++;
					}
				}

		// next iterate over softlists
		for (const device_t *dev = m_drivlist.config().devicelist().first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
		{
			software_list_config *swlist = (software_list_config *)downcast<const legacy_device_base *>(dev)->inline_config();

			for (int listnum = 0; listnum < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; listnum++)
				if (swlist->list_name[listnum] != NULL)
				{
					software_list *list = software_list_open(m_drivlist.options(), swlist->list_name[listnum], FALSE, NULL);

					for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
						for (software_part *part = software_find_part(swinfo, NULL, NULL); part != NULL; part = software_part_next(part))
							for (const rom_entry *region = part->romdata; region != NULL; region = rom_next_region(region))
								for (const rom_entry *rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))
								{
									hash_collection romhashes(ROM_GETHASHDATA(rom));
									if (hashes == romhashes)
									{
										bool baddump = romhashes.flag(hash_collection::FLAG_BAD_DUMP);

										// output information about the match
										if (!found)
											mame_printf_info("                    ");
										mame_printf_info("= %s%-20s  %s:%s %s\n", baddump ? "(BAD) " : "", ROM_GETNAME(rom), swlist->list_name[listnum], swinfo->shortname, swinfo->longname);
										found++;
									}
								}

					software_list_close(list);
				}
		}
	}

	return found;
}
