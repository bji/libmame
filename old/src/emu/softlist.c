/***************************************************************************

    softlist.c

    Software list construction helpers.


***************************************************************************/

#include "emu.h"
#include "pool.h"
#include "expat.h"
#include "emuopts.h"
#include "hash.h"
#include "softlist.h"

#include <ctype.h>

enum softlist_parse_position
{
	POS_ROOT,
	POS_MAIN,
	POS_SOFT,
	POS_PART,
	POS_DATA
};


typedef struct _parse_state
{
	XML_Parser	parser;
	int			done;

	void (*error_proc)(const char *message);
	void *param;

	enum softlist_parse_position pos;
	char **text_dest;
} parse_state;


struct _software_list
{
	emu_file	*file;
	object_pool	*pool;
	parse_state	state;
	const char *description;
	struct software_info	*software_info_list;
	struct software_info	*current_software_info;
	software_info	*softinfo;
	const char *look_for;
	int part_entries;
	int current_part_entry;
	int rom_entries;
	int current_rom_entry;
	void (*error_proc)(const char *message);
	int list_entries;
};


typedef tagmap_t<software_info *> softlist_map;

/***************************************************************************
    EXPAT INTERFACES
***************************************************************************/

/*-------------------------------------------------
    expat_malloc/expat_realloc/expat_free -
    wrappers for memory allocation functions so
    that they pass through out memory tracking
    systems
-------------------------------------------------*/

static void *expat_malloc(size_t size)
{
	return global_alloc_array_clear(UINT8,size);
}

static void *expat_realloc(void *ptr, size_t size)
{
	if (ptr) global_free(ptr);
	return global_alloc_array_clear(UINT8,size);
}

static void expat_free(void *ptr)
{
	global_free(ptr);
}


/*-------------------------------------------------
    parse_error
-------------------------------------------------*/

INLINE void ATTR_PRINTF(2,3) parse_error(parse_state *state, const char *fmt, ...)
{
	char buf[256];
	va_list va;

	if (state->error_proc)
	{
		va_start(va, fmt);
		vsnprintf(buf, ARRAY_LENGTH(buf), fmt, va);
		va_end(va);
		(*state->error_proc)(buf);
	}
}


/*-------------------------------------------------
    unknown_tag
-------------------------------------------------*/

INLINE void unknown_tag(parse_state *state, const char *tagname)
{
	parse_error(state, "[%lu:%lu]: Unknown tag: %s\n",
		XML_GetCurrentLineNumber(state->parser),
		XML_GetCurrentColumnNumber(state->parser),
		tagname);
}



/*-------------------------------------------------
    unknown_attribute
-------------------------------------------------*/

INLINE void unknown_attribute(parse_state *state, const char *attrname)
{
	parse_error(state, "[%lu:%lu]: Unknown attribute: %s\n",
		XML_GetCurrentLineNumber(state->parser),
		XML_GetCurrentColumnNumber(state->parser),
		attrname);
}



/*-------------------------------------------------
    unknown_attribute_value
-------------------------------------------------*/

INLINE void unknown_attribute_value(parse_state *state,
	const char *attrname, const char *attrvalue)
{
	parse_error(state, "[%lu:%lu]: Unknown attribute value: %s\n",
		XML_GetCurrentLineNumber(state->parser),
		XML_GetCurrentColumnNumber(state->parser),
		attrvalue);
}


/*-------------------------------------------------
    software_name_split
    helper; splits a software_list:software:part
    string into seperate software_list, software,
    and part strings.

    str1:str2:str3  => swlist_name - str1, swname - str2, swpart - str3
    str1:str2       => swlist_name - NULL, swname - str1, swpart - str2
    str1            => swlist_name - NULL, swname - str1, swpart - NULL

    swlist_namem, swnane and swpart will be global_alloc'ed
    from the global pool. So they should be global_free'ed
    when they are not used anymore.
-------------------------------------------------*/
static void software_name_split(running_machine* machine, const char *swlist_swname, char **swlist_name, char **swname, char **swpart )
{
	const char *split_1st_loc = strchr( swlist_swname, ':' );
	const char *split_2nd_loc = ( split_1st_loc ) ? strchr( split_1st_loc + 1, ':' ) : NULL;

	*swlist_name = NULL;
	*swname = NULL;
	*swpart = NULL;

	if ( split_1st_loc )
	{
		if ( split_2nd_loc )
		{
			int size = split_1st_loc - swlist_swname;
			*swlist_name = auto_alloc_array_clear(machine,char,size+1);
			memcpy( *swlist_name, swlist_swname, size );

			size = split_2nd_loc - ( split_1st_loc + 1 );
			*swname = auto_alloc_array_clear(machine,char,size+1);
			memcpy( *swname, split_1st_loc + 1, size );

			size = strlen( swlist_swname ) - ( split_2nd_loc + 1 - swlist_swname );
			*swpart = auto_alloc_array_clear(machine,char,size+1);
			memcpy( *swpart, split_2nd_loc + 1, size );
		}
		else
		{
			int size = split_1st_loc - swlist_swname;
			*swname = auto_alloc_array_clear(machine,char,size+1);
			memcpy( *swname, swlist_swname, size );

			size = strlen( swlist_swname ) - ( split_1st_loc + 1 - swlist_swname );
			*swpart = auto_alloc_array_clear(machine,char,size+1);
			memcpy( *swpart, split_1st_loc + 1, size );
		}
	}
	else
	{
		*swname = auto_strdup(machine,swlist_swname);
	}
}


/*-------------------------------------------------
    add_rom_entry
-------------------------------------------------*/

static void add_rom_entry(software_list *swlist, const char *name, const char *hashdata, UINT32 offset, UINT32 length, UINT32 flags)
{
	software_part *part = &swlist->softinfo->partdata[swlist->current_part_entry-1];
	struct rom_entry *entry = &part->romdata[swlist->current_rom_entry];

	entry->_name = name;
	entry->_hashdata = hashdata;
	entry->_offset = offset;
	entry->_length = length;
	entry->_flags = flags;

	swlist->current_rom_entry += 1;

	if ( swlist->current_rom_entry >= swlist->rom_entries )
	{
		struct rom_entry *new_entries;

		swlist->rom_entries += 10;
		new_entries = (struct rom_entry *)pool_realloc_lib(swlist->pool, part->romdata, swlist->rom_entries * sizeof(struct rom_entry) );

		if ( new_entries )
		{
			part->romdata = new_entries;
		}
		else
		{
			/* Allocation error */
			swlist->current_rom_entry -= 1;
		}
	}
}

/*-------------------------------------------------
    add_feature
-------------------------------------------------*/

static void add_feature(software_list *swlist, char *feature_name, char *feature_value)
{
	software_part *part = &swlist->softinfo->partdata[swlist->current_part_entry-1];
	feature_list *new_entry;

	/* First allocate the new entry */
	new_entry = (feature_list *)pool_malloc_lib(swlist->pool, sizeof(feature_list) );

	if ( new_entry )
	{
		new_entry->next = NULL;
		new_entry->name = feature_name;
		new_entry->value = feature_value ? feature_value : feature_name;

		/* Add new feature to end of feature list */
		if ( part->featurelist )
		{
			feature_list *list = part->featurelist;
			while ( list->next != NULL )
			{
				list = list->next;
			}
			list->next = new_entry;
		}
		else
		{
			part->featurelist = new_entry;
		}
	}
	else
	{
		/* Unable to allocate memory */
	}
}

