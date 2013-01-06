TARGET = raytrace
OBJS = bbox.o cr.o env.o fbuf.o geo.o huprn.o husetup.o hutv.o isect.o main.o matrix.o memory.o poly.o raystack.o shade.o sph.o trace.o tri.o workpool.o

CFLAGS := $(CFLAGS) -Wall -W -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wdisabled-optimization
CFLAGS := $(CFLAGS) -Wpadded -Winline -Wpointer-arith -Wsign-compare -Wendif-labels
LDFLAGS := $(LDFLAGS) -lm 

PREFIX=${PARSECDIR}/ext/splash2x/apps/raytrace/inst/${PARSECPLAT}
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


bbox.c: bbox.C rt.h
cr.c:   cr.C rt.h
env.c:  env.C rt.h
fbuf.c: fbuf.C rt.h
geo.c:  geo.C rt.h
huprn.c: huprn.C rt.h
husetup.c: husetup.C rt.h
hutv.c: hutv.C rt.h
isect.c: isect.C rt.h
main.c: main.C rt.h
matrix.c: matrix.C rt.h
memory.c: memory.C rt.h
poly.c: poly.C rt.h
raystack.c: raystack.C rt.h
shade.c: shade.C rt.h
sph.c:  sph.C rt.h
trace.c: trace.C rt.h
tri.c:  tri.C rt.h
workpool.c: workpool.C rt.h

