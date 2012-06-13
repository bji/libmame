/** **************************************************************************
 * osd_lock.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "osdcore.h"
#include <pthread.h>
#include <stdlib.h>


osd_lock *osd_lock_alloc(void)
{
    pthread_mutexattr_t mutexattr;

    if (pthread_mutexattr_init(&mutexattr) ||
        pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE))
    {
        return NULL;
    }

    pthread_mutex_t *ret = (pthread_mutex_t *) 
        osd_malloc(sizeof(pthread_mutex_t));

    if (ret == NULL)
    {
        return NULL;
    }

    if (pthread_mutex_init(ret, &mutexattr))
    {
        osd_free(ret);
        return NULL;
    }

    return (osd_lock *) ret;
}


void osd_lock_acquire(osd_lock *lock)
{
    if (pthread_mutex_lock((pthread_mutex_t *) lock))
    {
        /* Log an error here? */
        exit(-1);
    }
}


int osd_lock_try(osd_lock *lock)
{
    return (pthread_mutex_trylock((pthread_mutex_t *) lock) ? FALSE : TRUE);
}


void osd_lock_release(osd_lock *lock)
{
    if (pthread_mutex_unlock((pthread_mutex_t *) lock))
    {
        /* Log an error here? */
        exit(-1);
    }
}


void osd_lock_free(osd_lock *lock)
{
    (void) pthread_mutex_destroy((pthread_mutex_t *) lock);
    osd_free(lock);
}
