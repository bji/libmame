/***************************************************************************

    info.c

    Dumps the MAME internal data as an XML file.

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
#include "machine/ram.h"
#include "sound/samples.h"
#include "info.h"
#include "xmlfile.h"
#include "hash.h"
#include "config.h"

#include <ctype.h>

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// DTD string describing the data
const char info_xml_creator::s_dtd_string[] =
"<!DOCTYPE __XML_ROOT__ [\n"
"<!ELEMENT __XML_ROOT__ (__XML_TOP__+)>\n"
"\t<!ATTLIST __XML_ROOT__ build CDATA #IMPLIED>\n"
"\t<!ATTLIST __XML_ROOT__ debug (yes|no) \"no\">\n"
"\t<!ATTLIST __XML_ROOT__ mameconfig CDATA #REQUIRED>\n"
"\t<!ELEMENT __XML_TOP__ (description, year?, manufacturer?, biosset*, rom*, disk*, device_ref*, sample*, chip*, display*, sound?, input?, dipswitch*, configuration*, adjuster*, driver?, device*, slot*, softwarelist*, ramoption*)>\n"
"\t\t<!ATTLIST __XML_TOP__ name CDATA #REQUIRED>\n"
"\t\t<!ATTLIST __XML_TOP__ sourcefile CDATA #IMPLIED>\n"
"\t\t<!ATTLIST __XML_TOP__ isbios (yes|no) \"no\">\n"
"\t\t<!ATTLIST __XML_TOP__ isdevice (yes|no) \"no\">\n"
"\t\t<!ATTLIST __XML_TOP__ ismechanical (yes|no) \"no\">\n"
"\t\t<!ATTLIST __XML_TOP__ runnable (yes|no) \"yes\">\n"
"\t\t<!ATTLIST __XML_TOP__ cloneof CDATA #IMPLIED>\n"
"\t\t<!ATTLIST __XML_TOP__ romof CDATA #IMPLIED>\n"
"\t\t<!ATTLIST __XML_TOP__ sampleof CDATA #IMPLIED>\n"
"\t\t<!ELEMENT description (#PCDATA)>\n"
"\t\t<!ELEMENT year (#PCDATA)>\n"
"\t\t<!ELEMENT manufacturer (#PCDATA)>\n"
"\t\t<!ELEMENT biosset EMPTY>\n"
"\t\t\t<!ATTLIST biosset name CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST biosset description CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST biosset default (yes|no) \"no\">\n"
"\t\t<!ELEMENT rom EMPTY>\n"
"\t\t\t<!ATTLIST rom name CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST rom bios CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST rom size CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST rom crc CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST rom sha1 CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST rom merge CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST rom region CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST rom offset CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST rom status (baddump|nodump|good) \"good\">\n"
"\t\t\t<!ATTLIST rom optional (yes|no) \"no\">\n"
"\t\t<!ELEMENT disk EMPTY>\n"
"\t\t\t<!ATTLIST disk name CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST disk sha1 CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST disk merge CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST disk region CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST disk index CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST disk writable (yes|no) \"no\">\n"
"\t\t\t<!ATTLIST disk status (baddump|nodump|good) \"good\">\n"
"\t\t\t<!ATTLIST disk optional (yes|no) \"no\">\n"
"\t\t<!ELEMENT device_ref EMPTY>\n"
"\t\t\t<!ATTLIST device_ref name CDATA #REQUIRED>\n"
"\t\t<!ELEMENT sample EMPTY>\n"
"\t\t\t<!ATTLIST sample name CDATA #REQUIRED>\n"
"\t\t<!ELEMENT chip EMPTY>\n"
"\t\t\t<!ATTLIST chip name CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST chip tag CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST chip type (cpu|audio) #REQUIRED>\n"
"\t\t\t<!ATTLIST chip clock CDATA #IMPLIED>\n"
"\t\t<!ELEMENT display EMPTY>\n"
"\t\t\t<!ATTLIST display type (raster|vector|lcd|unknown) #REQUIRED>\n"
"\t\t\t<!ATTLIST display rotate (0|90|180|270) #REQUIRED>\n"
"\t\t\t<!ATTLIST display flipx (yes|no) \"no\">\n"
"\t\t\t<!ATTLIST display width CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST display height CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST display refresh CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST display pixclock CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST display htotal CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST display hbend CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST display hbstart CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST display vtotal CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST display vbend CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST display vbstart CDATA #IMPLIED>\n"
"\t\t<!ELEMENT sound EMPTY>\n"
"\t\t\t<!ATTLIST sound channels CDATA #REQUIRED>\n"
"\t\t<!ELEMENT input (control*)>\n"
"\t\t\t<!ATTLIST input service (yes|no) \"no\">\n"
"\t\t\t<!ATTLIST input tilt (yes|no) \"no\">\n"
"\t\t\t<!ATTLIST input players CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST input buttons CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST input coins CDATA #IMPLIED>\n"
"\t\t\t<!ELEMENT control EMPTY>\n"
"\t\t\t\t<!ATTLIST control type CDATA #REQUIRED>\n"
"\t\t\t\t<!ATTLIST control minimum CDATA #IMPLIED>\n"
"\t\t\t\t<!ATTLIST control maximum CDATA #IMPLIED>\n"
"\t\t\t\t<!ATTLIST control sensitivity CDATA #IMPLIED>\n"
"\t\t\t\t<!ATTLIST control keydelta CDATA #IMPLIED>\n"
"\t\t\t\t<!ATTLIST control reverse (yes|no) \"no\">\n"
"\t\t\t\t<!ATTLIST control ways CDATA #IMPLIED>\n"
"\t\t\t\t<!ATTLIST control ways2 CDATA #IMPLIED>\n"
"\t\t\t\t<!ATTLIST control ways3 CDATA #IMPLIED>\n"
"\t\t<!ELEMENT dipswitch (dipvalue*)>\n"
"\t\t\t<!ATTLIST dipswitch name CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST dipswitch tag CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST dipswitch mask CDATA #REQUIRED>\n"
"\t\t\t<!ELEMENT dipvalue EMPTY>\n"
"\t\t\t\t<!ATTLIST dipvalue name CDATA #REQUIRED>\n"
"\t\t\t\t<!ATTLIST dipvalue value CDATA #REQUIRED>\n"
"\t\t\t\t<!ATTLIST dipvalue default (yes|no) \"no\">\n"
"\t\t<!ELEMENT configuration (confsetting*)>\n"
"\t\t\t<!ATTLIST configuration name CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST configuration tag CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST configuration mask CDATA #REQUIRED>\n"
"\t\t\t<!ELEMENT confsetting EMPTY>\n"
"\t\t\t\t<!ATTLIST confsetting name CDATA #REQUIRED>\n"
"\t\t\t\t<!ATTLIST confsetting value CDATA #REQUIRED>\n"
"\t\t\t\t<!ATTLIST confsetting default (yes|no) \"no\">\n"
"\t\t<!ELEMENT adjuster EMPTY>\n"
"\t\t\t<!ATTLIST adjuster name CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST adjuster default CDATA #REQUIRED>\n"
"\t\t<!ELEMENT driver EMPTY>\n"
"\t\t\t<!ATTLIST driver status (good|imperfect|preliminary) #REQUIRED>\n"
"\t\t\t<!ATTLIST driver emulation (good|imperfect|preliminary) #REQUIRED>\n"
"\t\t\t<!ATTLIST driver color (good|imperfect|preliminary) #REQUIRED>\n"
"\t\t\t<!ATTLIST driver sound (good|imperfect|preliminary) #REQUIRED>\n"
"\t\t\t<!ATTLIST driver graphic (good|imperfect|preliminary) #REQUIRED>\n"
"\t\t\t<!ATTLIST driver cocktail (good|imperfect|preliminary) #IMPLIED>\n"
"\t\t\t<!ATTLIST driver protection (good|imperfect|preliminary) #IMPLIED>\n"
"\t\t\t<!ATTLIST driver savestate (supported|unsupported) #REQUIRED>\n"
"\t\t\t<!ATTLIST driver palettesize CDATA #REQUIRED>\n"
"\t\t<!ELEMENT device (instance*, extension*)>\n"
"\t\t\t<!ATTLIST device type CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST device tag CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST device mandatory CDATA #IMPLIED>\n"
"\t\t\t<!ATTLIST device interface CDATA #IMPLIED>\n"
"\t\t\t<!ELEMENT instance EMPTY>\n"
"\t\t\t\t<!ATTLIST instance name CDATA #REQUIRED>\n"
"\t\t\t\t<!ATTLIST instance briefname CDATA #REQUIRED>\n"
"\t\t\t<!ELEMENT extension EMPTY>\n"
"\t\t\t\t<!ATTLIST extension name CDATA #REQUIRED>\n"
"\t\t<!ELEMENT slot (slotoption*)>\n"
"\t\t\t<!ATTLIST slot name CDATA #REQUIRED>\n"
"\t\t\t<!ELEMENT slotoption EMPTY>\n"
"\t\t\t\t<!ATTLIST slotoption name CDATA #REQUIRED>\n"
"\t\t\t\t<!ATTLIST slotoption default (yes|no) \"no\">\n"
"\t\t<!ELEMENT softwarelist EMPTY>\n"
"\t\t\t<!ATTLIST softwarelist name CDATA #REQUIRED>\n"
"\t\t\t<!ATTLIST softwarelist status (original|compatible) #REQUIRED>\n"
"\t\t\t<!ATTLIST softwarelist filter CDATA #IMPLIED>\n"
"\t\t<!ELEMENT ramoption (#PCDATA)>\n"
"\t\t\t<!ATTLIST ramoption default CDATA #IMPLIED>\n"
"]>";


extern const device_type *s_devices_sorted[];
extern int m_device_count;

//**************************************************************************
//  INFO XML CREATOR
//**************************************************************************

//-------------------------------------------------
//  info_xml_creator - constructor
//-------------------------------------------------

info_xml_creator::info_xml_creator(driver_enumerator &drivlist)
	: m_output(NULL),
	  m_drivlist(drivlist),
	  m_lookup_options(m_drivlist.options())
{
	m_lookup_options.remove_device_options();
}


//-------------------------------------------------
//  output_mame_xml - print the XML information
//  for all known games
//-------------------------------------------------

void info_xml_creator::output(FILE *out)
{
	m_output = out;

	// output the DTD
	fprintf(m_output, "<?xml version=\"1.0\"?>\n");
	astring dtd(s_dtd_string);
	dtd.replace(0,"__XML_ROOT__", emulator_info::get_xml_root());
	dtd.replace(0,"__XML_TOP__", emulator_info::get_xml_top());

	fprintf(m_output, "%s\n\n", dtd.cstr());

	// top-level tag
	fprintf(m_output, "<%s build=\"%s\" debug=\""
#ifdef MAME_DEBUG
		"yes"
#else
		"no"
#endif
		"\" mameconfig=\"%d\">\n",
		emulator_info::get_xml_root(),
		xml_normalize_string(build_version),
		CONFIG_VERSION
	);

	m_device_used = global_alloc_array_clear(UINT8, m_device_count);

	// iterate through the drivers, outputting one at a time
	while (m_drivlist.next())
		output_one();

	// iterate through the devices, and output their roms
	output_devices();

	global_free(m_device_used);

	// close the top level tag
	fprintf(m_output, "</%s>\n",emulator_info::get_xml_root());
}


//-------------------------------------------------
//  output_devices - print the XML information
//  for one particular game driver
//-------------------------------------------------

void info_xml_creator::output_devices()
{
	m_drivlist.reset();
	m_drivlist.next();
	machine_config &config = m_drivlist.config();
	device_t &owner = config.root_device();
	// check if all are listed, note that empty one is included
	bool display_all = driver_list::total() == (m_drivlist.count()+1);
	for(int i=0;i<m_device_count;i++) {
		if (display_all || (m_device_used[i]!=0)) {
			device_type type = *s_devices_sorted[i];
			device_t *dev = (*type)(config, "dummy", &owner, 0);
			dev->config_complete();

			// print the header and the game name
			fprintf(m_output, "\t<%s",emulator_info::get_xml_top());
			fprintf(m_output, " name=\"%s\"", xml_normalize_string(dev->shortname()));
			fprintf(m_output, " isdevice=\"yes\"");
			fprintf(m_output, " runnable=\"no\"");
			fprintf(m_output, ">\n");

			// output device description
			if (dev->name() != NULL)
				fprintf(m_output, "\t\t<description>%s</description>\n", xml_normalize_string(dev->name()));

			output_rom(dev);

			// close the topmost tag
			fprintf(m_output, "\t</%s>\n",emulator_info::get_xml_top());
			global_free(dev);
		}
	}
}

//-------------------------------------------------
//  output_one - print the XML information
//  for one particular game driver
//-------------------------------------------------

void info_xml_creator::output_one()
{
	// no action if not a game
	const game_driver &driver = m_drivlist.driver();
	if (driver.flags & GAME_NO_STANDALONE)
		return;

	// allocate input ports
	machine_config &config = m_drivlist.config();
	ioport_list portlist;
	astring errors;
	device_iterator iter(config.root_device());
	for (device_t *device = iter.first(); device != NULL; device = iter.next())
		input_port_list_init(*device, portlist, errors);

	// print the header and the game name
	fprintf(m_output, "\t<%s",emulator_info::get_xml_top());
	fprintf(m_output, " name=\"%s\"", xml_normalize_string(driver.name));

	// strip away any path information from the source_file and output it
	const char *start = strrchr(driver.source_file, '/');
	if (start == NULL)
		start = strrchr(driver.source_file, '\\');
	if (start == NULL)
		start = driver.source_file - 1;
	fprintf(m_output, " sourcefile=\"%s\"", xml_normalize_string(start + 1));

	// append bios and runnable flags
	if (driver.flags & GAME_IS_BIOS_ROOT)
		fprintf(m_output, " isbios=\"yes\"");
	if (driver.flags & GAME_NO_STANDALONE)
		fprintf(m_output, " runnable=\"no\"");
	if (driver.flags & GAME_MECHANICAL)
		fprintf(m_output, " ismechanical=\"yes\"");

	// display clone information
	int clone_of = m_drivlist.find(driver.parent);
	if (clone_of != -1 && !(m_drivlist.driver(clone_of).flags & GAME_IS_BIOS_ROOT))
		fprintf(m_output, " cloneof=\"%s\"", xml_normalize_string(m_drivlist.driver(clone_of).name));
	if (clone_of != -1)
		fprintf(m_output, " romof=\"%s\"", xml_normalize_string(m_drivlist.driver(clone_of).name));

	// display sample information and close the game tag
	output_sampleof();
	fprintf(m_output, ">\n");

	// output game description
	if (driver.description != NULL)
		fprintf(m_output, "\t\t<description>%s</description>\n", xml_normalize_string(driver.description));

	// print the year only if is a number or another allowed character (? or +)
	if (driver.year != NULL && strspn(driver.year, "0123456789?+") == strlen(driver.year))
		fprintf(m_output, "\t\t<year>%s</year>\n", xml_normalize_string(driver.year));

	// print the manufacturer information
	if (driver.manufacturer != NULL)
		fprintf(m_output, "\t\t<manufacturer>%s</manufacturer>\n", xml_normalize_string(driver.manufacturer));

	// now print various additional information
	output_bios();
	output_rom(rom_first_source(m_drivlist.config()));
	output_device_roms();
	output_sample();
	output_chips();
	output_display();
	output_sound();
	output_input(portlist);
	output_switches(portlist, IPT_DIPSWITCH, "dipswitch", "dipvalue");
	output_switches(portlist, IPT_CONFIG, "configuration", "confsetting");
	output_adjusters(portlist);
	output_driver();
	output_images();
	output_slots();
	output_software_list();
	output_ramoptions();

	// close the topmost tag
	fprintf(m_output, "\t</%s>\n",emulator_info::get_xml_top());
}

//------------------------------------------------
//  output_device_roms - print the device
//  with roms, if appropriate
//-------------------------------------------------

void info_xml_creator::output_device_roms()
{
	int cnt=0;
	for (const rom_source *source = rom_first_source(m_drivlist.config()); source != NULL; source = rom_next_source(*source))
	{
		if (cnt!=0) {
			fprintf(m_output, "\t\t<device_ref name=\"%s\"/>\n", xml_normalize_string(source->shortname()));
			for(int i=0;i<m_device_count;i++) {
				if (source->type() == *s_devices_sorted[i]) m_device_used[i] = 1;
			}
		}
		cnt++;
	}
}

//------------------------------------------------
//  output_sampleof - print the 'sampleof'
//  attribute, if appropriate
//-------------------------------------------------

void info_xml_creator::output_sampleof()
{
	// iterate over sample devices
	samples_device_iterator iter(m_drivlist.config().root_device());
	for (samples_device *device = iter.first(); device != NULL; device = iter.next())
	{
		const char *const *samplenames = ((const samples_interface *)device->static_config())->samplenames;
		if (samplenames != NULL)

			// iterate over sample names
			for (int sampnum = 0; samplenames[sampnum] != NULL; sampnum++)
			{
				// only output sampleof if different from the game name
				const char *cursampname = samplenames[sampnum];
				if (cursampname[0] == '*' && strcmp(cursampname + 1, m_drivlist.driver().name) != 0)
					fprintf(m_output, " sampleof=\"%s\"", xml_normalize_string(cursampname + 1));

				// must stop here, as there can only be one attribute of the same name
				return;
			}
	}
}


//-------------------------------------------------
//  output_bios - print the BIOS set for a
//  game
//-------------------------------------------------

void info_xml_creator::output_bios()
{
	// skip if no ROMs
	if (m_drivlist.driver().rom == NULL)
		return;

	// iterate over ROM entries and look for BIOSes
	for (const rom_entry *rom = m_drivlist.driver().rom; !ROMENTRY_ISEND(rom); rom++)
		if (ROMENTRY_ISSYSTEM_BIOS(rom))
		{
			// output extracted name and descriptions
			fprintf(m_output, "\t\t<biosset");
			fprintf(m_output, " name=\"%s\"", xml_normalize_string(ROM_GETNAME(rom)));
			fprintf(m_output, " description=\"%s\"", xml_normalize_string(ROM_GETHASHDATA(rom)));
			if (ROM_GETBIOSFLAGS(rom) == 1)
				fprintf(m_output, " default=\"yes\"");
			fprintf(m_output, "/>\n");
		}
}


//-------------------------------------------------
//  output_rom - print the roms section of
//  the XML output
//-------------------------------------------------

void info_xml_creator::output_rom(const rom_source *source)
{
	// iterate over 3 different ROM "types": BIOS, ROMs, DISKs
	for (int rom_type = 0; rom_type < 3; rom_type++)
	{
			for (const rom_entry *region = rom_first_region(*source); region != NULL; region = rom_next_region(region))
			{
				bool is_disk = ROMREGION_ISDISKDATA(region);

				// disk regions only work for disks
				if ((is_disk && rom_type != 2) || (!is_disk && rom_type == 2))
					continue;

				// iterate through ROM entries
				for (const rom_entry *rom = rom_first_file(region); rom != NULL; rom = rom_next_file(rom))
				{
					bool is_bios = ROM_GETBIOSFLAGS(rom);
					const char *name = ROM_GETNAME(rom);
					int offset = ROM_GETOFFSET(rom);
					const char *merge_name = NULL;
					char bios_name[100];

					// BIOS ROMs only apply to bioses
					if ((is_bios && rom_type != 0) || (!is_bios && rom_type == 0))
						continue;

					// if we have a valid ROM and we are a clone, see if we can find the parent ROM
					hash_collection hashes(ROM_GETHASHDATA(rom));
					if (!hashes.flag(hash_collection::FLAG_NO_DUMP))
						merge_name = get_merge_name(hashes);

					// scan for a BIOS name
					bios_name[0] = 0;
					if (!is_disk && is_bios)
					{
						// scan backwards through the ROM entries
						for (const rom_entry *brom = rom - 1; brom != m_drivlist.driver().rom; brom--)
							if (ROMENTRY_ISSYSTEM_BIOS(brom))
							{
								strcpy(bios_name, ROM_GETNAME(brom));
								break;
							}
					}

					// opening tag
					if (!is_disk)
						fprintf(m_output, "\t\t<rom");
					else
						fprintf(m_output, "\t\t<disk");

					// add name, merge, bios, and size tags */
					if (name != NULL && name[0] != 0)
						fprintf(m_output, " name=\"%s\"", xml_normalize_string(name));
					if (merge_name != NULL)
						fprintf(m_output, " merge=\"%s\"", xml_normalize_string(merge_name));
					if (bios_name[0] != 0)
						fprintf(m_output, " bios=\"%s\"", xml_normalize_string(bios_name));
					if (!is_disk)
						fprintf(m_output, " size=\"%d\"", rom_file_size(rom));

					// dump checksum information only if there is a known dump
					if (!hashes.flag(hash_collection::FLAG_NO_DUMP))
					{
						// iterate over hash function types and print m_output their values
						astring tempstr;
						for (hash_base *hash = hashes.first(); hash != NULL; hash = hash->next())
							fprintf(m_output, " %s=\"%s\"", hash->name(), hash->string(tempstr));
					}

					// append a region name
					fprintf(m_output, " region=\"%s\"", ROMREGION_GETTAG(region));

					// add nodump/baddump flags
					if (hashes.flag(hash_collection::FLAG_NO_DUMP))
						fprintf(m_output, " status=\"nodump\"");
					if (hashes.flag(hash_collection::FLAG_BAD_DUMP))
						fprintf(m_output, " status=\"baddump\"");

					// for non-disk entries, print offset
					if (!is_disk)
						fprintf(m_output, " offset=\"%x\"", offset);

					// for disk entries, add the disk index
					else
					{
						fprintf(m_output, " index=\"%x\"", DISK_GETINDEX(rom));
						fprintf(m_output, " writable=\"%s\"", DISK_ISREADONLY(rom) ? "no" : "yes");
					}

					// add optional flag
					if ((!is_disk && ROM_ISOPTIONAL(rom)) || (is_disk && DISK_ISOPTIONAL(rom)))
						fprintf(m_output, " optional=\"yes\"");

					fprintf(m_output, "/>\n");
				}
			}
	}
}


