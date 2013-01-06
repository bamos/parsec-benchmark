#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/in.h>
#include <net/if.h>
#include <linux/filter.h>

#ifdef UPTCP_SOCKET
#include <uptcp_socket.h>
#endif

#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT        0x2234
#define BUFSIZE     0x100000  //1MB
char    			recvbuf[BUFSIZE];


int init_server_socket(int *accept_sd)
{
	int sd, asd;
	int addrlen;
  	struct sockaddr_in 	sin;
  	struct sockaddr_in 	pin;
  	struct hostent 	*hp;

 
	/* get an internet domain socket */
#ifdef UPTCP_SOCKET
	if ((sd = uptcp_socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#else
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#endif
		printf("Socket error: socket()\n");
		return 0;
	}

	/* complete the socket structure */
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(PORT);

	/* bind the socket to the port number */
#ifdef UPTCP_SOCKET
	if (uptcp_bind(sd, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
#else


	if (bind(sd, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
#endif
		printf("Socket error: bind()\n");
		goto error;
		
	}

	/* show that we are willing to listen */
#ifdef UPTCP_SOCKET
	if (uptcp_listen(sd, 5) == -1) {
#else
	if (listen(sd, 5) == -1) {
#endif
		printf("Socket error: listen()\n");
		goto error;
	}
	/* wait for a client to talk to us */
    	addrlen = sizeof(pin); 

#ifdef UPTCP_SOCKET
	if ((asd = uptcp_accept(sd, (struct sockaddr *)  &pin, &addrlen)) == -1) {
#else
	if ((asd = accept(sd, (struct sockaddr *)  &pin, &addrlen)) == -1) {
#endif
		printf("Socket error: accept()\n");
		goto error;
	}
	
	*accept_sd = asd;
	return sd;

error:


#ifdef UPTCP_SOCKET
	uptcp_close(sd);
#else
	close(sd);
#endif
	exit(-1);
}


int main(int argc, char** argv)
{
    int fd0, fd1, fd;
    char* preloading_buffer;
    char  *buf;
    char  infile[100];
    struct timeval start, end;
    int r = 0, r1 = 0, bytes_input = 0, bytes_recv = 0;
    char ch;

    fd0 = init_server_socket(&fd1);

#ifdef UPTCP_SOCKET
    if ((r = uptcp_recv(fd1, &bytes_input, sizeof(bytes_input), 0)) == -1) {
#else
    if ((r = recv(fd1, &bytes_input, sizeof(bytes_input), 0)) == -1) {
#endif
		printf("Socket error: recv()\n");
    }

    printf("total size=%d\n", bytes_input);

    preloading_buffer = (char*)calloc(bytes_input, 1);
    if(preloading_buffer == NULL)
          printf("Error allocating memory for input buffer.\n");



    int same = 1;
    int progress_bytes = bytes_input / 10; 
    int progress_unit = bytes_input / 10; 
    int progress = 0; 

    
    /* Warmup */
    printf("Server: Warmup...\n");
    int warmup_bytes = (0x1000000 < bytes_input) ? 0x1000000 : bytes_input;
    bytes_recv = 0;
    while(bytes_recv < warmup_bytes) {

#ifdef UPTCP_SOCKET
        if ((r = uptcp_recv(fd1, preloading_buffer+bytes_recv, (warmup_bytes-bytes_recv), 0)) == -1) {
#else
        if ((r = recv(fd1, preloading_buffer+bytes_recv, (warmup_bytes-bytes_recv), 0)) == -1) {
#endif
          	printf("I/O error\n");
		exit(1);

        }
        //if(r==0) break;
        bytes_recv += r;
    }

	/* send ack to client */
#ifdef UPTCP_SOCKET
    if (uptcp_send(fd1,  &(bytes_recv), sizeof(bytes_recv), 0) == -1) {
#else
    if (send(fd1,  &(bytes_recv), sizeof(bytes_recv), 0) == -1) {
#endif
 	printf("Socket error: cannot send recv to client\n");
    }
    printf("Send Ack\n");


#ifdef UPTCP_SOCKET
    parsec_enter_tcpip_roi();
#endif

    /* Real run */
    printf("Server: Real Run...\n");
    gettimeofday(&start, NULL);
    bytes_recv = 0;
    while(bytes_recv < bytes_input) {

#ifdef UPTCP_SOCKET
        if ((r = uptcp_recv(fd1, preloading_buffer+bytes_recv, (bytes_input-bytes_recv), 0)) == -1) {
#else
        if ((r = recv(fd1, preloading_buffer+bytes_recv, (bytes_input-bytes_recv), 0)) == -1) {
#endif
          	printf("I/O error\n");
		exit(1);

        }
        //if(r==0) break;
        bytes_recv += r;

	if(bytes_recv > progress_bytes){
	    progress += 10;
            printf("Complete: %12u(%d\%)\n", bytes_recv, progress);
	    progress_bytes += progress_unit;

        } 
	
    }
    printf("Complete: %12u(100\%)\n", bytes_recv);
    gettimeofday(&end, NULL);

#ifdef UPTCP_SOCKET
    parsec_exit_tcpip_roi();
#endif


    unsigned long long intval = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));

    printf("Data size = %dB, time = %9.6fs, BW = %8.3fMB/s\n", bytes_recv, intval/1000000.0, 1.0*bytes_recv/intval);

	/* send ack to client */
#ifdef UPTCP_SOCKET
    if (uptcp_send(fd1,  &(bytes_recv), sizeof(bytes_recv), 0) == -1) {
#else
    if (send(fd1,  &(bytes_recv), sizeof(bytes_recv), 0) == -1) {
#endif
   	printf("Socket error: cannot send recv to client\n");
    }
    printf("Send Ack\n");

#if 0  
    FILE* fp; 
    if((fp = fopen("recv.dat", "w+")) == NULL) 
	    printf("file open error %s\n", strerror(errno));
    printf("Writing received data into file...\n");
    if(fwrite(preloading_buffer, bytes_recv, 1, fp) < 0)
	    printf("file write error %s\n", strerror(errno));
    printf("Writing completed\n");

    fclose(fp);

    free(preloading_buffer);

    free(buf);
#endif

#ifdef UPTCP_SOCKET
    uptcp_close(fd1);
    uptcp_close(fd0);
#else
    close(fd1);
    close(fd0);
#endif


}
