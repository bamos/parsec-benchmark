TARGET = ocean_ncp
OBJS = jacobcalc.o laplacalc.o main.o multi.o slave1.o slave2.o

CFLAGS := $(CFLAGS) -Wall -W -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wdisabled-optimization
CFLAGS := $(CFLAGS) -Wpadded -Winline -Wpointer-arith -Wsign-compare -Wendif-labels
LDFLAGS := $(LDFLAGS) -lm 

PREFIX=${PARSECDIR}/ext/splash2x/apps/ocean_ncp/inst/${PARSECPLAT}
MACROS := ${PARSECDIR}/pkgs/libs/parmacs/inst/${PARSECPLAT}/m4/parmacs.${PARMACS_MACRO_FILE}.c.m4

ifdef version
  ifeq "$(version)" "pthreads"
    CFLAGS := $(CFLAGS) -DENABLE_THREADS -pthread
  endif
endif

ostype=$(findstring solaris, ${PARSECPLAT})

ifeq "$(ostype)" "solaris"
    CFLAGS += -DSPARC_SOLARIS
endif

.PHONY:	install clean

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(TARGET) $(LDFLAGS) $(LIBS)

install:
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin/$(TARGET)
	cp -f run.sh $(PREFIX)/bin/run.sh

clean:
	rm -rf *.c *.h *.o $(TARGET)

.SUFFIXES:
.SUFFIXES:	.o .c .C .h .H

.H.h:
	$(M4) -s -Ulen -Uindex $(MACROS) $*.H > $*.h

.C.c:
	$(M4) -s -Ulen -Uindex $(MACROS) $*.C > $*.c

.c.o:
	$(CC) -c $(CFLAGS) $*.c

.C.o:
	$(M4) -s -Ulen -Uindex $(MACROS) $*.C > $*.c
	$(CC) -c $(CFLAGS) $*.c

decs.h: decs.H
jacobcalc.c: decs.h
main.c: decs.h
slave1.c: decs.h
laplacalc.c: decs.h
multi.c : decs.h
slave2.c: decs.h