/*-------------------------------------------------
 add_info (same as add_feature, but its target
 is softinfo->other_info)
 -------------------------------------------------*/

static void add_info(software_list *swlist, char *feature_name, char *feature_value)
{
	software_info *info = swlist->softinfo;
	feature_list *new_entry;

	/* First allocate the new entry */
	new_entry = (feature_list *)pool_malloc_lib(swlist->pool, sizeof(feature_list) );

	if ( new_entry )
	{
		new_entry->next = NULL;
		new_entry->name = feature_name;
		new_entry->value = feature_value ? feature_value : feature_name;

		/* Add new feature to end of feature list */
		if ( info->other_info )
		{
			feature_list *list = info->other_info;
			while ( list->next != NULL )
			{
				list = list->next;
			}
			list->next = new_entry;
		}
		else
		{
			info->other_info = new_entry;
		}
	}
	else
	{
		/* Unable to allocate memory */
	}
}

/*-------------------------------------------------
    add_software_part
-------------------------------------------------*/

static void add_software_part(software_list *swlist, const char *name, const char *interface)
{
	software_part *part = &swlist->softinfo->partdata[swlist->current_part_entry];

	part->name = name;
	part->interface_ = interface;
	part->featurelist = NULL;
	part->romdata = NULL;

	swlist->current_part_entry += 1;

	if ( swlist->current_part_entry >= swlist->part_entries )
	{
		software_part *new_parts;

		swlist->part_entries += 2;
		new_parts = (software_part *)pool_realloc_lib(swlist->pool, swlist->softinfo->partdata, swlist->part_entries * sizeof(software_part) );

		if ( new_parts )
		{
			swlist->softinfo->partdata = new_parts;
		}
		else
		{
			/* Allocation error */
			swlist->current_part_entry -= 1;
		}
	}
}


/*-------------------------------------------------
    start_handler
-------------------------------------------------*/

