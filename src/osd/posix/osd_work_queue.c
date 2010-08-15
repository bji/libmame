/** **************************************************************************
 * osd_work_queue.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "osdcore.h"
#include <pthread.h>
#include <unistd.h>


/**
 * A work queue.  All it has is an items_count and a condition variable that
 * is signalled when the items_count goes to 0.  Each work item has a pointer
 * back to its work queue, which is how work queues get updated when items are
 * completed.
 **/
struct _osd_work_queue
{
    int items_count;

    pthread_cond_t cond;
};


/**
 * A work item.  It stores all of the parameters that were passed in when it
 * was created, and keeps track of the number of items that have been run.
 * The thread to run the last item will remove the work_item from the global
 * list, and will, after running the work item, either free it, or signal its
 * condition so that whoever is waiting on it can be notified that it is done,
 * depending on flags.
 **/
struct _osd_work_item
{
    osd_work_callback callback;

    INT32 numitems;

    INT32 numitems_per_worker;
    
    void *parambase;

    INT32 paramstep;

    UINT32 flags;

    void *result;

    int numitemsrun;
    
    int numitemscompleted;

    osd_work_queue *queue;
    
    osd_work_item *prev, *next;
};


/**
 * Work queues are implemented very simply; it's not clear whether or not a
 * sophisticated implementation is necessary.  The mame source current uses
 * work queues pretty sparingly, and until benchmarking is done, there is no
 * way to know if emulation of any game would be made significantly faster by
 * a faster work queue implementation.
 **/

/**
 * General structure:
 * - A pool of threads (num_processors + 1) is shared across all work queues.
 *   This pool of threads is created when the first work queue is created,
 *   and destroyed when the last work queue is destroyed.
 * - There is a single global queue of work items, across all queues; the
 *   pool of work queue threads takes items off of this single queue, and in
 *   this way, all work queues are handled simultaneously.
 * - Work queues themselves are nothing more than thin wrappers around a
 *   counter that is used to track when all items in that work queue have
 *   completed.
 **/

/** **************************************************************************
 * Global variables
 ************************************************************************** **/

/**
 * This is the number of work threads that have been started; when it is zero
 * and a work queue is created, worker threads are started.  When it is time
 * to shut down the worker threads (because there are no more work queues),
 * the count is decremented as each thread exits and when it gets to 0, the
 * g_threads_cond is signalled
 **/
static int g_threads_count;

/**
 * Keep track of the thread ids so as to be able to join them, which avoids
 * pthreads memory leaks in buggy pthreads implementations.
 **/
static pthread_t *g_thread_ids;

/**
 * This boolean is set to false when work threads are starting up, and is not
 * set to true until it is time for all work threads to exit
 **/
static bool g_threads_stop;

/**
 * This is the total number of work queues that are currently in existence;
 * when it goes to zero, the worker threads are shut down.
 **/
static int g_queues_count;

/**
 * This is the list of work items waiting to be handled.  Each work item may
 * need to be run multiple times (with a new parameter each time, as per the
 * numitems parameter).  The thread to run the last parameter in the work item
 * removes the work_item from the global list and either signals a condition,
 * or frees the work_item directly, depending upon the work_item's flags.
 **/
static osd_work_item *g_items;

/**
 * This is a static array of osd_work_item structures, to be used instead of
 * using dynamic memory allocation (unless there are no more available,
 * in which case they are dynamically allocated).
 **/
static osd_work_item g_static_items[10000];

/**
 * This is the list of static items that are freely available
 **/
static osd_work_item *g_free_items;

/**
 * This is set to true after the g_free_items list is initialized; only
 * happens once.
 **/
static bool g_free_items_initialized;

/**
 * This is a global mutex protecting g_threads_count, g_threads_stop,
 * g_queues_count, and g_items.
 **/
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Signalled when g_threads_count goes to 0
 **/
static pthread_cond_t g_threads_cond = PTHREAD_COND_INITIALIZER;

/**
 * Signalled when g_items has a new entry
 **/
static pthread_cond_t g_items_cond = PTHREAD_COND_INITIALIZER;


static void work_item_release_locked(osd_work_item *item)
{
    if ((item >= &(g_static_items[0])) &&
        (item <= &(g_static_items[sizeof(g_static_items) / 
                                  sizeof(g_static_items[0])]))) {
        if (g_free_items == 0) {
            g_free_items = item->next = item->prev = item;
        }
        else {
            item->prev = g_free_items->prev;
            item->next = g_free_items;
            g_free_items->prev->next = item;
            g_free_items->prev = item;
            g_free_items = item;
        }
    }
    else {
        osd_free(item);
    }
}