//-------------------------------------------------
//  output_sample - print a list of all
//  samples referenced by a game_driver
//-------------------------------------------------

void info_xml_creator::output_sample()
{
	// iterate over sample devices
	samples_device_iterator iter(m_drivlist.config().root_device());
	for (const device_t *device = iter.first(); device != NULL; device = iter.next())
	{
		const char *const *samplenames = ((const samples_interface *)device->static_config())->samplenames;
		if (samplenames != NULL)

			// iterate over sample names
			for (int sampnum = 0; samplenames[sampnum] != NULL; sampnum++)
			{
				// ignore the special '*' samplename
				const char *cursampname = samplenames[sampnum];
				if (sampnum == 0 && cursampname[0] == '*')
					continue;

				// filter m_output duplicates
				int dupnum;
				for (dupnum = 0; dupnum < sampnum; dupnum++)
					if (strcmp(samplenames[dupnum], cursampname) == 0)
						break;
				if (dupnum < sampnum)
					continue;

				// output the sample name
				fprintf(m_output, "\t\t<sample name=\"%s\"/>\n", xml_normalize_string(cursampname));
			}
	}
}


/*-------------------------------------------------
    output_chips - print a list of CPU and
    sound chips used by a game
-------------------------------------------------*/

void info_xml_creator::output_chips()
{
	// iterate over executable devices
	execute_interface_iterator execiter(m_drivlist.config().root_device());
	for (device_execute_interface *exec = execiter.first(); exec != NULL; exec = execiter.next())
	{
		fprintf(m_output, "\t\t<chip");
		fprintf(m_output, " type=\"cpu\"");
		fprintf(m_output, " tag=\"%s\"", xml_normalize_string(exec->device().tag()));
		fprintf(m_output, " name=\"%s\"", xml_normalize_string(exec->device().name()));
		fprintf(m_output, " clock=\"%d\"", exec->device().clock());
		fprintf(m_output, "/>\n");
	}

	// iterate over sound devices
	sound_interface_iterator sounditer(m_drivlist.config().root_device());
	for (device_sound_interface *sound = sounditer.first(); sound != NULL; sound = sounditer.next())
	{
		fprintf(m_output, "\t\t<chip");
		fprintf(m_output, " type=\"audio\"");
		fprintf(m_output, " tag=\"%s\"", xml_normalize_string(sound->device().tag()));
		fprintf(m_output, " name=\"%s\"", xml_normalize_string(sound->device().name()));
		if (sound->device().clock() != 0)
			fprintf(m_output, " clock=\"%d\"", sound->device().clock());
		fprintf(m_output, "/>\n");
	}
}


