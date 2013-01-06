# 
#  CS348C  Radiosity     Device independent graphics package GLIB.
#                        SUN/Xview version makefile
#
#

OBJS   = glibps.o
TARGET = glibps.a

INCLUDE_DIRS =

CCOPT  = -O 
CFLAGS = $(CCOPT) $(CCDEFS) $(INCLUDE_DIRS)
LDFLAGS =  -lm


all:	$(TARGET)

clean:
	touch dummy~
	rm -f core *.o *~ $(TARGET)

lint:
	lint $(INCLUDE_DIRS) $(SRCS)


$(TARGET): $(OBJS)
	ar crv $(TARGET) $(OBJS)
#	ranlib $(TARGET)

testlib:
	gcc -g -o testlib testlib.c $(TARGET) $(LDFLAGS)

