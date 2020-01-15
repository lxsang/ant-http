#include "h2.h"
#include "scheduler.h"

void* antd_h2_preface_ck(void* data)
{
    char buf[25];
    antd_request_t* rq = (antd_request_t*) data;
    int count = antd_recv(rq->client,buf,24);
    if(count != 24)
    {
        // TODO servers MUST treat an invalid connection preface as a
        // connection error (Section 5.4.1) of type PROTOCOL_ERROR
        ERROR("Unable to read preface for client %d: [%s]",rq->client->sock,buf);
        return antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    }
    buf[24] = '\0';
    if(strcmp(buf, H2_CONN_PREFACE) != 0)
    {
        ERROR("Connection preface is not correct for client %d: [%s]",rq->client->sock,buf);
        // TODO servers MUST treat an invalid connection preface as a
        // connection error (Section 5.4.1) of type PROTOCOL_ERROR
        return antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    }
    return antd_create_task(antd_h2_handle, (void *)rq, NULL, rq->client->last_io);
}

void* antd_h2_handle(void* data)
{
    antd_request_t* rq = (antd_request_t*) data;
    antd_task_t* task;
    if(rq->client->flags & CLIENT_FL_READABLE)
    {
        task = antd_create_task(antd_h2_read,(void *)rq, NULL, rq->client->last_io);
        task->priority++;
        schedule_task(task);
    }
    if(rq->client->flags & CLIENT_FL_WRITABLE)
    {
        task = antd_create_task(antd_h2_write,(void *)rq, NULL, rq->client->last_io);
        task->priority++;
        schedule_task(task);
    }

    task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    task->priority++;
    return task;
}

static int antd_h2_read_frame(antd_client_t* cl, antd_h2_frame_t* frame)
{
    uint8_t tmp;
    frame->length = 0;
    frame->type = 0;
    frame->flags = 0;
    frame->identifier= 0;
    if( antd_recv(cl,& frame->length,24) != 24) return 0;
    printf("length is %d\n", frame->length);
    // TODO:
    // Values greater than 2^14 (16,384) MUST NOT be
    // sent unless the receiver has set a larger value for
    // SETTINGS_MAX_FRAME_SIZE.
    if( antd_recv(cl,& frame->type,8) != 8) return 0;
     printf("type is %d\n", frame->type);
    if( antd_recv(cl,& frame->flags,8) != 8) return 0;
    if( antd_recv(cl,& tmp,1) != 1) return 0;
    // identifier
    if( antd_recv(cl,& frame->identifier,31) != 31) return 0;
    frame->data = (uint8_t*) malloc(frame->length);
    if(!frame->data) return 0;
    if( antd_recv(cl,frame->data, frame->length) != frame->length)
    {
        free(frame->data);
        return 0;
    }
    return 1;
}


void* antd_h2_read(void* data)
{
    antd_h2_frame_t frame;
    antd_request_t* rq = (antd_request_t*) data;
    antd_task_t* task;
    if(!antd_h2_read_frame(rq->client, &frame))
    {
        // TODO: frame error
        printf("error reading frame\n");
        ERROR("Unable to read frame from client %d",rq->client->sock);
        task = antd_create_task(NULL, (void *)rq, NULL, time(NULL));
        task->priority++;
        return task;
    }
    // verify frame
    printf("Frame type: %d\n", frame.type & 0xff);
    return antd_create_task(NULL, data, NULL, time(NULL));
}
void* antd_h2_write(void* data)
{
    printf("write task\n");
    return antd_create_task(NULL, data, NULL, time(NULL));
}