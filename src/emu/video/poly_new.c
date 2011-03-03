/***************************************************************************

    poly.c

    Helper routines for polygon rendering.

***************************************************************************/

#include "emu.h"
#include "poly.h"

#include <stdint.h>

#define NEW_IMPLEMENTATION 1

#if NEW_IMPLEMENTATION

#undef delete


/**
 * Should spinlocks or regular locks be used in the Queue implementation?
 **/
#define QUEUE_USE_SPIN_LOCKS 0

/**
 * Must be at least 32
 **/
#define MAX_VERTICES 32

/**
 * Maximum number of extents
 **/
#define MAX_EXTENTS 64

/**
 * Number of scanlines per bucket
 **/
#define SCANLINES_PER_BUCKET 8

/**
 * Maximum total number of scanlines across all buckets.
 **/
#define TOTAL_SCANLINE_COUNT 512

/**
 * Derived constant - total number of buckets
 **/
#define BUCKET_COUNT ((TOTAL_SCANLINE_COUNT / SCANLINES_PER_BUCKET) +   \
                      ((TOTAL_SCANLINE_COUNT % SCANLINES_PER_BUCKET) ? 1 : 0))


enum RenderMode
{
    RenderModeTriangles,
    RenderModeQuads,
    RenderModePolygon,
    RenderModeExtents
};


// --------------------------------------------------------------------------
// Forward declarations
// --------------------------------------------------------------------------
class Renderer;


// --------------------------------------------------------------------------
// Queue
// --------------------------------------------------------------------------

// Optionally use spinlocks, which may have better latency due to not
// requiring operating system thread scheduling operations; although in
// practice spinlocks seem to have identical performance to mutexes at least
// with pthreads on Linux (maybe Linux pthreads implements spinlocks as
// mutexes?).
#if QUEUE_USE_SPIN_LOCKS
#define QUEUE_LOCK osd_spinlock
#define QUEUE_LOCK_ALLOC osd_spinlock_alloc
#define QUEUE_LOCK_FREE osd_spinlock_free
#define QUEUE_LOCK_ACQUIRE osd_spinlock_acquire
#define QUEUE_LOCK_RELEASE osd_spinlock_release
#else
#define QUEUE_LOCK osd_lock
#define QUEUE_LOCK_ALLOC osd_lock_alloc
#define QUEUE_LOCK_FREE osd_lock_free
#define QUEUE_LOCK_ACQUIRE osd_lock_acquire
#define QUEUE_LOCK_RELEASE osd_lock_release
#endif

/**
 * Work queue.  This class is used to pass work items from the thread which
 * has the work parameters off to another thread to complete.  Its methods are
 * called extremely frequently and should be as efficient as possible, both in
 * terms of number of instructions executed, and in effects on cache locking
 * and thread scheduling.
 *
 * D is the work function parameter type
 * F is the work function
 **/
template<typename D, void F(D &, int)>
class WorkQueue
{
public:

    WorkQueue(osd_work_queue *osdWorkQueue, int size)
        : osdWorkQueueM(osdWorkQueue), workersM(0), sizeM(size), headM(0),
          tailM(0)
    {
        lockM = QUEUE_LOCK_ALLOC();
        elementsM = new D[size];
    }

    bool IsValid() const
    {
        return lockM && elementsM;
    }

    ~WorkQueue()
    {
        delete[] elementsM;
        QUEUE_LOCK_FREE(lockM);
    }

    /**
     * Schedules a call to the F template function with the given D parameter
     * to occur on a separate thread.  Returns true if the call was
     * successfully scheduled, false if it could not be scheduled due to the
     * queue being full.
     **/
    bool ScheduleWorkElement(const D &d)
    {
        QUEUE_LOCK_ACQUIRE(lockM);
        // If queue is full, bail out
        if (Increment(headM) == tailM) {
            QUEUE_LOCK_RELEASE(lockM);
            return false;
        }
        // New one on the queue
        elementsM[headM] = d;
        // If queue was empty before put, then must schedule the work callback
        if (headM == tailM) {
            headM = Increment(headM);
            workersM = 1;
            QUEUE_LOCK_RELEASE(lockM);
#if 1
#if 0
            printf("Scheduling work element on %d\n", d->polygonNumber);
#endif
            (void) osd_work_item_queue(osdWorkQueueM, WorkQueueCb, this,
                                       WORK_ITEM_FLAG_AUTO_RELEASE);
#else
            RunWorkElements(0);
#endif
        }
        // Else queue was not empty and the work callback will keep working on
        // it
        else {
            headM = Increment(headM);
            QUEUE_LOCK_RELEASE(lockM);
        }
        return true;
    }

private:

    uint16_t Increment(uint16_t value)
    {
        return ((value + 1) % sizeM);
    }

    static void *WorkQueueCb(void *data, int threadid)
    {
        (void) threadid;

        ((WorkQueue *) data)->RunWorkElements(threadid);

        return 0;
    }

    void RunWorkElements(int threadid)
    {
        QUEUE_LOCK_ACQUIRE(lockM);
        workersM += 1;
        do {
            // If the queue is empty then break out straight away
            if (headM == tailM) {
                workersM -= 1;
                QUEUE_LOCK_RELEASE(lockM);
                break;
            }
            D *next = &(elementsM[tailM]);
            tailM = Increment(tailM);
            bool hasMore;
            if (headM == tailM) {
                // When the queue is emptied, reset back to the beginning;
                // this will help with cache utilization, in that elements at
                // the front of the queue will be used more often instead of
                // all elements being used equally
                headM = tailM = 0;
                hasMore = false;
            }
            else {
                if (workersM < 16) {
                    workersM += 1;
                }
                hasMore = true;
            }
            int workers = workersM;
            QUEUE_LOCK_RELEASE(lockM);
            // If there is more to be processed after this one, then schedule
            // another work proc to do it so that another thread may work on
            // it in parallel while we work on this one.
            if (hasMore && (workers < 16)) {
#if 0
                printf("Scheduling another work element on %d (%d)\n", 
                       (*next)->polygonNumber, workers);
#endif
                /* Queue another thread to run on this work queue while this
                   thread makes the work callback */
                (void) osd_work_item_queue(osdWorkQueueM, WorkQueueCb, this,
                                           WORK_ITEM_FLAG_AUTO_RELEASE);
            }
            F(*next, threadid);
            // If there was no more to process, then break out
            if (!hasMore) {
                QUEUE_LOCK_ACQUIRE(lockM);
                workersM -= 1;
                QUEUE_LOCK_RELEASE(lockM);
                break;
            }
            QUEUE_LOCK_ACQUIRE(lockM);
        } while (true);
    }

    osd_work_queue *osdWorkQueueM;

    int workersM;

    int sizeM;

    uint16_t headM, tailM;

    QUEUE_LOCK *lockM;

    D *elementsM;
};


/**
 * A NumberedWorkQueue is a work queue where items will always be processed in
 * a specific order but may be added to the queue out-of-order.
 *
 * This *might* end up being used if I decide to try to allow the setup work
 * calls to be done in parallel.  If so, right before setup work callback
 * is called, that thread will schedule another work proc for the setup
 * stuff.  And the completion of each setup work will have the PolygonData
 * put onto a NumberedQueue with the index being the polygon number.
 * Finally, there will be a callback for the NumberedQueue that will
 * do the Bucketing only.
 * In this way, the scanline setup will be done multithreaded, with only the
 * Bucketing done sequentially in a single thread - instead of the current
 * implementation with just a single work queue, where both the scanline
 * setup and bucketing is done in sequence and single-threaded.
 **/
template<typename D, void F(int, D *, int)>
class NumberedWorkQueue
{
public:

    /**
     * size is the maximum number of elements that can be put into the queue.
     * Exceeding this number will crash the queue.
     **/
    NumberedWorkQueue(osd_work_queue *osdWorkQueue, int size)
        : osdWorkQueueM(osdWorkQueue), sizeM(size), nextWorkM(0),
          nextScheduleM(0)
    {
        lockM = QUEUE_LOCK_ALLOC();
        readyM = new bool[size];
        skippedM = new bool[size];
        memset(readyM, 0, sizeof(bool) * size);
        memset(skippedM, 0, sizeof(bool) * size);
        elementsM = new D[size];
    }

    /**
     * If this returns false, the newly-constructed NumberedQueue could not
     * allocate all required memory and should not be used.
     **/
    bool IsValid() const
    {
        return (lockM && readyM && elementsM);
    }

    ~NumberedWorkQueue()
    {
        delete[] elementsM;
        delete[] skippedM;
        delete[] readyM;
        QUEUE_LOCK_FREE(lockM);
    }
    
    /**
     * Not thread-safe with any other method here.  Reset only when the work
     * queue is empty or else work will be lost.  An element at a given index
     * cannot be added twice to the same NumberedWorkQueue without Reset()
     * being called in between.
     **/
    void Reset()
    {
        /* Just for memory barrier paranoia */
        QUEUE_LOCK_ACQUIRE(lockM);
        // XXX figure out how to do this more efficiently
        memset(readyM, 0, sizeof(bool) * sizeM);
        memset(skippedM, 0, sizeof(bool) * sizeM);
        nextWorkM = nextScheduleM = 0;
        QUEUE_LOCK_RELEASE(lockM);
    }

    /**
     * Schedules the template function F to be called in a separate thread
     * but only after all work elements scheduled at lower indexes have
     * been called.
     **/
    void ScheduleWorkElement(int index, const D &d)
    {
        this->ScheduleWorkElement(index, &d, false);
    }

    /**
     * Lets the numbered work queue know that there is no work to do for
     * a given element
     **/
    void SkipWorkElement(int index)
    {
        this->ScheduleWorkElement(index, 0, true);
    }

private:

    void ScheduleWorkElement(int index, const D *d, bool skip)
    {
        QUEUE_LOCK_ACQUIRE(lockM);
        readyM[index] = true;
        if (skip) {
            skippedM[index] = true;
        }
        else {
            elementsM[index] = *d;
        }
        // If the item that is scheduled is the next work item to be called,
        // then there may be work to do
        if (index == nextScheduleM) {
            // But first, skip anything that needs to be skipped
            if (skip) {
                nextScheduleM += 1;
                while ((nextScheduleM < sizeM) && readyM[nextScheduleM] &&
                       skippedM[nextScheduleM]) {
                    nextScheduleM += 1;
                }
            }
            if ((nextScheduleM < sizeM) && readyM[nextScheduleM]) {
                QUEUE_LOCK_RELEASE(lockM);
                (void) osd_work_item_queue(osdWorkQueueM, NumberedWorkQueueCb,
                                           this, WORK_ITEM_FLAG_AUTO_RELEASE);
                return;
            }
        }
        // Else, there were already elements available, so we will definitely
        // not schedule work
        QUEUE_LOCK_RELEASE(lockM);
    }

    static void *NumberedWorkQueueCb(void *data, int threadid)
    {
        ((NumberedWorkQueue *) data)->RunWorkElements(threadid);

        return 0;
    }