static void work_queue_run_items_locked(pthread_t self)
{
    /**
     * Save a pointer to the item, as g_items may change while we are working
     * on this item
     **/
    osd_work_item *item = g_items;

    /* Take a block of items to run.  This reduces locking overhead by not
       requiring a lock for every item. */

    /**
     * Save the start index of the items we are to run
     **/
    int index = item->numitemsrun;

    /**
     * Increment the numitemsrun since we are running items.  If we are taking
     * the last item to run out of the item, then no other work thread should
     * see this item any more on the work item list, so remove it
     **/

    int to_run = item->numitems_per_worker;
    int available = item->numitems - item->numitemsrun;
    if (to_run > available) {
        to_run = available;
    }
            
    item->numitemsrun += to_run;

    if (item->numitemsrun == item->numitems)
    {
        if (g_items->next == g_items)
        {
            g_items = NULL;
        }
        else
        {
            g_items->next->prev = g_items->prev;
            g_items->prev->next = g_items->next;
            g_items = g_items->next;
        }
    }

    /**
     * Release the locks; we know that the work item won't be taken out from
     * underneath us because it cannot be freed until the last item is
     * completed, and that can't happen until we're done with it
     **/
    pthread_mutex_unlock(&g_mutex);

    /**
     * Actually run the work item with the appropriate parameters
     **/

    void *result = NULL;
    int end = index + to_run;
    for ( ; index < end; index++)
    {
        result = (item->callback)
            (&(((char *) item->parambase)[index * item->paramstep]),
             self);
    }

    /**
     * Re-acquire the mutex so that we can safely update some fields
     **/
    pthread_mutex_lock(&g_mutex);

    /**
     * If the work item was complete (meaning that we were the work thread
     * that completed the very last work call), then we can handle its end of
     * life stuff
     **/
    item->numitemscompleted += to_run;
    if (item->numitemscompleted == item->numitems)
    {
        /**
         * Get a reference to its queue before releasing the item so that we
         * can update the queue if necessary
         **/
        osd_work_queue *queue = item->queue;
        
        /**
         * If the flag said so, then just immediately auto free it
         **/
        if (item->flags & WORK_ITEM_FLAG_AUTO_RELEASE)
        {
            /**
             * No one else can have a reference to it because the API user
             * never got a reference to it, and we've just completed the last
             * item so no other worker thread could be looking at it.  So it's
             * safe to release it here.
             **/
            work_item_release_locked(item);
        }

        /**
         * Decrement the queue's item count.  If it goes to zero, signal the
         * condition just in case someone is waiting for it to be done.
         **/
        if (--queue->items_count == 0) {
            pthread_cond_signal(&queue->cond);
        }
    }
}


/**
 * Worker thread function
 **/
static void *work_queue_thread_main(void *)
{
    pthread_t self = pthread_self();

    pthread_mutex_lock(&g_mutex);
    
    /**
     * If it's time to stop, then exit this loop.
     **/
    while (!g_threads_stop)
    {
        /**
         * If there are items, then handle the next item
         **/
        if (g_items)
        {
            work_queue_run_items_locked(self);
        }
        /**
         * Else there are currently no items, so wait until g_items_cond is
         * signalled, which happens either when there is a new item, or
         * g_threads_stop is set; either way we will have something new to
         * test at the top of the loop
         **/
        else
        {
            pthread_cond_wait(&g_items_cond, &g_mutex);
        }
    }

    /**
     * If this is the last work thread to exit, signal the g_threads_cond
     **/
    if (--g_threads_count == 0)
    {
        pthread_cond_signal(&g_threads_cond);
    }

    pthread_mutex_unlock(&g_mutex);

    return 0;
}


/**
 * This is called whenever it is time to destroy all of the work threads that
 * have been created
 **/
static void work_queue_destroy_threads_locked()
{
    /**
     * Set the flag that will tell all work threads to exit
     **/
    g_threads_stop = true;

    /**
     * Broadcast on the g_items_cond, so that any thread which is waiting for
     * an item will wake up and go back to the top of its loop and see that
     * the g_threads_stop flag is now set and will thus exit
     **/
    pthread_cond_broadcast(&g_items_cond);
    
    int threads_count = g_threads_count;

    /**
     * Wait util all of the threads have exited; because each decrements the
     * g_threads_count, and because the last one that decrements the count to
     * 0 signals the condition, waiting on the condition until g_threads_count
     * is zero is sufficient
     **/
    while (g_threads_count)
    {
        pthread_cond_wait(&g_threads_cond, &g_mutex);
    }

    /**
     * And, join them all to prevent pthreads memory leaks in buggy pthreads
     * implementations
     **/
    for (int i = 0; i < threads_count; i++) {
        void *dontcare;
        (void) pthread_join(g_thread_ids[i], &dontcare);
    }

    osd_free(g_thread_ids);

    g_thread_ids = NULL;
}


