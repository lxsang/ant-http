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
	uint8_t buff[8];
	if(ws_enable(rq))
	{
		while(1)
		{
			h = ws_read_header(cl);
			if(h)
			{
				if(h->opcode == WS_CLOSE)
				{
			    	LOG("%s\n","Websocket: connection closed");
					ws_close(cl, 1011);
					break;
				}
				else if(h->opcode == WS_BIN)
				{
					int l;
					if((l = ws_read_data(cl,h,sizeof(buff),buff)) > 0)
					{
						
						path = __s("%s/ws/img%d.jpg",__plugin__.htdocs,buff[0]);
						LOG("%s : %s\n", "send back data of", path);
						ws_f(cl,path);
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