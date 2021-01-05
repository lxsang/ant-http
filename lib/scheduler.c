#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <poll.h>
#include "scheduler.h"
#include "utils.h"
#include "bst.h"

#define MAX_VALIDITY_INTERVAL 60 // 1minute
#define MAX_FIFO_NAME_SZ 255
#define POLL_EVENT_TO 100 // ms

// callback definition
struct _antd_callback_t
{
    void *(*handle)(void *);
    struct _antd_callback_t *next;
};

typedef struct
{
    int flags;
    int fd;
    int timeout; // seconds
    antd_task_t *task;
} antd_task_evt_item_t;

struct _antd_queue_item_t
{
    union
    {
        antd_task_evt_item_t *evt;
        antd_task_t *task;
        void *raw_ptr;
    };
    struct _antd_queue_item_t *next;
};

typedef struct _antd_queue_item_t *antd_queue_item_t;

typedef antd_queue_item_t antd_queue_t;

typedef struct
{
    int id;
    pthread_t tid;
    void *manager;
} antd_worker_t;

struct _antd_scheduler_t
{
    // data lock
    pthread_mutex_t scheduler_lock;
    pthread_mutex_t worker_lock;
    pthread_mutex_t pending_lock;
    // event handle
    sem_t *scheduler_sem;
    sem_t *worker_sem;
    // worker and data
    bst_node_t *task_queue;
    antd_queue_t workers_queue;
    uint8_t status; // 0 stop, 1 working
    antd_worker_t *workers;
    int n_workers;
    int pending_task;
    int id_allocator;
    char stat_fifo[MAX_FIFO_NAME_SZ];
    int stat_fd;
    pthread_t stat_tid;
};

static antd_callback_t *callback_of(void *(*callback)(void *));
static void antd_execute_task(antd_scheduler_t *, antd_task_t *);
static void antd_task_schedule(antd_scheduler_t *scheduler, antd_task_t *task);
static void destroy_task(void *data);

static void set_nonblock(int fd)
{
    int flags;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        ERROR("Unable to set flag");
    }
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static void enqueue(antd_queue_t *q, void *data)
{
    antd_queue_item_t it = *q;
    while (it && it->next != NULL)
        it = it->next;
    antd_queue_item_t new_it = (antd_queue_item_t)malloc(sizeof *new_it);
    new_it->raw_ptr = data;
    new_it->next = NULL;
    if (!it) // first task
    {
        *q = new_it;
    }
    else
    {
        it->next = new_it;
    }
}

static void stop(antd_scheduler_t *scheduler)
{
    scheduler->status = 0;
    // unlock all idle workers if any
    for (int i = 0; i < scheduler->n_workers; i++)
        sem_post(scheduler->worker_sem);
    if (scheduler->scheduler_sem)
        sem_post(scheduler->scheduler_sem);
    for (int i = 0; i < scheduler->n_workers; i++)
        if (scheduler->workers[i].id != -1)
            pthread_join(scheduler->workers[i].tid, NULL);
    if (scheduler->workers)
        free(scheduler->workers);
    (void)pthread_cancel(scheduler->stat_tid);
    // destroy all the mutex
    pthread_mutex_destroy(&scheduler->scheduler_lock);
    pthread_mutex_destroy(&scheduler->worker_lock);
    pthread_mutex_destroy(&scheduler->pending_lock);
    sem_unlink("scheduler");
    sem_unlink("worker");
    sem_close(scheduler->scheduler_sem);
    sem_close(scheduler->worker_sem);
}

static antd_queue_item_t dequeue(antd_queue_t *q)
{
    antd_queue_item_t it = *q;
    if (it)
    {
        *q = it->next;
        it->next = NULL;
    }
    return it;
}

static antd_callback_t *callback_of(void *(*callback)(void *))
{
    antd_callback_t *cb = NULL;
    if (callback)
    {
        cb = (antd_callback_t *)malloc(sizeof *cb);
        cb->handle = callback;
        cb->next = NULL;
    }
    return cb;
}

static void free_callback(antd_callback_t *cb)
{
    antd_callback_t *it = cb;
    antd_callback_t *curr;
    while (it)
    {
        curr = it;
        it = it->next;
        free(curr);
    }
}

