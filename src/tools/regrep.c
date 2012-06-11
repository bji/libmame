/***************************************************************************

    Regression test report generator

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

****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <new>
#include "osdcore.h"
#include "png.h"


/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

#define MAX_COMPARES            16
#define BITMAP_SPACE			4

enum
{
	STATUS_NOT_PRESENT = 0,
	STATUS_SUCCESS,
	STATUS_SUCCESS_DIFFERENT,
	STATUS_MISSING_FILES,
	STATUS_EXCEPTION,
	STATUS_FATAL_ERROR,
	STATUS_FAILED_VALIDITY,
	STATUS_OTHER,
	STATUS_COUNT
};

enum
{
	BUCKET_UNKNOWN = 0,
	BUCKET_IMPROVED,
	BUCKET_REGRESSED,
	BUCKET_CHANGED,
	BUCKET_MULTI_ERROR,
	BUCKET_CONSISTENT_ERROR,
	BUCKET_GOOD,
	BUCKET_GOOD_BUT_CHANGED,
	BUCKET_GOOD_BUT_CHANGED_SCREENSHOTS,
	BUCKET_COUNT
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _summary_file summary_file;
struct _summary_file
{
    summary_file *  next;
    char            name[20];
    char            source[100];
    UINT8           status[MAX_COMPARES];
    UINT8			matchbitmap[MAX_COMPARES];
    char *          text[MAX_COMPARES];
    UINT32          textsize[MAX_COMPARES];
    UINT32          textalloc[MAX_COMPARES];
};


typedef struct _summary_list summary_list;
struct _summary_list
{
    summary_list *  next;
    summary_file *  files;
    char *			dir;
    char			version[40];
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static summary_file *filehash[128][128];
static summary_list lists[MAX_COMPARES];
static int list_count;

static const char *const bucket_name[] =
{
	"Unknown",
	"Games That Have Improved",
	"Games That Have Regressed",
	"Games With Changed Screenshots",
	"Games With Multiple Errors",
	"Games With Consistent Errors",
	"Games That Are Consistently Good",
	"Games That Regressed But Improved",
	"Games With Changed Screenshots",
};

static const int bucket_output_order[] =
{
	BUCKET_REGRESSED,
	BUCKET_IMPROVED,
	BUCKET_CHANGED,
	BUCKET_GOOD_BUT_CHANGED_SCREENSHOTS,
	BUCKET_GOOD_BUT_CHANGED,
	BUCKET_MULTI_ERROR,
	BUCKET_CONSISTENT_ERROR
};

static const char *const status_text[] =
{
	"",
	"Success",
	"Changed",
	"Missing Files",
	"Exception",
	"Fatal Error",
	"Failed Validity Check",
	"Other Unknown Error"
};

static const char *const status_color[] =
{
	"",
	"background:#00A000",
	"background:#E0E000",
	"background:#8000C0",
	"background:#C00000",
	"background:#C00000",
	"background:#C06000",
	"background:#C00000",
	"background:#C00000",
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* summary parsing */
static int read_summary_log(const char *filename, int index);
static summary_file *parse_driver_tag(char *linestart, int index);
static summary_file *get_file(const char *filename);
static int CLIB_DECL compare_file(const void *file0ptr, const void *file1ptr);
static summary_file *sort_file_list(void);

/* HTML helpers */
static core_file *create_file_and_output_header(astring &filename, astring &templatefile, astring &title);
static void output_footer_and_close_file(core_file *file, astring &templatefile, astring &title);

/* report generators */
static void output_report(astring &dirname, astring &tempheader, astring &tempfooter, summary_file *filelist);
static int compare_screenshots(summary_file *curfile);
static int generate_png_diff(const summary_file *curfile, astring &destdir, const char *destname);
static void create_linked_file(astring &dirname, const summary_file *curfile, const summary_file *prevfile, const summary_file *nextfile, const char *pngfile, astring &tempheader, astring &tempfooter);
static void append_driver_list_table(const char *header, astring &dirname, core_file *indexfile, const summary_file *listhead, astring &tempheader, astring &tempfooter);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    trim_string - trim leading/trailing spaces
    from a string
-------------------------------------------------*/

INLINE char *trim_string(char *string)
{
	int length;

	/* trim leading spaces */
	while (*string != 0 && isspace((UINT8)*string))
		string++;

	/* trim trailing spaces */
	length = strlen(string);
	while (length > 0 && isspace((UINT8)string[length - 1]))
		string[--length] = 0;

	return string;
}


