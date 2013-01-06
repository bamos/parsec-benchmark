PREFIX = ${PARSECDIR}/pkgs/netapps/netferret/inst/${PARSECPLAT}
TARGET = client
TARGET_SIM = client_sim

LIB_UPTCP= -luptcp.client -lpthread
LIB_UPTCP_SIM= -luptcp.client.sim -lpthread

OBJS = client.o tpool.o

OTHER_OBJS = ../server/parsec/obj/queue.o 

CFLAGS += -I../server/include -DENABLE_PARSEC_UPTCPIP

.PHONY:	all 

all: $(TARGET) $(TARGET_SIM)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(OTHER_OBJS) $(LIBS) $(LIB_UPTCP) -o $(TARGET)

$(TARGET_SIM): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(OTHER_OBJS) $(LIBS) $(LIB_UPTCP_SIM) -o $(TARGET_SIM)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@ 

