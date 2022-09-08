#include <thread_pool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>


// Worker objects is a struct containing a pointer to the thpool and the
// thread itself
typedef struct worker_t
{
    int id;
    thrd_t thread;
    thpool_t * thpool;
} worker_t;

// A job is a queued up object containing the function that a woken thread
// should perform.
typedef struct job_t job_t;
struct job_t
{
    job_t * next_job;
    void (* job_function)(void * job_arg);
	void * job_arg;
};

// The work queue contains a list of first come, first served jobs that are
// consumed by the thread pool. When the queue is empty, the threads will
// block until a new job is enqueued.
typedef struct work_queue_t
{
    job_t * job_head;
    job_t * job_tail;
    uint32_t job_count;
    mtx_t queue_access_mutex;
} work_queue_t;

// The main structure contains pointers to the mutexes and atomic variables
// that maintain synchronization between all the threads
struct thpool_t
{
    uint8_t thread_count;
    atomic_uint_fast8_t workers_alive;
    atomic_uint_fast8_t workers_working;

    worker_t ** workers;
    work_queue_t * work_queue;
    mtx_t run_mutex;
    cnd_t run_cond;
};

// Flag is used to keep the pool while loops running. This value is
// set to 0 when destroy is called
static volatile atomic_bool thpool_active = 0;

// Atomic flag is used to indicate when there is an available job to be
// consumed by a thread
static volatile atomic_bool thpool_job_available = 0;

static void * do_work(worker_t * worker);

/*!
 * @brief Initialize the threadpool object and spawns the number of threads
 * specified. The threads will begin to execute their main function and block
 * until a new task is enqueued into the job queue. As soon as a job is
 * enqueued, a single thread will be woken up to consume the job. When the job
 * is complete, the thread will return back to blocking with the others until
 * another job is enqueued.
 *
 * Note that this function will block until all threads have been initialized
 *
 * @param thread_count Number of threads to spawn for the threadpool
 * @return Pointer to the threadpool object or NULL
 */
thpool_t * thpool_init(uint8_t thread_count)
{
    // Return NULL if 0 was passed in
    if (0 == thread_count)
    {
        debug_print("%s", "Attempted to allocate thpool with 0 threads\n");
        return NULL;
    }

    /*
     * Thread pool init
     */
    thpool_t * thpool = (thpool_t *)calloc(1, sizeof(thpool_t));
    if (UV_INVALID_ALLOC == verify_alloc(thpool))
    {
        return NULL;
    }

    /*
     * Workers init
     */
    thpool->workers = (worker_t **)calloc(thread_count, sizeof(worker_t *));
    if (UV_INVALID_ALLOC == verify_alloc(thpool->workers))
    {
        free(thpool);
        return NULL;
    }

    /*
     * Threads init
     */
    int result = 0;
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
        result = thrd_create(&(worker->thread), (thrd_start_t)do_work, worker);
        if (thrd_success != result)
        {
            debug_print_err("%s", "Unable to create a thread for the thread pool\n");
            free(thpool->workers);
            free(thpool);
            return NULL;

        }
	    result = thrd_detach(worker->thread);
        if (thrd_success != result)
        {
            debug_print_err("%s", "Unable to detach thread\n");
            free(thpool->workers);
            free(thpool);
            return NULL;
        }

    }

    /*
     * Init work queue
     */
    work_queue_t * work_queue = (work_queue_t *)calloc(1, sizeof(work_queue_t));
    if (UV_INVALID_ALLOC == verify_alloc(work_queue))
    {
        thpool_destroy(thpool);
        return NULL;
    }


    /*
     * Init Mutexes and conditional variables
     */
    result = mtx_init(&thpool->run_mutex, mtx_plain);
    if (thrd_success != result)
    {
        debug_print_err("%s", "Unable to init run_mutex\n");
        thpool_destroy(thpool);
        return NULL;
    }

    result = cnd_init(&thpool->run_cond);
    if (thrd_success != result)
    {
        debug_print_err("%s", "Unable to init run_cond\n");
        thpool_destroy(thpool);
        return NULL;
    }
    result = mtx_init(&work_queue->queue_access_mutex, mtx_plain);
    if (thrd_success != result)
    {
        debug_print_err("%s", "Unable to init queue_access\n");
        thpool_destroy(thpool);
        return NULL;
    }

    /*
     * Assign vars to thpool object
     */
    thpool->work_queue = work_queue;
    thpool->thread_count = thread_count;
    thpool->workers_alive = 0;
    thpool->workers_working = 0;
    atomic_flag_test_and_set(&thpool_active);


    /*
     * Block until all threads have been initialized
     */
    while (thread_count != atomic_load(&thpool->workers_alive))
    {
        debug_print("%s", "Waiting for threads to init\n");
        sleep(1);
    }

    return thpool;
}