/**
 * This method is called to create the work threads when the first work queue
 * is created.
 **/
static int work_queue_create_threads_locked()
{
    int threads_count;

    /**
     * The number of threads to run is determined to be the number of
     * processors in the system plus one.  This is a guess as to how to most
     * effectively keep the processors busy.  I/O bound work queues may
     * benefit from more threads, and purely processor-bound work queues only
     * need one thread per CPU, but this compromise will likely work well in
     * most cases.
     **/

    /**
     * Allow a preprocessor symbol named OSDLIB_WORK_QUEUE_THREAD_COUNT to fix
     * the number of threads to start.  This may be useful for testing or for
     * the rare cases where the normal processor counting logic is not
     * sufficient.
     **/
#ifdef OSDLIB_WORK_QUEUE_THREAD_COUNT
    threads_count = OSDLIB_WORK_QUEUE_THREAD_COUNT;
#else
    /**
     * If that's not defined, then if _SC_NPROCESSORS_ONLN is defined, it
     * means that the POSIX sysconf call to get the number of processors is
     * available, so use it; but subtract one because empirical evidence shows
     * that this is best.
     **/
#ifdef _SC_NPROCESSORS_ONLN
    threads_count = sysconf(_SC_NPROCESSORS_ONLN) - 1;
#else
    /**
     * Else assume a really ancient system without proper POSIX support, and
     * only give it one thread on the assumption that it's probably only got
     * one processor (more modern systems are expected to be multi-processor
     * and to support recent POSIX).
    **/
    threads_count = 1;
#endif
#endif

    if (threads_count < 1) {
        threads_count = 1;
    }

    /**
     * This variable will be set if the creation of any of the work threads
     * fails, and will provoke a cleanup operation
     **/
    int ret = 0;

    /**
     * Be sure to initialize this global variable so that the threads that
     * start do not think that they need to exit
     **/
    g_threads_stop = false;

    /**
     * Create the array to hold thread ids
     **/
    g_thread_ids = (pthread_t *) osd_malloc(sizeof(pthread_t) * threads_count);

    /**
     * Create threads_count work threads
     **/
    for (int i = 0; i < threads_count; i++)
    {
        pthread_t newthread;
        if (pthread_create(&newthread, NULL, &work_queue_thread_main, NULL))
        {
            /**
             * On failure, set the flag, and stop trying to create new threads
             **/
            ret = 1;
            break;
        }
        else
        {
            /* Store the newly-created thread's ID in the list */
            g_thread_ids[i] = newthread;
            
            /**
             * On success, increment the number of threads that have been
             * created.  Note that the newly created thread will not do
             * anything because we were called with g_mutex locked, and the
             * work threads immediately try to acquire that lock and thus
             * immediately wait until we are done
             **/
            g_threads_count += 1;
        }
    }

    /**
     * If there was a failure, destroy all of the newly created threads
     **/
    if (ret)
    {
        work_queue_destroy_threads_locked();
    }

    return ret;
}


osd_work_queue *osd_work_queue_alloc(int flags)
{
    osd_work_queue *queue = (osd_work_queue *) 
        osd_malloc(sizeof(osd_work_queue));

    if (queue == NULL)
    {
        return NULL;
    }

    if (pthread_cond_init(&(queue->cond), NULL))
    {
        osd_free(queue);
        return NULL;
    }

    queue->items_count = 0;

    /* Take this opportunity to create the work threads if they need to be
       created */
    pthread_mutex_lock(&g_mutex);

    if (g_queues_count == 0)
    {
        if (work_queue_create_threads_locked())
        {
            pthread_mutex_unlock(&g_mutex);
            pthread_cond_destroy(&(queue->cond));
            osd_free(queue);
            return NULL;
        }
    }

    g_queues_count += 1;

    if (!g_free_items_initialized) {
        for (int i = 0; 
             i < (sizeof(g_static_items) / sizeof(g_static_items[0])); i++) {
            osd_work_item *item = &(g_static_items[i]);
            if (g_free_items) {
                item->prev = g_free_items->prev;
                item->next = g_free_items;
                g_free_items->prev->next = item;
                g_free_items->prev = item;
            }
            else {
                g_free_items = item->prev = item->next = item;
            }
        }
        g_free_items_initialized = true;
    }

    pthread_mutex_unlock(&g_mutex);
    
    return queue;
}


