
#include <netinet/in.h>
#include "h2.h"
#include "scheduler.h"

struct antd_h2_stream_list_t{
    struct antd_h2_stream_list_t* next;
    antd_h2_stream_t* stream;
};

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
    memcpy( ((uint8_t*)(&frame->length)) + 1,header,3);
    frame->length = ntohl(frame->length);
    // frame type
    frame->type = *(header + 3);
    // frame flags
    frame->flags = *(header + 4);
    memcpy(&frame->identifier,header+5,4);
    frame->identifier = ntohl(frame->identifier) & 0x7FFFFFFF;
    return 1;
}

static int process_setting_frame(antd_request_t* rq, antd_h2_frame_header_t* frame_h)
{
    if(frame_h->length == 0)
        return H2_NO_ERROR;
    if(frame_h->flags & H2_SETTING_ACK_FLG)
    {
        return H2_FRAME_SIZE_ERROR;
    }
    if(frame_h->identifier != 0)
    {
        return H2_PROTOCOL_ERROR;
    }
    if(frame_h->length % 6 != 0)
    {
        return H2_FRAME_SIZE_ERROR;
    }

    uint8_t* frame_data = (uint8_t*)malloc(frame_h->length);
    if(!frame_data)
    {
        return H2_INTERNAL_ERROR;
    }
    antd_h2_conn_t* conn = (antd_h2_conn_t*) dvalue(rq->request,"H2_CONNECTION");
    if(!conn)
    {
        return H2_INTERNAL_ERROR;
    }
    if(antd_recv(rq->client,frame_data,frame_h->length) != (int)frame_h->length)
    {
        ERROR("Cannot read all frame data");
        free(frame_data);
        return H2_PROTOCOL_ERROR;
    }

    // read each identifier
    uint16_t param_id;
    int param_val;
    uint8_t* ptr;
    for (size_t i = 0; i < frame_h->length / 6 ; i++)
    {
        ptr = frame_data  + i*6;
        memcpy(&param_id,ptr,2);
        param_id = ntohs(param_id);
        ptr += 2;
        memcpy(&param_val,ptr,4);
        param_val = ntohl(param_val);
        printf("id: %d val: %d\n", param_id, param_val);
        switch (param_id)
        {
        case H2_SETTINGS_HEADER_TABLE_SIZE:
            conn->settings.header_table_sz = param_val;
            break;
        
        case H2_SETTINGS_ENABLE_PUSH:
            if(param_val != 0 || param_val != 1)
            {
                free(frame_data);
                return H2_PROTOCOL_ERROR;
            }
            conn->settings.enable_push = param_val;
            break;

        case H2_SETTINGS_MAX_CONCURRENT_STREAMS:
            conn->settings.max_concurrent_streams = param_val;
            break;
        case H2_SETTINGS_INITIAL_WINDOW_SIZE:
            if(param_val > 2147483647)// 2^31-1
            {
                free(frame_data);
                return H2_FLOW_CONTROL_ERROR;
            }
            int offset = param_val - conn->settings.init_win_sz;
            conn->settings.init_win_sz = param_val;
            // this should applied only to streams with active flow-control windows
            antd_h2_update_streams_win_sz(conn->streams[0], offset);
            /*
            In addition to changing the flow-control window for streams that are
            not yet active, a SETTINGS frame can alter the initial flow-control
            window size for streams with active flow-control windows (that is,
            streams in the "open" or "half-closed (remote)" state).  When the
            value of SETTINGS_INITIAL_WINDOW_SIZE changes, a receiver MUST adjust
            the size of all stream flow-control windows that it maintains by the
            difference between the new value and the old value.
            */
            break;
        case H2_SETTINGS_MAX_FRAME_SIZE:
            if(param_val < 16384 || param_val > 16777215) // < 2^14 or > 2^24-1
            {
                free(frame_data);
                return H2_PROTOCOL_ERROR;
            }
            conn->settings.max_frame_sz = param_val;
            break;
        case H2_SETTINGS_MAX_HEADER_LIST_SIZE:
            conn->settings.max_header_list_sz = param_val;
            break;
        default:
            free(frame_data);
            return H2_IGNORED;
        }
    }
    free(frame_data);
    // send back ack setting frame
    return H2_NO_ERROR;
}

