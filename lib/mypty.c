//#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
//#define __USE_BSD #define _BSD_SOURCE 
#include <termios.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <string.h>
//#include <strings.h>
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

int main(int ac, char *av[])
{
	int fdm, fds;
	int rc;
	char input[150];
	// Check arguments
	fdm = posix_openpt(O_RDWR);
	if (fdm < 0)
	{
		fprintf(stderr, "Error %d on posix_openpt()\n", errno);
		return 1;
	}

	rc = grantpt(fdm);
	if (rc != 0)
	{
		fprintf(stderr, "Error %d on grantpt()\n", errno);
		return 1;
	}

	rc = unlockpt(fdm);
	if (rc != 0)
	{
		fprintf(stderr, "Error %d on unlockpt()\n", errno);
		return 1;
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

		while (1)
		{
			// Wait for data from standard input and master side of PTY
			FD_ZERO(&fd_in);
			FD_SET(0, &fd_in);
			FD_SET(fdm, &fd_in);

			rc = select(fdm + 1, &fd_in, NULL, NULL, NULL);
			switch(rc)
			{
				case -1 : fprintf(stderr, "Error %d on select()\n", errno);
				exit(1);

				default :
				{
					// If data on standard input
					if (FD_ISSET(0, &fd_in))
					{
						rc = read(0, input, sizeof(input));
						if (rc > 0)
						{
							// Send data on the master side of PTY
							//printf("DATA sent\n");
							write(fdm, input, rc);
						}
						else
						{
							if (rc < 0)
							{
								fprintf(stderr, "Error %d on read standard input\n", errno);
								exit(1);
							}
						}
					}

					// If data on master side of PTY
					if (FD_ISSET(fdm, &fd_in))
					{
						rc = read(fdm, input, sizeof(input));
						if (rc > 0)
						{
							// Send data on standard output
							write(1, input, rc);
						}
						else
						{
							if (rc <= 0)
							{
								fprintf(stderr, "Error %d on read master PTY\n", errno);
								exit(1);
							}
						}
					}
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
		rc = tcgetattr(fds, &slave_orig_term_settings);

		// Set RAW mode on slave side of PTY
		new_term_settings = slave_orig_term_settings;
		cfmakeraw (&new_term_settings);
		tcsetattr (fds, TCSANOW, &new_term_settings);

		// The slave side of the PTY becomes the standard input and outputs of the child process
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
		
		system("/bin/bash");
		/*fd_set child_fds;
		char cinput[150];
		while(1){
			//printf("Wait for data\n");
			FD_ZERO(&child_fds);
			FD_SET(0, &child_fds);
			rc = select(1, &child_fds,NULL, NULL, NULL);
			if(rc == -1)
			{
				break;
			}
			else
			{
				if (FD_ISSET(0, &child_fds))
				{
					rc = read_buf(0, cinput, sizeof(cinput));
					if (rc > 0)
					{
						system(cinput);
					} else if(rc < 0) break;
				}
			}
		}*/
		
		// if Error...
		_exit(1);
	}

	return 0;
}