    void RunWorkElements(int threadid)
    {
        QUEUE_LOCK_ACQUIRE(lockM);
        while ((nextWorkM < sizeM) && readyM[nextWorkM]) {
            // Find the first and last one to run
            int nextWork = nextWorkM;
            int lastWork = nextWork + 1;
            while ((lastWork < sizeM) && readyM[lastWork]) {
                lastWork += 1;
            }
            QUEUE_LOCK_RELEASE(lockM);
            // Run the elements
            do {
                int firstWork = nextWork++;
                bool skip = skippedM[firstWork];
                while ((nextWork < lastWork) && (skippedM[nextWork] == skip)) {
                    nextWork += 1;
                }
                if (!skip) {
                    F(nextWork - firstWork, &(elementsM[firstWork]), threadid);
                }
            } while (nextWork < lastWork);
            QUEUE_LOCK_ACQUIRE(lockM);
            nextWorkM = lastWork;
        }
        nextScheduleM = nextWorkM;
        QUEUE_LOCK_RELEASE(lockM);
    }

    osd_work_queue *osdWorkQueueM;

    int sizeM;

    int nextWorkM, nextScheduleM;

    QUEUE_LOCK *lockM;
    
    bool *readyM;

    bool *skippedM;

    D *elementsM;
};


// --------------------------------------------------------------------------
// Pool
// --------------------------------------------------------------------------

template<typename T>
class Pool
{
public:

    Pool(int count)
        : countM(count), nextM(0)
    {
        tsM = new T[count];
    }

    bool IsValid() const
    {
        return tsM;
    }

    ~Pool()
    {
        delete[] tsM;
    }

    T *Acquire()
    {
        return (nextM == countM) ? 0 : &(tsM[nextM++]);
    }

    void ReleaseAll()
    {
        nextM = 0;
    }

private:

    int countM;
    int nextM;
    T *tsM;
};


// --------------------------------------------------------------------------
// SharedPool
// --------------------------------------------------------------------------

template<typename T, int blockSizeC>
class SharedPool
{
public:

    SharedPool(int initialCount)
    {
        lockM = QUEUE_LOCK_ALLOC();
        fixedBlockM.nextT = 0;
        fixedBlockM.nextBlock = 0;
        currentBlockM = &fixedBlockM;
    }

    bool IsValid() const
    {
        return lockM;
    }

    ~SharedPool()
    {
        this->ReleaseAll();
        QUEUE_LOCK_FREE(lockM);
    }

    /* count must be <= blockSizeC */
    T *Acquire(int count)
    {
        QUEUE_LOCK_ACQUIRE(lockM);
        if ((currentBlockM->nextT + count) > blockSizeC) {
            printf("Acquire\n");
            Block *newBlock = (Block *) osd_malloc(sizeof(Block));
            if (!newBlock) {
                QUEUE_LOCK_RELEASE(lockM);
                return 0;
            }
            newBlock->nextT = 0;
            currentBlockM->nextBlock = newBlock;
            currentBlockM = newBlock;
        }
        T *ret = &(currentBlockM->ts[currentBlockM->nextT]);
        currentBlockM->nextT += count;
        QUEUE_LOCK_RELEASE(lockM);
        return ret;
    }

    void ReleaseAll()
    {
        /* paranoia */
        QUEUE_LOCK_ACQUIRE(lockM);
        Block *thisBlock = fixedBlockM.nextBlock;
        while (thisBlock) {
            Block *nextBlock = thisBlock->nextBlock;
            printf("Release\n");
            osd_free(thisBlock);
            thisBlock = nextBlock;
        }
        currentBlockM = &fixedBlockM;
        fixedBlockM.nextT = 0;
        fixedBlockM.nextBlock = 0;
        QUEUE_LOCK_RELEASE(lockM);
    }

private:

    typedef struct Block
    {
        T ts[blockSizeC];
        int nextT;
        struct Block *nextBlock;
    } Block;

    QUEUE_LOCK *lockM;
    Block fixedBlockM;
    Block *currentBlockM;
};


// --------------------------------------------------------------------------
// PolygonData
// --------------------------------------------------------------------------

/**
 * Each frame, each polygon that is rendered gets one of these; it tracks all
 * data necessary to render the polygon.
 **/
class PolygonData
{
public:

    Renderer *renderer;
    
    int polygonNumber;

    void *extraData;

    void *dest;

    rectangle cliprect_saved, *cliprect;
    
    poly_draw_scanline_func callback;

    int paramcount;
    
    RenderMode renderMode;
    
    union {
        struct {
            int vertex_count;
            poly_vertex v[MAX_VERTICES];
        };

        struct {
            int startscanline;
            int extent_count;
            poly_extent e[MAX_EXTENTS];
        };
    };
};


// --------------------------------------------------------------------------
// BucketItem
// --------------------------------------------------------------------------

/**
 * Data associated with each scanline bucket for each polygon
 **/
class BucketItem
{
public:

    PolygonData *pd;

    int bucketNumber; /* TEMPORARY TESTING */

    INT32 scanlineFirst, scanlineCount;
    
    poly_extent extents[SCANLINES_PER_BUCKET];
};


// --------------------------------------------------------------------------
// Renderer
// --------------------------------------------------------------------------

/**
 * This class holds all of the callback data and work queues
 **/
class Renderer
{
public:

    Renderer(int maxPolys, int extraDataSize, bool includeRightEdge,
             bool includeBottomEdge)
        : maxPolysM(maxPolys), rightEdgeExtraPixelsM(includeRightEdge ? 1 : 0),
          bottomEdgeExtraPixelsM(includeBottomEdge ? 1 : 0),
          polygonDataPoolM(maxPolys), bucketItemPoolM(maxPolys * 4),
          extraDataSizeM(extraDataSize), extraDataNextM(0),
          currentExtraDataM(0), 
          osdWorkQueueM(osd_work_queue_alloc(WORK_QUEUE_FLAG_MULTI | 
                                             WORK_QUEUE_FLAG_HIGH_FREQ)),
          setupWorkQueueM(osdWorkQueueM, maxPolys)
    {
        extraDataM = (void **) osd_malloc(sizeof(void *) * maxPolys);
        if (extraDataM) {
            for (int i = 0; i < maxPolys; i++) {
                extraDataM[i] = osd_malloc(extraDataSize);
                if (!extraDataM[i]) {
                    extraDataNextM = maxPolysM;
                }
            }
        }
        else {
            extraDataNextM = maxPolysM;
        }
        
        for (int i = 0; i < BUCKET_COUNT; i++) {
            bucketWorkQueuesM[i] = 
                new NumberedWorkQueue<BucketItem *, BucketWorkQueueCb>
                (osdWorkQueueM, maxPolys);
            if (!bucketWorkQueuesM[i]) {
                extraDataNextM = maxPolysM;
            }
        }
    }

    bool IsValid()
    {
        return (polygonDataPoolM.IsValid() && (extraDataNextM < maxPolysM) && 
                setupWorkQueueM.IsValid() && osdWorkQueueM);
    }

    ~Renderer()
    {
        for (int i = 0; i < BUCKET_COUNT; i++) {
            delete bucketWorkQueuesM[i];
        }
        if (extraDataM) {
            for (int i = 0; i < maxPolysM; i++) {
                if (extraDataM[i]) {
                    osd_free(extraDataM[i]);
                }
            }
            osd_free(extraDataM);
        }
        if (osdWorkQueueM) {
            osd_work_queue_free(osdWorkQueueM);
        }
    }

    /**
     * Wait until all polygons have rendered and reset to be ready for more
     * polygons.
     **/
    void WaitAndReset()
    {
        /* Wait for all pending work items to complete */

        osd_work_queue_wait(osdWorkQueueM, (UINT32) -1);

        /* Reset frame state */

        /* Because the 'extra' data is supposed to be supplied for each
           subsequent polygon, must preserve that of the last call
           poly_get_extra_data, since it must continue to be passed in */
        if (extraDataNextM > 1) {
            memcpy(extraDataM[0], extraDataM[extraDataNextM - 1], 
                   extraDataSizeM);
            extraDataNextM = 1;
            currentExtraDataM = extraDataM;
        }

        /* Return all callback datas */
        polygonDataPoolM.ReleaseAll();
        bucketItemPoolM.ReleaseAll();

        nextPolygonNumberM = 0;

        /* Reset all of the bucket work queues */
        for (int i = 0; i < BUCKET_COUNT; i++) {
            bucketWorkQueuesM[i]->Reset();
        }
    }

    /**
     * Get the next extra data to associate with polygons on this frame
     **/
    void *GetExtraData()
    {
        currentExtraDataM = extraDataM[extraDataNextM++];
        return currentExtraDataM;
    }
    
    UINT32 RenderTriangle(bool needPixelCount, void *dest,
                          const rectangle *cliprect,
                          poly_draw_scanline_func callback, int paramcount,
                          const poly_vertex *v1, const poly_vertex *v2,
                          const poly_vertex *v3)
    {
        PolygonData *pd = this->GetPolygonData
            (dest, cliprect, callback, paramcount);

        pd->vertex_count = 3;
        pd->v[0] = *v1;
        pd->v[1] = *v2;
        pd->v[2] = *v3;

        return this->Render(needPixelCount, RenderModeTriangles, pd);
    }

    UINT32 RenderTriangleFan(bool needPixelCount, void *dest,
                             const rectangle *cliprect,
                             poly_draw_scanline_func callback, int paramcount,
                             int numverts, const poly_vertex *v)
    {
        UINT32 total = 0;
        PolygonData *pd = this->GetPolygonData
            (dest, cliprect, callback, paramcount);

        for (int i = 2; i < numverts; i++) {
            pd->vertex_count = 3;
            pd->v[0] = v[0];
            pd->v[1] = v[i - 1];
            pd->v[2] = v[i];
            total += this->Render(needPixelCount, RenderModeTriangles, pd);
        }

        return total;
    }

    UINT32 RenderQuad(bool needPixelCount, void *dest,
                      const rectangle *cliprect,
                      poly_draw_scanline_func callback, int paramcount,
                      const poly_vertex *v1, const poly_vertex *v2,
                      const poly_vertex *v3, const poly_vertex *v4)
    {
        PolygonData *pd = this->GetPolygonData
            (dest, cliprect, callback, paramcount);

        pd->vertex_count = 4;
        pd->v[0] = *v1;
        pd->v[1] = *v2;
        pd->v[2] = *v3;
        pd->v[3] = *v4;

        return this->Render(needPixelCount, RenderModeQuads, pd);
    }

    UINT32 RenderQuadFan(bool needPixelCount, void *dest,
                         const rectangle *cliprect,
                         poly_draw_scanline_func callback, int paramcount,
                         int numverts, const poly_vertex *v)
    {
        UINT32 total = 0;
        PolygonData *pd = this->GetPolygonData
            (dest, cliprect, callback, paramcount);

        for (int i = 2; i < numverts; i++) {
            if ((pd->vertex_count + 4) >= MAX_VERTICES) {
                total += this->Render(needPixelCount, RenderModeQuads, pd);
                pd = this->GetPolygonData
                    (dest, cliprect, callback, paramcount);
            }
            pd->v[pd->vertex_count++] = v[0];
            pd->v[pd->vertex_count++] = v[i - 1];
            pd->v[pd->vertex_count++] = v[i];
            pd->v[pd->vertex_count++] = v[MIN(i + 1, i - 1)];
        }
        
        return total + this->Render(needPixelCount, RenderModeQuads, pd);
    }

