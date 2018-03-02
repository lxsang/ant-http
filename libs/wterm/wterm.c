#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <string.h>
#include "../plugin.h"

void pexit()
{
	
}
void handler(void* cl, const char* m, const char* rqp, dictionary rq)
{
	ws_msg_header_t* h = NULL;
	if(ws_enable(rq))
	{
		
		int fdm, fds;
		int rc;
		char buff[1024];
		
		// Check arguments
		fdm = posix_openpt(O_RDWR);
		if (fdm < 0)
		{
			LOG("Error %d on posix_openpt()\n", errno);
			ws_close(cl, 1011);
			return ;
		}

		rc = grantpt(fdm);
		if (rc != 0)
		{
			LOG("Error %d on grantpt()\n", errno);
			ws_close(cl, 1011);
			return ;
		}

		rc = unlockpt(fdm);
		if (rc != 0)
		{
			LOG( "Error %d on unlockpt()\n", errno);
			ws_close(cl, 1011);
			return ;
		}

		// Open the slave side ot the PTY
		fds = open(ptsname(fdm), O_RDWR);

		// Create the child process
		if (fork())
		{
			fd_set fd_in;

			// FATHER

			// Close the slave side of the PTY
			close(fds);
			int max_fdm;
			int cl_fd = ((antd_client_t*)cl)->sock;
			while (1)
			{	
				FD_ZERO(&fd_in);
				//FD_SET(0, &fd_in);
				FD_SET(fdm, &fd_in);
				FD_SET(cl_fd,&fd_in);
				max_fdm = fdm>cl_fd?fdm:cl_fd;
				rc = select(max_fdm + 1, &fd_in, NULL, NULL, NULL);
				switch(rc)
				{
					case -1 : 
						LOG("Error %d on select()\n", errno);
						ws_close(cl, 1011);
						return;

					default :
					{
	   					// If data is on websocket side
						if (FD_ISSET(cl_fd, &fd_in))
						{
			      			h = ws_read_header(cl);
			      			if(h)
			      			{
								if(h->mask == 0)
								{
			      					LOG("%s\n","Data is not mask");
			   						write(fdm, "exit\n", 5);
									free(h);
			      					return;
								}
			      				if(h->opcode == WS_CLOSE)
			      				{
			      					LOG("%s\n","Websocket: connection closed");
			   						write(fdm, "exit\n", 5);
									free(h);
			      					return;
			      				}
			      				else if(h->opcode == WS_TEXT)
			      				{
			      					int l;
									char * tok = NULL;
									char* tok1 = NULL;
									char* orgs = NULL;
			      					while((l = ws_read_data(cl,h,sizeof(buff),buff)) > 0)
			      					{
										char c = buff[0];
										switch(c)
										{
											case 'i': // input from user
											write(fdm, buff+1, l-1);
											break;
											
											case 's': // terminal resize event
											buff[l] = '\0';
											tok = strdup(buff+1);
											orgs = tok;
											tok1 = strsep(&tok,":");
											if(tok != NULL && tok1 != NULL)
											{
												int cols = atoi(tok1);
												int rows = atoi(tok);
												//free(tok);
												struct winsize win = { 0, 0, 0, 0 };
												if (ioctl(fdm, TIOCGWINSZ, &win) != 0) {
													if (errno != EINVAL) {
															printf("Exit now \n");
															break;
													}
													memset(&win, 0, sizeof(win));
												}
												//printf("Setting winsize\n");
												if (rows >= 0)
													win.ws_row = rows;
												if (cols >= 0)
													win.ws_col = cols;

												if (ioctl(fdm, TIOCSWINSZ, (char *) &win) != 0)
													printf("Cannot set winsize\n");
												
												free(orgs);
											}
											
											break;
											
											default:
											break;
										}
										//ws_t(cl,buff);
			      					}
									/*if(l == -1)
									{
										printf("EXIT FROM CLIENT \n");
				   						write(fdm, "exit\n", 5);
				      					return;
									}*/
			      				}

			      				free(h);
			      			} 
							else
							{
								write(fdm, "exit\n", 5);
								ws_close(cl,1000);
							} 
						}
						// If data on master side of PTY
						if (FD_ISSET(fdm, &fd_in))
						{
							//rc = read(fdm, buff, sizeof(buff));
							if ( (rc = read(fdm, buff,sizeof(buff)-1)) > 0)
							{
								// Send data to websocket
								buff[rc] = '\0';
								ws_t(cl,buff);
							} else
							{
								if (rc < 0)
								{
									LOG("Error %d on read standard input. Exit now\n", errno);
									write(fdm, "exit\n", 5);
									ws_close(cl,1011);
									return;
								}
							}
						}
						//printf("DONE\n");
					}
				} // End switch
			} // End while
		}
		else
		{
			struct termios slave_orig_term_settings; // Saved terminal settings
			struct termios new_term_settings; // Current terminal settings

			// CHILD
		
			// Close the master side of the PTY
			close(fdm);

			// Save the defaults parameters of the slave side of the PTY
			//rc = tcgetattr(fds, &slave_orig_term_settings);

			// Set RAW mode on slave side of PTY
			//new_term_settings = slave_orig_term_settings;
			//cfmakeraw (&new_term_settings);
			//tcsetattr (fds, TCSANOW, &new_term_settings);

			// The slave side of the PTY becomes the standard input and outputs of the child process
			// we use cook mode here
			close(0); // Close standard input (current terminal)
			close(1); // Close standard output (current terminal)
			close(2); // Close standard error (current terminal)

			dup(fds); // PTY becomes standard input (0)
			dup(fds); // PTY becomes standard output (1)
			dup(fds); // PTY becomes standard error (2)

			// Now the original file descriptor is useless
			close(fds);

			// Make the current process a new session leader
			setsid();

			// As the child is a session leader, set the controlling terminal to be the slave side of the PTY
			// (Mandatory for programs like the shell to make them manage correctly their outputs)
			ioctl(0, TIOCSCTTY, 1);
		
			//system("/bin/bash");
			system("TERM=linux sudo login");
			// if Error...
			ws_close(cl,1000);
			//LOG("%s\n","Terminal exit");
			_exit(1);
		}
	}

	LOG("%s\n","All processes exit");
}

/*
static void set_window_size(int rows, int cols)
{
struct winsize win = { 0, 0, 0, 0 };
if (ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
	if (errno != EINVAL) {
			goto bail;
		}
		memset(&win, 0, sizeof(win));
	}

	if (rows >= 0)
		win.ws_row = rows;
	if (cols >= 0)
		win.ws_col = cols;

	if (ioctl(STDIN_FILENO, TIOCSWINSZ, (char *) &win))
bail:
		perror_on_device("%s");
}
*/