//-------------------------------------------------
//  output_display - print a list of all the
//  displays
//-------------------------------------------------

void info_xml_creator::output_display()
{
	// iterate over screens
	screen_device_iterator iter(m_drivlist.config().root_device());
	for (const screen_device *device = iter.first(); device != NULL; device = iter.next())
	{
		fprintf(m_output, "\t\t<display");

		switch (device->screen_type())
		{
			case SCREEN_TYPE_RASTER:	fprintf(m_output, " type=\"raster\"");	break;
			case SCREEN_TYPE_VECTOR:	fprintf(m_output, " type=\"vector\"");	break;
			case SCREEN_TYPE_LCD:		fprintf(m_output, " type=\"lcd\"");		break;
			default:					fprintf(m_output, " type=\"unknown\"");	break;
		}

		// output the orientation as a string
		switch (m_drivlist.driver().flags & ORIENTATION_MASK)
		{
			case ORIENTATION_FLIP_X:
				fprintf(m_output, " rotate=\"0\" flipx=\"yes\"");
				break;
			case ORIENTATION_FLIP_Y:
				fprintf(m_output, " rotate=\"180\" flipx=\"yes\"");
				break;
			case ORIENTATION_FLIP_X|ORIENTATION_FLIP_Y:
				fprintf(m_output, " rotate=\"180\"");
				break;
			case ORIENTATION_SWAP_XY:
				fprintf(m_output, " rotate=\"90\" flipx=\"yes\"");
				break;
			case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_X:
				fprintf(m_output, " rotate=\"90\"");
				break;
			case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_Y:
				fprintf(m_output, " rotate=\"270\"");
				break;
			case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_X|ORIENTATION_FLIP_Y:
				fprintf(m_output, " rotate=\"270\" flipx=\"yes\"");
				break;
			default:
				fprintf(m_output, " rotate=\"0\"");
				break;
		}

		// output width and height only for games that are not vector
		if (device->screen_type() != SCREEN_TYPE_VECTOR)
		{
			const rectangle &visarea = device->visible_area();
			fprintf(m_output, " width=\"%d\"", visarea.width());
			fprintf(m_output, " height=\"%d\"", visarea.height());
		}

		// output refresh rate
		fprintf(m_output, " refresh=\"%f\"", ATTOSECONDS_TO_HZ(device->refresh_attoseconds()));

		// output raw video parameters only for games that are not vector
		// and had raw parameters specified
		if (device->screen_type() != SCREEN_TYPE_VECTOR && !device->oldstyle_vblank_supplied())
		{
			int pixclock = device->width() * device->height() * ATTOSECONDS_TO_HZ(device->refresh_attoseconds());

			fprintf(m_output, " pixclock=\"%d\"", pixclock);
			fprintf(m_output, " htotal=\"%d\"", device->width());
			fprintf(m_output, " hbend=\"%d\"", device->visible_area().min_x);
			fprintf(m_output, " hbstart=\"%d\"", device->visible_area().max_x+1);
			fprintf(m_output, " vtotal=\"%d\"", device->height());
			fprintf(m_output, " vbend=\"%d\"", device->visible_area().min_y);
			fprintf(m_output, " vbstart=\"%d\"", device->visible_area().max_y+1);
		}
		fprintf(m_output, " />\n");
	}
}


