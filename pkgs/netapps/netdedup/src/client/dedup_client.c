#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#ifdef ENABLE_PARSEC_UPTCPIP
#include <uptcp_socket.h>
#endif

#include "debug.h"

#define PORT        0x2233
#ifdef ENABLE_PARSEC_UPTCPIP
#define SERVER_IP   0x0a0a0a0c   //"10.10.10.12"
#else
#define SERVER_IP   0x7f000001   //127.0.0.1
#endif

void usage(char* prog)
{
  printf("usage: %s [-h] [ -i file]\n",prog);
  printf("-i file\t\t\tthe input file\n");
  printf("-h \t\t\thelp\n");
}

int main(int argc, char **argv)
{
        char 				infile[100];		
	int				sd;
	struct sockaddr_in 	pin;
	int 				ch;
	
	/* parse the args */
	opterr = 0;
	optind = 1;
	while (-1 != (ch = getopt(argc, argv, "pi:h"))) {
	    switch (ch) {
	    case 'i':
	      strcpy(infile, optarg);
	      break;
	    case 'h':
	      usage(argv[0]);
	      return -1;
	    case '?':
	      fprintf(stdout, "Unknown option `-%c'.\n", optopt);
	      usage(argv[0]);
	      return -1;
	    }
	}

	/* fill in the socket structure with host information */
	memset(&pin, 0, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = htonl(SERVER_IP);
	pin.sin_port = htons(PORT);

	/* grab an Internet domain socket */
#ifdef ENABLE_PARSEC_UPTCPIP
	if ((sd = uptcp_socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#else
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#endif
		EXIT_TRACE("Socket error: socket()\n");
	}

	/* connect to PORT on HOST */
#ifdef ENABLE_PARSEC_UPTCPIP
	if (uptcp_connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) < 0) {
#else
	if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) < 0) {
#endif
		EXIT_TRACE("Socket error: connect()\n");
		
	}

	/* Read data from input file*/
	struct stat filestat;
  	int fd;

	  /* src file stat */
	if (stat(infile, &filestat) < 0) 
	      EXIT_TRACE("stat() %s failed: %s\n",infile, strerror(errno));

	if (!S_ISREG(filestat.st_mode)) 
	    EXIT_TRACE("not a normal file: %s\n", infile);

        /* send file size to server */
#ifdef ENABLE_PARSEC_UPTCPIP
        if (uptcp_send(sd,  &(filestat.st_size), sizeof(filestat.st_size), 0) == -1) {
#else
        if (send(sd,  &(filestat.st_size), sizeof(filestat.st_size), 0) == -1) {
#endif
              EXIT_TRACE("Socket error: cannot send input file size to server\n");
        }


        /* src file open */
	if((fd = open(infile, O_RDONLY)) < 0) 
	    EXIT_TRACE("%s file open error %s\n", infile, strerror(errno));

	  /* Load entire file into memory if requested by user */
	char *preloading_buffer;
	size_t bytes_read=0;
	int r;

	preloading_buffer = (char*)malloc(filestat.st_size);
	if(preloading_buffer == NULL)
	      EXIT_TRACE("Error allocating memory for input buffer.\n");

	    /* Read data until buffer full */
	while(bytes_read < filestat.st_size) {
	    	r = read(fd, preloading_buffer+bytes_read, filestat.st_size-bytes_read);
	      	if(r<0){
			switch(errno) {
	        	case EAGAIN:
	          		EXIT_TRACE("I/O error: No data available\n");break;
		        case EBADF:
		          	EXIT_TRACE("I/O error: Invalid file descriptor\n");break;
		        case EFAULT:
		          	EXIT_TRACE("I/O error: Buffer out of range\n");break;
		        case EINTR:
		          	EXIT_TRACE("I/O error: Interruption\n");break;
		        case EINVAL:
		          	EXIT_TRACE("I/O error: Unable to read from file descriptor\n");break;
		        case EIO:
		          	EXIT_TRACE("I/O error: Generic I/O error\n");break;
		        case EISDIR:
		          	EXIT_TRACE("I/O error: Cannot read from a directory\n");break;
		        default:
		          	EXIT_TRACE("I/O error: Unrecognized error\n");break;
		      }
	      }
	      if(r==0) break;
	      bytes_read += r;
	}


	/* Send data to server*/
	int bytes_left = filestat.st_size;
	int bytes_tosend = 0;
	char	*send_ptr = preloading_buffer;
	while(bytes_left >0){
		bytes_tosend = bytes_left; //BUFSIZE <  bytes_left ? BUFSIZE: bytes_left;
#ifdef ENABLE_PARSEC_UPTCPIP
		if (uptcp_send(sd, send_ptr, bytes_tosend, 0) == -1) {
#else
		if (send(sd, send_ptr, bytes_tosend, 0) == -1) {
#endif
			EXIT_TRACE("Socket error: send input file data error\n");
		}
		bytes_left -= bytes_tosend;
		send_ptr += bytes_tosend;
	        /* wait for a message to come back from the server */
	}

	printf("[netdedup.client]: Send data to server ok!\n");

	free(preloading_buffer);
	close(fd);
#ifdef ENABLE_PARSEC_UPTCPIP
	uptcp_close(sd);
#else
	close(sd);
#endif
	return 0;
}
