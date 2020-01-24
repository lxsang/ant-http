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



void* antd_h2_read(void* rq);
void* antd_h2_write(void* rq);

void* antd_h2_preface_ck(void* rq);

void* antd_h2_handle(void* rq);

#endif