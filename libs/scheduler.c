#include "scheduler.h"

/*
private data & methods
*/
static antd_scheduler_t scheduler;

static void enqueue(antd_task_t* task)
{
    // check if task is exist
    int prio = task->priority>N_PRIORITY-1?N_PRIORITY-1:task->priority;
    antd_task_queue_t q = scheduler.task_queue[prio];
    antd_task_item_t it = q;
    while(it && it->task->id != task->id && it->next != NULL)
        it = it->next;
    if(it && it->task->id == task->id)
    {
        LOG("Task %d exists, ignore it\n", task->id);
        return;
    }
    antd_task_item_t taski = (antd_task_item_t)malloc(sizeof *taski);
    taski->task = task;
    taski->next = NULL;
    if(!it) // first task
    {
        scheduler.task_queue[prio] = taski;
    }
    else
    {
        it->next = taski;
    }
}

static int working()
{
    return scheduler.status;
}

static void stop()
{
    pthread_mutex_lock(&scheduler.server_lock);
    scheduler.status = 0;
    pthread_mutex_unlock(&scheduler.server_lock);
    for (int i = 0; i < scheduler.n_workers; i++)
        pthread_join(scheduler.workers[i].pid, NULL);
    if(scheduler.workers) free(scheduler.workers);
}

static antd_task_item_t dequeue(int priority)
{
    int prio = priority>N_PRIORITY-1?N_PRIORITY-1:priority;
    antd_task_item_t it = scheduler.task_queue[prio];
    if(it)
    {
        scheduler.task_queue[prio] = it->next;
    }
    return it;
}

static antd_task_item_t next_task() 
{
    antd_task_item_t it = NULL;
    pthread_mutex_lock(&scheduler.server_lock);
    for(int i = 0; i< N_PRIORITY; i++)
    {
        
        it = dequeue(i);
        if(it) break;
    }
    pthread_mutex_unlock(&scheduler.server_lock);
    return it;
}

static int available_workers()
{
    int n = 0;
    pthread_mutex_lock(&scheduler.server_lock);
    for(int i=0; i < scheduler.n_workers; i++)
        if(scheduler.workers[i].status == 0) n++;
    pthread_mutex_unlock(&scheduler.server_lock);
    return n;
}

static void work(void* data)
{
    antd_task_item_t it;
    antd_worker_t* worker = (antd_worker_t*)data;
    while(working())
    {
        pthread_mutex_lock(&scheduler.server_lock);
        worker->status = 0;
        pthread_mutex_unlock(&scheduler.server_lock);
        // fetch the next in queue
        it = next_task();
        if(!it) continue;
        //LOG("worker processing \n");
        pthread_mutex_lock(&scheduler.server_lock);
        worker->status = 1;
        pthread_mutex_unlock(&scheduler.server_lock);
        // execute the task
        antd_execute_task(it);
    }
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

static void execute_callback(antd_task_t* task)
{
    antd_callback_t* cb = task->callback;
    if(cb)
    {
        // call the first come call back
        task->handle = cb->handle;
        task->callback = task->callback->next;
        free(cb);
        antd_add_task(task);
    }
    else
    {
        free(task);
    }
}


/*
    Main API methods
    init the main scheduler
*/
void antd_scheduler_init(int n)
{
    time_t t;
    srand((unsigned) time(&t));
    scheduler.n_workers = n;
    scheduler.status = 1;
    scheduler.workers = (antd_worker_t*)malloc(n*(sizeof(antd_worker_t)));
    if(!scheduler.workers)
    {
        LOG("Cannot allocate memory for worker\n");
        exit(-1);
    }
    for(int i = 0; i < N_PRIORITY; i++) scheduler.task_queue[i] = NULL;
    // create scheduler.workers
    for(int i = 0; i < scheduler.n_workers;i++)
    {
        scheduler.workers[i].status = 0;
        if (pthread_create(&scheduler.workers[i].pid , NULL,(void *(*)(void *))work, (void*)&scheduler.workers[i]) != 0)
        {
            scheduler.workers[i].status = -1;
            perror("pthread_create: cannot create worker\n");
        }
    }
    LOG("Antd scheduler initialized with %d worker\n", scheduler.n_workers);
}
void antd_task_lock()
{
    pthread_mutex_lock(&scheduler.task_lock);
}
void antd_task_unlock()
{
    pthread_mutex_unlock(&scheduler.task_lock);
}
/* 
    destroy all pending task
*/
void antd_scheduler_destroy()
{
    // free all the chains
    antd_task_item_t it, curr;
    for(int i=0; i < N_PRIORITY; i++)
    {
        it = scheduler.task_queue[i];
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
    stop();
}

/*
    create a task
*/
antd_task_t* antd_create_task(void* (*handle)(void*), void *data, void* (*callback)(void*))
{
    antd_task_t* task = (antd_task_t*)malloc(sizeof *task);
    task->stamp = (unsigned long)time(NULL);
    task->id = rand();
    task->data = data;
    task->handle = handle;
    task->callback = callback_of(callback);
    task->priority = NORMAL_PRIORITY;
    return task;
}

/*
    scheduling a task
*/
void antd_add_task(antd_task_t* task)
{
    pthread_mutex_lock(&scheduler.server_lock);
    enqueue(task);
    pthread_mutex_unlock(&scheduler.server_lock);
}


void antd_execute_task(antd_task_item_t taski)
{
    // execute the task
    void *ret = (*(taski->task->handle))(taski->task->data);
    // check the return data if it is a new task
    if(!ret)
    {
        // call the first callback
        execute_callback(taski->task);
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
            execute_callback(rtask);
            free(taski->task);
            free(taski);
        }
        else
        {
            antd_add_task(rtask);
            free(taski->task);
            free(taski);
        }
    }
}
int antd_scheduler_busy()
{
    int ret = 0;
    if(available_workers() != scheduler.n_workers) return 1;

    pthread_mutex_lock(&scheduler.server_lock);
	for(int i = 0; i < N_PRIORITY; i++)
        if(scheduler.task_queue[i] != NULL)
        {
            ret = 1;
            break;
        }
	pthread_mutex_unlock(&scheduler.server_lock);
    return ret;
}
int antd_scheduler_status()
{
    return scheduler.status;
}