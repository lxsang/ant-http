#ifndef ANT_SCHEDULER
#define ANT_SCHEDULER

#include "utils.h"
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#define N_PRIORITY 10
#define NORMAL_PRIORITY ((int)((N_PRIORITY - 1) / 2))
#define LOW_PRIORITY (N_PRIORITY - 1)
#define HIGH_PRIORITY 0

typedef enum
{
    LIGHT,
    HEAVY
} antd_task_type_t;
// callback definition
typedef struct __callback_t
{
    void *(*handle)(void *);
    struct __callback_t *next;
} antd_callback_t;
// task definition
typedef struct
{
    /*
        creation time of a task
    */
    unsigned long stamp;
    /*
        Last access time of
        task data
    */
    time_t access_time;
    /*
        priority from 0 to N_PRIORITY - 1
        higher value is lower priority
    */
    uint8_t priority;
    /*
        the callback
    */
    void *(*handle)(void *);
    antd_callback_t *callback;
    /*
        user data if any
   */
    void *data;
    /*
    type of a task
    light tasks are executed directly
    heavy tasks are delegated to workers
    */
    antd_task_type_t type;
} antd_task_t;

typedef struct __task_item_t
{
    antd_task_t *task;
    struct __task_item_t *next;
} * antd_task_item_t;

typedef antd_task_item_t antd_task_queue_t;

typedef struct
{
    int id;
    pthread_t tid;
    void *manager;
} antd_worker_t;

typedef struct
{
    // data lock
    pthread_mutex_t scheduler_lock;
    pthread_mutex_t worker_lock;
    pthread_mutex_t pending_lock;
    // event handle
    sem_t *scheduler_sem;
    sem_t *worker_sem;
    // worker and data
    antd_task_queue_t task_queue[N_PRIORITY];
    antd_task_queue_t workers_queue;
    uint8_t status; // 0 stop, 1 working
    antd_worker_t *workers;
    int n_workers;
    int pending_task;
    /*
    function pointer that free data in a task if
    the task is not valid
    default to NULL
    */
    void* (*destroy_data)(void*);
    int (*validate_data)(antd_task_t*);
    int (*task_ready)(antd_task_t*);
} antd_scheduler_t;

/*
    init the main scheduler
*/
void antd_scheduler_init(antd_scheduler_t *, int);
/*
    destroy all pending task
*/
void antd_scheduler_destroy(antd_scheduler_t *);

/*
    create a task
    parameter:
        - handle
        - data
        - callback
        - last data access time
*/
antd_task_t *antd_create_task(void *(*handle)(void *), void *data, void *(*callback)(void *), time_t);

/*
    add a task
*/
void antd_add_task(antd_scheduler_t *, antd_task_t *);
/*
    execute and free a task a task
*/
void antd_execute_task(antd_scheduler_t *, antd_task_item_t);
/*
    scheduler status
*/
int antd_scheduler_busy(antd_scheduler_t *);
/*
    schedule a task
*/
int antd_task_schedule(antd_scheduler_t *);
/*
wait for event
*/
void antd_scheduler_wait(antd_scheduler_t *);

antd_callback_t* callback_of( void* (*callback)(void*) );
#endif