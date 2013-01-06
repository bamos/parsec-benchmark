TARGET = fmm
OBJS = box.o construct_grid.o cost_zones.o defs.o fmm.o interactions.o memory.o particle.o partition_grid.o

CFLAGS := $(CFLAGS) -Wall -W -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wdisabled-optimization
CFLAGS := $(CFLAGS) -Wpadded -Winline -Wpointer-arith -Wsign-compare -Wendif-labels
LDFLAGS := $(LDFLAGS) -lm 

PREFIX=${PARSECDIR}/ext/splash2x/apps/fmm/inst/${PARSECPLAT}
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

defs.c: defs.h
box.c: box.h
construct_grid.c: construct_grid.h
cost_zones.c: cost_zones.h
interactions.c: interactions.h
memory.c: memory.h
particle.c: particle.h
partition_grid.c: partition_grid.h

fmm.o: fmm.C defs.h memory.h particle.h box.h partition_grid.h cost_zones.h construct_grid.h interactions.h
interactions.o: interactions.C defs.h memory.h particle.h box.h partition_grid.h interactions.h
shell.o: shell.C defs.h memory.h particle.h box.h partition_grid.h interactions.h
construct_grid.o: construct_grid.C defs.h memory.h particle.h box.h partition_grid.h construct_grid.h
cost_zones.o: cost_zones.C defs.h memory.h box.h partition_grid.h cost_zones.h
#orb.o: orb.C defs.h memory.h box.h partition_grid.h orb.h
partition_grid.o: partition_grid.C defs.h memory.h box.h partition_grid.h
box.o: box.C defs.h memory.h particle.h box.h
particle.o: particle.C defs.h memory.h particle.h
memory.o: memory.C defs.h memory.h
defs.o: defs.C defs.h memory.h
memory.h: defs.h particle.h box.h
