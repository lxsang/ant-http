#include "h2.h"
#include "scheduler.h"

static int antd_h2_read_frame_header(antd_client_t* cl, antd_h2_frame_header_t* frame)
{
    frame->length = 0;
    frame->type = 0;
    frame->flags = 0;
    frame->identifier= 0;
    uint8_t header[9];
    if( antd_recv(cl,& header,sizeof(header)) != sizeof(header)) return 0;
    // network byte order is big endian
    // read frame length
    frame->length = (*header << 16) + (*(header + 1)<< 8) + *(header+2);
    // frame type
    frame->type = *(header + 3);
    // frame flags
    frame->flags = *(header + 4);
    // frame identifier
    frame->identifier = ((*(header + 5) & 0x7F) << 24) + (*(header + 6)<< 16) + (*(header + 7)<< 8) + *(header + 8);
    
    return 1;
}


static int process_frame(void* source, antd_h2_frame_header_t* frame_h)
{
     // verify frame
    printf("Frame type: %d\n", frame_h->type & 0xff);
    printf("Frame flag: %d\n",frame_h->flags);
    printf("frame identifier: %d\n", frame_h->identifier);
    uint8_t* frame_data = (uint8_t*)malloc(frame_h->length);
    if(!frame_data)
    {
        return 0;
    }
    antd_request_t* rq = (antd_request_t*) source;
    if(antd_recv(rq->client,frame_data,frame_h->length) != frame_h->length)
    {
        // TODO error
        // go away
        ERROR("Cannot read all frame data");
        free(frame_data);
        return H2_NO_ERROR;
    }

    free(frame_data);
    return H2_NO_ERROR;
}

void* antd_h2_preface_ck(void* data)
{
    char buf[25];
    antd_h2_frame_header_t frame_h;
    antd_request_t* rq = (antd_request_t*) data;
    int count = antd_recv(rq->client,buf,24);
    if(count != 24)
    {
        // TODO servers MUST treat an invalid connection preface as a
        // connection error (Section 5.4.1) of type PROTOCOL_ERROR
        ERROR("Unable to read preface for client %d: [%s]",rq->client->sock,buf);
        return antd_empty_task((void *)rq,rq->client->last_io);
    }
    buf[24] = '\0';
    if(strcmp(buf, H2_CONN_PREFACE) != 0)
    {
        ERROR("Connection preface is not correct for client %d: [%s]",rq->client->sock,buf);
        // TODO servers MUST treat an invalid connection preface as a
        // connection error (Section 5.4.1) of type PROTOCOL_ERROR
        return antd_empty_task((void *)rq, rq->client->last_io);
    }
    // read the setting frame
    if(!antd_h2_read_frame_header(rq->client, &frame_h) || frame_h.type != H2_FRM_SETTINGS)
    {
        // TODO: frame error
        // 
        // send go away with PROTOCOL_ERROR
        printf("error reading setting frame\n");
        ERROR("Unable to read setting frame from client %d",rq->client->sock);
        return antd_empty_task((void *)rq, rq->client->last_io);
    }
    process_frame(rq, &frame_h);
    return antd_create_task(antd_h2_handle, (void *)rq, NULL, rq->client->last_io);
}

void* antd_h2_handle(void* data)
{
    antd_request_t* rq = (antd_request_t*) data;
    antd_task_t* task;
    if(rq->client->flags & CLIENT_FL_READABLE)
    {
        antd_h2_read(data);
    }
    if(rq->client->flags & CLIENT_FL_WRITABLE)
    {
        antd_h2_write(data);
    }

    task = antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
    task->priority++;
    return task;
}




void* antd_h2_read(void* data)
{
    antd_h2_frame_header_t frame_h;
    antd_request_t* rq = (antd_request_t*) data;
    if(!antd_h2_read_frame_header(rq->client, &frame_h))
    {
        // TODO: frame error
        // send goaway frame
        ERROR("Unable to read frame from client %d",rq->client->sock);
        return antd_empty_task(data, rq->client->last_io);
    }
    process_frame(data, &frame_h);
    return  antd_empty_task(data, rq->client->last_io);
}
void* antd_h2_write(void* data)
{
    antd_request_t* rq = (antd_request_t*) data;
    printf("write task\n");
    return antd_empty_task(data, rq->client->last_io);
}