/*-------------------------------------------------
    get_unique_index - get the unique bitmap
    index for a given entry
-------------------------------------------------*/

INLINE int get_unique_index(const summary_file *curfile, int index)
{
	int listnum, curindex = 0;

	/* if we're invalid, just return that */
	if (curfile->matchbitmap[index] == 0xff)
		return -1;

	/* count unique elements up to us */
	for (listnum = 0; listnum < curfile->matchbitmap[index]; listnum++)
		if (curfile->matchbitmap[listnum] == listnum)
			curindex++;
	return curindex;
}



/***************************************************************************
    MAIN
***************************************************************************/

/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	UINT32 bufsize;
	void *buffer;
	int listnum;
    int result;

    /* first argument is the directory */
    if (argc < 4)
    {
    	fprintf(stderr, "Usage:\nregrep <template> <outputdir> <summary1> [<summary2> [<summary3> ...]]\n");
    	return 1;
    }
    astring tempfilename(argv[1]);
    astring dirname(argv[2]);
    list_count = argc - 3;

	/* read the template file into an astring */
	astring tempheader;
	if (core_fload(tempfilename, &buffer, &bufsize) == FILERR_NONE)
	{
		tempheader.cpy((const char *)buffer, bufsize);
		osd_free(buffer);
	}

	/* verify the template */
	if (tempheader.len() == 0)
	{
		fprintf(stderr, "Unable to read template file\n");
		return 1;
	}
	result = tempheader.find(0, "<!--CONTENT-->");
	if (result == -1)
	{
		fprintf(stderr, "Template is missing a <!--CONTENT--> marker\n");
		return 1;
	}
	astring tempfooter(tempheader);
	tempfooter.substr(result + 14);
	tempheader.substr(0, result);

    /* loop over arguments and read the files */
    for (listnum = 0; listnum < list_count; listnum++)
    {
        result = read_summary_log(argv[listnum + 3], listnum);
        if (result != 0)
            return result;
    }

    /* output the summary */
    output_report(dirname, tempheader, tempfooter, sort_file_list());
    return 0;
}



/***************************************************************************
    SUMMARY PARSING
***************************************************************************/

/*-------------------------------------------------
    get_file - lookup a driver name in the hash
    table and return a pointer to it; if none
    found, allocate a new entry
-------------------------------------------------*/

static summary_file *get_file(const char *filename)
{
    summary_file *file;

    /* use the first two characters as a lookup */
    for (file = filehash[filename[0] & 0x7f][filename[1] & 0x7f]; file != NULL; file = file->next)
        if (strcmp(filename, file->name) == 0)
            return file;

    /* didn't find one -- allocate */
    file = (summary_file *)malloc(sizeof(*file));
    if (file == NULL)
        return NULL;
    memset(file, 0, sizeof(*file));

    /* set the name so we find it in the future */
    strcpy(file->name, filename);

    /* add to the head of the list */
    file->next = filehash[filename[0] & 0x7f][filename[1] & 0x7f];
    filehash[filename[0] & 0x7f][filename[1] & 0x7f] = file;
    return file;
}


/*-------------------------------------------------
    read_summary_log - read a summary.log file
    and build entries for its data
-------------------------------------------------*/

