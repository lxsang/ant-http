#ifndef ANT_SCHEDULER
#define ANT_SCHEDULER

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#define antd_create_task(h, d, c, t) (antd_mktask(h, d, c, t, HEAVY))

typedef enum
{
    LIGHT,
    HEAVY
} antd_task_type_t;

typedef struct _antd_scheduler_t antd_scheduler_t;
typedef struct _antd_callback_t antd_callback_t;
typedef struct _antd_queue_item_t antd_scheduler_evt_t;
typedef struct
{
    /**
     * task id
    */
    int id;
    /**
    * creation time of a task
    */
    unsigned long stamp;
    /**
    * Last access time of
    * task data
    */
    time_t access_time;
    /**
    * the handle and callback
    */
    void *(*handle)(void *);
    antd_callback_t *callback;
    /**
     * The task events
     * each task must be binded to
     * one or more event, otherwise it will be
     * rejected by the scheduler
     * */
    antd_scheduler_evt_t* events;
    /**
    * user data if any
    */
    void *data;
    /**
    * type of a task
    * light tasks are executed directly
    * heavy tasks are delegated to workers
    */
    antd_task_type_t type;
} antd_task_t;
/*
* nit the main scheduler
*/
antd_scheduler_t *antd_scheduler_init(int, const char *stat_name);
/*
* destroy all pending task
*/
void antd_scheduler_destroy(antd_scheduler_t *);

/**
*    create a task
*    parameter:
*      - handle
*      - data
*      - callback
*      - last data access time
*/
antd_task_t *antd_mktask(void *(*handle)(void *), void *data, void *(*callback)(void *), time_t, antd_task_type_t type);

/**
* add a task
*/
void antd_scheduler_add_task(antd_scheduler_t *, antd_task_t *);

/**
* check if scheduler is busy
*/
int antd_scheduler_busy(antd_scheduler_t *);
/**
 * get scheduler status
 * */
int antd_scheduler_ok(antd_scheduler_t *scheduler);
/**
*
* wait for event
*/
void* antd_scheduler_wait(void *);

/**
 * lock the scheduler
 * */
void antd_scheduler_lock(antd_scheduler_t *);
/**
 * Get next valid task id
 * */
int antd_scheduler_next_id(antd_scheduler_t* sched, int input);
/**
 * unlock the scheduler
 * */
void antd_scheduler_unlock(antd_scheduler_t *);

/**
 * weak functions that should be overridden by the application
 * that user the scheduler as library
*/
void __attribute__((weak)) antd_scheduler_ext_statistic(int fd, void *data);
int __attribute__((weak)) antd_scheduler_validate_data(antd_task_t *task);
void __attribute__((weak)) antd_scheduler_destroy_data(void *data);
int __attribute__((weak)) antd_task_data_id(void *data);
#endif