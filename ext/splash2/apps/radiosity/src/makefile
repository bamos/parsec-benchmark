TARGET = radiosity

OBJS = rad_main.o patchman.o elemman.o taskman.o modelman.o smallobj.o\
         display.o visible.o rad_tools.o room_model.o 
HHS = model.H parallel.H patch.H radiosity.H task.H
HS = model.h parallel.h patch.h radiosity.h task.h

GLIBDIR = ./glibdumb
GLIB    = glib.a
# look me up in man sigfpe
LIBFPE = -lfpe

MACROS = ../../../null_macros/c.m4.null.pthread
CCOPTS = -I -float -I/usr/include $(CFLAGS)
LDFLAGS = -g -w -lpthread -lm 
CFLAGS = $(CINCLUDE) -g 
CINCLUDE = -I$(GLIBDIR) -I./glibps -I/usr/local/include

LOADLIBES =  $(GLIBDIR)/$(GLIB) glibps/glibps.a

x = *

ifdef version
  ifeq "$(version)" "IN_PARSEC"
    PREFIX=${PARSECDIR}/ext/splash2/apps/${TARGET}/inst/${PARSECPLAT}
    MACROS = ../../../../null_macros/c.m4.null.pthread
  endif
endif

$(TARGET): $(OBJS) 
	make -C glibdumb	
	make -C glibps	
	gcc $(CFLAGS) $(LDFLAGS) $(OBJS) $(LOADLIBES) -o $(TARGET)

install:
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin/$(TARGET)
	cp -f run.sh $(PREFIX)/bin/run.sh

clean:
	make -C glibdumb clean	
	make -C glibps	clean
	rm -rf *.c *.h *.o $(TARGET)

.SUFFIXES:
.SUFFIXES:	.o .c .C .h .H

.H.h:	
	m4 -Ulen -Uindex ${MACROS} $*.H > $*.h

.C.c:	
	m4 -Ulen -Uindex $(MACROS) $*.C > $*.c

.c.o:	
	gcc -c $(CFLAGS) $*.c

.C.o:	
	m4 -Ulen -Uindex $(MACROS) $*.C > $*.c
	gcc -c $(CFLAGS) $*.c

$(HS): $(HHS)

radiosity.h: patch.h model.h parallel.h task.h
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