static int read_summary_log(const char *filename, int index)
{
    summary_file *curfile = NULL;
    char linebuffer[1024];
    char *linestart;
    int drivers = 0;
    FILE *file;

    /* open the logfile */
    file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error: file '%s' not found\n", filename);
        return 1;
    }

    /* parse it */
    while (fgets(linebuffer, sizeof(linebuffer), file) != NULL)
    {
    	/* trim the leading/trailing spaces */
    	linestart = trim_string(linebuffer);

        /* is this one of our specials? */
        if (strncmp(linestart, "@@@@@", 5) == 0)
        {
        	/* advance past the signature */
        	linestart += 5;

			/* look for the driver= tag */
			if (strncmp(linestart, "driver=", 7) == 0)
			{
				curfile = parse_driver_tag(linestart + 7, index);
				if (curfile == NULL)
					goto error;
				drivers++;
			}

			/* look for the source= tag */
			else if (strncmp(linestart, "source=", 7) == 0)
			{
				/* error if no driver yet */
				if (curfile == NULL)
				{
					fprintf(stderr, "Unexpected @@@@@source= tag\n");
					goto error;
				}

				/* copy the string */
				strcpy(curfile->source, trim_string(linestart + 7));
			}

			/* look for the dir= tag */
			else if (strncmp(linestart, "dir=", 4) == 0)
			{
				char *dirname = trim_string(linestart + 4);

				/* allocate a copy of the string */
				lists[index].dir = (char *)malloc(strlen(dirname) + 1);
				if (lists[index].dir == NULL)
					goto error;
				strcpy(lists[index].dir, dirname);
				fprintf(stderr, "Directory %s\n", lists[index].dir);
			}
		}

        /* if not, consider other options */
        else if (curfile != NULL)
        {
            int foundchars = 0;
            char *curptr;

            /* look for the pngcrc= tag */
			if (strncmp(linestart, "pngcrc: ", 7) == 0)
            {
            }

			/* otherwise, accumulate the text */
			else
			{
				/* find the end of the line and normalize it with a CR */
				for (curptr = linestart; *curptr != 0 && *curptr != '\n' && *curptr != '\r'; curptr++)
					if (!isspace((UINT8)*curptr))
						foundchars = 1;
				*curptr++ = '\n';
				*curptr = 0;

				/* ignore blank lines */
				if (!foundchars)
					continue;

				/* see if we have enough room */
				if (curfile->textsize[index] + (curptr - linestart) + 1 >= curfile->textalloc[index])
				{
					curfile->textalloc[index] = curfile->textsize[index] + (curptr - linestart) + 256;
					curfile->text[index] = (char *)realloc(curfile->text[index], curfile->textalloc[index]);
					if (curfile->text[index] == NULL)
					{
						fprintf(stderr, "Unable to allocate memory for text\n");
						goto error;
					}
				}

				/* append our text */
				strcpy(curfile->text[index] + curfile->textsize[index], linestart);
				curfile->textsize[index] += curptr - linestart;
			}
		}

		/* look for the M.A.M.E. header */
		else if (strncmp(linestart, "M.A.M.E. v", 10) == 0)
		{
			char *start = linestart + 10;
			char *end;

			/* find the end */
			for (end = start; !isspace((UINT8)*end); end++) ;
			*end = 0;
			strcpy(lists[index].version, start);
			fprintf(stderr, "Parsing results from version %s\n", lists[index].version);
		}
    }

    fclose(file);
    fprintf(stderr, "Parsed %d drivers\n", drivers);
    return 0;

error:
    fclose(file);
    return 1;
}


/*-------------------------------------------------
    parse_driver_tag - parse the status info
    from a driver tag
-------------------------------------------------*/

static summary_file *parse_driver_tag(char *linestart, int index)
{
	summary_file *curfile;
	char *colon;

	/* find the colon separating name from status */
	colon = strchr(linestart, ':');
	if (colon == NULL)
	{
		fprintf(stderr, "Unexpected text after @@@@@driver=\n");
		return NULL;
	}

	/* NULL terminate at the colon and look up the file */
	*colon = 0;
	curfile = get_file(trim_string(linestart));
	if (curfile == NULL)
	{
		fprintf(stderr, "Unable to allocate memory for driver\n");
		return NULL;
	}

	/* clear out any old status for this file */
	curfile->status[index] = STATUS_NOT_PRESENT;
	if (curfile->text[index] != NULL)
		free(curfile->text[index]);
	curfile->text[index] = NULL;
	curfile->textsize[index] = 0;
	curfile->textalloc[index] = 0;

	/* strip leading/trailing spaces from the status */
	colon = trim_string(colon + 1);

	/* convert status into statistics */
	if (strcmp(colon, "Success") == 0)
		curfile->status[index] = STATUS_SUCCESS;
	else if (strcmp(colon, "Missing files") == 0)
		curfile->status[index] = STATUS_MISSING_FILES;
	else if (strcmp(colon, "Exception") == 0)
		curfile->status[index] = STATUS_EXCEPTION;
	else if (strcmp(colon, "Fatal error") == 0)
		curfile->status[index] = STATUS_FATAL_ERROR;
	else if (strcmp(colon, "Failed validity check") == 0)
		curfile->status[index] = STATUS_FAILED_VALIDITY;
	else
		curfile->status[index] = STATUS_OTHER;

	return curfile;
}


/*-------------------------------------------------
    compare_file - compare two files, sorting
    first by source filename, then by driver name
-------------------------------------------------*/

static int CLIB_DECL compare_file(const void *file0ptr, const void *file1ptr)
{
	summary_file *file0 = *(summary_file **)file0ptr;
	summary_file *file1 = *(summary_file **)file1ptr;
	int result = strcmp(file0->source, file1->source);
	if (result == 0)
		result = strcmp(file0->name, file1->name);
	return result;
}