void thpool_destroy(thpool_t * thpool)
{
    // set the ininite loop var to 0
    atomic_flag_clear(&thpool_active);

    // update all the threads until they are done with their tasks

    while (0 != atomic_load(&thpool->workers_alive))
    {
        debug_print("Workers alive: %d\n", atomic_load(&thpool->workers_alive));
        cnd_broadcast(&thpool->run_cond);
        sleep(1);
    }

    // Free the threads
    for (uint8_t i = 0; i < thpool->thread_count; i++)
    {
        free(thpool->workers[i]);
    }

    // Free the mutexes
    mtx_destroy(&thpool->run_mutex);
    cnd_destroy(&thpool->run_cond);
    mtx_destroy(&thpool->work_queue->queue_access_mutex);

    free(thpool->work_queue);
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

    mtx_lock(&work_queue->queue_access_mutex);

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
    atomic_flag_test_and_set(&thpool_job_available);

    // Signal at least one thread that the run condition has changed
    // indicating that a new job has been added to the queue
    debug_print("%s", "Signaling of new job added to queue\n");
    cnd_signal(&thpool->run_cond);

    mtx_unlock(&work_queue->queue_access_mutex);
    return THP_SUCCESS;
}



static void * do_work(worker_t * worker)
{
    thpool_t * thpool = worker->thpool;
    atomic_fetch_add(&thpool->workers_alive, 1);

    while (1 == atomic_load(&thpool_active))
    {
        mtx_lock(&thpool->run_mutex);
        while (1 != atomic_load(&thpool_job_available))
        {
            debug_print("[!] Thread %d waiting for a job.\n[threads: %d || working: %d]\n\n",
                        worker->id, thpool->workers_alive, thpool->workers_working);
            cnd_wait(&thpool->run_cond, &thpool->run_mutex);
            if (0 == atomic_load(&thpool_active))
            {
                break;
            }
        }

        //TODO: MOve this to queue function
        atomic_flag_clear(&thpool_job_available);

        // As soon as the thread wakes up, unlock the run lock
        // We do not need it locked for operation
        mtx_unlock(&thpool->run_mutex);
        debug_print("\n[!] Thread %d has woken up!\n[threads: %d || working: %d]\n\n",
                    worker->id,
                    atomic_load(&thpool->workers_alive),
                    atomic_load(&thpool->workers_working));

        // Add a second check. This is used when the destroy function is
        // called. All threads are broadcast to wake them up to exit
        if (0 == atomic_load(&thpool_active))
        {
            break;
        }
        // Before beginning work, increment the working thread count
        atomic_fetch_add(&thpool->workers_working, 1);
        debug_print("\n[!] Thread %d starting work...\n[threads: %d || working: %d]\n\n",
                    worker->id,
                    atomic_load(&thpool->workers_alive),
                    atomic_load(&thpool->workers_working));

        uint64_t val = 0;
        while (val < 10000000)
        {
           val++;
        }

        debug_print("\n[!] Thread %d finished work...\n[threads: %d || working: %d]\n\n",
                    worker->id,
                    atomic_load(&thpool->workers_alive),
                    atomic_load(&thpool->workers_working));

        // Decrement threads working then unlock the mutex
        atomic_fetch_sub(&thpool->workers_working, 1);
    }

    debug_print("\n[!] Thread %d is exiting...\n[threads: %d || working: %d]\n\n",
                worker->id,
                atomic_load(&thpool->workers_alive),
                atomic_load(&thpool->workers_working));
    atomic_fetch_sub(&thpool->workers_alive, 1);
    return NULL;
}

