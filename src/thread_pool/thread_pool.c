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
    atomic_uint_fast64_t job_count;
    mtx_t queue_access_mutex;
} work_queue_t;

// The main structure contains pointers to the mutexes and atomic variables
// that maintain synchronization between all the threads
struct thpool_t
{
    uint8_t thread_count;
    atomic_uint_fast8_t workers_alive;
    atomic_uint_fast8_t workers_working;
    atomic_uint_fast8_t thpool_active;

    worker_t ** workers;
    work_queue_t * work_queue;
    mtx_t run_mutex;
    cnd_t run_cond;
};

static void * do_work(worker_t * worker);
static job_t * thpool_dequeue_job(thpool_t * thpool);

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
    int result = 0;
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
    thpool->thpool_active = 1;
    thpool->workers_alive = 0;
    thpool->workers_working = 0;
    thpool->work_queue = work_queue;
    thpool->thread_count = thread_count;



    /*
     * Threads init
     */
    for (uint8_t i = 0; i < thread_count; i++)
    {
        thpool->workers[i] = (worker_t *)malloc(sizeof(worker_t));
        worker_t * worker = thpool->workers[i];
        if (UV_INVALID_ALLOC == verify_alloc(worker))
        {
            thpool_destroy(thpool);
            return NULL;
        }

        worker->thpool = thpool;
        worker->id = i;
        result = thrd_create(&(worker->thread), (thrd_start_t)do_work, worker);
        if (thrd_success != result)
        {
            debug_print_err("%s", "Unable to create a thread for the thread pool\n");
            thpool_destroy(thpool);
            return NULL;

        }
	    result = thrd_detach(worker->thread);
        if (thrd_success != result)
        {
            debug_print_err("%s", "Unable to detach thread\n");
            thpool_destroy(thpool);
            return NULL;
        }

    }



    /*
     * Block until all threads have been initialized
     */
    while (thread_count != atomic_load(&thpool->workers_alive))
    {
        debug_print("Waiting for threads to init [%d/%d]\n",
                    atomic_load(&thpool->workers_alive), thread_count);
        sleep(1);
    }

    return thpool;
}

/*!
 * @brief Free the thread pool
 * @param thpool Pointer to the threadpool object
 */
void thpool_destroy(thpool_t * thpool)
{
    // Set running bool to false and set the queue to have a value of 1
    // instructing the threads to look for work.
    atomic_fetch_sub(&thpool->thpool_active, 1);


    while (thpool->thread_count != atomic_load(&thpool->workers_alive))
    {
        debug_print("Waiting for threads to init [%d/%d]\n",
                    atomic_load(&thpool->workers_alive), thpool->thread_count);
        sleep(1);
    }

    // update all the threads until they are done with their tasks

    while (0 != atomic_load(&thpool->workers_alive))
    {
        debug_print("\n[!] Broadcasting threads to exit...\n"
                    "Workers still alive: %d\n",
                    atomic_load(&thpool->workers_alive));
        cnd_broadcast(&thpool->run_cond);
        sleep(1);
    }

    // Free any jobs left in the queue that have not been consumed
    job_t * job = thpool->work_queue->job_head;
    job_t * temp;
    while (NULL != job)
    {
        temp = job->next_job;
        free(job);
        job = temp;
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
    job->next_job = NULL;
    work_queue_t * work_queue = thpool->work_queue;

    mtx_lock(&work_queue->queue_access_mutex);

    // If job queue is empty then assign the new job as the head and tail
    if (0 == atomic_load(&work_queue->job_count))
    {
        work_queue->job_head = job;
        work_queue->job_tail = job;
    }
    else // If work queue HAS jobs already
    {
        work_queue->job_tail->next_job = job;
        work_queue->job_tail = job;
    }
    atomic_fetch_add(&work_queue->job_count, 1);

    // Signal at least one thread that the run condition has changed
    // indicating that a new job has been added to the queue
    debug_print("[!] New job enqueued. Total jobs in queue: %ld\n",
                atomic_load(&work_queue->job_count));
    cnd_signal(&thpool->run_cond);

    mtx_unlock(&work_queue->queue_access_mutex);
    return THP_SUCCESS;
}


static job_t * thpool_dequeue_job(thpool_t * thpool)
{
    mtx_lock(&thpool->work_queue->queue_access_mutex);
    work_queue_t * work_queue = thpool->work_queue;

    job_t * work = work_queue->job_head;

    // If there is only one job queued, then prep the queue to point to "none"
    if (1 == atomic_load(&work_queue->job_count))
    {
        work_queue->job_head = NULL;
        work_queue->job_tail = NULL;
        atomic_fetch_sub(&work_queue->job_count, 1);
    }

    // Else if the queue has more than on task left, then update head to
    // point to the next item then signal the threads that work is available
    else if (atomic_load(&work_queue->job_count) > 1)
    {
        work_queue->job_head = work->next_job;
        atomic_fetch_sub(&work_queue->job_count, 1);
        cnd_signal(&thpool->run_cond);
    }

    mtx_unlock(&work_queue->queue_access_mutex);
    return work;
}

static void * do_work(worker_t * worker)
{
    // Increment the number of threads alive. This is useful to indicate that
    // a thread has successfully init
    atomic_fetch_add(&worker->thpool->workers_alive, 1);
    thpool_t * thpool = worker->thpool;

    while (1 == atomic_load(&thpool->thpool_active))
    {
        mtx_lock(&thpool->run_mutex);

        // Block while the work queue is empty
        while ((0 == atomic_load(&thpool->work_queue->job_count)) && (1 == atomic_load(&thpool->thpool_active)))
        {
            debug_print("[!] Thread %d waiting for a job.\n[threads: %d || working: %d]\n\n",
                        worker->id, thpool->workers_alive, thpool->workers_working);
            cnd_wait(&thpool->run_cond, &thpool->run_mutex);

            // As soon as the thread wakes up, unlock the run lock
            // We do not need it locked for operation
            mtx_unlock(&thpool->run_mutex);
        }

        // Second check to make sure that the woken up thread should
        // execute work logic
        if (0 == atomic_load(&thpool->thpool_active))
        {
            break;
        }

        // Before beginning work, increment the working thread count
        atomic_fetch_add(&thpool->workers_working, 1);
        debug_print("\n[!] Thread %d activated, starting work...\n"
                    "[threads: %d || working: %d]\n\n",
                    worker->id,
                    atomic_load(&thpool->workers_alive),
                    atomic_load(&thpool->workers_working));

        /*
         * Fetch a job and execute it
         */
        job_t * job = thpool_dequeue_job(thpool);
        if (NULL != job)
        {
            job->job_function(job->job_arg);
            free(job);
        }

        debug_print("\n[!] Thread %d finished work...\n[threads: %d || working: %d]\n\n",
                    worker->id,
                    atomic_load(&thpool->workers_alive),
                    atomic_load(&thpool->workers_working));

        // Decrement threads working before going back to blocking
        atomic_fetch_sub(&thpool->workers_working, 1);
    }

    debug_print("\n[!] Thread %d is exiting...\n[threads: %d || working: %d]\n\n",
                worker->id,
                atomic_load(&thpool->workers_alive),
                atomic_load(&thpool->workers_working));
    atomic_fetch_sub(&thpool->workers_alive, 1);
    return NULL;
}

