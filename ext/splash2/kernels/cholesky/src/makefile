TARGET = cholesky
OBJS = amal.o assign.o bfac.o bksolve.o block2.o fo.o mf.o numLL.o \
     parts.o seg.o solve.o tree.o util.o malloc.o 

LDFLAGS = -lpthread -lm
MACROS = ../../../null_macros/c.m4.null.pthread


x = *

ifdef version
  ifeq "$(version)" "IN_PARSEC"
    PREFIX=${PARSECDIR}/ext/splash2/kernels/${TARGET}/inst/${PARSECPLAT}
    MACROS = ../../../../null_macros/c.m4.null.pthread
  endif
endif

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(TARGET) $(LDFLAGS)

install:
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin/$(TARGET)
	cp -f run.sh $(PREFIX)/bin/run.sh

clean:
	rm -rf *.c *.h *.o $(TARGET)

.SUFFIXES:
.SUFFIXES:	.o .c .C .h .H

.H.h:
	m4 ${MACROS} $*.H > $*.h

.C.c:
	m4 $(MACROS) $*.C > $*.c

.c.o:
	$(CC) -c $(CFLAGS) $*.c

.C.o:
	m4 $(MACROS) $*.C > $*.c
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

