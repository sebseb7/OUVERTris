#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
/* port we're listening on */
#define PORT 2020

static fd_set master;
static int fdmax;
static int listener;

void tcpinit(void)
{
	/* server address */
	struct sockaddr_in serveraddr;
	/* for setsockopt() SO_REUSEADDR, below */
	int yes = 1;
	/* clear the master and temp sets */
	FD_ZERO(&master);

	/* get the listener */
	if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Server-socket() error lol!");
		/*just exit lol!*/
		exit(1);
	}
	/*"address already in use" error message */
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("Server-setsockopt() error lol!");
		exit(1);
	}

	/* bind */
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	memset(&(serveraddr.sin_zero), '\0', 8);

	if(bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
	{
		perror("Server-bind() error lol!");
		exit(1);
	}

	/* listen */
	if(listen(listener, 10) == -1)
	{
		perror("Server-listen() error lol!");
		exit(1);
	}

	/* add the listener to the master set */
	FD_SET(listener, &master);
	/* keep track of the biggest file descriptor */
	fdmax = listener; /* so far, it's this one*/

}

int * tcphandle(void)
{
	static fd_set read_fds;
	FD_ZERO(&read_fds);
	/* newly accept()ed socket descriptor */
	int newfd;
	/* client address */
	struct sockaddr_in clientaddr;
	int nbytes;
	/* buffer for client data */
	char buf[1024];
	
	/* copy it */
	read_fds = master;

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if(select(fdmax+1, &read_fds, NULL, NULL, &tv) == -1)
	{
		perror("Server-select() error lol!");
		exit(1);
	}

	/*run through the existing connections looking for data to be read*/
	for(int i = 0; i <= fdmax; i++)
	{
		if(FD_ISSET(i, &read_fds))
		{ /* we got one... */
			if(i == listener)
			{
				/* handle new connections */
				socklen_t addrlen = sizeof(clientaddr);
				if((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
				{
					perror("Server-accept() error lol!");
				}
				else
				{

					FD_SET(newfd, &master); /* add to master set */
					if(newfd > fdmax)
					{ /* keep track of the maximum */
						fdmax = newfd;
					}
					printf("New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
				}
			}
			else
			{
				/* handle data from a client */
				if((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0)
				{
					/* got error or connection closed by client */
					if(nbytes == 0)
						/* connection closed */
						printf("socket %d hung up\n", i);

					else
						perror("recv() error lol!");

					/* close it... */
					close(i);
					/* remove from master set */
					FD_CLR(i, &master);
				}
				else
				{
					if(send(i, "ok\n", 3, 0) == -1)
						perror("send() error lol!");
				
					static int  r[3];

					if(nbytes > 2)
					{
						r[0]=buf[0];
						r[1]=buf[1];
						r[2]=buf[2];
					}
					
					return r;
				}
			}
		}
	}

	return NULL;
}


