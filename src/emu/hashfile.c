/*********************************************************************

    hashfile.c

    Code for parsing hash info (*.hsi) files

*********************************************************************/

#include "hashfile.h"
#include "pool.h"
#include "expat.h"
#include "emuopts.h"
#include "hash.h"
#include "drivenum.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _hash_info hash_info;
struct _hash_info
{
	hash_collection *hashes;
	const char *extrainfo;
};

typedef struct _hash_file hash_file;

typedef void (*hashfile_error_func)(const char *message);



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* opens a hash file; if is_preload is non-zero, the entire file is preloaded */
hash_file *hashfile_open(emu_options &options, const char *sysname, int is_preload, hashfile_error_func error_proc);

/* closes a hash file and associated resources */
void hashfile_close(hash_file *hashfile);

/* looks up information in a hash file */
const hash_info *hashfile_lookup(hash_file *hashfile, const hash_collection *hashes);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _hash_file
{
	emu_file *file;
	object_pool *pool;
	astring functions[IO_COUNT];

	hash_info **preloaded_hashes;
	int preloaded_hash_count;

	void (*error_proc)(const char *message);
};



enum hash_parse_position
{
	HASH_POS_ROOT,
	HASH_POS_MAIN,
	HASH_POS_HASH
};



struct hash_parse_state
{
	XML_Parser parser;
	hash_file *hashfile;
	int done;

	int (*selector_proc)(hash_file *hashfile, void *param, const char *name, const hash_collection *hashes);
	void (*use_proc)(hash_file *hashfile, void *param, hash_info *hi);
	void (*error_proc)(const char *message);
	void *param;

