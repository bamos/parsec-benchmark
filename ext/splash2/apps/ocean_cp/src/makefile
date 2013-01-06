TARGET = ocean_cp
OBJS = main.o jacobcalc.o jacobcalc2.o laplacalc.o \
	slave1.o slave2.o multi.o linkup.o subblock.o 
CFLAGS = -O2 -w  
LDFLAGS = -lpthread -lm 
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
	gcc $(OBJS) -o $(TARGET) $(LDFLAGS)

install:
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin/$(TARGET)
	cp -f run.sh $(PREFIX)/bin/run.sh

clean:
	rm -rf *.c *.h *.o $(TARGET)

.SUFFIXES:
.SUFFIXES:	.o .c .C .h .H

.H.h: 
	${M4} ${MACROS} $*.H > $*.h

.C.c: 
	${M4} $(MACROS) $*.C > $*.c

.c.o: 
	gcc -c $(CFLAGS) $*.c

.C.o: 
	${M4} $(MACROS) $*.C > $*.c
	gcc -c $(CFLAGS) $*.c

decs.h: decs.H
jacobcalc.c: decs.h
linkup.c: decs.h
slave1.c: decs.h
jacobcalc2.c: decs.h
main.c: decs.h
slave2.c: decs.h
laplacalc.c: decs.h
multi.c: decs.h
subblock.c: decs.h