static void enqueue_callback(antd_callback_t *cb, antd_callback_t *el)
{
    antd_callback_t *it = cb;
    while (it && it->next != NULL)
        it = it->next;
    if (!it)
        return; // this should not happend
    it->next = el;
}

static void execute_callback(antd_scheduler_t *scheduler, antd_task_t *task)
{
    antd_callback_t *cb = task->callback;
    if (cb)
    {
        // call the first come call back
        task->handle = cb->handle;
        task->callback = task->callback->next;
        free(cb);
        //antd_task_bind_event(task, 0, 0, TASK_EVT_ALWAY_ON);
        antd_scheduler_add_task(scheduler, task);
    }
    else
    {
        destroy_task(task);
    }
}

static void destroy_queue(antd_queue_t q, int is_task)
{
    antd_queue_item_t it, curr;
    it = q;
    while (it)
    {
        if (is_task)
        {
            // first free the task
            destroy_task(it->task);
        }
        else
        {
            if (it->raw_ptr)
            {
                free(it->raw_ptr);
            }
        }
        // then free the placeholder
        curr = it;
        it = it->next;
        free(curr);
    }
}
static void *work(antd_worker_t *worker)
{
    antd_scheduler_t *scheduler = (antd_scheduler_t *)worker->manager;
    while (scheduler->status)
    {
        antd_queue_item_t it;
        pthread_mutex_lock(&scheduler->worker_lock);
        it = dequeue(&scheduler->workers_queue);
        pthread_mutex_unlock(&scheduler->worker_lock);
        // execute the task
        //LOG("task executed by worker %d\n", worker->pid);
        // no task to execute, just sleep wait
        if (!it)
        {
            //LOG("Worker %d goes to idle state\n", worker->id);
            sem_wait(scheduler->worker_sem);
        }
        else
        {
            //LOG("task executed by worker %d\n", worker->id);
            antd_execute_task(scheduler, it->task);
            free(it);
        }
    }
    return NULL;
}
static void print_static_info(bst_node_t *node, void **args, int argc)
{
    if (argc != 2)
    {
        return;
    }
    int ret;
    char *buffer = args[0];
    int *fdp = args[1];
    antd_task_t *task = (antd_task_t *)node->data;
    // send statistic on task data
    snprintf(buffer, MAX_FIFO_NAME_SZ, "---- Task %d created at: %lu ----\n", task->id, task->stamp);
    ret = write(*fdp, buffer, strlen(buffer));

    // send statistic on task data
    snprintf(buffer, MAX_FIFO_NAME_SZ, "Access time: %lu\nn", (unsigned long)task->access_time);
    ret = write(*fdp, buffer, strlen(buffer));

    snprintf(buffer, MAX_FIFO_NAME_SZ, "Current time: %lu\n", (unsigned long)time(NULL));
    ret = write(*fdp, buffer, strlen(buffer));

    if (task->handle)
    {
        snprintf(buffer, MAX_FIFO_NAME_SZ, "Has handle: yes\n");
        ret = write(*fdp, buffer, strlen(buffer));
    }

    if (task->callback)
    {
        snprintf(buffer, MAX_FIFO_NAME_SZ, "Has callback: yes\n");
        ret = write(*fdp, buffer, strlen(buffer));
    }
    UNUSED(ret);
    // now print all task data statistic
    antd_scheduler_ext_statistic(*fdp, task->data);
}
static void *statistic(antd_scheduler_t *scheduler)
{
    fd_set fd_out;
    int ret;
    char buffer[MAX_FIFO_NAME_SZ];
    void *argc[2];
    while (scheduler->status)
    {
        if (scheduler->stat_fd == -1)
        {
            scheduler->stat_fd = open(scheduler->stat_fifo, O_WRONLY);
            if (scheduler->stat_fd == -1)
            {
                ERROR("Unable to open FIFO %s: %s", scheduler->stat_fifo, strerror(errno));
                return NULL;
            }
            else
            {
                set_nonblock(scheduler->stat_fd);
            }
        }
        argc[0] = buffer;
        argc[1] = &scheduler->stat_fd;
        FD_ZERO(&fd_out);
        FD_SET(scheduler->stat_fd, &fd_out);
        ret = select(scheduler->stat_fd + 1, NULL, &fd_out, NULL, NULL);
        switch (ret)
        {
        case -1:
            ERROR("Error on select(): %s\n", strerror(errno));
            close(scheduler->stat_fd);
            return NULL;

        case 0:
            break;
        // we have data
        default:
            if (FD_ISSET(scheduler->stat_fd, &fd_out))
            {
                if (scheduler->pending_task > 0)
                {
                    pthread_mutex_lock(&scheduler->scheduler_lock);
                    // write statistic data
                    snprintf(buffer, MAX_FIFO_NAME_SZ, "Pending task: %d. Detail:\n", scheduler->pending_task);
                    ret = write(scheduler->stat_fd, buffer, strlen(buffer));

                    bst_for_each(scheduler->task_queue, print_static_info, argc, 2);

                    pthread_mutex_unlock(&scheduler->scheduler_lock);
                    ret = close(scheduler->stat_fd);
                    scheduler->stat_fd = -1;
                    usleep(5000);
                }
                else
                {
                    ret = write(scheduler->stat_fd, ".", 1);
                    if (ret == -1)
                    {
                        ret = close(scheduler->stat_fd);
                        scheduler->stat_fd = -1;
                        usleep(5000);
                    }
                    else
                    {
                        ret = write(scheduler->stat_fd, "\b", 1);
                    }
                }
            }
            else
            {
                ret = close(scheduler->stat_fd);
                scheduler->stat_fd = -1;
            }
            break;
        }
        /*   else
            {
                ret = write(scheduler->stat_fd, ".", 1);
                if(ret == -1)
                {
                    ret = close(scheduler->stat_fd);
                    scheduler->stat_fd = -1;
                }
            } */
    }
    return NULL;
}

