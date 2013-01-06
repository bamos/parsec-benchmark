TARGET = water_nsquared

OBJS = water.o mdmain.o syscons.o kineti.o bndry.o cnstnt.o initia.o \
       interf.o intraf.o cshift.o poteng.o predcor.o
CFLAGS = -O2 -w  
LDFLAGS = -lm -pthread 
MACROS = ../../../null_macros/c.m4.null.pthread

x = *

ifdef version
  ifeq "$(version)" "IN_PARSEC"
    PREFIX=${PARSECDIR}/ext/splash2/apps/${TARGET}/inst/${PARSECPLAT}
    MACROS = ../../../../null_macros/c.m4.null.pthread
  endif
endif


.SUFFIXES:
.SUFFIXES: .o .S .s .c .C .U .h .H

.H.h:
	m4 $(MACROS) $*.H > $*.h
.C.c:
	m4 $(MACROS) $*.C > $*.c
.c.o:
	gcc -c $(CFLAGS) $*.c
.C.o:
	m4 $(MACROS) $*.C > $*.c
	gcc -c $(CFLAGS) $*.c

$(TARGET): $(OBJS)
	gcc $(OBJS) $(CFLAGS) -o $(TARGET) $(LDFLAGS)

install:
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin/$(TARGET)
	cp -f run.sh $(PREFIX)/bin/run.sh

clean:
	rm -rf *.c *.h *.o $(TARGET)

cnstnt.c: cnstnt.C water.h wwpot.h cnst.h frcnst.h fileio.h parameters.h
water.c: water.C parameters.h mdvar.h water.h wwpot.h cnst.h mddata.h \
         fileio.h frcnst.h randno.h global.h split.h
mdmain.c: mdmain.C  parameters.h mdvar.h water.h wwpot.h cnst.h \
          mddata.h fileio.h frcnst.h randno.h global.h split.h
syscons.c: syscons.C parameters.h mdvar.h water.h wwpot.h cnst.h mddata.h
kineti.c: kineti.C global.h split.h mdvar.h parameters.h mddata.h
bndry.c: bndry.C split.h mdvar.h parameters.h mddata.h
initia.c: initia.C split.h mdvar.h water.h cnst.h fileio.h \
           parameters.h mddata.h
predcor.c: predcor.C split.h mdvar.h parameters.h mddata.h
intraf.c: intraf.C frcnst.h mdvar.h water.h wwpot.h parameters.h \
           mddata.h global.h split.h
interf.c: interf.C mdvar.h water.h wwpot.h cnst.h global.h split.h \
           parameters.h mddata.h
cshift.c: cshift.C water.h
poteng.c: poteng.C global.h split.h mdvar.h frcnst.h water.h wwpot.h \
           parameters.h mddata.h