/*-------------------------------------------------
    sort_file_list - convert the hashed lists
    into a single, sorted list
-------------------------------------------------*/

static summary_file *sort_file_list(void)
{
	summary_file *listhead, **tailptr, *curfile, **filearray;
	int numfiles, filenum;
    int c0, c1;

    /* count the total number of files */
    numfiles = 0;
    for (c0 = 0; c0 < 128; c0++)
	    for (c1 = 0; c1 < 128; c1++)
	    	for (curfile = filehash[c0][c1]; curfile != NULL; curfile = curfile->next)
	    		numfiles++;

	/* allocate an array of files */
	filearray = (summary_file **)malloc(numfiles * sizeof(*filearray));
	if (filearray == NULL)
	{
		fprintf(stderr, "Out of memory!\n");
		return NULL;
	}

	/* populate the array */
    numfiles = 0;
    for (c0 = 0; c0 < 128; c0++)
	    for (c1 = 0; c1 < 128; c1++)
	    	for (curfile = filehash[c0][c1]; curfile != NULL; curfile = curfile->next)
	    		filearray[numfiles++] = curfile;

	/* sort the array */
	qsort(filearray, numfiles, sizeof(filearray[0]), compare_file);

	/* now regenerate a single list */
    listhead = NULL;
    tailptr = &listhead;
    for (filenum = 0; filenum < numfiles; filenum++)
    {
    	*tailptr = filearray[filenum];
    	tailptr = &(*tailptr)->next;
    }
    *tailptr = NULL;
    free(filearray);

    return listhead;
}



/***************************************************************************
    HTML OUTPUT HELPERS
***************************************************************************/

/*-------------------------------------------------
    create_file_and_output_header - create a new
    HTML file with a standard header
-------------------------------------------------*/

static core_file *create_file_and_output_header(astring &filename, astring &templatefile, astring &title)
{
	core_file *file;

	/* create the indexfile */
	if (core_fopen(filename, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS | OPEN_FLAG_NO_BOM, &file) != FILERR_NONE)
		return NULL;

	/* print a header */
	astring modified(templatefile);
	modified.replace("<!--TITLE-->", title);
	core_fwrite(file, modified.cstr(), modified.len());

	/* return the file */
	return file;
}


/*-------------------------------------------------
    output_footer_and_close_file - write a
    standard footer to an HTML file and close it
-------------------------------------------------*/

static void output_footer_and_close_file(core_file *file, astring &templatefile, astring &title)
{
	astring modified(templatefile);
	modified.replace(0, "<!--TITLE-->", title);
	core_fwrite(file, modified.cstr(), modified.len());
	core_fclose(file);
}



/***************************************************************************
    REPORT GENERATORS
***************************************************************************/

/*-------------------------------------------------
    output_report - generate the summary
    report HTML files
-------------------------------------------------*/