//-------------------------------------------------
//  output_sound - print a list of all the
//  displays
//------------------------------------------------

void info_xml_creator::output_sound()
{
	speaker_device_iterator spkiter(m_drivlist.config().root_device());
	int speakers = spkiter.count();

	// if we have no sound, zero m_output the speaker count
	sound_interface_iterator snditer(m_drivlist.config().root_device());
	if (snditer.first() == NULL)
		speakers = 0;

	fprintf(m_output, "\t\t<sound channels=\"%d\"/>\n", speakers);
}


//-------------------------------------------------
//  output_input - print a summary of a game's
//  input
//-------------------------------------------------

void info_xml_creator::output_input(const ioport_list &portlist)
{
	// enumerated list of control types
	enum
	{
		ANALOG_TYPE_PADDLE,
		ANALOG_TYPE_PEDAL,
		ANALOG_TYPE_JOYSTICK,
		ANALOG_TYPE_POSITIONAL,
		ANALOG_TYPE_LIGHTGUN,
		ANALOG_TYPE_DIAL,
		ANALOG_TYPE_TRACKBALL,
		ANALOG_TYPE_MOUSE,
		ANALOG_TYPE_COUNT
	};

	// directions
	const UINT8 DIR_UP = 0x01;
	const UINT8 DIR_DOWN = 0x02;
	const UINT8 DIR_LEFT = 0x04;
	const UINT8 DIR_RIGHT = 0x08;
	const UINT8 DIR_4WAY = 0x10;

	// initialize the list of control types
	struct
	{
		const char *	type;			/* general type of input */
		bool			analog;
		bool			keyb;
		INT32			min;			/* analog minimum value */
		INT32			max;			/* analog maximum value  */
		INT32			sensitivity;	/* default analog sensitivity */
		INT32			keydelta;		/* default analog keydelta */
		bool			reverse;		/* default analog reverse setting */
	} control_info[ANALOG_TYPE_COUNT];

	memset(&control_info, 0, sizeof(control_info));

	// tracking info as we iterate
	int nplayer = 0;
	int nbutton = 0;
	int ncoin = 0;
	UINT8 joytype[3] = { 0,0,0 };
	bool service = false;
	bool tilt = false;
	bool keypad = false;
	bool keyboard = false;
	bool mahjong = false;
	bool hanafuda = false;
	bool gambling = false;

	// iterate over the ports
	for (input_port_config *port = portlist.first(); port != NULL; port = port->next())
		for (input_field_config *field = port->fieldlist().first(); field != NULL; field = field->next())
		{
			int analogtype = -1;

			// track the highest player number
			if (nplayer < field->player + 1)
				nplayer = field->player + 1;

			// switch off of the type
			switch (field->type)
			{
				// map which joystick directions are present
				case IPT_JOYSTICK_UP:			joytype[0] |= DIR_UP | ((field->way == 4) ? DIR_4WAY : 0);		break;
				case IPT_JOYSTICK_DOWN:			joytype[0] |= DIR_DOWN | ((field->way == 4) ? DIR_4WAY : 0);	break;
				case IPT_JOYSTICK_LEFT:			joytype[0] |= DIR_LEFT | ((field->way == 4) ? DIR_4WAY : 0);	break;
				case IPT_JOYSTICK_RIGHT:		joytype[0] |= DIR_RIGHT | ((field->way == 4) ? DIR_4WAY : 0);	break;

				case IPT_JOYSTICKLEFT_UP:		joytype[1] |= DIR_UP | ((field->way == 4) ? DIR_4WAY : 0);		break;
				case IPT_JOYSTICKLEFT_DOWN:		joytype[1] |= DIR_DOWN | ((field->way == 4) ? DIR_4WAY : 0);	break;
				case IPT_JOYSTICKLEFT_LEFT:		joytype[1] |= DIR_LEFT | ((field->way == 4) ? DIR_4WAY : 0);	break;
				case IPT_JOYSTICKLEFT_RIGHT:	joytype[1] |= DIR_RIGHT | ((field->way == 4) ? DIR_4WAY : 0);	break;

				case IPT_JOYSTICKRIGHT_UP:		joytype[2] |= DIR_UP | ((field->way == 4) ? DIR_4WAY : 0);		break;
				case IPT_JOYSTICKRIGHT_DOWN:	joytype[2] |= DIR_DOWN | ((field->way == 4) ? DIR_4WAY : 0);	break;
				case IPT_JOYSTICKRIGHT_LEFT:	joytype[2] |= DIR_LEFT | ((field->way == 4) ? DIR_4WAY : 0);	break;
				case IPT_JOYSTICKRIGHT_RIGHT:	joytype[2] |= DIR_RIGHT | ((field->way == 4) ? DIR_4WAY : 0);	break;

				// mark as an analog input, and get analog stats after switch
				case IPT_AD_STICK_X:
				case IPT_AD_STICK_Y:
				case IPT_AD_STICK_Z:
					control_info[analogtype = ANALOG_TYPE_JOYSTICK].type = "stick";
					break;

				case IPT_PADDLE:
				case IPT_PADDLE_V:
					control_info[analogtype = ANALOG_TYPE_PADDLE].type = "paddle";
					break;

				case IPT_PEDAL:
				case IPT_PEDAL2:
				case IPT_PEDAL3:
					control_info[analogtype = ANALOG_TYPE_PEDAL].type = "pedal";
					break;

				case IPT_LIGHTGUN_X:
				case IPT_LIGHTGUN_Y:
					control_info[analogtype = ANALOG_TYPE_LIGHTGUN].type = "lightgun";
					break;

				case IPT_POSITIONAL:
				case IPT_POSITIONAL_V:
					control_info[analogtype = ANALOG_TYPE_POSITIONAL].type = "positional";
					break;

				case IPT_DIAL:
				case IPT_DIAL_V:
					control_info[analogtype = ANALOG_TYPE_DIAL].type = "dial";
					break;

				case IPT_TRACKBALL_X:
				case IPT_TRACKBALL_Y:
					control_info[analogtype = ANALOG_TYPE_TRACKBALL].type = "trackball";
					break;

				case IPT_MOUSE_X:
				case IPT_MOUSE_Y:
					control_info[analogtype = ANALOG_TYPE_MOUSE].type = "mouse";
					break;

				// track maximum button index
				case IPT_BUTTON1:
				case IPT_BUTTON2:
				case IPT_BUTTON3:
				case IPT_BUTTON4:
				case IPT_BUTTON5:
				case IPT_BUTTON6:
				case IPT_BUTTON7:
				case IPT_BUTTON8:
				case IPT_BUTTON9:
				case IPT_BUTTON10:
				case IPT_BUTTON11:
				case IPT_BUTTON12:
				case IPT_BUTTON13:
				case IPT_BUTTON14:
				case IPT_BUTTON15:
				case IPT_BUTTON16:
					nbutton = MAX(nbutton, field->type - IPT_BUTTON1 + 1);
					break;

				// track maximum coin index
				case IPT_COIN1:
				case IPT_COIN2:
				case IPT_COIN3:
				case IPT_COIN4:
				case IPT_COIN5:
				case IPT_COIN6:
				case IPT_COIN7:
				case IPT_COIN8:
					ncoin = MAX(ncoin, field->type - IPT_COIN1 + 1);
					break;

				// track presence of these guys
				case IPT_KEYPAD:
					keypad = true;
					break;

				case IPT_KEYBOARD:
					keyboard = true;
					break;

				// additional types
				case IPT_SERVICE:
					service = true;
					break;

				case IPT_TILT:
					tilt = true;
					break;

				default:
					if (field->type >= __ipt_mahjong_start && field->type <= __ipt_mahjong_end)
						mahjong = true;
					else if (field->type >= __ipt_hanafuda_start && field->type <= __ipt_hanafuda_end)
						hanafuda = true;
					else if (field->type >= __ipt_gambling_start && field->type <= __ipt_gambling_end)
						gambling = true;
					break;
			}

			// get the analog stats
			if (analogtype != -1)
			{
				if (field->min != 0)
					control_info[analogtype].min = field->min;
				if (field->max != 0)
					control_info[analogtype].max = field->max;
				if (field->sensitivity != 0)
					control_info[analogtype].sensitivity = field->sensitivity;
				if (field->delta != 0)
					control_info[analogtype].keydelta = field->delta;
				if ((field->flags & ANALOG_FLAG_REVERSE) != 0)
					control_info[analogtype].reverse = true;
			}
		}

	// output the basic info
	fprintf(m_output, "\t\t<input");
	fprintf(m_output, " players=\"%d\"", nplayer);
	if (nbutton != 0)
		fprintf(m_output, " buttons=\"%d\"", nbutton);
	if (ncoin != 0)
		fprintf(m_output, " coins=\"%d\"", ncoin);
	if (service)
		fprintf(m_output, " service=\"yes\"");
	if (tilt)
		fprintf(m_output, " tilt=\"yes\"");
	fprintf(m_output, ">\n");

	// output the joystick types
	if (joytype[1]==0 && joytype[2]!=0) { joytype[1] = joytype[2]; joytype[2] = 0; }
	if (joytype[0]==0 && joytype[1]!=0) { joytype[0] = joytype[1]; joytype[1] = 0; }
	if (joytype[1]==0 && joytype[2]!=0) { joytype[1] = joytype[2]; joytype[2] = 0; }
	if (joytype[0] != 0)
	{
		const char *joys = (joytype[2]!=0) ? "triple" : (joytype[1]!=0) ? "double" : "";
		fprintf(m_output, "\t\t\t<control type=\"%sjoy\"", joys);
		for (int lp=0; lp<3 && joytype[lp]!=0; lp++)
		{
			const char *plural = (lp==2) ? "3" : (lp==1) ? "2" : "";
			const char *ways;
			switch (joytype[lp] & (DIR_UP | DIR_DOWN | DIR_LEFT | DIR_RIGHT))
			{
				case DIR_UP | DIR_DOWN | DIR_LEFT | DIR_RIGHT:
					ways = ((joytype[lp] & DIR_4WAY) != 0) ? "4" : "8";
					break;
				case DIR_LEFT | DIR_RIGHT:
					ways = "2";
					break;
				case DIR_UP | DIR_DOWN:
					ways = "vertical2";
					break;
				case DIR_UP:
				case DIR_DOWN:
				case DIR_LEFT:
				case DIR_RIGHT:
					ways = "1";
					break;
				case DIR_UP | DIR_DOWN | DIR_LEFT:
				case DIR_UP | DIR_DOWN | DIR_RIGHT:
				case DIR_UP | DIR_LEFT | DIR_RIGHT:
				case DIR_DOWN | DIR_LEFT | DIR_RIGHT:
					ways = ((joytype[lp] & DIR_4WAY) != 0) ? "3 (half4)" : "5 (half8)";
					break;
				default:
					ways = "strange2";
					break;
			}
			fprintf(m_output, " ways%s=\"%s\"", plural,ways);
		}
		fprintf(m_output, "/>\n");
	}

	// output analog types
	for (int type = 0; type < ANALOG_TYPE_COUNT; type++)
		if (control_info[type].type != NULL)
		{
			fprintf(m_output, "\t\t\t<control type=\"%s\"", xml_normalize_string(control_info[type].type));
			if (control_info[type].min != 0 || control_info[type].max != 0)
			{
				fprintf(m_output, " minimum=\"%d\"", control_info[type].min);
				fprintf(m_output, " maximum=\"%d\"", control_info[type].max);
			}
			if (control_info[type].sensitivity != 0)
				fprintf(m_output, " sensitivity=\"%d\"", control_info[type].sensitivity);
			if (control_info[type].keydelta != 0)
				fprintf(m_output, " keydelta=\"%d\"", control_info[type].keydelta);
			if (control_info[type].reverse)
				fprintf(m_output, " reverse=\"yes\"");

			fprintf(m_output, "/>\n");
		}

	// output keypad and keyboard
	if (keypad)
		fprintf(m_output, "\t\t\t<control type=\"keypad\"/>\n");
	if (keyboard)
		fprintf(m_output, "\t\t\t<control type=\"keyboard\"/>\n");

	// misc
	if (mahjong)
		fprintf(m_output, "\t\t\t<control type=\"mahjong\"/>\n");
	if (hanafuda)
		fprintf(m_output, "\t\t\t<control type=\"hanafuda\"/>\n");
	if (gambling)
		fprintf(m_output, "\t\t\t<control type=\"gambling\"/>\n");

	fprintf(m_output, "\t\t</input>\n");
}


