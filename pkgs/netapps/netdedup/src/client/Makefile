# Makefile for dedup kernel

PREFIX=${PARSECDIR}/pkgs/netapps/netdedup/inst/${PARSECPLAT}

TARGET=netdedup_client
TARGET_SIM=netdedup_client_sim

BIGFILE_CFLAGS=$(shell getconf LFS_CFLAGS)

CFLAGS += -Wall -fno-strict-aliasing -D_XOPEN_SOURCE=600 -DENABLE_PARSEC_UPTCPIP -O0 -g 
#CFLAGS += -Wall -fno-strict-aliasing -D_XOPEN_SOURCE=600  -O0 -g 
LIBS += -lpthread

LIB_UPTCP= -luptcp.client
LIB_UPTCP_SIM= -luptcp.client.sim

DEDUP_OBJ =  dedup_client.o 

all: $(TARGET) $(TARGET_SIM)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(DEDUP_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(DEDUP_OBJ) $(LIBS) $(LIB_UPTCP)

$(TARGET_SIM): $(DEDUP_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET_SIM) $(DEDUP_OBJ) $(LIBS) $(LIB_UPTCP_SIM)

clean:
	rm -f *~ *.o $(TARGET) $(TARGET_SIM)