static void output_report(astring &dirname, astring &tempheader, astring &tempfooter, summary_file *filelist)
{
	summary_file *buckethead[BUCKET_COUNT], **buckettailptr[BUCKET_COUNT];
	summary_file *curfile;
	astring title("MAME Regressions");
	astring tempname;
	int listnum, bucknum;
	core_file *indexfile;
	int count = 0, total;

	/* initialize the lists */
	for (bucknum = 0; bucknum < BUCKET_COUNT; bucknum++)
	{
		buckethead[bucknum] = NULL;
		buckettailptr[bucknum] = &buckethead[bucknum];
	}

	/* compute the total number of files */
	total = 0;
	for (curfile = filelist; curfile != NULL; curfile = curfile->next)
		total++;

	/* first bucketize the games */
	for (curfile = filelist; curfile != NULL; curfile = curfile->next)
	{
		int statcount[STATUS_COUNT] = { 0 };
		int bucket = BUCKET_UNKNOWN;
		int unique_codes = 0;
		int first_valid;

		/* print status */
		if (++count % 100 == 0)
			fprintf(stderr, "Processing file %d/%d\n", count, total);

		/* find the first valid entry */
		for (first_valid = 0; curfile->status[first_valid] == STATUS_NOT_PRESENT; first_valid++) ;

		/* do we need to output anything? */
		for (listnum = first_valid; listnum < list_count; listnum++)
			if (statcount[curfile->status[listnum]]++ == 0)
				unique_codes++;

		/* were we consistent? */
		if (unique_codes == 1)
		{
			/* were we consistently ok? */
			if (curfile->status[first_valid] == STATUS_SUCCESS)
				bucket = compare_screenshots(curfile);

			/* must have been consistently erroring */
			else
				bucket = BUCKET_CONSISTENT_ERROR;
		}

		/* ok, we're not consistent; could be a number of things */
		else
		{
			/* were we ok at the start and end but not in the middle? */
			if (curfile->status[first_valid] == STATUS_SUCCESS && curfile->status[list_count - 1] == STATUS_SUCCESS)
				bucket = BUCKET_GOOD_BUT_CHANGED;

			/* did we go from good to bad? */
			else if (curfile->status[first_valid] == STATUS_SUCCESS)
				bucket = BUCKET_REGRESSED;

			/* did we go from bad to good? */
			else if (curfile->status[list_count - 1] == STATUS_SUCCESS)
				bucket = BUCKET_IMPROVED;

			/* must have had multiple errors */
			else
				bucket = BUCKET_MULTI_ERROR;
		}

		/* add us to the appropriate list */
		*buckettailptr[bucket] = curfile;
		buckettailptr[bucket] = &curfile->next;
	}

	/* terminate all the lists */
	for (bucknum = 0; bucknum < BUCKET_COUNT; bucknum++)
		*buckettailptr[bucknum] = NULL;

	/* output header */
	tempname.printf("%s" PATH_SEPARATOR "%s", dirname.cstr(), "index.html");
	indexfile = create_file_and_output_header(tempname, tempheader, title);
	if (indexfile == NULL)
	{
		fprintf(stderr, "Error creating file '%s'\n", tempname.cstr());
		return;
	}

	/* iterate over buckets and output them */
	for (bucknum = 0; bucknum < ARRAY_LENGTH(bucket_output_order); bucknum++)
	{
		int curbucket = bucket_output_order[bucknum];

		if (buckethead[curbucket] != NULL)
		{
			fprintf(stderr, "Outputting bucket: %s\n", bucket_name[curbucket]);
			append_driver_list_table(bucket_name[curbucket], dirname, indexfile, buckethead[curbucket], tempheader, tempfooter);
		}
	}

	/* output footer */
	output_footer_and_close_file(indexfile, tempfooter, title);
}


/*-------------------------------------------------
    compare_screenshots - compare the screenshots
    for all the games in a file
-------------------------------------------------*/

static int compare_screenshots(summary_file *curfile)
{
	bitmap_argb32 bitmaps[MAX_COMPARES];
	int unique[MAX_COMPARES];
	int numunique = 0;
	int listnum;

	/* iterate over all files and load their bitmaps */
	for (listnum = 0; listnum < list_count; listnum++)
		if (curfile->status[listnum] == STATUS_SUCCESS)
		{
			astring fullname;
			file_error filerr;
			core_file *file;

			/* get the filename for the image */
			fullname.printf("%s" PATH_SEPARATOR "snap" PATH_SEPARATOR "%s" PATH_SEPARATOR "final.png", lists[listnum].dir, curfile->name);

			/* open the file */
			filerr = core_fopen(fullname, OPEN_FLAG_READ, &file);

			/* if that failed, look in the old location */
			if (filerr != FILERR_NONE)
			{
				/* get the filename for the image */
				fullname.printf("%s" PATH_SEPARATOR "snap" PATH_SEPARATOR "_%s.png", lists[listnum].dir, curfile->name);

				/* open the file */
				filerr = core_fopen(fullname, OPEN_FLAG_READ, &file);
			}

			/* if that worked, load the file */
			if (filerr == FILERR_NONE)
			{
				png_read_bitmap(file, bitmaps[listnum]);
				core_fclose(file);
			}
		}

	/* now find all the different bitmap types */
	for (listnum = 0; listnum < list_count; listnum++)
	{
		curfile->matchbitmap[listnum] = 0xff;
		if (bitmaps[listnum].valid())
		{
			bitmap_argb32 &this_bitmap = bitmaps[listnum];

			/* compare against all unique bitmaps */
			int compnum;
			for (compnum = 0; compnum < numunique; compnum++)
			{
				/* if the sizes are different, we differ; otherwise start off assuming we are the same */
				bitmap_argb32 &base_bitmap = bitmaps[unique[compnum]];
				bool bitmaps_differ = (this_bitmap.width() != base_bitmap.width() || this_bitmap.height() != base_bitmap.height());

				/* compare scanline by scanline */
				for (int y = 0; y < this_bitmap.height() && !bitmaps_differ; y++)
				{
					UINT32 *base = &base_bitmap.pix32(y);
					UINT32 *curr = &this_bitmap.pix32(y);

					/* scan the scanline */
					int x;
					for (x = 0; x < this_bitmap.width(); x++)
						if (*base++ != *curr++)
							break;
					bitmaps_differ = (x != this_bitmap.width());
				}

				/* if we matched, remember which listnum index we matched, and stop */
				if (!bitmaps_differ)
				{
					curfile->matchbitmap[listnum] = unique[compnum];
					break;
				}

				/* if different from the first unique entry, adjust the status */
				if (bitmaps_differ && compnum == 0)
					curfile->status[listnum] = STATUS_SUCCESS_DIFFERENT;
			}

			/* if we're unique, add ourselves to the list */
			if (compnum >= numunique)
			{
				unique[numunique++] = listnum;
				curfile->matchbitmap[listnum] = listnum;
				continue;
			}
		}
	}

	/* if all screenshots matched, we're good */
	if (numunique == 1)
		return BUCKET_GOOD;

	/* if the last screenshot matched the first unique one, we're good but changed */
	if (curfile->matchbitmap[listnum - 1] == unique[0])
		return BUCKET_GOOD_BUT_CHANGED_SCREENSHOTS;

	/* otherwise we're just changed */
	return BUCKET_CHANGED;
}


