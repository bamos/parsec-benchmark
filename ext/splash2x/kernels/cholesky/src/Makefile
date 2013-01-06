TARGET = cholesky
OBJS = amal.o block2.o numLL.o tree.o assign.o fo.o parts.o util.o bfac.o malloc.o seg.o bksolve.o mf.o solve.o

CFLAGS := $(CFLAGS) -Wall -W -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wdisabled-optimization
CFLAGS := $(CFLAGS) -Wpadded -Winline -Wpointer-arith -Wsign-compare -Wendif-labels
LDFLAGS := $(LDFLAGS) -lm 

PREFIX=${PARSECDIR}/ext/splash2x/kernels/cholesky/inst/${PARSECPLAT}
MACROS := ${PARSECDIR}/pkgs/libs/parmacs/inst/${PARSECPLAT}/m4/parmacs.${PARMACS_MACRO_FILE}.c.m4

ifdef version
  ifeq "$(version)" "pthreads"
    CFLAGS := $(CFLAGS) -DENABLE_THREADS -pthread
  endif
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


matrix.h: matrix.H
amal.c: matrix.h
block2.c: matrix.h
numLL.c: matrix.h
tree.c: matrix.h
assign.c: matrix.h
fo.c: matrix.h
parts.c: matrix.h
util.c: matrix.h
bfac.c: matrix.h
malloc.c: matrix.h
seg.c: matrix.h
bksolve.c: matrix.h
mf.c: matrix.h
solve.c: matrix.h


