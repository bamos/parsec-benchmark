include ../Makefile.config

all: host_serv.o if_host.o uptcp_statis.o 

uptcp_statis.o:	uptcp_statis.c
	$(CC) $(CFLAGS) -c -o uptcp_statis.o uptcp_statis.c 
	$(CC) $(CFLAGS) -DUPTCP_CLIENT -c -o uptcp_statis.o.client uptcp_statis.c 

host_serv.o:	host_serv.c
	$(CC) $(CFLAGS) -c -o host_serv.o host_serv.c
	$(CC) $(CFLAGS) -DUPTCP_CLIENT -c -o host_serv.o.client host_serv.c

if_host.o: if_host.c
	$(CC) $(CFLAGS) -c -o if_host.o if_host.c
	$(CC) $(CFLAGS) -DUPTCP_CLIENT -c -o if_host.o.client if_host.c
	$(CC) $(CFLAGS) -DFOR_SIMULATION -c -o if_host.o.sim if_host.c
	$(CC) $(CFLAGS) -DFOR_SIMULATION -DUPTCP_CLIENT -c -o if_host.o.client.sim if_host.c

clean:
	rm *.o *.client uptcp_daemon -rf

