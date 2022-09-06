#include <thread_pool.h>
#include <stdlib.h>

/*
 * The thread pool is going to be a base structure containing a list of the
 * actual threads (or workers). and a queue of tasks. A task is any enqueued
 * tasks that need to be processed by a single thread.
 *
 * // Tasks
 * Tasks is going to be a queue of tasks that need to be processed asap.
 * Everytime a task is added we need to broadcast to the threads that a
 * job is available. We'll need a conditional mutex for this. So basically,
 * all the threads will be champing at the bit until a single thread is woken
 * up from a task being added. We'll probably need to set a mutex on the
 * queue so we do not read and write at the same time.
 *
 * // Thread creation
 * When the threads are created, they will all start an internal tasks which
 * will make them block until a job is queued. The thread will then wait
 * on a conditional variable that gets signaled when a job is added. When
 * the conditional variable is signaled a single thread will get woken up to
 * check the job queue again. The thread will then process the tasks until
 * it is complete.
 */

typedef struct worker_t
{
    int id;
    pthread_t thread;
    thpool_t * thpool;
} worker_t;

struct thpool_t
{
    uint8_t thread_count;
    worker_t ** workers;
};

static void * do_work(worker_t * worker);

thpool_t * thpool_init(uint8_t thread_count)
{
    // Return NULL if 0 was passed in
    if (0 == thread_count)
    {
        debug_print("%s", "Attempted to allocate thpool with 0 threads\n");
        return NULL;
    }

    thpool_t * thpool = (thpool_t *)malloc(sizeof(thpool_t));
    if (UV_INVALID_ALLOC == verify_alloc(thpool))
    {
        return NULL;
    }
    thpool->thread_count = thread_count;

    // Create array of worker objects
    thpool->workers = (worker_t **)calloc(thread_count, sizeof(worker_t *));
    if (UV_INVALID_ALLOC == verify_alloc(thpool->workers))
    {
        free(thpool);
        return NULL;
    }

    // Create the individual thread objects from the pthread api
    for (uint8_t i = 0; i < thread_count; i++)
    {
        thpool->workers[i] = (worker_t *)malloc(sizeof(worker_t));
        worker_t * worker = thpool->workers[i];
        if (UV_INVALID_ALLOC == verify_alloc(worker))
        {
            free(thpool->workers);
            free(thpool);
            return NULL;
        }

        worker->thpool = thpool;
        worker->id = i;
        pthread_create(&(worker->thread), NULL,(void * (*)(void *))do_work, worker);
	    pthread_detach(worker->thread);
    }

    return thpool;
}

void thpool_destroy(thpool_t * thpool)
{
    for (uint8_t i = 0; i < thpool->thread_count; i++)
    {
        free(thpool->workers[i]);
    }
    free(thpool->workers);
    free(thpool);
}



thpool_status thpool_queue_job(thpool_t * thpool, void (* job_callback)(void *), void * job_args);

static void * do_work(worker_t * worker)
{
    printf("Worker did some worker babyyyyy from id %d\n", worker->id);
    return NULL;
}