    UINT32 RenderPolygon(bool needPixelCount, void *dest,
                         const rectangle *cliprect,
                         poly_draw_scanline_func callback, int paramcount,
                         int numverts, const poly_vertex *v)
    {
        PolygonData *pd = this->GetPolygonData
            (dest, cliprect, callback, paramcount);

        pd->vertex_count = numverts;
        memcpy(pd->v, v, sizeof(poly_vertex) * numverts);

        return this->Render(needPixelCount, RenderModePolygon, pd);
    }

    UINT32 RenderExtents(bool needPixelCount, void *dest,
                         const rectangle *cliprect,
                         poly_draw_scanline_func callback, 
                         int startscanline, int numscanlines,
                         const poly_extent *e)
    {
        UINT32 total = 0;

        while (numscanlines) {
            PolygonData *pd = this->GetPolygonData
                (dest, cliprect, callback, 0);
            int todo = MIN(numscanlines, MAX_EXTENTS);
            pd->startscanline = startscanline;
            pd->extent_count = numscanlines;
            memcpy(pd->e, e, sizeof(poly_extent) * todo);
            total += this->Render(needPixelCount, RenderModeExtents, pd);
            startscanline += todo;
            numscanlines -= todo;
            e = &(e[todo]);
        }

        return total;
    }

private:

    inline INT32 round_coordinate(float value)
    {
        INT32 result = floor(value);
        return result + ((value - ((float) result)) > 0.5f);
    }

    PolygonData *GetPolygonData(void *dest, const rectangle *cliprect,
                                  poly_draw_scanline_func callback,
                                  int paramcount)
    {
        PolygonData *pd = polygonDataPoolM.Acquire();
        if (!pd) {
            this->WaitAndReset();
            pd = polygonDataPoolM.Acquire();
        }
        
        pd->polygonNumber = nextPolygonNumberM++;
        pd->renderer = this;
        pd->extraData = extraDataNextM ? extraDataM[extraDataNextM - 1] : 0;
        pd->dest = dest;
        if (cliprect) {
            pd->cliprect_saved = *cliprect;
            pd->cliprect = &(pd->cliprect_saved);
        }
        else {
            pd->cliprect = 0;
        }
        pd->callback = callback;
        pd->paramcount = paramcount;
        
        return pd;
    }

    UINT32 Render(bool needPixelCount, RenderMode renderMode, PolygonData *pd)
    {
        pd->renderMode = renderMode;

        // If pixel count is needed, then must do all scanline setup in this
        // thread and afterwards, count it all
        if (needPixelCount) {
            this->Setup(pd);

            return CountPixels(pd);
        }
        // Else schedule the render
        else {
#if 0
            printf("Scheduling polygon %d\n", pd->polygonNumber);
#endif
            while (!setupWorkQueueM.ScheduleWorkElement(pd)) {
                printf("Waiting for queue\n");
                /* Possibly wait a little bit */
            }

            return 0;
        }
    }

    static UINT32 CountPixels(const PolygonData *pd)
    {
        // XXX TODO
        (void) pd;

        return 0;
    }

    static void SetupWorkQueueCb(PolygonData *&pd, int threadid)
    {
        (void) threadid;

        // XXX all of the Setup methods currently need to do the setup
        // and bucketizing, and ought to do it in any order that the
        // threading allows.  It is the bucket work queues that are numbered
        // and that prevent out-of-order rendering.
        pd->renderer->Setup(pd);
    }

    static void BucketWorkQueueCb(int count, BucketItem **bi, int threadid)
    {
        (*bi)->pd->renderer->Render(count, bi, threadid);
    }

    void Setup(PolygonData *pd)
    {
        switch (pd->renderMode) {
        case RenderModeTriangles:
            this->SetupTriangles(pd);
            break;
        case RenderModeQuads:
            this->SetupQuads(pd);
            break;
        case RenderModePolygon:
            this->SetupPolygon(pd);
            break;
        default: // case RenderModeExtents:
            this->SetupExtents(pd);
            break;
        }
    }

    void Render(int count, BucketItem **bis, int threadid)
    {
        for (int i = 0; i < count; i++) {
            BucketItem *bi = bis[i];
            PolygonData *pd = bi->pd;
            for (int j = 0; j < bi->scanlineCount; j++) {
                /* TEMPORARY use polygon number instead of threadid */
#if 0
                (pd->callback)(pd->dest, bi->scanlineFirst + j, 
                               &(bi->extents[j]), pd->extraData, threadid);
#else
                (pd->callback)(pd->dest, bi->scanlineFirst + j, 
                               &(bi->extents[j]), pd->extraData, 
                               pd->polygonNumber);
#endif
            }
        }
    }

    void SetupTriangles(PolygonData *pd)
    {
#if 0
        printf("Setting up polygon %d\n", pd->polygonNumber);
#endif
        const poly_vertex *v1, *v2, *v3;

        // PolygonData has three vertices; sort them by y into v1, v2, and v3

        if (pd->v[0].y < pd->v[1].y) {
            if (pd->v[1].y < pd->v[2].y) {
                v1 = &(pd->v[0]);
                v2 = &(pd->v[1]);
                v3 = &(pd->v[2]);
            }
            else if (pd->v[0].y < pd->v[2].y) {
                v1 = &(pd->v[0]);
                v2 = &(pd->v[2]);
                v3 = &(pd->v[1]);
            }
            else {
                v1 = &(pd->v[2]);
                v2 = &(pd->v[0]);
                v3 = &(pd->v[1]);
            }
        }
        else {
            if (pd->v[2].y < pd->v[1].y) {
                v1 = &(pd->v[2]);
                v2 = &(pd->v[1]);
                v3 = &(pd->v[0]);
            }
            else if (pd->v[0].y < pd->v[2].y) {
                v1 = &(pd->v[1]);
                v2 = &(pd->v[0]);
                v3 = &(pd->v[2]);
            }
            else {
                v1 = &(pd->v[1]);
                v2 = &(pd->v[2]);
                v3 = &(pd->v[0]);
            }
        }

        // Compute parameter starting points and deltas

        float dpdxs[MAX_VERTEX_PARAMS];
        float dpdys[MAX_VERTEX_PARAMS];
        float starts[MAX_VERTEX_PARAMS];
        if (pd->paramcount > 0) {
            float a02 = (v2->x * v3->y) - (v3->x * v2->y);
            float a12 = (v3->x * v1->y) - (v1->x * v3->y);
            float a22 = (v1->x * v2->y) - (v2->x * v1->y);
            float det = a02 + a12 + a22;
            
            if (fabsf(det) < 0.001) {
                memset(dpdxs, 0, sizeof(dpdxs));
                memset(dpdys, 0, sizeof(dpdys));
                for (int paramnum = 0; paramnum < pd->paramcount; paramnum++) {
                    starts[paramnum] = v1->p[paramnum];
                }
            }
            else {
                float a00 = v2->y - v3->y;
                float a01 = v3->x - v2->x;
                float a10 = v3->y - v1->y;
                float a11 = v1->x - v3->x;
                float a20 = v1->y - v2->y;
                float a21 = v2->x - v1->x;
                float idet = 1 / det;
                
                for (int paramnum = 0; paramnum < pd->paramcount; paramnum++) {
                    dpdxs[paramnum] = (idet * ((v1->p[paramnum] * a00) + 
                                               (v2->p[paramnum] * a10) + 
                                               (v3->p[paramnum] * a20)));
                    dpdys[paramnum] = (idet * ((v1->p[paramnum] * a01) + 
                                               (v2->p[paramnum] * a11) +
                                               (v3->p[paramnum] * a21)));
                    starts[paramnum] = (idet * ((v1->p[paramnum] * a02) +
                                                (v2->p[paramnum] * a12) + 
                                                (v3->p[paramnum] * a22)));
                }
            }
        }

        /* compute some integral X/Y vertex values */
        INT32 v1y = round_coordinate(v1->y);
        INT32 v3y = round_coordinate(v3->y);
        
        /* clip coordinates */
        v3y += bottomEdgeExtraPixelsM;
        if (pd->cliprect != NULL) {
            v1y = MAX(v1y, pd->cliprect->min_y);
            v3y = MIN(v3y, pd->cliprect->max_y + 1);
        }
        if (v3y <= v1y) {
            return;
        }
        
        /* compute the slopes for each portion of the triangle */
        float dxdy_v1v2 = 
            (v2->y == v1->y) ? 0.0f : (v2->x - v1->x) / (v2->y - v1->y);
        float dxdy_v1v3 = 
            (v3->y == v1->y) ? 0.0f : (v3->x - v1->x) / (v3->y - v1->y);
        float dxdy_v2v3 =
            (v3->y == v2->y) ? 0.0f : (v3->x - v2->x) / (v3->y - v2->y);
        
        /* Compute the first bucket into which this polygon will be
           bucketized */
        int first_bucket = (v1y / SCANLINES_PER_BUCKET);

        /* Compute last bucket to be included */
        int last_bucket = (v3y / SCANLINES_PER_BUCKET);

        /* Compute the total number of buckets that the polygon spans */
        int bucket_count = (last_bucket - first_bucket) + 1;

        /* Acquire the bucket items that will be used */
        BucketItem *bucketItems = bucketItemPoolM.Acquire(bucket_count);
        if (!bucketItems) {
            /* out of memory while rendering frame.  Just don't render this
               one at all. */
            return;
        }

        int scanlineCount = 
            (SCANLINES_PER_BUCKET - (v1y % SCANLINES_PER_BUCKET));
        for (int i = first_bucket; i <= last_bucket; i++) {
            BucketItem *bucketItem = bucketItems++;
            bucketItem->pd = pd;
            bucketItem->bucketNumber = i;
            bucketItem->scanlineFirst = v1y;
            /* Figure out how many scanlines to do for this bucket */
            int max_scanlineCount = v3y - v1y;
            if (scanlineCount > max_scanlineCount) {
                bucketItem->scanlineCount = max_scanlineCount;
            }
            else {
                bucketItem->scanlineCount = scanlineCount;
            }
            for (int j = 0; j < bucketItem->scanlineCount; j++) {
                poly_extent &extent = bucketItem->extents[j];
                int y = v1y + j;
                float fully = ((float) y) + 0.5f;
                float startx = v1->x + ((fully - v1->y) * dxdy_v1v3);
                float stopx;

                /* compute the ending X based on which part of the triangle
                   we're in */
                if (fully < v2->y) {
                    stopx = v1->x + ((fully - v1->y) * dxdy_v1v2);
                }
                else {
                    stopx = v2->x + ((fully - v2->y) * dxdy_v2v3);
                }

                /* clamp to full pixels */
                INT32 istartx = round_coordinate(startx);
                INT32 istopx = round_coordinate(stopx);

                /* force start < stop */
                if (istartx > istopx) {
                    INT32 temp = istartx;
                    istartx = istopx;
                    istopx = temp;
                }

                /* include the right edge if requested */
                istopx += rightEdgeExtraPixelsM;

                /* apply left/right clipping */
                if (pd->cliprect) {
                    if (istartx < pd->cliprect->min_x) {
                        istartx = pd->cliprect->min_x;
                    }
                    if (istopx > pd->cliprect->max_x) {
                        istopx = pd->cliprect->max_x + 1;
                    }
                }

                /* set the extent and update the total pixel count */
                if (istartx >= istopx) {
                    extent.startx = 0;
                    extent.stopx = 0;

                    /* Compute params for this scanline */
                    for (int k = 0; k < pd->paramcount; k++) {
                        extent.param[k].start = (starts[k] + (y * dpdys[k]));
                        extent.param[k].dpdx = 0;
                    }
                }
                else {
                    extent.startx = istartx;
                    extent.stopx = istopx;

                    /* Compute params for this scanline */
                    for (int k = 0; k < pd->paramcount; k++) {
                        extent.param[k].start = 
                            (starts[k] + (istartx * dpdxs[k]) + (y * dpdys[k]));
                        extent.param[k].dpdx = dpdxs[k];
                    }
                }
            }
            
            /* Schedule the bucket item to be rendered */
            bucketWorkQueuesM[i]->
                ScheduleWorkElement(pd->polygonNumber, bucketItem);

            /* Advance to next scanline block */
            v1y += bucketItem->scanlineCount;

            scanlineCount = SCANLINES_PER_BUCKET;
        }

        /* Mark all buckets up to first_bucket as skipping this polygon */
        for (int i = 0; i < first_bucket; i++) {
            bucketWorkQueuesM[i]->SkipWorkElement(pd->polygonNumber);
        }

        /* Mark all buckets after the last_bucket as skipping this polygon */
        for (int i = last_bucket + 1; i < BUCKET_COUNT; i++) {
            bucketWorkQueuesM[i]->SkipWorkElement(pd->polygonNumber);
        }
    }