static int process_window_update_frame(antd_request_t* rq, antd_h2_frame_header_t* frame_h)
{
    int window_size_incr;
    if(antd_recv(rq->client,&window_size_incr, 4) != 4)
    {
        return H2_PROTOCOL_ERROR;
    }
    window_size_incr = ntohl(window_size_incr) & 0x7FFFFFFF;
    if(window_size_incr <1 || window_size_incr > 2147483647)
        return H2_PROTOCOL_ERROR;
    antd_h2_conn_t* conn = H2_CONN(rq);
    if(!conn)
        return H2_INTERNAL_ERROR;
    if(frame_h->identifier == 0)
    {
        conn->win_sz += window_size_incr;
    }
    else
    {
        antd_h2_stream_t* stream = antd_h2_get_stream(conn->streams,frame_h->identifier);
        if(!stream)
            return H2_INTERNAL_ERROR;
        stream->win_sz += window_size_incr;
    }
    return H2_NO_ERROR;
}

static int process_frame(void* source, antd_h2_frame_header_t* frame_h)
{
    int stat;
    switch (frame_h->type)
    {
        case H2_FRM_SETTINGS:
            stat = process_setting_frame(source, frame_h);
            break;
        case H2_FRM_WINDOW_UPDATE:
            stat = process_window_update_frame(source,frame_h);
            break;
        default:
            printf("Frame: %d, length: %d id: %d\n", frame_h->type, frame_h->length, frame_h->identifier);
            stat = H2_IGNORED;
    }
    if(stat == H2_NO_ERROR || stat == H2_IGNORED) 
        return stat;

    antd_h2_conn_t* conn = H2_CONN(source);
    if(conn)
    {
        // send error frame
        antd_h2_frame_header_t header;
        header.identifier = frame_h->identifier;
        //header.type = stat;
        header.flags = 0;
        header.length = 8;
        uint8_t error_body[8];
        int tmp = htonl(conn->last_stream_id);
        memcpy(error_body, &tmp , 4 );
        tmp = htonl(stat);
        memcpy(error_body + 4, &tmp ,4);
        header.type = H2_FRM_RST_STREAM;
        if(frame_h->identifier == 0)
            header.type = H2_FRM_GOAWAY;
        antd_h2_send_frame(source,&header,error_body);
    }
    return stat;

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
    // create a connection
    dput(rq->request,"H2_CONNECTION",antd_h2_open_conn());
    int stat = process_frame(rq, &frame_h);
    if( stat == H2_NO_ERROR || stat == H2_IGNORED)
    {
        //TODO: send back setting frame
        // and init the conn
        antd_h2_frame_header_t header;
        header.length = 0;
        header.type = H2_FRM_SETTINGS;
        header.flags = 0x1;
        header.identifier = 0;
        if(antd_h2_send_frame(rq, &header,NULL))
        {
            printf("frame sent\n");
            return antd_create_task(antd_h2_handle, (void *)rq, NULL, rq->client->last_io);
        }
        else
        {
            printf("cannot send frame\n");
            return antd_empty_task(data, rq->client->last_io);
        }  
    }
    else
    {
        printf("Error process frame %d\n", stat);
        return antd_empty_task(data, rq->client->last_io);
    }
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

    task = antd_create_task(antd_h2_handle, (void *)rq, NULL, rq->client->last_io);
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
    //printf("write task\n");
    return antd_empty_task(data, rq->client->last_io);
}

