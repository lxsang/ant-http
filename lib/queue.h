// code based on : https://github.com/majek/dump/blob/master/msqueue/queue.h

#ifndef QUEUE_H
#define QUEUE_H

struct queue_root;

struct queue_head {
    void* data;
	struct queue_head *next;
};

struct queue_root *ALLOC_QUEUE_ROOT();
void INIT_QUEUE_HEAD(struct queue_head *head);

void queue_put(struct queue_head *,struct queue_root *);
int queue_readable(struct queue_root *);
void queue_empty(struct queue_root *, void (*)(void*) );
struct queue_head *queue_get(struct queue_root *root);

#endif // QUEUE_H
