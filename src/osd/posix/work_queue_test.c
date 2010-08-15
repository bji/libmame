/** **************************************************************************
 * work_queue_test.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "osdcore.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define A_LONG_TIME (osd_ticks_per_second() * 1000)

typedef struct work_queue_list_element
{
    osd_work_queue *queue;

    struct work_queue_list_element *next;
    struct work_queue_list_element *prev;
} work_queue_list_element;

typedef struct work_item_list_element
{
    osd_work_item *item;
    osd_work_queue *queue;
    struct work_item_list_element *next;
    struct work_item_list_element *prev;
} work_item_list_element;

static work_queue_list_element *g_work_queue_list;
static int g_work_queue_list_count;

static work_item_list_element *g_work_item_list;
static int g_work_item_list_count;

static int g_allparams[100];

static void add_work_queue()
{
    work_queue_list_element *elem = (work_queue_list_element *)
        osd_malloc(sizeof(work_queue_list_element));
    elem->queue = osd_work_queue_alloc(0);
    if (g_work_queue_list) {
        elem->next = g_work_queue_list;
        elem->prev = g_work_queue_list->prev;
        g_work_queue_list->prev->next = elem;
        g_work_queue_list->prev = elem;
    }
    else {
        g_work_queue_list = elem->next = elem->prev = elem;
    }
    g_work_queue_list_count++;
}


static void remove_work_queue(work_queue_list_element *elem)
{
    osd_work_queue_wait(elem->queue, A_LONG_TIME);
    osd_work_queue_free(elem->queue);
    if (elem == g_work_queue_list) {
        if (g_work_queue_list == g_work_queue_list->next) {
            g_work_queue_list = NULL;
        }
        else {
            g_work_queue_list->prev->next = g_work_queue_list->next;
            g_work_queue_list->next->prev = g_work_queue_list->prev;
            g_work_queue_list = g_work_queue_list->next;
        }
    }
    else {
        elem->prev->next = elem->next;
        elem->next->prev = elem->prev;
    }
    osd_free(elem);
    g_work_queue_list_count--;
}


static work_queue_list_element *get_random_queue()
{
    int which = random() % g_work_queue_list_count;
    work_queue_list_element *elem = g_work_queue_list;
    for (int i = 0; i < which; i++) {
        elem = elem->next;
    }
    return elem;
}


static void *work_callback(void *param, int threadid)
{
    int ret = 0;

    for (int i = 0; i < 1000; i++) {
        ret += g_work_item_list_count;
    }

    return (char *) param + ret;
}


static void add_work_item(work_queue_list_element *elem, bool multiple, 
                          bool auto_release)
{
    osd_work_item *item;

    if (multiple) {
        item = osd_work_item_queue_multiple
            (elem->queue, &work_callback, (random() % 1000) + 1,
             (void *) g_allparams, sizeof(int),
             auto_release ? WORK_ITEM_FLAG_AUTO_RELEASE : 0);
    }
    else {
        item = osd_work_item_queue
            (elem->queue, &work_callback, (void *) g_allparams,
             auto_release ? WORK_ITEM_FLAG_AUTO_RELEASE : 0);
    }

    if (!auto_release) {
        work_item_list_element *item_elem = (work_item_list_element *)
            osd_malloc(sizeof(work_item_list_element));
        item_elem->item = item;
        item_elem->queue = elem->queue;
        if (g_work_item_list) {
            item_elem->next = g_work_item_list;
            item_elem->prev = g_work_item_list->prev;
            g_work_item_list->prev->next = item_elem;
            g_work_item_list->prev = item_elem;
        }
        else {
            g_work_item_list = item_elem->next = item_elem->prev = item_elem;
        }
        g_work_item_list_count++;
    }
}


int main(int argc, char **argv)
{
    unsigned int seed;

    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    else {
        seed = time(NULL);
    }

    printf("Seeding random with: %u\n", seed);

    srandom(seed);

    int iteration = 0;

    for (int outer = 1; outer < 500; outer++) {
        g_work_queue_list_count = g_work_item_list_count = 0;
        for (int i = 0; i < 1000; i++) {
            if ((++iteration % 1000) == 999) {
                printf("%d\n", (iteration + 1));
            }
            /* Create a new work queue if there are no work queues, or with a
               5% chance if there are less than 10 work queues */
            if ((g_work_queue_list_count < 10) &&
                ((g_work_queue_list_count == 0) || ((random() % 100) < 5))) {
                add_work_queue();
            }
            /* Remove a work queue with a 5% chance */
            else if ((random() % 100) < 5) {
                remove_work_queue(get_random_queue());
            }
            /* Else create a new work item */
            else {
                work_queue_list_element *elem = get_random_queue();
                long r = random();
                add_work_item(elem, r % 2, (r / 2) % 2);
            }
        }

        /* Wait on remainders */
        while (g_work_queue_list) {
            work_queue_list_element *next = g_work_queue_list->next;
            osd_work_queue_wait(g_work_queue_list->queue, A_LONG_TIME);
            osd_work_queue_free(g_work_queue_list->queue);
            if (g_work_queue_list == next) {
                osd_free(g_work_queue_list);
                g_work_queue_list = NULL;
            }
            else {
                g_work_queue_list->prev->next = g_work_queue_list->next;
                g_work_queue_list->next->prev = g_work_queue_list->prev;
                osd_free(g_work_queue_list);
                g_work_queue_list = next;
            }
        }

        /* Clear out items */
        while (g_work_item_list) {
            work_item_list_element *next = g_work_item_list->next;
            if (g_work_item_list == next) {
                osd_free(g_work_item_list);
                g_work_item_list = NULL;
            }
            else {
                g_work_item_list->prev->next = g_work_item_list->next;
                g_work_item_list->next->prev = g_work_item_list->prev;
                osd_free(g_work_item_list);
                g_work_item_list = next;
            }
        }
    }

    return 0;
}