//-------------------------------------------------
//  output_switches - print the configurations or
//  DIP switch settings
//-------------------------------------------------

void info_xml_creator::output_switches(const ioport_list &portlist, int type, const char *outertag, const char *innertag)
{
	// iterate looking for DIP switches
	for (input_port_config *port = portlist.first(); port != NULL; port = port->next())
		for (input_field_config *field = port->fieldlist().first(); field != NULL; field = field->next())
			if (field->type == type)
			{
				// output the switch name information
				fprintf(m_output, "\t\t<%s name=\"%s\"", outertag, xml_normalize_string(input_field_name(field)));
				fprintf(m_output, " tag=\"%s\"", xml_normalize_string(field->port().tag()));
				fprintf(m_output, " mask=\"%u\"", field->mask);
				fprintf(m_output, ">\n");

				// loop over settings
				for (input_setting_config *setting = field->settinglist().first(); setting != NULL; setting = setting->next())
				{
					fprintf(m_output, "\t\t\t<%s name=\"%s\"", innertag, xml_normalize_string(setting->name));
					fprintf(m_output, " value=\"%u\"", setting->value);
					if (setting->value == field->defvalue)
						fprintf(m_output, " default=\"yes\"");
					fprintf(m_output, "/>\n");
				}

				// terminate the switch entry
				fprintf(m_output, "\t\t</%s>\n", outertag);
			}
}