static void start_handler(void *data, const char *tagname, const char **attributes)
{
	software_list *swlist = (software_list *) data;
	char **text_dest;

	switch(swlist->state.pos)
	{
		case POS_ROOT:
			if (!strcmp(tagname, "softwarelist"))
			{
				for( ; attributes[0]; attributes += 2 )
				{
					if ( ! strcmp(attributes[0], "name" ) )
					{
					}
					if ( ! strcmp(attributes[0], "description" ) )
					{
						swlist->description =  (const char *)pool_malloc_lib(swlist->pool, (strlen(attributes[1])  + 1) * sizeof(char));
						if (!swlist->description)
							return;

						strcpy((char *)swlist->description, attributes[1]);
					}
				}
			}
			else
			{
				unknown_tag(&swlist->state, tagname);
			}
			break;

		case POS_MAIN:
			if ( !strcmp( tagname, "software" ) )
			{
				const char *name = NULL;
				const char *parent = NULL;
				const char *supported = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
					{
						name = attributes[1];
					}
					if ( !strcmp( attributes[0], "cloneof" ) )
					{
						parent = attributes[1];
					}
					if ( !strcmp( attributes[0], "supported" ) )
					{
						supported = attributes[1];
					}
				}

				if ( name )
				{
					struct software_info *elem = (struct software_info *)pool_malloc_lib(swlist->pool,sizeof(struct software_info));

					if ( !elem )
						return;

					/* Clear element and add element to list */
					memset(elem,0,sizeof(struct software_info));

					/* Allocate space to hold the shortname and copy the short name */
					elem->shortname = (const char*)pool_malloc_lib(swlist->pool, ( strlen( name ) + 1 ) * sizeof(char) );

					if ( ! elem->shortname )
						return;

					strcpy( (char *)elem->shortname, name );

					/* Allocate space to hold the parentname and copy the parent name */
					if (parent)
					{
						elem->parentname = (const char*)pool_malloc_lib(swlist->pool, ( strlen(parent) + 1 ) * sizeof(char) );
						strcpy((char *)elem->parentname, parent);
					}

					/* Allocate initial space to hold part information */
					swlist->part_entries = 2;
					swlist->current_part_entry = 0;
					elem->partdata = (software_part *)pool_malloc_lib(swlist->pool, swlist->part_entries * sizeof(software_part) );
					if ( !elem->partdata )
						return;
					elem->other_info = (feature_list *)pool_malloc_lib(swlist->pool, sizeof(feature_list) );
					if ( !elem->other_info )
						return;
					else
					{
						elem->other_info->next = (feature_list *)pool_malloc_lib(swlist->pool, sizeof(feature_list) );
						elem->other_info->next = NULL;
						elem->other_info->name = NULL;
						elem->other_info->value = NULL;
					}

					/* Handle the supported flag */
					elem->supported = SOFTWARE_SUPPORTED_YES;
					if ( supported && ! strcmp( supported, "partial" ) )
						elem->supported = SOFTWARE_SUPPORTED_PARTIAL;
					if ( supported && ! strcmp( supported, "no" ) )
						elem->supported = SOFTWARE_SUPPORTED_NO;

					/* Add the entry to the end of the list */
					if ( swlist->software_info_list == NULL )
					{
						swlist->software_info_list = elem;
						swlist->current_software_info = elem;
					}
					else
					{
						swlist->current_software_info->next = elem;
						swlist->current_software_info = elem;
					}

					/* Quick lookup for setting software information */
					swlist->softinfo = swlist->current_software_info;
				}
				else
				{
					swlist->softinfo = NULL;
				}
			}
			else
			{
				unknown_tag(&swlist->state, tagname);
			}
			break;

		case POS_SOFT:
			text_dest = NULL;

			if (!strcmp(tagname, "description"))
				text_dest = (char **) &swlist->softinfo->longname;
			else if (!strcmp(tagname, "year"))
				text_dest = (char **) &swlist->softinfo->year;
			else if (!strcmp(tagname, "publisher"))
				text_dest = (char **) &swlist->softinfo->publisher;
			else if ( !strcmp(tagname, "info") )
			{
				const char *str_feature_name = NULL;
				const char *str_feature_value = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_feature_name = attributes[1];

					if ( !strcmp( attributes[0], "value" ) )
						str_feature_value = attributes[1];
				}

				/* Prepare for adding feature to feature list */
				if ( str_feature_name && swlist->softinfo )
				{
					char *name = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_feature_name ) + 1 ) * sizeof(char) );
					char *value = NULL;

					if ( !name )
						return;

					strcpy( name, str_feature_name );

					if ( str_feature_value )
					{
						value = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_feature_value ) + 1 ) * sizeof(char) );

						if ( !value )
							return;

						strcpy( value, str_feature_value );
					}

					add_info( swlist, name, value );
				}
			}
			else if ( !strcmp(tagname, "part" ) )
			{
				const char *str_name = NULL;
				const char *str_interface = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_name = attributes[1];

					if ( !strcmp( attributes[0], "interface" ) )
						str_interface = attributes[1];
				}

				if ( str_name && str_interface )
				{
					if ( swlist->softinfo )
					{
						char *name = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_name ) + 1 ) * sizeof(char) );
						char *interface = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_interface ) + 1 ) * sizeof(char) );

						if ( !name || !interface )
							return;

						strcpy( name, str_name );
						strcpy( interface, str_interface );

						add_software_part( swlist, name, interface );

						/* Allocate initial space to hold the rom information */
						swlist->rom_entries = 3;
						swlist->current_rom_entry = 0;
						swlist->softinfo->partdata[swlist->current_part_entry-1].romdata = (struct rom_entry *)pool_malloc_lib(swlist->pool, swlist->rom_entries * sizeof(struct rom_entry));
						if ( ! swlist->softinfo->partdata[swlist->current_part_entry-1].romdata )
							return;
					}
				}
				else
				{
					/* Incomplete/incorrect part definition */
				}
			}
			else
				unknown_tag(&swlist->state, tagname);

			if (text_dest && swlist->softinfo)
				swlist->state.text_dest = text_dest;
			break;

		case POS_PART:
			if (!strcmp(tagname, "dataarea"))
			{
				const char *str_name = NULL;
				const char *str_size = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_name = attributes[1];

					if ( !strcmp( attributes[0], "size") )
						str_size = attributes[1];
				}
				if ( str_name && str_size )
				{
					if ( swlist->softinfo )
					{
						UINT32 length = strtol( str_size, NULL, 0 );
						char *s = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_name ) + 1 ) * sizeof(char) );

						if ( !s )
							return;

						strcpy( s, str_name );

						/* ROM_REGION( length, "name", flags ) */
						add_rom_entry( swlist, s, NULL, 0, length, ROMENTRYTYPE_REGION );
					}
				}
				else
				{
					/* Missing dataarea name or size */
				}
			}
			else if (!strcmp(tagname, "diskarea"))
			{
				const char *str_name = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_name = attributes[1];
				}
				if ( str_name )
				{
					if ( swlist->softinfo )
					{
						char *s = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_name ) + 1 ) * sizeof(char) );

						if ( !s )
							return;

						strcpy( s, str_name );

						/* ROM_REGION( length, "name", flags ) */
						add_rom_entry( swlist, s, NULL, 0, 1, ROMENTRYTYPE_REGION | ROMREGION_DATATYPEDISK);
					}
				}
				else
				{
					/* Missing dataarea name or size */
				}
			}
			else if ( !strcmp(tagname, "feature") )
			{
				const char *str_feature_name = NULL;
				const char *str_feature_value = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_feature_name = attributes[1];

					if ( !strcmp( attributes[0], "value" ) )
						str_feature_value = attributes[1];
				}

				/* Prepare for adding feature to feature list */
				if ( str_feature_name && swlist->softinfo )
				{
					char *name = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_feature_name ) + 1 ) * sizeof(char) );
					char *value = NULL;

					if ( !name )
						return;

					strcpy( name, str_feature_name );

					if ( str_feature_value )
					{
						value = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_feature_value ) + 1 ) * sizeof(char) );

						if ( !value )
							return;

						strcpy( value, str_feature_value );
					}

					add_feature( swlist, name, value );
				}
			}
			else
				unknown_tag( &swlist->state, tagname );
			break;

		case POS_DATA:
			if (!strcmp(tagname, "rom"))
			{
				const char *str_name = NULL;
				const char *str_size = NULL;
				const char *str_crc = NULL;
				const char *str_sha1 = NULL;
				const char *str_offset = NULL;
				const char *str_value = NULL;
				const char *str_status = NULL;
				const char *str_loadflag = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_name = attributes[1];
					if ( !strcmp( attributes[0], "size" ) )
						str_size = attributes[1];
					if ( !strcmp( attributes[0], "crc" ) )
						str_crc = attributes[1];
					if ( !strcmp( attributes[0], "sha1" ) )
						str_sha1 = attributes[1];
					if ( !strcmp( attributes[0], "offset" ) )
						str_offset = attributes[1];
					if ( !strcmp( attributes[0], "value" ) )
						str_value = attributes[1];
					if ( !strcmp( attributes[0], "status" ) )
						str_status = attributes[1];
					if ( !strcmp( attributes[0], "loadflag" ) )
						str_loadflag = attributes[1];
				}
				if ( swlist->softinfo )
				{
					if ( str_size && str_offset )
					{
						UINT32 length = strtol( str_size, NULL, 0 );
						UINT32 offset = strtol( str_offset, NULL, 0 );

						if ( str_loadflag && !strcmp(str_loadflag, "reload") )
						{
							/* Handle 'reload' loadflag */
							add_rom_entry( swlist, NULL, NULL, offset, length, ROMENTRYTYPE_RELOAD | ROM_INHERITFLAGS );
						}
						else if ( str_loadflag && !strcmp(str_loadflag, "continue") )
						{
							/* Handle 'continue' loadflag */
							add_rom_entry( swlist, NULL, NULL, offset, length, ROMENTRYTYPE_CONTINUE | ROM_INHERITFLAGS );
						}
						else if ( str_loadflag && !strcmp(str_loadflag, "fill") )
						{
							/* Handle 'fill' loadflag */
							add_rom_entry( swlist, NULL, (const char*)atoi(str_value), offset, length, ROMENTRYTYPE_FILL );
						}
						else
						{
							if ( str_name && str_crc && str_sha1 )
							{
								char *s_name = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_name ) + 1 ) * sizeof(char) );
								char *hashdata = (char *)pool_malloc_lib( swlist->pool, sizeof(char) * ( strlen(str_crc) + strlen(str_sha1) + 7 + 4 ) );
								int baddump = ( str_status && !strcmp(str_status, "baddump") ) ? 1 : 0;
								int nodump = ( str_status && !strcmp(str_status, "nodump" ) ) ? 1 : 0;
								int romflags = 0;

								if ( !s_name || !hashdata )
									return;

								strcpy( s_name, str_name );
								sprintf( hashdata, "%c%s%c%s%s", hash_collection::HASH_CRC, str_crc, hash_collection::HASH_SHA1, str_sha1, ( nodump ? NO_DUMP : ( baddump ? BAD_DUMP : "" ) ) );

								/* Handle loadflag attribute */
								if ( str_loadflag && !strcmp(str_loadflag, "load16_word_swap") )
									romflags = ROM_GROUPWORD | ROM_REVERSE;
								else if ( str_loadflag && !strcmp(str_loadflag, "load16_byte") )
									romflags = ROM_SKIP(1);
								else if ( str_loadflag && !strcmp(str_loadflag, "load32_word_swap") )
									romflags = ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2);
								else if ( str_loadflag && !strcmp(str_loadflag, "load32_word") )
									romflags = ROM_GROUPWORD | ROM_SKIP(2);

								/* ROM_LOAD( name, offset, length, hash ) */
								add_rom_entry( swlist, s_name, hashdata, offset, length, ROMENTRYTYPE_ROM | romflags );
							}
						}
					}
				}
				else
				{
					/* Missing name, size, crc, sha1, or offset */
				}
			}
			else
			if (!strcmp(tagname, "disk"))
			{
				const char *str_name = NULL;
				const char *str_sha1 = NULL;
				const char *str_status = NULL;
				const char *str_writeable = NULL;

				for ( ; attributes[0]; attributes += 2 )
				{
					if ( !strcmp( attributes[0], "name" ) )
						str_name = attributes[1];
					if ( !strcmp( attributes[0], "sha1" ) )
						str_sha1 = attributes[1];
					if ( !strcmp( attributes[0], "status" ) )
						str_status = attributes[1];
					if ( !strcmp( attributes[0], "writeable" ) )
						str_writeable = attributes[1];
				}
				if ( swlist->softinfo )
				{
					if ( str_name && str_sha1 )
					{
						char *s_name = (char *)pool_malloc_lib(swlist->pool, ( strlen( str_name ) + 1 ) * sizeof(char) );
						char *hashdata = (char *)pool_malloc_lib( swlist->pool, sizeof(char) * ( strlen(str_sha1) + 7 + 4 ) );
						int baddump = ( str_status && !strcmp(str_status, "baddump") ) ? 1 : 0;
						int nodump = ( str_status && !strcmp(str_status, "nodump" ) ) ? 1 : 0;
						int writeable = ( str_writeable && !strcmp(str_writeable, "yes" ) ) ? 1 : 0;

						if ( !s_name || !hashdata )
							return;

						strcpy( s_name, str_name );
						sprintf( hashdata, "%c%s%s", hash_collection::HASH_SHA1, str_sha1, ( nodump ? NO_DUMP : ( baddump ? BAD_DUMP : "" ) ) );

						add_rom_entry( swlist, s_name, hashdata, 0, 0, ROMENTRYTYPE_ROM | (writeable ? DISK_READWRITE : DISK_READONLY ) );
					}
				}
			}
			else
				unknown_tag(&swlist->state, tagname);
			break;
	}
	swlist->state.pos = (softlist_parse_position) (swlist->state.pos + 1);
}

