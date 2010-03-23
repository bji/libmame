/** **************************************************************************
 * osd_directory.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "osdcore.h"
#include "osdepend.h"
#include "osd_util.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

struct _osd_directory
{
    DIR *posix_dir;
    /* It is assumed that a call to osd_readdir will invalidate the previous
       osd_directory_entry returned, and thus only one such entry needs to be
       retained for a given osd_directory.  This is the only sane way for this
       API to work. */
    osd_directory_entry current_entry;
    /* This is the path that has most recently been returned.  It is composed
       here from a prefix that is the path of the osd_directory, plus the
       current entry name */
    char current_entry_path[4096];
    /* This is the length of the prefix that is saved in current_entry_path,
       which is itself the path of this osd_directory */
    int base_len;
};

osd_directory *osd_opendir(const char *dirname)
{
    char posix_dirname[4096];

    /* Convert the path to a POSIX path */
    if (convert_to_posix_path(dirname, posix_dirname, sizeof(posix_dirname)))
    {
        return NULL;
    }

    DIR *dir = opendir(posix_dirname);
    if (dir == NULL)
    {
        return NULL;
    }

    osd_directory *od = (osd_directory *) osd_malloc(sizeof(osd_directory));

    if (od == NULL)
    {
        closedir(dir);
        return NULL;
    }

    od->posix_dir = dir;

    /* Copy in the posix path */
    od->base_len = snprintf(od->current_entry_path, 
                            sizeof(od->current_entry_path), "%s", 
                            posix_dirname);
    if (od->base_len >= sizeof(od->current_entry_path))
    {
        closedir(dir);
        osd_free(od);
        return NULL;
    }
    /* If it wasn't terminated with '/', terminate it now */
    if (od->current_entry_path[od->base_len - 1] != '/')
    {
        if (od->base_len == (sizeof(od->current_entry_path) - 1))
        {
            closedir(dir);
            osd_free(od);
            return NULL;
        }
        od->current_entry_path[od->base_len - 1] = '/';
        od->base_len += 1;
    }

    return od;
}


const osd_directory_entry *osd_readdir(osd_directory *dir)
{
    struct dirent *dentry = readdir(dir->posix_dir);

    if (dentry == NULL)
    {
        return NULL;
    }

    int amt_available = sizeof(dir->current_entry_path) - dir->base_len;

    dir->current_entry.name = dir->current_entry_path;
    if (snprintf(&(dir->current_entry_path[dir->base_len]),
                 amt_available, "%s", dentry->d_name) >= amt_available)
    {
        return NULL;
    }
    
    /* We do not rely on Linux' non-POSIX d_type parameter */
    struct stat statbuf;
    if (stat(dir->current_entry_path, /* returns */ &statbuf))
    {
        /* No really reasonable way to proceed */
        dir->current_entry.type = ENTTYPE_NONE;
        dir->current_entry.size = 0;
    }
    else
    {
        if (S_ISREG(statbuf.st_mode))
        {
            dir->current_entry.type = ENTTYPE_FILE;
        }
        else if (S_ISDIR(statbuf.st_mode))
        {
            dir->current_entry.type = ENTTYPE_DIR;
        }
        else
        {
            dir->current_entry.type = ENTTYPE_OTHER;
        }
        dir->current_entry.size = statbuf.st_size;
    }

    return &(dir->current_entry);
}


void osd_closedir(osd_directory *dir)
{
    closedir(dir->posix_dir);
    osd_free(dir);
}


int osd_is_absolute_path(const char *path)
{
    return !strncmp(path, PATH_SEPARATOR, sizeof(PATH_SEPARATOR) - 1);
}