/*-------------------------------------------------
    generate_png_diff - create a new PNG file
    that shows multiple differing PNGs side by
    side with a third set of differences
-------------------------------------------------*/

static int generate_png_diff(const summary_file *curfile, astring &destdir, const char *destname)
{
	bitmap_argb32 bitmaps[MAX_COMPARES];
	astring srcimgname;
	astring dstfilename;
	astring tempname;
	bitmap_argb32 finalbitmap;
	int width, height, maxwidth;
	int bitmapcount = 0;
	int listnum, bmnum;
	core_file *file = NULL;
	file_error filerr;
	png_error pngerr;
	int error = -1;
	int starty;

	/* generate the common source filename */
	dstfilename.printf("%s" PATH_SEPARATOR "%s", destdir.cstr(), destname);
	srcimgname.printf("snap" PATH_SEPARATOR "%s" PATH_SEPARATOR "final.png", curfile->name);

	/* open and load all unique bitmaps */
	for (listnum = 0; listnum < list_count; listnum++)
		if (curfile->matchbitmap[listnum] == listnum)
		{
			tempname.printf("%s" PATH_SEPARATOR "%s", lists[listnum].dir, srcimgname.cstr());

			/* open the source image */
			filerr = core_fopen(tempname, OPEN_FLAG_READ, &file);
			if (filerr != FILERR_NONE)
				goto error;

			/* load the source image */
			pngerr = png_read_bitmap(file, bitmaps[bitmapcount++]);
			core_fclose(file);
			if (pngerr != PNGERR_NONE)
				goto error;
		}

	/* if there's only one unique bitmap, skip it */
	if (bitmapcount <= 1)
		goto error;

	/* determine the size of the final bitmap */
	height = width = 0;
	maxwidth = bitmaps[0].width();
	for (bmnum = 1; bmnum < bitmapcount; bmnum++)
	{
		int curwidth;

		/* determine the maximal width */
		maxwidth = MAX(maxwidth, bitmaps[bmnum].width());
		curwidth = bitmaps[0].width() + BITMAP_SPACE + maxwidth + BITMAP_SPACE + maxwidth;
		width = MAX(width, curwidth);

		/* add to the height */
		height += MAX(bitmaps[0].height(), bitmaps[bmnum].height());
		if (bmnum != 1)
			height += BITMAP_SPACE;
	}

	/* allocate the final bitmap */
	finalbitmap.allocate(width, height);

	/* now copy and compare each set of bitmaps */
	starty = 0;
	for (bmnum = 1; bmnum < bitmapcount; bmnum++)
	{
		bitmap_argb32 &bitmap1 = bitmaps[0];
		bitmap_argb32 &bitmap2 = bitmaps[bmnum];
		int curheight = MAX(bitmap1.height(), bitmap2.height());
		int x, y;

		/* iterate over rows in these bitmaps */
		for (y = 0; y < curheight; y++)
		{
			UINT32 *src1 = (y < bitmap1.height()) ? &bitmap1.pix32(y) : NULL;
			UINT32 *src2 = (y < bitmap2.height()) ? &bitmap2.pix32(y) : NULL;
			UINT32 *dst1 = &finalbitmap.pix32(starty + y, 0);
			UINT32 *dst2 = &finalbitmap.pix32(starty + y, bitmap1.width() + BITMAP_SPACE);
			UINT32 *dstdiff = &finalbitmap.pix32(starty + y, bitmap1.width() + BITMAP_SPACE + maxwidth + BITMAP_SPACE);

			/* now iterate over columns */
			for (x = 0; x < maxwidth; x++)
			{
				int pix1 = -1, pix2 = -2;

				if (src1 != NULL && x < bitmap1.width())
					pix1 = dst1[x] = src1[x];
				if (src2 != NULL && x < bitmap2.width())
					pix2 = dst2[x] = src2[x];
				dstdiff[x] = (pix1 != pix2) ? 0xffffffff : 0xff000000;
			}
		}

		/* update the starting Y position */
		starty += BITMAP_SPACE + MAX(bitmap1.height(), bitmap2.height());
	}

	/* write the final PNG */
	filerr = core_fopen(dstfilename, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &file);
	if (filerr != FILERR_NONE)
		goto error;
	pngerr = png_write_bitmap(file, NULL, finalbitmap, 0, NULL);
	core_fclose(file);
	if (pngerr != PNGERR_NONE)
		goto error;

	/* if we get here, we are error free */
	error = 0;

error:
	if (error)
		osd_rmfile(dstfilename);
	return error;
}


