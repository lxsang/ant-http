#include "scheduler.h"

static void enqueue(antd_task_queue_t* q, antd_task_t* task)
{
    antd_task_item_t it = *q;
    while(it && it->next != NULL)
        it = it->next;
    antd_task_item_t taski = (antd_task_item_t)malloc(sizeof *taski);
    taski->task = task;
    taski->next = NULL;
    if(!it) // first task
    {
        *q = taski;
    }
    else
    {
        it->next = taski;
    }
}


static void stop(antd_scheduler_t* scheduler)
{
    scheduler->status = 0;
    for (int i = 0; i < scheduler->n_workers; i++)
        pthread_join(scheduler->workers[i], NULL);
    if(scheduler->workers) free(scheduler->workers);
    // destroy all the mutex
    pthread_mutex_destroy(&scheduler->scheduler_lock);
    pthread_mutex_destroy(&scheduler->worker_lock);
    pthread_mutex_destroy(&scheduler->pending_lock);
}

static antd_task_item_t dequeue(antd_task_queue_t* q)
{
    antd_task_item_t it = *q;
    if(it)
    {
        *q = it->next;
        it->next = NULL;
    }
    return it;
}


static antd_callback_t* callback_of( void* (*callback)(void*) )
{
    antd_callback_t* cb = NULL;
    if(callback)
    {
        cb = (antd_callback_t*)malloc(sizeof *cb);
        cb->handle = callback;
        cb->next = NULL;
    }
    return cb;
}

static void free_callback(antd_callback_t* cb)
{
    antd_callback_t* it = cb;
    antd_callback_t* curr;
    while(it)
    {
        curr = it;
        it = it->next;
        free(curr);
    }
}

static void enqueue_callback(antd_callback_t* cb, antd_callback_t* el)
{
    antd_callback_t* it = cb;
    while(it && it->next != NULL)
        it = it->next;
    if(!it) return; // this should not happend
    it->next = el;
}

static void execute_callback(antd_scheduler_t* scheduler, antd_task_t* task)
{
    antd_callback_t* cb = task->callback;
    if(cb)
    {
        // call the first come call back
        task->handle = cb->handle;
        task->callback = task->callback->next;
        free(cb);
        antd_add_task(scheduler, task);
    }
    else
    {
        free(task);
    }
}

static void destroy_queue(antd_task_queue_t q)
{
    antd_task_item_t it, curr;
    it = q;
    while(it)
    {
        // first free the task
        if(it->task && it->task->callback) free_callback(it->task->callback);
        if(it->task) free(it->task);
        // then free the placeholder
        curr = it;
        it = it->next;
        free(curr);
    }
}
static void work(antd_scheduler_t* scheduler)
{
    while(scheduler->status)
    {
        antd_task_item_t it;
        pthread_mutex_lock(&scheduler->worker_lock);
        it = dequeue(&scheduler->workers_queue);
        pthread_mutex_unlock(&scheduler->worker_lock);
        // execute the task
        //LOG("task executed by worker %d\n", worker->pid);
        // no task to execute, just sleep for 500usec
        if(!it)
        {
            struct timespec ts_sleep;
            ts_sleep.tv_sec = 0;
            ts_sleep.tv_nsec = 500000;
            nanosleep(&ts_sleep, NULL);
            continue;
        }
        antd_execute_task(scheduler, it);
    }
}

/*
    Main API methods
    init the main scheduler
*/