/*-------------------------------------------------
    end_handler
-------------------------------------------------*/

static void end_handler(void *data, const char *name)
{
	software_list *swlist = (software_list *) data;
	swlist->state.text_dest = NULL;

	swlist->state.pos = (softlist_parse_position) (swlist->state.pos - 1);
	switch(swlist->state.pos)
	{
		case POS_ROOT:
			break;

		case POS_SOFT:
			if ( ! strcmp( name, "part" ) && swlist->softinfo )
			{
				/* ROM_END */
				add_rom_entry( swlist, NULL, NULL, 0, 0, ROMENTRYTYPE_END );
			}
			break;

		case POS_MAIN:
			if ( swlist->softinfo )
			{
				add_software_part( swlist, NULL, NULL );
			}
			break;

		case POS_PART:
			/* Add other_info inherited from the software_info level, if any */
			if ( swlist->softinfo && swlist->softinfo->other_info )
			{
				feature_list *list = swlist->softinfo->other_info;

				while( list->next )
				{
					add_feature( swlist, list->next->name, list->next->value );
					list = list->next;
				}
			}
			break;

		case POS_DATA:
			break;
	}
}


/*-------------------------------------------------
    data_handler
-------------------------------------------------*/

static void data_handler(void *data, const XML_Char *s, int len)
{
	software_list *swlist = (software_list *) data;
	int text_len;
	char *text;

	if (swlist->state.text_dest)
	{
		text = *swlist->state.text_dest;

		text_len = text ? strlen(text) : 0;
		text = (char*)pool_realloc_lib(swlist->pool, text, text_len + len + 1);
		if (!text)
			return;

		memcpy(&text[text_len], s, len);
		text[text_len + len] = '\0';
		*swlist->state.text_dest = text;
	}
}


/*-------------------------------------------------
 software_list_get_count
 -------------------------------------------------*/

static int software_list_get_count(software_list *swlist)
{
	int count = 0;

	for (software_info *swinfo = software_list_find(swlist, "*", NULL); swinfo != NULL; swinfo = software_list_find(swlist, "*", swinfo))
		count++;

	return count;
}


/*-------------------------------------------------
    software_get_clone - retrive name string of the
    parent software, if any
 -------------------------------------------------*/

const char *software_get_clone(core_options &options, char *swlist, const char *swname)
{
	software_list *software_list_ptr = software_list_open(options, swlist, FALSE, NULL);
	const char *retval = NULL;
	if (software_list_ptr)
	{
		software_info *tmp = software_list_find(software_list_ptr, swname, NULL);
		retval = core_strdup(tmp->parentname);
		software_list_close(software_list_ptr);
	}

	return retval;
}


/*-------------------------------------------------
    software_get_support - retrive support state of
    the software
 -------------------------------------------------*/

UINT32 software_get_support(core_options &options, char *swlist, const char *swname)
{
	software_list *software_list_ptr = software_list_open(options, swlist, FALSE, NULL);
	UINT32 retval = 0;

	if (software_list_ptr)
	{
		software_info *tmp = software_list_find(software_list_ptr, swname, NULL);
		retval = tmp->supported;
		software_list_close(software_list_ptr);
	}

	return retval;
}


/*-------------------------------------------------
    software_list_parse
-------------------------------------------------*/

void software_list_parse(software_list *swlist,
	void (*error_proc)(const char *message),
	void *param)
{
	char buf[1024];
	UINT32 len;
	XML_Memory_Handling_Suite memcallbacks;

	swlist->file->seek(0, SEEK_SET);

	memset(&swlist->state, 0, sizeof(swlist->state));
	swlist->state.error_proc = error_proc;
	swlist->state.param = param;

	/* create the XML parser */
	memcallbacks.malloc_fcn = expat_malloc;
	memcallbacks.realloc_fcn = expat_realloc;
	memcallbacks.free_fcn = expat_free;
	swlist->state.parser = XML_ParserCreate_MM(NULL, &memcallbacks, NULL);
	if (!swlist->state.parser)
		goto done;

	XML_SetUserData(swlist->state.parser, swlist);
	XML_SetElementHandler(swlist->state.parser, start_handler, end_handler);
	XML_SetCharacterDataHandler(swlist->state.parser, data_handler);

	while(!swlist->state.done)
	{
		len = swlist->file->read(buf, sizeof(buf));
		swlist->state.done = swlist->file->eof();
		if (XML_Parse(swlist->state.parser, buf, len, swlist->state.done) == XML_STATUS_ERROR)
		{
			parse_error(&swlist->state, "[%lu:%lu]: %s\n",
				XML_GetCurrentLineNumber(swlist->state.parser),
				XML_GetCurrentColumnNumber(swlist->state.parser),
				XML_ErrorString(XML_GetErrorCode(swlist->state.parser)));
			goto done;
		}
	}

done:
	if (swlist->state.parser)
		XML_ParserFree(swlist->state.parser);
	swlist->state.parser = NULL;
	swlist->current_software_info = swlist->software_info_list;
	swlist->list_entries = software_list_get_count(swlist);
}


/*-------------------------------------------------
    software_list_open
-------------------------------------------------*/

software_list *software_list_open(core_options &options, const char *listname, int is_preload,
	void (*error_proc)(const char *message))
{
	software_list *swlist = NULL;
	object_pool *pool = NULL;
	file_error filerr;

	/* create a pool for this software list file */
	pool = pool_alloc_lib(error_proc);
	if (!pool)
		goto error;

	/* allocate space for this software list file */
	swlist = (software_list *) pool_malloc_lib(pool, sizeof(*swlist));
	if (!swlist)
		goto error;

	/* set up the software_list structure */
	memset(swlist, 0, sizeof(*swlist));
	swlist->pool = pool;
	swlist->error_proc = error_proc;

	/* open a file */
	swlist->file = global_alloc(emu_file(options, SEARCHPATH_HASH, OPEN_FLAG_READ));
	filerr = swlist->file->open(listname, ".xml");
	if (filerr != FILERR_NONE)
		goto error;

	if (is_preload)
	{
		software_list_parse(swlist, swlist->error_proc, NULL);
		swlist->current_software_info = NULL;
	}

	return swlist;

error:
	if (swlist != NULL)
		software_list_close(swlist);
	return NULL;
}


/*-------------------------------------------------
    software_list_close
-------------------------------------------------*/

void software_list_close(software_list *swlist)
{
	if (swlist == NULL)
		return;

	if (swlist->file != NULL)
		global_free(swlist->file);
	pool_free_lib(swlist->pool);
}


