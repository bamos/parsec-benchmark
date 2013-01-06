# 
#  Device independent graphics package GLIB.
#  NULL graphic device version
#

OBJS   = glib.o
TARGET = glib.a
PANELDIR = /usr/local/panel
CCOPT  = -O 
INCLUDE_DIRS = -I${PANELDIR}/include
CFLAGS = $(CCOPT) $(CCDEFS) $(INCLUDE_DIRS)
LDFLAGS =  -L${PANELDIR}/lib -lgl -lpanel -lm


all:	$(TARGET)

clean:
	touch dummy~
	rm -f core *.o *~ $(TARGET)

lint:
	lint $(INCLUDE_DIRS) $(SRCS)


$(TARGET): glib.o
	ar crv $(TARGET) glib.o

testlib:
	cc -O -o testlib testlib.c $(TARGET) $(LDFLAGS)