void antd_scheduler_init(antd_scheduler_t* scheduler, int n)
{
    scheduler->n_workers = n;
    scheduler->status = 1;
    scheduler->workers_queue = NULL;
    scheduler->pending_task = 0 ;
    // init lock
    pthread_mutex_init(&scheduler->scheduler_lock,NULL);
    pthread_mutex_init(&scheduler->worker_lock, NULL);
    pthread_mutex_init(&scheduler->pending_lock, NULL);
    for(int i = 0; i < N_PRIORITY; i++) scheduler->task_queue[i] = NULL;
    // create scheduler.workers
    if(n > 0)
    {
        scheduler->workers = (pthread_t*)malloc(n*(sizeof(pthread_t)));
        if(!scheduler->workers)
        {
            LOG("Cannot allocate memory for worker\n");
            exit(-1);
        }
        for(int i = 0; i < scheduler->n_workers;i++)
        {
            if (pthread_create(&scheduler->workers[i], NULL,(void *(*)(void *))work, (void*)scheduler) != 0)
            {
                perror("pthread_create: cannot create worker\n");
            }
        }
    }
    LOG("Antd scheduler initialized with %d worker\n", scheduler->n_workers);
}
/* 
    destroy all pending task
    pthread_mutex_lock(&scheduler.queue_lock);
*/
void antd_scheduler_destroy(antd_scheduler_t* scheduler)
{
    // free all the chains
    stop(scheduler);
    LOG("Destroy remaining queue\n");
    for(int i=0; i < N_PRIORITY; i++)
    {
        destroy_queue(scheduler->task_queue[i]);
    }
    destroy_queue(scheduler->workers_queue);
}

/*
    create a task
*/
antd_task_t* antd_create_task(void* (*handle)(void*), void *data, void* (*callback)(void*))
{
    antd_task_t* task = (antd_task_t*)malloc(sizeof *task);
    task->stamp = (unsigned long)time(NULL);
    task->data = data;
    task->handle = handle;
    task->callback = callback_of(callback);
    task->priority = NORMAL_PRIORITY;
    task->type = LIGHT;
    return task;
}

/*
    scheduling a task
*/
void antd_add_task(antd_scheduler_t* scheduler, antd_task_t* task)
{
    // check if task is exist
    int prio = task->priority>N_PRIORITY-1?N_PRIORITY-1:task->priority;
    //LOG("Prio is %d\n", prio);
    pthread_mutex_lock(&scheduler->scheduler_lock);
    enqueue(&scheduler->task_queue[prio], task);
    pthread_mutex_unlock(&scheduler->scheduler_lock);
    pthread_mutex_lock(&scheduler->pending_lock);
    scheduler->pending_task++;
    pthread_mutex_unlock(&scheduler->pending_lock);
}


void antd_execute_task(antd_scheduler_t* scheduler, antd_task_item_t taski)
{
    if(!taski)
        return;
    // execute the task
    void *ret = (*(taski->task->handle))(taski->task->data);
    // check the return data if it is a new task
    if(!ret)
    {
        // call the first callback
        execute_callback(scheduler, taski->task);
        free(taski);
    }
    else
    {
        antd_task_t* rtask = (antd_task_t*) ret;
        if(taski->task->callback)
        {   
            if(rtask->callback)
            {
                enqueue_callback(rtask->callback, taski->task->callback);
            }
            else
            {
                rtask->callback = taski->task->callback;
            }
        }
        if(!rtask->handle)
        {
            // call the first callback
            execute_callback(scheduler, rtask);
            free(taski->task);
            free(taski);
        }
        else
        {
            antd_add_task(scheduler, rtask);
            free(taski->task);
            free(taski);
        }
    }
    pthread_mutex_lock(&scheduler->pending_lock);
    scheduler->pending_task--;
    pthread_mutex_unlock(&scheduler->pending_lock);
}

int antd_scheduler_busy(antd_scheduler_t* scheduler)
{
    return scheduler->pending_task != 0;
}

int antd_task_schedule(antd_scheduler_t* scheduler)
{
    // fetch next task from the task_queue
    antd_task_item_t it = NULL;
    pthread_mutex_lock(&scheduler->scheduler_lock);
    for(int i = 0; i< N_PRIORITY; i++)
    {
        
        it = dequeue(&scheduler->task_queue[i]);
        if(it)
            break;
    }
    pthread_mutex_unlock(&scheduler->scheduler_lock);
    if(!it)
    {
        return 0;
    }
    // has the task now
    // check the type of tas
    if(it->task->type == LIGHT || scheduler->n_workers <= 0)
    {
        // do it by myself
        antd_execute_task( scheduler, it);
    }
    else
    {
        // delegate to other workers by
        //pushing to the worker queue
        pthread_mutex_lock(&scheduler->worker_lock);
        enqueue(&scheduler->workers_queue, it->task);
        pthread_mutex_unlock(&scheduler->worker_lock);
        free(it);
    }
    return 1;
}