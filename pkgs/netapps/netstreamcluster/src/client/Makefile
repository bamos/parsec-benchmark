# Makefile for streamcluster

PREFIX=${PARSECDIR}/pkgs/netapps/netstreamcluster/inst/${PARSECPLAT}

TARGET=client
TARGET_SIM=client_sim

OBJS=client.o

CXXFLAGS += -DENABLE_PARSEC_UPTCPIP

LIB_UPTCP= -luptcp.client -lpthread
LIB_UPTCP_SIM= -luptcp.client.sim -lpthread

all: $(TARGET) $(TARGET_SIM)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) $(LIB_UPTCP) -o $(TARGET)

$(TARGET_SIM): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) $(LIB_UPTCP_SIM) -o $(TARGET_SIM)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o $(TARGET)

