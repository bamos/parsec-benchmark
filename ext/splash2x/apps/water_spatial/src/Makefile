TARGET = water_spatial
OBJS = bndry.o cnstnt.o cshift.o initia.o interf.o intraf.o kineti.o mdmain.o poteng.o predcor.o syscons.o water.o

CFLAGS := $(CFLAGS) -Wall -W -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wdisabled-optimization
CFLAGS := $(CFLAGS) -Wpadded -Winline -Wpointer-arith -Wsign-compare -Wendif-labels
LDFLAGS := $(LDFLAGS) -lm 

PREFIX=${PARSECDIR}/ext/splash2x/apps/water_spatial/inst/${PARSECPLAT}
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

bndry.o: bndry.C split.h mdvar.h parameters.h mddata.h global.h
cnstnt.o: cnstnt.C water.h wwpot.h cnst.h frcnst.h fileio.h parameters.h global.h
cshift.o: cshift.C water.h global.h
initia.o: initia.C split.h mdvar.h water.h cnst.h fileio.h parameters.h mddata.h global.h
interf.o: interf.C mdvar.h water.h wwpot.h cnst.h global.h split.h parameters.h mddata.h
intraf.o: intraf.C frcnst.h mdvar.h water.h wwpot.h parameters.h mddata.h global.h split.h
kineti.o: kineti.C global.h split.h mdvar.h parameters.h mddata.h
mdmain.o: mdmain.C  parameters.h mdvar.h water.h wwpot.h cnst.h mddata.h fileio.h frcnst.h randno.h global.h split.h
poteng.o: poteng.C global.h split.h mdvar.h frcnst.h water.h wwpot.h parameters.h mddata.h
predcor.o: predcor.C split.h mdvar.h parameters.h mddata.h global.h
syscons.o: syscons.C parameters.h mdvar.h water.h wwpot.h cnst.h mddata.h global.h
water.o: water.C parameters.h mdvar.h water.h wwpot.h cnst.h mddata.h fileio.h frcnst.h randno.h global.h split.h
