#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include "../plugin.h"

void init();
call __init__ = init;

void init()
{
	
}
void pexit()
{
	
}

int read_buf(int fd, char*buf,int size)
{
	int i = 0;
	char c = '\0';
	int n;
	while ((i < size - 1) && (c != '\n'))
	{
		n = read(fd, &c, 1);
		if (n > 0)
		{
			buf[i] = c;
			i++;
		}
		else if(n == -1) return n;
		else
			c = '\n';
	}
	buf[i] = '\0';
	return i;
}
void handler(int client, const char* m, const char* rqp, dictionary rq)
{
	textstream(client);
	int filedes[2];
	char* code = R_STR(rq, "cmd");
	if(!code) return;
	if(pipe(filedes) == -1)
	{
		perror("pipe");
		return;
	}
	pid_t pid = fork();
	if(pid == -1)
	{
		perror("folk");
		return;
	} else if(pid == 0)
	{
	    while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
	     close(filedes[1]);
	     close(filedes[0]);
	    // executecomand
		 system(code);
	     //perror("execl");
	     _exit(1);
	}
	close(filedes[1]);
	char buffer[1024];
	while (1) {
		ssize_t count = read_buf(filedes[0],buffer, sizeof(buffer));
		if (count == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				perror("read");
				return;
			}
		} else if (count == 0) {
			break;
		} else {
			__t(client,"data:%s\n",buffer);
			//handle_child_process_output(buffer, count);
		}
	}
	close(filedes[0]);
	wait(0);
	free(code);
	printf("Child process exit\n");
}