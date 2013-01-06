include ../Makefile.config

OBJS = kern_tc.o\
	kern_shutdown.o\
	subr_prf.o\
	subr_bufring.o\
	kern_mbuf.o\
	uma_core.o\
	kern_malloc.o\
	kern_timeout.o\
	kern_clock.o\
	kern_intr.o\
	kern_synch.o\
	kern_mutex.o\
	kern_rwlock.o\
	kern_sx.o\
    uipc_syscalls.o

all: $(OBJS)

clean:
	rm *.o -rf

