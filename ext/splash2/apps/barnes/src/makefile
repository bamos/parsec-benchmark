TARGET = barnes
FILES = code.C code_io.C load.C grav.C getparam.C util.C \
	code.H defs.H stdinc.H vectmath.H
CS = code.c code_io.c load.c grav.c getparam.c util.c 
OBJS = code.o code_io.o load.o grav.o getparam.o util.o
CFLAGS = -O2 -w 
LDFLAGS = -lm -lpthread
MACROS = ../../../null_macros/c.m4.null.pthread
M4 = m4 -s -Ulen -Uindex

x = *

ifdef version
  ifeq "$(version)" "IN_PARSEC"
    PREFIX=${PARSECDIR}/ext/splash2/apps/${TARGET}/inst/${PARSECPLAT}
    MACROS = ../../../../null_macros/c.m4.null.pthread
  endif
endif

$(TARGET): $(OBJS)
	gcc $(OBJS) $(CFLAGS) -o $(TARGET) $(LDFLAGS)


install:
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin/$(TARGET)
	cp -f run.sh $(PREFIX)/bin/run.sh

clean:
	rm -rf *.c *.h *.o $(TARGET)

.SUFFIXES:
.SUFFIXES:	.o .c .C .h .H

.H.h:
	$(M4) ${MACROS} $*.H > $*.h

.C.c:
	$(M4) $(MACROS) $*.C > $*.c

.c.o:
	gcc -c $(CFLAGS) $*.c

.C.o:
	$(M4) $(MACROS) $*.C > $*.c
	gcc -c $(CFLAGS) $*.c

code.h: code.H defs.h
defs.h: defs.H stdinc.h vectmath.h
stdinc.h: stdinc.H 
vectmath.h : vectmath.H

code.o: code.C code.h defs.h
code_io.o: code_io.C code.h
getparam.o: getparam.C stdinc.h
grav.o: grav.C code.h
load.o: load.C code.h defs.h
util.o: util.C stdinc.h