    void SetupQuads(PolygonData *pd)
    {
    }

    void SetupPolygon(PolygonData *pd)
    {
    }

    void SetupExtents(PolygonData *pd)
    {
    }

    /**
     * TEMPORARY TESTING
     **/
    int nextPolygonNumberM;

    /**
     * This is the expected maximum number of polygons in a frame
     **/
    int maxPolysM;

    /**
     * Number of pixels to add the right edge
     **/
    int rightEdgeExtraPixelsM;

    /**
     * Number of pixels to add the bottom edge
     **/
    int bottomEdgeExtraPixelsM;

    /**
     * These are acquired (in the render calls) and released (in the poly_wait
     * call) by the main thread only, so no locking is needed to protect this
     * pool.
     **/
    Pool<PolygonData> polygonDataPoolM;

    /**
     * These are acquired (in the setup calls) and release (in the poly_wait
     * call) by the setup threads, so locking is needed to protect this pool.
     **/
    SharedPool<BucketItem, 65536> bucketItemPoolM;

    /**
     * "Extra" data that the main thread can allocate and will be associated
     * with all polygons until the next allocation.  Reset when WaitAndReset()
     * is called.
     **/
    void **extraDataM;
    int extraDataSizeM;
    int extraDataNextM;
    void *currentExtraDataM;

    /**
     * This is the osd_work_queue on which work items are scheduled
     **/
	osd_work_queue *osdWorkQueueM;

    /**
     * This is WorkQueue on which scanline setup work is scheduled
     **/
    WorkQueue<PolygonData *, SetupWorkQueueCb> setupWorkQueueM;

    /**
     * This is the set of WorkQueues on which bucket render work is scheduled
     **/
    NumberedWorkQueue<BucketItem *, 
                      BucketWorkQueueCb> *bucketWorkQueuesM[BUCKET_COUNT];
};


// --------------------------------------------------------------------------
// poly function implementation
// --------------------------------------------------------------------------

struct _poly_manager
{
    _poly_manager(int max_polys, int extra_data_size, bool include_right_edge,
                  bool include_bottom_edge)
        : renderer(max_polys, extra_data_size, include_right_edge,
                   include_bottom_edge)
    {
    }

    Renderer renderer;
};

poly_manager *poly_alloc(running_machine *machine, int max_polys,
                         size_t extra_data_size, UINT8 flags)
{
    poly_manager *ret = new poly_manager(max_polys, extra_data_size, 
                                         flags & POLYFLAG_INCLUDE_RIGHT_EDGE,
                                         flags & POLYFLAG_INCLUDE_BOTTOM_EDGE);
    if (!ret->renderer.IsValid()) {
        delete ret;
        ret = 0;
    }

    return ret;
}


/* free a poly manager */
void poly_free(poly_manager *poly)
{
    delete poly;
}


void poly_wait(poly_manager *poly, const char *debug_reason)
{
    (void) debug_reason;

    poly->renderer.WaitAndReset();
}


void *poly_get_extra_data(poly_manager *poly)
{
    return poly->renderer.GetExtraData();
}


UINT32 poly_render_triangle(poly_manager *poly, void *dest,
                            const rectangle *cliprect,
                            poly_draw_scanline_func callback, int paramcount,
                            const poly_vertex *v1, const poly_vertex *v2,
                            const poly_vertex *v3)
{
    return poly->renderer.RenderTriangle(/* temporary hardcoded */ false,
                                         dest, cliprect, callback, paramcount,
                                         v1, v2, v3);
}


UINT32 poly_render_triangle_fan(poly_manager *poly, void *dest,
                                const rectangle *cliprect,
                                poly_draw_scanline_func callback,
                                int paramcount, int numverts,
                                const poly_vertex *v)
{
    return poly->renderer.RenderTriangleFan(/* temporarily hardcoded */ false,
                                            dest, cliprect, callback, 
                                            paramcount, numverts, v);
}


UINT32 poly_render_triangle_custom(poly_manager *poly, void *dest,
                                   const rectangle *cliprect,
                                   poly_draw_scanline_func callback,
                                   int startscanline, int numscanlines,
                                   const poly_extent *extents)
{
    return poly->renderer.RenderExtents
        (/* temporarily hardcoded */ false, dest, cliprect, callback,
         startscanline, numscanlines, extents);
}


UINT32 poly_render_quad(poly_manager *poly, void *dest,
                        const rectangle *cliprect,
                        poly_draw_scanline_func callback, int paramcount,
                        const poly_vertex *v1, const poly_vertex *v2,
                        const poly_vertex *v3, const poly_vertex *v4)
{
    return poly->renderer.RenderQuad(/* temporarily hardcoded */ false,
                                     dest, cliprect, callback, 
                                     paramcount, v1, v2, v3, v4);
}


UINT32 poly_render_quad_fan(poly_manager *poly, void *dest,
                            const rectangle *cliprect,
                            poly_draw_scanline_func callback, int paramcount,
                            int numverts, const poly_vertex *v)
{
    return poly->renderer.RenderQuadFan(/* temporarily hardcoded */ false,
                                        dest, cliprect, callback, 
                                        paramcount, numverts, v);
}


UINT32 poly_render_polygon(poly_manager *poly, void *dest,
                           const rectangle *cliprect,
                           poly_draw_scanline_func callback, int paramcount,
                           int numverts, const poly_vertex *v)
{
    return poly->renderer.RenderPolygon(/* temporarily harcoded */ false,
                                        dest, cliprect, callback, 
                                        paramcount, numverts, v);
}


#else // !NEW_IMPLEMENTATION

/***************************************************************************
    DEBUGGING
***************************************************************************/

/* keep statistics */
#define KEEP_STATISTICS					0

/* turn this on to log the reasons for any long waits */
#define LOG_WAITS						0

/* number of profiling ticks before we consider a wait "long" */
#define LOG_WAIT_THRESHOLD				1000



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CACHE_LINE_SIZE					64			/* this is a general guess */
#define TOTAL_BUCKETS					(512 / SCANLINES_PER_BUCKET)
#define UNITS_PER_POLY					(100 / SCANLINES_PER_BUCKET)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* forward definitions */
typedef struct _polygon_info polygon_info;


/* tri_extent describes start/end points for a scanline */
typedef struct _tri_extent tri_extent;
struct _tri_extent
{
	INT16		startx;						/* starting X coordinate (inclusive) */
	INT16		stopx;						/* ending X coordinate (exclusive) */
};


/* single set of polygon per-parameter data */
typedef struct _poly_param poly_param;
struct _poly_param
{
	float		start;						/* parameter value at starting X,Y */
	float		dpdx;						/* dp/dx relative to starting X */
	float		dpdy;						/* dp/dy relative to starting Y */
};


/* poly edge is used internally for quad rendering */
typedef struct _poly_edge poly_edge;
struct _poly_edge
{
	poly_edge *			next;					/* next edge in sequence */
	int					index;					/* index of this edge */
	const poly_vertex *	v1;						/* pointer to first vertex */
	const poly_vertex *	v2;						/* pointer to second vertex */
	float				dxdy;					/* dx/dy along the edge */
	float				dpdy[MAX_VERTEX_PARAMS];/* per-parameter dp/dy values */
};


/* poly section is used internally for quad rendering */
typedef struct _poly_section poly_section;
struct _poly_section
{
	const poly_edge *	ledge;					/* pointer to left edge */
	const poly_edge *	redge;					/* pointer to right edge */
	float				ybottom;				/* bottom of this section */
};


/* work_unit_shared is a common set of data shared between tris and quads */
typedef struct _work_unit_shared work_unit_shared;
struct _work_unit_shared
{
	polygon_info *		polygon;				/* pointer to polygon */
	volatile UINT32		count_next;				/* number of scanlines and index of next item to process */
	INT16				scanline;				/* starting scanline and count */
	UINT16				previtem;				/* index of previous item in the same bucket */
#ifndef PTR64
	UINT32				dummy;					/* pad to 16 bytes */
#endif
};


/* tri_work_unit is a triangle-specific work-unit */
typedef struct _tri_work_unit tri_work_unit;
struct _tri_work_unit
{
	work_unit_shared	shared;					/* shared data */
	tri_extent			extent[SCANLINES_PER_BUCKET]; /* array of scanline extents */
};


/* quad_work_unit is a quad-specific work-unit */
typedef struct _quad_work_unit quad_work_unit;
struct _quad_work_unit
{
	work_unit_shared	shared;					/* shared data */
	poly_extent			extent[SCANLINES_PER_BUCKET]; /* array of scanline extents */
};


/* work_unit is a union of the two types */
typedef union _work_unit work_unit;
union _work_unit
{
	work_unit_shared	shared;					/* shared data */
	tri_work_unit		tri;					/* triangle work unit */
	quad_work_unit		quad;					/* quad work unit */
};


/* polygon_info describes a single polygon, which includes the poly_params */
struct _polygon_info
{
	poly_manager *		poly;					/* pointer back to the poly manager */
	void *				dest;					/* pointer to the destination we are rendering to */
	void *				extra;					/* extra data pointer */
	UINT8				numparams;				/* number of parameters for this polygon  */
	UINT8				numverts;				/* number of vertices in this polygon */
	poly_draw_scanline_func 	callback;				/* callback to handle a scanline's worth of work */
	INT32				xorigin;				/* X origin for all parameters */
	INT32				yorigin;				/* Y origin for all parameters */
	poly_param			param[MAX_VERTEX_PARAMS];/* array of parameter data */
};


/* full poly manager description */
struct _poly_manager
{
	/* queue management */
	osd_work_queue *	queue;					/* work queue */