/*-------------------------------------------------
 software_list_get_description
 -------------------------------------------------*/

const char *software_list_get_description(software_list *swlist)
{
	return swlist->description;
}


/*-------------------------------------------------
 software_list_find_by_number
 -------------------------------------------------*/

INLINE software_info *software_list_find_by_number(software_list *swlist, int number)
{
	int length = swlist->list_entries;
	if (number > length)
		return NULL;

	software_info *cur_info = software_list_find(swlist, "*", NULL);

	for (int count = 0; count < number; count++)
		cur_info = software_list_find(swlist, "*", cur_info);

	return cur_info;
}


/*-------------------------------------------------
 softlist_penalty_compare (borrowed from driver.c)
 -------------------------------------------------*/

static int softlist_penalty_compare(const char *source, const char *target)
{
	int gaps = 1;
	int last = TRUE;

	/* scan the strings */
	for ( ; *source && *target; target++)
	{
		/* do a case insensitive match */
		int match = (tolower((UINT8)*source) == tolower((UINT8)*target));

		/* if we matched, advance the source */
		if (match)
			source++;

		/* if the match state changed, count gaps */
		if (match != last)
		{
			last = match;
			if (!match)
				gaps++;
		}
	}

	/* penalty if short string does not completely fit in */
	for ( ; *source; source++)
		gaps++;

	/* if we matched perfectly, gaps == 0 */
	if (gaps == 1 && *source == 0 && *target == 0)
		gaps = 0;

	return gaps;
}


/*-------------------------------------------------
 software_list_find_approx_matches
 -------------------------------------------------*/

void software_list_find_approx_matches(software_list *swlist, const char *name, int matches, software_info **list, const char* interface)
{
#undef rand

	int matchnum;
	int *penalty;

	/* if no name, return */
	if (name == NULL || name[0] == 0)
		return;

	/* allocate some temp memory */
	penalty = global_alloc_array(int, matches);

	/* initialize everyone's states */
	for (matchnum = 0; matchnum < matches; matchnum++)
	{
		penalty[matchnum] = 9999;
		list[matchnum] = NULL;
	}

	for (software_info *swinfo = software_list_find(swlist, "*", NULL); swinfo != NULL; swinfo = software_list_find(swlist, "*", swinfo))
	{
		int curpenalty, tmp;
		software_info *candidate = swinfo;

		software_part *part = software_find_part(swinfo, NULL, NULL);
		if (!strcmp(interface, part->interface_))
		{

			/* pick the best match between driver name and description */
			curpenalty = softlist_penalty_compare(name, candidate->longname);
			tmp = softlist_penalty_compare(name, candidate->shortname);
			curpenalty = MIN(curpenalty, tmp);

			/* insert into the sorted table of matches */
			for (matchnum = matches - 1; matchnum >= 0; matchnum--)
			{
				/* stop if we're worse than the current entry */
				if (curpenalty >= penalty[matchnum])
					break;

				/* as long as this isn't the last entry, bump this one down */
				if (matchnum < matches - 1)
				{
					penalty[matchnum + 1] = penalty[matchnum];
					list[matchnum + 1] = list[matchnum];
				}
				list[matchnum] = candidate;
				penalty[matchnum] = curpenalty;
			}
		}
	}

	/* free our temp memory */
	global_free(penalty);
}


/*-------------------------------------------------
    software_list_find
-------------------------------------------------*/

software_info *software_list_find(software_list *swlist, const char *look_for, software_info *prev)
{
	if (swlist == NULL)
		return NULL;

	if (look_for == NULL)
		return NULL;

	/* If we haven't read in the xml file yet, then do it now */
	if ( ! swlist->software_info_list )
		software_list_parse( swlist, swlist->error_proc, NULL );

	for ( prev = prev ? prev->next : swlist->software_info_list; prev; prev = prev->next )
	{
		if ( !mame_strwildcmp( look_for, prev->shortname ) )
			break;
	}

	return prev;
}


/*-------------------------------------------------
    software_find_romdata (for validation purposes)
 -------------------------------------------------*/

static struct rom_entry *software_find_romdata(software_part *swpart, const char *dataname)
{
	struct rom_entry *data = swpart ? swpart->romdata : NULL;

	/* If no dataname supplied, then we just return the first entry */
	if (data)
	{
		while(data && data->_name)
		{
			if (dataname)
			{
				if (!strcmp(dataname, data->_name))
				{
					break;
				}
			}
			/* No specific dataname supplied, return the first rom_entry */
			else
				break;

			data++;
		}
	}

	if (!data->_name)
		data = NULL;

	return data;
}


/*-------------------------------------------------
    software_romdata_next (for validation purposes)
 -------------------------------------------------*/

static struct rom_entry *software_romdata_next(struct rom_entry *romdata)
{
	if (romdata && romdata->_name)
	{
		romdata++;
	}
	else
		romdata = NULL;

	return romdata;
}


/*-------------------------------------------------
    software_find_part
-------------------------------------------------*/

