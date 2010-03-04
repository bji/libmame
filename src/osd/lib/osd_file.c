/** **************************************************************************
 * osd_file.c
 *
 * Copyright Nicola Salmoria and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

/**
 * This file contains some of the glue between the MAME osd layer and libmame.
 * It implements all of the osd_ functions that MAME calls that are related to
 * file I/O, in such a way that they integrate with the API that libmame
 * provides.
 **/

#include "osdcore.h"
#include "osdepend.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct _osd_file
{
    FILE *posix_file;
    UINT64 current_offset; /* To avoid fseek calls */
};


static int convert_to_posix_path(const char *path, char *posix_path,
                                 int pathsize)
{
    char *posix_path_end = &(posix_path[pathsize - 1]);

    while (*path) {
        if (posix_path == posix_path_end) {
            return 1;
        }
        if (!strncmp(path, PATH_SEPARATOR, sizeof(PATH_SEPARATOR) - 1)) {
            *posix_path++ = '/';
            path += sizeof(PATH_SEPARATOR) - 1;
        }
        else {
            *posix_path++ = *path++;
        }
    }

    *posix_path = 0;

    return 0;
}


/* Has the same effect as mkdir -p.  Returns 0 on success and nonzero on
   failure */
static int mkdir_dash_p(const char *path)
{
    /* Support a maximum of 4K path length */
    char dir[4096], *d = dir, *dend = &(dir[sizeof(dir)]);
    const char *p = path, *pnext = p;
    while (1) {
        /* Copy the next path segment into dir */
        while ((p < pnext) && (d < dend)) {
            *d++ = *p++;
        }
        if (d == dend) {
            /* Ran out of room in dir */
            return 1;
        }
        if ((d - dir) > 1) {
            /* Terminate directory.  Now dir is the path of the next directory
               in the hierarchy */
            *(d - 1) = 0;
            struct stat statbuf;
            /* If can't stat it then it probably doesn't exist */
            if (stat(dir, /* returns */ &statbuf)) {
                if (errno != ENOENT) {
                    /* There is something wrong with this path. */
                    return 1;
                }
                /* Create it */
                printf("mkdir %s\n", dir);
                if (mkdir(dir, 0777)) {
                    return 1;
                }
            }
            /* Else can stat it; if it's not a directory, then we're hosed */
            else if (!S_ISDIR(statbuf.st_mode)) {
                return 1;
            }
            /* Else, it exists and is a directory, so skip it */
            *(d - 1) = '/';
        }
        /* Get the next segment */
        p = pnext;
        /* Find the next path separator */
        while (*pnext && (*pnext != '/')) {
            pnext++;
        }
        if (!*pnext) {
            /* No next path separator, so no more directories to
               create */
            break;
        }
        /* Skip the path separator */
        pnext++;
    }

    return 0;
}


file_error osd_open(const char *path, UINT32 openflags, osd_file **file,
                    UINT64 *filesize)
{
    /* POSIX API, so use POSIX path separators */
    char posix_path[4096];
    if (convert_to_posix_path(path, posix_path, sizeof(posix_path))) {
        return FILERR_FAILURE;
    }

    const char *mode;

    /* If necessary, create the paths */
    if (openflags & OPEN_FLAG_CREATE_PATHS) {
        if (mkdir_dash_p(posix_path)) {
            return FILERR_FAILURE;
        }
    }

    /* In POSIX, it is nonsensical to open a file write-only; so we consider
       OPEN_FLAG_WRITE to imply OPEN_FLAG_READ */
    if (openflags & OPEN_FLAG_WRITE) {
        if (openflags & OPEN_FLAG_CREATE) {
            mode = "w+";
        }
        else {
            mode = "r+";
        }
    }
    else {
        /* Don't even bother to check for OPEN_FLAG_READ, just assume it as it
           is nonsensical to call osd_open without OPEN_FLAG_READ or
           OPEN_FLAG_WRITE.  Also don't check for OPEN_FLAG_CREATE, as it is
           nonsensical to create/truncate a file and then open it read-only. */
        mode = "r";
    }

    FILE *f;

    if ((f = fopen(posix_path, mode)) == NULL) {
        switch (errno) {
        case EACCES:
        case EISDIR:
        case EROFS:
            return FILERR_ACCESS_DENIED;
        case ENAMETOOLONG:
        case ENOTDIR:
            return FILERR_INVALID_DATA;
        case EMFILE:
            return FILERR_TOO_MANY_FILES;
        case ENOENT:
            return FILERR_NOT_FOUND;
        case ENOMEM:
            return FILERR_OUT_OF_MEMORY;
        defaut:
            return FILERR_FAILURE;
        }
    }

    struct stat statbuf;

    if (fstat(fileno(f), /* returns */ &statbuf)) {
        switch (errno) {
            (void) fclose(f);
            return FILERR_FAILURE;
        }
    }

    if ((*file = (osd_file *) osd_malloc(sizeof(osd_file))) == NULL) {
        (void) fclose(f);
        return FILERR_OUT_OF_MEMORY;
    }
    
    (*file)->posix_file = f;
    (*file)->current_offset = 0;

    *filesize = statbuf.st_size;

    return FILERR_NONE;
}


file_error osd_close(osd_file *file)
{
    (void) fclose(file->posix_file);

    osd_free(file);

    return FILERR_NONE;
}


file_error osd_read(osd_file *file, void *buffer, UINT64 offset,
                    UINT32 length, UINT32 *actual)
{
    if (file->current_offset != offset) {
        if (fseek(file->posix_file, offset, SEEK_SET)) {
            return FILERR_FAILURE;
        }
        file->current_offset = offset;
    }

    *actual = fread(buffer, 1, length, file->posix_file);
    if ((*actual == 0) && ferror(file->posix_file)) {
        return FILERR_FAILURE;
    }

    file->current_offset += *actual;

    return FILERR_NONE;
}


file_error osd_write(osd_file *file, const void *buffer, UINT64 offset,
                     UINT32 length, UINT32 *actual)
{
    if (file->current_offset != offset) {
        if (fseek(file->posix_file, offset, SEEK_SET)) {
            return FILERR_FAILURE;
        }
        file->current_offset = offset;
    }

    *actual = fwrite(buffer, 1, length, file->posix_file);
    if ((*fwrite == 0) && ferror(file->posix_file)) {
        return FILERR_FAILURE;
    }

    file->current_offset += *actual;

    return FILERR_NONE;
}


file_error osd_rmfile(const char *filename)
{
    if (unlink(filename)) {
        switch (errno) {
        case EACCES:
        case EPERM:
            return FILERR_ACCESS_DENIED;
        case ENOENT:
            /* Consider this to be a success - deleting something that doesn't
               already exist */
            return FILERR_NONE;
        default:
            return FILERR_FAILURE;
        }
    }

    return FILERR_NONE;
}


int osd_get_physical_drive_geometry(const char *filename, UINT32 *cylinders,
                                    UINT32 *heads, UINT32 *sectors,
                                    UINT32 *bps)
{
    /* This has something to do with hard drive images.  Or something. */
    return FILERR_FAILURE;
}


int osd_uchar_from_osdchar(UINT32 /* unicode_char */ *uchar,
                           const char *osdchar, size_t count)
{
    size_t ret = count;

    /* The format of osdchar is OS-defined; we use ISO-8891
       single-byte characters so the conversion is trivial */
    while (count--) {
        *uchar++ = *osdchar++;
    }
    
    return ret;
}