//-------------------------------------------------
//  output_adjusters - print the Analog
//  Adjusters for a game
//-------------------------------------------------

void info_xml_creator::output_adjusters(const ioport_list &portlist)
{
	// iterate looking for Adjusters
	for (input_port_config *port = portlist.first(); port != NULL; port = port->next())
		for (input_field_config *field = port->fieldlist().first(); field != NULL; field = field->next())
			if (field->type == IPT_ADJUSTER)
				fprintf(m_output, "\t\t<adjuster name=\"%s\" default=\"%d\"/>\n", xml_normalize_string(input_field_name(field)), field->defvalue);
}


//-------------------------------------------------
//  output_driver - print driver status
//-------------------------------------------------

void info_xml_creator::output_driver()
{
	fprintf(m_output, "\t\t<driver");

	/* The status entry is an hint for frontend authors */
	/* to select working and not working games without */
	/* the need to know all the other status entries. */
	/* Games marked as status=good are perfectly emulated, games */
	/* marked as status=imperfect are emulated with only */
	/* some minor issues, games marked as status=preliminary */
	/* don't work or have major emulation problems. */

	if (m_drivlist.driver().flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_NO_SOUND | GAME_WRONG_COLORS | GAME_MECHANICAL))
		fprintf(m_output, " status=\"preliminary\"");
	else if (m_drivlist.driver().flags & (GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS))
		fprintf(m_output, " status=\"imperfect\"");
	else
		fprintf(m_output, " status=\"good\"");

	if (m_drivlist.driver().flags & GAME_NOT_WORKING)
		fprintf(m_output, " emulation=\"preliminary\"");
	else
		fprintf(m_output, " emulation=\"good\"");

	if (m_drivlist.driver().flags & GAME_WRONG_COLORS)
		fprintf(m_output, " color=\"preliminary\"");
	else if (m_drivlist.driver().flags & GAME_IMPERFECT_COLORS)
		fprintf(m_output, " color=\"imperfect\"");
	else
		fprintf(m_output, " color=\"good\"");

	if (m_drivlist.driver().flags & GAME_NO_SOUND)
		fprintf(m_output, " sound=\"preliminary\"");
	else if (m_drivlist.driver().flags & GAME_IMPERFECT_SOUND)
		fprintf(m_output, " sound=\"imperfect\"");
	else
		fprintf(m_output, " sound=\"good\"");

	if (m_drivlist.driver().flags & GAME_IMPERFECT_GRAPHICS)
		fprintf(m_output, " graphic=\"imperfect\"");
	else
		fprintf(m_output, " graphic=\"good\"");

	if (m_drivlist.driver().flags & GAME_NO_COCKTAIL)
		fprintf(m_output, " cocktail=\"preliminary\"");

	if (m_drivlist.driver().flags & GAME_UNEMULATED_PROTECTION)
		fprintf(m_output, " protection=\"preliminary\"");

	if (m_drivlist.driver().flags & GAME_SUPPORTS_SAVE)
		fprintf(m_output, " savestate=\"supported\"");
	else
		fprintf(m_output, " savestate=\"unsupported\"");

	fprintf(m_output, " palettesize=\"%d\"", m_drivlist.config().m_total_colors);

	fprintf(m_output, "/>\n");
}


