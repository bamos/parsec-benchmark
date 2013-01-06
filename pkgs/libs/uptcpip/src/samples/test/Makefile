CC = gcc
CFLAGS1 = -g -DUPTCP_SOCKET -g -I../../../inst/amd64-linux.gcc/include/ -L../../../inst/amd64-linux.gcc/lib/
CFLAGS2 = -g 

all: 
	$(CC) $(CFLAGS1) --static -o client.uptcp  client.c -luptcp.client -pthread
	$(CC) $(CFLAGS1) --static -o server.uptcp  server.c -luptcp -lpthread 
	$(CC) $(CFLAGS2) -o client.local  client.c 
	$(CC) $(CFLAGS2) -o server.local  server.c 

clean:
	rm -rf *.uptcp  *.local
