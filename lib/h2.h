#ifndef HTTP2_H
#define HTTP2_H
#include "handle.h"
#include "hpack.h"

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

/**
 * Struct that holds a
 * h2 connection
*/
typedef struct {

} antd_h2_conn_t;

/**
 * Struct that holds a
 * h2 stream
*/
typedef struct {

} antd_h2_stream_t;

/**
 * a H2 frame
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
    uint8_t* data;
} antd_h2_frame_t;

void* antd_h2_read(void* rq);
void* antd_h2_write(void* rq);

void* antd_h2_preface_ck(void* rq);

void* antd_h2_handle(void* rq);

#endif