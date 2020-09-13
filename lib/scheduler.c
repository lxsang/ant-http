#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "scheduler.h"
#include "utils.h"

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

static void enqueue(antd_task_queue_t *q, antd_task_t *task)
{
    antd_task_item_t it = *q;
    while (it && it->next != NULL)
        it = it->next;
    antd_task_item_t taski = (antd_task_item_t)malloc(sizeof *taski);
    taski->task = task;
    taski->next = NULL;
    if (!it) // first task
    {
        *q = taski;
    }
    else
    {
        it->next = taski;
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

static antd_task_item_t dequeue(antd_task_queue_t *q)
{
    antd_task_item_t it = *q;
    if (it)
    {
        *q = it->next;
        it->next = NULL;
    }
    return it;
}

antd_callback_t *callback_of(void *(*callback)(void *))
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
        task->priority = task->priority + 1;
        if (task->priority > N_PRIORITY - 1)
        {
            task->priority = N_PRIORITY - 1;
        }
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
    while (it)
    {
        // first free the task
        if (it->task && it->task->callback)
            free_callback(it->task->callback);
        if (it->task)
            free(it->task);
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
        antd_task_item_t it;
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
            antd_execute_task(scheduler, it);
        }
    }
    return NULL;
}

static void *statistic(antd_scheduler_t *scheduler)
{
    fd_set fd_out;
    int ret;
    char buffer[MAX_FIFO_NAME_SZ];
    antd_task_item_t it;
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

                    for (int i = 0; i < N_PRIORITY; i++)
                    {
                        snprintf(buffer, MAX_FIFO_NAME_SZ, "#### PRIORITY: %d\n", i);
                        ret = write(scheduler->stat_fd, buffer, strlen(buffer));

                        it = scheduler->task_queue[i];
                        while (it)
                        {
                            // send statistic on task data
                            snprintf(buffer, MAX_FIFO_NAME_SZ, "---- Task created at: %lu ----\n", it->task->stamp);
                            ret = write(scheduler->stat_fd, buffer, strlen(buffer));

                            // send statistic on task data
                            snprintf(buffer, MAX_FIFO_NAME_SZ, "Access time: %lu\nn", (unsigned long)it->task->access_time);
                            ret = write(scheduler->stat_fd, buffer, strlen(buffer));

                            snprintf(buffer, MAX_FIFO_NAME_SZ, "Current time: %lu\n", (unsigned long)time(NULL));
                            ret = write(scheduler->stat_fd, buffer, strlen(buffer));

                            snprintf(buffer, MAX_FIFO_NAME_SZ, "Task type: %d\n", it->task->type);
                            ret = write(scheduler->stat_fd, buffer, strlen(buffer));

                            if (it->task->handle)
                            {
                                snprintf(buffer, MAX_FIFO_NAME_SZ, "Has handle: yes\n");
                                ret = write(scheduler->stat_fd, buffer, strlen(buffer));
                            }

                            if (it->task->callback)
                            {
                                snprintf(buffer, MAX_FIFO_NAME_SZ, "Has callback: yes\n");
                                ret = write(scheduler->stat_fd, buffer, strlen(buffer));
                            }

                            // now print all task data statistic
                            if (scheduler->stat_data_cb)
                            {
                                scheduler->stat_data_cb(scheduler->stat_fd, it->task->data);
                            }
                            it = it->next;
                        }
                    }
                    pthread_mutex_unlock(&scheduler->scheduler_lock);
                    ret = close(scheduler->stat_fd);
                    scheduler->stat_fd = -1;
                    usleep(5000);
                }
                else
                {
                    ret = write(scheduler->stat_fd, ".", 1);
                    if(ret == -1)
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

int antd_scheduler_init(antd_scheduler_t *scheduler, int n)
{
    scheduler->n_workers = n;
    scheduler->status = 1;
    scheduler->workers_queue = NULL;
    scheduler->pending_task = 0;
    scheduler->validate_data = 0;
    scheduler->destroy_data = NULL;
    scheduler->stat_fd = -1;
    //scheduler->stat_data_cb = NULL;
    //memset(scheduler->stat_fifo, 0, MAX_FIFO_NAME_SZ);
    // init semaphore
    scheduler->scheduler_sem = sem_open("scheduler", O_CREAT, 0600, 0);
    if (scheduler->scheduler_sem == SEM_FAILED)
    {
        ERROR("Cannot open semaphore for scheduler");
        return -1;
    }
    scheduler->worker_sem = sem_open("worker", O_CREAT, 0600, 0);
    if (!scheduler->worker_sem)
    {
        ERROR("Cannot open semaphore for workers");
        return -1;
    }
    // init lock
    pthread_mutex_init(&scheduler->scheduler_lock, NULL);
    pthread_mutex_init(&scheduler->worker_lock, NULL);
    pthread_mutex_init(&scheduler->pending_lock, NULL);
    for (int i = 0; i < N_PRIORITY; i++)
        scheduler->task_queue[i] = NULL;
    // create scheduler.workers
    if (n > 0)
    {
        scheduler->workers = (antd_worker_t *)malloc(n * (sizeof(antd_worker_t)));
        if (!scheduler->workers)
        {
            ERROR("Cannot allocate memory for worker");
            return -1;
        }
        for (int i = 0; i < scheduler->n_workers; i++)
        {
            scheduler->workers[i].id = -1;
            scheduler->workers[i].manager = (void *)scheduler;
            if (pthread_create(&scheduler->workers[i].tid, NULL, (void *(*)(void *))work, (void *)&scheduler->workers[i]) != 0)
            {
                ERROR("pthread_create: cannot create worker: %s", strerror(errno));
                return -1;
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
    return 0;
}
/* 
    destroy all pending task
    pthread_mutex_lock(&scheduler.queue_lock);
*/
void antd_scheduler_destroy(antd_scheduler_t *scheduler)
{
    // free all the chains
    stop(scheduler);
    LOG("Destroy remaining queue");
    for (int i = 0; i < N_PRIORITY; i++)
    {
        destroy_queue(scheduler->task_queue[i]);
    }
    destroy_queue(scheduler->workers_queue);
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
    task->callback = callback_of(callback);
    task->priority = HIGH_PRIORITY;
    task->type = HEAVY;
    //task->type = LIGHT;
    task->access_time = atime;
    return task;
}

/*
    scheduling a task
*/
void antd_add_task(antd_scheduler_t *scheduler, antd_task_t *task)
{
    // check if task is exist
    int prio = task->priority > N_PRIORITY - 1 ? N_PRIORITY - 1 : task->priority;
    //LOG("Prio is %d\n", prio);
    pthread_mutex_lock(&scheduler->scheduler_lock);
    enqueue(&scheduler->task_queue[prio], task);
    pthread_mutex_unlock(&scheduler->scheduler_lock);
    pthread_mutex_lock(&scheduler->pending_lock);
    scheduler->pending_task++;
    pthread_mutex_unlock(&scheduler->pending_lock);
    // wake up the scheduler if idle
    sem_post(scheduler->scheduler_sem);
}

void antd_execute_task(antd_scheduler_t *scheduler, antd_task_item_t taski)
{
    if (!taski)
        return;
    // execute the task
    void *ret = (*(taski->task->handle))(taski->task->data);
    // check the return data if it is a new task
    if (!ret)
    {
        // call the first callback
        execute_callback(scheduler, taski->task);
        free(taski);
    }
    else
    {
        antd_task_t *rtask = (antd_task_t *)ret;
        if (taski->task->callback)
        {
            if (rtask->callback)
            {
                enqueue_callback(rtask->callback, taski->task->callback);
            }
            else
            {
                rtask->callback = taski->task->callback;
            }
        }
        if (!rtask->handle)
        {
            // call the first callback
            execute_callback(scheduler, rtask);
            free(taski->task);
            free(taski);
        }
        else
        {
            rtask->priority = taski->task->priority + 1;
            if (rtask->priority > N_PRIORITY - 1)
            {
                rtask->priority = N_PRIORITY - 1;
            }
            antd_add_task(scheduler, rtask);
            free(taski->task);
            free(taski);
        }
    }
}

int antd_scheduler_busy(antd_scheduler_t *scheduler)
{
    return scheduler->pending_task != 0;
}

int antd_task_schedule(antd_scheduler_t *scheduler)
{
    // fetch next task from the task_queue
    antd_task_item_t it = NULL;
    pthread_mutex_lock(&scheduler->scheduler_lock);
    for (int i = 0; i < N_PRIORITY; i++)
    {

        it = dequeue(&scheduler->task_queue[i]);
        if (it)
            break;
    }
    pthread_mutex_unlock(&scheduler->scheduler_lock);
    // no task
    if (!it)
    {
        return 0;
    }
    pthread_mutex_lock(&scheduler->pending_lock);
    scheduler->pending_task--;
    pthread_mutex_unlock(&scheduler->pending_lock);
    // has the task now
    // validate the task
    if (scheduler->validate_data && difftime(time(NULL), it->task->access_time) > MAX_VALIDITY_INTERVAL && it->task->priority == N_PRIORITY - 1)
    {
        // data task is not valid
        LOG("Task is no longer valid and will be killed");
        if (scheduler->destroy_data)
            scheduler->destroy_data(it->task->data);
        if (it->task->callback)
            free_callback(it->task->callback);
        free(it->task);
        free(it);
        return 0;
    }

    // check the type of task
    if (it->task->type == LIGHT || scheduler->n_workers <= 0)
    {
        // do it by myself
        antd_execute_task(scheduler, it);
    }
    else
    {
        // delegate to other workers by
        //pushing to the worker queue
        pthread_mutex_lock(&scheduler->worker_lock);
        enqueue(&scheduler->workers_queue, it->task);
        pthread_mutex_unlock(&scheduler->worker_lock);
        // wake up idle worker
        sem_post(scheduler->worker_sem);
        free(it);
    }
    return 1;
}
void antd_wait(antd_scheduler_t *scheduler)
{
    int stat;
    while (scheduler->status)
    {
        stat = antd_task_schedule(scheduler);
        if (!stat)
        {
            // no task found, go to idle state
            sem_wait(scheduler->scheduler_sem);
        }
    }
}
