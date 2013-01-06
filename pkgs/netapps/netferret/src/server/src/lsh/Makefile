.PHONY:	all clean

all:	liblsh.a

clean:
	rm *.o liblsh.a

liblsh.a:	$(OBJS)
	ar r $@ $^
	ranlib $@

C_SRC = *.c
CXX_SRC = *.cpp
OBJS = $(C_SRC:%.c=%.o) $(CXX_SRC:%.cpp=%.o)

all:	$(LIBDIR)/liblsh.a
	make -C lsh

clean:
	rm $(LIBDIR)/libcass.a *.o
	make -C lsh clean

$(LIBDIR)/libcass.a:	$(OBJS)
	ar r $@ $^
	ranlib $@