	/* triangle work units */
	work_unit **		unit;					/* array of work unit pointers */
	UINT32				unit_next;				/* index of next unit to allocate */
	UINT32				unit_count;				/* number of work units available */
	size_t				unit_size;				/* size of each work unit, in bytes */

	/* quad work units */
	UINT32				quadunit_next;			/* index of next unit to allocate */
	UINT32				quadunit_count;			/* number of work units available */
	size_t				quadunit_size;			/* size of each work unit, in bytes */

	/* poly data */
	polygon_info **		polygon;				/* array of polygon pointers */
	UINT32				polygon_next;			/* index of next polygon to allocate */
	UINT32				polygon_count;			/* number of polygon items available */
	size_t				polygon_size;			/* size of each polygon, in bytes */

	/* extra data */
	void **				extra;					/* array of extra data pointers */
	UINT32				extra_next;				/* index of next extra data to allocate */
	UINT32				extra_count;			/* number of extra data items available */
	size_t				extra_size;				/* size of each extra data, in bytes */

	/* misc data */
	UINT8				flags;					/* flags */

	/* buckets */
	UINT16				unit_bucket[TOTAL_BUCKETS]; /* buckets for tracking unit usage */

	/* statistics */
	UINT32				triangles;				/* number of triangles queued */
	UINT32				quads;					/* number of quads queued */
	UINT64				pixels;					/* number of pixels rendered */
#if KEEP_STATISTICS
	UINT32				unit_waits;				/* number of times we waited for a unit */
	UINT32				unit_max;				/* maximum units used */
	UINT32				polygon_waits;			/* number of times we waited for a polygon */
	UINT32				polygon_max;			/* maximum polygons used */
	UINT32				extra_waits;			/* number of times we waited for an extra data */
	UINT32				extra_max;				/* maximum extra data used */
	UINT32				conflicts[WORK_MAX_THREADS]; /* number of conflicts found, per thread */
	UINT32				resolved[WORK_MAX_THREADS];	/* number of conflicts resolved, per thread */
#endif
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void **allocate_array(running_machine *machine, size_t *itemsize, UINT32 itemcount);
static void *poly_item_callback(void *param, int threadid);
static STATE_PRESAVE( poly_state_presave );



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    round_coordinate - round a coordinate to
    an integer, following rules that 0.5 rounds
    down
-------------------------------------------------*/

INLINE INT32 round_coordinate(float value)
{
	INT32 result = floor(value);
	return result + (value - (float)result > 0.5f);
}


/*-------------------------------------------------
    convert_tri_extent_to_poly_extent - convert
    a simple tri_extent to a full poly_extent
-------------------------------------------------*/

INLINE void convert_tri_extent_to_poly_extent(poly_extent *dstextent, const tri_extent *srcextent, const polygon_info *polygon, INT32 y)
{
	/* copy start/stop always */
	dstextent->startx = srcextent->startx;
	dstextent->stopx = srcextent->stopx;

	/* if we have parameters, process them as well */
	for (int paramnum = 0; paramnum < polygon->numparams; paramnum++)
	{
		dstextent->param[paramnum].start = polygon->param[paramnum].start + srcextent->startx * polygon->param[paramnum].dpdx + y * polygon->param[paramnum].dpdy;
		dstextent->param[paramnum].dpdx = polygon->param[paramnum].dpdx;
	}
}

#endif // !NEW_IMPLEMENTATION

/*-------------------------------------------------
    interpolate_vertex - interpolate values in
    a vertex based on p[0] crossing the clipval
-------------------------------------------------*/

INLINE void interpolate_vertex(poly_vertex *outv, const poly_vertex *v1, const poly_vertex *v2, int paramcount, float clipval)
{
	float frac = (clipval - v1->p[0]) / (v2->p[0] - v1->p[0]);
	int paramnum;

	/* create a new one at the intersection point */
	outv->x = v1->x + frac * (v2->x - v1->x);
	outv->y = v1->y + frac * (v2->y - v1->y);
	for (paramnum = 0; paramnum < paramcount; paramnum++)
		outv->p[paramnum] = v1->p[paramnum] + frac * (v2->p[paramnum] - v1->p[paramnum]);
}

/*-------------------------------------------------
    copy_vertex - copy vertex data from one to
    another
-------------------------------------------------*/

INLINE void copy_vertex(poly_vertex *outv, const poly_vertex *v, int paramcount)
{
	int paramnum;

	outv->x = v->x;
	outv->y = v->y;
	for (paramnum = 0; paramnum < paramcount; paramnum++)
		outv->p[paramnum] = v->p[paramnum];
}


#if NEW_IMPLEMENTATION
#else // !NEW_IMPLEMENTATION


/*-------------------------------------------------
    allocate_polygon - allocate a new polygon
    object, blocking if we run out
-------------------------------------------------*/

INLINE polygon_info *allocate_polygon(poly_manager *poly, int miny, int maxy)
{
	/* wait for a work item if we have to */
	if (poly->polygon_next + 1 > poly->polygon_count)
	{
		poly_wait(poly, "Out of polygons");
#if KEEP_STATISTICS
		poly->polygon_waits++;
#endif
	}
	else if (poly->unit_next + (maxy - miny) / SCANLINES_PER_BUCKET + 2 > poly->unit_count)
	{
		poly_wait(poly, "Out of work units");
#if KEEP_STATISTICS
		poly->unit_waits++;
#endif
	}
#if KEEP_STATISTICS
	poly->polygon_max = MAX(poly->polygon_max, poly->polygon_next + 1);
#endif
	return poly->polygon[poly->polygon_next++];
}



/***************************************************************************
    INITIALIZATION/TEARDOWN
***************************************************************************/

/*-------------------------------------------------
    poly_alloc - initialize a new polygon
    manager
-------------------------------------------------*/

poly_manager *poly_alloc(running_machine *machine, int max_polys, size_t extra_data_size, UINT8 flags)
{
	poly_manager *poly;

	/* allocate the manager itself */
	poly = auto_alloc_clear(machine, poly_manager);
	poly->flags = flags;

	/* allocate polygons */
	poly->polygon_size = sizeof(polygon_info);
	poly->polygon_count = MAX(max_polys, 1);
	poly->polygon_next = 0;
	poly->polygon = (polygon_info **)allocate_array(machine, &poly->polygon_size, poly->polygon_count);

	/* allocate extra data */
	poly->extra_size = extra_data_size;
	poly->extra_count = poly->polygon_count;
	poly->extra_next = 1;
	poly->extra = allocate_array(machine, &poly->extra_size, poly->extra_count);

	/* allocate triangle work units */
	poly->unit_size = (flags & POLYFLAG_ALLOW_QUADS) ? sizeof(quad_work_unit) : sizeof(tri_work_unit);
	poly->unit_count = MIN(poly->polygon_count * UNITS_PER_POLY, 65535);
	poly->unit_next = 0;
	poly->unit = (work_unit **)allocate_array(machine, &poly->unit_size, poly->unit_count);

	/* create the work queue */
	if (!(flags & POLYFLAG_NO_WORK_QUEUE))
		poly->queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_MULTI | WORK_QUEUE_FLAG_HIGH_FREQ);

	/* request a pre-save callback for synchronization */
	state_save_register_presave(machine, poly_state_presave, poly);
	return poly;
}


/*-------------------------------------------------
    poly_free - free a polygon manager
-------------------------------------------------*/

void poly_free(poly_manager *poly)
{
#if KEEP_STATISTICS
{
	int i, conflicts = 0, resolved = 0;
	for (i = 0; i < ARRAY_LENGTH(poly->conflicts); i++)
	{
		conflicts += poly->conflicts[i];
		resolved += poly->resolved[i];
	}
	printf("Total triangles = %d\n", poly->triangles);
	printf("Total quads = %d\n", poly->quads);
	if (poly->pixels > 1000000000)
		printf("Total pixels   = %d%09d\n", (UINT32)(poly->pixels / 1000000000), (UINT32)(poly->pixels % 1000000000));
	else
		printf("Total pixels   = %d\n", (UINT32)poly->pixels);
	printf("Conflicts:  %d resolved, %d total\n", resolved, conflicts);
	printf("Units:      %5d used, %5d allocated, %5d waits, %4d bytes each, %7d total\n", poly->unit_max, poly->unit_count, poly->unit_waits, poly->unit_size, poly->unit_count * poly->unit_size);
	printf("Polygons:   %5d used, %5d allocated, %5d waits, %4d bytes each, %7d total\n", poly->polygon_max, poly->polygon_count, poly->polygon_waits, poly->polygon_size, poly->polygon_count * poly->polygon_size);
	printf("Extra data: %5d used, %5d allocated, %5d waits, %4d bytes each, %7d total\n", poly->extra_max, poly->extra_count, poly->extra_waits, poly->extra_size, poly->extra_count * poly->extra_size);
}
#endif

	/* free the work queue */
	if (poly->queue != NULL)
		osd_work_queue_free(poly->queue);
}



/***************************************************************************
    COMMON FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    poly_wait - wait for all pending rendering
    to complete
-------------------------------------------------*/

void poly_wait(poly_manager *poly, const char *debug_reason)
{
	osd_ticks_t time;

	/* remember the start time if we're logging */
	if (LOG_WAITS)
		time = get_profile_ticks();

	/* wait for all pending work items to complete */
	if (poly->queue != NULL)
		osd_work_queue_wait(poly->queue, osd_ticks_per_second() * 100);

	/* if we don't have a queue, just run the whole list now */
	else
	{
		int unitnum;
		for (unitnum = 0; unitnum < poly->unit_next; unitnum++)
			poly_item_callback(poly->unit[unitnum], 0);
	}

	/* log any long waits */
	if (LOG_WAITS)
	{
		time = get_profile_ticks() - time;
		if (time > LOG_WAIT_THRESHOLD)
			logerror("Poly:Waited %d cycles for %s\n", (int)time, debug_reason);
	}

	/* reset the state */
	poly->polygon_next = poly->unit_next = 0;
	memset(poly->unit_bucket, 0xff, sizeof(poly->unit_bucket));

	/* we need to preserve the last extra data that was supplied */
	if (poly->extra_next > 1)
		memcpy(poly->extra[0], poly->extra[poly->extra_next - 1], poly->extra_size);
	poly->extra_next = 1;
}


/*-------------------------------------------------
    poly_get_extra_data - get a pointer to the
    extra data for the next polygon
-------------------------------------------------*/

void *poly_get_extra_data(poly_manager *poly)
{
	/* wait for a work item if we have to */
	if (poly->extra_next + 1 > poly->extra_count)
	{
		poly_wait(poly, "Out of extra data");
#if KEEP_STATISTICS
		poly->extra_waits++;
#endif
	}

	/* return a pointer to the extra data for the next item */
#if KEEP_STATISTICS
	poly->extra_max = MAX(poly->extra_max, poly->extra_next + 1);
#endif
	return poly->extra[poly->extra_next++];
}



/***************************************************************************
    CORE TRIANGLE RENDERING
***************************************************************************/

/*-------------------------------------------------
    poly_render_triangle - render a single
    triangle given 3 vertexes
-------------------------------------------------*/

