/* AUTORIGHTS
Copyright (C) 2007 Princeton University
      
This file is part of Ferret Toolkit.

Ferret Toolkit is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "tpool.h"
#include "queue.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#ifdef ENABLE_PARSEC_UPTCPIP
#include <uptcp_socket.h>
#endif

#define PORT       42285
/* REPLACE with your server machine name*/
#define SERVER_IP       0x0a0a0a0c //"10.10.10.12"
//#define SERVER_IP       0x7f000001 //"127.0.0.1"

#define MAX_THREADS		16
#define MAX_SOCKET_BUF		0x4000

#define DEFAULT_DEPTH	25

const char *query_dir = NULL;
const char *output_path = NULL;

FILE *fout;
pthread_mutex_t fout_lock = PTHREAD_MUTEX_INITIALIZER;


int NTHREAD_LOAD = 1;
int NTHREAD_PROCESS	= 1;
int DEPTH = DEFAULT_DEPTH;

struct load_data
{
	char *name;
};

struct queue q_load_process;


struct packet{
	int	len;
	char	buf[1];
};

/* ------- The Helper Functions ------- */
int cnt_enqueue;
int cnt_dequeue;
char path[BUFSIZ];


int scan_dir (const char *, char *head);

int dir_helper (char *dir, char *head)
{
	DIR *pd = NULL;
	struct dirent *ent = NULL;
	int result = 0;
	pd = opendir(dir);
	if (pd == NULL) goto except;
	for (;;)
	{
		ent = readdir(pd);
		if (ent == NULL) break;
		if (scan_dir(ent->d_name, head) != 0) return -1;
	}
	goto final;

except:
	result = -1;
	perror("Error:");
final:
	if (pd != NULL) closedir(pd);
	return result;
}

/* the whole path to the file */
int file_helper (const char *file)
{
	int r;
	struct load_data *data;

	data = (struct load_data *)malloc(sizeof(struct load_data));
	assert(data != NULL);

	data->name = strdup(file);

	cnt_enqueue++;
	enqueue(&q_load_process, data);

	return 0;
}

int scan_dir (const char *dir, char *head)
{
	struct stat st;
	int ret;
	/* test for . and .. */
	if (dir[0] == '.')
	{
		if (dir[1] == 0) return 0;
		else if (dir[1] == '.')
		{
			if (dir[2] == 0) return 0;
		}
	}

	/* append the name to the path */
	strcat(head, dir);
	ret = stat(path, &st);
	if (ret != 0)
	{
		perror("Error:");
		return -1;
	}
	if (S_ISREG(st.st_mode)) file_helper(path);
	else if (S_ISDIR(st.st_mode))
	{
		strcat(head, "/");
		dir_helper(path, head + strlen(head));
	}
	/* removed the appended part */
	head[0] = 0;
	return 0;
}



/* ------ The Stages ------ */
void *t_load (void *dummy)
{
	const char *dir = (const char *)dummy;

	path[0] = 0;

	if (strcmp(dir, ".") == 0)
	{
		dir_helper(".", path);
	}
	else
	{
		scan_dir(dir, path);
	}

	queue_signal_terminate(&q_load_process);
	return NULL;
}