/*-------------------------------------------------
    create_linked_file - create a comparison
    file between differing versions
-------------------------------------------------*/

static void create_linked_file(astring &dirname, const summary_file *curfile, const summary_file *prevfile, const summary_file *nextfile, const char *pngfile, astring &tempheader, astring &tempfooter)
{
	astring linkname;
	astring filename;
	astring title;
	core_file *linkfile;
	int listnum;

	/* create the filename */
	filename.printf("%s.html", curfile->name);

	/* output header */
	title.printf("%s Regressions (%s)", curfile->name, curfile->source);
	linkname.printf("%s" PATH_SEPARATOR "%s", dirname.cstr(), filename.cstr());
	linkfile = create_file_and_output_header(linkname, tempheader, title);
	if (linkfile == NULL)
	{
		fprintf(stderr, "Error creating file '%s'\n", filename.cstr());
		return;
	}

	/* link to the previous/next entries */
	core_fprintf(linkfile, "\t<p>\n");
	core_fprintf(linkfile, "\t<table width=\"100%%\">\n");
	core_fprintf(linkfile, "\t\t<td align=\"left\" width=\"40%%\" style=\"border:none\">");
	if (prevfile != NULL)
		core_fprintf(linkfile, "<a href=\"%s.html\"><< %s (%s)</a>", prevfile->name, prevfile->name, prevfile->source);
	core_fprintf(linkfile, "</td>\n");
	core_fprintf(linkfile, "\t\t<td align=\"center\" width=\"20%%\" style=\"border:none\"><a href=\"index.html\">Home</a></td>\n");
	core_fprintf(linkfile, "\t\t<td align=\"right\" width=\"40%%\" style=\"border:none\">");
	if (nextfile != NULL)
		core_fprintf(linkfile, "<a href=\"%s.html\">%s (%s) >></a>", nextfile->name, nextfile->name, nextfile->source);
	core_fprintf(linkfile, "</td>\n");
	core_fprintf(linkfile, "\t</table>\n");
	core_fprintf(linkfile, "\t</p>\n");

	/* output data for each one */
	for (listnum = 0; listnum < list_count; listnum++)
	{
		int imageindex = -1;

		/* generate the HTML */
		core_fprintf(linkfile, "\n\t<h2>%s</h2>\n", lists[listnum].version);
		core_fprintf(linkfile, "\t<p>\n");
		core_fprintf(linkfile, "\t<b>Status:</b> %s\n", status_text[curfile->status[listnum]]);
		if (pngfile != NULL)
			imageindex = get_unique_index(curfile, listnum);
		if (imageindex != -1)
			core_fprintf(linkfile, " [%d]", imageindex);
		core_fprintf(linkfile, "\t</p>\n");
		if (curfile->text[listnum] != NULL)
		{
			core_fprintf(linkfile, "\t<p>\n");
			core_fprintf(linkfile, "\t<b>Errors:</b>\n");
			core_fprintf(linkfile, "\t<pre>%s</pre>\n", curfile->text[listnum]);
			core_fprintf(linkfile, "\t</p>\n");
		}
	}

	/* output link to the image */
	if (pngfile != NULL)
	{
		core_fprintf(linkfile, "\n\t<h2>Screenshot Comparisons</h2>\n");
		core_fprintf(linkfile, "\t<p>\n");
		core_fprintf(linkfile, "\t<img src=\"%s\" />\n", pngfile);
		core_fprintf(linkfile, "\t</p>\n");
	}

	/* output footer */
	output_footer_and_close_file(linkfile, tempfooter, title);
}


