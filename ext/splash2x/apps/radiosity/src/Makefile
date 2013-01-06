TARGET = radiosity
OBJS = display.o elemman.o modelman.o patchman.o rad_main.o rad_tools.o room_model.o smallobj.o taskman.o visible.o

HHS = model.H parallel.H patch.H radiosity.H task.H
HS = model.h parallel.h patch.h radiosity.h task.h

LOADLIBES = glibdumb/glib.a glibps/glibps.a


CFLAGS := $(CFLAGS) -I./glibdumb -I./glibps

CFLAGS := $(CFLAGS) -Wall -W -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wdisabled-optimization
CFLAGS := $(CFLAGS) -Wpadded -Winline -Wpointer-arith -Wsign-compare -Wendif-labels
LDFLAGS := $(LDFLAGS) -lm 

PREFIX=${PARSECDIR}/ext/splash2x/apps/radiosity/inst/${PARSECPLAT}
MACROS := ${PARSECDIR}/pkgs/libs/parmacs/inst/${PARSECPLAT}/m4/parmacs.${PARMACS_MACRO_FILE}.c.m4

ifdef version
  ifeq "$(version)" "pthreads"
    CFLAGS := $(CFLAGS) -DENABLE_THREADS -pthread
  endif
endif


.PHONY:	install clean

$(TARGET): $(OBJS) $(LOADLIBES)
	$(CC) $(OBJS) $(CFLAGS) $(LOADLIBES) -o $(TARGET) $(LDFLAGS) $(LIBS)

glibdumb/glib.a: 
	$(MAKE) -C glibdumb

glibps/glibps.a:
	$(MAKE) -C glibps


#$(TARGET): $(OBJS)
#	$(CC) $(OBJS) $(CFLAGS) -o $(TARGET) $(LDFLAGS) $(LIBS)

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


#CCOPTS = -I -float -I/usr/include $(CFLAGS)
#LDFLAGS = -lm -lpthread
#CFLAGS = $(CINCLUDE) -O4 -march=i686 -mcpu=i686 -malign-double -funroll-loops -finline-functions -fomit-frame-pointer -D_REENTRANT
#CINCLUDE = -I$(GLIBDIR) -I./glibps


radiosity.h: patch.h model.h parallel.h task.h
model.h: model.H
parallel.h: parallel.H
patch.h: patch.H
task.h: task.h
rad_main.c: rad_main.C $(HS)
patchman.c: patchman.C $(HS)
elemman.c: elemman.C $(HS)
taskman.c: taskman.C $(HS)
modelman.c: modelman.C $(HS)
smallobj.c: smallobj.C $(HS)
display.c: display.C $(HS)
visible.c: visible.C $(HS)
rad_tools.c: rad_tools.C $(HS)
room_model.c: room_model.C $(HS)
