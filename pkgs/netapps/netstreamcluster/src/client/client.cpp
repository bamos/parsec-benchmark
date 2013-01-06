/*
 * Copyright (C) 2008 Princeton University
 * All rights reserved.
 * Authors: Jia Deng, Gilberto Contreras
 *
 * streamcluster - Online clustering algorithm
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <pthread.h>

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

#define PORT       42284 
/* REPLACE with your server machine name*/
#define SERVER_IP       0x0a0a0a0c //"10.10.10.12"
//#define SERVER_IP       0x7f000001 //"127.0.0.1"


using namespace std;

#define SEED 1
#define MAX_THREAD	16

typedef struct thread_arg{
  int           tid;
  int		fd;
  long          size; 
} thread_arg;

static int nproc; //# of threads
static int dim;
static long chunksize;
static pthread_barrier_t thread_barrier;

/*****************************//**
 *
 * send to server 
 *
 *******************************/ 
void* send_to_server(void* arg)
{
  thread_arg* t_arg = (thread_arg*)arg;
  int     		tid = t_arg->tid;
  int			sd = t_arg->fd;
  struct sockaddr_in 	sin;
  struct sockaddr_in 	pin;
  struct hostent 	*hp;
  int   		chunks;
  float 		*send_buf;


  if(tid != 0){
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
          printf("Socket error: socket()\n");
          goto out;
      }

      /* connect to PORT on SERVER_IP */
#ifdef ENABLE_PARSEC_UPTCPIP
      if (uptcp_connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
#else
      if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
#endif
          printf("Socket error: connect()\n");
          goto out;
      }
  }

  /* allocate memory buffer */
  chunks = t_arg->size / chunksize;
  send_buf = (float*)malloc(chunksize*dim*sizeof(float));
  if(send_buf == NULL){
      printf("not enough ememory\n");
      goto out;
  }
 
  /* Send data to server */
  printf("[Client:%d]: Sending ...\n", tid);
  for(int i = 0; i < chunks; i++){

      /* generate data */
      for( int j = 0; j < chunksize ; j++ ) {
          for( int k = 0; k < dim; k++ ) {
     	     send_buf[j*dim + k] = lrand48()/(float)(2147483647);  //INT_MAX;
          }
      }

      /* send data */
      int   bytes_left = chunksize * dim * sizeof(float); 
      int   bytes_tosend = 0;
      int   ss;
      char* send_ptr = (char*)send_buf; 

      while(bytes_left >0){
          bytes_tosend = bytes_left; 
#ifdef ENABLE_PARSEC_UPTCPIP
          if ((ss = uptcp_send(sd, send_ptr, bytes_tosend, 0)) == -1) {
#else
          if ((ss = send(sd, send_ptr, bytes_tosend, 0)) == -1) {
#endif
              printf("Socket error: send input file data error\n");
          }
          printf("[Client:%d] send bytes = %d\n", tid, ss);
          bytes_left -= ss;
          send_ptr += ss;
      }
  }
 
  printf("[Client:%d] Send data to server ok!\n", tid);

  if(tid == 0){
#ifdef ENABLE_PARSEC_UPTCPIP
      if (uptcp_recv(sd, &tid, sizeof(tid), 0) == -1) {
#else
      if (recv(sd, &tid, sizeof(tid), 0) == -1) {
#endif
           printf("Socket error: recv()\n");
      }
      printf("Receive Ack from Server\n");
  }
  
  pthread_barrier_wait(&thread_barrier);

out:	
  free(t_arg);

#ifdef ENABLE_PARSEC_UPTCPIP
  uptcp_close(sd);
#else
  close(sd);
#endif

  pthread_exit(NULL);
}


/*****************************//**
 *
 * main() 
 *
 *******************************/ 
int main(int argc, char **argv)
{
  long n;
  pthread_t  thread_id[MAX_THREAD];
  int        thread_count = 0;

  /* deal with input arguments */
  if (argc != 5) {
    fprintf(stderr,"usage: %s d n chunksize nproc\n",
	    argv[0]);
    fprintf(stderr,"  d:           Dimension of each data point\n");
    fprintf(stderr,"  n:           Number of data points\n");
    fprintf(stderr,"  chunksize:   Number of data points to handle per step\n");
    fprintf(stderr,"  nproc:       Number of threads to use\n");
    fprintf(stderr,"\n");
    fprintf(stderr, "if n > 0, points will be randomly generated instead of reading from infile.\n");
    exit(1);
  }

  dim = atoi(argv[1]);
  n = atoi(argv[2]);
  chunksize = atoi(argv[3]);
  nproc = atoi(argv[4]);

  if(n % chunksize != 0){
      printf("[Client] ensure that n \% chunksize == 0\n");
      exit(1);
  }

  int chunks = n/chunksize;
  if(chunks < nproc){
      printf("[Client] %d threads is too many for %d chunks. Use %d threads instead", nproc, chunks, chunks);
      nproc = chunks;
  }
   
  int chunks_per_thread = chunks / nproc;
  int rest = chunks % nproc;

  srand48(SEED);

  /* initial connection */
  int			sd;
  struct sockaddr_in 	sin;
  struct sockaddr_in 	pin;
  struct hostent 		*hp;

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
      printf("Socket error: socket()\n");
      exit(1);
  }

  /* connect to PORT on SERVER_IP */
#ifdef ENABLE_PARSEC_UPTCPIP
  if (uptcp_connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
#else
  if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
#endif
      printf("Socket error: connect()\n");
      exit(1);
  }

  /* send file size to server */
#ifdef ENABLE_PARSEC_UPTCPIP
  if (uptcp_send(sd,  &nproc, sizeof(nproc), 0) == -1) {
#else
  if (send(sd,  &nproc, sizeof(nproc), 0) == -1) {
#endif
      printf("Socket error: cannot send input file size to server\n");
      exit(1);
  }
  
  pthread_barrier_init(&thread_barrier, NULL, nproc);

  /* create threads */
  thread_arg* arg_ptr;
  while(thread_count < nproc){
 
      arg_ptr = (thread_arg*)malloc(sizeof(thread_arg));
      arg_ptr->tid = thread_count;
      if(thread_count == 0)
            arg_ptr->fd = sd;
      else  arg_ptr->fd = -1;
      if(thread_count < rest)
            arg_ptr->size = (chunks_per_thread + 1) * chunksize;
      else  arg_ptr->size = chunks_per_thread * chunksize;

#ifdef ENABLE_PARSEC_UPTCPIP
      if(uptcp_pthread_create(&thread_id[thread_count], NULL, send_to_server, (void*)arg_ptr) != 0){
#else
      if(pthread_create(&thread_id[thread_count], NULL, send_to_server, arg_ptr) != 0){
#endif
          printf("pthread_create() error\n");
      } 

      thread_count ++;
  }

  while(thread_count > 0){
      pthread_join(thread_id[thread_count-1], NULL);
      thread_count --;
  }

  return 0;
}