/*-------------------------------------------------
    append_driver_list_table - append a table
    of drivers from a list to an HTML file
-------------------------------------------------*/

static void append_driver_list_table(const char *header, astring &dirname, core_file *indexfile, const summary_file *listhead, astring &tempheader, astring &tempfooter)
{
	const summary_file *curfile, *prevfile;
	int width = 100 / (2 + list_count);
	int listnum;

	/* output a header */
	core_fprintf(indexfile, "\t<h2>%s</h2>\n", header);

	/* start the table */
	core_fprintf(indexfile, "\t<p><table width=\"90%%\">\n");
	core_fprintf(indexfile, "\t\t<tr>\n\t\t\t<th width=\"%d%%\">Source</th><th width=\"%d%%\">Driver</th>", width, width);
	for (listnum = 0; listnum < list_count; listnum++)
		core_fprintf(indexfile, "<th width=\"%d%%\">%s</th>", width, lists[listnum].version);
	core_fprintf(indexfile, "\n\t\t</tr>\n");

	/* if nothing, print a default message */
	if (listhead == NULL)
	{
		core_fprintf(indexfile, "\t\t<tr>\n\t\t\t");
		core_fprintf(indexfile, "<td colspan=\"%d\" align=\"center\">(No regressions detected)</td>", list_count + 2);
		core_fprintf(indexfile, "\n\t\t</tr>\n");
	}

	/* iterate over files */
	for (prevfile = NULL, curfile = listhead; curfile != NULL; prevfile = curfile, curfile = curfile->next)
	{
		int rowspan = 0, uniqueshots = 0;
		char pngdiffname[40];

		/* if this is the first entry in this source file, count how many rows we need to span */
		if (prevfile == NULL || strcmp(prevfile->source, curfile->source) != 0)
		{
			const summary_file *cur;
			for (cur = curfile; cur != NULL; cur = cur->next)
				if (strcmp(cur->source, curfile->source) == 0)
					rowspan++;
				else
					break;
		}

		/* create screenshots if necessary */
		pngdiffname[0] = 0;
		for (listnum = 0; listnum < list_count; listnum++)
			if (curfile->matchbitmap[listnum] == listnum)
				uniqueshots++;
		if (uniqueshots > 1)
		{
			sprintf(pngdiffname, "compare_%s.png", curfile->name);
			if (generate_png_diff(curfile, dirname, pngdiffname) != 0)
				pngdiffname[0] = 0;
		}

		/* create a linked file */
		create_linked_file(dirname, curfile, prevfile, curfile->next, (pngdiffname[0] == 0) ? NULL : pngdiffname, tempheader, tempfooter);

		/* create a row */
		core_fprintf(indexfile, "\t\t<tr>\n\t\t\t");
		if (rowspan > 0)
			core_fprintf(indexfile, "<td rowspan=\"%d\">%s</td>", rowspan, curfile->source);
		core_fprintf(indexfile, "<td><a href=\"%s.html\">%s</a></td>", curfile->name, curfile->name);
		for (listnum = 0; listnum < list_count; listnum++)
		{
			int unique_index = -1;

			if (pngdiffname[0] != 0)
				unique_index = get_unique_index(curfile, listnum);
			if (unique_index != -1)
				core_fprintf(indexfile, "<td><span style=\"%s\">&nbsp;&nbsp;&nbsp;</span> %s [<a href=\"%s\" target=\"blank\">%d</a>]</td>", status_color[curfile->status[listnum]], status_text[curfile->status[listnum]], pngdiffname, unique_index);
			else
				core_fprintf(indexfile, "<td><span style=\"%s\">&nbsp;&nbsp;&nbsp;</span> %s</td>", status_color[curfile->status[listnum]], status_text[curfile->status[listnum]]);
		}
		core_fprintf(indexfile, "\n\t\t</tr>\n");

		/* also print the name and source file */
		printf("%s %s\n", curfile->name, curfile->source);
	}

	/* end of table */
	core_fprintf(indexfile, "</table></p>\n");
}