//-------------------------------------------------
//  output_images - prints m_output all info on
//  image devices
//-------------------------------------------------

void info_xml_creator::output_images()
{
	image_interface_iterator iter(m_drivlist.config().root_device());
	for (const device_image_interface *dev = iter.first(); dev != NULL; dev = iter.next())
	{
		// print m_output device type
		fprintf(m_output, "\t\t<device type=\"%s\"", xml_normalize_string(dev->image_type_name()));

		// does this device have a tag?
		if (dev->device().tag())
			fprintf(m_output, " tag=\"%s\"", xml_normalize_string(dev->device().tag()));

		// is this device mandatory?
		if (dev->must_be_loaded())
			fprintf(m_output, " mandatory=\"1\"");

		if (dev->image_interface() && dev->image_interface()[0])
			fprintf(m_output, " interface=\"%s\"", xml_normalize_string(dev->image_interface()));

		// close the XML tag
		fprintf(m_output, ">\n");

		const char *name = dev->instance_name();
		const char *shortname = dev->brief_instance_name();

		fprintf(m_output, "\t\t\t<instance");
		fprintf(m_output, " name=\"%s\"", xml_normalize_string(name));
		fprintf(m_output, " briefname=\"%s\"", xml_normalize_string(shortname));
		fprintf(m_output, "/>\n");

		astring extensions(dev->file_extensions());

		char *ext = strtok((char *)extensions.cstr(), ",");
		while (ext != NULL)
		{
			fprintf(m_output, "\t\t\t<extension");
			fprintf(m_output, " name=\"%s\"", xml_normalize_string(ext));
			fprintf(m_output, "/>\n");
			ext = strtok(NULL, ",");
		}

		fprintf(m_output, "\t\t</device>\n");
	}
}