antd_h2_conn_t* antd_h2_open_conn()
{
    antd_h2_conn_t* conn = (antd_h2_conn_t*) malloc(sizeof(conn));
    if(! conn)
        return NULL;

    conn->settings.header_table_sz = 4096;
    conn->settings.enable_push = 1;
    conn->settings.max_concurrent_streams = 100;
    conn->settings.init_win_sz = 65535;
    conn->settings.max_frame_sz = 16384;
    conn->settings.max_header_list_sz = 0; //unlimited
    conn->win_sz = conn->settings.init_win_sz;
    conn->last_stream_id = 0;
    conn->streams = (antd_h2_stream_list_t*) malloc(2*sizeof(antd_h2_stream_list_t));

    return conn;
}


void antd_h2_close_conn(antd_h2_conn_t* conn)
{
    if(! conn)
        return;

    if(conn->streams)
    {
        antd_h2_close_all_streams(conn->streams[0]);
        antd_h2_close_all_streams(conn->streams[1]);
        free(conn->streams);
    }
    free(conn);
}

int antd_h2_send_frame(antd_request_t* rq, antd_h2_frame_header_t* fr_header, uint8_t* data)
{
    // send the frame header in network bytes order
    uint8_t header[9];
    int nbo_int = htonl(fr_header->length) >> 8;
    memcpy(header,&nbo_int,3);
    // type
    *(header+3) = fr_header->type;
    // flag
    *(header+4) = fr_header->flags;
    // identifier
    nbo_int = htonl(fr_header->identifier);
    memcpy(header+5, &nbo_int,4);
    if(antd_send(rq->client,header,sizeof(header)) != sizeof(header))
    {
        return 0;
    }
    if(fr_header->length == 0)
    {
        return 1;
    }
    if(data == NULL)
    {
        return 0;
    }
    // send data
    if(antd_send(rq->client,data,fr_header->length) !=(int)fr_header->length)
    {
        return 0;
    }
    return 1;
}

/*stream utilities functions*/
void antd_h2_add_stream(antd_h2_stream_list_t* streams, antd_h2_stream_t* stream)
{
    if(!stream || !streams) return;
    int idx = stream->id % 2;
    antd_h2_stream_list_t it = (antd_h2_stream_list_t) malloc(sizeof(it));
    it->next = streams[idx];
    it->stream = stream;
    streams[idx] = it;
}
antd_h2_stream_t* antd_h2_get_stream(antd_h2_stream_list_t* streams, int id)
{
    int idx = id % 2;
    antd_h2_stream_list_t it;

    for(it = streams[idx]; it != NULL; it = it->next)
    {
        if(id == it->stream->id)
            return it->stream;
    }
    return NULL;
}
void antd_h2_close_stream(antd_h2_stream_t* stream)
{
    if(!stream) return;
    if(stream->stdin) free(stream->stdin);
    if(stream->stdout) free(stream->stdout);
    free(stream);
}

void antd_h2_close_all_streams(antd_h2_stream_list_t streams)
{
    antd_h2_stream_list_t it;
    for(it = streams; it != NULL; it = it->next)
    {
        if(it->stream)
        {
            antd_h2_close_stream(it->stream);
        }
        free(it);
    }
}
void antd_h2_update_streams_win_sz(antd_h2_stream_list_t streams, int offset)
{
    antd_h2_stream_list_t it;
    for(it = streams; it != NULL; it = it->next)
    {
        if(it->stream)
        {
            it->stream->win_sz += offset;   
        }
    }
}

void antd_h2_del_stream(antd_h2_stream_list_t* streams, int id)
{
    int idx = id % 2;
    antd_h2_stream_list_t it;
    if(streams[idx] && streams[idx]->stream->id == id)
    {
        it = streams[idx];
        streams[idx] = it->next;
        antd_h2_close_stream(it->stream);
        free(it);
        return;
    }
    for(it = streams[idx]; it != NULL; it = it->next)
    {
        if(it->next!= NULL && id == it->next->stream->id)
        {
            antd_h2_stream_list_t np;
            np = it->next;
            it->next = np->next;
            np->next = NULL;
            antd_h2_close_stream(np->stream);
            free(np);
            return;
        }
    }
}