software_part *software_find_part(software_info *sw, const char *partname, const char *interface)
{
	software_part *part = sw ? sw->partdata : NULL;

	 /* If neither partname nor interface supplied, then we just return the first entry */
	if ( partname || interface )
	{
		while( part && part->name )
		{
			if ( partname )
			{
				if ( !strcmp(partname, part->name ) )
				{
					if ( interface )
					{
						if ( !strcmp(interface, part->interface_) )
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				/* No specific partname supplied, find the first match based on interface */
				if ( interface )
				{
					if ( !strcmp(interface, part->interface_) )
					{
						break;
					}
				}
			}
			part++;
		}
	}

	if ( ! part->name )
		part = NULL;

	return part;
}


/*-------------------------------------------------
    software_part_next
-------------------------------------------------*/

software_part *software_part_next(software_part *part)
{
	if ( part && part->name )
	{
		part++;
	}

	if ( ! part->name )
		part = NULL;

	return part;
}

/*-------------------------------------------------
    load_software_part

    Load a software part for a device. The part to
    load is determined by the "path", software lists
    configured for a driver, and the interface
    supported by the device.

    returns true if the software could be loaded,
    false otherwise. If the software could be loaded
    sw_info and sw_part are also set.
-------------------------------------------------*/

bool load_software_part(device_image_interface *image, const char *path, software_info **sw_info, software_part **sw_part, char **full_sw_name)
{
	char *swlist_name, *swname, *swpart, *swname_bckp;
	bool result = false;
	software_list *software_list_ptr = NULL;
	software_info *software_info_ptr = NULL;
	software_part *software_part_ptr = NULL;

	*sw_info = NULL;
	*sw_part = NULL;

	/* Split full software name into software list name and short software name */
	software_name_split( image->device().machine, path, &swlist_name, &swname, &swpart );
	swname_bckp = swname;

	const char *interface = image->image_config().image_interface();

	if ( swlist_name )
	{
		/* Try to open the software list xml file explicitly named by the user */
		software_list_ptr = software_list_open( image->device().machine->options(), swlist_name, FALSE, NULL );

		if ( software_list_ptr )
		{
			software_info_ptr = software_list_find( software_list_ptr, swname, NULL );

			if ( software_info_ptr )
			{
				software_part_ptr = software_find_part( software_info_ptr, swpart, interface );
			}
		}
	}
	else
	{
		/* Loop through all the software lists named in the driver */
		for (device_t *swlists = image->device().machine->m_devicelist.first(SOFTWARE_LIST); swlists != NULL; swlists = swlists->typenext())
		{
			if ( swlists )
			{

				software_list_config *swlist = (software_list_config *)downcast<const legacy_device_config_base *>(&swlists->baseconfig())->inline_config();
				UINT32 i = DEVINFO_STR_SWLIST_0;

				while ( ! software_part_ptr && i <= DEVINFO_STR_SWLIST_MAX )
				{
					swlist_name = swlist->list_name[i-DEVINFO_STR_SWLIST_0];

					if ( swlist_name && *swlist_name && (swlist->list_type == SOFTWARE_LIST_ORIGINAL_SYSTEM))
					{
						if ( software_list_ptr )
						{
							software_list_close( software_list_ptr );
						}

						software_list_ptr = software_list_open( image->device().machine->options(), swlist_name, FALSE, NULL );

						if ( software_list_ptr )
						{
							software_info_ptr = software_list_find( software_list_ptr, swname, NULL );

							if ( software_info_ptr )
							{
								software_part_ptr = software_find_part( software_info_ptr, swpart, interface );
							}
						}
					}
					i++;
				}
			}
		}

		/* If not found try to load the software list using the driver name */
		if ( ! software_part_ptr )
		{
			swlist_name = (char *)image->device().machine->gamedrv->name;

			if ( software_list_ptr )
			{
				software_list_close( software_list_ptr );
			}

			software_list_ptr = software_list_open( image->device().machine->options(), swlist_name, FALSE, NULL );

			if ( software_list_ptr )
			{
				software_info_ptr = software_list_find( software_list_ptr, swname, NULL );

				if ( software_info_ptr )
				{
					software_part_ptr = software_find_part( software_info_ptr, swpart, interface );
				}
			}
		}

		/* If not found try to load the software list using the software name as software */
		/* list name and software part name as software name. */
		if ( ! software_part_ptr )
		{
			swlist_name = swname;
			swname = swpart;
			swpart = NULL;

			if ( software_list_ptr )
			{
				software_list_close( software_list_ptr );
			}

			software_list_ptr = software_list_open( image->device().machine->options(), swlist_name, FALSE, NULL );

			if ( software_list_ptr )
			{
				software_info_ptr = software_list_find( software_list_ptr, swname, NULL );

				if ( software_info_ptr )
				{
					software_part_ptr = software_find_part( software_info_ptr, swpart, interface );
				}

				if ( ! software_part_ptr )
				{
					software_list_close( software_list_ptr );
					software_list_ptr = NULL;
				}
			}
		}
	}

	// if no match has been found, we suggest similar shortnames
	if (software_info_ptr == NULL)
	{
		// check if there is at least a software list
		if (image->device().machine->m_devicelist.first(SOFTWARE_LIST))
		{
			mame_printf_error("\n\"%s\" approximately matches the following\n"
							  "supported software items (best match first):\n\n", swname_bckp);
		}

		for (device_t *swlists = image->device().machine->m_devicelist.first(SOFTWARE_LIST); swlists != NULL; swlists = swlists->typenext())
		{
			software_list_config *swlist = (software_list_config *)downcast<const legacy_device_config_base *>(&swlists->baseconfig())->inline_config();

			for (int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++)
			{
				if (swlist->list_name[i] && *swlist->list_name[i])
				{
					software_list *list = software_list_open(image->device().machine->options(), swlist->list_name[i], FALSE, NULL);

					if (list)
					{
						software_info *matches[10] = { 0 };
						int softnum;

						software_list_parse(list, list->error_proc, NULL);
						// get the top 5 approximate matches for the selected device interface (i.e. only carts for cartslot, etc.)
						software_list_find_approx_matches(list, swname_bckp, ARRAY_LENGTH(matches), matches, image->image_config().image_interface());

						if (matches[0] != 0)
						{
							if (swlist->list_type == SOFTWARE_LIST_ORIGINAL_SYSTEM)
								mame_printf_error("* Software list \"%s\" (%s) matches: \n", swlist->list_name[i], software_list_get_description(list));
							else
								mame_printf_error("* Compatible software list \"%s\" (%s) matches: \n", swlist->list_name[i], software_list_get_description(list));

							// print them out
							for (softnum = 0; softnum < ARRAY_LENGTH(matches); softnum++)
								if (matches[softnum] != NULL)
									mame_printf_error("%-18s%s\n", matches[softnum]->shortname, matches[softnum]->longname);

							mame_printf_error("\n");
						}
						software_list_close(list);
					}
				}
			}
		}
	}

	if ( software_part_ptr )
	{
		/* Load the software part */
		try {
			result = image->call_softlist_load((char *)swlist_name, (char *)software_info_ptr->shortname, software_part_ptr->romdata );
		}
		catch (emu_fatalerror &fatal)
		{
			software_list_close( software_list_ptr );
			throw fatal;
		}

		/* Create a copy of the software and part information */
		*sw_info = auto_alloc_clear( image->device().machine, software_info );
		(*sw_info)->shortname = auto_strdup( image->device().machine, software_info_ptr->shortname );
		(*sw_info)->longname = auto_strdup( image->device().machine, software_info_ptr->longname );
		if ( software_info_ptr->year )
			(*sw_info)->year = auto_strdup( image->device().machine, software_info_ptr->year );
		if ( software_info_ptr->publisher )
			(*sw_info)->publisher = auto_strdup( image->device().machine, software_info_ptr->publisher );

		*sw_part = auto_alloc_clear( image->device().machine, software_part );
		(*sw_part)->name = auto_strdup( image->device().machine, software_part_ptr->name );
		if ( software_part_ptr->interface_ )
			(*sw_part)->interface_ = auto_strdup( image->device().machine, software_part_ptr->interface_ );

		if ( software_part_ptr->featurelist )
		{
			feature_list *list = software_part_ptr->featurelist;
			feature_list *new_list = auto_alloc_clear( image->device().machine, feature_list );

			(*sw_part)->featurelist = new_list;

			new_list->name = auto_strdup( image->device().machine, list->name );
			new_list->value = auto_strdup( image->device().machine, list->value );

			list = list->next;

			while( list )
			{
				new_list->next = auto_alloc_clear( image->device().machine, feature_list );
				new_list = new_list->next;
				new_list->name = auto_strdup( image->device().machine, list->name );
				new_list->value = auto_strdup( image->device().machine, list->value );

				list = list->next;
			}

			new_list->next = NULL;
		}

		/* Tell the world which part we actually loaded */
		*full_sw_name = auto_alloc_array( image->device().machine, char, strlen(swlist_name) + strlen(software_info_ptr->shortname) + strlen(software_part_ptr->name) + 3 );
		sprintf( *full_sw_name, "%s:%s:%s", swlist_name, software_info_ptr->shortname, software_part_ptr->name );
	}

	/* Close the software list if it's still open */
	if ( software_list_ptr )
	{
		software_list_close( software_list_ptr );
		software_info_ptr = NULL;
		software_list_ptr = NULL;
	}
	auto_free( image->device().machine, swlist_name );
	auto_free( image->device().machine, swname );
	auto_free( image->device().machine, swpart );

	return result;
}


/*-------------------------------------------------
    software_part_get_feature
 -------------------------------------------------*/

const char *software_part_get_feature(software_part *part, const char *feature_name)
{
	feature_list *feature;

	if (part == NULL)
		return NULL;

	for (feature = part->featurelist; feature; feature = feature->next)
	{
		if (!strcmp(feature->name, feature_name))
			return feature->value;
	}

	return NULL;

}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/


static DEVICE_START( software_list )
{
}

static DEVICE_VALIDITY_CHECK( software_list )
{
	software_list_config *swlist = (software_list_config *)downcast<const legacy_device_config_base *>(device)->inline_config();
	int error = FALSE;
	softlist_map names;
	softlist_map descriptions;

	enum { NAME_LEN_PARENT = 8, NAME_LEN_CLONE = 16 };

	for (int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++)
	{
		if (swlist->list_name[i])
		{
			software_list *list = software_list_open(options, swlist->list_name[i], FALSE, NULL);

			/* if no .xml list is found, then return (this happens e.g. if you moved/renamed the xml list) */
			if (list == NULL)
				return FALSE;

			for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
			{
				const char *s;
				int is_clone = 0;

				/* First, check if the xml got corrupted: */

				/* Did we lost any description? */
				if (swinfo->longname == NULL)
				{
					mame_printf_error("%s: %s has no description\n", swlist->list_name[i], swinfo->shortname);
					return TRUE;
				}

				/* Did we lost any year? */
				if (swinfo->year == NULL)
				{
					mame_printf_error("%s: %s has no year\n", swlist->list_name[i], swinfo->shortname);
					return TRUE;
				}

				/* Did we lost any publisher? */
				if (swinfo->publisher == NULL)
				{
					mame_printf_error("%s: %s has no publisher\n", swlist->list_name[i], swinfo->shortname);
					return TRUE;
				}

				/* Second, since the xml is fine, run additional checks: */

				/* check for duplicate names */
				if (names.add(swinfo->shortname, swinfo, FALSE) == TMERR_DUPLICATE)
				{
					software_info *match = names.find(swinfo->shortname);
					mame_printf_error("%s: %s is a duplicate name (%s)\n", swlist->list_name[i], swinfo->shortname, match->shortname);
					error = TRUE;
				}

				/* check for duplicate descriptions */
				if (descriptions.add(swinfo->longname, swinfo, FALSE) == TMERR_DUPLICATE)
				{
					software_info *match = names.find(swinfo->shortname);
					mame_printf_error("%s: %s is a duplicate description (%s)\n", swlist->list_name[i], swinfo->longname, match->longname);
					error = TRUE;
				}

				if (swinfo->parentname != NULL)
				{
					is_clone = 1;

					if (strcmp(swinfo->parentname, swinfo->shortname) == 0)
					{
						mame_printf_error("%s: %s is set as a clone of itself\n", swlist->list_name[i], swinfo->shortname);
						error = TRUE;
						break;
					}

					/* make sure the parent exists */
					software_info *swinfo2 = software_list_find(list, swinfo->parentname, NULL );

					if (!swinfo2)
					{
						mame_printf_error("%s: parent '%s' software for '%s' not found\n", swlist->list_name[i], swinfo->parentname, swinfo->shortname);
						error = TRUE;
					}
					else
					{
						if (swinfo2->parentname != NULL)
						{
							mame_printf_error("%s: %s is a clone of a clone\n", swlist->list_name[i], swinfo->shortname);
							error = TRUE;
						}
					}
				}

				/* make sure the driver name is 8 chars or less */
				if ((is_clone && strlen(swinfo->shortname) > NAME_LEN_CLONE) || ((!is_clone) && strlen(swinfo->shortname) > NAME_LEN_PARENT))
				{
					mame_printf_error("%s: %s %s driver name must be %d characters or less\n", swlist->list_name[i], swinfo->shortname,
									  is_clone ? "clone" : "parent", is_clone ? NAME_LEN_CLONE : NAME_LEN_PARENT);
					error = TRUE;
				}

				/* make sure the year is only digits, '?' or '+' */
				for (s = swinfo->year; *s; s++)
					if (!isdigit((UINT8)*s) && *s != '?' && *s != '+')
					{
						mame_printf_error("%s: %s has an invalid year '%s'\n", swlist->list_name[i], swinfo->shortname, swinfo->year);
						error = TRUE;
						break;
					}

				for (software_part *swpart = software_find_part(swinfo, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
				{
					if (swpart->interface_ == NULL)
					{
						mame_printf_error("%s: %s has a part (%s) without interface\n", swlist->list_name[i], swinfo->shortname, swpart->name);
						error = TRUE;
					}

					if (software_find_romdata(swpart, NULL) == NULL)
					{
						mame_printf_error("%s: %s has a part (%s) with no data\n", swlist->list_name[i], swinfo->shortname, swpart->name);
						error = TRUE;
					}

					for (struct rom_entry *swdata = software_find_romdata(swpart, NULL); swdata != NULL;  swdata = software_romdata_next(swdata))
					{
						struct rom_entry *data = swdata;

						if (data->_name && data->_hashdata)
						{
							const char *s;

							/* make sure it's all lowercase */
							for (s = data->_name; *s; s++)
								if (tolower((UINT8)*s) != *s)
								{
									mame_printf_error("%s: %s has upper case ROM name %s\n", swlist->list_name[i], swinfo->shortname, data->_name);
									error = TRUE;
									break;
								}

							/* make sure the hash is valid */
							hash_collection hashes;
							if (!hashes.from_internal_string(data->_hashdata))
							{
								mame_printf_error("%s: %s has rom '%s' with an invalid hash string '%s'\n", swlist->list_name[i], swinfo->shortname, data->_name, data->_hashdata);
								error = TRUE;
							}
						}
					}
				}
			}

			software_list_close(list);
		}
	}
	return error;
}

DEVICE_GET_INFO( software_list )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = 1;										break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = sizeof(software_list_config);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( software_list );	break;
		case DEVINFO_FCT_STOP:							/* Nothing */										break;
		case DEVINFO_FCT_VALIDITY_CHECK:				info->p = (void*)DEVICE_VALIDITY_CHECK_NAME( software_list ); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Software lists");					break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Software lists");					break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");								break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright MESS Team");				break;
	}

	if ( state >= DEVINFO_STR_SWLIST_0 && state <= DEVINFO_STR_SWLIST_MAX )
	{
		software_list_config *config = (software_list_config *)downcast<const legacy_device_config_base *>(device)->inline_config();

		if ( config->list_name[ state - DEVINFO_STR_SWLIST_0 ] )
			strcpy(info->s, config->list_name[ state - DEVINFO_STR_SWLIST_0 ]);
	}
}


