#ifndef HTTP_SERVER
#define HTTP_SERVER

void *accept_request(void *);
void *proxify(void *data);
void *resolve_request(void *data);
void *finish_request(void *data);
#endif