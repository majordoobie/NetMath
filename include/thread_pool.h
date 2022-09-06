#ifndef JG_NETCALC_SRC_THREAD_POOL_THREAD_POOL_H_
#define JG_NETCALC_SRC_THREAD_POOL_THREAD_POOL_H_

#ifdef __cplusplus
extern "C" {
#endif //END __cplusplus
#include <utils.h>
#include <stdint.h>
#include <pthread.h>

typedef enum
{
    THP_SUCCESS,
    THP_FAILURE
} thpool_status;

typedef struct thpool_t thpool_t;
thpool_t * thpool_init(uint8_t thread_count);
thpool_status thpool_queue_job(thpool_t * thpool, void (* job_callback)(void *), void * job_args);
void thpool_destroy(thpool_t * thpool);


#ifdef __cplusplus
}
#endif //END __cplusplus
#endif //JG_NETCALC_SRC_THREAD_POOL_THREAD_POOL_H_