/***************************************************************************
    MENU SUPPORT
***************************************************************************/

/* state of the software menu */
typedef struct _software_menu_state software_menu_state;
struct _software_menu_state
{
	char *list_name;	/* currently selected list */
	device_image_interface* image;
};

/* state of a software entry */
typedef struct _software_entry_state software_entry_state;
struct _software_entry_state
{
	const char *short_name;
	const char *interface;
	char *list_name;
	device_image_interface* image;
};

/* state of a software part */
typedef struct _software_part_state software_part_state;
struct _software_part_state
{
	const char *part_name;
	const char *interface;
};


static void ui_mess_menu_populate_software_parts(running_machine *machine, ui_menu *menu, const char *swlist, const char *swinfo, const char *interface)
{
	software_list *list = software_list_open(machine->options(), swlist, FALSE, NULL);

	if (list)
	{
		software_info *info = software_list_find(list, swinfo, NULL);

		if (info)
		{
			for (software_part *swpart = software_find_part(info, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
			{
				if (strcmp(interface, swpart->interface_) == 0)
				{
					software_part_state *entry = (software_part_state *) ui_menu_pool_alloc(menu, sizeof(*entry));
					// check if the available parts have specific part_id to be displayed (e.g. "Map Disc", "Bonus Disc", etc.)
					// if not, we simply display "part_name"; if yes we display "part_name (part_id)"
					astring menu_part_name(swpart->name);
					if (software_part_get_feature(swpart, "part_id") != NULL)
					{
						menu_part_name.cat(" (");
						menu_part_name.cat(software_part_get_feature(swpart, "part_id"));
						menu_part_name.cat(")");
					}
					entry->part_name = ui_menu_pool_strdup(menu, swpart->name);	// part_name is later used to build up the filename to load, so we use swpart->name!
					entry->interface = ui_menu_pool_strdup(menu, swpart->interface_);
					ui_menu_item_append(menu, info->shortname, menu_part_name.cstr(), 0, entry);
				}
			}
		}

		software_list_close(list);
	}
}

void ui_mess_menu_software_parts(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;
	software_entry_state *sw_state = (software_entry_state *)state;
	const char *swlist = sw_state->list_name;
	const char *swinfo = sw_state->short_name;
	const char *interface = sw_state->interface;

	// generate list of available parts
	if (!ui_menu_populated(menu))
	{
		if (sw_state->list_name)
		{
			ui_mess_menu_populate_software_parts(machine, menu, swlist, swinfo, interface);
		}
	}

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);

	if (event != NULL && event->iptkey == IPT_UI_SELECT && event->itemref != NULL)
	{
		software_part_state *entry = (software_part_state *) event->itemref;

		// build the name for the part to be loaded
		astring temp_name(sw_state->short_name);
		temp_name.cat(":");
		temp_name.cat(entry->part_name);
		//printf("%s\n", temp_name.cstr());

		sw_state->image->load(temp_name.cstr());
	}
}

/* populate a specific list */
static void ui_mess_menu_populate_software_entries(running_machine *machine, ui_menu *menu, char *list_name, device_image_interface* image)
{
	software_list *list = software_list_open(machine->options(), list_name, FALSE, NULL);
	const char *interface = image->image_config().image_interface();
	if (list)
	{
		for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
		{
			software_entry_state *entry = (software_entry_state *) ui_menu_pool_alloc(menu, sizeof(*entry));
			entry->short_name = ui_menu_pool_strdup(menu, swinfo->shortname);
			entry->list_name = list_name;
			entry->image = image;

			// check if at least one of the parts has the correct interface
			for (software_part *swpart = software_find_part(swinfo, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
			{
				if (strcmp(interface, swpart->interface_) == 0)
				{
					entry->interface = ui_menu_pool_strdup(menu, swpart->interface_);
					ui_menu_item_append(menu, swinfo->shortname, swinfo->longname, 0, entry);
					break;
				}
			}
		}

		software_list_close(list);
	}
}

bool swinfo_has_multiple_parts(software_info *swinfo, const char *interface)
{
	int count = 0;

	for (software_part *swpart = software_find_part(swinfo, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
	{
		if (strcmp(interface, swpart->interface_) == 0)
			count++;
	}
	return (count > 1) ? TRUE : FALSE;
}

void ui_mess_menu_software_list(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;
	software_menu_state *sw_state = (software_menu_state *)state;

	if (!ui_menu_populated(menu))
	{
		if (sw_state->list_name)
		{
			ui_mess_menu_populate_software_entries(machine, menu, sw_state->list_name, sw_state->image);
		}
	}

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);

	if (event != NULL && event->iptkey == IPT_UI_SELECT && event->itemref != NULL)
	{
		device_image_interface *image = sw_state->image;
		software_entry_state *entry = (software_entry_state *) event->itemref;
		software_list *tmp_list = software_list_open(machine->options(), sw_state->list_name, FALSE, NULL);
		software_info *tmp_info = software_list_find(tmp_list, entry->short_name, NULL);

		// if the selected software has multiple parts that can be loaded, open the submenu
		if (swinfo_has_multiple_parts(tmp_info, image->image_config().image_interface()))
		{
			ui_menu *child_menu = ui_menu_alloc(machine, &machine->render().ui_container(), ui_mess_menu_software_parts, entry);
			software_entry_state *child_menustate = (software_entry_state *)ui_menu_alloc_state(child_menu, sizeof(*child_menustate), NULL);
			child_menustate->short_name = entry->short_name;
			child_menustate->interface = image->image_config().image_interface();
			child_menustate->list_name = sw_state->list_name;
			child_menustate->image = image;
			ui_menu_stack_push(child_menu);
		}
		else
		{
			// otherwise, load the file
			if (image != NULL)
				image->load(entry->short_name);
			else
				popmessage("No matching device found for interface '%s'!", entry->interface);
		}
		software_list_close(tmp_list);
	}
}

/* list of available software lists - i.e. cartridges, floppies */
static void ui_mess_menu_populate_software_list(running_machine *machine, ui_menu *menu, device_image_interface* image)
{
	bool haveCompatible = FALSE;
	const char *interface = image->image_config().image_interface();

	for (const device_config *dev = machine->config->m_devicelist.first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
	{
		software_list_config *swlist = (software_list_config *)downcast<const legacy_device_config_base *>(dev)->inline_config();

		for (int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++)
		{
			if (swlist->list_name[i] && (swlist->list_type == SOFTWARE_LIST_ORIGINAL_SYSTEM))
			{
				software_list *list = software_list_open(machine->options(), swlist->list_name[i], FALSE, NULL);

				if (list)
				{
					bool found = FALSE;
					for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
					{
						software_part *part = software_find_part(swinfo, NULL, NULL);
						if (strcmp(interface,part->interface_)==0) {
							found = TRUE;
						}
					}
					if (found) {
						ui_menu_item_append(menu, list->description, NULL, 0, swlist->list_name[i]);
					}

					software_list_close(list);
				}
			}
		}
	}

	for (const device_config *dev = machine->config->m_devicelist.first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
	{
		software_list_config *swlist = (software_list_config *)downcast<const legacy_device_config_base *>(dev)->inline_config();

		for (int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++)
		{
			if (swlist->list_name[i] && (swlist->list_type == SOFTWARE_LIST_COMPATIBLE_SYSTEM))
			{
				software_list *list = software_list_open(machine->options(), swlist->list_name[i], FALSE, NULL);

				if (list)
				{
					bool found = FALSE;
					for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
					{
						software_part *part = software_find_part(swinfo, NULL, NULL);
						if (strcmp(interface,part->interface_)==0) {
							found = TRUE;
						}
					}
					if (found) {
						if (!haveCompatible) {
							ui_menu_item_append(menu, "[compatible lists]", NULL, 0, NULL);
						}
						ui_menu_item_append(menu, list->description, NULL, 0, swlist->list_name[i]);
					}

					haveCompatible = TRUE;
					software_list_close(list);
				}
			}
		}
	}

}

void ui_image_menu_software(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;
	device_image_interface* image = (device_image_interface*)parameter;
	if (!ui_menu_populated(menu))
		ui_mess_menu_populate_software_list(machine, menu, image);

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);

	if (event != NULL && event->iptkey == IPT_UI_SELECT)
	{
		ui_menu *child_menu = ui_menu_alloc(machine, &machine->render().ui_container(), ui_mess_menu_software_list, NULL);
		software_menu_state *child_menustate = (software_menu_state *)ui_menu_alloc_state(child_menu, sizeof(*child_menustate), NULL);
		child_menustate->list_name = (char *)event->itemref;
		child_menustate->image = image;
		ui_menu_stack_push(child_menu);
	}
}

DEFINE_LEGACY_DEVICE(SOFTWARE_LIST, software_list);