/*
    Main API methods
    init the main scheduler
*/

antd_scheduler_t *antd_scheduler_init(int n, const char *stat_name)
{
    antd_scheduler_t *scheduler = (antd_scheduler_t *)malloc(sizeof(antd_scheduler_t));
    scheduler->n_workers = n;
    scheduler->status = 1;
    scheduler->workers_queue = NULL;
    scheduler->pending_task = 0;
    scheduler->stat_fd = -1;
    scheduler->id_allocator = 0;
    (void)memset(scheduler->stat_fifo, 0, MAX_FIFO_NAME_SZ);
    if (stat_name)
    {
        (void)strncpy(scheduler->stat_fifo, stat_name, MAX_FIFO_NAME_SZ - 1);
    }
    // init semaphore
    scheduler->scheduler_sem = sem_open("scheduler", O_CREAT, 0600, 0);
    if (scheduler->scheduler_sem == SEM_FAILED)
    {
        ERROR("Cannot open semaphore for scheduler");
        free(scheduler);
        return NULL;
    }
    scheduler->worker_sem = sem_open("worker", O_CREAT, 0600, 0);
    if (!scheduler->worker_sem)
    {
        ERROR("Cannot open semaphore for workers");
        free(scheduler);
        return NULL;
    }
    // init lock
    pthread_mutex_init(&scheduler->scheduler_lock, NULL);
    pthread_mutex_init(&scheduler->worker_lock, NULL);
    pthread_mutex_init(&scheduler->pending_lock, NULL);
    scheduler->task_queue = NULL;
    // create scheduler.workers
    if (n > 0)
    {
        scheduler->workers = (antd_worker_t *)malloc(n * (sizeof(antd_worker_t)));
        if (!scheduler->workers)
        {
            ERROR("Cannot allocate memory for worker");
            free(scheduler);
            return NULL;
        }
        for (int i = 0; i < scheduler->n_workers; i++)
        {
            scheduler->workers[i].id = -1;
            scheduler->workers[i].manager = (void *)scheduler;
            if (pthread_create(&scheduler->workers[i].tid, NULL, (void *(*)(void *))work, (void *)&scheduler->workers[i]) != 0)
            {
                ERROR("pthread_create: cannot create worker: %s", strerror(errno));
                free(scheduler);
                return NULL;
            }
            else
            {
                scheduler->workers[i].id = i;
            }
        }
    }
    // delete the fifo if any
    if (scheduler->stat_fifo[0] != '\0')
    {
        LOG("Statistic fifo at: %s", scheduler->stat_fifo);
        (void)remove(scheduler->stat_fifo);
        // create the fifo file
        if (mkfifo(scheduler->stat_fifo, 0666) == -1)
        {
            ERROR("Unable to create statictis FIFO %s: %s", scheduler->stat_fifo, strerror(errno));
        }
        else
        {
            if (pthread_create(&scheduler->stat_tid, NULL, (void *(*)(void *))statistic, scheduler) != 0)
            {
                ERROR("pthread_create: cannot create statistic thread: %s", strerror(errno));
            }
        }
    }
    LOG("Antd scheduler initialized with %d worker", scheduler->n_workers);
    return scheduler;
}