UINT32 poly_render_triangle(poly_manager *poly, void *dest, const rectangle *cliprect, poly_draw_scanline_func callback, int paramcount, const poly_vertex *v1, const poly_vertex *v2, const poly_vertex *v3)
{
	float dxdy_v1v2, dxdy_v1v3, dxdy_v2v3;
	const poly_vertex *tv;
	INT32 curscan, scaninc;
	polygon_info *polygon;
	INT32 v1yclip, v3yclip;
	INT32 v1y, v3y, v1x;
	INT32 pixels = 0;
	UINT32 startunit;

	/* first sort by Y */
	if (v2->y < v1->y)
	{
		tv = v1;
		v1 = v2;
		v2 = tv;
	}
	if (v3->y < v2->y)
	{
		tv = v2;
		v2 = v3;
		v3 = tv;
		if (v2->y < v1->y)
		{
			tv = v1;
			v1 = v2;
			v2 = tv;
		}
	}

	/* compute some integral X/Y vertex values */
	v1x = round_coordinate(v1->x);
	v1y = round_coordinate(v1->y);
	v3y = round_coordinate(v3->y);

	/* clip coordinates */
	v1yclip = v1y;
	v3yclip = v3y + ((poly->flags & POLYFLAG_INCLUDE_BOTTOM_EDGE) ? 1 : 0);
	if (cliprect != NULL)
	{
		v1yclip = MAX(v1yclip, cliprect->min_y);
		v3yclip = MIN(v3yclip, cliprect->max_y + 1);
	}
	if (v3yclip - v1yclip <= 0)
		return 0;

	/* allocate a new polygon */
	polygon = allocate_polygon(poly, v1yclip, v3yclip);

	/* fill in the polygon information */
	polygon->poly = poly;
	polygon->dest = dest;
	polygon->callback = callback;
	polygon->extra = poly->extra[poly->extra_next - 1];
	polygon->numparams = paramcount;
	polygon->numverts = 3;

	/* set the start X/Y coordinates */
	polygon->xorigin = v1x;
	polygon->yorigin = v1y;

	/* compute the slopes for each portion of the triangle */
	dxdy_v1v2 = (v2->y == v1->y) ? 0.0f : (v2->x - v1->x) / (v2->y - v1->y);
	dxdy_v1v3 = (v3->y == v1->y) ? 0.0f : (v3->x - v1->x) / (v3->y - v1->y);
	dxdy_v2v3 = (v3->y == v2->y) ? 0.0f : (v3->x - v2->x) / (v3->y - v2->y);

	/* compute the X extents for each scanline */
	startunit = poly->unit_next;
	for (curscan = v1yclip; curscan < v3yclip; curscan += scaninc)
	{
		UINT32 bucketnum = ((UINT32)curscan / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		UINT32 unit_index = poly->unit_next++;
		tri_work_unit *unit = &poly->unit[unit_index]->tri;
		int extnum;

		/* determine how much to advance to hit the next bucket */
		scaninc = SCANLINES_PER_BUCKET - (UINT32)curscan % SCANLINES_PER_BUCKET;

		/* fill in the work unit basics */
		unit->shared.polygon = polygon;
		unit->shared.count_next = MIN(v3yclip - curscan, scaninc);
		unit->shared.scanline = curscan;
		unit->shared.previtem = poly->unit_bucket[bucketnum];
		poly->unit_bucket[bucketnum] = unit_index;

		/* iterate over extents */
		for (extnum = 0; extnum < unit->shared.count_next; extnum++)
		{
			float fully = (float)(curscan + extnum) + 0.5f;
			float startx = v1->x + (fully - v1->y) * dxdy_v1v3;
			float stopx;
			INT32 istartx, istopx;

			/* compute the ending X based on which part of the triangle we're in */
			if (fully < v2->y)
				stopx = v1->x + (fully - v1->y) * dxdy_v1v2;
			else
				stopx = v2->x + (fully - v2->y) * dxdy_v2v3;

			/* clamp to full pixels */
			istartx = round_coordinate(startx);
			istopx = round_coordinate(stopx);

			/* force start < stop */
			if (istartx > istopx)
			{
				INT32 temp = istartx;
				istartx = istopx;
				istopx = temp;
			}

			/* include the right edge if requested */
			if (poly->flags & POLYFLAG_INCLUDE_RIGHT_EDGE)
				istopx++;

			/* apply left/right clipping */
			if (cliprect != NULL)
			{
				if (istartx < cliprect->min_x)
					istartx = cliprect->min_x;
				if (istopx > cliprect->max_x)
					istopx = cliprect->max_x + 1;
			}

			/* set the extent and update the total pixel count */
			if (istartx >= istopx)
				istartx = istopx = 0;
			unit->extent[extnum].startx = istartx;
			unit->extent[extnum].stopx = istopx;
			pixels += istopx - istartx;
		}
	}
#if KEEP_STATISTICS
	poly->unit_max = MAX(poly->unit_max, poly->unit_next);
#endif

	/* compute parameter starting points and deltas */
	if (paramcount > 0)
	{
		float a00 = v2->y - v3->y;
		float a01 = v3->x - v2->x;
		float a02 = v2->x*v3->y - v3->x*v2->y;
		float a10 = v3->y - v1->y;
		float a11 = v1->x - v3->x;
		float a12 = v3->x*v1->y - v1->x*v3->y;
		float a20 = v1->y - v2->y;
		float a21 = v2->x - v1->x;
		float a22 = v1->x*v2->y - v2->x*v1->y;
		float det = a02 + a12 + a22;

		if(fabsf(det) < 0.001) {
			for (int paramnum = 0; paramnum < paramcount; paramnum++)
			{
				poly_param *params = &polygon->param[paramnum];
				params->dpdx = 0;
				params->dpdy = 0;
				params->start = v1->p[paramnum];
			}
		}
		else
		{
			float idet = 1/det;
			for (int paramnum = 0; paramnum < paramcount; paramnum++)
			{
				poly_param *params = &polygon->param[paramnum];
				params->dpdx  = idet*(v1->p[paramnum]*a00 + v2->p[paramnum]*a10 + v3->p[paramnum]*a20);
				params->dpdy  = idet*(v1->p[paramnum]*a01 + v2->p[paramnum]*a11 + v3->p[paramnum]*a21);
				params->start = idet*(v1->p[paramnum]*a02 + v2->p[paramnum]*a12 + v3->p[paramnum]*a22);
			}
		}
	}

	/* enqueue the work items */
	if (poly->queue != NULL)
		osd_work_item_queue_multiple(poly->queue, poly_item_callback, poly->unit_next - startunit, poly->unit[startunit], poly->unit_size, WORK_ITEM_FLAG_AUTO_RELEASE);

	/* return the total number of pixels in the triangle */
	poly->triangles++;
	poly->pixels += pixels;
	return pixels;
}


/*-------------------------------------------------
    poly_render_triangle_fan - render a set of
    triangles in a fan
-------------------------------------------------*/

UINT32 poly_render_triangle_fan(poly_manager *poly, void *dest, const rectangle *cliprect, poly_draw_scanline_func callback, int paramcount, int numverts, const poly_vertex *v)
{
	UINT32 pixels = 0;
	int vertnum;

	/* iterate over vertices */
	for (vertnum = 2; vertnum < numverts; vertnum++)
		pixels += poly_render_triangle(poly, dest, cliprect, callback, paramcount, &v[0], &v[vertnum - 1], &v[vertnum]);
	return pixels;
}


/*-------------------------------------------------
    poly_render_triangle_custom - perform a custom
    render of an object, given specific extents
-------------------------------------------------*/

UINT32 poly_render_triangle_custom(poly_manager *poly, void *dest, const rectangle *cliprect, poly_draw_scanline_func callback, int startscanline, int numscanlines, const poly_extent *extents)
{
	INT32 curscan, scaninc;
	polygon_info *polygon;
	INT32 v1yclip, v3yclip;
	INT32 pixels = 0;
	UINT32 startunit;

	/* clip coordinates */
	if (cliprect != NULL)
	{
		v1yclip = MAX(startscanline, cliprect->min_y);
		v3yclip = MIN(startscanline + numscanlines, cliprect->max_y + 1);
	}
	else
	{
		v1yclip = startscanline;
		v3yclip = startscanline + numscanlines;
	}
	if (v3yclip - v1yclip <= 0)
		return 0;

	/* allocate a new polygon */
	polygon = allocate_polygon(poly, v1yclip, v3yclip);

	/* fill in the polygon information */
	polygon->poly = poly;
	polygon->dest = dest;
	polygon->callback = callback;
	polygon->extra = poly->extra[poly->extra_next - 1];
	polygon->numparams = 0;
	polygon->numverts = 3;

	/* compute the X extents for each scanline */
	startunit = poly->unit_next;
	for (curscan = v1yclip; curscan < v3yclip; curscan += scaninc)
	{
		UINT32 bucketnum = ((UINT32)curscan / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		UINT32 unit_index = poly->unit_next++;
		tri_work_unit *unit = &poly->unit[unit_index]->tri;
		int extnum;

		/* determine how much to advance to hit the next bucket */
		scaninc = SCANLINES_PER_BUCKET - (UINT32)curscan % SCANLINES_PER_BUCKET;

		/* fill in the work unit basics */
		unit->shared.polygon = polygon;
		unit->shared.count_next = MIN(v3yclip - curscan, scaninc);
		unit->shared.scanline = curscan;
		unit->shared.previtem = poly->unit_bucket[bucketnum];
		poly->unit_bucket[bucketnum] = unit_index;

		/* iterate over extents */
		for (extnum = 0; extnum < unit->shared.count_next; extnum++)
		{
			const poly_extent *extent = &extents[(curscan + extnum) - startscanline];
			INT32 istartx = extent->startx, istopx = extent->stopx;

			/* force start < stop */
			if (istartx > istopx)
			{
				INT32 temp = istartx;
				istartx = istopx;
				istopx = temp;
			}

			/* apply left/right clipping */
			if (cliprect != NULL)
			{
				if (istartx < cliprect->min_x)
					istartx = cliprect->min_x;
				if (istopx > cliprect->max_x)
					istopx = cliprect->max_x + 1;
			}

			/* set the extent and update the total pixel count */
			unit->extent[extnum].startx = istartx;
			unit->extent[extnum].stopx = istopx;
			if (istartx < istopx)
				pixels += istopx - istartx;
		}
	}
#if KEEP_STATISTICS
	poly->unit_max = MAX(poly->unit_max, poly->unit_next);
#endif

	/* enqueue the work items */
	if (poly->queue != NULL)
		osd_work_item_queue_multiple(poly->queue, poly_item_callback, poly->unit_next - startunit, poly->unit[startunit], poly->unit_size, WORK_ITEM_FLAG_AUTO_RELEASE);

	/* return the total number of pixels in the object */
	poly->triangles++;
	poly->pixels += pixels;
	return pixels;
}



/***************************************************************************
    CORE QUAD RENDERING
***************************************************************************/

/*-------------------------------------------------
    poly_render_quad - render a single quad
    given 4 vertexes
-------------------------------------------------*/

UINT32 poly_render_quad(poly_manager *poly, void *dest, const rectangle *cliprect, poly_draw_scanline_func callback, int paramcount, const poly_vertex *v1, const poly_vertex *v2, const poly_vertex *v3, const poly_vertex *v4)
{
	poly_edge fedgelist[3], bedgelist[3];
	const poly_edge *ledge, *redge;
	const poly_vertex *v[4];
	poly_edge *edgeptr;
	int minv, maxv, curv;
	INT32 minyclip, maxyclip;
	INT32 miny, maxy;
	INT32 curscan, scaninc;
	polygon_info *polygon;
	INT32 pixels = 0;
	UINT32 startunit;

	assert(poly->flags & POLYFLAG_ALLOW_QUADS);

	/* arrays make things easier */
	v[0] = v1;
	v[1] = v2;
	v[2] = v3;
	v[3] = v4;

	/* determine min/max Y vertices */
	if (v[1]->y < v[0]->y)
		minv = 1, maxv = 0;
	else
		minv = 0, maxv = 1;
	if (v[2]->y < v[minv]->y)
		minv = 2;
	else if (v[2]->y > v[maxv]->y)
		maxv = 2;
	if (v[3]->y < v[minv]->y)
		minv = 3;
	else if (v[3]->y > v[maxv]->y)
		maxv = 3;

	/* determine start/end scanlines */
	miny = round_coordinate(v[minv]->y);
	maxy = round_coordinate(v[maxv]->y);

	/* clip coordinates */
	minyclip = miny;
	maxyclip = maxy + ((poly->flags & POLYFLAG_INCLUDE_BOTTOM_EDGE) ? 1 : 0);
	if (cliprect != NULL)
	{
		minyclip = MAX(minyclip, cliprect->min_y);
		maxyclip = MIN(maxyclip, cliprect->max_y + 1);
	}
	if (maxyclip - minyclip <= 0)
		return 0;

	/* allocate a new polygon */
	polygon = allocate_polygon(poly, minyclip, maxyclip);

	/* fill in the polygon information */
	polygon->poly = poly;
	polygon->dest = dest;
	polygon->callback = callback;
	polygon->extra = poly->extra[poly->extra_next - 1];
	polygon->numparams = paramcount;
	polygon->numverts = 4;

	/* walk forward to build up the forward edge list */
	edgeptr = &fedgelist[0];
	for (curv = minv; curv != maxv; curv = (curv + 1) & 3)
	{
		int paramnum;
		float ooy;

		/* set the two vertices */
		edgeptr->v1 = v[curv];
		edgeptr->v2 = v[(curv + 1) & 3];

		/* if horizontal, skip altogether */
		if (edgeptr->v1->y == edgeptr->v2->y)
			continue;

		/* need dx/dy always, and parameter deltas as necessary */
		ooy = 1.0f / (edgeptr->v2->y - edgeptr->v1->y);
		edgeptr->dxdy = (edgeptr->v2->x - edgeptr->v1->x) * ooy;
		for (paramnum = 0; paramnum < paramcount; paramnum++)
			edgeptr->dpdy[paramnum] = (edgeptr->v2->p[paramnum] - edgeptr->v1->p[paramnum]) * ooy;
		edgeptr++;
	}

	/* walk backward to build up the backward edge list */
	edgeptr = &bedgelist[0];
	for (curv = minv; curv != maxv; curv = (curv - 1) & 3)
	{
		int paramnum;
		float ooy;

		/* set the two vertices */
		edgeptr->v1 = v[curv];
		edgeptr->v2 = v[(curv - 1) & 3];

		/* if horizontal, skip altogether */
		if (edgeptr->v1->y == edgeptr->v2->y)
			continue;

		/* need dx/dy always, and parameter deltas as necessary */
		ooy = 1.0f / (edgeptr->v2->y - edgeptr->v1->y);
		edgeptr->dxdy = (edgeptr->v2->x - edgeptr->v1->x) * ooy;
		for (paramnum = 0; paramnum < paramcount; paramnum++)
			edgeptr->dpdy[paramnum] = (edgeptr->v2->p[paramnum] - edgeptr->v1->p[paramnum]) * ooy;
		edgeptr++;
	}

	/* determine which list is left/right: */
	/* if the first vertex is shared, compare the slopes */
	/* if the first vertex is not shared, compare the X coordinates */
	if ((fedgelist[0].v1 == bedgelist[0].v1 && fedgelist[0].dxdy < bedgelist[0].dxdy) ||
		(fedgelist[0].v1 != bedgelist[0].v1 && fedgelist[0].v1->x < bedgelist[0].v1->x))
	{
		ledge = fedgelist;
		redge = bedgelist;
	}
	else
	{
		ledge = bedgelist;
		redge = fedgelist;
	}

	/* compute the X extents for each scanline */
	startunit = poly->unit_next;
	for (curscan = minyclip; curscan < maxyclip; curscan += scaninc)
	{
		UINT32 bucketnum = ((UINT32)curscan / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		UINT32 unit_index = poly->unit_next++;
		quad_work_unit *unit = &poly->unit[unit_index]->quad;
		int extnum;

		/* determine how much to advance to hit the next bucket */
		scaninc = SCANLINES_PER_BUCKET - (UINT32)curscan % SCANLINES_PER_BUCKET;

		/* fill in the work unit basics */
		unit->shared.polygon = polygon;
		unit->shared.count_next = MIN(maxyclip - curscan, scaninc);
		unit->shared.scanline = curscan;
		unit->shared.previtem = poly->unit_bucket[bucketnum];
		poly->unit_bucket[bucketnum] = unit_index;

		/* iterate over extents */
		for (extnum = 0; extnum < unit->shared.count_next; extnum++)
		{
			float fully = (float)(curscan + extnum) + 0.5f;
			float startx, stopx;
			INT32 istartx, istopx;
			int paramnum;

			/* compute the ending X based on which part of the triangle we're in */
			while (fully > ledge->v2->y && fully < v[maxv]->y)
				ledge++;
			while (fully > redge->v2->y && fully < v[maxv]->y)
				redge++;
			startx = ledge->v1->x + (fully - ledge->v1->y) * ledge->dxdy;
			stopx = redge->v1->x + (fully - redge->v1->y) * redge->dxdy;

			/* clamp to full pixels */
			istartx = round_coordinate(startx);
			istopx = round_coordinate(stopx);

			/* compute parameter starting points and deltas */
			if (paramcount > 0)
			{
				float ldy = fully - ledge->v1->y;
				float rdy = fully - redge->v1->y;
				float oox = 1.0f / (stopx - startx);

				/* iterate over parameters */
				for (paramnum = 0; paramnum < paramcount; paramnum++)
				{
					float lparam = ledge->v1->p[paramnum] + ldy * ledge->dpdy[paramnum];
					float rparam = redge->v1->p[paramnum] + rdy * redge->dpdy[paramnum];
					float dpdx = (rparam - lparam) * oox;

					unit->extent[extnum].param[paramnum].start = lparam;// - ((float)istartx + 0.5f) * dpdx;
					unit->extent[extnum].param[paramnum].dpdx = dpdx;
				}
			}

			/* include the right edge if requested */
			if (poly->flags & POLYFLAG_INCLUDE_RIGHT_EDGE)
				istopx++;

			/* apply left/right clipping */
			if (cliprect != NULL)
			{
				if (istartx < cliprect->min_x)
				{
					for (paramnum = 0; paramnum < paramcount; paramnum++)
						unit->extent[extnum].param[paramnum].start += (cliprect->min_x - istartx) * unit->extent[extnum].param[paramnum].dpdx;
					istartx = cliprect->min_x;
				}
				if (istopx > cliprect->max_x)
					istopx = cliprect->max_x + 1;
			}

			/* set the extent and update the total pixel count */
			if (istartx >= istopx)
				istartx = istopx = 0;
			unit->extent[extnum].startx = istartx;
			unit->extent[extnum].stopx = istopx;
			pixels += istopx - istartx;
		}
	}
#if KEEP_STATISTICS
	poly->unit_max = MAX(poly->unit_max, poly->unit_next);
#endif

	/* enqueue the work items */
	if (poly->queue != NULL)
		osd_work_item_queue_multiple(poly->queue, poly_item_callback, poly->unit_next - startunit, poly->unit[startunit], poly->unit_size, WORK_ITEM_FLAG_AUTO_RELEASE);

	/* return the total number of pixels in the triangle */
	poly->quads++;
	poly->pixels += pixels;
	return pixels;
}


/*-------------------------------------------------
    poly_render_quad_fan - render a set of
    quads in a fan
-------------------------------------------------*/

UINT32 poly_render_quad_fan(poly_manager *poly, void *dest, const rectangle *cliprect, poly_draw_scanline_func callback, int paramcount, int numverts, const poly_vertex *v)
{
	UINT32 pixels = 0;
	int vertnum;

	/* iterate over vertices */
	for (vertnum = 2; vertnum < numverts; vertnum += 2)
		pixels += poly_render_quad(poly, dest, cliprect, callback, paramcount, &v[0], &v[vertnum - 1], &v[vertnum], &v[MIN(vertnum + 1, numverts - 1)]);
	return pixels;
}



/***************************************************************************
    CORE POLYGON RENDERING
***************************************************************************/

/*-------------------------------------------------
    poly_render_polygon - render a single polygon up
    to 32 vertices
-------------------------------------------------*/

UINT32 poly_render_polygon(poly_manager *poly, void *dest, const rectangle *cliprect, poly_draw_scanline_func callback, int paramcount, int numverts, const poly_vertex *v)
{
	poly_edge fedgelist[MAX_POLYGON_VERTS - 1], bedgelist[MAX_POLYGON_VERTS - 1];
	const poly_edge *ledge, *redge;
	poly_edge *edgeptr;
	int minv, maxv, curv;
	INT32 minyclip, maxyclip;
	INT32 miny, maxy;
	INT32 curscan, scaninc;
	polygon_info *polygon;
	INT32 pixels = 0;
	UINT32 startunit;
	int vertnum;

	assert(poly->flags & POLYFLAG_ALLOW_QUADS);

	/* determine min/max Y vertices */
	minv = maxv = 0;
	for (vertnum = 1; vertnum < numverts; vertnum++)
	{
		if (v[vertnum].y < v[minv].y)
			minv = vertnum;
		else if (v[vertnum].y > v[maxv].y)
			maxv = vertnum;
	}

	/* determine start/end scanlines */
	miny = round_coordinate(v[minv].y);
	maxy = round_coordinate(v[maxv].y);

	/* clip coordinates */
	minyclip = miny;
	maxyclip = maxy + ((poly->flags & POLYFLAG_INCLUDE_BOTTOM_EDGE) ? 1 : 0);
	if (cliprect != NULL)
	{
		minyclip = MAX(minyclip, cliprect->min_y);
		maxyclip = MIN(maxyclip, cliprect->max_y + 1);
	}
	if (maxyclip - minyclip <= 0)
		return 0;

	/* allocate a new polygon */
	polygon = allocate_polygon(poly, minyclip, maxyclip);

	/* fill in the polygon information */
	polygon->poly = poly;
	polygon->dest = dest;
	polygon->callback = callback;
	polygon->extra = poly->extra[poly->extra_next - 1];
	polygon->numparams = paramcount;
	polygon->numverts = numverts;

	/* walk forward to build up the forward edge list */
	edgeptr = &fedgelist[0];
	for (curv = minv; curv != maxv; curv = (curv == numverts - 1) ? 0 : (curv + 1))
	{
		int paramnum;
		float ooy;

		/* set the two vertices */
		edgeptr->v1 = &v[curv];
		edgeptr->v2 = &v[(curv == numverts - 1) ? 0 : (curv + 1)];

		/* if horizontal, skip altogether */
		if (edgeptr->v1->y == edgeptr->v2->y)
			continue;

		/* need dx/dy always, and parameter deltas as necessary */
		ooy = 1.0f / (edgeptr->v2->y - edgeptr->v1->y);
		edgeptr->dxdy = (edgeptr->v2->x - edgeptr->v1->x) * ooy;
		for (paramnum = 0; paramnum < paramcount; paramnum++)
			edgeptr->dpdy[paramnum] = (edgeptr->v2->p[paramnum] - edgeptr->v1->p[paramnum]) * ooy;
		edgeptr++;
	}

	/* walk backward to build up the backward edge list */
	edgeptr = &bedgelist[0];
	for (curv = minv; curv != maxv; curv = (curv == 0) ? (numverts - 1) : (curv - 1))
	{
		int paramnum;
		float ooy;

		/* set the two vertices */
		edgeptr->v1 = &v[curv];
		edgeptr->v2 = &v[(curv == 0) ? (numverts - 1) : (curv - 1)];

		/* if horizontal, skip altogether */
		if (edgeptr->v1->y == edgeptr->v2->y)
			continue;

		/* need dx/dy always, and parameter deltas as necessary */
		ooy = 1.0f / (edgeptr->v2->y - edgeptr->v1->y);
		edgeptr->dxdy = (edgeptr->v2->x - edgeptr->v1->x) * ooy;
		for (paramnum = 0; paramnum < paramcount; paramnum++)
			edgeptr->dpdy[paramnum] = (edgeptr->v2->p[paramnum] - edgeptr->v1->p[paramnum]) * ooy;
		edgeptr++;
	}

	/* determine which list is left/right: */
	/* if the first vertex is shared, compare the slopes */
	/* if the first vertex is not shared, compare the X coordinates */
	if ((fedgelist[0].v1 == bedgelist[0].v1 && fedgelist[0].dxdy < bedgelist[0].dxdy) ||
		(fedgelist[0].v1 != bedgelist[0].v1 && fedgelist[0].v1->x < bedgelist[0].v1->x))
	{
		ledge = fedgelist;
		redge = bedgelist;
	}
	else
	{
		ledge = bedgelist;
		redge = fedgelist;
	}

	/* compute the X extents for each scanline */
	startunit = poly->unit_next;
	for (curscan = minyclip; curscan < maxyclip; curscan += scaninc)
	{
		UINT32 bucketnum = ((UINT32)curscan / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		UINT32 unit_index = poly->unit_next++;
		quad_work_unit *unit = &poly->unit[unit_index]->quad;
		int extnum;

		/* determine how much to advance to hit the next bucket */
		scaninc = SCANLINES_PER_BUCKET - (UINT32)curscan % SCANLINES_PER_BUCKET;

		/* fill in the work unit basics */
		unit->shared.polygon = polygon;
		unit->shared.count_next = MIN(maxyclip - curscan, scaninc);
		unit->shared.scanline = curscan;
		unit->shared.previtem = poly->unit_bucket[bucketnum];
		poly->unit_bucket[bucketnum] = unit_index;

		/* iterate over extents */
		for (extnum = 0; extnum < unit->shared.count_next; extnum++)
		{
			float fully = (float)(curscan + extnum) + 0.5f;
			float startx, stopx;
			INT32 istartx, istopx;
			int paramnum;

			/* compute the ending X based on which part of the triangle we're in */
			while (fully > ledge->v2->y && fully < v[maxv].y)
				ledge++;
			while (fully > redge->v2->y && fully < v[maxv].y)
				redge++;
			startx = ledge->v1->x + (fully - ledge->v1->y) * ledge->dxdy;
			stopx = redge->v1->x + (fully - redge->v1->y) * redge->dxdy;

			/* clamp to full pixels */
			istartx = round_coordinate(startx);
			istopx = round_coordinate(stopx);

			/* compute parameter starting points and deltas */
			if (paramcount > 0)
			{
				float ldy = fully - ledge->v1->y;
				float rdy = fully - redge->v1->y;
				float oox = 1.0f / (stopx - startx);

				/* iterate over parameters */
				for (paramnum = 0; paramnum < paramcount; paramnum++)
				{
					float lparam = ledge->v1->p[paramnum] + ldy * ledge->dpdy[paramnum];
					float rparam = redge->v1->p[paramnum] + rdy * redge->dpdy[paramnum];
					float dpdx = (rparam - lparam) * oox;

					unit->extent[extnum].param[paramnum].start = lparam;// - ((float)istartx + 0.5f) * dpdx;
					unit->extent[extnum].param[paramnum].dpdx = dpdx;
				}
			}

			/* include the right edge if requested */
			if (poly->flags & POLYFLAG_INCLUDE_RIGHT_EDGE)
				istopx++;

			/* apply left/right clipping */
			if (cliprect != NULL)
			{
				if (istartx < cliprect->min_x)
				{
					for (paramnum = 0; paramnum < paramcount; paramnum++)
						unit->extent[extnum].param[paramnum].start += (cliprect->min_x - istartx) * unit->extent[extnum].param[paramnum].dpdx;
					istartx = cliprect->min_x;
				}
				if (istopx > cliprect->max_x)
					istopx = cliprect->max_x + 1;
			}

			/* set the extent and update the total pixel count */
			if (istartx >= istopx)
				istartx = istopx = 0;
			unit->extent[extnum].startx = istartx;
			unit->extent[extnum].stopx = istopx;
			pixels += istopx - istartx;
		}
	}
#if KEEP_STATISTICS
	poly->unit_max = MAX(poly->unit_max, poly->unit_next);
#endif

	/* enqueue the work items */
	if (poly->queue != NULL)
		osd_work_item_queue_multiple(poly->queue, poly_item_callback, poly->unit_next - startunit, poly->unit[startunit], poly->unit_size, WORK_ITEM_FLAG_AUTO_RELEASE);

	/* return the total number of pixels in the triangle */
	poly->quads++;
	poly->pixels += pixels;
	return pixels;
}


#endif // !NEW_IMPLEMENTATION


/***************************************************************************
    CLIPPING
***************************************************************************/

/*-------------------------------------------------
    poly_zclip_if_less - z clip a polygon against
    the given value, returning a set of clipped
    vertices
-------------------------------------------------*/

int poly_zclip_if_less(int numverts, const poly_vertex *v, poly_vertex *outv, int paramcount, float clipval)
{
	int prevclipped = (v[numverts - 1].p[0] < clipval);
	poly_vertex *nextout = outv;
	int vertnum;

	/* iterate over vertices */
	for (vertnum = 0; vertnum < numverts; vertnum++)
	{
		int thisclipped = (v[vertnum].p[0] < clipval);

		/* if we switched from clipped to non-clipped, interpolate a vertex */
		if (thisclipped != prevclipped)
			interpolate_vertex(nextout++, &v[(vertnum == 0) ? (numverts - 1) : (vertnum - 1)], &v[vertnum], paramcount, clipval);

		/* if this vertex is not clipped, copy it in */
		if (!thisclipped)
			copy_vertex(nextout++, &v[vertnum], paramcount);

		/* remember the last state */
		prevclipped = thisclipped;
	}
	return nextout - outv;
}

#if NEW_IMPLEMENTATION
#else // !NEW_IMPLEMENTATION


/***************************************************************************
    INTERNAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    allocate_array - allocate an array of pointers
-------------------------------------------------*/

static void **allocate_array(running_machine *machine, size_t *itemsize, UINT32 itemcount)
{
	void **ptrarray;
	int itemnum;

	/* fail if 0 */
	if (itemcount == 0)
		return NULL;

	/* round to a cache line boundary */
	*itemsize = ((*itemsize + CACHE_LINE_SIZE - 1) / CACHE_LINE_SIZE) * CACHE_LINE_SIZE;

	/* allocate the array */
	ptrarray = auto_alloc_array_clear(machine, void *, itemcount);

	/* allocate the actual items */
	ptrarray[0] = auto_alloc_array_clear(machine, UINT8, *itemsize * itemcount);

	/* initialize the pointer array */
	for (itemnum = 1; itemnum < itemcount; itemnum++)
		ptrarray[itemnum] = (UINT8 *)ptrarray[0] + *itemsize * itemnum;
	return ptrarray;
}


/*-------------------------------------------------
    poly_item_callback - callback for each poly
    item
-------------------------------------------------*/

static void *poly_item_callback(void *param, int threadid)
{
	while (1)
	{
		work_unit *unit = (work_unit *)param;
		polygon_info *polygon = unit->shared.polygon;
		int count = unit->shared.count_next & 0xffff;
		UINT32 orig_count_next;
		int curscan;

		/* if our previous item isn't done yet, enqueue this item to the end and proceed */
		if (unit->shared.previtem != 0xffff)
		{
			work_unit *prevunit = polygon->poly->unit[unit->shared.previtem];
			if (prevunit->shared.count_next != 0)
			{
				UINT32 unitnum = ((UINT8 *)unit - (UINT8 *)polygon->poly->unit[0]) / polygon->poly->unit_size;
				UINT32 new_count_next;

				/* attempt to atomically swap in this new value */
				do
				{
					orig_count_next = prevunit->shared.count_next;
					new_count_next = orig_count_next | (unitnum << 16);
				} while (compare_exchange32((volatile INT32 *)&prevunit->shared.count_next, orig_count_next, new_count_next) != orig_count_next);

#if KEEP_STATISTICS
				/* track resolved conflicts */
				polygon->poly->conflicts[threadid]++;
				if (orig_count_next != 0)
					polygon->poly->resolved[threadid]++;
#endif
				/* if we succeeded, skip out early so we can do other work */
				if (orig_count_next != 0)
					break;
			}
		}

		/* iterate over extents */
		for (curscan = 0; curscan < count; curscan++)
		{
			if (polygon->numverts == 3)
			{
				poly_extent tmpextent;
				convert_tri_extent_to_poly_extent(&tmpextent, &unit->tri.extent[curscan], polygon, unit->shared.scanline + curscan);
				(*polygon->callback)(polygon->dest, unit->shared.scanline + curscan, &tmpextent, polygon->extra, threadid);
			}
			else
				(*polygon->callback)(polygon->dest, unit->shared.scanline + curscan, &unit->quad.extent[curscan], polygon->extra, threadid);
		}

		/* set our count to 0 and re-fetch the original count value */
		do
		{
			orig_count_next = unit->shared.count_next;
		} while (compare_exchange32((volatile INT32 *)&unit->shared.count_next, orig_count_next, 0) != orig_count_next);

		/* if we have no more work to do, do nothing */
		orig_count_next >>= 16;
		if (orig_count_next == 0)
			break;
		param = polygon->poly->unit[orig_count_next];
	}
	return NULL;
}


/*-------------------------------------------------
    poly_state_presave - pre-save callback to
    ensure everything is synced before saving
-------------------------------------------------*/

static STATE_PRESAVE( poly_state_presave )
{
	poly_manager *poly = (poly_manager *)param;
	poly_wait(poly, "pre-save");
}

#endif // !NEW_IMPLEMENTATION
