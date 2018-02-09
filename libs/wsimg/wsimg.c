#include <stdlib.h>
#include <string.h>
#include "../plugin.h"

void pexit()
{
	
}
void handler(int cl, const char* m, const char* rqp, dictionary rq)
{
	char* path = NULL;
	int nimg = 19;
	ws_msg_header_t* h = NULL;
	char buff[1024];
	if(ws_enable(rq))
	{
		while(1)
		{
			h = ws_read_header(cl);
			if(h)
			{
				if(h->mask == 0)
				{
					LOG("%s\n","data is not masked");
					ws_close(cl, 1012);
					break;
				}
				if(h->opcode == WS_CLOSE)
				{
			    	LOG("%s\n","Websocket: connection closed");
					ws_close(cl, 1011);
					break;
				}
				else if(h->opcode == WS_TEXT)
				{
					int l;
					if((l = ws_read_data(cl,h,sizeof(buff),buff)) > 0)
					{
						
						//path = __s("%s/ws/img%d.jpg",__plugin__.htdocs,buff[0]);
						LOG("%s : %s\n", "send back data of", buff);
						//ws_f(cl,path);
						ws_t(cl,buff);
						free(path);
					}
					else
					{
						LOG("%s\n","Invalid request");
						ws_close(cl, 1011);
						break;
					}
				}

				free(h);
			}
		} 
	}
	LOG("%s\n", "EXIT Streaming..");
}