	enum hash_parse_position pos;
	char **text_dest;
	hash_info *hi;
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

static void *expat_malloc(size_t size);
static void *expat_realloc(void *ptr, size_t size);
static void expat_free(void *ptr);



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    parse_error
-------------------------------------------------*/

static void ATTR_PRINTF(2,3) parse_error(struct hash_parse_state *state, const char *fmt, ...)
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

static void unknown_tag(struct hash_parse_state *state, const char *tagname)
{
	parse_error(state, "[%lu:%lu]: Unknown tag: %s\n",
		XML_GetCurrentLineNumber(state->parser),
		XML_GetCurrentColumnNumber(state->parser),
		tagname);
}



/*-------------------------------------------------
    unknown_attribute
-------------------------------------------------*/

static void unknown_attribute(struct hash_parse_state *state, const char *attrname)
{
	parse_error(state, "[%lu:%lu]: Unknown attribute: %s\n",
		XML_GetCurrentLineNumber(state->parser),
		XML_GetCurrentColumnNumber(state->parser),
		attrname);
}



/*-------------------------------------------------
    unknown_attribute_value
-------------------------------------------------*/

static void unknown_attribute_value(struct hash_parse_state *state,
	const char *attrname, const char *attrvalue)
{
	parse_error(state, "[%lu:%lu]: Unknown attribute value: %s\n",
		XML_GetCurrentLineNumber(state->parser),
		XML_GetCurrentColumnNumber(state->parser),
		attrvalue);
}



/*-------------------------------------------------
    start_handler
-------------------------------------------------*/

static void start_handler(void *data, const char *tagname, const char **attributes)
{
	struct hash_parse_state *state = (struct hash_parse_state *) data;
	const char *name;
	hash_info *hi;
	char **text_dest;
	hash_collection hashes;
	astring all_functions;
	char functions;
//  iodevice_t device;
	int i;

	switch(state->pos)
	{
		case HASH_POS_ROOT:
			if (!strcmp(tagname, "hashfile"))
			{
			}
			else
			{
				unknown_tag(state, tagname);
			}
			break;

		case HASH_POS_MAIN:
			if (!strcmp(tagname, "hash"))
			{
				// we are now examining a hash tag
				name = NULL;
//              device = IO_COUNT;

				while(attributes[0])
				{
					functions = 0;
					if (!strcmp(attributes[0], "name"))
					{
						/* name attribute */
						name = attributes[1];
					}
					else if (!strcmp(attributes[0], "crc32"))
					{
						/* crc32 attribute */
						functions = hash_collection::HASH_CRC;
					}
					else if (!strcmp(attributes[0], "sha1"))
					{
						/* sha1 attribute */
						functions = hash_collection::HASH_SHA1;
					}
					else if (!strcmp(attributes[0], "type"))
					{
						/* type attribute */
						i = 0;//device_typeid(attributes[1]);
						if (i < 0)
							unknown_attribute_value(state, attributes[0], attributes[1]);
						else
						{
	 //                         device = (iodevice_t) i;
						}
					}
					else
					{
						/* unknown attribute */
						unknown_attribute(state, attributes[0]);
					}

					if (functions)
					{
						hashes.add_from_string(functions, attributes[1], strlen(attributes[1]));
						all_functions.cat(functions);
					}

					attributes += 2;
				}

				/* do we use this hash? */
				if (!state->selector_proc || state->selector_proc(state->hashfile, state->param, name, &hashes))
				{
					hi = (hash_info*)pool_malloc_lib(state->hashfile->pool, sizeof(hash_info));
					if (!hi)
						return;
					memset(hi, 0, sizeof(*hi));

					hi->hashes = &hashes;
					state->hi = hi;
				}
			}
			else
			{
				unknown_tag(state, tagname);
			}
			break;

		case HASH_POS_HASH:
			text_dest = NULL;

			if (!strcmp(tagname, "year")) {
			}
			else if (!strcmp(tagname, "manufacturer")){
			}
			else if (!strcmp(tagname, "status")){
			}
			else if (!strcmp(tagname, "pcb")){
			}
			else if (!strcmp(tagname, "extrainfo")) {
				text_dest = (char **) &state->hi->extrainfo;
			}
			else
				unknown_tag(state, tagname);

			if (text_dest && state->hi)
				state->text_dest = text_dest;
			break;
	}
	state->pos = (hash_parse_position) (state->pos + 1);
}



/*-------------------------------------------------
    end_handler
-------------------------------------------------*/

static void end_handler(void *data, const char *name)
{
	struct hash_parse_state *state = (struct hash_parse_state *) data;
	state->text_dest = NULL;

	state->pos = (hash_parse_position) (state->pos - 1);
	switch(state->pos)
	{
		case HASH_POS_ROOT:
		case HASH_POS_HASH:
			break;

		case HASH_POS_MAIN:
			if (state->hi)
			{
				if (state->use_proc)
					(*state->use_proc)(state->hashfile, state->param, state->hi);
				state->hi = NULL;
			}
			break;
	}
}



/*-------------------------------------------------
    data_handler
-------------------------------------------------*/

static void data_handler(void *data, const XML_Char *s, int len)
{
	struct hash_parse_state *state = (struct hash_parse_state *) data;
	int text_len;
	char *text;

	if (state->text_dest)
	{
		text = *state->text_dest;

		text_len = text ? strlen(text) : 0;
		text = (char*)pool_realloc_lib(state->hashfile->pool, text, text_len + len + 1);
		if (!text)
			return;

		memcpy(&text[text_len], s, len);
		text[text_len + len] = '\0';
		*state->text_dest = text;
	}
}



/*-------------------------------------------------
    hashfile_parse
-------------------------------------------------*/

static void hashfile_parse(hash_file *hashfile,
	int (*selector_proc)(hash_file *hashfile, void *param, const char *name, const hash_collection *hashes),
	void (*use_proc)(hash_file *hashfile, void *param, hash_info *hi),
	void (*error_proc)(const char *message),
	void *param)
{
	struct hash_parse_state state;
	char buf[1024];
	UINT32 len;
	XML_Memory_Handling_Suite memcallbacks;

	hashfile->file->seek(0, SEEK_SET);

	memset(&state, 0, sizeof(state));
	state.hashfile = hashfile;
	state.selector_proc = selector_proc;
	state.use_proc = use_proc;
	state.error_proc = error_proc;
	state.param = param;

	/* create the XML parser */
	memcallbacks.malloc_fcn = expat_malloc;
	memcallbacks.realloc_fcn = expat_realloc;
	memcallbacks.free_fcn = expat_free;
	state.parser = XML_ParserCreate_MM(NULL, &memcallbacks, NULL);
	if (!state.parser)
		goto done;

	XML_SetUserData(state.parser, &state);
	XML_SetElementHandler(state.parser, start_handler, end_handler);
	XML_SetCharacterDataHandler(state.parser, data_handler);

	while(!state.done)
	{
		len = hashfile->file->read(buf, sizeof(buf));
		state.done = hashfile->file->eof();
		if (XML_Parse(state.parser, buf, len, state.done) == XML_STATUS_ERROR)
		{
			parse_error(&state, "[%lu:%lu]: %s\n",
				XML_GetCurrentLineNumber(state.parser),
				XML_GetCurrentColumnNumber(state.parser),
				XML_ErrorString(XML_GetErrorCode(state.parser)));
			goto done;
		}
	}

done:
	if (state.parser)
		XML_ParserFree(state.parser);
}



/*-------------------------------------------------
    preload_use_proc
-------------------------------------------------*/

static void preload_use_proc(hash_file *hashfile, void *param, hash_info *hi)
{
	hash_info **new_preloaded_hashes;

	new_preloaded_hashes = (hash_info **)pool_realloc_lib(hashfile->pool, hashfile->preloaded_hashes,
		(hashfile->preloaded_hash_count + 1) * sizeof(*new_preloaded_hashes));
	if (!new_preloaded_hashes)
		return;

	hashfile->preloaded_hashes = new_preloaded_hashes;
	hashfile->preloaded_hashes[hashfile->preloaded_hash_count++] = hi;
}



/*-------------------------------------------------
    hashfile_open
-------------------------------------------------*/

hash_file *hashfile_open(emu_options &options, const char *sysname, int is_preload,
	void (*error_proc)(const char *message))
{
	hash_file *hashfile = NULL;
	object_pool *pool = NULL;
	file_error filerr;

	/* create a pool for this hash file */
	pool = pool_alloc_lib(error_proc);
	if (!pool)
		goto error;

	/* allocate space for this hash file */
	hashfile = (hash_file *) pool_malloc_lib(pool, sizeof(*hashfile));
	if (!hashfile)
		goto error;

	/* set up the hashfile structure */
	memset(hashfile, 0, sizeof(*hashfile));
	hashfile->pool = pool;
	hashfile->error_proc = error_proc;

	/* open a file */
	hashfile->file = global_alloc(emu_file(options.hash_path(), OPEN_FLAG_READ));
	filerr = hashfile->file->open(sysname, ".hsi");
	if (filerr != FILERR_NONE)
	{
		global_free(hashfile->file);
		hashfile->file = NULL;
		goto error;
	}

	if (is_preload)
		hashfile_parse(hashfile, NULL, preload_use_proc, hashfile->error_proc, NULL);

	return hashfile;

error:
	if (hashfile != NULL)
		hashfile_close(hashfile);
	return NULL;
}



/*-------------------------------------------------
    hashfile_close
-------------------------------------------------*/

void hashfile_close(hash_file *hashfile)
{
	global_free(hashfile->file);
	pool_free_lib(hashfile->pool);
}



/*-------------------------------------------------
    singular_selector_proc
-------------------------------------------------*/

struct hashlookup_params
{
	const hash_collection *hashes;
	hash_info *hi;
};

static int singular_selector_proc(hash_file *hashfile, void *param, const char *name, const hash_collection *hashes)
{
	astring tempstr;
	struct hashlookup_params *hlparams = (struct hashlookup_params *) param;
	return (*hashes == *hlparams->hashes);
}



/*-------------------------------------------------
    singular_use_proc
-------------------------------------------------*/

static void singular_use_proc(hash_file *hashfile, void *param, hash_info *hi)
{
	struct hashlookup_params *hlparams = (struct hashlookup_params *) param;
	hlparams->hi = hi;
}



/*-------------------------------------------------
    hashfile_lookup
-------------------------------------------------*/

const hash_info *hashfile_lookup(hash_file *hashfile, const hash_collection *hashes)
{
	struct hashlookup_params param;
	int i;

	param.hashes = hashes;
	param.hi = NULL;

	for (i = 0; i < hashfile->preloaded_hash_count; i++)
	{
		if (singular_selector_proc(hashfile, &param, NULL, hashfile->preloaded_hashes[i]->hashes))
			return hashfile->preloaded_hashes[i];
	}

	hashfile_parse(hashfile, singular_selector_proc, singular_use_proc,
		hashfile->error_proc, (void *) &param);
	return param.hi;
}

const char *extra_info = NULL;

const char *read_hash_config(device_image_interface &image, const char *sysname)
{
	hash_file *hashfile = NULL;
	const hash_info *info = NULL;

	/* open the hash file */
	hashfile = hashfile_open(image.device().machine().options(), sysname, FALSE, NULL);
	if (!hashfile)
		return NULL;

	/* look up this entry in the hash file */
	info = hashfile_lookup(hashfile, &image.hash());

	if (!info || !info->extrainfo)
	{
		hashfile_close(hashfile);
		return NULL;
	}

	extra_info = auto_strdup(image.device().machine(), info->extrainfo);
	if (!extra_info)
	{
		hashfile_close(hashfile);
		return NULL;
	}

	/* copy the relevant entries */
	hashfile_close(hashfile);

	return extra_info;
}

const char *hashfile_extrainfo(device_image_interface &image)
{
	const char *rc;

	/* now read the hash file */
	image.crc();
	extra_info = NULL;
	int drv = driver_list::find(image.device().machine().system());
	int compat, open = drv;
	do
	{
		rc = read_hash_config(image, driver_list::driver(open).name);
		// first check if there are compatible systems
		compat = driver_list::compatible_with(open);
		// if so, try to open its hashfile
		if (compat != -1)
			open = compat;
		// otherwise, try with the parent
		else
		{
			drv = driver_list::clone(drv);
			open = drv;
		}
	}
	// if no extrainfo has been found but we can try a compatible or a parent set, go back
	while (rc == NULL && open != -1);
	return rc;
}

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
