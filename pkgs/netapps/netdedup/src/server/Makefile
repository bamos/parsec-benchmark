# Makefile for dedup kernel

PREFIX=${PARSECDIR}/pkgs/netapps/netdedup/inst/${PARSECPLAT}

TARGET=netdedup
TARGET_SIM=netdedup_sim

CFLAGS += -O0 -g -Wall -fno-strict-aliasing -D_XOPEN_SOURCE=600

ostype=$(findstring solaris, ${PARSECPLAT})

ifeq "$(ostype)" "solaris"
    CFLAGS += -std=gnu99
endif

LIBS += -lm -lpthread

LIB_UPTCP= -luptcp
LIB_UPTCP_SIM= -luptcp.sim

DEDUP_OBJ = hashtable.o util.o dedup.o rabin.o encoder.o decoder.o mbuffer.o sha.o

# Uncomment the following to enable gzip compression
CFLAGS += -DENABLE_PARSEC_UPTCPIP -DENABLE_GZIP_COMPRESSION
#CFLAGS +=  -DENABLE_GZIP_COMPRESSION

LIBS += -lz

# Uncomment the following to enable bzip2 compression
#CFLAGS += -DENABLE_BZIP2_COMPRESSION
#LIBS += -lbz2

ifdef version
  ifeq "$(version)" "pthreads"
    CFLAGS += -DENABLE_PTHREADS -lpthread
    DEDUP_OBJ += queue.o binheap.o tree.o
  endif
endif


all: $(TARGET) $(TARGET_SIM)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(DEDUP_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(DEDUP_OBJ) $(LIBS) $(LIB_UPTCP)

$(TARGET_SIM): $(DEDUP_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET_SIM) $(DEDUP_OBJ) $(LIBS) $(LIB_UPTCP_SIM)

clean:
	rm -f *~ *.o $(TARGET) $(TARGET_SIM)

