TARGET = volrend
OBJS = main.o file.o option.o map.o normal.o opacity.o octree.o view.o \
       render.o adaptive.o raytrace.o
CFLAGS += -O2 -w -DPRESHD $(CDEBUG) -Ilibtiff
CCOPTS = -S -I -I$(SIMDIR) -I$(IDIR) -I/usr/include -DAUG $(CFLAGS)
LDFLAGS = -O2 -w -pthread -lm -Ilibtiff -Llibtiff -ltiff
MACROS = ../../../null_macros/c.m4.null.pthread

x = *

ifdef version
  ifeq "$(version)" "IN_PARSEC"
    PREFIX=${PARSECDIR}/ext/splash2/apps/${TARGET}/inst/${PARSECPLAT}
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

main.c:	main.C incl.h anl.h
file.c:	file.C incl.h
option.c: option.C incl.h
map.c: map.C incl.h
normal.c: normal.C incl.h anl.h address.h
opacity.c: opacity.C incl.h anl.h
octree.c: octree.C incl.h anl.h
view.c:	view.C incl.h
render.c: render.C incl.h
adaptive.c: adaptive.C incl.h anl.h
raytrace.c: raytrace.C incl.h address.h

incl.h:	user_options.h const.h my_types.h global.h macros.h address.h