static void destroy_task(void *data)
{
    antd_task_t *task = (antd_task_t *)data;
    if (!task)
        return;
    if (task->callback)
    {
        free_callback(task->callback);
        task->callback = NULL;
    }
    if (task->events)
    {
        destroy_queue(task->events, 0);
        task->events = NULL;
    }
    if (task)
        free(task);
}

/* 
    destroy all pending task
    pthread_mutex_lock(&scheduler.queue_lock);
*/
void antd_scheduler_destroy(antd_scheduler_t *scheduler)
{
    if (!scheduler)
        return;
    // free all the chains
    stop(scheduler);
    LOG("Destroy remaining queue");
    bst_free_cb(scheduler->task_queue, destroy_task);
    scheduler->task_queue = NULL;
    destroy_queue(scheduler->workers_queue, 1);
    free(scheduler);
}

/*
    create a task
*/
antd_task_t *antd_create_task(void *(*handle)(void *), void *data, void *(*callback)(void *), time_t atime)
{
    antd_task_t *task = (antd_task_t *)malloc(sizeof *task);
    task->stamp = (unsigned long)time(NULL);
    task->data = data;
    task->handle = handle;
    task->id = antd_task_data_id(data);
    task->callback = callback_of(callback);
    task->access_time = atime;
    task->events = NULL;
    return task;
}

/*
    scheduling a task
*/
void antd_scheduler_add_task(antd_scheduler_t *scheduler, antd_task_t *task)
{
    if (task->id == 0)
        task->id = antd_scheduler_next_id(scheduler, task->id);
    pthread_mutex_lock(&scheduler->scheduler_lock);
    scheduler->task_queue = bst_insert(scheduler->task_queue, task->id, (void *)task);
    pthread_mutex_unlock(&scheduler->scheduler_lock);
    pthread_mutex_lock(&scheduler->pending_lock);
    scheduler->pending_task++;
    pthread_mutex_unlock(&scheduler->pending_lock);
    // wake up the scheduler if idle
    sem_post(scheduler->scheduler_sem);
}

static void antd_execute_task(antd_scheduler_t *scheduler, antd_task_t *task)
{
    if (!task)
        return;
    // execute the task
    void *ret = (*(task->handle))(task->data);
    // check the return data if it is a new task
    if (!ret)
    {
        // call the first callback
        execute_callback(scheduler, task);
    }
    else
    {
        antd_task_t *rtask = (antd_task_t *)ret;
        if (task->callback)
        {
            if (rtask->callback)
            {
                enqueue_callback(rtask->callback, task->callback);
            }
            else
            {
                rtask->callback = task->callback;
            }
            task->callback = NULL;
        }
        if (!rtask->handle)
        {
            // call the first callback
            execute_callback(scheduler, rtask);
            destroy_task(task);
        }
        else
        {
            antd_scheduler_add_task(scheduler, rtask);
            destroy_task(task);
        }
    }
}

int antd_scheduler_busy(antd_scheduler_t *scheduler)
{
    return scheduler->pending_task != 0;
}

void antd_scheduler_lock(antd_scheduler_t *sched)
{
    pthread_mutex_lock(&sched->scheduler_lock);
}

