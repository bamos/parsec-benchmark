include ../Makefile.config

CC = gcc
CFLAGS = -std=c99 -fPIC -ffreestanding ${GCC_OX} -D_XOPEN_SOURCE -Wall -I../host.support/

OBJS_1 = ../host.support/uptcp_statis.o         ../host.support/host_serv.o         ../host.support/if_host.o
OBJS_2 = ../host.support/uptcp_statis.o.client  ../host.support/host_serv.o.client  ../host.support/if_host.o.client
OBJS_3 = ../host.support/uptcp_statis.o         ../host.support/host_serv.o         ../host.support/if_host.o.sim
OBJS_4 = ../host.support/uptcp_statis.o.client  ../host.support/host_serv.o.client  ../host.support/if_host.o.client.sim


all: uptcp_api.o 
	ar rcs libuptcp.a ../freebsd.kern/*.o ../freebsd.netinet/*.o *.o $(OBJS_1) 
	ar rcs libuptcp.client.a ../freebsd.kern/*.o ../freebsd.netinet/*.o *.o.client $(OBJS_2)
	ar rcs libuptcp.sim.a ../freebsd.kern/*.o ../freebsd.netinet/*.o  *.o $(OBJS_3)
	ar rcs libuptcp.client.sim.a ../freebsd.kern/*.o ../freebsd.netinet/*.o *.o.client $(OBJS_4)


uptcp_api.o: uptcp_api.c
	$(CC) $(CFLAGS)  -c -o uptcp_api.o uptcp_api.c 
	$(CC) $(CFLAGS) -DUPTCP_CLIENT -c -o uptcp_api.o.client uptcp_api.c

install:
	mkdir -p ${UPTCPIP_INSTDIR}/lib
	cp *.a ${UPTCPIP_INSTDIR}/lib
	mkdir -p ${UPTCPIP_INSTDIR}/include
	cp *.h ${UPTCPIP_INSTDIR}/include

clean:
	rm *.o *.client -rf *.a

