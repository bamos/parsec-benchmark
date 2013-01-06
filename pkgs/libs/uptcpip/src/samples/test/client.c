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
#ifdef UPTCP_SOCKET
#include <uptcp_socket.h>
#endif

#define PORT        0x2234
/* REPLACE with your server machine name*/
#define SERVER_IP        0x0a0a0a0c //"10.10.10.12"

#define BUFSIZE     0x100000  //512KB

#include <signal.h>
#include <execinfo.h>

void show_stackframe() {
  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;

  trace_size = backtrace(trace, 16);
  messages = backtrace_symbols(trace, trace_size);
  printf("[bt] Execution path:\n");
  for (i=0; i<trace_size; ++i)
	printf("[bt] %s\n", messages[i]);
}


void usage(char* prog)
{
  show_stackframe();
  printf("usage: %s [-h] [ -i file]\n",prog);
  printf("-i file\t\t\tthe input file\n");
  printf("-h \t\t\thelp\n");
}

int main(int argc, char **argv)
{
        char 				infile[100];		
	char    			sendbuf[BUFSIZE];
	int				sd, len;
	struct sockaddr_in 	sin;
	struct sockaddr_in 	pin;
	struct hostent 		*hp;
	int 				i = 0;
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
#ifdef UPTCP_SOCKET
	if ((sd = uptcp_socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#else
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#endif
		printf("Socket error: socket()\n");
		goto out;
	}

	/* connect to PORT on SERVER_IP */
#ifdef UPTCP_SOCKET
	if (uptcp_connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
#else
	if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
#endif
		printf("Socket error: connect()\n");
		goto out;
	}

	/* Read data from input file*/
	struct stat filestat;
  	int fd;

	  /* src file stat */
	if (stat(infile, &filestat) < 0) 
	      printf("stat() %s failed: %s\n",infile, strerror(errno));

	if (!S_ISREG(filestat.st_mode)) 
	    printf("not a normal file: %s\n", infile);

	  /* src file open */
	if((fd = open(infile, O_RDONLY)) < 0) 
	    printf("%s file open error %s\n", infile, strerror(errno));

	  /* Load entire file into memory if requested by user */
	char *preloading_buffer;
	size_t bytes_read=0;
	int r;

	preloading_buffer = (char*)malloc(filestat.st_size);
	if(preloading_buffer == NULL)
	      printf("Error allocating memory for input buffer.\n");

	    /* Read data until buffer full */
	while(bytes_read < filestat.st_size) {
	    	r = read(fd, preloading_buffer+bytes_read, filestat.st_size-bytes_read);
	      	if(r<0){
			switch(errno) {
	        	case EAGAIN:
	          		printf("I/O error: No data available\n");break;
		        case EBADF:
		          	printf("I/O error: Invalid file descriptor\n");break;
		        case EFAULT:
		          	printf("I/O error: Buffer out of range\n");break;
		        case EINTR:
		          	printf("I/O error: Interruption\n");break;
		        case EINVAL:
		          	printf("I/O error: Unable to read from file descriptor\n");break;
		        case EIO:
		          	printf("I/O error: Generic I/O error\n");break;
		        case EISDIR:
		          	printf("I/O error: Cannot read from a directory\n");break;
		        default:
		          	printf("I/O error: Unrecognized error\n");break;
		      }
	      }
	      if(r==0) break;
	      bytes_read += r;
	}

	/* send file size to server */
#ifdef UPTCP_SOCKET
	if (uptcp_send(sd,  &(filestat.st_size), sizeof(filestat.st_size), 0) == -1) {
#else
	if (send(sd,  &(filestat.st_size), sizeof(filestat.st_size), 0) == -1) {
#endif
		printf("Socket error: cannot send input file size to server\n");
	}


	/* Send data to server*/
	int bytes_left = filestat.st_size;
	int bytes_tosend = 0;
	char	*send_ptr = preloading_buffer;
	int ss;

	/* warmup */
	printf("Client: Warmup...\n");
 	bytes_left = (0x1000000 < bytes_left) ? 0x1000000 : bytes_left;	
	while(bytes_left >0){
		bytes_tosend = bytes_left; //BUFSIZE <  bytes_left ? BUFSIZE: bytes_left;
#ifdef UPTCP_SOCKET
		if ((ss = uptcp_send(sd, send_ptr, bytes_tosend, 0)) == -1) {
#else
		if ((ss = send(sd, send_ptr, bytes_tosend, 0)) == -1) {
#endif
			printf("Socket error: send input file data error\n");
		}
		bytes_left -= bytes_tosend;
		send_ptr += bytes_tosend;
	}

#ifdef UPTCP_SOCKET
        if ((ss = uptcp_recv(sd, &bytes_tosend, sizeof(bytes_tosend), 0)) == -1) {
#else
        if ((ss = recv(sd, &bytes_tosend, sizeof(bytes_tosend), 0)) == -1) {
#endif
		printf("Socket error: recv()\n");
        }
        printf("Received: %d\n", bytes_tosend);

#ifdef UPTCP_SOCKET
    parsec_enter_tcpip_roi();
#endif

	/* Real Run */
	bytes_left = filestat.st_size;
	bytes_tosend = 0;
	send_ptr = preloading_buffer;

	printf("Client: Real Run...\n");
	while(bytes_left >0){
		bytes_tosend = bytes_left; //BUFSIZE <  bytes_left ? BUFSIZE: bytes_left;
#ifdef UPTCP_SOCKET
		if ((ss = uptcp_send(sd, send_ptr, bytes_tosend, 0)) == -1) {
#else
		if ((ss = send(sd, send_ptr, bytes_tosend, 0)) == -1) {
#endif
			printf("Socket error: send input file data error\n");
		}
		printf("send bytes = %d\n", ss);
		bytes_left -= bytes_tosend;
		send_ptr += bytes_tosend;
	        /* wait for a message to come back from the server */
	}

	printf("Send data to server ok!\n");

#ifdef UPTCP_SOCKET
    parsec_exit_tcpip_roi();
#endif


#ifdef UPTCP_SOCKET
        if ((ss = uptcp_recv(sd, &bytes_tosend, sizeof(bytes_tosend), 0)) == -1) {
#else
        if ((ss = recv(sd, &bytes_tosend, sizeof(bytes_tosend), 0)) == -1) {
#endif
 		printf("Socket error: recv()\n");
        }
        printf("Received: %d\n", bytes_tosend);


	free(preloading_buffer);
out:
	close(fd);

#ifdef UPTCP_SOCKET
	uptcp_close(sd);
#else
	close(sd);
#endif
}