void antd_scheduler_unlock(antd_scheduler_t *sched)
{
    pthread_mutex_unlock(&sched->scheduler_lock);
}

static void antd_task_schedule(antd_scheduler_t *scheduler, antd_task_t *task)
{
    // no task
    if (!task)
    {
        return;
    }
    pthread_mutex_lock(&scheduler->pending_lock);
    scheduler->pending_task--;
    pthread_mutex_unlock(&scheduler->pending_lock);
    // has the task now
    // validate the task
    //if (scheduler->validate_data && difftime(time(NULL), it->task->access_time) > MAX_VALIDITY_INTERVAL && it->task->priority == N_PRIORITY - 1)
    if (antd_scheduler_validate_data(task) == 0)
    {
        // data task is not valid
        LOG("Task is no longer valid and will be killed");
        antd_scheduler_destroy_data(task->data);
        destroy_task(task);
        return;
    }

    if (scheduler->n_workers <= 0)
    {
        // do it by myself
        antd_execute_task(scheduler, task);
    }
    else
    {
        // delegate to other workers by
        //pushing to the worker queue
        pthread_mutex_lock(&scheduler->worker_lock);
        enqueue(&scheduler->workers_queue, task);
        pthread_mutex_unlock(&scheduler->worker_lock);
        // wake up idle worker
        sem_post(scheduler->worker_sem);
    }
}

static void task_polls_collect(bst_node_t* node, void** argv, int argc)
{
    UNUSED(argc);
    antd_task_evt_item_t* it = (antd_task_evt_item_t*)node->data;
    struct pollfd* pfds = (struct pollfd*)argv[0];
    if(it)
    {
        pfds[node->key].fd = it->fd;
        if(it->flags & TASK_EVT_ON_READABLE)
        {
            pfds[node->key].events |= POLLIN;
        }
        if(it->flags & TASK_EVT_ON_WRITABLE)
        {
            pfds[node->key].events |= POLLOUT;
        }
    }
}

static void task_event_collect(bst_node_t* node, void** argv, int argc)
{
    UNUSED(argc);
    antd_task_t* task = (antd_task_t*) node->data;
    antd_queue_t* exec_list = (antd_queue_t*) argv[0];
    bst_node_t** poll_list = (bst_node_t**) argv[1];
    int* pollsize = (int*) argv[2];

    if(!task->events)
    {
        enqueue(exec_list, task);
        return;
    }
    antd_queue_item_t it = task->events;
    while(it)
    {
        if(it->evt->flags & TASK_EVT_ALWAY_ON)
        {
            enqueue(exec_list, task);
        }
        else if(it->evt->flags & TASK_EVT_ON_TIMEOUT)
        {
            // check if timeout
            if(difftime(time(NULL),task->stamp) > it->evt->timeout )
            {
                enqueue(exec_list, task);
            }
        }
        else
        {
            *poll_list = bst_insert(*poll_list, *pollsize, it->evt);
            *pollsize = (*pollsize)+1;
        }
        it = it->next;
    }
}

void antd_task_bind_event(antd_task_t *task, int fd, int timeout, int flags)
{
    antd_task_evt_item_t *eit = (antd_task_evt_item_t *)malloc(sizeof(antd_task_evt_item_t));
    eit->fd = fd;
    eit->timeout = timeout;
    eit->flags = flags;
    eit->task = task;
    enqueue(&task->events, eit);
}