int osd_work_queue_items(osd_work_queue *queue)
{
    return queue->items_count;
}


int osd_work_queue_wait(osd_work_queue *queue, osd_ticks_t timeout)
{
    pthread_t self = pthread_self();

    /**
     * Calculate what the end tick is (if we get to this tick, we've timed
     * out).
     **/
    osd_ticks_t end = osd_ticks() + timeout;

    pthread_mutex_lock(&g_mutex);

    /**
     * Wait until the queue has no more items.
     **/
    while (queue->items_count)
    {
        /**
         * We allow special behavior here: if timeout is 0, we wait forever.
         **/
        if ((timeout != 0) && (osd_ticks() >= end))
        {
            /**
             * We would like to wait for the queue to become empty, but we're
             * already past the end tick, so time out
             **/
            pthread_mutex_unlock(&g_mutex);
            return FALSE;
        }

        /**
         * If there are items to run, then run them, to try to complete the
         * items
         **/
        if (g_items) {
            work_queue_run_items_locked(self);
        }
        /**
         * Else, some other thread must be completing the queue on our
         * behalf.  Wait for them to finish.
         **/
        else {
            pthread_cond_wait(&queue->cond, &g_mutex);
        }
    }

    /**
     * If we got here, then the queue is now empty so we can return
     **/

    pthread_mutex_unlock(&g_mutex);

    return TRUE;
}


void osd_work_queue_free(osd_work_queue *queue)
{
    /**
     * Wait until the work queue is empty.  We use a special timeout value,
     * that we know means 'wait forever'.
     **/
    osd_work_queue_wait(queue, 0);

    /**
     * Destroy the queue
     **/
    osd_free(queue);

    /**
     * Now decrement the queue count, and if it has gone to zero, destroy the
     * work threads as there are no more work queues that need to be serviced
     **/
    pthread_mutex_lock(&g_mutex);

    if (--g_queues_count == 0)
    {
        work_queue_destroy_threads_locked();
    }

    pthread_mutex_unlock(&g_mutex);
}


osd_work_item *osd_work_item_queue_multiple(osd_work_queue *queue,
                                            osd_work_callback callback,
                                            INT32 numitems, void *parambase,
                                            INT32 paramstep, UINT32 flags)
{
    osd_work_item *item;

    pthread_mutex_lock(&g_mutex);

    if (g_free_items) {
        item = g_free_items;
        if (item->next == item) {
            g_free_items = 0;
        }
        else {
            item->prev->next = item->next;
            item->next->prev = item->prev;
            g_free_items = item->next;
        }
    }
    else {
        item = (osd_work_item *) osd_malloc(sizeof(osd_work_item));

        if (item == NULL)
        {
            return NULL;
        }
    }

    item->callback = callback;

    item->numitems = numitems;

    item->numitems_per_worker = 
        (numitems / (g_threads_count ? g_threads_count : 1)) + 1;

    item->parambase = parambase;

    item->paramstep = paramstep;

    item->flags = flags;

    item->numitemsrun = 0;

    item->numitemscompleted = 0;

    item->queue = queue;
    
    queue->items_count += 1;

    if (g_items)
    {
        item->prev = g_items->prev;
        item->next = g_items;
        g_items->prev->next = item;
        g_items->prev = item;
    }
    else
    {
        g_items = item->prev = item->next = item;
    }

    pthread_cond_broadcast(&g_items_cond);
    
    pthread_mutex_unlock(&g_mutex);

    /* Enforce that the caller is not allowed to reference this item when they
       say to auto release it but returning NULL in that case */
    return (flags & WORK_ITEM_FLAG_AUTO_RELEASE) ? NULL : item;
}


int osd_work_item_wait(osd_work_item *item, osd_ticks_t timeout)
{
    /**
     * Just wait on the queue that this item belongs to.  This is done because
     * MAME actually doesn't really use this method and this is the simplest
     * way to implement it.  Providing for an implementation that can wait for
     * this specific item to be finished is more heavyweight and hurts the
     * performance of the rest of the work queue implementation.
     **/
    return osd_work_queue_wait(item->queue, timeout);
}


void *osd_work_item_result(osd_work_item *item)
{
    return item->result;
}


void osd_work_item_release(osd_work_item *item)
{
    pthread_mutex_lock(&g_mutex);

    work_item_release_locked(item);

    pthread_mutex_unlock(&g_mutex);
}
