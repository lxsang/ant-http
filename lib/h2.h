#ifndef HTTP2_H
#define HTTP2_H

#include "handle.h"
#include "hpack.h"
#include "queue.h"

#define H2_CONN_PREFACE  "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"

#define H2_FRM_DATA             0x0
#define H2_FRM_HEADER           0x1
#define H2_FRM_PRIORITY         0x2
#define H2_FRM_RST_STREAM       0x3
#define H2_FRM_SETTINGS         0x4
#define H2_FRM_PUSH_PROMISE     0x5
#define H2_FRM_PING             0x6
#define H2_FRM_GOAWAY           0x7
#define H2_FRM_WINDOW_UPDATE    0x8
#define H2_FRM_CONTINUATION     0x9

// ERROR code
/*
The associated condition is not a result of an
error.  For example, a GOAWAY might include this code to indicate
graceful shutdown of a connection.
*/
#define H2_NO_ERROR             0x0
/*
The endpoint detected an unspecific protocol
error.  This error is for use when a more specific error code is
not available.
*/
#define H2_PROTOCOL_ERROR       0x1
/*
The endpoint encountered an unexpected
internal error.
*/
#define H2_INTERNAL_ERROR       0x2
/*
The endpoint detected that its peer
violated the flow-control protocol.
*/
#define H2_FLOW_CONTROL_ERROR   0x3
/*
The endpoint sent a SETTINGS frame but did
not receive a response in a timely manner.  See Section 6.5.3
("Settings Synchronization").
*/
#define H2_SETTINGS_TIMEOUT     0x4
/*
The endpoint received a frame after a stream
was half-closed.
*/
#define H2_STREAM_CLOSED        0x5
/*
The endpoint received a frame with an invalid size.
*/
#define H2_FRAME_SIZE_ERROR     0x6
/*
The endpoint refused the stream prior to
performing any application processing (see Section 8.1.4 for
details).
*/
#define H2_REFUSED_STREAM       0x7
/*
Used by the endpoint to indicate that the stream is no
longer needed.
*/
#define H2_CANCEL               0x8
/*
The endpoint is unable to maintain the
header compression context for the connection.
*/
#define H2_COMPRESSION_ERROR    0x9
/*
The connection established in response to a CONNECT request (Section 8.3) was reset or abnormally closed.
*/
#define H2_CONNECT_ERROR        0xa
/*
The endpoint detected that its peer is
exhibiting a behavior that might be generating excessive load.
*/
#define H2_ENHANCE_YOUR_CALM    0xb
/*
The underlying transport has properties
that do not meet minimum security requirements (see Section 9.2). 
*/
#define H2_INADEQUATE_SECURITY  0xc
/*
The endpoint requires that HTTP/1.1 be used
instead of HTTP/2.
*/
#define H2_HTTP_1_1_REQUIRED    0xd

/*
The frame should be ignore
*/
#define H2_IGNORED              0xe

/*
SETTING FRAME CONSTs
*/
/*
When set, bit 0 indicates that this frame acknowledges
receipt and application of the peer's SETTINGS frame.  When this
bit is set, the payload of the SETTINGS frame MUST be empty.
Receipt of a SETTINGS frame with the ACK flag set and a length
field value other than 0 MUST be treated as a connection error
(Section 5.4.1) of type FRAME_SIZE_ERROR
*/
#define H2_SETTING_ACK_FLG                      0x1
#define H2_SETTINGS_HEADER_TABLE_SIZE           0x1
#define H2_SETTINGS_ENABLE_PUSH                 0x2
#define H2_SETTINGS_MAX_CONCURRENT_STREAMS      0x3
#define H2_SETTINGS_INITIAL_WINDOW_SIZE         0x4
#define H2_SETTINGS_MAX_FRAME_SIZE              0x5
#define H2_SETTINGS_MAX_HEADER_LIST_SIZE        0x6


/*header flag*/
#define H2_END_STREAM_FLG        0x1
#define H2_END_HEADERS_FLG       0x4
#define H2_PADDED_FLG            0x8
#define H2_PRIORITY_FLG          0x20
#define H2_EXCLUSIVE_FLG         0x40



// stream state
typedef enum {
    H2_STR_IDLE,
    H2_STR_OPEN,
    H2_STR_REV_LOC,
    H2_STR_REV_REM,
    H2_STR_HALF_CLOSED_LOC,
    H2_STR_HALF_CLOSED_REM,
    H2_STR_CLOSED,
    H2_STR_FINALIZED
} antd_h2_stream_state_t;

typedef struct{
    uint32_t header_table_sz;
    uint32_t enable_push;
    uint32_t max_concurrent_streams;
    uint32_t init_win_sz;
    uint32_t max_frame_sz;
    uint32_t max_header_list_sz;
} antd_h2_conn_setting_t;


typedef struct antd_h2_stream_list_t* antd_h2_stream_list_t;

/**
 * Struct that holds a
 * h2 connection
*/
typedef struct {
    antd_h2_conn_setting_t settings;
    antd_h2_stream_list_t* streams;
    int win_sz;
    int last_stream_id;
} antd_h2_conn_t;

#define H2_CONN(rq) ((antd_h2_conn_t*)dvalue(((antd_request_t*)rq)->request,"H2_CONNECTION"))
#define H2_SETTING(rq) (H2_CON(rq)->settings)
/**
 * Struct that holds a
 * h2 stream
*/
typedef struct {
    struct queue_root* stdin;
    struct queue_root* stdout;
    int win_sz;
    antd_h2_stream_state_t state;
    uint8_t flags;
    uint32_t dependency;
    uint8_t weight;
    int id;
} antd_h2_stream_t;

/**
 * a H2 frame header
*/
typedef struct {
    // 24 bits length
    unsigned int length;
    // 8 bits type
    uint8_t type;
    // 8 bits flags
    uint8_t flags;
    // 31 bits identifier
    unsigned int identifier;
} antd_h2_frame_header_t;

typedef struct {
    antd_h2_frame_header_t header;
    uint8_t* pageload;
} antd_h2_frame_t;


/*Frame utilities functions*/
void antd_h2_destroy_frame(antd_h2_frame_t*);

/*stream utilities functions*/
antd_h2_stream_t* antd_h2_init_stream(int id, int wsz);
void antd_h2_close_stream(antd_h2_stream_t* stream);
void antd_h2_add_stream(antd_h2_stream_list_t*, antd_h2_stream_t*);
antd_h2_stream_t* antd_h2_get_stream(antd_h2_stream_list_t*, int);
void antd_h2_del_stream(antd_h2_stream_list_t*, int);
void antd_h2_close_all_streams(antd_h2_stream_list_t*);
void antd_h2_update_streams_win_sz(antd_h2_stream_list_t streams, int offset);
void h2_stream_io_put(antd_h2_stream_t*, antd_h2_frame_t*);
antd_h2_frame_t* antd_h2_streamio_get(struct queue_root*);
antd_request_t* antd_h2_request_init(antd_request_t*, antd_h2_stream_t*);


/*Connection utilities funtions*/
antd_h2_conn_t* antd_h2_open_conn();
void antd_h2_close_conn(antd_h2_conn_t*);

int antd_h2_read(void* rq);
int antd_h2_write(void* rq);
void* antd_h2_preface_ck(void* rq);
void* antd_h2_handle(void* rq);
int antd_h2_send_frame(antd_request_t*, antd_h2_frame_header_t*, uint8_t*);
#endif