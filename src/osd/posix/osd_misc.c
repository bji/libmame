/** **************************************************************************
 * osd_misc.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "osdcore.h"
#include "osdepend.h"
#include <signal.h>
#include <stdlib.h>
#ifdef WINDOWS
#include <windows.h>
#else
#include <sys/mman.h>
#endif
#include <sys/time.h>
#include <unistd.h>

/**
 * Undo mame definition of malloc and free as macros.  That doesn't seem
 * kosher to me and it requires these kinds of workarounds.
 **/
#undef malloc
#undef free


osd_ticks_t osd_ticks(void)
{
    struct timeval tv;

    /* This function is not allowed to fail */
    (void) gettimeofday(/* returns */ &tv, NULL);

    osd_ticks_t ret = tv.tv_sec;
    ret *= (1000 * 1000);
    ret += tv.tv_usec;

    return ret;
}


osd_ticks_t osd_ticks_per_second(void)
{
    return (1000 * 1000);
}


void osd_sleep(osd_ticks_t duration)
{
    usleep(duration);
}


void *osd_malloc(size_t size)
{
    return malloc(size);
}


void osd_free(void *ptr)
{
    return free(ptr);
}


void *osd_alloc_executable(size_t size)
{
#ifdef WINDOWS /* XXX need to find a way to make this work on Microsoft
                  Windows without resorting to this ifdef */
	return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
#else
    return mmap(0, size, PROT_EXEC | PROT_READ | PROT_WRITE,
                MAP_ANON | MAP_SHARED, 0, 0);
#endif
}


void osd_free_executable(void *ptr, size_t size)
{
#ifdef WINDOWS /* XXX need to find a way to make this work on Microsoft
                  Windows without resorting to this ifdef */
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
    munmap(ptr, size);
#endif
}


void osd_break_into_debugger(const char *message)
{
// #ifdef MAME_DEBUG
#if 0 // don't bother
    /* Only do this if the environment tells us to */
    if (getenv("MAME_TRAP_ENABLED")) {
        kill(getpid(), SIGTRAP);
    }
#else
    (void) message;
#endif
}


char *osd_get_clipboard_text()
{
    /**
     * This MESS-specific function is not implemented.
     **/
    return NULL;
}


#ifdef MESS

osd_directory_entry *osd_stat(const char *path)
{
    struct stat statbuf;
    
    if (stat(dir, /* returns */ &statbuf))
    {
        return NULL;
    }

    // This deficient API requires that all memory associated with the
	// osd_directory_entry be allocated in one shot
	osd_directory_entry *ret = (osd_directory_entry *) osd_malloc
        (sizeof(osd_directory_entry) + strlen(path) + 1);
	ret->name = ((char *) ret) + sizeof(osd_directory_entry);
	strcpy(ret->name, path);
	ret->type = S_ISDIR(statbuf.st_mode) ? ENTTYPE_DIR : ENTTYPE_FILE;
	ret->size = statbuf.st_size;

	return ret;
}


file_error osd_get_full_path(char **dst, const char *path)
{
    to be implemented;
}


const char *osd_get_volume_name(int idx)
{
    to be implemented;
}


#endif