void send_end_tag()
{
	int		 fd;
        struct sockaddr_in      sin;
        struct sockaddr_in      pin;
        struct hostent          *hp;
	int		end_tag;;

        memset(&pin, 0, sizeof(pin));
       	pin.sin_family = AF_INET;
        pin.sin_addr.s_addr = htonl(SERVER_IP);
       	pin.sin_port = htons(PORT);

        /* grab an Internet domain socket */
#ifdef ENABLE_PARSEC_UPTCPIP
        if ((fd = uptcp_socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#else
        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#endif  
       	        fprintf(stderr, "Socket error: socket()\n");
        }
        
        /* connect to PORT on HOST */
#ifdef ENABLE_PARSEC_UPTCPIP
        if (uptcp_connect(fd,(struct sockaddr *)  &pin, sizeof(pin)) < 0) {
#else     
        if (connect(fd,(struct sockaddr *)  &pin, sizeof(pin)) < 0) {
#endif  
       	        fprintf(stderr, "Socket error: connect()\n");
        }

	end_tag = -1;
#ifdef ENABLE_PARSEC_UPTCPIP
        if (uptcp_send(fd, &end_tag, sizeof(int), 0) == -1) {
#else           
        if (send(fd, &end_tag, sizeof(int), 0) == -1) {
#endif                  
		fprintf(stderr, "Socket error: send input file data error\n");
        }

}


void *t_process (void *dummy)
{
	struct load_data *load;
	struct packet    *pkt_ptr;
	char		 *buf_ptr;
	int		 fd;
        struct sockaddr_in      sin;
        struct sockaddr_in      pin;
        struct hostent          *hp;
        struct stat filestat; 
	
 
	pkt_ptr = (struct packet*)malloc(MAX_SOCKET_BUF);
	if(pkt_ptr == NULL)
		fprintf(stderr, "allocate packet buffer error\n");
	
	while(1){
		/* 1. dequeue */
		if(dequeue(&q_load_process, &load) < 0)
			break;

		assert(load != NULL);

		/* 2. read data */
             
	        /* src file stat */
	        if (stat(load->name, &filestat) < 0)
	       	      fprintf(stderr, "stat() %s failed: %s\n",load->name, strerror(errno));
        
	        if (!S_ISREG(filestat.st_mode))
       		    fprintf(stderr, "not a normal file: %s\n", load->name);

		pkt_ptr->len = filestat.st_size;
	
        	/* src file open */ 
	        if((fd = open(load->name, O_RDONLY)) < 0)  
        	    fprintf(stderr, "%s file open error %s\n", load->name, strerror(errno));

	        size_t bytes_read = 0;
        	int r;
		buf_ptr = pkt_ptr->buf;
	        /* Read data until buffer full */
	        while(bytes_read < pkt_ptr->len) {
        		r = read(fd, buf_ptr, pkt_ptr->len - bytes_read);
	                if(r<0)
				fprintf(stderr, "read data from file error\n");

			if(r==0) 
				break;

			bytes_read += r;
			buf_ptr += r;
	        }
		close(fd);

		/* 3. send request to server */
	        memset(&pin, 0, sizeof(pin));
        	pin.sin_family = AF_INET;
	        pin.sin_addr.s_addr = htonl(SERVER_IP);
        	pin.sin_port = htons(PORT);

	        /* grab an Internet domain socket */
#ifdef ENABLE_PARSEC_UPTCPIP
	        if ((fd = uptcp_socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#else
	        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#endif  
        	        fprintf(stderr, "Socket error: socket()\n");
	        }
        
	        /* connect to PORT on HOST */
#ifdef ENABLE_PARSEC_UPTCPIP
	        if (uptcp_connect(fd,(struct sockaddr *)  &pin, sizeof(pin)) < 0) {
#else     
	        if (connect(fd,(struct sockaddr *)  &pin, sizeof(pin)) < 0) {
#endif  
        	        fprintf(stderr, "Socket error: connect()\n");
	        }

	        /* Send data to server*/
	        int bytes_left = pkt_ptr->len + sizeof(int);
	        int bytes_tosend = 0;
        	buf_ptr = (char*)pkt_ptr;
	        while(bytes_left >0){
        	        bytes_tosend = bytes_left; 
#ifdef ENABLE_PARSEC_UPTCPIP
	                if (uptcp_send(fd, buf_ptr, bytes_tosend, 0) == -1) {
#else           
	                if (send(fd, buf_ptr, bytes_tosend, 0) == -1) {
#endif                  
	                        fprintf(stderr, "Socket error: send input file data error\n");
	                }
        	        bytes_left -= bytes_tosend;
	                buf_ptr += bytes_tosend;
        	}


		/* 4. wait for results */
	        int bytes_recv = 0;
		int bytes_input = MAX_SOCKET_BUF;

	        buf_ptr = (char*)pkt_ptr; 
	        while(bytes_recv < bytes_input) {
#ifdef ENABLE_PARSEC_UPTCPIP
			if ((r = uptcp_recv(fd, buf_ptr+bytes_recv, (bytes_input-bytes_recv), 0)) == -1) {
#else
			if ((r = recv(fd, buf_ptr+bytes_recv, (bytes_input-bytes_recv), 0)) == -1) {
#endif
				fprintf(stderr, "recv error\n\n");
			}

			if(bytes_input == MAX_SOCKET_BUF){
				bytes_input = pkt_ptr->len;
			}

			bytes_recv += r; 

			if(r == 0)
				break;
			bytes_recv += r;
		}

		/* 5. write results */
		pthread_mutex_lock(&fout_lock);

		fwrite(load->name, 1, strlen(load->name), fout);
		fwrite(pkt_ptr->buf, 1, pkt_ptr->len, fout);
		fwrite("\n", 1, 1, fout);

		cnt_dequeue++;

		if(cnt_enqueue == cnt_dequeue){ /* last item */
			send_end_tag();
		}	

		pthread_mutex_unlock(&fout_lock);

		free(load->name);
		free(load);

#ifdef ENABLE_PARSEC_UPTCPIP
		uptcp_close(fd);
#else           
		close(fd);
#endif                  
	}

	free(pkt_ptr);
	return NULL;

}

int main (int argc, char *argv[])
{
	tdesc_t *t_load_desc;
	tdesc_t *t_process_desc;
	
	tpool_t *p_load;
	tpool_t *p_process;

	int ret, i;

	if (argc < 4)
	{
		printf("%s <query dir> <n> <out>\n", argv[0]); 
		return 0;
	}

	query_dir = argv[1];
	NTHREAD_PROCESS = atoi(argv[2]);

	output_path = argv[3];

	fout = fopen(output_path, "w");
	assert(fout != NULL);

	queue_init(&q_load_process, DEPTH, NTHREAD_LOAD);

	t_load_desc = (tdesc_t *)calloc(NTHREAD_LOAD, sizeof(tdesc_t));
	t_process_desc = (tdesc_t *)calloc(NTHREAD_PROCESS, sizeof(tdesc_t));

	t_load_desc[0].attr = NULL;
	t_load_desc[0].start_routine = t_load;
	t_load_desc[0].arg = query_dir;

	for (i = 1; i < NTHREAD_LOAD; i++) t_load_desc[i] = t_load_desc[0];

	t_process_desc[0].attr = NULL;
	t_process_desc[0].start_routine = t_process;
	t_process_desc[0].arg = NULL;

	for (i = 1; i < NTHREAD_PROCESS; i++) t_process_desc[i] = t_process_desc[0];

	cnt_enqueue = cnt_dequeue = 0;

	p_load = tpool_create(t_load_desc, NTHREAD_LOAD);
	p_process = tpool_create(t_process_desc, NTHREAD_PROCESS);

	tpool_join(p_process, NULL);
	tpool_join(p_load, NULL);

	tpool_destroy(p_load);
	tpool_destroy(p_process);

	free(t_load_desc);
	free(t_process_desc);

	queue_destroy(&q_load_process);

	fclose(fout);

	return 0;
}