//-------------------------------------------------
//  output_images - prints all info about slots
//-------------------------------------------------

void info_xml_creator::output_slots()
{
	slot_interface_iterator iter(m_drivlist.config().root_device());
	for (const device_slot_interface *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		// print m_output device type
		fprintf(m_output, "\t\t<slot name=\"%s\">\n", xml_normalize_string(slot->device().tag()+1));

		/*
        if (slot->slot_interface()[0])
            fprintf(m_output, " interface=\"%s\"", xml_normalize_string(slot->slot_interface()));
         */

		const slot_interface* intf = slot->get_slot_interfaces();
		for (int i = 0; intf[i].name != NULL; i++)
		{
			fprintf(m_output, "\t\t\t<slotoption");
			fprintf(m_output, " name=\"%s\"", xml_normalize_string(intf[i].name));
			if (slot->get_default_card(m_drivlist.config(), m_drivlist.options()))
			{
				if (slot->get_default_card(m_drivlist.config(), m_drivlist.options()) == intf[i].name)
					fprintf(m_output, " default=\"yes\"");
			}
			fprintf(m_output, "/>\n");
		}

		fprintf(m_output, "\t\t</slot>\n");
	}
}


//-------------------------------------------------
//  output_software_list - print the information
//  for all known software lists for this system
//-------------------------------------------------

void info_xml_creator::output_software_list()
{
	software_list_device_iterator iter(m_drivlist.config().root_device());
	for (const software_list_device *swlist = iter.first(); swlist != NULL; swlist = iter.next())
	{
		fprintf(m_output, "\t\t<softwarelist name=\"%s\" ", swlist->list_name());
		fprintf(m_output, "status=\"%s\" ", (swlist->list_type() == SOFTWARE_LIST_ORIGINAL_SYSTEM) ? "original" : "compatible");
		if (swlist->filter()) {
			fprintf(m_output, "filter=\"%s\" ", swlist->filter());
		}
		fprintf(m_output, "/>\n");
	}
}



//-------------------------------------------------
//  output_ramoptions - prints m_output all RAM
//  options for this system
//-------------------------------------------------

void info_xml_creator::output_ramoptions()
{
	ram_device_iterator iter(m_drivlist.config().root_device());
	for (const ram_device *ram = iter.first(); ram != NULL; ram = iter.next())
	{
		fprintf(m_output, "\t\t<ramoption default=\"1\">%u</ramoption>\n", ram->default_size());

		if (ram->extra_options() != NULL)
		{
			astring options(ram->extra_options());
			for (int start = 0, end = options.chr(0, ','); ; start = end + 1, end = options.chr(start, ','))
			{
				astring option;
				option.cpysubstr(options, start, (end == -1) ? -1 : end - start);
				fprintf(m_output, "\t\t<ramoption>%u</ramoption>\n", ram_device::parse_string(option));
				if (end == -1)
					break;
			}
		}
	}
}


//-------------------------------------------------
//  get_merge_name - get the rom name from a
//  parent set
//-------------------------------------------------

const char *info_xml_creator::get_merge_name(const hash_collection &romhashes)
{
	const char *merge_name = NULL;
	// walk the parent chain
	for (int clone_of = m_drivlist.find(m_drivlist.driver().parent); clone_of != -1; clone_of = m_drivlist.find(m_drivlist.driver(clone_of).parent))

		// look in the parent's ROMs
		for (const rom_source *psource = rom_first_source(m_drivlist.config(clone_of,m_lookup_options)); psource != NULL; psource = rom_next_source(*psource))
			for (const rom_entry *pregion = rom_first_region(*psource); pregion != NULL; pregion = rom_next_region(pregion))
				for (const rom_entry *prom = rom_first_file(pregion); prom != NULL; prom = rom_next_file(prom))
				{
					hash_collection phashes(ROM_GETHASHDATA(prom));
					if (!phashes.flag(hash_collection::FLAG_NO_DUMP) && romhashes == phashes)
					{
						// stop when we find a match
						merge_name = ROM_GETNAME(prom);
						break;
					}
				}

	return merge_name;
}
