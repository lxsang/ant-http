#include "../plugin.h"

#define TEXT "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum."
void pexit()
{
	
}
void handler(int cl, const char* m, const char* rqp, dictionary rq)
{
	//html(cl);
	ws_msg_header_t* h = NULL;
	if(ws_enable(rq))
	{
		printf("Doc: %s\n","Websocket is available" );
		while(1)
		{
			 h = ws_read_header(cl);
			if(h)
			{
				if(h->opcode == WS_CLOSE)
				{
					printf("WARNING: Connection close\n");
					break;
				}
				else if(h->opcode == WS_TEXT)
				{
					char buff[1025];
					int l;
					while((l = ws_read_data(cl,h,1024,buff)) != -1)
					{
						buff[l] = '\0';
						printf("Received:%d '%s'\n", l, buff);
					}
					ws_t(cl,  TEXT);
					ws_t(cl,  "test");
				}
				
				free(h);
			} 
		}
	}

	printf("Child process exit\n");
}