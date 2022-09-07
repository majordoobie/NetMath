#include <thread_pool.h>
#include <stdlib.h>
#include <bits/types/sig_atomic_t.h>
#include <unistd.h>

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

typedef struct job_t job_t;
struct job_t
{
    job_t * next_job;
    void (* job_function)(void * job_arg);
	void * job_arg;
};

typedef struct work_queue_t
{
    job_t * job_head;
    job_t * job_tail;
    uint32_t job_count;
    pthread_mutex_t queue_access_mutex;
} work_queue_t;

struct thpool_t
{
    uint8_t thread_count;
    volatile sig_atomic_t workers_alive;
    worker_t ** workers;
    work_queue_t * work_queue;
    pthread_mutex_t run_mutex;
    pthread_cond_t run_cond;
};

// Flag is used to keep the pool while loops running. This value is
// set to 0 when destroy is called
static volatile sig_atomic_t thpool_active = 1;

// Atomic flag is used to indicate when there is an available job to be
// consumed by a thread
static volatile sig_atomic_t thpool_job_available = 0;

static void * do_work(worker_t * worker);

thpool_t * thpool_init(uint8_t thread_count)
{
    // Return NULL if 0 was passed in
    if (0 == thread_count)
    {
        debug_print("%s", "Attempted to allocate thpool with 0 threads\n");
        return NULL;
    }

    // Create the thpool object
    thpool_t * thpool = (thpool_t *)malloc(sizeof(thpool_t));
    if (UV_INVALID_ALLOC == verify_alloc(thpool))
    {
        return NULL;
    }
    thpool->thread_count = thread_count;
    thpool->workers_alive = 0;

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

    // create the job queue structure
    work_queue_t * work_queue = (work_queue_t *)calloc(1, sizeof(work_queue_t));
    if (UV_INVALID_ALLOC == verify_alloc(work_queue))
    {
        thpool_destroy(thpool);
        return NULL;
    }
    thpool->work_queue = work_queue;

    pthread_mutex_init(&thpool->run_mutex, NULL);
    pthread_cond_init(&thpool->run_cond, NULL);
    pthread_mutex_init(&thpool->work_queue->queue_access_mutex, NULL);

    while (thpool->workers_alive != thread_count)
    {
        debug_print("%s", "Waiting for threads to init\n");
        sleep(1);
    }

    fflush(stderr);
    return thpool;
}

void thpool_destroy(thpool_t * thpool)
{
    // set the ininite loop var to 0
    thpool_active = 0;

    // update all the threads until they are done with their tasks
    while (0 != thpool->workers_alive)
    {
        debug_print("%s%d%s", "Workers alive: ", thpool->workers_alive, "\n");
        pthread_cond_broadcast(&thpool->run_cond);
        sleep(1);
    }

    for (uint8_t i = 0; i < thpool->thread_count; i++)
    {
        free(thpool->workers[i]);
    }
    free(thpool->workers);
    free(thpool);
}



thpool_status thpool_enqueue_job(thpool_t * thpool, void (* job_function)(void *), void * job_arg)
{
    job_t * job = (job_t *)malloc(sizeof(job_t));
    if (UV_INVALID_ALLOC == verify_alloc(job))
    {
        return THP_FAILURE;
    }

    job->job_arg = job_arg;
    job->job_function = job_function;
    work_queue_t * work_queue = thpool->work_queue;

    pthread_mutex_lock(&work_queue->queue_access_mutex);

    // If job queue is empty then assign the new job as the head and tail
    if (0 == work_queue->job_count)
    {
        work_queue->job_head = job;
        work_queue->job_tail = job;
    }
    else // If work queue HAS jobs already
    {
        work_queue->job_tail->next_job = job;
        work_queue->job_tail = job;
    }
    work_queue->job_count++;
    thpool_job_available = 1;

    // Signal at least one thread that the run condition has changed
    // indicating that a new job has been added to the queue
    debug_print("%s", "Signaling of new job added to queue\n");
    pthread_cond_signal(&thpool->run_cond);

    pthread_mutex_unlock(&work_queue->queue_access_mutex);
    return THP_SUCCESS;
}



static void * do_work(worker_t * worker)
{
    thpool_t * thpool = worker->thpool;

    while (1 == thpool_active)
    {
        pthread_mutex_lock(&thpool->run_mutex);
        thpool->workers_alive = thpool->workers_alive + 1;
        while ((0 == thpool_job_available) && (1 == thpool_active))
        {
            debug_print("%s%d%s", "Thread ", worker->id, " is waiting for job\n");
            pthread_cond_wait(&thpool->run_cond, &thpool->run_mutex);
        }
        debug_print("%s%d%s", "Thread ", worker->id, " Caught wakeup signal\n");
        sleep(1);
        thpool_job_available = 0;
        // Add a second check. This is used when the destroy function is
        // called. All threads are broadcast to wake them up to exit
        if (0 == thpool_active)
        {
            debug_print("%s%d%s", "Thread ", worker->id, " woke up to process thread\n");
        }
        pthread_mutex_unlock(&thpool->run_mutex);
    }

    debug_print("%s%d%s", "Thread ", worker->id, " is exiting\n");
    thpool->workers_alive = thpool->workers_alive - 1;
    return NULL;
}