void *antd_scheduler_wait(void *ptr)
{
    int pollsize, ready;
    void *argv[3];
    antd_queue_t exec_list = NULL;
    bst_node_t* poll_list = NULL;
    antd_queue_item_t it = NULL;
    antd_queue_item_t curr = NULL;
    antd_task_evt_item_t *eit = NULL;
    bst_node_t* node = NULL;
    struct pollfd *pfds = NULL;
    antd_scheduler_t *scheduler = (antd_scheduler_t *)ptr;
   
    while (scheduler->status)
    {
        pollsize = 0;
         argv[0] = &exec_list;
        argv[1] = &poll_list;
        argv[2] = &pollsize;
        pthread_mutex_lock(&scheduler->scheduler_lock);
        bst_for_each(scheduler->task_queue, task_event_collect, argv, 3);
        pthread_mutex_unlock(&scheduler->scheduler_lock);
        // schedule exec list first
        it = exec_list;
        while(it)
        {
            if(it->task)
            {
                
                pthread_mutex_lock(&scheduler->scheduler_lock);
                scheduler->task_queue = bst_delete(scheduler->task_queue, it->task->id);
                pthread_mutex_unlock(&scheduler->scheduler_lock);
                antd_task_schedule(scheduler, it->task);
            }
            curr = it;
            it = it->next;
            free(curr);
        }
        // Detect event on pollist
        if(pollsize > 0)
        {
            pfds = (struct pollfd*)malloc(pollsize*sizeof(struct pollfd));
            memset(pfds, 0, pollsize*sizeof(struct pollfd));
            if(pfds)
            {
                argv[0] = pfds;
                bst_for_each(poll_list,task_polls_collect, argv, 1);
                // now poll event
                ready = poll(pfds, pollsize, POLL_EVENT_TO);
                if(ready == -1)
                {
                    // this should not happends
                    ERROR("Unable to poll: %s", strerror(errno));
                    // TODO: exit ?
                }
                else if(ready > 0)
                {
                    for (int i = 0; i < pollsize; i++)
                    {
                        // find the event
                        node = bst_find(poll_list,i);
                        if(node)
                            eit = (antd_task_evt_item_t *)node->data;
                        if(eit)
                        {
                            if( ((eit->flags & TASK_EVT_ON_READABLE) && (pfds[i].revents & POLLIN))
                                || ( (eit->flags & TASK_EVT_ON_WRITABLE) && (pfds[i].revents & POLLOUT))
                            ) {
                                // event triggered schedule the task
                                pthread_mutex_lock(&scheduler->scheduler_lock);
                                scheduler->task_queue = bst_delete(scheduler->task_queue, eit->task->id);
                                pthread_mutex_unlock(&scheduler->scheduler_lock);
                                antd_task_schedule(scheduler, eit->task);
                            }
                            else if( (pfds[i].revents & POLLERR) || (pfds[i].revents & POLLHUP) ) {
                                // task is no longer available
                                ERROR("Poll: Task %d is no longer valid. Remove it", eit->task->id);
                                // remove task from task queue
                                pthread_mutex_lock(&scheduler->scheduler_lock);
                                scheduler->task_queue = bst_delete(scheduler->task_queue, eit->task->id);
                                pthread_mutex_unlock(&scheduler->scheduler_lock);
                                antd_scheduler_destroy_data(eit->task->data);
                                destroy_task(eit->task);
                            }
                        }
                    }
                    
                }
                free(pfds);
            }
        }
        exec_list = NULL;
        bst_free(poll_list);
        poll_list = NULL;

        if (!scheduler->task_queue)
        {
            // no task found, go to idle state
            sem_wait(scheduler->scheduler_sem);
        }
    }
    return NULL;
}

int antd_scheduler_ok(antd_scheduler_t *scheduler)
{
    return scheduler->status;
}
int antd_scheduler_next_id(antd_scheduler_t *sched, int input)
{
    int id = input;
    pthread_mutex_lock(&sched->scheduler_lock);
    if (id == 0)
    {
        sched->id_allocator++;
        id = sched->id_allocator;
    }

    while (bst_find(sched->task_queue, id) != NULL)
    {
        id = sched->id_allocator;
    }
    pthread_mutex_unlock(&sched->scheduler_lock);
    return id;
}
void antd_scheduler_ext_statistic(int fd, void *data)
{
    UNUSED(fd);
    UNUSED(data);
}
int antd_scheduler_validate_data(antd_task_t *task)
{
    UNUSED(task);
    return !(difftime(time(NULL), task->access_time) > MAX_VALIDITY_INTERVAL);
}
void antd_scheduler_destroy_data(void *data)
{
    UNUSED(data);
}

int antd_task_data_id(void *data)
{
    UNUSED(data);
    intptr_t ptr = (intptr_t)data;
    return (int)ptr;
}