#ifndef ANT_SCHEDULER
#define ANT_SCHEDULER

#include "utils.h"
#include <pthread.h>
// thread pool of workers
#define N_PRIORITY 10
#define NORMAL_PRIORITY ((int)((N_PRIORITY - 1) / 2))
#define LOW_PRIORITY (N_PRIORITY - 1)
#define HIGH_PRIORITY 0

// callback definition
typedef struct __callback_t{
    void* (*handle)(void*);
    struct __callback_t * next;
} antd_callback_t;
// task definition
typedef struct {
    /*
        creation time of a task
    */
    unsigned long stamp;
    /*
        unique id
    */
    int id;
    /*
        priority from 0 to 9
        higher value is lower priority
    */
    uint8_t priority;
    /*
        the callback
    */
   void* (*handle)(void*);
   antd_callback_t* callback;
   /*
        user data if any
   */
  void * data;

} antd_task_t;

typedef struct {
    pthread_t pid;
    uint8_t status; // -1 quit, 0 available, 1 busy
} antd_worker_t;


typedef struct __task_item_t{
    antd_task_t* task;
    struct __task_item_t* next;
}* antd_task_item_t;

typedef antd_task_item_t antd_task_queue_t;

typedef struct {
    pthread_mutex_t server_lock;
    pthread_mutex_t task_lock;
    antd_task_queue_t task_queue[N_PRIORITY];
    uint8_t status; // 0 stop, 1 working
    antd_worker_t* workers;
    int n_workers;
} antd_scheduler_t;

/*
    init the main scheduler
*/
void antd_scheduler_init();
/*
    destroy all pending task
*/
void antd_scheduler_destroy();

/*
    create a task
*/
antd_task_t* antd_create_task(void* (*handle)(void*), void *data, void* (*callback)(void*));

/*
    scheduling a task
*/
void antd_add_task(antd_task_t*);

void antd_task_lock();
void antd_task_unlock();
/*
    Execute a task
*/
int antd_scheduler_status();
/*
    execute and free a task a task
*/
void antd_execute_task(antd_task_item_t);

int antd_scheduler_busy();
#endif